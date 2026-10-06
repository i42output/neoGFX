// main_window.cpp
/*
neoGFX Design Studio
Copyright(C) 2020 Leigh Johnston

This program is free software: you can redistribute it and / or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <neogfx/tools/DesignStudio/DesignStudio.hpp>
#include <neogfx/tools/DesignStudio/context_menu.hpp>
#include <neogfx/tools/DesignStudio/i_node.hpp>
#include "main_window.hpp"
#include "widget_caddy.hpp"
#include <neogfx/gui/dialog/color_dialog.hpp>
#include <neogfx/gui/dialog/font_dialog.hpp>

namespace neogfx::DesignStudio
{
    namespace
    {
        // set while the Properties toolbox and a property are being made consistent (a cell updated to show its property's changed value or 
        // a property set from its cell) so that neither recursively updates the other
        thread_local bool tUpdatingProperty = false;

        // the .nrc attribute that would be associated with an object property: its name in snake case (e.g. SizePolicy: "size_policy") 
        // except FixedSize ("size": an .nrc size is the fixed size) and Size (none: it is the extents)
        std::string attribute_name_of(std::string const& aPropertyName)
        {
            if (aPropertyName == "FixedSize")
                return "size";
            if (aPropertyName == "Size")
                return {};
            std::string result;
            for (auto ch : aPropertyName)
            {
                if (std::isupper(static_cast<unsigned char>(ch)))
                {
                    if (!result.empty())
                        result += '_';
                    result += static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
                }
                else
                    result += ch;
            }
            return result;
        }

        // the object property that would be associated with an .nrc attribute (the reverse of attribute_name_of)
        std::string property_name_of(std::string const& aAttribute)
        {
            if (aAttribute == "size")
                return "FixedSize";
            std::string result;
            bool upper = true;
            for (auto ch : aAttribute)
            {
                if (ch == '_')
                    upper = true;
                else
                {
                    result += upper ? static_cast<char>(std::toupper(static_cast<unsigned char>(ch))) : ch;
                    upper = false;
                }
            }
            return result;
        }

        // an .nrc attribute value's items (e.g. "[ 42px 3cm ]": "42px" and "3cm"; strings unquoted)
        std::vector<std::string> attribute_items(std::string const& aValue)
        {
            std::vector<std::string> result;
            for (auto const& item : nrc_attributes::items(aValue))
                result.push_back(nrc_attributes::unquote(item));
            return result;
        }

        // an .nrc attribute value from items (as entered: a length's units are kept, e.g. "42 px" is "42px"); strings are quoted
        std::string attribute_value(std::vector<std::string> const& aItems, bool aStrings)
        {
            std::vector<std::string> values;
            for (auto const& item : aItems)
            {
                std::string value;
                if (aStrings)
                {
                    value = "\"";
                    for (auto ch : item)
                        switch (ch)
                        {
                        case '\"': value += "\\\""; break;
                        case '\\': value += "\\\\"; break;
                        case '\n': value += "\\n"; break;
                        case '\r': value += "\\r"; break;
                        case '\t': value += "\\t"; break;
                        default: value += ch; break;
                        }
                    value += "\"";
                }
                else
                    for (auto ch : item)
                        if (ch != ' ' && ch != '\t')
                            value += ch;
                values.push_back(value);
            }
            if (values.size() == 1u)
                return values[0];
            std::string result = "[";
            for (auto const& value : values)
                result += " " + value;
            return result + " ]";
        }
    }

    class compact_tree_view : public ng::tree_view
    {
    public:
        using ng::tree_view::tree_view;
    public:
        size minimum_size(optional_size const& aAvailableSpace = {}) const override
        {
            return ng::tree_view::minimum_size(aAvailableSpace) + total_item_area(*this);
        }
    public:
        void model_changed() override
        {
            ng::tree_view::model_changed();
            update_layout(true, true);
        }
        void presentation_model_changed() override
        {
            ng::tree_view::presentation_model_changed();
            update_layout(true, true);
        }
        void tree_changed() override
        {
            ng::tree_view::tree_changed();
            update_layout(true, true);
        }
    };

    main_window_ex::main_window_ex(main_app& aApp, settings& aSettings, project_manager& aProjectManager) :
        main_window{ aApp },
        iProjectManager{ aProjectManager },
        autoscaleDocks{ aSettings.setting("environment.windows_and_tabs.autoscale_docks"_s) },
        workspaceSize{ aSettings.setting("environment.windows_and_tabs.workspace_size"_s) },
        workspacePosition{ aSettings.setting("environment.windows_and_tabs.workspace_position"_s) },
        leftDockWidth{ aSettings.setting("environment.windows_and_tabs.left_dock_width"_s) },
        rightDockWidth{ aSettings.setting("environment.windows_and_tabs.right_dock_width"_s) },
        leftDockWeight{ aSettings.setting("environment.windows_and_tabs.left_dock_weight"_s) },
        rightDockWeight{ aSettings.setting("environment.windows_and_tabs.right_dock_weight"_s) },
        workspaceFont{ aSettings.setting("environment.fonts_and_colors.workspace_font"_s) },
        subpixelRendering{ aSettings.setting("environment.fonts_and_colors.subpixel"_s) },
        toolbarIconSize{ aSettings.setting("environment.toolbars.icon_size"_s) },
        themeColor{ aSettings.setting("environment.fonts_and_colors.theme"_s) },
        workspaceGridType{ aSettings.setting("environment.workspace.grid_type"_s) },
        workspaceGridSize{ aSettings.setting("environment.workspace.grid_size"_s) },
        workspaceGridSubdivisions{ aSettings.setting("environment.workspace.grid_subdivisions"_s) },
        workspaceGridColor{ aSettings.setting("environment.workspace.grid_color"_s) },
        iLeftDock{ dock_layout(ng::dock_area::Left), ng::dock_area::Left, ng::size{ leftDockWidth.value<double>() }, ng::size{ leftDockWeight.value<double>() } },
        iRightDock{ dock_layout(ng::dock_area::Right), ng::dock_area::Right, ng::size{ rightDockWidth.value<double>() }, ng::size{ rightDockWeight.value<double>() } },
        iProject{ ng::make_dockable<compact_tree_view>("Project"_t, ng::dock_area::Left, true, ng::frame_style::NoFrame) },
        iToolbox{ ng::make_dockable<ng::tree_view>("Toolbox"_t, ng::dock_area::Left, true, ng::frame_style::NoFrame) },
        iWorkflow{ ng::make_dockable<ng::list_view>("Workflow"_t, ng::dock_area::Left, true, ng::frame_style::NoFrame) },
        iObjects{ ng::make_dockable<ng::table_view>("Object Explorer"_t, ng::dock_area::Right, true, ng::frame_style::NoFrame) },
        iProperties{ ng::make_dockable<ng::table_view>("Properties"_t, ng::dock_area::Right, true, ng::frame_style::NoFrame) },
        iWorkspaceLayout{ client_layout() },
        iWorkspace{ iWorkspaceLayout },
        iBackgroundTexture1{ ng::image{ ":/neogfx/DesignStudio/resources/neoGFX.png" } },
        iBackgroundTexture2{ ng::image{ ":/neogfx/DesignStudio/resources/logo_i42.png" } },
        iProjectPresentationModel{ aProjectManager },
        iToolboxPresentationModel{ aProjectManager },
        iWorkflowPresentationModel{ aProjectManager },
        iObjectPresentationModel{ aProjectManager }
    {
        hide();

        init_view_actions(aApp, aSettings);
        init_docks();
        init_appearance();
        init_tool_views();
        init_object_explorer();
        init_properties();
        init_workspace(aApp);
        init_file_actions(aApp, aSettings, aProjectManager);

        activate();
    }

    // View menu: toolbar, status bar and layout icons
    void main_window_ex::init_view_actions(main_app& aApp, settings& aSettings)
    {
        aApp.actionShowStandardToolbar.checked([&]() { standardToolbar.show(); });
        aApp.actionShowStandardToolbar.unchecked([&]() { standardToolbar.hide(); });
        aApp.actionShowStatusBar.checked([&]() { statusBar.show(); });
        aApp.actionShowStatusBar.unchecked([&]() { statusBar.hide(); });

        auto& showLayoutIcons = aSettings.setting("environment.workspace.show_layout_icons"_s);
        aApp.actionShowLayoutIcons.set_checked(showLayoutIcons.value<bool>(true));
        aApp.actionShowLayoutIcons.checked([&showLayoutIcons]() { if (!showLayoutIcons.value<bool>(true)) showLayoutIcons.set_value(true); });
        aApp.actionShowLayoutIcons.unchecked([&showLayoutIcons]() { if (showLayoutIcons.value<bool>(true)) showLayoutIcons.set_value(false); });
        auto showLayoutIconsChanged = [&aApp, &showLayoutIcons]()
        {
            aApp.actionShowLayoutIcons.set_checked(showLayoutIcons.value<bool>(true));
        };
        showLayoutIcons.changing(showLayoutIconsChanged);
        showLayoutIcons.changed(showLayoutIconsChanged);
    }

    // the window's size and position, the docks and their dockables (with their sizes/weights saved in the settings)
    void main_window_ex::init_docks()
    {
        if (!workspaceSize.is_default())
            set_extents(workspaceSize.value<ng::size>());
        if (!workspacePosition.is_default())
            move(workspacePosition.value<ng::point>());
        else
            center(false);

        // todo: tidier way of doing this...
        dock_layout(ng::layout_position::Center).set_weight(ng::size{ 1.0 } - iLeftDock.parent_layout().weight() - iRightDock.parent_layout().weight());
        iLeftDock.hide();
        iRightDock.hide();

        auto autoscaleDocksChanged = [&]()
        {
            dock_layout(ng::layout_position::Center).parent_layout().fix_weightings();
            dock_layout(ng::layout_position::Center).parent_layout().set_autoscale(autoscaleDocks.value<bool>(true) ? ng::autoscale::Active : ng::autoscale::Default);
        };
        autoscaleDocks.changing(autoscaleDocksChanged);
        autoscaleDocks.changed(autoscaleDocksChanged);
        dock_layout(ng::layout_position::Center).parent_layout().set_autoscale(autoscaleDocks.value<bool>(true) ? ng::autoscale::Active : ng::autoscale::Default);

        ng::get_property(mainWindow, "Size").property_changed([&](const ng::property_variant& aValue)
        {
            workspaceSize.set_value(std::get<ng::size>(aValue));
        });
        ng::get_property(mainWindow, "Position").property_changed([&](const ng::property_variant& aValue)
        {
            workspacePosition.set_value(std::get<ng::point>(aValue));
        });
        ng::get_property(iLeftDock.parent_layout(), "Size").property_changed([&](const ng::property_variant& aValue)
        {
            leftDockWidth.set_value(std::get<ng::size>(aValue).cx);
        });
        ng::get_property(iRightDock.parent_layout(), "Size").property_changed([&](const ng::property_variant& aValue)
        {
            rightDockWidth.set_value(std::get<ng::size>(aValue).cx);
        });
        ng::get_property(iLeftDock.parent_layout(), "Weight").property_changed([&](const ng::property_variant& aValue)
        {
            if (aValue != ng::none)
                leftDockWeight.set_value(std::get<ng::size>(aValue).cx);
        });
        ng::get_property(iRightDock.parent_layout(), "Weight").property_changed([&](const ng::property_variant& aValue)
        {
            if (aValue != ng::none)
                rightDockWeight.set_value(std::get<ng::size>(aValue).cx);
        });

        iProject.set_weight(ng::size{ 0.0 });
        iToolbox.set_weight(ng::size{ 3.0 });
        iWorkflow.set_weight(ng::size{ 1.0 });

        iProject.dock(iLeftDock);
        iToolbox.dock(iLeftDock);
        iWorkflow.dock(iLeftDock);
        iObjects.dock(iRightDock);
        iProperties.dock(iRightDock);

        iToolbox.docked_widget<ng::tree_view>().enable_drag_drop_source();
        iWorkflow.docked_widget<ng::list_view>().enable_drag_drop_source();

        ng::i_layout& mainLayout = client_layout();
        mainLayout.set_padding(ng::padding{});
        mainLayout.set_spacing(ng::size{});
    }

    // appearance settings: font, subpixel rendering, toolbar icon size, theme colour and workspace grid
    void main_window_ex::init_appearance()
    {
        auto fontChanged = [&]()
        {
            ng::service<ng::i_app>().current_style().set_font_info(workspaceFont.value<ng::font_info>(true));
        };
        workspaceFont.changing(fontChanged);
        workspaceFont.changed(fontChanged);
        fontChanged();

        auto subpixelRenderingChanged = [&]()
        {
            if (subpixelRendering.value<bool>(true))
                ng::service<ng::i_rendering_engine>().subpixel_rendering_on();
            else
                ng::service<ng::i_rendering_engine>().subpixel_rendering_off();
        };
        subpixelRendering.changing(subpixelRenderingChanged);
        subpixelRendering.changed(subpixelRenderingChanged);
        subpixelRenderingChanged();

        auto toolbarIconSizeChanged = [&]()
        {
            switch (toolbarIconSize.value<toolbar_icon_size>(true))
            {
            case toolbar_icon_size::Size16x16:
                standardToolbar.set_button_image_extents(ng::size{ 16.0_dip, 16.0_dip });
                break;
            case toolbar_icon_size::Size24x24:
                standardToolbar.set_button_image_extents(ng::size{ 24.0_dip, 24.0_dip });
                break;
            case toolbar_icon_size::Size32x32:
                standardToolbar.set_button_image_extents(ng::size{ 32.0_dip, 32.0_dip });
                break;
            case toolbar_icon_size::Size48x48:
                standardToolbar.set_button_image_extents(ng::size{ 48.0_dip, 48.0_dip });
                break;
            case toolbar_icon_size::Size64x64:
                standardToolbar.set_button_image_extents(ng::size{ 64.0_dip, 64.0_dip });
                break;
            }
        };
        toolbarIconSize.changing(toolbarIconSizeChanged);
        toolbarIconSize.changed(toolbarIconSizeChanged);
        toolbarIconSizeChanged();

        auto themeColorChanged = [&]()
        {
            ng::service<ng::i_app>().current_style().set_palette_color(ng::color_role::Theme, themeColor.value<ng::color>(true));
            iWorkspace.view_stack().set_background_color(ng::service<ng::i_app>().current_style().palette().color(ng::color_role::Base));
            workspaceGridColor.set_default_value(ng::gradient{ ng::service<ng::i_app>().current_style().palette().color(ng::color_role::Background).with_alpha(0.25) });
        };
        themeColor.changing(themeColorChanged);
        themeColor.changed(themeColorChanged);
        themeColorChanged();

        auto workspaceGridChanged = [&]()
        {
            iWorkspace.view_stack().update();
        };
        workspaceGridType.changing(workspaceGridChanged);
        workspaceGridType.changed(workspaceGridChanged);
        workspaceGridColor.changing(workspaceGridChanged);
        workspaceGridColor.changed(workspaceGridChanged);
        workspaceGridSize.changing(workspaceGridChanged);
        workspaceGridSize.changed(workspaceGridChanged);
        workspaceGridSubdivisions.changing(workspaceGridChanged);
        workspaceGridSubdivisions.changed(workspaceGridChanged);
    }

    // the Project, Toolbox and Workflow views
    void main_window_ex::init_tool_views()
    {
        populate_project_model(iProjectModel, iProjectPresentationModel);
        auto& projectTree = iProject.docked_widget<compact_tree_view>();
        projectTree.set_presentation_model(iProjectPresentationModel);
        projectTree.selection_model().set_mode(ng::item_selection_mode::SingleSelection);
        projectTree.set_focus_policy(ng::focus_policy::TabFocus);

        populate_toolbox_model(iToolboxModel, iToolboxPresentationModel);
        auto& toolboxTree = iToolbox.docked_widget<ng::tree_view>();
        toolboxTree.set_minimum_size(ng::size{ 128_dip, 128_dip });
        toolboxTree.set_presentation_model(iToolboxPresentationModel);
        toolboxTree.selection_model().set_mode(ng::item_selection_mode::SingleSelection);
        toolboxTree.set_focus_policy(ng::focus_policy::TabFocus);

        populate_workflow_model(iWorkflowModel, iWorkflowPresentationModel);
        iWorkflowPresentationModel.set_item_model(iWorkflowModel);
        iWorkflowPresentationModel.set_column_read_only(0u);
        auto& workflowTree = iWorkflow.docked_widget<ng::list_view>();
        workflowTree.set_minimum_size(ng::size{ 128_dip, 128_dip });
        workflowTree.set_presentation_model(iWorkflowPresentationModel);
        workflowTree.selection_model().set_mode(ng::item_selection_mode::NoSelection);
        workflowTree.set_focus_policy(ng::focus_policy::TabFocus);
    }

    // Object Explorer: the current/selected elements, moving elements by drag and drop, context menu and clipboard
    void main_window_ex::init_object_explorer()
    {
        iObjectModel.set_column_name(0u, "Object"_t);
        iObjectModel.set_column_name(1u, "Type"_t);

        iObjectPresentationModel.set_item_model(iObjectModel);
        iObjectPresentationModel.set_column_read_only(1u);
        iObjectPresentationModel.set_alternating_row_color(true);
        auto& objectTree = iObjects.docked_widget<ng::table_view>();
        objectTree.set_minimum_size(ng::size{ 128_dip, 128_dip });
        objectTree.set_selection_model(iObjectPresentationModel.selection_model());
        iObjectPresentationModel.selection_model().set_mode(ng::item_selection_mode::ExtendedSelection);
        objectTree.set_presentation_model(iObjectPresentationModel);
        objectTree.column_header().set_expand_last_column(true);
        iObjectPresentationModel.selection_model().current_index_changed([&](const optional_item_presentation_model_index& aCurrentIndex, const optional_item_presentation_model_index& aPreviousIndex)
        {
            if (aCurrentIndex)
            {
                auto& element = *iObjectModel.item(iObjectPresentationModel.to_item_model_index(*aCurrentIndex));
                element.set_mode(element_mode::Edit);
                iPropertyElement = weak_ref_ptr<i_element>{ element };
            }
            else 
            {
                if (aPreviousIndex)
                {
                    auto& element = *iObjectModel.item(iObjectPresentationModel.to_item_model_index(*aPreviousIndex));
                    element.set_mode(element_mode::None);
                }
                iPropertyElement = weak_ref_ptr<i_element>{};
            }
            iPropertiesNeedUpdate = true;
        });
        iObjectPresentationModel.selection_model().selection_changed([&](const ng::item_selection& aCurrentSelection, const ng::item_selection& aPreviousSelection)
        {
            static bool inHere;
            if (inHere)
                return;
            neolib::scoped_flag sf{ inHere };
            for (auto row = row_begin(aCurrentSelection); row != row_end(aCurrentSelection); ++row)
            {
                auto& element = *iObjectModel.item(iObjectPresentationModel.to_item_model_index(*row));
                element.select(true, false);
            }
            for (auto row = row_begin(aPreviousSelection); row != row_end(aPreviousSelection); ++row)
            {
                if (!contains(aCurrentSelection, *row))
                {
                    auto& element = *iObjectModel.item(iObjectPresentationModel.to_item_model_index(*row));
                    element.select(false, false);
                }
            }
        });

        // dragging elements onto Object Explorer (from within it, from the Toolbox and from the design surface) moves them (or, from the 
        // Toolbox, adds them): onto a container (layout, window, group box, tab page) appends to it, onto any other element inserts before it, 
        // onto the project/user interface makes a widget top level (not from the Toolbox)
        auto& toolboxTree = iToolbox.docked_widget<ng::tree_view>();
        objectTree.enable_drag_drop_source();
        objectTree.enable_drag_drop_target();
        struct object_drop
        {
            i_element* container = nullptr;
            i_element const* before = nullptr;
            bool toCanvas = false;
        };
        // where an element dropped at a position in Object Explorer goes (aDragged: the element being moved, if any (a new element from the 
        // Toolbox has none yet))
        auto resolve_drop_at = [this, &objectTree](i_element* aDragged, point const& aPosition) -> object_drop
        {
            object_drop result;
            if (preview_mode() || !iProjectManager.project_active())
                return result;
            if (aDragged != nullptr && !can_be_moved(*aDragged))
                return result;
            auto const targetIndex = objectTree.item_at(aPosition);
            if (!targetIndex)
                return result;
            auto& target = *iObjectModel.item(iObjectPresentationModel.to_item_model_index(*targetIndex));
            if (aDragged != nullptr)
            {
                if (&target == aDragged)
                    return result;
                for (i_element const* e = &target; e->has_parent(); e = &e->parent())
                    if (&e->parent() == aDragged)
                        return result; // can't move into itself
            }
            if (target.group() == element_group::Project || target.group() == element_group::UserInterface)
            {
                if (aDragged != nullptr && aDragged->group() == element_group::Widget && aDragged->is_nested())
                {
                    result.container = &target;
                    result.toCanvas = true;
                }
                return result;
            }
            if (target.has_child_layout())
                result.container = &target;
            else if (target.has_parent() && target.parent().has_child_layout())
            {
                result.container = &target.parent();
                result.before = &target;
            }
            return result;
        };
        // the element being dragged (from within Object Explorer) or, from the Toolbox, none (a new element); false if not an element
        auto dragged_element = [this, &toolboxTree](i_drag_drop_object const& aObject, i_element*& aElement) -> bool
        {
            aElement = nullptr;
            if (aObject.ddo_type() != i_drag_drop_item::otid())
                return false;
            auto const& item = static_cast<i_drag_drop_item const&>(aObject);
            if (&item.presentation_model() == static_cast<i_item_presentation_model const*>(&iObjectPresentationModel))
            {
                aElement = iObjectModel.item(iObjectPresentationModel.to_item_model_index(item.index()));
                return true;
            }
            if (&item.presentation_model() == &toolboxTree.presentation_model())
                return std::holds_alternative<element_tool_t>(iToolboxModel.item(iToolboxPresentationModel.to_item_model_index(item.index())));
            return false;
        };
        auto move_dropped = [this](i_element& aElement, object_drop const& aDrop)
        {
            auto& project = iProjectManager.active_project();
            try
            {
                if (aDrop.toCanvas)
                {
                    auto& workspace = iWorkspace.view_stack();
                    move_element_to_canvas(project, aElement, *aDrop.container, workspace, design_rect(workspace).top_left() + point{ 128.0_dip, 128.0_dip });
                }
                else
                    move_element_to_container(project, aElement, *aDrop.container, aDrop.before);
            }
            catch (...)
            {
                // not droppable there
            }
        };
        objectTree.object_acceptable([resolve_drop_at, dragged_element](i_drag_drop_object const& aObject, optional_point const& aDropPosition, drop_operation& aOperation)
        {
            i_element* dragged = nullptr;
            if (aDropPosition && dragged_element(aObject, dragged) && resolve_drop_at(dragged, *aDropPosition).container != nullptr)
                aOperation = drop_operation::Move;
        });
        // (a new element from the Toolbox is added by the Toolbox when it is dropped (just after this): where it goes is noted here for it)
        auto toolboxDrop = std::make_shared<std::optional<object_drop>>();
        objectTree.object_dropped([resolve_drop_at, dragged_element, move_dropped, toolboxDrop](i_drag_drop_object const& aObject, optional_point const& aDropPosition)
        {
            i_element* dragged = nullptr;
            if (!aDropPosition || !dragged_element(aObject, dragged))
                return;
            auto const drop = resolve_drop_at(dragged, *aDropPosition);
            if (drop.container == nullptr)
                return;
            if (dragged == nullptr)
                *toolboxDrop = drop;
            else
                move_dropped(*dragged, drop);
        });
        iToolboxPresentationModel.set_drop_resolver([&objectTree, toolboxDrop](i_drag_drop_target& aTarget) -> std::optional<std::pair<i_element*, i_element const*>>
        {
            if (!aTarget.is_widget() || &aTarget.as_widget() != static_cast<i_widget const*>(&objectTree))
                return {};
            std::pair<i_element*, i_element const*> result;
            if (*toolboxDrop)
                result = { (*toolboxDrop)->container, (*toolboxDrop)->before };
            *toolboxDrop = std::nullopt;
            return result;
        });
        // an element dragged on the design surface and released over Object Explorer
        set_external_element_drop([this, &objectTree, resolve_drop_at, move_dropped](i_element& aElement, point const& aPosition) -> bool
        {
            auto const position = objectTree.to_client_coordinates(aPosition - objectTree.root().window_position());
            if (objectTree.effectively_hidden() || !objectTree.client_rect().contains(position))
                return false;
            auto const drop = resolve_drop_at(&aElement, position);
            if (drop.container != nullptr)
                move_dropped(aElement, drop);
            return true; // (released over Object Explorer: not dropped on the design surface whether or not it was moved)
        });
        // what is dragged from Object Explorer is shown under the mouse: the element's icon and id
        iSink += iObjectPresentationModel.dragging_item_render_info([&objectTree](i_drag_drop_item const& aItem, bool& aCanRender, size& aExtents)
        {
            size const icon{ 16.0_dip, 16.0_dip };
            aCanRender = true;
            aExtents = size{ icon.cx * 1.25 + objectTree.font().height() * 8.0, std::max(icon.cy, objectTree.font().height()) };
        });
        iSink += iObjectPresentationModel.dragging_item_render([this, &objectTree](i_drag_drop_item const& aItem, i_graphics_context& aGc, point const& aPosition)
        {
            auto const& element = *iObjectModel.item(iObjectPresentationModel.to_item_model_index(aItem.index()));
            size const icon{ 16.0_dip, 16.0_dip };
            auto const position = aPosition + point{ icon * 0.75 }; // (beside the mouse pointer)
            aGc.draw_texture(rect{ position, icon }, element.library().element_icon(element.type()));
            aGc.draw_text(position + point{ icon.cx * 1.25, (icon.cy - objectTree.font().height()) / 2.0 }, string{ element.id() }, objectTree.font(),
                text_format{ service<i_app>().current_style().palette().color(color_role::Text), text_effect{ text_effect_type::Outline, 
                    service<i_app>().current_style().palette().color(color_role::Base), 2.0 } });
        });
        // while an element is dragged (from Object Explorer, the Toolbox or the design surface) where it would be dropped is shown: in 
        // Object Explorer the row it would go into (highlighted) or before (a line above it) and, for one from Object Explorer, on the 
        // design surface the layout it would go into and where (as when dragged on the design surface)
        struct drop_indicator
        {
            std::optional<item_presentation_model_index> row;
            bool before = false;
            ref_ptr<i_widget> designHighlight;
            std::optional<std::pair<i_element*, bool>> drag; // the drag in progress (if any): the element dragged (none: new, from the Toolbox) and aDesignSurface
        };
        auto indicator = std::make_shared<drop_indicator>();
        // (aPosition: the (screen) position being dragged over, if any (none: the drag has ended); aDesignSurface: also show it on the design surface)
        auto update_drop_indicators = [this, &objectTree, resolve_drop_at, indicator](i_element* aDragged, optional_point const& aPosition, bool aDesignSurface)
        {
            if (aPosition)
                indicator->drag.emplace(aDragged, aDesignSurface);
            else
                indicator->drag = std::nullopt;
            std::optional<item_presentation_model_index> row;
            bool before = false;
            if (aPosition && !objectTree.effectively_hidden())
            {
                auto const position = objectTree.to_client_coordinates(*aPosition - objectTree.root().window_position());
                if (objectTree.client_rect().contains(position))
                {
                    auto const drop = resolve_drop_at(aDragged, position);
                    if (drop.container != nullptr)
                    {
                        row = objectTree.item_at(position);
                        before = (drop.before != nullptr);
                    }
                }
            }
            if (row != indicator->row || before != indicator->before)
            {
                indicator->row = row;
                indicator->before = before;
                objectTree.update();
            }
            i_element* container = nullptr;
            if (aDesignSurface && aDragged != nullptr && aPosition && !row && iProjectManager.project_active() && can_be_moved(*aDragged))
            {
                auto& viewStack = iWorkspace.view_stack();
                if (viewStack.client_rect().contains(viewStack.to_client_coordinates(*aPosition - viewStack.root().window_position())))
                    container = find_drop_container(iProjectManager.active_project().root(), *aDragged, *aPosition);
            }
            if (container != nullptr)
            {
                try
                {
                    show_drop_highlight(*container, aDragged->type(), indicator->designHighlight, *aPosition, &aDragged->caddy());
                    return;
                }
                catch (...)
                {
                    // not droppable there
                }
            }
            // (only if it is shown: hiding it also stops the insertion line (there is one) shown by a drag on the design surface or from the Toolbox)
            if (indicator->designHighlight)
                hide_drop_highlight(indicator->designHighlight);
        };
        iSink += objectTree.painted([this, &objectTree, indicator](i_graphics_context& aGc)
        {
            if (!indicator->row)
                return;
            auto const lastColumn = iObjectPresentationModel.columns() - 1u;
            auto const rowRect = objectTree.cell_rect(indicator->row->with_column(0u), cell_part::Background).combined(
                objectTree.cell_rect(indicator->row->with_column(lastColumn), cell_part::Background));
            if (indicator->before)
                aGc.draw_line(rowRect.top_left(), rowRect.top_right(), pen{ color::Yellow, 2.0_dip });
            else
            {
                aGc.fill_rect(rowRect, color::Yellow.with_alpha(0.25));
                aGc.draw_rect(rowRect, pen{ color::Yellow, 1.0 });
            }
        });
        // (Object Explorer scrolled (e.g. by the mouse wheel) while dragging: the row under the mouse has changed)
        iSink += static_cast<ng::i_object&>(objectTree.vertical_scrollbar()).property_changed([&objectTree, indicator, update_drop_indicators](i_property const& aProperty)
        {
            if (!indicator->drag || aProperty.name().to_std_string_view() != "Position")
                return;
            auto const drag = *indicator->drag;
            update_drop_indicators(drag.first, objectTree.root().mouse_position() + objectTree.root().window_position(), drag.second);
        });
        // (dragging from Object Explorer or the Toolbox: these views capture the mouse while dragging)
        iSink += objectTree.mouse_event([&objectTree, dragged_element, update_drop_indicators](neogfx::mouse_event const& aEvent)
        {
            i_element* dragged = nullptr;
            if (aEvent.type() != mouse_event_type::Moved || !objectTree.drag_drop_active() || 
                !dragged_element(objectTree.object_being_dragged(), dragged) || dragged == nullptr)
                return;
            // (once dragged out of Object Explorer it is a design drag (as on the design surface): layouts there show where things can be dropped)
            auto const position = aEvent.position() - objectTree.origin();
            if (!design_drag_active() && !objectTree.client_rect().contains(position))
                set_design_drag_active(true);
            update_drop_indicators(dragged, objectTree.to_window_coordinates(position) + objectTree.root().window_position(), true);
        });
        iSink += toolboxTree.mouse_event([&toolboxTree, dragged_element, update_drop_indicators](neogfx::mouse_event const& aEvent)
        {
            i_element* dragged = nullptr;
            if (aEvent.type() != mouse_event_type::Moved || !toolboxTree.drag_drop_active() || 
                !dragged_element(toolboxTree.object_being_dragged(), dragged))
                return;
            update_drop_indicators(nullptr, toolboxTree.to_window_coordinates(aEvent.position() - toolboxTree.origin()) + toolboxTree.root().window_position(), false);
        });
        set_external_element_drag([update_drop_indicators](i_element& aElement, optional_point const& aPosition)
        {
            update_drop_indicators(&aElement, aPosition, false);
        });
        iSink += iObjectPresentationModel.dragging_item_cancelled([update_drop_indicators](i_drag_drop_item const&)
        {
            update_drop_indicators(nullptr, {}, false);
            set_design_drag_active(false);
        });
        iSink += iObjectPresentationModel.item_dropped([update_drop_indicators](i_drag_drop_item const&, i_drag_drop_target&)
        {
            update_drop_indicators(nullptr, {}, false);
            set_design_drag_active(false); // (after the drop (so its insertion point is where it was shown))
        });
        iSink += iToolboxPresentationModel.dragging_item_cancelled([update_drop_indicators](i_drag_drop_item const&)
        {
            update_drop_indicators(nullptr, {}, false);
        });
        iSink += iToolboxPresentationModel.item_dropped([update_drop_indicators](i_drag_drop_item const&, i_drag_drop_target&)
        {
            update_drop_indicators(nullptr, {}, false);
        });

        objectTree.cell_context_menu([&](item_presentation_model_index const& aIndex)
        {
            auto& element = *iObjectModel.item(iObjectPresentationModel.to_item_model_index(aIndex));
            display_element_context_menu(objectTree, element);
        });

        objectTree.Focus([&](neogfx::focus_event aEvent, focus_reason aReason)
        {
            if (aEvent == neogfx::focus_event::FocusGained)
            {
                service<i_clipboard>().activate(*this);
            }
            else if (aEvent == neogfx::focus_event::FocusLost &&
                service<i_clipboard>().sink_active() && &service<i_clipboard>().active_sink() == this)
            {
                service<i_clipboard>().deactivate(*this);
            }
        });
    }

    // Properties toolbox: editing elements' .nrc attributes and their objects' properties (see update_properties)
    void main_window_ex::init_properties()
    {
        iPropertyModel.set_column_name(0u, "Property"_t);
        iPropertyModel.set_column_name(1u, "Value"_t);
        iPropertyPresentationModel.set_item_model(iPropertyModel);
        iPropertyPresentationModel.set_alternating_row_color(true);
        auto& propertyTable = iProperties.docked_widget<ng::table_view>();
        propertyTable.set_minimum_size(ng::size{ 128_dip, 128_dip });
        propertyTable.set_presentation_model(iPropertyPresentationModel);
        propertyTable.column_header().set_expand_last_column(true);
        // each cell has a 1 pixel border
        iSink += propertyTable.painted([this, &propertyTable](ng::i_graphics_context& aGc)
        {
            auto const& palette = ng::service<ng::i_app>().current_style().palette();
            ng::pen const border{ palette.color(ng::color_role::Base).mid(palette.color(ng::color_role::Text)), 1.0 };
            auto const visible = propertyTable.client_rect();
            for (std::uint32_t row = 0u; row < iPropertyPresentationModel.rows(); ++row)
                for (std::uint32_t col = 0u; col < iPropertyPresentationModel.columns(); ++col)
                {
                    auto const cellRect = propertyTable.cell_rect(ng::item_presentation_model_index{ row, col }, ng::cell_part::Background);
                    if (cellRect.intersects(visible))
                        aGc.draw_rect(cellRect, border);
                }
        });
        iSink += iPropertyModel.item_changed([&](item_model_index const& aIndex)
        {
            if (tUpdatingProperty || iUpdatingProperties || !iPropertyElement.valid() || !iProjectManager.project_active())
                return;
            neolib::scoped_flag sfUpdatingProperty{ tUpdatingProperty };
            auto& element = *iPropertyElement;
            auto cell_text = [&](item_model_index const& aCellIndex)
            {
                auto const& cellData = iPropertyModel.cell_data(aCellIndex);
                return std::holds_alternative<string>(cellData) ? std::get<string>(cellData) : string{};
            };
            auto const text = cell_text(aIndex);
            auto const& item = iPropertyModel.item(aIndex);
            if (std::holds_alternative<std::monostate>(item))
                return; // class node
            if (std::holds_alternative<i_property*>(item))
            {
                // object property
                if (aIndex.column() != 1u)
                    return;
                auto& property = *std::get<i_property*>(item);
                ng::scoped_units_context suc{ element.layout_item() }; // (lengths can have units, e.g. "42 px")
                // (a property with an associated .nrc attribute: the text as entered (e.g. with units) is the attribute's value)
                auto const value = property_value_from_string(property, text.to_std_string());
                if (value && !set_property_attribute(property, {}, text.to_std_string()))
                    property.set_from_variant(*value);
                // show the resulting value (not rebuilding the tree so expanded composite properties stay expanded)
                neolib::scoped_flag sf{ iUpdatingProperties };
                update_property_rows(property);
                return;
            }
            if (std::holds_alternative<property_component>(item))
            {
                // a component of a composite property (e.g. a size's width)
                if (aIndex.column() != 1u)
                    return;
                auto const& component = std::get<property_component>(item);
                ng::scoped_units_context suc{ element.layout_item() }; // (lengths can have units, e.g. "42 px")
                if (auto const value = property_component_from_string(*component.property, component.index, text.to_std_string()))
                    if (!set_property_attribute(*component.property, component.index, text.to_std_string()))
                        component.property->set_from_variant(*value);
                neolib::scoped_flag sf{ iUpdatingProperties };
                update_property_rows(*component.property);
                return;
            }
            auto const attributeIndex = std::get<std::uint32_t>(item);
            if (attributeIndex == property_presentation_model::new_property_row)
            {
                // an attribute that can be added (a row of the "Attributes" node): added when given a value
                if (aIndex.column() != 1u || text.empty())
                    return;
                element.attributes().push_back(neolib::pair<string, string>{ cell_text(aIndex.with_column(0u)), text });
                iPropertiesNeedUpdate = true;
            }
            else if (aIndex.column() == 1u && attributeIndex < element.attributes().size())
                std::next(element.attributes().begin(), attributeIndex)->second() = text; // an empty value removes the property on save
            else
                return;
            element.apply_attributes(show_ids());
            iProjectManager.active_project().set_dirty();
        });
        // the property being edited in place (if any)
        auto edited_property = [this, &propertyTable]() -> i_property*
        {
            if (!propertyTable.editing() || propertyTable.editing()->column() != 1u)
                return nullptr;
            auto const& item = iPropertyModel.item(iPropertyPresentationModel.to_item_model_index(*propertyTable.editing()));
            return std::holds_alternative<i_property*>(item) ? std::get<i_property*>(item) : nullptr;
        };
        // the property whose "..." button is shown: the one being edited or, as its (read only) row can't be edited, the current composite 
        // property (e.g. a font: its row shows a summary of its components)
        auto dialog_property = [this, &propertyTable, edited_property]() -> i_property*
        {
            if (auto property = edited_property())
                return property;
            if (propertyTable.editing() || !propertyTable.selection_model().has_current_index())
                return nullptr;
            auto const& item = iPropertyModel.item(iPropertyPresentationModel.to_item_model_index(propertyTable.selection_model().current_index()));
            if (std::holds_alternative<i_property*>(item) && !property_components(*std::get<i_property*>(item)).empty())
                return std::get<i_property*>(item);
            return nullptr;
        };
        // "..." button: opens the color or font dialog for the property being edited (the result replaces the editor text and is committed) 
        // or for the current composite property (the result is its new value)
        auto open_property_dialog = [this, &propertyTable, edited_property, dialog_property]()
        {
            auto property = dialog_property();
            if (property == nullptr || iPropertyDialogOpen)
                return;
            neolib::scoped_flag sf{ iPropertyDialogOpen }; // the button mustn't be destroyed while its click handler is running
            std::string const text = edited_property() == property && propertyTable.editor_has_text_edit() ? 
                propertyTable.editor_text_edit().text().to_std_string() : property_value_to_string(*property);
            std::optional<std::string> newText;
            switch (property_dialog_for(*property))
            {
            case property_dialog::Color:
                {
                    auto const current = property_parse(text, static_cast<ng::color const*>(nullptr));
                    ng::color_dialog dialog{ *this, current && text != "(none)" && !text.empty() ? *current : ng::color::Black };
                    if (dialog.exec() == ng::dialog_result::Accepted)
                        newText = property_text(dialog.selected_color());
                }
                break;
            case property_dialog::Font:
                {
                    auto const current = property_parse(text, static_cast<ng::font const*>(nullptr));
                    ng::font_dialog dialog{ *this, current ? *current : ng::font{} };
                    if (dialog.exec() == ng::dialog_result::Accepted)
                        newText = property_text(dialog.selected_font());
                }
                break;
            default:
                break;
            }
            if (!newText)
                return;
            if (edited_property() == property && propertyTable.editor_has_text_edit())
            {
                propertyTable.editor_text_edit().set_text(*newText);
                propertyTable.end_edit(true);
            }
            else if (auto const value = property_value_from_string(*property, *newText))
                property->set_from_variant(*value); // (a composite property, or the in-place edit ended while the dialog was open; its row shows the new value when it changes)
        };
        iPropertiesUpdater.emplace(*this, [this, &propertyTable, edited_property, dialog_property, open_property_dialog](ng::widget_timer& aTimer)
        {
            aTimer.again();
            if (iPropertiesNeedUpdate || (!iPropertyElement.valid() && iPropertyModel.rows() != 0u))
            {
                iPropertiesNeedUpdate = false;
                update_properties();
            }
            else if (!iChangedProperties.empty())
            {
                // a batch update of the rows of the properties that have changed (see iChangedProperties) except one being edited
                std::optional<std::uint32_t> editingRow;
                if (propertyTable.editing())
                    editingRow = iPropertyPresentationModel.to_item_model_index(*propertyTable.editing()).row();
                neolib::scoped_flag sf{ iUpdatingProperties };
                neolib::scoped_flag sfUpdatingProperty{ tUpdatingProperty };
                {
                    // (cell by cell: a presentation model update (begin_update/end_update) resets all of its cell metadata (every cell's text 
                    // is then shaped again) and the view lays out all of its items again)
                    for (auto property : iChangedProperties)
                        update_property_rows(*property, editingRow);
                }
                for (auto property : iChangedProperties)
                    iPropertyPresentationModel.sync_cell_widgets(*property);
                iChangedProperties.clear();
            }
            // show the "..." button at the right of the in-place editor of a color or font property (or of a composite one's current row)
            if (iPropertyDialogOpen)
                return;
            auto property = dialog_property();
            if (property != nullptr && property_dialog_for(*property) != property_dialog::None)
            {
                if (!iPropertyDialogButton)
                {
                    iPropertyDialogButton = std::make_unique<ng::push_button>(propertyTable, ng::string{ "..." });
                    iPropertyDialogButton->set_focus_policy(ng::focus_policy::NoFocus); // keep focus (and the edit) in the editor
                    iPropertyDialogButton->Clicked(open_property_dialog);
                }
                auto const cellRect = edited_property() == property ? ng::rect{ propertyTable.editor().position(), propertyTable.editor().extents() } :
                    propertyTable.cell_rect(propertyTable.selection_model().current_index().with_column(1u), ng::cell_part::Editor);
                auto const buttonSize = cellRect.cy;
                iPropertyDialogButton->move(ng::point{ cellRect.x + cellRect.cx - buttonSize, cellRect.y });
                iPropertyDialogButton->resize(ng::size{ buttonSize, buttonSize });
                iPropertyDialogButton->bring_to_front();
            }
            else
                iPropertyDialogButton = nullptr;
        }, std::chrono::milliseconds{ 20 });
    }

    // the workspace (design surface): drops, painting, rubber band selection, context menu, keyboard, preview and display ids
    void main_window_ex::init_workspace(main_app& aApp)
    {
        auto& toolboxTree = iToolbox.docked_widget<ng::tree_view>();
        auto& objectTree = iObjects.docked_widget<ng::table_view>();

        // an element dragged from Object Explorer (otherwise it is from the Toolbox or Workflow (which add it when it is dropped))
        auto object_explorer_element = [this](const ng::i_drag_drop_object& aObject) -> i_element*
        {
            if (aObject.ddo_type() != i_drag_drop_item::otid())
                return nullptr;
            auto const& item = static_cast<i_drag_drop_item const&>(aObject);
            if (&item.presentation_model() != static_cast<i_item_presentation_model const*>(&iObjectPresentationModel))
                return nullptr;
            return iObjectModel.item(iObjectPresentationModel.to_item_model_index(item.index()));
        };
        iWorkspace.view_stack().enable_drag_drop_target();
        iWorkspace.view_stack().object_acceptable([&, object_explorer_element](const ng::i_drag_drop_object& aObject, ng::optional_point const& aDropPosition, ng::drop_operation& aOperation)
        {
            auto const element = object_explorer_element(aObject);
            aOperation = preview_mode() || (element != nullptr && !can_be_moved(*element)) ? ng::drop_operation::None : ng::drop_operation::Move;
        });
        // an element dragged from Object Explorer onto the design surface: into the container there (as when dragged on the design surface) 
        // or, if none, a widget is made top level there
        iWorkspace.view_stack().object_dropped([&, object_explorer_element](const ng::i_drag_drop_object& aObject, ng::optional_point const& aDropPosition)
        {
            auto const element = object_explorer_element(aObject);
            if (element == nullptr || !aDropPosition || !iProjectManager.project_active())
                return;
            auto& project = iProjectManager.active_project();
            auto& viewStack = iWorkspace.view_stack();
            auto const designPosition = viewStack.to_window_coordinates(*aDropPosition) + viewStack.root().window_position();
            try
            {
                if (auto const container = find_drop_container(project.root(), *element, designPosition))
                    move_element_to(project, *element, *container, designPosition);
                else if (element->is_nested())
                    move_element_to_canvas(project, *element, project.root(), viewStack, designPosition);
            }
            catch (...)
            {
                // not droppable there
            }
        });
        iWorkspace.view_stack().set_focus_policy(ng::focus_policy::ClickFocus);

        thread_local optional_point tMouseSelectorAnchor;
        thread_local optional_point tMouseSelectorMousePos;

        iWorkspace.view_stack().Painting([&](ng::i_graphics_context& aGc) 
        { 
            paint_workspace(aGc); 
        });

        iWorkspace.view_stack().Painted([&](ng::i_graphics_context& aGc)
        {
            if (tMouseSelectorAnchor)
            {
                aGc.draw_rect(rect{ tMouseSelectorAnchor->min(*tMouseSelectorMousePos), tMouseSelectorAnchor->max(*tMouseSelectorMousePos) },
                    service<i_app>().current_style().palette().color(color_role::Selection), service<i_app>().current_style().palette().color(color_role::Selection).with_alpha(0.25));
            }
        });

        iWorkspace.view_stack().Focus([&](neogfx::focus_event aEvent, focus_reason aReason)
        {
            if (aEvent == neogfx::focus_event::FocusGained)
            {
                service<i_clipboard>().activate(*this);
            }
            else if (aEvent == neogfx::focus_event::FocusLost &&
                service<i_clipboard>().sink_active() && &service<i_clipboard>().active_sink() == this)
            {
                service<i_clipboard>().deactivate(*this);
            }
        });

        iWorkspace.view_stack().set_focus();

        iWorkspace.view_stack().Mouse([&](ng::mouse_event const& aEvent)
        {
            if (!iProjectManager.project_active() || preview_mode())
                return;
            auto const eventPos = aEvent.position() - iWorkspace.view_stack().origin();
            switch (aEvent.type())
            {
            case mouse_event_type::ButtonClicked:
                if (aEvent.is_left_button())
                {
                    // only a click on the empty canvas starts a rubber band (not one on an element, which its caddy handles)
                    auto const designPos = iWorkspace.view_stack().to_window_coordinates(eventPos) + iWorkspace.view_stack().root().window_position();
                    bool onElement = false;
                    iProjectManager.active_project().root().visit([&](i_element& aElement)
                    {
                        if (!onElement && aElement.has_caddy() && !aElement.caddy().effectively_hidden() && design_rect(aElement.caddy()).contains(designPos))
                            onElement = true;
                    });
                    if (onElement)
                        break;
                    iProjectManager.active_project().root().select(false, true);
                    iProjectManager.active_project().root().visit([&](i_element& aElement) { aElement.set_mode(element_mode::None); });
                    tMouseSelectorAnchor = eventPos;
                    tMouseSelectorMousePos = eventPos;
                    iWorkspace.view_stack().update();
                }
                break;
            case mouse_event_type::Moved:
                if (tMouseSelectorAnchor)
                {
                    tMouseSelectorMousePos = eventPos;
                    // (positions compared as design positions so that elements in a window being designed (a nested window) are where they appear)
                    auto& viewStack = iWorkspace.view_stack();
                    auto const origin = viewStack.to_window_coordinates(point{}) + viewStack.root().window_position();
                    rect const band{ tMouseSelectorAnchor->min(*tMouseSelectorMousePos) + origin, tMouseSelectorAnchor->max(*tMouseSelectorMousePos) + origin };
                    iProjectManager.active_project().root().visit([&](i_element& aElement)
                    {
                        if (aElement.has_caddy() && !aElement.caddy().effectively_hidden() &&
                            (aElement.group() == element_group::Widget || aElement.group() == element_group::Layout))
                            aElement.select(band.contains(design_rect(aElement.caddy()).center()), false);
                    });
                    iWorkspace.view_stack().update();
                }
                break;
            case mouse_event_type::ButtonReleased:
                if (aEvent.is_left_button() && tMouseSelectorAnchor)
                {
                    tMouseSelectorAnchor = std::nullopt;
                    tMouseSelectorMousePos = std::nullopt;
                    iWorkspace.view_stack().update();
                }
                else if (aEvent.is_right_button())
                {
                    context_menu menu{ *this, root().mouse_position() + root().window_position() };
                    auto& actionCut = service<i_app>().action_cut();
                    auto& actionCopy = service<i_app>().action_copy();
                    auto& actionPaste = service<i_app>().action_paste();
                    auto& actionDelete = service<i_app>().action_delete();
                    auto& actionSelectAll = service<i_app>().action_select_all();
                    menu.menu().add_action(actionCut);
                    menu.menu().add_action(actionCopy);
                    menu.menu().add_action(actionPaste);
                    menu.menu().add_action(actionDelete);
                    menu.menu().add_separator();
                    menu.menu().add_action(actionSelectAll);
                    menu.exec();
                }
                break;
            }
        });

        iWorkspace.view_stack().QueryMouseCursor([&](ng::mouse_cursor& cursor)
        {
            if (cursor.is_system_cursor() && cursor.system_cursor() == ng::mouse_system_cursor::Arrow)
            {
                if (!toolboxTree.selection_model().selection().empty())
                    cursor = ng::mouse_system_cursor::Crosshair;
            }
        });

        toolboxTree.presentation_model().dragging_item([&](i_drag_drop_item const&)
        {
            toolboxTree.selection_model().clear_selection();
        });

        // preview (off by default): widgets behave as in a running application; no editing
        aApp.actionPreview.set_checked(preview_mode());
        aApp.actionPreview.checked([&]() { set_preview_mode(true); });
        aApp.actionPreview.unchecked([&]() { set_preview_mode(false); });
        // display ids (off by default): the editor shows element ids rather than their text (never in preview)
        aApp.actionDisplayIds.set_checked(display_ids());
        aApp.actionDisplayIds.checked([&]() { set_display_ids(true); });
        aApp.actionDisplayIds.unchecked([&]() { set_display_ids(false); });
        iSink += preview_mode_changed()([&]()
        {
            toolboxTree.enable_drag_drop_source(!preview_mode());
            objectTree.enable_drag_drop_source(!preview_mode());
            iWorkspace.view_stack().update();
        });

        iWorkspace.view_stack().Keyboard([&](const ng::keyboard_event& aEvent)
        {
            if (aEvent.type() == ng::keyboard_event_type::KeyPressed && aEvent.scan_code() == ScanCode_ESCAPE && iProjectManager.project_active())
                iProjectManager.active_project().root().select(false, true);
        });
    }

    // File menu (new, open, add .nrc file, save, close) and settings; enabling actions and docks for the active project
    void main_window_ex::init_file_actions(main_app& aApp, settings& aSettings, project_manager& aProjectManager)
    {
        auto update_ui = [&]()
        {
            aApp.actionFileClose.enable(aProjectManager.project_active());
            aApp.actionFileAddNrc.enable(aProjectManager.project_active());
            aApp.actionFileSave.enable(aProjectManager.project_active() && aProjectManager.active_project().dirty());
            iLeftDock.show(aProjectManager.project_active());
            iRightDock.show(aProjectManager.project_active());
        };

        update_ui();

        iSink += aProjectManager.ProjectAdded([&, update_ui](i_project& aProject) 
        { 
            iSink += aProject.modified([update_ui]() { update_ui(); });
            update_ui(); 
        });
        iSink += aProjectManager.ProjectRemoved([update_ui](i_project&) { update_ui(); });
        iSink += aProjectManager.ProjectActivated([update_ui](i_project&) { update_ui(); });

        iSink += aApp.actionFileClose.triggered([&]() 
        { 
            if (aProjectManager.project_active())
            {
                remove_caddies(aProjectManager.active_project());
                aProjectManager.close_project(aProjectManager.active_project());
            }
        });

        iSink += aApp.actionFileSave.triggered([&, update_ui]()
        {
            if (!aProjectManager.project_active())
                return;
            auto& project = aProjectManager.active_project();
            std::string path;
            // a project of more than one .nrc file is saved as a project file (.dsproj) listing them
            bool const multipleFiles = project.file_count() > 1u;
            if (project.has_path() && (!multipleFiles || std::filesystem::path{ project.path().to_std_string() }.extension() == ".dsproj"))
                path = project.path().to_std_string();
            else
            {
                auto const extension = multipleFiles ? std::string{ ".dsproj" } : std::string{ ".nrc" };
                auto file = ng::save_file_dialog(mainWindow, ng::file_dialog_spec{ "Save Project", project.name().to_std_string() + extension, { "*" + extension }, "Project Files" });
                if (!file)
                    return;
                path = *file;
                if (std::filesystem::path{ path }.extension().empty())
                    path += extension;
            }
            try
            {
                project.save(ng::string{ path });
            }
            catch (std::exception const& e)
            {
                ng::service<ng::i_surface_manager>().display_error_message("Save Project"_t, ng::string{ e.what() });
            }
            update_ui();
        });

        aApp.action_file_open().triggered([&]()
        {
            auto files = ng::open_file_dialog(mainWindow, ng::file_dialog_spec{ "Open Project", {}, { "*.dsproj", "*.nrc" }, "Project Files" });
            if (files)
            {
                // .nrc files opened together form one project (the first being its primary file); a project file (.dsproj) lists them
                i_project* nrcProject = nullptr;
                for (auto const& file : files.value())
                {
                    std::filesystem::path const filePath{ file };
                    if (filePath.extension() == ".dsproj")
                    {
                        auto& project = aProjectManager.open_project(file);
                        create_caddies(project, iWorkspace.view_stack());
                    }
                    else if (filePath.extension() == ".nrc")
                    {
                        try
                        {
                            if (nrcProject == nullptr)
                                nrcProject = &aProjectManager.open_project(file);
                            else
                                nrcProject->add_file(ng::string{ file });
                        }
                        catch (std::exception const& e)
                        {
                            ng::service<ng::i_surface_manager>().display_error_message("Open Project"_t, ng::string{ e.what() });
                        }
                    }
                    else if (aProjectManager.project_active())
                        aProjectManager.active_project().create_element(aProjectManager.active_project().root(), "file"_s, ng::string{ file });
                    else
                    {
                        ng::sink sink = aProjectManager.project_added([&](i_project& aProject)
                        {
                            aProject.create_element(aProject.root(), "file"_s, ng::string{ file });
                        });
                        aApp.action_file_new().triggered()();
                    }
                }
                // (caddies once all of its files are loaded so resources from any of them are available)
                if (nrcProject != nullptr)
                    create_caddies(*nrcProject, iWorkspace.view_stack());
            }
        });

        // adding .nrc files to the open project (e.g. one with resources it uses)
        aApp.actionFileAddNrc.triggered([&]()
        {
            if (!aProjectManager.project_active())
                return;
            auto files = ng::open_file_dialog(mainWindow, ng::file_dialog_spec{ "Add .nrc File to Project", {}, { "*.nrc" }, "Resource Files" });
            if (!files)
                return;
            auto& project = aProjectManager.active_project();
            for (auto const& file : files.value())
            {
                try
                {
                    project.add_file(ng::string{ file });
                }
                catch (std::exception const& e)
                {
                    ng::service<ng::i_surface_manager>().display_error_message("Add .nrc File to Project"_t, ng::string{ e.what() });
                }
            }
            create_caddies(project, iWorkspace.view_stack()); // (only for elements that don't already have one)
            // elements already shown may use resources from the added files so apply their attributes again
            project.root().visit([](i_element& aElement) { aElement.apply_attributes(show_ids()); });
        });

        aApp.action_file_new().triggered([&]()
        {
            new_project_dialog_ex dialog{ mainWindow };
            if (dialog.exec() == ng::dialog_result::Accepted)
            {
                auto& project = aProjectManager.create_project(dialog.projectName.text(), dialog.projectNamespace.text());
                project.set_dirty();
                project.root();
            }
        });

        aApp.actionSettings.triggered([&]()
        {
            ng::settings_dialog dialog{ mainWindow, aSettings };
            dialog.exec();
        });
    }

    void main_window_ex::close()
    {
        if (iProjectManager.project_active())
        {
            remove_caddies(iProjectManager.active_project());
            iProjectManager.close_project(iProjectManager.active_project());
        }
        main_window::close();
    }

    void main_window_ex::add_action(uuid const& aMenuId, i_action& aAction)
    {
        if (aMenuId == id::MenuBar)
        {
            menuBar.insert_action_at(menuBar.find(menuTools), aAction);
        }
        else if (aMenuId == id::ToolsMenu)
        {
            menuTools.add_action(aAction);
        }
        else
        {
            menuBar.insert_action_at(menuBar.find_sub_menu(aMenuId), aAction);
        }
    }

    void main_window_ex::add_sub_menu(uuid const& aMenuId, i_menu& aSubMenu)
    {
        if (aMenuId == id::MenuBar)
        {
            menuBar.insert_sub_menu_at(menuBar.find(menuTools), aSubMenu);
        }
        else if (aMenuId == id::ToolsMenu)
        {
            menuTools.add_sub_menu(aSubMenu);
        }
        else
        {
            menuBar.insert_sub_menu_at(menuBar.find_sub_menu(aMenuId), aSubMenu);
        }
    }

    bool main_window_ex::can_undo() const
    {
        // todo
        return false;
    }

    bool main_window_ex::can_redo() const
    {
        // todo
        return false;
    }

    bool main_window_ex::can_cut() const
    {
        // todo
        return false;
    }

    bool main_window_ex::can_copy() const
    {
        // todo
        return false;
    }

    bool main_window_ex::can_paste() const
    {
        // todo
        return false;
    }

    bool main_window_ex::can_delete_selected() const
    {
        if (!iProjectManager.project_active() || preview_mode())
            return false;
        bool someSelected = false;
        iProjectManager.active_project().root().visit([&](i_element& aElement)
        {
            if (aElement.is_selected())
                someSelected = true;
        });
        return someSelected;
    }

    bool main_window_ex::can_select_all() const
    {
        if (!iProjectManager.project_active() || preview_mode())
            return false;
        return !iProjectManager.active_project().root().children().empty();
    }

    void main_window_ex::undo(i_clipboard& aClipboard)
    {
        // todo
    }

    void main_window_ex::redo(i_clipboard& aClipboard)
    {
        // todo
    }

    void main_window_ex::cut(i_clipboard& aClipboard)
    {
        // todo
    }

    void main_window_ex::copy(i_clipboard& aClipboard)
    {
        // todo
    }

    void main_window_ex::paste(i_clipboard& aClipboard)
    {
        // todo
    }

    void main_window_ex::delete_selected()
    {
        if (!iProjectManager.project_active())
            return;
        thread_local std::vector<weak_ref_ptr<i_element>> tToDelete;
        iProjectManager.active_project().root().visit([&](i_element& aElement)
        {
            if (aElement.is_selected())
                tToDelete.push_back(aElement);
        });
        for (auto& e : tToDelete)
        {
            if (e.valid())
                iProjectManager.active_project().remove_element(*e);
        }
        tToDelete.clear();
    }

    void main_window_ex::select_all()
    {
        if (!iProjectManager.project_active())
            return;
        iProjectManager.active_project().root().visit([&](i_element& aElement)
        {
            if (aElement.has_layout_item())
                aElement.select(true, false);
        });
    }

    void main_window_ex::update_properties()
    {
        neolib::scoped_flag sf{ iUpdatingProperties };
        iPropertySink.clear();
        iPropertyRows.clear();
        iChangedProperties.clear();
        iPropertyPresentationModel.clear_cell_widgets();
        iPropertyModel.clear();
        if (!iPropertyElement.valid())
            return;
        auto& element = *iPropertyElement;
        // (its .nrc attributes changed elsewhere, e.g. default_size by resizing a window: shown again)
        iPropertySink += element.attributes_changed([this]()
        {
            iPropertiesNeedUpdate = true;
        });
        // the object's property names (an .nrc attribute with an associated object property (e.g. "size_policy" and SizePolicy) isn't shown 
        // as the property is shown instead: editing the property sets the attribute, see set_property_attribute)
        std::set<std::string> propertyNames;
        if (element.has_layout_item())
            for (auto const& entry : std::as_const(element.layout_item().properties()).property_map())
                propertyNames.insert(entry.second()->name().to_std_string());
        auto has_property = [&](std::string const& aAttribute)
        {
            return propertyNames.find(property_name_of(aAttribute)) != propertyNames.end();
        };
        // .nrc attributes (saved to the project file)
        auto attributesNode = iPropertyModel.insert_item(iPropertyModel.send(), property_model_item{}, string{ element.type().to_std_string() + " (.nrc)" });
        std::uint32_t attributeIndex = 0u;
        for (auto const& attribute : element.attributes())
        {
            // skip metadata ('#' prefix), removed (empty) properties and those with an associated object property
            if (!attribute.first().empty() && attribute.first().to_std_string_view()[0] != '#' && !attribute.second().empty() && 
                !has_property(attribute.first().to_std_string()))
            {
                auto row = iPropertyModel.append_item(attributesNode, property_model_item{ attributeIndex }, string{ attribute.first() });
                iPropertyModel.insert_cell_data(row, 1u, string{ attribute.second() });
            }
            ++attributeIndex;
        }
        // the attributes that can be added (those supported not already added) are rows of an "Attributes" node; giving one a value adds it
        // (except those with an associated object property)
        neolib::vector<string> available;
        element.available_attributes(available);
        std::vector<std::string> attributesToShow;
        for (auto const& name : available)
            if (!has_property(name.to_std_string()))
                attributesToShow.push_back(name.to_std_string());
        if (!attributesToShow.empty())
        {
            auto availableNode = iPropertyModel.append_item(attributesNode, property_model_item{}, string{ "Attributes" });
            for (auto const& name : attributesToShow)
                iPropertyModel.append_item(availableNode, property_model_item{ property_presentation_model::new_property_row }, string{ name });
        }
        // object properties grouped by class: derived classes first, base classes last
        if (!element.has_layout_item())
            return;
        auto& owner = element.layout_item();
        // show a property's new value when it changes (e.g. Size/Position changed with the mouse on the design surface) unless it is being edited
        // (noted here and their rows updated in a batch by iPropertiesUpdater: e.g. resizing a widget with the mouse changes properties at a high rate)
        iPropertySink += static_cast<ng::i_object&>(owner).property_changed([this](i_property const& aProperty)
        {
            if (iPropertyRows.find(&aProperty) != iPropertyRows.end())
                iChangedProperties.insert(&aProperty);
        });
        // (the properties shown are all the object's: if it is destroyed they are too)
        iPropertySink += static_cast<ng::i_object&>(owner).destroyed([this]()
        {
            iChangedProperties.clear();
            iPropertyRows.clear();
        });
        std::map<std::string, std::vector<i_property*>> classes;
        for (auto const& entry : std::as_const(owner.properties()).property_map())
            classes[property_class_name(*entry.second())].push_back(entry.second());
        auto unqualified = [](std::string const& aName, std::string const& aSeparator)
        {
            auto const pos = aName.rfind(aSeparator);
            return pos == std::string::npos ? aName : aName.substr(pos + aSeparator.size());
        };
        std::vector<std::string> hierarchy; // class_names(): most derived first, e.g. "push_button:button:widget:layout_item:..."
        {
            std::istringstream names{ class_names(owner) };
            std::string name;
            while (std::getline(names, name, ':'))
                if (!name.empty())
                    hierarchy.push_back(unqualified(name, "--"));
        }
        auto rank = [&](std::string const& aClass)
        {
            auto const name = unqualified(aClass, "::");
            auto const existing = std::find(hierarchy.begin(), hierarchy.end(), name);
            return static_cast<std::size_t>(std::distance(hierarchy.begin(), existing));
        };
        std::vector<std::pair<std::size_t, std::string>> orderedClasses;
        for (auto const& c : classes)
            orderedClasses.emplace_back(rank(c.first), c.first);
        std::sort(orderedClasses.begin(), orderedClasses.end());
        // e.g. "neogfx::layout_item" -> "Layout Item"
        // e.g. "BorderColor" -> "Border Color", "DpiAutoScale" -> "Dpi Auto Scale", "RGBValue" -> "RGB Value"
        auto property_display_name = [](std::string const& aName)
        {
            std::string displayName;
            for (std::size_t i = 0u; i < aName.size(); ++i)
            {
                auto const is_upper = [&](std::size_t j) { return j < aName.size() && std::isupper(static_cast<unsigned char>(aName[j])); };
                auto const is_lower = [&](std::size_t j) { return j < aName.size() && std::islower(static_cast<unsigned char>(aName[j])); };
                if (i != 0u && is_upper(i) && (is_lower(i - 1u) || (is_upper(i - 1u) && is_lower(i + 1u))))
                    displayName += ' ';
                displayName += aName[i];
            }
            return displayName;
        };
        auto display_name = [&](std::string const& aName)
        {
            std::string displayName;
            for (auto const& word : neolib::tokens(unqualified(aName, "::"), "_"s))
            {
                if (word.empty())
                    continue;
                if (!displayName.empty())
                    displayName += ' ';
                displayName += static_cast<char>(std::toupper(static_cast<unsigned char>(word[0])));
                displayName += word.substr(1);
            }
            return displayName;
        };
        // (categories in rank order: see property_category_rank)
        for (auto const& c : orderedClasses)
        {
            auto classNode = iPropertyModel.insert_item(iPropertyModel.send(), property_model_item{}, string{ display_name(c.second) });
            std::map<std::pair<std::size_t, std::string>, std::vector<i_property*>> categories;
            for (auto property : classes[c.second])
            {
                categories[{ property_category_rank(property->category()), property_class_name(property->category()) }].push_back(property);
            }
            for (auto& category : categories)
            {
                auto categoryNode = iPropertyModel.append_item(classNode, property_model_item{}, string{ display_name(category.first.second) });
                auto& properties = category.second;
                std::sort(properties.begin(), properties.end(), [](i_property const* lhs, i_property const* rhs) 
                { 
                    return lhs->name().to_std_string_view() < rhs->name().to_std_string_view(); 
                });
                for (auto property : properties)
                {
                    auto row = iPropertyModel.append_item(categoryNode, property_model_item{ property }, string{ property_display_name(property->name().to_std_string()) });
                    iPropertyModel.insert_cell_data(row, 1u, string{ property_cell_text(*property) });
                    // a composite property's components (e.g. a size's width and height) are edited individually
                    auto const components = property_components(*property);
                    for (std::uint32_t component = 0u; component < components.size(); ++component)
                    {
                        auto componentRow = iPropertyModel.append_item(row, property_model_item{ property_component{ property, component } }, string{ components[component] });
                        iPropertyModel.insert_cell_data(componentRow, 1u, string{ property_cell_text(*property, component) });
                    }
                }
            }
        }
        // composite properties' components are initially hidden (expanding the property's row shows them); each property's rows are noted
        for (std::uint32_t propertyRow = 0u; propertyRow < iPropertyModel.rows(); ++propertyRow)
        {
            auto const& item = iPropertyModel.item(item_model_index{ propertyRow, 0u });
            if (std::holds_alternative<i_property*>(item))
                iPropertyRows[std::get<i_property*>(item)].push_back(propertyRow);
            else if (std::holds_alternative<property_component>(item))
                iPropertyRows[std::get<property_component>(item).property].push_back(propertyRow);
            if (std::holds_alternative<i_property*>(item) && !property_components(*std::get<i_property*>(item)).empty() && 
                iPropertyPresentationModel.has_item_model_index(item_model_index{ propertyRow, 0u }))
                iPropertyPresentationModel.collapse(iPropertyPresentationModel.from_item_model_index(item_model_index{ propertyRow, 0u }));
        }
    }

    // the .nrc attribute associated with a property of the element whose properties are shown, if it has one (one the element supports, see 
    // i_element::available_attributes, or has)
    std::optional<std::string> main_window_ex::property_attribute(i_property const& aProperty) const
    {
        if (!iPropertyElement.valid())
            return {};
        auto const name = attribute_name_of(aProperty.name().to_std_string());
        if (name.empty())
            return {};
        auto const& element = *iPropertyElement;
        for (auto const& attribute : element.attributes())
            if (attribute.first().to_std_string() == name)
                return name;
        neolib::vector<string> available;
        element.available_attributes(available);
        for (auto const& attribute : available)
            if (attribute.to_std_string() == name)
                return name;
        return {};
    }

    // the text of a property's row (or of one of its components' rows): its .nrc attribute's value (as entered, e.g. with units) if it has 
    // one, otherwise its value
    std::string main_window_ex::property_cell_text(i_property const& aProperty, std::optional<std::uint32_t> aComponent) const
    {
        if (auto const name = property_attribute(aProperty))
        {
            // (the last of the attribute's entries is the one that applies)
            std::vector<std::string> items;
            for (auto const& attribute : (*iPropertyElement).attributes())
                if (attribute.first().to_std_string() == *name && !attribute.second().empty())
                    items = attribute_items(attribute.second().to_std_string());
            if (!items.empty())
            {
                if (aComponent)
                    return items[*aComponent % items.size()]; // (e.g. "[ 4px 2px ]" padding: left/right 4px, top/bottom 2px)
                std::string result;
                for (auto const& item : items)
                    result += (result.empty() ? "" : ", ") + item;
                return result;
            }
        }
        return aComponent ? property_component_to_string(aProperty, *aComponent) : property_value_to_string(aProperty);
    }

    // an edit of a property (or of one of its components) with an associated .nrc attribute: the text as entered (e.g. with units) becomes the 
    // attribute's value (which is applied to the object and saved); false if it has no associated attribute
    bool main_window_ex::set_property_attribute(i_property const& aProperty, std::optional<std::uint32_t> aComponent, std::string const& aText)
    {
        auto const name = property_attribute(aProperty);
        if (!name)
            return false;
        auto& element = *iPropertyElement;
        bool const strings = property_has_type<ng::string>(aProperty);
        std::vector<std::string> items;
        if (aComponent)
        {
            auto const components = static_cast<std::uint32_t>(property_components(aProperty).size());
            for (std::uint32_t component = 0u; component < components; ++component)
                items.push_back(component == *aComponent ? aText : property_cell_text(aProperty, component));
        }
        else if (strings)
            items.push_back(aText);
        else if (!(aProperty.optional() && (aText.empty() || aText == "(none)"))) // (unset: the attribute is removed)
            for (auto const& token : property_text_tokens(aText, ','))
                items.push_back(token);
        string const value{ items.empty() ? std::string{} : attribute_value(items, strings) };
        // (the last of the attribute's entries is the one that applies; an empty value removes the attribute on save)
        auto existing = element.attributes().end();
        for (auto attribute = element.attributes().begin(); attribute != element.attributes().end(); ++attribute)
            if (attribute->first().to_std_string() == *name)
                existing = attribute;
        if (existing != element.attributes().end())
            existing->second() = value;
        else if (!value.empty())
            element.attributes().push_back(neolib::pair<string, string>{ string{ *name }, value });
        element.apply_attributes(show_ids());
        iProjectManager.active_project().set_dirty();
        return true;
    }

    // show a property's value (or attribute value, see property_cell_text) in its rows
    void main_window_ex::update_property_rows(i_property const& aProperty, std::optional<std::uint32_t> aExceptRow)
    {
        auto const rows = iPropertyRows.find(&aProperty);
        if (rows == iPropertyRows.end())
            return;
        for (auto propertyRow : rows->second)
        {
            if (propertyRow == aExceptRow)
                continue;
            auto const& item = iPropertyModel.item(item_model_index{ propertyRow, 0u });
            auto const text = std::holds_alternative<property_component>(item) ?
                property_cell_text(aProperty, std::get<property_component>(item).index) : property_cell_text(aProperty);
            iPropertyModel.update_cell_data(item_model_index{ propertyRow, 1u }, string{ text });
        }
    }

    void main_window_ex::paint_workspace(ng::i_graphics_context& aGc)
    {
        auto const& scrollArea = iWorkspace.view_stack().scroll_area();
        if (iProjectManager.projects().empty())
        {
            aGc.draw_texture(
                ng::point{ (scrollArea.extents() - iBackgroundTexture1.extents()) / 2.0 },
                iBackgroundTexture1,
                ng::color::White.with_alpha(0.25));
            aGc.draw_texture(
                ng::rect{ ng::point{ scrollArea.bottom_right() - iBackgroundTexture2.extents() / 2.0 }, iBackgroundTexture2.extents() / 2.0 },
                iBackgroundTexture2,
                ng::color::White.with_alpha(0.25));
        }
        else
        {
            if (workspaceGridType.value<workspace_grid>(true) != workspace_grid::None)
            {
                auto const& gridSize = ng::from_dip(ng::basic_size<std::uint32_t>{
                    workspaceGridSize.value<std::uint32_t>(true) / workspaceGridSubdivisions.value<std::uint32_t>(true),
                        workspaceGridSize.value<std::uint32_t>(true) / workspaceGridSubdivisions.value<std::uint32_t>(true)});
                ng::basic_size<std::int32_t> const cells = ng::size{ scrollArea.cx / gridSize.cx, scrollArea.cy / gridSize.cy };
                aGc.set_gradient(workspaceGridColor.value<ng::gradient>(true), scrollArea);
                if (workspaceGridType.value<workspace_grid>(true) == workspace_grid::Lines)
                {
                    for (std::int32_t x = 0; x <= cells.cx; ++x)
                        aGc.draw_line(ng::point{ scrollArea.left() + x * gridSize.cx, scrollArea.top() }, ng::point{ scrollArea.left() + x * gridSize.cx, scrollArea.bottom() }, ng::color::White);
                    for (std::int32_t y = 0; y <= cells.cy; ++y)
                        aGc.draw_line(ng::point{ scrollArea.left(), scrollArea.top() + y * gridSize.cy }, ng::point{ scrollArea.right(), scrollArea.top() + y * gridSize.cy }, ng::color::White);
                }
                else if (workspaceGridType.value<workspace_grid>(true) == workspace_grid::Quads)
                {
                    for (std::int32_t x = 0; x <= cells.cx; ++x)
                        for (std::int32_t y = 0; y <= cells.cy; ++y)
                            if ((x + y) % 2 == 0)
                                aGc.fill_rect(ng::rect{ ng::point{ scrollArea.left() + x * gridSize.cx, scrollArea.top() + y * gridSize.cy }, gridSize }, ng::color::White);
                }
                else // Points
                {
                    for (std::int32_t x = 0; x <= cells.cx; ++x)
                        for (std::int32_t y = 0; y <= cells.cy; ++y)
                            aGc.draw_pixel(ng::point{ scrollArea.left() + x * gridSize.cx, scrollArea.top() + y * gridSize.cy }, ng::color::White);
                }
                aGc.clear_gradient();
                for (std::int32_t x = 0; x <= cells.cx; x += workspaceGridSubdivisions.value<std::uint32_t>(true))
                    aGc.draw_line(ng::point{ scrollArea.left() + x * gridSize.cx, scrollArea.top() }, ng::point{ scrollArea.left() + x * gridSize.cx, scrollArea.bottom() }, service<i_app>().current_style().palette().color(color_role::Void));
                for (std::int32_t y = 0; y <= cells.cy; y += workspaceGridSubdivisions.value<std::uint32_t>(true))
                    aGc.draw_line(ng::point{ scrollArea.left(), scrollArea.top() + y * gridSize.cy }, ng::point{ scrollArea.right(), scrollArea.top() + y * gridSize.cy }, service<i_app>().current_style().palette().color(color_role::Void));
            }            
        }
        if (iProjectManager.project_active())
        {
            for (auto& e : iProjectManager.active_project().root())
            {
                if (e->group() == element_group::Node)
                {
                    auto& node = static_cast<i_node&>(*e);
                    for (auto const& connection : node.connections())
                    {
                        if (&connection->source().get() == &node)
                        {
                            auto const& node0 = connection->source().get();
                            auto const& node1 = connection->destination().get();
                            auto const& pinWidget0 = connection->source().as_widget();
                            auto const& pinWidget1 = connection->destination().as_widget();
                            auto const nodeRect0 = iWorkspace.view_stack().to_client_coordinates(node0.widget().to_window_coordinates(node0.widget().client_rect()));
                            auto const nodeRect1 = iWorkspace.view_stack().to_client_coordinates(node1.widget().to_window_coordinates(node1.widget().client_rect()));
                            auto const pinRect0 = iWorkspace.view_stack().to_client_coordinates(pinWidget0.icon().to_window_coordinates(pinWidget0.icon().client_rect()));
                            auto const pinRect1 = iWorkspace.view_stack().to_client_coordinates(pinWidget1.icon().to_window_coordinates(pinWidget1.icon().client_rect()));
                            auto const placementRect = pinRect0.combined(pinRect1);
                            auto p0 = pinRect0.center();
                            auto p3 = pinRect1.center();
                            auto const bendRadius = 128.0_dip;
                            auto const negativeControlPoint = bendRadius * 2.0 + std::sqrt(placementRect.width());
                            bool const xPositive = (p3.x - p0.x > bendRadius / 4.0);
                            bool const yPositive = (p3.y >= p0.y);
                            auto p1 = point{ xPositive ? p0.mid(p3).x : nodeRect1.left() - negativeControlPoint, placementRect.top() };
                            auto p2 = point{ xPositive ? p0.mid(p3).x : nodeRect0.right() + negativeControlPoint, placementRect.bottom() };
                            if (xPositive)
                            {
                                if (!yPositive)
                                    std::swap(p1.y, p2.y);
                            }
                            else
                            {
                                std::swap(p0, p3);
                                if (!yPositive)
                                    std::swap(p1.y, p2.y);
                                if (std::abs(yPositive ? nodeRect1.top() - nodeRect0.bottom() : nodeRect0.top() - nodeRect1.bottom()) > bendRadius)
                                {
                                    p1.y = p0.mid(p3).y;
                                    p2.y = p0.mid(p3).y;
                                }
                            }
                            aGc.draw_cubic_bezier(p0, p1, p2, p3, pen{ connection->source().color(), 2.0_dip });
                            aGc.draw_cubic_bezier(p0, p1, p2, p3, pen{ color::Black, 4.0_dip });
                            aGc.draw_cubic_bezier(p0, p1, p2, p3, pen{ connection->source().color(), 2.0_dip });
                        }
                    }
                }
            }
        }
    }
}

