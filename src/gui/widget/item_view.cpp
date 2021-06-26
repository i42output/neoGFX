// item_view.cpp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2015, 2020 Leigh Johnston.  All Rights Reserved.
  
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

#include <neogfx/neogfx.hpp>
#include <neolib/core/scoped.hpp>
#include <neogfx/app/i_app.hpp>
#include <neogfx/app/i_basic_services.hpp>
#include <neogfx/gfx/graphics_context.hpp>
#include <neogfx/gui/widget/i_skin_manager.hpp>
#include <neogfx/gui/widget/item_view.hpp>
#include <neogfx/gui/widget/line_edit.hpp>
#include <neogfx/gui/widget/spin_box.hpp>

namespace neogfx
{
    item_view::item_view(frame_style aFrameStyle, neogfx::scrollbar_style aScrollbarStyle) :
        base_type{ aScrollbarStyle, aFrameStyle }, iHotTracking{ false }, iIgnoreNextMouseMove{ false }, iBeginningEdit{ false }, iEndingEdit{ false }, iDefaultTransitionDuration{ 0.5 }
    {
        init();
    }

    item_view::item_view(i_widget& aParent, frame_style aFrameStyle, neogfx::scrollbar_style aScrollbarStyle) :
        base_type{ aParent, aScrollbarStyle, aFrameStyle }, iHotTracking{ false }, iIgnoreNextMouseMove{ false }, iBeginningEdit{ false }, iEndingEdit{ false }, iDefaultTransitionDuration{ 0.5 }
    {
        init();
    }

    item_view::item_view(i_layout& aLayout, frame_style aFrameStyle, neogfx::scrollbar_style aScrollbarStyle) :
        base_type{ aLayout, aScrollbarStyle, aFrameStyle }, iHotTracking{ false }, iIgnoreNextMouseMove{ false }, iBeginningEdit{ false }, iEndingEdit{ false }, iDefaultTransitionDuration{ 0.5 }
    {
        init();
    }

    item_view::~item_view()
    {
    }

    bool item_view::has_model() const
    {
        if (iModel)
            return true;
        else
            return false;
    }
    
    const i_item_model& item_view::model() const
    {
        if (has_model())
            return *iModel;
        throw no_model();
    }

    i_item_model& item_view::model()
    {
        if (has_model())
            return *iModel;
        throw no_model();
    }

    void item_view::set_model(i_item_model& aModel)
    {
        set_model(ref_ptr<i_item_model>{ aModel });
    }

    void item_view::set_model(i_ref_ptr<i_item_model> const& aModel)
    {
        if (iModel == aModel)
            return;
        iModelSink.clear();
        iModel = aModel;
        if (has_model())
        {
            iModelSink += model().column_info_changed([this](item_model_index::value_type aColumnIndex) { column_info_changed(aColumnIndex); });
            iModelSink += model().item_added([this](const item_model_index& aItemIndex) { item_added(aItemIndex); });
            iModelSink += model().item_changed([this](const item_model_index& aItemIndex) { item_changed(aItemIndex); });
            iModelSink += model().item_removed([this](const item_model_index& aItemIndex) { item_removed(aItemIndex); });
            iModelSink += neolib::destroying(model(), [this]() { iModel = nullptr; });
            if (has_presentation_model())
                presentation_model().set_item_model(*aModel);
        }
        model_changed();
        update_scrollbar_visibility();
        update();
    }

    bool item_view::has_presentation_model() const
    {
        if (iPresentationModel)
            return true;
        else
            return false;
    }

    const i_item_presentation_model& item_view::presentation_model() const
    {
        if (has_presentation_model())
            return *iPresentationModel;
        throw no_presentation_model();
    }

    i_item_presentation_model& item_view::presentation_model()
    {
        if (has_presentation_model())
            return *iPresentationModel;
        throw no_presentation_model();
    }

    void item_view::set_presentation_model(i_item_presentation_model& aPresentationModel)
    {
        set_presentation_model(ref_ptr<i_item_presentation_model>{ aPresentationModel });
    }

    void item_view::set_presentation_model(i_ref_ptr<i_item_presentation_model> const& aPresentationModel)
    {
        if (iPresentationModel == aPresentationModel)
            return;
        iPresentationModelSink.clear();
        iPresentationModel = aPresentationModel;
        if (has_presentation_model())
        {
            if (presentation_model().has_item_model())
                set_model(presentation_model().item_model());
            else if (has_model())
                presentation_model().set_item_model(model());
        }
        if (has_presentation_model() && has_selection_model())
            selection_model().set_presentation_model(*aPresentationModel);
        if (has_presentation_model())
        {
            iPresentationModelSink += presentation_model().item_model_changed([this](const i_item_model& aItemModel) { item_model_changed(aItemModel); });
            iPresentationModelSink += presentation_model().item_added([this](item_presentation_model_index const& aItemIndex) { item_added(aItemIndex); });
            iPresentationModelSink += presentation_model().item_changed([this](item_presentation_model_index const& aItemIndex) { item_changed(aItemIndex); });
            iPresentationModelSink += presentation_model().item_removed([this](item_presentation_model_index const& aItemIndex) { item_removed(aItemIndex); });
            iPresentationModelSink += presentation_model().item_expanded([this](item_presentation_model_index const& aItemIndex) { invalidate_item(aItemIndex); });
            iPresentationModelSink += presentation_model().item_collapsed([this](item_presentation_model_index const& aItemIndex) { invalidate_item(aItemIndex); });
            iPresentationModelSink += presentation_model().item_toggled([this](item_presentation_model_index const& aItemIndex) { update(cell_rect(aItemIndex, cell_part::Background)); });
            iPresentationModelSink += presentation_model().items_sorting([this]() { items_sorting(); });
            iPresentationModelSink += presentation_model().items_sorted([this]() { items_sorted(); });
            iPresentationModelSink += presentation_model().items_filtering([this]() { items_filtering(); });
            iPresentationModelSink += presentation_model().items_filtered([this]() { items_filtered(); });
        }
        presentation_model_changed();
        update();
    }

    bool item_view::has_selection_model() const
    {
        if (iSelectionModel)
            return true;
        else
            return false;
    }

    const i_item_selection_model& item_view::selection_model() const
    {
        if (has_selection_model())
            return *iSelectionModel;
        throw no_selection_model();
    }

    i_item_selection_model& item_view::selection_model()
    {
        if (has_selection_model())
            return *iSelectionModel;
        throw no_selection_model();
    }

    void item_view::set_selection_model(i_item_selection_model& aSelectionModel)
    {
        set_selection_model(ref_ptr<i_item_selection_model>{ref_ptr<i_item_selection_model>{}, &aSelectionModel});
    }

    void item_view::set_selection_model(i_ref_ptr<i_item_selection_model> const& aSelectionModel)
    {
        if (iSelectionModel == aSelectionModel)
            return;
        iSelectionModelSink.clear();
        iSelectionModel = aSelectionModel;
        if (has_selection_model())
        {
            if (has_presentation_model())
                selection_model().set_presentation_model(presentation_model());
            iSelectionModelSink += selection_model().presentation_model_added([this](i_item_presentation_model& aNewModel) { presentation_model_added(aNewModel); });
            iSelectionModelSink += selection_model().presentation_model_changed([this](i_item_presentation_model& aNewModel, i_item_presentation_model& aOldModel) { presentation_model_changed(aNewModel, aOldModel); });
            iSelectionModelSink += selection_model().presentation_model_removed([this](i_item_presentation_model& aOldModel) { presentation_model_removed(aOldModel); });
            iSelectionModelSink += selection_model().mode_changed([this](item_selection_mode aNewMode) { mode_changed(aNewMode); });
            iSelectionModelSink += selection_model().current_index_changed([this](const optional_item_presentation_model_index& aCurrentIndex, const optional_item_presentation_model_index& aPreviousIndex) { current_index_changed(aCurrentIndex, aPreviousIndex); });
            iSelectionModelSink += selection_model().selection_changed([this](const item_selection& aCurrentSelection, const item_selection& aPreviousSelection) { selection_changed(aCurrentSelection, aPreviousSelection); });
            iSelectionModelSink += neolib::destroyed(selection_model(), [this]() { iSelectionModel = nullptr; });
        }
        selection_model_changed();
        update_scrollbar_visibility();
        update();
    }

    const optional_easing& item_view::default_transition() const
    {
        return iDefaultTransition;
    }

    double item_view::default_transition_duration() const
    {
        return iDefaultTransitionDuration;
    }

    void item_view::set_default_transition(const optional_easing& aTransition, double aTransitionDuration)
    {
        iDefaultTransition = aTransition;
        iDefaultTransitionDuration = aTransitionDuration;
    }

    std::pair<item_presentation_model_index::value_type, coordinate> item_view::first_visible_item(i_graphics_context& aGc) const
    {
        return presentation_model().item_at(vertical_scrollbar().position(), aGc);
    }

    std::pair<item_presentation_model_index::value_type, coordinate> item_view::last_visible_item(i_graphics_context& aGc) const
    {
        return presentation_model().item_at(vertical_scrollbar().position() + item_display_rect().height(), aGc);
    }

    void item_view::layout_items_completed()
    {
        base_type::layout_items_completed();
    }

    widget_part item_view::hit_test(const point& aPosition) const
    {
        if (item_display_rect().contains(aPosition))
            return widget_part{ *this, widget_part::Client };
        else
        {
            auto result = base_type::hit_test(aPosition);
            if (result.part != widget_part::Client)
                return result;
            else
                return widget_part{ *this, widget_part::NonClient };
        }
    }

    size_policy item_view::size_policy() const
    {
        if (has_size_policy())
            return base_type::size_policy();
        else if (has_fixed_size())
            return size_constraint::Fixed;
        else
            return size_constraint::Expanding;
    }

    void item_view::paint(i_graphics_context& aGc) const
    {
        base_type::paint(aGc);
        auto first = first_visible_item(aGc);
        bool finished = false;
        rect clipRect = default_clip_rect().intersection(item_display_rect());
        for (item_presentation_model_index::value_type row = first.first; row < presentation_model().rows() && !finished; ++row)
        {
            finished = true;
            for (uint32_t col = 0u; col < presentation_model().columns(); ++col)
            {
                auto const itemIndex = item_presentation_model_index{ row, col };
                bool const currentCell = selection_model().has_current_index() && selection_model().current_index() == itemIndex;
                rect cellRect = cell_rect(itemIndex, aGc);
                if (cellRect.y > clipRect.bottom())
                    continue;
                finished = false;
                if (cellRect.bottom() < clipRect.y)
                    continue;
                optional_color cellBackgroundColor = presentation_model().cell_color(itemIndex, color_role::Background);
                if (!cellBackgroundColor)
                    cellBackgroundColor = selection_model().is_selected(itemIndex) ? 
                        service<i_app>().current_style().palette().color(color_role::Selection).to_hsv().with_saturation(0.2).to_rgb<color>().with_alpha(has_focus() ? 1.0 : 0.5) : 
                        service<i_app>().current_style().palette().color(presentation_model().alternating_row_color() ? row % 2 == 0 ? color_role::Base : color_role::AlternateBase : color_role::Base);
                optional_color textColor = presentation_model().cell_color(itemIndex, color_role::Text);
                if (!textColor)
                    textColor = service<i_app>().current_style().palette().color(selection_model().is_selected(itemIndex) ? color_role::SelectedText : color_role::Text);
                rect cellBackgroundRect = cell_rect(itemIndex, aGc, cell_part::Background);
                {
                    scoped_scissor scissor(aGc, clipRect.intersection(cellBackgroundRect));
                    if (selection_model().is_selected(itemIndex) && (!currentCell || !editing()))
                        aGc.fill_rect(cellBackgroundRect,
                            cellBackgroundColor->with_combined_alpha(selection_model().has_current_index() && selection_model().current_index().row() == itemIndex.row() ?
                                1.0 : 0.5));
                    else
                        aGc.fill_rect(cellBackgroundRect, *cellBackgroundColor);
                }
                if (model().is_tree() && col == 0u && model().has_children(presentation_model().to_item_model_index(itemIndex)))
                {
                    auto const expanderRect = cell_rect(itemIndex, aGc, cell_part::TreeExpander);
                    scoped_scissor scissor(aGc, clipRect.intersection(expanderRect));
                    thread_local struct : i_skinnable_item
                    {
                        const item_view* widget;
                        rect treeExpanderRect;
                        bool is_widget() const override
                        {
                            return true;
                        }
                        const i_widget& as_widget() const override
                        {
                            return *widget;
                        }
                        rect element_rect(skin_element aElement) const override
                        {
                            switch (aElement)
                            {
                            case skin_element::ClickableArea:
                            case skin_element::TreeExpander:
                                return treeExpanderRect;
                            default:
                                return widget->element_rect(aElement);
                            }
                        }
                    } skinnableItem = {};
                    skinnableItem.widget = this;
                    skinnableItem.treeExpanderRect = expanderRect;
                    service<i_skin_manager>().active_skin().draw_tree_expander(aGc, skinnableItem, presentation_model().cell_meta(itemIndex).expanded);
                }
                {
                    scoped_scissor scissor(aGc, clipRect.intersection(cellRect));
                    if (presentation_model().cell_checkable(itemIndex))
                    {
                        thread_local struct : i_skinnable_item
                        {
                            const item_view* widget;
                            rect checkBoxRect;

                            bool is_widget() const override
                            {
                                return true;
                            }

                            const i_widget& as_widget() const override
                            {
                                return *widget;
                            }

                            rect element_rect(skin_element aElement) const override
                            {
                                switch (aElement)
                                {
                                case skin_element::ClickableArea:
                                case skin_element::CheckBox:
                                    return checkBoxRect;
                                default:
                                    return widget->element_rect(aElement);
                                }
                            }
                        } skinnableItem = {};
                        skinnableItem.widget = this;
                        skinnableItem.checkBoxRect = cell_rect(itemIndex, aGc, cell_part::CheckBox);
                        service<i_skin_manager>().active_skin().draw_check_box(aGc, skinnableItem, presentation_model().cell_meta(itemIndex).checked);
                    }
                    auto const& cellImage = presentation_model().cell_image(itemIndex);
                    if (cellImage != std::nullopt)
                        aGc.draw_texture(cell_rect(itemIndex, aGc, cell_part::Image), *cellImage);
                    auto cellTextRect = cell_rect(itemIndex, aGc, cell_part::Text);
                    auto const& glyphText = presentation_model().cell_glyph_text(itemIndex, aGc);
                    aGc.draw_glyph_text(cellTextRect.top_left(), glyphText, *textColor);
                }
                if (currentCell)
                {
                    scoped_scissor scissor(aGc, clipRect.intersection(cellBackgroundRect));
                    if (selection_model().current_index() != editing() && has_focus())
                        aGc.draw_focus_rect(cellBackgroundRect);
                }
            }
        }
    }

    color item_view::palette_color(color_role aColorRole) const
    {
        if (has_palette_color(aColorRole))
            return base_type::palette_color(aColorRole);
        if (aColorRole == color_role::Background)
            return palette_color(color_role::Base);
        return base_type::palette_color(aColorRole);
    }

    void item_view::capture_released()
    {
        iMouseTracker = std::nullopt;
    }

    neogfx::focus_policy item_view::focus_policy() const
    {
        if (base_type::has_focus_policy())
            return base_type::focus_policy();
        auto result = base_type::focus_policy();
        if (editing() != std::nullopt)
            result |= (focus_policy::ConsumeReturnKey | focus_policy::ConsumeTabKey);
        return result;
    }

    void item_view::focus_gained(focus_reason aFocusReason)
    {
        base_type::focus_gained(aFocusReason);
        if (aFocusReason != focus_reason::ClickClient && aFocusReason != focus_reason::Other)
        {
            if (model().rows() > 0 && !selection_model().has_current_index())
                select(item_presentation_model_index{ 0, 0 });
        }
    }

    void item_view::mouse_button_pressed(mouse_button aButton, const point& aPosition, key_modifiers_e aKeyModifiers)
    {
        base_type::mouse_button_pressed(aButton, aPosition, aKeyModifiers);
        if (aButton == mouse_button::Left)
        {
            if (!drag_drop_active() && capturing() && item_display_rect(true).contains(aPosition))
            {
                auto item = item_at(aPosition);
                if (item != std::nullopt)
                {
                    if (item->column() == 0 && model().is_tree() && cell_rect(*item, cell_part::TreeExpander).contains(aPosition))
                    {
                        end_edit(true);
                        if (presentation_model().toggle_expanded(*item))
                            return;
                    }
                    if (presentation_model().cell_checkable(*item) && cell_rect(*item, cell_part::CheckBox).contains(aPosition))
                        iClickedCheckBox = item;
                    bool const itemWasCurrent = (selection_model().has_current_index() && selection_model().current_index() == *item);
                    select(*item, aKeyModifiers);
                    bool const itemIsCurrent = (selection_model().has_current_index() && selection_model().current_index() == *item);
                    if (itemWasCurrent && itemIsCurrent)
                        iClickedItem = item;
                    if (aKeyModifiers == KeyModifier_NONE && !iClickedCheckBox && itemIsCurrent &&
                        (presentation_model().cell_editable_when_focused(*item)))
                        edit(*item);
                }
                if (capturing())
                {
                    if (!iClickedCheckBox)
                    {
                        if (!drag_drop_enabled())
                        {
                            iMouseTracker.emplace(service<i_async_task>(), [this, aKeyModifiers](neolib::callback_timer& aTimer)
                            {
                                aTimer.again();
                                auto const pos = mouse_position();
                                auto const item = item_at(pos);
                                if (item != std::nullopt)
                                {
                                    if ((to_selection_operation(aKeyModifiers) & item_selection_operation::Toggle) == item_selection_operation::Toggle)
                                        select(*item, item_selection_operation::None);
                                    else
                                        select(*item);
                                }
                            }, std::chrono::milliseconds{ 20 });
                        }
                    }
                    else
                        update(cell_rect(*iClickedCheckBox, cell_part::Background));
                }
            }
            else if (capturing() && client_rect().contains(aPosition))
                release_capture();
        }
        else if (aButton == mouse_button::Right)
        {
            auto item = item_at(aPosition);
            if (item != std::nullopt)
            {
                if (!selection_model().is_selected(*item))
                    select(*item, aKeyModifiers);
                CellContextMenu.trigger(*item);
            }
        }
    }

    void item_view::mouse_button_double_clicked(mouse_button aButton, const point& aPosition, key_modifiers_e aKeyModifiers)
    {
        base_type::mouse_button_double_clicked(aButton, aPosition, aKeyModifiers);
        if (aButton == mouse_button::Left && item_display_rect(true).contains(aPosition))
        {
            auto item = item_at(aPosition);
            if (item != std::nullopt)
            {
                if (item->column() == 0 && model().is_tree() && cell_rect(*item, cell_part::TreeExpander).contains(aPosition))
                {
                    if (presentation_model().toggle_expanded(*item))
                        return;
                }
                bool actioned = false;
                if (presentation_model().cell_checkable(*item) && cell_rect(*item, cell_part::CheckBox).contains(aPosition))
                {
                    iClickedCheckBox = item;
                    actioned = true;
                }
                bool const itemWasCurrent = (selection_model().has_current_index() && selection_model().current_index() == *item);
                if ((to_selection_operation(aKeyModifiers) & item_selection_operation::Toggle) == item_selection_operation::Toggle)
                    select(*item, item_selection_operation::None);
                else
                    select(*item);
                bool const itemIsCurrent = (selection_model().has_current_index() && selection_model().current_index() == *item);
                if (aKeyModifiers == KeyModifier_NONE && !iClickedCheckBox && itemIsCurrent &&
                    (presentation_model().cell_editable_when_focused(*item) ||
                        (itemWasCurrent && presentation_model().cell_editable_on_input_event(*item))))
                {
                    edit(*item);
                    actioned = true;
                }
                if (model().is_tree() && !actioned)
                    presentation_model().toggle_expanded(*item);
            }
        }
    }

    void item_view::mouse_button_released(mouse_button aButton, const point& aPosition)
    {
        base_type::mouse_button_released(aButton, aPosition);
        if (iClickedItem != std::nullopt)
        {
            auto item = item_at(aPosition);
            bool doCheck = (item == iClickedItem && cell_rect(*item, cell_part::Text).contains(aPosition));
            iClickedItem = std::nullopt;
            if (doCheck && (presentation_model().cell_editable_on_input_event(*item)))
                edit(*item);
        }
        if (iClickedCheckBox != std::nullopt)
        {
            auto item = item_at(aPosition);
            bool doCheck = (item == iClickedCheckBox && cell_rect(*item, cell_part::CheckBox).contains(aPosition));
            if (!doCheck)
                update(cell_rect(*iClickedCheckBox, cell_part::Background));
            iClickedCheckBox = std::nullopt;
            if (doCheck)
                presentation_model().toggle_check(*item);
        }
    }

    void item_view::mouse_moved(const point& aPosition, key_modifiers_e aKeyModifiers)
    {
        base_type::mouse_moved(aPosition, aKeyModifiers);
        if (!iIgnoreNextMouseMove)
        {
            if (!drag_drop_enabled() && (iMouseTracker || hot_tracking()) && client_rect().contains(aPosition))
            {
                auto item = item_at(aPosition);
                if (item != std::nullopt)
                {
                    if ((to_selection_operation(aKeyModifiers) & item_selection_operation::Toggle) == item_selection_operation::Toggle)
                        select(*item, item_selection_operation::None);
                    else
                        select(*item);
                }
            }
            update_hover(aPosition);
        }
        else
            iIgnoreNextMouseMove = false;
    }

    void item_view::mouse_entered(const point& aPosition)
    {
        base_type::mouse_entered(aPosition);
        update_hover(aPosition);
    }

    void item_view::mouse_left()
    {
        base_type::mouse_left();
        update_hover({});
    }

    bool item_view::key_pressed(scan_code_e aScanCode, key_code_e aKeyCode, key_modifiers_e aKeyModifiers)
    {
        bool handled = true;
        if (selection_model().has_current_index())
        {
            item_presentation_model_index currentIndex = selection_model().current_index();
            item_presentation_model_index newIndex = currentIndex;
            switch (aScanCode)
            {
            case ScanCode_RETURN:
            case ScanCode_KP_ENTER:
                if (editing() != std::nullopt)
                {
                    end_edit(true);
                    return true;
                }
                break;
            case ScanCode_TAB:
                if (editing() != std::nullopt)
                {
                    end_edit(true);
                    item_presentation_model_index originalIndex = selection_model().current_index();
                    if ((aKeyModifiers & KeyModifier_SHIFT) == KeyModifier_NONE)
                        select(selection_model().relative_to_current_index(index_location::NextCell, true, true));
                    else
                        select(selection_model().relative_to_current_index(index_location::PreviousCell, true, true));
                    edit(selection_model().current_index());
                    if (editing() != std::nullopt && editor_has_text_edit())
                    {
                        editor_text_edit().cursor().set_anchor(0);
                        editor_text_edit().cursor().set_position(editor_text_edit().text().size(), false);
                    }
                    return true;
                }
                break;
            case ScanCode_F2:
                if (editing() == std::nullopt && selection_model().has_current_index())
                    edit(selection_model().current_index());
                break;
            case ScanCode_LEFT:
                newIndex = selection_model().relative_to_current_index(index_location::CellToLeft);
                break;
            case ScanCode_RIGHT:
                newIndex = selection_model().relative_to_current_index(index_location::CellToRight);
                break;
            case ScanCode_UP:
                newIndex = selection_model().relative_to_current_index(index_location::RowAbove);
                break;
            case ScanCode_DOWN:
                newIndex = selection_model().relative_to_current_index(index_location::RowBelow);
                break;
            case ScanCode_PAGEUP:
                {
                    graphics_context gc{ *this, graphics_context::type::Unattached };
                    newIndex.set_row(
                        iPresentationModel->item_at(
                            iPresentationModel->item_position(currentIndex, gc) - 
                            item_display_rect().height() + 
                            iPresentationModel->item_height(currentIndex, gc), gc).first);
                }
                break;
            case ScanCode_PAGEDOWN:
                {
                    graphics_context gc{ *this, graphics_context::type::Unattached };
                    newIndex.set_row(
                        iPresentationModel->item_at(
                            iPresentationModel->item_position(currentIndex, gc) + 
                            item_display_rect().height(), gc).first);
                }
                break;
            case ScanCode_HOME:
                if ((aKeyModifiers & KeyModifier_CTRL) == 0)
                    newIndex = selection_model().relative_to_current_index(index_location::StartOfCurrentRow);
                else
                    newIndex = selection_model().relative_to_current_index(index_location::FirstCell);
                break;
            case ScanCode_END:
                if ((aKeyModifiers & KeyModifier_CTRL) == 0)
                    newIndex = selection_model().relative_to_current_index(index_location::EndOfCurrentRow);
                else
                    newIndex = selection_model().relative_to_current_index(index_location::LastCell);
                break;
            default:
                handled = base_type::key_pressed(aScanCode, aKeyCode, aKeyModifiers);
                break;
            }
            if (aScanCode == ScanCode_SPACE)
                select(newIndex, aKeyModifiers);
            else if (newIndex != currentIndex)
                select(newIndex, item_selection_operation::None);
        }
        else
        {
            switch (aScanCode)
            {
            case ScanCode_LEFT:
            case ScanCode_RIGHT:
            case ScanCode_UP:
            case ScanCode_DOWN:
            case ScanCode_PAGEUP:
            case ScanCode_PAGEDOWN:
            case ScanCode_HOME:
            case ScanCode_END:
                if (presentation_model().rows() > 0 && has_focus())
                    select(item_presentation_model_index{ 0, 0 });
                else
                    handled = false;
                break;
            default:
                handled = base_type::key_pressed(aScanCode, aKeyCode, aKeyModifiers);
                break;
            }
        }
        return handled;
    }

    bool item_view::text_input(i_string const& aText)
    {
        bool handled = base_type::text_input(aText);
        if (editing() == std::nullopt && selection_model().has_current_index() && aText[0] != '\r' && aText[0] != '\n' && aText[0] != '\t')
        {
            edit(selection_model().current_index());
            if (editing())
            {
                if (editor_has_text_edit())
                {
                    editor_text_edit().set_text(aText);
                    handled = true;
                }
            }
        }
        return handled;
    }

    scrolling_disposition item_view::scrolling_disposition() const
    {
        return neogfx::scrolling_disposition::ScrollChildWidgetVertically | neogfx::scrolling_disposition::ScrollChildWidgetHorizontally;
    }

    void item_view::update_scrollbar_visibility()
    {
        bool wasVisible = has_presentation_model() && has_selection_model() &&
            selection_model().has_current_index() && is_visible(selection_model().current_index());
        base_type::update_scrollbar_visibility();
        if (wasVisible)
            make_visible(selection_model().current_index());
    }

    void item_view::update_scrollbar_visibility(usv_stage_e aStage)
    {
        scoped_units su{ *this, units::Pixels };
        switch (aStage)
        {
        case UsvStageInit:
            {
                iOldPositionForScrollbarVisibility = size{ horizontal_scrollbar().position(), vertical_scrollbar().position() };
                vertical_scrollbar().hide();
                horizontal_scrollbar().hide();
            }
            break;
        case UsvStageCheckVertical1:
        case UsvStageCheckVertical2:
            vertical_scrollbar().set_maximum(units_converter(*this).to_device_units(total_item_area(*this)).cy);
            vertical_scrollbar().set_step(font().height() + (has_presentation_model() ? presentation_model().cell_padding(*this).size().cy + presentation_model().cell_spacing(*this).cy : 0.0));
            vertical_scrollbar().set_page(std::max(units_converter(*this).to_device_units(item_display_rect()).cy, 0.0));
            vertical_scrollbar().set_position(iOldPositionForScrollbarVisibility.cy);
            if (vertical_scrollbar().page() > 0 && vertical_scrollbar().maximum() - vertical_scrollbar().page() > 0.0)
                vertical_scrollbar().show();
            else
                vertical_scrollbar().hide();
            break;
        case UsvStageCheckHorizontal:
            horizontal_scrollbar().set_maximum(units_converter(*this).to_device_units(total_item_area(*this)).cx);
            horizontal_scrollbar().set_step(font().height() + (has_presentation_model() ? presentation_model().cell_padding(*this).size().cy + presentation_model().cell_spacing(*this).cy : 0.0));
            horizontal_scrollbar().set_page(std::max(units_converter(*this).to_device_units(item_display_rect()).cx, 0.0));
            horizontal_scrollbar().set_position(iOldPositionForScrollbarVisibility.cx);
            if (horizontal_scrollbar().page() > 0 && horizontal_scrollbar().maximum() - horizontal_scrollbar().page() > 0.0)
                horizontal_scrollbar().show();
            else
                horizontal_scrollbar().hide();
            break;
        case UsvStageDone:
            layout_items();
            break;
        default:
            break;
        }
    }

    void item_view::column_info_changed(item_model_index::value_type)
    {
        update_scrollbar_visibility();
        update();
        if (editing() != std::nullopt && !presentation_model().cell_editable(*editing()))
            end_edit(false);
    }

    void item_view::item_added(const item_model_index&)
    {
    }

    void item_view::item_changed(const item_model_index&)
    {
    }

    void item_view::item_removed(const item_model_index&)
    {
    }

    void item_view::item_model_changed(const i_item_model&)
    {
        update_scrollbar_visibility();
        update();
    }

    void item_view::item_added(item_presentation_model_index const& aItemIndex)
    {
        invalidate_item(aItemIndex);
    }

    void item_view::item_changed(item_presentation_model_index const& aItemIndex)
    {
        invalidate_item(aItemIndex);
    }

    void item_view::item_removed(item_presentation_model_index const& aItemIndex)
    {
        invalidate_item(aItemIndex);
    }

    void item_view::items_sorting()
    {
        if (selection_model().has_current_index())
            iSavedModelIndex = presentation_model().to_item_model_index(selection_model().current_index());
        end_edit(true);
    }

    void item_view::items_sorted()
    {
        if (iSavedModelIndex != std::nullopt && presentation_model().has_item_model_index(*iSavedModelIndex))
            select(presentation_model().from_item_model_index(*iSavedModelIndex));
        iSavedModelIndex = std::nullopt;
        update();
    }

    void item_view::items_filtering()
    {
        end_edit(true);
    }

    void item_view::items_filtered()
    {
        if (presentation_model().rows() != 0)
            select(item_presentation_model_index{});
        update_scrollbar_visibility();
        update();
    }

    void item_view::presentation_model_added(i_item_presentation_model&)
    {
    }

    void item_view::presentation_model_changed(i_item_presentation_model&, i_item_presentation_model&)
    {
    }

    void item_view::presentation_model_removed(i_item_presentation_model&)
    {
    }

    void item_view::mode_changed(item_selection_mode)
    {
    }

    void item_view::current_index_changed(const optional_item_presentation_model_index& aCurrentIndex, const optional_item_presentation_model_index& aPreviousIndex)
    {
        bool needUpdate = true;
        if (aCurrentIndex != std::nullopt)
        {
            if (make_visible(*aCurrentIndex))
                needUpdate = false;
            if (!selection_model().sorting() && !selection_model().filtering())
            {
                if (aPreviousIndex != std::nullopt)
                {
                    if (editing() != std::nullopt && presentation_model().cell_editable_when_focused(*aCurrentIndex) && editing() != aCurrentIndex && iSavedModelIndex == std::nullopt)
                        edit(*aCurrentIndex);
                    else if (editing() != std::nullopt)
                        end_edit(true);
                }
            }
        }
        if (needUpdate)
            update();
    }

    void item_view::selection_changed(const item_selection&, const item_selection&)
    {
        update();
    }

    bool item_view::hot_tracking() const
    {
        return iHotTracking;
    }

    void item_view::enable_hot_tracking()
    {
        if (!iHotTracking)
        {
            iHotTracking = true;
            if (&widget_for_mouse_event(mouse_position()) == this)
                iIgnoreNextMouseMove = true;
        }
    }

    void item_view::disable_hot_tracking()
    {
        iHotTracking = false;
    }

    bool item_view::is_visible(item_presentation_model_index const& aItemIndex) const
    {
        return item_display_rect().contains(cell_rect(aItemIndex, cell_part::Background));
    }

    bool item_view::make_visible(item_presentation_model_index const& aItemIndex, const optional_easing& aTransition, const std::optional<double>& aTransitionDuration)
    {
        bool changed = false;
        auto const& transition = (aTransition == std::nullopt ? default_transition() : aTransition);
        auto const transitionDuration = (aTransitionDuration == std::nullopt ? default_transition_duration() : *aTransitionDuration);
        graphics_context gc{ *this, graphics_context::type::Unattached };
        auto const& cellRect = cell_rect(aItemIndex, gc, cell_part::Background);
        auto const& displayRect = item_display_rect();
        auto const& intersectRect = cellRect.intersection(displayRect);
        if (intersectRect.height() < cellRect.height())
        {
            if (cellRect.top() < displayRect.top())
                changed = vertical_scrollbar().set_position(vertical_scrollbar().position() + (cellRect.top() - displayRect.top()), transition, transitionDuration) || changed;
            else if (cellRect.bottom() > displayRect.bottom() && cellRect.height() <= displayRect.height())
                changed = vertical_scrollbar().set_position(vertical_scrollbar().position() + (cellRect.bottom() - displayRect.bottom()), transition, transitionDuration) || changed;
        }
        if (intersectRect.width() < cellRect.width())
        {
            if (cellRect.left() < displayRect.left())
                changed = horizontal_scrollbar().set_position(horizontal_scrollbar().position() + (cellRect.left() - displayRect.left()), transition, transitionDuration) || changed;
            else if (cellRect.right() > displayRect.right() && cellRect.width() <= displayRect.width())
                changed = horizontal_scrollbar().set_position(horizontal_scrollbar().position() + (cellRect.right() - displayRect.right()), transition, transitionDuration) || changed;
        }
        return changed;
    }

    const optional_item_presentation_model_index& item_view::editing() const
    {
        return iEditing;
    }

    void item_view::edit(item_presentation_model_index const& aItemIndex)
    {
        if (editing() == aItemIndex || beginning_edit() || ending_edit() || !presentation_model().cell_editable(aItemIndex) )
            return;
        neolib::scoped_flag sf{ iBeginningEdit };
        auto modelIndex = presentation_model().to_item_model_index(aItemIndex);
        end_edit(true);
        auto const& cellInfo = model().cell_info(modelIndex);
        if (cellInfo.dataStep == neolib::none)
            iEditor = std::make_shared<item_editor<line_edit>>(*this);
        else
        {
            switch (model().cell_info(presentation_model().to_item_model_index(aItemIndex)).dataType)
            {
            case item_data_type::Int32:
                iEditor = std::make_shared<item_editor<basic_spin_box<int32_t>>>(*this);
                static_cast<basic_spin_box<int32_t>&>(editor()).set_step(static_variant_cast<int32_t>(cellInfo.dataStep));
                static_cast<basic_spin_box<int32_t>&>(editor()).set_minimum(static_variant_cast<int32_t>(cellInfo.dataMin));
                static_cast<basic_spin_box<int32_t>&>(editor()).set_maximum(static_variant_cast<int32_t>(cellInfo.dataMax));
                break;
            case item_data_type::UInt32:
                iEditor = std::make_shared<item_editor<basic_spin_box<uint32_t>>>(*this);
                static_cast<basic_spin_box<uint32_t>&>(editor()).set_step(static_variant_cast<uint32_t>(cellInfo.dataStep));
                static_cast<basic_spin_box<uint32_t>&>(editor()).set_minimum(static_variant_cast<uint32_t>(cellInfo.dataMin));
                static_cast<basic_spin_box<uint32_t>&>(editor()).set_maximum(static_variant_cast<uint32_t>(cellInfo.dataMax));
                break;
            case item_data_type::Int64:
                iEditor = std::make_shared<item_editor<basic_spin_box<int64_t>>>(*this);
                static_cast<basic_spin_box<int64_t>&>(editor()).set_step(static_variant_cast<int64_t>(cellInfo.dataStep));
                static_cast<basic_spin_box<int64_t>&>(editor()).set_minimum(static_variant_cast<int64_t>(cellInfo.dataMin));
                static_cast<basic_spin_box<int64_t>&>(editor()).set_maximum(static_variant_cast<int64_t>(cellInfo.dataMax));
                break;
            case item_data_type::UInt64:
                iEditor = std::make_shared<item_editor<basic_spin_box<uint64_t>>>(*this);
                static_cast<basic_spin_box<uint64_t>&>(editor()).set_step(static_variant_cast<uint64_t>(cellInfo.dataStep));
                static_cast<basic_spin_box<uint64_t>&>(editor()).set_minimum(static_variant_cast<uint64_t>(cellInfo.dataMin));
                static_cast<basic_spin_box<uint64_t>&>(editor()).set_maximum(static_variant_cast<uint64_t>(cellInfo.dataMax));
                break;
            case item_data_type::Float:
                iEditor = std::make_shared<item_editor<basic_spin_box<float>>>(*this);
                static_cast<basic_spin_box<float>&>(editor()).set_step(static_variant_cast<float>(cellInfo.dataStep));
                static_cast<basic_spin_box<float>&>(editor()).set_minimum(static_variant_cast<float>(cellInfo.dataMin));
                static_cast<basic_spin_box<float>&>(editor()).set_maximum(static_variant_cast<float>(cellInfo.dataMax));
                break;
            case item_data_type::Double:
                iEditor = std::make_shared<item_editor<basic_spin_box<double>>>(*this);
                static_cast<basic_spin_box<double>&>(editor()).set_step(static_variant_cast<double>(cellInfo.dataStep));
                static_cast<basic_spin_box<double>&>(editor()).set_minimum(static_variant_cast<double>(cellInfo.dataMin));
                static_cast<basic_spin_box<double>&>(editor()).set_maximum(static_variant_cast<double>(cellInfo.dataMax));
                break;
            default:
                throw unknown_editor_type();
            }
        }
        auto newIndex = presentation_model().from_item_model_index(modelIndex);
        if (!selection_model().has_current_index() || selection_model().current_index() != newIndex)
            select(newIndex);
        iEditing = newIndex;
        if (presentation_model().cell_color(newIndex, color_role::Background) != optional_color{})
            editor().set_background_color(presentation_model().cell_color(newIndex, color_role::Background));
        optional_color textColor = presentation_model().cell_color(newIndex, color_role::Text);
        if (textColor == std::nullopt)
            textColor = has_base_color() ? base_color() : service<i_app>().current_style().palette().color(color_role::Text);
        static_cast<framed_widget<>&>(editor()).set_frame_color(textColor);
        auto editorRect = cell_rect(newIndex, cell_part::Editor);
        editor().move(editorRect.position());
        editor().resize(editorRect.extents());
        if (editor_has_text_edit())
        {
            auto& textEdit = editor_text_edit();
            auto const& textEditRect = to_client_coordinates(textEdit.to_window_coordinates(textEdit.client_rect()));
            bool const childTextEdit = (&textEdit != &editor());
            auto const& textRect = cell_rect(newIndex, cell_part::Text);
            auto const adjust = textRect.position() - textEditRect.position();
            auto const paddingAdjust = neogfx::padding{ adjust.x, adjust.y, adjust.x, 0 };
            textEdit.set_padding(paddingAdjust);
            if (presentation_model().cell_color(newIndex, color_role::Background) != optional_color{})
                textEdit.set_background_color(presentation_model().cell_color(newIndex, color_role::Background));
            optional_color backgroundColor = presentation_model().cell_color(newIndex, color_role::Background);
            auto cellFont = presentation_model().cell_font(newIndex);
            textEdit.set_default_style(text_edit::character_style{ cellFont ? *cellFont : presentation_model().default_font(), *textColor, backgroundColor != std::nullopt ? color_or_gradient{ *backgroundColor } : color_or_gradient{} });
            textEdit.set_text(string{ presentation_model().cell_to_string(newIndex) });
            textEdit.Focus([this, newIndex](neogfx::focus_event fe, neogfx::focus_reason)
            {
                if (fe == neogfx::focus_event::FocusLost && !has_focus() && (!root().has_focused_widget() || !root().focused_widget().is_descendent_of(*this) || !selection_model().has_current_index() || selection_model().current_index() != newIndex))
                    end_edit(true);
            });
            textEdit.Keyboard([this, &textEdit, newIndex](neogfx::keyboard_event const& ke)
            {
                if (ke.type() == neogfx::keyboard_event_type::KeyPressed && ke.scan_code() == ScanCode_ESCAPE && textEdit.cursor().position() == textEdit.cursor().anchor())
                    end_edit(false);
            });
        }
        begin_edit();
    }

    void item_view::begin_edit()
    {
        if (editing() != std::nullopt && !ending_edit() && editor_has_text_edit())
        {
            auto& textEdit = editor_text_edit();
            textEdit.set_focus();
            if (textEdit.client_rect().contains(textEdit.mouse_position()))
                textEdit.set_cursor_position(textEdit.mouse_position(), true, capturing());
            else
                textEdit.cursor().set_anchor(textEdit.cursor().position());
        }
    }

    void item_view::end_edit(bool aCommit)
    {
        if (editing() == std::nullopt)
            return;
        if (aCommit && !presentation_model().cell_editable(*editing()))
            aCommit = false;
        neolib::scoped_flag sf{ iEndingEdit };
        bool hadFocus = (editor().has_focus() || (has_root() && root().has_focused_widget() && root().focused_widget().is_descendent_of(editor())));
        if (aCommit)
        {
            auto modelIndex = presentation_model().to_item_model_index(*editing());
            item_cell_data cellData;
            bool error = false;
            if (editor_has_text_edit())
                cellData = presentation_model().string_to_cell_data(*editing(), editor_text_edit().text(), error);
            iEditing = std::nullopt;
            iEditor = nullptr;
            if (!error)
                model().update_cell_data(modelIndex, cellData);
            else
                service<i_basic_services>().system_beep();
        }
        else
        {
            iEditing = std::nullopt;
            iEditor = nullptr;
        }
        if (hadFocus)
            set_focus();
    }

    bool item_view::beginning_edit() const
    {
        return iBeginningEdit;
    }

    bool item_view::ending_edit() const
    {
        return iEndingEdit;
    }
    
    i_widget& item_view::editor() const
    {
        if (iEditor == nullptr)
            throw no_editor();
        return iEditor->as_widget();
    }

    bool item_view::editor_has_text_edit() const
    {
        return iEditor->has_text_edit();
    }

    text_edit& item_view::editor_text_edit() const
    {
        return iEditor->text_edit();
    }

    void item_view::enable_drag_drop(bool aEnable)
    {
        if (drag_drop_enabled() != aEnable)
        {
            base_type::enable_drag_drop(aEnable);
            if (drag_drop_enabled())
            {
                iDragDropSink = DraggingObject([&](i_drag_drop_object const&)
                {
#ifdef NEOGFX_DEBUG
                    if (debug::item == this)
                        service<debug::logger>() << typeid(*this).name() << ": dragging object." << endl;
#endif
                    presentation_model().dragging_item().trigger(*iDragDropItem);
                });
                iDragDropSink += DraggingCancelled([&](i_drag_drop_object const&)
                {
#ifdef NEOGFX_DEBUG
                    if (debug::item == this)
                        service<debug::logger>() << typeid(*this).name() << ": dragging cancelled." << endl;
#endif
                    presentation_model().dragging_item_cancelled().trigger(*iDragDropItem);
                    iDragDropItem = std::nullopt;
                });
                iDragDropSink += ObjectDropped([&](i_drag_drop_object const&, i_drag_drop_target& aTarget)
                {
#ifdef NEOGFX_DEBUG
                    if (debug::item == this)
                        service<debug::logger>() << typeid(*this).name() << ": object dropped." << endl;
#endif
                    presentation_model().item_dropped().trigger(*iDragDropItem, aTarget);
                    iDragDropItem = std::nullopt;
                });
            }
            else
            {
                iDragDropSink.clear();
            }
        }
    }

    bool item_view::is_drag_drop_object(point const& aPosition) const
    {
        if (!item_display_rect().contains(aPosition))
            return false;
        auto i = item_at(aPosition);
        return i && (presentation_model().cell_flags(*i) & item_cell_flags::Draggable) == item_cell_flags::Draggable;
    }

    i_drag_drop_object const* item_view::drag_drop_object(point const& aPosition)
    {
        if (!item_display_rect().contains(aPosition))
            return nullptr;
        auto i = item_at(aPosition);
        if (i && (presentation_model().cell_flags(*i) & item_cell_flags::Draggable) == item_cell_flags::Draggable)
        {
            iDragDropItem.emplace(*this, presentation_model(), *i);
            return &*iDragDropItem;
        }
        return nullptr;
    }

    void item_view::header_view_updated(header_view&, header_view_update_reason aUpdateReason)
    {
        if (aUpdateReason == header_view_update_reason::FullUpdate)
        {
            bool wasVisible = selection_model().has_current_index() && is_visible(selection_model().current_index());
            if (wasVisible)
                make_visible(selection_model().current_index());
            layout_items();
        }
        else
        {
            update_scrollbar_visibility();
            layout_items();
        }
        if (editing() != std::nullopt)
        {
            auto editorRect = cell_rect(*editing(), cell_part::Text);
            editorRect.inflate(presentation_model().cell_padding(*this));
            editor().move(editorRect.position());
            editor().resize(editorRect.extents());
        }
    }

    rect item_view::row_rect(item_presentation_model_index const& aItemIndex) const
    {
        rect result = item_display_rect();
        result.y = presentation_model().item_position(aItemIndex, *this) - vertical_scrollbar().position();
        result.cy = presentation_model().item_height(aItemIndex, *this);
        return result;
    }

    rect item_view::cell_rect(item_presentation_model_index const& aItemIndex, cell_part aPart) const
    {
        graphics_context gc{ *this, graphics_context::type::Unattached };
        switch(aPart)
        {
        case cell_part::Background:
        case cell_part::Base:
            {
                size const cellSpacing = presentation_model().cell_spacing(*this);
                coordinate y = presentation_model().item_position(aItemIndex, *this);
                dimension const h = presentation_model().item_height(aItemIndex, *this);
                coordinate const indent = presentation_model().indent(aItemIndex, gc);
                coordinate x = indent;
                if (aPart == cell_part::Background && aItemIndex.column() == 0)
                    x = 0.0;
                for (uint32_t col = 0; col < presentation_model().columns(); ++col)
                {
                    bool const firstColumn = (col == 0);
                    bool const lastColumn = (col == presentation_model().columns() - 1);
                    if (!firstColumn)
                        x += cellSpacing.cx;
                    else
                        x += cellSpacing.cx / 2.0;
                    if (col == aItemIndex.column())
                    {
                        x -= horizontal_scrollbar().position();
                        y -= vertical_scrollbar().position();
                        rect result{ point{x, y} + item_display_rect().top_left(), size{ column_width(col), h } };
                        switch (aPart)
                        {
                        case cell_part::Background:
                            result.inflate(size{ cellSpacing.cx / 2.0, 0.0 });
                            break;
                        case cell_part::Base:
                            result.deflate(size{ 0.0, cellSpacing.cy / 2.0 });
                            break;
                        }
                        if (aPart != cell_part::Background)
                            result.cx -= indent;
                        if (lastColumn)
                            result.cx += (item_display_rect().right() - result.right());
                        return result;
                    }
                    x += column_width(col);
                }
                throw i_item_presentation_model::bad_index();
            }
        default:
            return cell_rect(aItemIndex, gc, aPart);
        }
    }
        
    rect item_view::cell_rect(item_presentation_model_index const& aItemIndex, i_graphics_context& aGc, cell_part aPart) const
    {
        switch (aPart)
        {
        case cell_part::Background:
        case cell_part::Base:
            {
                auto cellRect = cell_rect(aItemIndex, aPart);
                return cellRect;
            }
            break;
        case cell_part::CheckBox:
            {
                auto const& cellCheckBoxSize = presentation_model().cell_check_box_size(aItemIndex, aGc);
                if (!cellCheckBoxSize)
                    throw invalid_cell_part();
                auto cellRect = cell_rect(aItemIndex);
                cellRect.indent(point{ presentation_model().cell_padding(*this).left, std::ceil((cellRect.cy - cellCheckBoxSize->cy) / 2.0) });
                cellRect.extents() = *cellCheckBoxSize;
                return cellRect;
            }
            break;
        case cell_part::TreeExpander:
            {
                auto const& cellTreeExpanderSize = presentation_model().cell_tree_expander_size(aItemIndex, aGc);
                if (!cellTreeExpanderSize)
                    throw invalid_cell_part();
                auto cellRect = cell_rect(aItemIndex);
                cellRect.indent(point{ presentation_model().cell_padding(*this).left, std::ceil((cellRect.cy - cellTreeExpanderSize->cy) / 2.0) });
                cellRect.extents() = *cellTreeExpanderSize;
                cellRect.x -= presentation_model().cell_tree_expander_size(aItemIndex, aGc)->cx;
                return cellRect;
            }
            break;
        case cell_part::Image:
            {
                auto const& cellImageSize = presentation_model().cell_image_size(aItemIndex);
                if (!cellImageSize)
                    throw invalid_cell_part();
                auto cellRect = cell_rect(aItemIndex);
                cellRect.indent(point{ presentation_model().cell_padding(*this).left, std::ceil((cellRect.cy - cellImageSize->cy) / 2.0) });
                cellRect.extents() = *cellImageSize;
                auto const& cellCheckBoxSize = presentation_model().cell_check_box_size(aItemIndex, aGc);
                if (cellCheckBoxSize)
                    cellRect.x += (cellCheckBoxSize->cx + presentation_model().cell_spacing(aGc).cx);
                return cellRect;
            }
            break;
        case cell_part::Text:
            {
                auto cellRect = cell_rect(aItemIndex);
                auto const& glyphText = presentation_model().cell_glyph_text(aItemIndex, aGc);
                auto const textHeight = std::max(glyphText.extents().cy,
                    (presentation_model().cell_font(aItemIndex) == std::nullopt ? presentation_model().default_font() : *presentation_model().cell_font(aItemIndex)).height());
                auto const textAdjust = std::floor((cellRect.height() - textHeight) / 2.0);
                cellRect.deflate(presentation_model().cell_padding(*this).left, textAdjust, presentation_model().cell_padding(*this).right, textAdjust);
                auto const& cellCheckBoxSize = presentation_model().cell_check_box_size(aItemIndex, aGc);
                if (cellCheckBoxSize)
                    cellRect.indent(point{ cellCheckBoxSize->cx + presentation_model().cell_spacing(aGc).cx, 0.0 });
                auto const& cellImageSize = presentation_model().cell_image_size(aItemIndex);
                if (cellImageSize)
                    cellRect.indent(point{ cellImageSize->cx + presentation_model().cell_spacing(aGc).cx, 0.0 });
                return cellRect;
            }
            break;
        case cell_part::Editor:
            {
                auto cellRect = cell_rect(aItemIndex);
                auto const& cellCheckBoxSize = presentation_model().cell_check_box_size(aItemIndex, aGc);
                if (cellCheckBoxSize)
                    cellRect.indent(point{ cellCheckBoxSize->cx + presentation_model().cell_spacing(aGc).cx, 0.0 });
                auto const& cellImageSize = presentation_model().cell_image_size(aItemIndex);
                if (cellImageSize)
                    cellRect.indent(point{ cellImageSize->cx + presentation_model().cell_spacing(aGc).cx, 0.0 });
                cellRect.inflate(presentation_model().cell_padding(*this));
                cellRect.inflate(size{ 1.0_dip }); // editor frame width
                bool const lastColumn = (aItemIndex.column() == presentation_model().columns() - 1);
                if (lastColumn)
                {
                    cellRect.cx += presentation_model().cell_spacing(aGc).cx;
                    cellRect.cx = std::min(cellRect.cx, cellRect.cx + (item_display_rect().right() - cellRect.right()));
                }
                return cellRect;
            }
            break;
        }
        return rect{};
    }

    optional_item_presentation_model_index item_view::item_at(const point& aPosition, bool aIncludeEntireRow) const
    {
        if (model().rows() == 0 || !item_display_rect(true).contains(aPosition) && !capturing())
            return optional_item_presentation_model_index{};
        size const cellSpacing = presentation_model().cell_spacing(*this);
        point const adjustedPos = aPosition.max(item_display_rect().top_left()).min(item_display_rect().bottom_right() - size{ 1.0, 1.0 } );
        item_presentation_model_index const rowIndex = presentation_model().item_at(adjustedPos.y - item_display_rect().top() + vertical_scrollbar().position(), *this).first;
        item_presentation_model_index index = rowIndex;
        for (uint32_t col = 0; col < presentation_model().columns(); ++col) // TODO: O(n) isn't good enough if lots of columns
        {
            index.set_column(col);
            if (cell_rect(index, cell_part::Background).contains_x(adjustedPos))
            {
                if (aPosition.y < item_display_rect().top() && index.row() > 0)
                    index.set_row(index.row() - 1);
                else if (aPosition.y >= item_display_rect().bottom() && index.row() < presentation_model().rows() - 1)
                    index.set_row(index.row() + 1);
                if (aPosition.x < item_display_rect().left() && index.column() > 0)
                    index.set_column(index.column() - 1);
                else if (aPosition.x >= item_display_rect().right() && index.column() < presentation_model().columns(index.row()) - 1)
                    index.set_column(index.column() + 1);
                return index;
            }
        }
        return aIncludeEntireRow ? rowIndex : optional_item_presentation_model_index{};
    }

    void item_view::init()
    {
        set_focus_policy(focus_policy::ClickTabFocus);
        set_padding(neogfx::padding{});
        iSink += service<i_app>().current_style_changed([this](style_aspect)
        {
            if (selection_model().has_current_index())
                make_visible(selection_model().current_index());
        });
    }

    void item_view::invalidate_item(item_presentation_model_index const& aItemIndex)
    {
        update_scrollbar_visibility();
        update();
        if (iHoverCell && iHoverCell->row() >= aItemIndex.row())
            iHoverCell = std::nullopt;
    }

    void item_view::update_hover(const optional_point& aPosition)
    {
        auto oldHoverCell = iHoverCell;
        iHoverCell = aPosition ? item_at(*aPosition) : std::nullopt;
        if (iHoverCell != oldHoverCell || (iHoverCell && (presentation_model().cell_checkable(*iHoverCell))))
        {
            if (oldHoverCell)
                update(cell_rect(*oldHoverCell, cell_part::Background));
            if (iHoverCell)
                update(cell_rect(*iHoverCell, cell_part::Background));
        }
    }

    item_selection_operation item_view::to_selection_operation(key_modifiers_e aKeyModifiers) const
    {
        switch (selection_model().mode())
        {
        case item_selection_mode::NoSelection:
            return item_selection_operation::None;
        case item_selection_mode::SingleSelection:
            if ((aKeyModifiers & KeyModifier_CTRL) != KeyModifier_NONE)
                return item_selection_operation::ClearAndToggle;
            else
                return item_selection_operation::ClearAndSelect;
            break;
        case item_selection_mode::MultipleSelection:
            // todo
            return item_selection_operation::None;
        case item_selection_mode::ExtendedSelection:
            if ((aKeyModifiers & KeyModifier_CTRL) != KeyModifier_NONE)
                return item_selection_operation::Toggle;
            else
                return item_selection_operation::ClearAndSelect;
            break;
        default:
            return item_selection_operation::None;
        }
    }

    void item_view::select(item_presentation_model_index const& aItemIndex, key_modifiers_e aKeyModifiers)
    {
        select(aItemIndex, to_selection_operation(aKeyModifiers));
    }

    void item_view::select(item_presentation_model_index const& aItemIndex, item_selection_operation aSelectionOperation)
    {
        selection_model().set_current_index(aItemIndex);
        selection_model().select(aItemIndex, aSelectionOperation);
    }
}