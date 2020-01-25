// item_view.cpp
/*
  neogfx C++ GUI Library
  Copyright (c) 2015 Leigh Johnston.  All Rights Reserved.
  
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
#include <neolib/scoped.hpp>
#include <neogfx/app/i_app.hpp>
#include <neogfx/gfx/graphics_context.hpp>
#include <neogfx/gui/widget/item_view.hpp>
#include <neogfx/gui/widget/line_edit.hpp>
#include <neogfx/gui/widget/spin_box.hpp>

namespace neogfx
{
    item_view::item_view(scrollbar_style aScrollbarStyle, frame_style aFrameStyle) :
        scrollable_widget{ aScrollbarStyle, aFrameStyle }, iHotTracking{ false }, iIgnoreNextMouseMove{ false }, iBeginningEdit{ false }, iEndingEdit{ false }
    {
        init();
    }

    item_view::item_view(i_widget& aParent, scrollbar_style aScrollbarStyle, frame_style aFrameStyle) :
        scrollable_widget{ aParent, aScrollbarStyle, aFrameStyle }, iHotTracking{ false }, iIgnoreNextMouseMove{ false }, iBeginningEdit{ false }, iEndingEdit{ false }
    {
        init();
    }

    item_view::item_view(i_layout& aLayout, scrollbar_style aScrollbarStyle, frame_style aFrameStyle) :
        scrollable_widget{ aLayout, aScrollbarStyle, aFrameStyle }, iHotTracking{ false }, iIgnoreNextMouseMove{ false }, iBeginningEdit{ false }, iEndingEdit{ false }
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
        set_model(std::shared_ptr<i_item_model>{std::shared_ptr<i_item_model>{}, &aModel});
    }

    void item_view::set_model(std::shared_ptr<i_item_model> aModel)
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
            iModelSink += model().destroying([this]() { iModel = nullptr; });
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
        set_presentation_model(std::shared_ptr<i_item_presentation_model>{std::shared_ptr<i_item_presentation_model>{}, &aPresentationModel});
    }

    void item_view::set_presentation_model(std::shared_ptr<i_item_presentation_model> aPresentationModel)
    {
        if (iPresentationModel == aPresentationModel)
            return;
        iPresentationModelSink.clear();
        iPresentationModel = aPresentationModel;
        if (has_presentation_model())
        {
            iPresentationModelSink += presentation_model().item_model_changed([this](const i_item_model& aItemModel) { item_model_changed(aItemModel); });
            iPresentationModelSink += presentation_model().item_added([this](const item_presentation_model_index& aItemIndex) { item_added(aItemIndex); });
            iPresentationModelSink += presentation_model().item_changed([this](const item_presentation_model_index& aItemIndex) { item_changed(aItemIndex); });
            iPresentationModelSink += presentation_model().item_removed([this](const item_presentation_model_index& aItemIndex) { item_removed(aItemIndex); });
            iPresentationModelSink += presentation_model().items_sorting([this]() { items_sorting(); });
            iPresentationModelSink += presentation_model().items_sorted([this]() { items_sorted(); });
            iPresentationModelSink += presentation_model().items_filtering([this]() { items_filtering(); });
            iPresentationModelSink += presentation_model().items_filtered([this]() { items_filtered(); });
        }
        if (has_presentation_model() && has_model())
            presentation_model().set_item_model(model());
        if (has_presentation_model() && has_selection_model())
            selection_model().set_presentation_model(*aPresentationModel);
        presentation_model_changed();
        update_scrollbar_visibility();
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
        set_selection_model(std::shared_ptr<i_item_selection_model>{std::shared_ptr<i_item_selection_model>{}, &aSelectionModel});
    }

    void item_view::set_selection_model(std::shared_ptr<i_item_selection_model> aSelectionModel)
    {
        if (iSelectionModel == aSelectionModel)
            return;
        iSelectionModelSink.clear();
        iSelectionModel = aSelectionModel;
        if (has_selection_model())
        {
            iSelectionModelSink += selection_model().presentation_model_added([this](i_item_presentation_model& aNewModel) { presentation_model_added(aNewModel); });
            iSelectionModelSink += selection_model().presentation_model_changed([this](i_item_presentation_model& aNewModel, i_item_presentation_model& aOldModel) { presentation_model_changed(aNewModel, aOldModel); });
            iSelectionModelSink += selection_model().presentation_model_removed([this](i_item_presentation_model& aOldModel) { presentation_model_removed(aOldModel); });
            iSelectionModelSink += selection_model().mode_changed([this](item_selection_mode aNewMode) { mode_changed(aNewMode); });
            iSelectionModelSink += selection_model().current_index_changed([this](const optional_item_presentation_model_index& aCurrentIndex, const optional_item_presentation_model_index& aPreviousIndex) { current_index_changed(aCurrentIndex, aPreviousIndex); });
            iSelectionModelSink += selection_model().selection_changed([this](const item_selection& aCurrentSelection, const item_selection& aPreviousSelection) { selection_changed(aCurrentSelection, aPreviousSelection); });
            iSelectionModelSink += selection_model().destroyed([this]() { iSelectionModel = nullptr; });
            if (has_presentation_model())
                selection_model().set_presentation_model(presentation_model());
        }
        selection_model_changed();
        update_scrollbar_visibility();
        update();
    }

    const optional_easing& item_view::default_transition() const
    {
        return iDefaultTransition;
    }

    void item_view::set_default_transition(const optional_easing& aTransition)
    {
        iDefaultTransition = aTransition;
    }

    std::pair<item_presentation_model_index::value_type, coordinate> item_view::first_visible_item(i_graphics_context& aGraphicsContext) const
    {
        return presentation_model().item_at(vertical_scrollbar().position(), aGraphicsContext);
    }

    std::pair<item_presentation_model_index::value_type, coordinate> item_view::last_visible_item(i_graphics_context& aGraphicsContext) const
    {
        return presentation_model().item_at(vertical_scrollbar().position() + item_display_rect().height(), aGraphicsContext);
    }

    void item_view::layout_items_completed()
    {
        scrollable_widget::layout_items_completed();
    }

    widget_part item_view::hit_test(const point& aPosition) const
    {
        if (item_display_rect().contains(aPosition))
            return widget_part::Client;
        else
        {
            auto result = scrollable_widget::hit_test(aPosition);
            if (result != widget_part::Client)
                return result;
            else
                return widget_part::NonClient;
        }
    }

    size_policy item_view::size_policy() const
    {
        if (has_size_policy())
            return scrollable_widget::size_policy();
        return size_constraint::Expanding;
    }

    void item_view::paint(i_graphics_context& aGraphicsContext) const
    {
        scrollable_widget::paint(aGraphicsContext);
        auto first = first_visible_item(aGraphicsContext);
        bool finished = false;
        rect clipRect = default_clip_rect().intersection(item_display_rect());
        for (item_presentation_model_index::value_type row = first.first; row < presentation_model().rows() && !finished; ++row)
        {
            finished = true;
            for (uint32_t col = 0; col < presentation_model().columns(); ++col)
            {
                auto const itemIndex = item_presentation_model_index{ row, col };
                rect cellRect = cell_rect(itemIndex, aGraphicsContext);
                if (cellRect.y > clipRect.bottom())
                    continue;
                finished = false;
                optional_colour backgroundColour = presentation_model().cell_colour(itemIndex, item_cell_colour_type::Background);
                rect cellBackgroundRect = cell_rect(itemIndex, aGraphicsContext, cell_part::Background);
                if (backgroundColour != std::nullopt)
                {
                    scoped_scissor scissor(aGraphicsContext, clipRect.intersection(cellBackgroundRect));
                    aGraphicsContext.fill_rect(cellBackgroundRect, *backgroundColour);
                }
                {
                    scoped_scissor scissor(aGraphicsContext, clipRect.intersection(cellRect));
                    auto const& cellImage = presentation_model().cell_image(itemIndex);
                    if (cellImage != std::nullopt)
                        aGraphicsContext.draw_texture(cell_rect(itemIndex, aGraphicsContext, cell_part::Image), *cellImage);
                    auto cellTextRect = cell_rect(itemIndex, aGraphicsContext, cell_part::Text);
                    auto const& glyphText = presentation_model().cell_glyph_text(itemIndex, aGraphicsContext);
                    optional_colour textColour = presentation_model().cell_colour(itemIndex, item_cell_colour_type::Foreground);
                    if (textColour == std::nullopt)
                        textColour = has_foreground_colour() ? foreground_colour() : service<i_app>().current_style().palette().text_colour();
                    aGraphicsContext.draw_glyph_text(cellTextRect.top_left(), glyphText, *textColour);
                }
                if (selection_model().has_current_index() && selection_model().current_index() != editing() && selection_model().current_index() == itemIndex && has_focus())
                {
                    scoped_scissor scissor(aGraphicsContext, clipRect.intersection(cellBackgroundRect));
                    aGraphicsContext.draw_focus_rect(cellBackgroundRect);
                }
            }
        }
    }

    void item_view::capture_released()
    {
        iMouseTracker = std::nullopt;
    }

    neogfx::focus_policy item_view::focus_policy() const
    {
        auto result = scrollable_widget::focus_policy();
        if (editing() != std::nullopt)
            result |= (focus_policy::ConsumeReturnKey | focus_policy::ConsumeTabKey);
        return result;
    }

    void item_view::focus_gained(focus_reason aFocusReason)
    {
        scrollable_widget::focus_gained(aFocusReason);
        if (aFocusReason == focus_reason::ClickClient)
        {
            auto item = item_at(root().mouse_position() - origin());
            if (item != std::nullopt)
                selection_model().set_current_index(*item);
            if (editing() == std::nullopt && selection_model().has_current_index() && presentation_model().cell_editable(selection_model().current_index()) == item_cell_editable::WhenFocused)
                edit(selection_model().current_index());
        }
        else if (aFocusReason != focus_reason::Other)
        {
            if (model().rows() > 0 && !selection_model().has_current_index())
                selection_model().set_current_index(item_presentation_model_index{ 0, 0 });
        }
    }

    void item_view::mouse_button_pressed(mouse_button aButton, const point& aPosition, key_modifiers_e aKeyModifiers)
    {
        scrollable_widget::mouse_button_pressed(aButton, aPosition, aKeyModifiers);
        if (capturing() && aButton == mouse_button::Left && item_display_rect().contains(aPosition))
        {
            auto item = item_at(aPosition);
            if (item != std::nullopt)
            {
                selection_model().set_current_index(*item);
                if (selection_model().has_current_index() && selection_model().current_index() == *item && presentation_model().cell_editable(*item) == item_cell_editable::WhenFocused)
                    edit(*item);
            }            
            if (capturing())
            {
                iMouseTracker.emplace(service<neolib::async_task>(), [this](neolib::callback_timer& aTimer)
                {
                    aTimer.again();
                    auto item = item_at(root().mouse_position() - origin());
                    if (item != std::nullopt)
                        selection_model().set_current_index(*item);
                }, 20);
            }
        }
    }

    void item_view::mouse_button_double_clicked(mouse_button aButton, const point& aPosition, key_modifiers_e aKeyModifiers)
    {
        scrollable_widget::mouse_button_double_clicked(aButton, aPosition, aKeyModifiers);
        if (aButton == mouse_button::Left && item_display_rect().contains(aPosition))
        {
            auto item = item_at(aPosition);
            if (item != std::nullopt)
            {
                selection_model().set_current_index(*item);
                if (selection_model().current_index() == *item && presentation_model().cell_editable(*item) == item_cell_editable::OnInputEvent)
                    edit(*item);
                else
                    service<i_basic_services>().system_beep();
            }
        }
    }

    void item_view::mouse_moved(const point& aPosition)
    {
        scrollable_widget::mouse_moved(aPosition);
        if (!iIgnoreNextMouseMove)
        {
            if (hot_tracking() && client_rect().contains(aPosition))
            {
                auto item = item_at(aPosition);
                if (item != std::nullopt)
                    selection_model().set_current_index(*item);
            }
        }
        else
            iIgnoreNextMouseMove = false;
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
                        selection_model().set_current_index(selection_model().relative_to_current_index(index_location::NextCell, true, true));
                    else
                        selection_model().set_current_index(selection_model().relative_to_current_index(index_location::PreviousCell, true, true));
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
                handled = scrollable_widget::key_pressed(aScanCode, aKeyCode, aKeyModifiers);
                break;
            }
            selection_model().set_current_index(newIndex);
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
                    selection_model().set_current_index(item_presentation_model_index{ 0, 0 });
                else
                    handled = false;
                break;
            default:
                handled = scrollable_widget::key_pressed(aScanCode, aKeyCode, aKeyModifiers);
                break;
            }
        }
        return handled;
    }

    bool item_view::text_input(const std::string& aText)
    {
        bool handled = scrollable_widget::text_input(aText);
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
        scrollable_widget::update_scrollbar_visibility();
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
            vertical_scrollbar().set_step(font().height() + (has_presentation_model() ? presentation_model().cell_margins(*this).size().cy + presentation_model().cell_spacing(*this).cy : 0.0));
            vertical_scrollbar().set_page(std::max(units_converter(*this).to_device_units(item_display_rect()).cy, 0.0));
            vertical_scrollbar().set_position(iOldPositionForScrollbarVisibility.cy);
            if (vertical_scrollbar().page() > 0 && vertical_scrollbar().maximum() - vertical_scrollbar().page() > 0.0)
                vertical_scrollbar().show();
            else
                vertical_scrollbar().hide();
            break;
        case UsvStageCheckHorizontal:
            horizontal_scrollbar().set_maximum(units_converter(*this).to_device_units(total_item_area(*this)).cx);
            horizontal_scrollbar().set_step(font().height() + (has_presentation_model() ? presentation_model().cell_margins(*this).size().cy + presentation_model().cell_spacing(*this).cy : 0.0));
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
        if (editing() != std::nullopt && presentation_model().cell_editable(*editing()) == item_cell_editable::No)
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

    void item_view::item_added(const item_presentation_model_index&)
    {
        update_scrollbar_visibility();
        update();
    }

    void item_view::item_changed(const item_presentation_model_index&)
    {
        update_scrollbar_visibility();
        update();
    }

    void item_view::item_removed(const item_presentation_model_index&)
    {
        update_scrollbar_visibility();
        update();
    }

    void item_view::items_sorting()
    {
        if (selection_model().has_current_index())
            iSavedModelIndex = presentation_model().to_item_model_index(selection_model().current_index());
        end_edit(true);
    }

    void item_view::items_sorted()
    {
        if (iSavedModelIndex != std::nullopt && presentation_model().have_item_model_index(*iSavedModelIndex))
            selection_model().set_current_index(presentation_model().from_item_model_index(*iSavedModelIndex));
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
            selection_model().set_current_index(item_presentation_model_index{});
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
        if (aCurrentIndex != std::nullopt)
        {
            make_visible(*aCurrentIndex);
            update(cell_rect(*aCurrentIndex, cell_part::Background));
            if (!selection_model().sorting() && !selection_model().filtering())
            {
                if (aPreviousIndex != std::nullopt)
                {
                    if (editing() != std::nullopt && presentation_model().cell_editable(*aCurrentIndex) == item_cell_editable::WhenFocused && editing() != aCurrentIndex && iSavedModelIndex == std::nullopt)
                        edit(*aCurrentIndex);
                    else if (editing() != std::nullopt)
                        end_edit(true);
                }
            }
        }
        if (aPreviousIndex != std::nullopt)
            update(cell_rect(*aPreviousIndex, cell_part::Background));
    }

    void item_view::selection_changed(const item_selection&, const item_selection&)
    {
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
            if (&widget_for_mouse_event(root().mouse_position() - origin()) == this)
                iIgnoreNextMouseMove = true;
        }
    }

    void item_view::disable_hot_tracking()
    {
        iHotTracking = false;
    }

    bool item_view::is_visible(const item_presentation_model_index& aItemIndex) const
    {
        return item_display_rect().contains(cell_rect(aItemIndex, cell_part::Background));
    }

    void item_view::make_visible(const item_presentation_model_index& aItemIndex, const optional_easing& aTransition)
    {
        auto const& transition = (aTransition == std::nullopt ? default_transition() : aTransition);
        graphics_context gc{ *this, graphics_context::type::Unattached };
        auto const& cellRect = cell_rect(aItemIndex, gc, cell_part::Background);
        auto const& displayRect = item_display_rect();
        auto const& intersectRect = cellRect.intersection(displayRect);
        if (intersectRect.height() < cellRect.height())
        {
            if (cellRect.top() < displayRect.top())
                vertical_scrollbar().set_position(vertical_scrollbar().position() + (cellRect.top() - displayRect.top()), transition);
            else if (cellRect.bottom() > displayRect.bottom())
                vertical_scrollbar().set_position(vertical_scrollbar().position() + (cellRect.bottom() - displayRect.bottom()), transition);
        }
        if (intersectRect.width() < cellRect.width())
        {
            if (cellRect.left() < displayRect.left())
                horizontal_scrollbar().set_position(horizontal_scrollbar().position() + (cellRect.left() - displayRect.left()), transition);
            else if (cellRect.right() > displayRect.right())
                horizontal_scrollbar().set_position(horizontal_scrollbar().position() + (cellRect.right() - displayRect.right()), transition);
        }
    }

    const optional_item_presentation_model_index& item_view::editing() const
    {
        return iEditing;
    }

    void item_view::edit(const item_presentation_model_index& aItemIndex)
    {
        if (editing() == aItemIndex || beginning_edit() || ending_edit() || presentation_model().cell_editable(aItemIndex) == item_cell_editable::No )
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
            selection_model().set_current_index(newIndex);
        iEditing = newIndex;
        if (presentation_model().cell_colour(newIndex, item_cell_colour_type::Background) != optional_colour{})
            editor().set_background_colour(presentation_model().cell_colour(newIndex, item_cell_colour_type::Background));
        editor().set_margins(presentation_model().cell_margins(*this));
        auto editorRect = cell_rect(newIndex, cell_part::Text);
        editorRect.inflate(presentation_model().cell_margins(*this));
        editor().move(editorRect.position());
        editor().resize(editorRect.extents());
        if (editor_has_text_edit())
        {
            auto& textEdit = editor_text_edit();
            if (presentation_model().cell_colour(newIndex, item_cell_colour_type::Background) != optional_colour{})
                textEdit.set_background_colour(presentation_model().cell_colour(newIndex, item_cell_colour_type::Background));
            if (&editor() != &textEdit)
                textEdit.set_margins(neogfx::margins{});
            optional_colour textColour = presentation_model().cell_colour(newIndex, item_cell_colour_type::Foreground);
            if (textColour == std::nullopt)
                textColour = has_foreground_colour() ? foreground_colour() : service<i_app>().current_style().palette().text_colour();
            optional_colour backgroundColour = presentation_model().cell_colour(newIndex, item_cell_colour_type::Background);
            textEdit.set_default_style(text_edit::style{ presentation_model().cell_font(newIndex), *textColour, backgroundColour != std::nullopt ? colour_or_gradient{ *backgroundColour } : colour_or_gradient{} });
            textEdit.set_text(presentation_model().cell_to_string(newIndex));
            textEdit.Focus([this, newIndex](neogfx::focus_event fe)
            {
                if (fe == neogfx::focus_event::FocusLost && !has_focus() && (!root().has_focused_widget() || !root().focused_widget().is_descendent_of(*this) || !selection_model().has_current_index() || selection_model().current_index() != newIndex))
                    end_edit(true);
            });
            textEdit.Keyboard([this, &textEdit, newIndex](neogfx::keyboard_event ke)
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
            if (textEdit.client_rect().contains(root().mouse_position() - textEdit.origin()))
            {
                bool enableDragger = capturing();
                if (enableDragger)
                    release_capture();
                textEdit.set_cursor_position(root().mouse_position() - textEdit.origin(), true, enableDragger);
            }
            else
                textEdit.cursor().set_anchor(textEdit.cursor().position());
        }
    }

    void item_view::end_edit(bool aCommit)
    {
        if (editing() == std::nullopt)
            return;
        if (aCommit && presentation_model().cell_editable(*editing()) == item_cell_editable::No)
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

    void item_view::header_view_updated(header_view&, header_view_update_reason aUpdateReason)
    {
        if (aUpdateReason == header_view_update_reason::FullUpdate)
        {
            bool wasVisible = selection_model().has_current_index() && is_visible(selection_model().current_index());
            layout_items();
            if (wasVisible)
                make_visible(selection_model().current_index());
        }
        else
        {
            update_scrollbar_visibility();
            update();
        }
        if (editing() != std::nullopt)
        {
            auto editorRect = cell_rect(*editing(), cell_part::Text);
            editorRect.inflate(presentation_model().cell_margins(*this));
            editor().move(editorRect.position());
            editor().resize(editorRect.extents());
        }
    }

    rect item_view::row_rect(const item_presentation_model_index& aItemIndex) const
    {
        rect result = item_display_rect();
        result.y = presentation_model().item_position(aItemIndex, *this) - vertical_scrollbar().position();
        result.cy = presentation_model().item_height(aItemIndex, *this);
        return result;
    }

    rect item_view::cell_rect(const item_presentation_model_index& aItemIndex, cell_part aPart) const
    {
        switch(aPart)
        {
        case cell_part::Background:
        case cell_part::Foreground:
            {
                size const cellSpacing = presentation_model().cell_spacing(*this);
                coordinate y = presentation_model().item_position(aItemIndex, *this);
                dimension const h = presentation_model().item_height(aItemIndex, *this);
                coordinate x = 0.0;
                for (uint32_t col = 0; col < presentation_model().columns(); ++col)
                {
                    if (col != 0)
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
                            if (col == presentation_model().columns() - 1)
                            {
                                result.indent(point{ -cellSpacing.cx / 2.0, 0.0 });
                                result.cx += (item_display_rect().right() - result.right());
                            }
                            else
                                result.inflate(size{ cellSpacing.cx / 2.0, 0.0 });
                            break;
                        case cell_part::Foreground:
                            result.deflate(size{ 0.0, cellSpacing.cy / 2.0 });
                            break;
                        }
                        return result;
                    }
                    x += column_width(col);
                }
                throw i_item_presentation_model::bad_column_index();
            }
        default:
            {
                graphics_context gc{ *this, graphics_context::type::Unattached };
                return cell_rect(aItemIndex, gc, aPart);
            }
        }
    }
        
    rect item_view::cell_rect(const item_presentation_model_index& aItemIndex, i_graphics_context& aGraphicsContext, cell_part aPart) const
    {
        switch (aPart)
        {
        case cell_part::Background:
        case cell_part::Foreground:
            return cell_rect(aItemIndex, aPart);
        case cell_part::Image:
            {
                auto const& cellImageSize = presentation_model().cell_image_size(aItemIndex);
                if (!cellImageSize)
                    throw invalid_cell_part();
                auto cellRect = cell_rect(aItemIndex);
                cellRect.indent(point{ presentation_model().cell_margins(*this).left, ((cellRect.cy - cellImageSize->cy) / 2.0) });
                cellRect.extents() = *cellImageSize;
                return cellRect;
            }
            break;
        case cell_part::Text:
            {
                auto cellRect = cell_rect(aItemIndex);
                cellRect.deflate(presentation_model().cell_margins(*this));
                auto const& cellImageSize = presentation_model().cell_image_size(aItemIndex);
                if (cellImageSize)
                    cellRect.indent(point{ cellImageSize->cx + presentation_model().cell_spacing(aGraphicsContext).cx, 0.0 });
                auto const& glyphText = presentation_model().cell_glyph_text(aItemIndex, aGraphicsContext);
                auto const textHeight = std::max(glyphText.extents().cy,
                    (presentation_model().cell_font(aItemIndex) == std::nullopt ? presentation_model().default_font() : *presentation_model().cell_font(aItemIndex)).height());
                cellRect.indent(point{ 0.0, (cellRect.height() - textHeight) / 2.0 });
                return cellRect;
            }
        }
        return rect{};
    }

    optional_item_presentation_model_index item_view::item_at(const point& aPosition, bool aIncludeEntireRow) const
    {
        if (model().rows() == 0)
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
                else if (aPosition.y >= item_display_rect().bottom() && index.row() < model().rows() - 1)
                    index.set_row(index.row() + 1);
                if (aPosition.x < item_display_rect().left() && index.column() > 0)
                    index.set_column(index.column() - 1);
                else if (aPosition.x >= item_display_rect().right() && index.column() < model().columns(index.row()) - 1)
                    index.set_column(index.column() + 1);
                return index;
            }
        }
        return aIncludeEntireRow ? rowIndex : optional_item_presentation_model_index{};
    }

    void item_view::init()
    {
        set_focus_policy(focus_policy::ClickTabFocus);
        set_margins(neogfx::margins{});
        iSink += service<i_app>().current_style_changed([this](style_aspect)
        {
            if (selection_model().has_current_index())
                make_visible(selection_model().current_index());
        });
    }
}