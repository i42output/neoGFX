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

namespace neogfx::DesignStudio
{
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
        font{ aSettings.setting("environment.fonts_and_colors.font"_s) },
        subpixelRendering{ aSettings.setting("environment.fonts_and_colors.subpixel"_s) },
        toolbarIconSize{ aSettings.setting("environment.toolbars.icon_size"_s) },
        themeColor{ aSettings.setting("environment.fonts_and_colors.theme"_s) },
        workspaceGridType{ aSettings.setting("environment.workspace.grid_type"_s) },
        workspaceGridSize{ aSettings.setting("environment.workspace.grid_size"_s) },
        workspaceGridSubdivisions{ aSettings.setting("environment.workspace.grid_subdivisions"_s) },
        workspaceGridColor{ aSettings.setting("environment.workspace.grid_color"_s) },
        iLeftDock{ dock_layout(ng::dock_area::Left), ng::dock_area::Left, ng::size{ leftDockWidth.value<double>() }, ng::size{ leftDockWeight.value<double>() } },
        iRightDock{ dock_layout(ng::dock_area::Right), ng::dock_area::Right, ng::size{ rightDockWidth.value<double>() }, ng::size{ rightDockWeight.value<double>() } },
        iToolbox{ ng::make_dockable<ng::tree_view>("Toolbox"_t, ng::dock_area::Left, true, ng::frame_style::NoFrame) },
        iWorkflow{ ng::make_dockable<ng::list_view>("Workflow"_t, ng::dock_area::Left, true, ng::frame_style::NoFrame) },
        iObjects{ ng::make_dockable<ng::table_view>("Object Explorer"_t, ng::dock_area::Right, true, ng::frame_style::NoFrame) },
        iProperties{ ng::make_dockable<ng::table_view>("Properties"_t, ng::dock_area::Right, true, ng::frame_style::NoFrame) },
        iWorkspaceLayout{ client_layout() },
        iWorkspace{ iWorkspaceLayout },
        iBackgroundTexture1{ ng::image{ ":/neogfx/DesignStudio/resources/neoGFX.png" } },
        iBackgroundTexture2{ ng::image{ ":/neogfx/DesignStudio/resources/logo_i42.png" } },
        iToolboxPresentationModel{ aProjectManager },
        iWorkflowPresentationModel{ aProjectManager },
        iObjectPresentationModel{ aProjectManager, iObjectSelectionModel }
    {
        // todo: decompose this ctor body into smaller member initialization functions...

        hide();

        aApp.actionShowStandardToolbar.checked([&]() { standardToolbar.show(); });
        aApp.actionShowStandardToolbar.unchecked([&]() { standardToolbar.hide(); });
        aApp.actionShowStatusBar.checked([&]() { statusBar.show(); });
        aApp.actionShowStatusBar.unchecked([&]() { statusBar.hide(); });

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

        iToolbox.set_weight(ng::size{ 3.0 });
        iWorkflow.set_weight(ng::size{ 1.0 });

        iToolbox.dock(iLeftDock);
        iWorkflow.dock(iLeftDock);
        iObjects.dock(iRightDock);
        iProperties.dock(iRightDock);

        iToolbox.docked_widget<ng::tree_view>().enable_drag_drop();
        iWorkflow.docked_widget<ng::list_view>().enable_drag_drop();

        ng::i_layout& mainLayout = client_layout();
        mainLayout.set_padding(ng::padding{});
        mainLayout.set_spacing(ng::size{});

        auto fontChanged = [&]()
        {
            ng::service<ng::i_app>().current_style().set_font_info(font.value<ng::font_info>(true));
        };
        font.changing(fontChanged);
        font.changed(fontChanged);
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

        iWorkspace.view_stack().object_acceptable([&](const ng::i_drag_drop_object& aObject, ng::drop_operation& aOperation)
        {
            aOperation = ng::drop_operation::Move;
        });
        iWorkspace.view_stack().set_focus_policy(ng::focus_policy::ClickFocus);

        populate_toolbox_model(iToolboxModel, iToolboxPresentationModel);
        auto& toolboxTree = iToolbox.docked_widget<ng::tree_view>();
        toolboxTree.set_minimum_size(ng::size{ 128_dip, 128_dip });
        toolboxTree.set_presentation_model(iToolboxPresentationModel);
        toolboxTree.selection_model().set_mode(ng::item_selection_mode::NoSelection);
        toolboxTree.set_focus_policy(ng::focus_policy::TabFocus);

        populate_workflow_model(iWorkflowModel, iWorkflowPresentationModel);
        iWorkflowPresentationModel.set_item_model(iWorkflowModel);
        iWorkflowPresentationModel.set_column_read_only(0u);
        auto& workflowTree = iWorkflow.docked_widget<ng::list_view>();
        workflowTree.set_minimum_size(ng::size{ 128_dip, 128_dip });
        workflowTree.set_presentation_model(iWorkflowPresentationModel);
        workflowTree.selection_model().set_mode(ng::item_selection_mode::NoSelection);
        workflowTree.set_focus_policy(ng::focus_policy::TabFocus);

        iObjectModel.set_column_name(0u, "Object"_t);
        iObjectModel.set_column_name(1u, "Type"_t);

        iObjectPresentationModel.set_item_model(iObjectModel);
        iObjectPresentationModel.set_column_read_only(1u);
        iObjectPresentationModel.set_alternating_row_color(true);
        auto& objectTree = iObjects.docked_widget<ng::table_view>();
        objectTree.set_minimum_size(ng::size{ 128_dip, 128_dip });
        objectTree.set_selection_model(iObjectSelectionModel);
        iObjectSelectionModel.set_mode(ng::item_selection_mode::ExtendedSelection);
        objectTree.set_presentation_model(iObjectPresentationModel);
        objectTree.column_header().set_expand_last_column(true);
        iObjectSelectionModel.current_index_changed([&](const optional_item_presentation_model_index& aCurrentIndex, const optional_item_presentation_model_index& aPreviousIndex)
        {
            if (aCurrentIndex)
            {
                auto& element = *iObjectModel.item(iObjectPresentationModel.to_item_model_index(*aCurrentIndex));
                element.set_mode(element_mode::Edit);
            }
            else if (aPreviousIndex)
            {
                auto& element = *iObjectModel.item(iObjectPresentationModel.to_item_model_index(*aCurrentIndex));
                element.set_mode(element_mode::None);
            }
        });
        iObjectSelectionModel.selection_changed([&](const ng::item_selection& aCurrentSelection, const ng::item_selection& aPreviousSelection)
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

        thread_local std::optional<point> tMouseSelectorAnchor;
        thread_local std::optional<point> tMouseSelectorMousePos;

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
            if (!iProjectManager.project_active())
                return;
            auto const eventPos = aEvent.position() - iWorkspace.view_stack().origin();
            switch (aEvent.type())
            {
            case mouse_event_type::ButtonClicked:
                if (aEvent.is_left_button())
                {
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
                    iProjectManager.active_project().root().visit([&](i_element& aElement)
                    {
                        if (aElement.has_layout_item() && aElement.layout_item().is_widget())
                        {
                            auto& elementWidget = aElement.layout_item().as_widget();
                            if (rect{ tMouseSelectorAnchor->min(*tMouseSelectorMousePos), tMouseSelectorAnchor->max(*tMouseSelectorMousePos) }.contains(
                                iWorkspace.view_stack().to_client_coordinates(elementWidget.to_window_coordinates(elementWidget.client_rect())).center()))
                                aElement.select(true, false);
                            else
                                aElement.select(false, false);
                        }
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

        iWorkspace.view_stack().Keyboard([&](const ng::keyboard_event& aEvent)
        {
            if (aEvent.type() == ng::keyboard_event_type::KeyPressed && aEvent.scan_code() == ScanCode_ESCAPE && iProjectManager.project_active())
                iProjectManager.active_project().root().select(false, true);
        });

        auto update_ui = [&]()
        {
            aApp.actionFileClose.enable(aProjectManager.project_active());
            aApp.actionFileSave.enable(aProjectManager.project_active() && aProjectManager.active_project().dirty());
            iLeftDock.show(aProjectManager.project_active());
            iRightDock.show(aProjectManager.project_active());
        };

        update_ui();

        iSink += aProjectManager.ProjectAdded([update_ui](i_project&) { update_ui(); });
        iSink += aProjectManager.ProjectRemoved([update_ui](i_project&) { update_ui(); });
        iSink += aProjectManager.ProjectActivated([update_ui](i_project&) { update_ui(); });

        iSink += aApp.actionFileClose.triggered([&]() { if (aProjectManager.project_active()) aProjectManager.close_project(aProjectManager.active_project()); });

        aApp.action_file_open().triggered([&]()
        {
            ng::service<ng::i_window_manager>().save_mouse_cursor();
            ng::service<ng::i_window_manager>().set_mouse_cursor(ng::mouse_system_cursor::Wait);
            auto projectFile = ng::open_file_dialog(mainWindow, ng::file_dialog_spec{ "Open Project", {}, { "*.nrc" }, "Project Files" });
            if (projectFile)
                aProjectManager.open_project((*projectFile)[0]);
            ng::service<ng::i_window_manager>().restore_mouse_cursor(mainWindow);
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

        //        ng::css css{"test.css"};

        activate();
    }

    void main_window_ex::close()
    {
        if (iProjectManager.project_active())
            iProjectManager.close_project(iProjectManager.active_project());
        main_window::close();
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
        if (!iProjectManager.project_active())
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
        if (!iProjectManager.project_active())
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
                aGc.set_gradient(workspaceGridColor.value<ng::gradient>(true), scrollArea);
                auto const& gridSize = ng::from_dip(ng::basic_size<uint32_t>{
                    workspaceGridSize.value<uint32_t>(true) / workspaceGridSubdivisions.value<uint32_t>(true),
                        workspaceGridSize.value<uint32_t>(true) / workspaceGridSubdivisions.value<uint32_t>(true)});
                ng::basic_size<int32_t> const cells = ng::size{ scrollArea.cx / gridSize.cx, scrollArea.cy / gridSize.cy };
                if (workspaceGridType.value<workspace_grid>(true) == workspace_grid::Lines)
                {
                    for (int32_t x = 0; x <= cells.cx; ++x)
                    {
                        aGc.draw_line(ng::point{ scrollArea.left() + x * gridSize.cx, scrollArea.top() }, ng::point{ scrollArea.left() + x * gridSize.cx, scrollArea.bottom() }, ng::color::White);
                    }
                    for (int32_t y = 0; y <= cells.cy; ++y)
                    {
                        aGc.draw_line(ng::point{ scrollArea.left(), scrollArea.top() + y * gridSize.cy }, ng::point{ scrollArea.right(), scrollArea.top() + y * gridSize.cy }, ng::color::White);
                    }
                }
                else if (workspaceGridType.value<workspace_grid>(true) == workspace_grid::Quads)
                {
                    for (int32_t x = 0; x <= cells.cx; ++x)
                        for (int32_t y = 0; y <= cells.cy; ++y)
                            if ((x + y) % 2 == 0)
                                aGc.fill_rect(ng::rect{ ng::point{ scrollArea.left() + x * gridSize.cx, scrollArea.top() + y * gridSize.cy }, gridSize }, ng::color::White);
                }
                else // Points
                {
                    for (int32_t x = 0; x <= cells.cx; ++x)
                        for (int32_t y = 0; y <= cells.cy; ++y)
                            aGc.draw_pixel(ng::point{ scrollArea.left() + x * gridSize.cx, scrollArea.top() + y * gridSize.cy }, ng::color::White);
                }
                aGc.clear_gradient();
                for (int32_t x = 0; x <= cells.cx; x += workspaceGridSubdivisions.value<uint32_t>(true))
                {
                    aGc.draw_line(ng::point{ scrollArea.left() + x * gridSize.cx, scrollArea.top() }, ng::point{ scrollArea.left() + x * gridSize.cx, scrollArea.bottom() }, service<i_app>().current_style().palette().color(color_role::Void));
                }
                for (int32_t y = 0; y <= cells.cy; y += workspaceGridSubdivisions.value<uint32_t>(true))
                {
                    aGc.draw_line(ng::point{ scrollArea.left(), scrollArea.top() + y * gridSize.cy }, ng::point{ scrollArea.right(), scrollArea.top() + y * gridSize.cy }, service<i_app>().current_style().palette().color(color_role::Void));
                }
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
                            auto const& w0 = connection->source().as_widget();
                            auto const& w1 = connection->destination().as_widget();
                            auto const r0 = iWorkspace.view_stack().to_client_coordinates(w0.icon().to_window_coordinates(w0.icon().client_rect()));
                            auto const r1 = iWorkspace.view_stack().to_client_coordinates(w1.icon().to_window_coordinates(w1.icon().client_rect()));
                            auto const placementRect = r0.combined(r1);
                            auto const p0 = r0.center();
                            auto const p3 = r1.center();
                            auto const dxy = (p0 - p3).abs();
                            auto const p1 =
                                dxy.dx >= dxy.dy || true ? 
                                    (p0.x <= p3.x && p0.y <= p3.y) || (p0.x > p3.x && p0.y <= p3.y) ? 
                                        point{ p0.mid(p3).x, placementRect.top() } :
                                        point{ p0.mid(p3).x, placementRect.bottom() } :
                                    (p0.y <= p3.y && p0.x <= p3.x) || (p0.y > p3.y && p0.x <= p3.x) ?
                                        point{ placementRect.right(), p0.mid(p3).y } :
                                        point{ placementRect.left(), p0.mid(p3).y };
                            auto const p2 =
                                dxy.dx >= dxy.dy || true ?
                                    (p0.x <= p3.x && p0.y <= p3.y) || (p0.x > p3.x && p0.y <= p3.y) ?
                                        point{ p0.mid(p3).x, placementRect.bottom() } :
                                        point{ p0.mid(p3).x, placementRect.top() } :
                                    (p0.y <= p3.y && p0.x <= p3.x) || (p0.y > p3.y && p0.x <= p3.x) ?
                                        point{ placementRect.left(), p0.mid(p3).y } :
                                        point{ placementRect.right(), p0.mid(p3).y };
                            aGc.draw_cubic_bezier(p0, p1, p2, p3, pen{ connection->source().color(), 2.0_dip });
                        }
                    }
                }
            }
        }
    }
}

