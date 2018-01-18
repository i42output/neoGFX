// item_view.cpp
/*
  neogfx C++ GUI Library
  Copyright(C) 2016 Leigh Johnston
  
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
#include <neolib/raii.hpp>
#include <neogfx/app/app.hpp>
#include <neogfx/gui/widget/item_view.hpp>
#include <neogfx/gui/widget/line_edit.hpp>
#include <neogfx/gui/widget/spin_box.hpp>

namespace neogfx
{
	item_view::item_view(scrollbar_style aScrollbarStyle, frame_style aFrameStyle) :
		scrollable_widget{ aScrollbarStyle, aFrameStyle }, iBeginningEdit{ false }, iEndingEdit {	false }
	{
		set_focus_policy(focus_policy::ClickTabFocus);
		set_margins(neogfx::margins{});
	}

	item_view::item_view(i_widget& aParent, scrollbar_style aScrollbarStyle, frame_style aFrameStyle) :
		scrollable_widget{ aParent, aScrollbarStyle, aFrameStyle }, iBeginningEdit{ false }, iEndingEdit{ false }
	{
		set_focus_policy(focus_policy::ClickTabFocus);
		set_margins(neogfx::margins{});
	}

	item_view::item_view(i_layout& aLayout, scrollbar_style aScrollbarStyle, frame_style aFrameStyle) :
		scrollable_widget{ aLayout, aScrollbarStyle, aFrameStyle }, iBeginningEdit{ false }, iEndingEdit{ false }
	{
		set_focus_policy(focus_policy::ClickTabFocus);
		set_margins(neogfx::margins{});
	}

	item_view::~item_view()
	{
		if (has_selection_model())
			selection_model().unsubscribe(*this);
		if (has_presentation_model())
			presentation_model().unsubscribe(*this);
		if (has_model())
			model().unsubscribe(*this);
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
		return *iModel;
	}

	i_item_model& item_view::model()
	{
		return *iModel;
	}

	void item_view::set_model(i_item_model& aModel)
	{
		if (has_model())
			model().unsubscribe(*this);
		iModel = std::shared_ptr<i_item_model>(std::shared_ptr<i_item_model>(), &aModel);
		if (has_model())
		{
			model().subscribe(*this);
			if (has_presentation_model())
				presentation_model().set_item_model(aModel);
		}
		model_changed();
		update_scrollbar_visibility();
		update();
	}

	void item_view::set_model(std::shared_ptr<i_item_model> aModel)
	{
		if (has_model())
			model().unsubscribe(*this);
		iModel = aModel;
		if (has_model())
		{
			model().subscribe(*this);
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
		return *iPresentationModel;
	}

	i_item_presentation_model& item_view::presentation_model()
	{
		return *iPresentationModel;
	}

	void item_view::set_presentation_model(i_item_presentation_model& aPresentationModel)
	{
		if (has_presentation_model())
			presentation_model().unsubscribe(*this);
		auto oldModel = iPresentationModel;
		iPresentationModel = std::shared_ptr<i_item_presentation_model>(std::shared_ptr<i_item_presentation_model>(), &aPresentationModel);
		presentation_model().subscribe(*this);
		if (has_model())
			presentation_model().set_item_model(model());
		if (has_selection_model())
			selection_model().set_presentation_model(aPresentationModel);
		presentation_model_changed();
		update_scrollbar_visibility();
		update();
	}

	void item_view::set_presentation_model(std::shared_ptr<i_item_presentation_model> aPresentationModel)
	{
		auto oldModel = iPresentationModel;
		iPresentationModel = aPresentationModel;
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
		return *iSelectionModel;
	}

	i_item_selection_model& item_view::selection_model()
	{
		return *iSelectionModel;
	}

	void item_view::set_selection_model(i_item_selection_model& aSelectionModel)
	{
		if (has_selection_model())
			selection_model().unsubscribe(*this);
		iSelectionModel = std::shared_ptr<i_item_selection_model>(std::shared_ptr<i_item_selection_model>(), &aSelectionModel);
		selection_model().subscribe(*this);
		if (has_presentation_model())
			selection_model().set_presentation_model(presentation_model());
		selection_model_changed();
		update_scrollbar_visibility();
		update();
	}

	void item_view::set_selection_model(std::shared_ptr<i_item_selection_model> aSelectionModel)
	{
		if (has_selection_model())
			selection_model().unsubscribe(*this);
		iSelectionModel = aSelectionModel;
		selection_model().subscribe(*this);
		if (has_presentation_model() && has_selection_model())
			selection_model().set_presentation_model(presentation_model());
		selection_model_changed();
		update_scrollbar_visibility();
		update();
	}

	std::pair<item_presentation_model_index::value_type, coordinate> item_view::first_visible_item(graphics_context& aGraphicsContext) const
	{
		return presentation_model().item_at(vertical_scrollbar().position(), aGraphicsContext);
	}

	std::pair<item_presentation_model_index::value_type, coordinate> item_view::last_visible_item(graphics_context& aGraphicsContext) const
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
		return size_policy::Expanding;
	}

	void item_view::paint(graphics_context& aGraphicsContext) const
	{
		scrollable_widget::paint(aGraphicsContext);
		auto first = first_visible_item(aGraphicsContext);
		bool finished = false;
		rect clipRect = default_clip_rect().intersection(item_display_rect());
		for (item_presentation_model_index::value_type row = first.first; row < presentation_model().rows() && !finished; ++row)
		{
			optional_font of = presentation_model().cell_font(item_presentation_model_index{ row });
			const neogfx::font& f = (of != boost::none ? *of : app::instance().current_style().font());
			finished = true;
			for (uint32_t col = 0; col < presentation_model().columns(); ++col)
			{
				rect cellRect = cell_rect(item_presentation_model_index{ row, col });
				if (cellRect.y > clipRect.bottom())
					continue;
				finished = false;
				optional_colour textColour = presentation_model().cell_colour(item_presentation_model_index{ row, col }, item_cell_colour_type::Foreground);
				if (textColour == boost::none)
					textColour = has_foreground_colour() ? foreground_colour() : app::instance().current_style().palette().text_colour();
				optional_colour backgroundColour = presentation_model().cell_colour(item_presentation_model_index{ row, col }, item_cell_colour_type::Background);
				rect cellBackgroundRect = cell_rect(item_presentation_model_index{ row, col }, true);
				if (backgroundColour != boost::none)
				{
					aGraphicsContext.scissor_on(clipRect.intersection(cellBackgroundRect));
					aGraphicsContext.fill_rect(cellBackgroundRect, *backgroundColour);
					aGraphicsContext.scissor_off();
				}
				aGraphicsContext.scissor_on(clipRect.intersection(cellRect));
				aGraphicsContext.draw_glyph_text(cellRect.top_left() + point(presentation_model().cell_margins(*this).left, presentation_model().cell_margins(*this).top), presentation_model().cell_glyph_text(item_presentation_model_index{ row, col }, aGraphicsContext), f, *textColour);
				aGraphicsContext.scissor_off();
				if (selection_model().has_current_index() && selection_model().current_index() != editing() && selection_model().current_index() == item_presentation_model_index{ row, col } && has_focus())
				{
					aGraphicsContext.scissor_on(clipRect.intersection(cellBackgroundRect));
					aGraphicsContext.draw_focus_rect(cellBackgroundRect);
					aGraphicsContext.scissor_off();
				}
			}
		}
	}

	void item_view::released()
	{
		iMouseTracker = boost::none;
	}

	neogfx::focus_policy item_view::focus_policy() const
	{
		auto result = scrollable_widget::focus_policy();
		if (editing() != boost::none)
			result |= (focus_policy::ConsumeReturnKey | focus_policy::ConsumeTabKey);
		return result;
	}

	void item_view::focus_gained(focus_reason aFocusReason)
	{
		scrollable_widget::focus_gained(aFocusReason);
		if (model().rows() > 0 && !selection_model().has_current_index())
			selection_model().set_current_index(item_presentation_model_index{ 0, 0 });
		if (aFocusReason == focus_reason::ClickClient)
		{
			if (editing() == boost::none && selection_model().has_current_index() && presentation_model().cell_editable(selection_model().current_index()) == item_cell_editable::WhenFocused)
				edit(selection_model().current_index());
		}
	}

	void item_view::mouse_button_pressed(mouse_button aButton, const point& aPosition, key_modifiers_e aKeyModifiers)
	{
		scrollable_widget::mouse_button_pressed(aButton, aPosition, aKeyModifiers);
		if (capturing() && aButton == mouse_button::Left && item_display_rect().contains(aPosition))
		{
			auto item = item_at(aPosition);
			if (item != boost::none)
			{
				selection_model().set_current_index(*item);
				if (selection_model().current_index() == *item && presentation_model().cell_editable(*item) == item_cell_editable::WhenFocused)
					edit(*item);
			}			
			if (capturing())
			{
				iMouseTracker = std::make_shared<neolib::callback_timer>(app::instance(), [this](neolib::callback_timer& aTimer)
				{
					aTimer.again();
					auto item = item_at(root().mouse_position() - origin());
					if (item != boost::none)
						selection_model().set_current_index(*item);
				}, 100);
			}
		}
	}

	void item_view::mouse_button_double_clicked(mouse_button aButton, const point& aPosition, key_modifiers_e aKeyModifiers)
	{
		scrollable_widget::mouse_button_double_clicked(aButton, aPosition, aKeyModifiers);
		if (aButton == mouse_button::Left && item_display_rect().contains(aPosition))
		{
			auto item = item_at(aPosition);
			if (item != boost::none)
			{
				selection_model().set_current_index(*item);
				if (selection_model().current_index() == *item && presentation_model().cell_editable(*item) == item_cell_editable::OnInputEvent)
					edit(*item);
				else
					app::instance().basic_services().system_beep();
			}
		}
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
				if (editing() != boost::none)
				{
					end_edit(true);
					return true;
				}
				break;
			case ScanCode_TAB:
				if (editing() != boost::none)
				{
					end_edit(true);
					item_presentation_model_index originalIndex = selection_model().current_index();
					if ((aKeyModifiers & KeyModifier_SHIFT) == KeyModifier_NONE)
						selection_model().set_current_index(selection_model().relative_to_current_index(index_location::NextCell, true, true));
					else
						selection_model().set_current_index(selection_model().relative_to_current_index(index_location::PreviousCell, true, true));
					edit(selection_model().current_index());
					if (editing() != boost::none && editor_has_text_edit())
					{
						editor_text_edit().cursor().set_anchor(0);
						editor_text_edit().cursor().set_position(editor_text_edit().text().size(), false);
					}
					return true;
				}
				break;
			case ScanCode_F2:
				if (editing() == boost::none && selection_model().has_current_index())
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
				selection_model().set_current_index(item_presentation_model_index{0, 0});
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
		if (editing() == boost::none && selection_model().has_current_index() && aText[0] != '\n' && aText[0] != '\t')
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
			else
				app::instance().basic_services().system_beep();
		}
		return handled;
	}

	scrolling_disposition item_view::scrolling_disposition() const
	{
		return neogfx::scrolling_disposition::ScrollChildWidgetVertically | neogfx::scrolling_disposition::ScrollChildWidgetHorizontally;
	}

	void item_view::update_scrollbar_visibility()
	{
		bool wasVisible = selection_model().has_current_index() && is_visible(selection_model().current_index());
		scrollable_widget::update_scrollbar_visibility();
		if (wasVisible)
			make_visible(selection_model().current_index());
	}

	void item_view::update_scrollbar_visibility(usv_stage_e aStage)
	{
		switch (aStage)
		{
		case UsvStageInit:
			{
				scoped_units su{ *this, units::Pixels };
				i_scrollbar::value_type oldPosition = vertical_scrollbar().position();
				vertical_scrollbar().set_maximum(units_converter(*this).to_device_units(total_item_area(*this)).cy);
				vertical_scrollbar().set_step(font().height() + (has_presentation_model() ? presentation_model().cell_margins(*this).size().cy + presentation_model().cell_spacing(*this).cy : 0.0));
				vertical_scrollbar().set_page(std::max(units_converter(*this).to_device_units(item_display_rect()).cy, 0.0));
				vertical_scrollbar().set_position(oldPosition);
				if (vertical_scrollbar().maximum() - vertical_scrollbar().page() > 0.0)
					vertical_scrollbar().show();
				else
					vertical_scrollbar().hide();
				oldPosition = horizontal_scrollbar().position();
				horizontal_scrollbar().set_maximum(units_converter(*this).to_device_units(total_item_area(*this)).cx);
				horizontal_scrollbar().set_step(font().height() + (has_presentation_model() ? presentation_model().cell_margins(*this).size().cy + presentation_model().cell_spacing(*this).cy : 0.0));
				horizontal_scrollbar().set_page(std::max(units_converter(*this).to_device_units(item_display_rect()).cx, 0.0));
				horizontal_scrollbar().set_position(oldPosition);
				if (horizontal_scrollbar().maximum() - horizontal_scrollbar().page() > 0.0)
					horizontal_scrollbar().show();
				else
					horizontal_scrollbar().hide();
				layout_items();
			}
			break;
		case UsvStageCheckVertical1:
		case UsvStageCheckVertical2:
		case UsvStageCheckHorizontal:
		case UsvStageDone:
			break;
		default:
			break;
		}
	}

	void item_view::column_info_changed(const i_item_model&, item_model_index::value_type)
	{
		update_scrollbar_visibility();
		update();
		if (editing() != boost::none && presentation_model().cell_editable(*editing()) == item_cell_editable::No)
			end_edit(false);
	}

	void item_view::item_added(const i_item_model&, const item_model_index&)
	{
	}

	void item_view::item_changed(const i_item_model&, const item_model_index&)
	{
	}

	void item_view::item_removed(const i_item_model&, const item_model_index&)
	{
	}

	void item_view::model_destroyed(const i_item_model&)
	{
		iModel = nullptr;
	}

	void item_view::column_info_changed(const i_item_presentation_model&, item_presentation_model_index::column_type)
	{
	}

	void item_view::item_model_changed(const i_item_presentation_model&, const i_item_model&)
	{
		update_scrollbar_visibility();
		update();
	}

	void item_view::item_added(const i_item_presentation_model&, const item_presentation_model_index&)
	{
		update_scrollbar_visibility();
		update();
	}

	void item_view::item_changed(const i_item_presentation_model&, const item_presentation_model_index&)
	{
		update_scrollbar_visibility();
		update();
	}

	void item_view::item_removed(const i_item_presentation_model&, const item_presentation_model_index&)
	{
		update_scrollbar_visibility();
		update();
	}

	void item_view::items_sorting(const i_item_presentation_model&)
	{
		if (selection_model().has_current_index())
			iSavedModelIndex = presentation_model().to_item_model_index(selection_model().current_index());
		end_edit(true);
	}

	void item_view::items_sorted(const i_item_presentation_model&)
	{
		if (iSavedModelIndex != boost::none)
			selection_model().set_current_index(presentation_model().from_item_model_index(*iSavedModelIndex));
		iSavedModelIndex = boost::none;
		update();
	}

	void item_view::items_filtering(const i_item_presentation_model&)
	{
		if (selection_model().has_current_index())
			iSavedModelIndex = presentation_model().to_item_model_index(selection_model().current_index());
		end_edit(true);
	}

	void item_view::items_filtered(const i_item_presentation_model&)
	{
		if (iSavedModelIndex != boost::none && presentation_model().have_item_model_index(*iSavedModelIndex))
			selection_model().set_current_index(presentation_model().from_item_model_index(*iSavedModelIndex));
		iSavedModelIndex = boost::none;
		update();
	}

	void item_view::model_destroyed(const i_item_presentation_model&)
	{
		iPresentationModel = nullptr;
	}

	void item_view::model_added(const i_item_selection_model&, i_item_presentation_model&)
	{
	}

	void item_view::model_changed(const i_item_selection_model&, i_item_presentation_model&, i_item_presentation_model&)
	{
	}

	void item_view::model_removed(const i_item_selection_model&, i_item_presentation_model&)
	{
	}

	void item_view::selection_mode_changed(const i_item_selection_model&, item_selection_mode)
	{
	}

	void item_view::current_index_changed(const i_item_selection_model&, const optional_item_presentation_model_index& aCurrentIndex, const optional_item_presentation_model_index& aPreviousIndex)
	{
		if (aCurrentIndex != boost::none)
		{
			make_visible(*aCurrentIndex);
			update(cell_rect(*aCurrentIndex, true));
			if (aPreviousIndex != boost::none)
			{
				if (editing() != boost::none && presentation_model().cell_editable(*aCurrentIndex) == item_cell_editable::WhenFocused && editing() != aCurrentIndex && iSavedModelIndex == boost::none)
					edit(*aCurrentIndex);
				else if (editing() != boost::none)
					end_edit(true);
			}
		}
		if (aPreviousIndex != boost::none)
			update(cell_rect(*aPreviousIndex, true));
	}

	void item_view::selection_changed(const i_item_selection_model&, const item_selection&, const item_selection&)
	{
	}

	void item_view::selection_model_destroyed(const i_item_selection_model&)
	{
	}

	bool item_view::is_visible(const item_presentation_model_index& aItemIndex) const
	{
		return item_display_rect().contains(cell_rect(aItemIndex, true));
	}

	void item_view::make_visible(const item_presentation_model_index& aItemIndex)
	{
		graphics_context gc{ *this, graphics_context::type::Unattached };
		rect cellRect = cell_rect(aItemIndex, true);
		if (cellRect.height() < item_display_rect().height() || cellRect.intersection(item_display_rect()).height() == 0.0)
		{
			if (cellRect.top() < item_display_rect().top())
				vertical_scrollbar().set_position(vertical_scrollbar().position() + (cellRect.top() - item_display_rect().top()));
			else if (cellRect.bottom() > item_display_rect().bottom())
				vertical_scrollbar().set_position(vertical_scrollbar().position() + (cellRect.bottom() - item_display_rect().bottom()));
		}
		if (cellRect.width() < item_display_rect().width() || cellRect.intersection(item_display_rect()).width() == 0.0)
		{
			if (cellRect.left() < item_display_rect().left())
				horizontal_scrollbar().set_position(horizontal_scrollbar().position() + (cellRect.left() - item_display_rect().left()));
			else if (cellRect.right() > item_display_rect().right())
				horizontal_scrollbar().set_position(horizontal_scrollbar().position() + (cellRect.right() - item_display_rect().right()));
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
		auto const& cellDataInfo = model().cell_data_info(modelIndex);
		if (cellDataInfo.step == boost::none)
			iEditor = std::make_shared<item_editor<line_edit>>(*this);
		else
		{
			switch (model().cell_data_info(presentation_model().to_item_model_index(aItemIndex)).type)
			{
			case item_cell_data_type::Int32:
				iEditor = std::make_shared<item_editor<basic_spin_box<int32_t>>>(*this);
				static_cast<basic_spin_box<int32_t>&>(editor()).set_step(static_variant_cast<int32_t>(cellDataInfo.step));
				static_cast<basic_spin_box<int32_t>&>(editor()).set_minimum(static_variant_cast<int32_t>(cellDataInfo.min));
				static_cast<basic_spin_box<int32_t>&>(editor()).set_maximum(static_variant_cast<int32_t>(cellDataInfo.max));
				break;
			case item_cell_data_type::UInt32:
				iEditor = std::make_shared<item_editor<basic_spin_box<uint32_t>>>(*this);
				static_cast<basic_spin_box<uint32_t>&>(editor()).set_step(static_variant_cast<uint32_t>(cellDataInfo.step));
				static_cast<basic_spin_box<uint32_t>&>(editor()).set_minimum(static_variant_cast<uint32_t>(cellDataInfo.min));
				static_cast<basic_spin_box<uint32_t>&>(editor()).set_maximum(static_variant_cast<uint32_t>(cellDataInfo.max));
				break;
			case item_cell_data_type::Int64:
				iEditor = std::make_shared<item_editor<basic_spin_box<int64_t>>>(*this);
				static_cast<basic_spin_box<int64_t>&>(editor()).set_step(static_variant_cast<int64_t>(cellDataInfo.step));
				static_cast<basic_spin_box<int64_t>&>(editor()).set_minimum(static_variant_cast<int64_t>(cellDataInfo.min));
				static_cast<basic_spin_box<int64_t>&>(editor()).set_maximum(static_variant_cast<int64_t>(cellDataInfo.max));
				break;
			case item_cell_data_type::UInt64:
				iEditor = std::make_shared<item_editor<basic_spin_box<uint64_t>>>(*this);
				static_cast<basic_spin_box<uint64_t>&>(editor()).set_step(static_variant_cast<uint64_t>(cellDataInfo.step));
				static_cast<basic_spin_box<uint64_t>&>(editor()).set_minimum(static_variant_cast<uint64_t>(cellDataInfo.min));
				static_cast<basic_spin_box<uint64_t>&>(editor()).set_maximum(static_variant_cast<uint64_t>(cellDataInfo.max));
				break;
			case item_cell_data_type::Float:
				iEditor = std::make_shared<item_editor<basic_spin_box<float>>>(*this);
				static_cast<basic_spin_box<float>&>(editor()).set_step(static_variant_cast<float>(cellDataInfo.step));
				static_cast<basic_spin_box<float>&>(editor()).set_minimum(static_variant_cast<float>(cellDataInfo.min));
				static_cast<basic_spin_box<float>&>(editor()).set_maximum(static_variant_cast<float>(cellDataInfo.max));
				break;
			case item_cell_data_type::Double:
				iEditor = std::make_shared<item_editor<basic_spin_box<double>>>(*this);
				static_cast<basic_spin_box<double>&>(editor()).set_step(static_variant_cast<double>(cellDataInfo.step));
				static_cast<basic_spin_box<double>&>(editor()).set_minimum(static_variant_cast<double>(cellDataInfo.min));
				static_cast<basic_spin_box<double>&>(editor()).set_maximum(static_variant_cast<double>(cellDataInfo.max));
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
		editor().move(cell_rect(newIndex).position());
		editor().resize(cell_rect(newIndex).extents());
		if (editor_has_text_edit())
		{
			auto& textEdit = editor_text_edit();
			if (presentation_model().cell_colour(newIndex, item_cell_colour_type::Background) != optional_colour{})
				textEdit.set_background_colour(presentation_model().cell_colour(newIndex, item_cell_colour_type::Background));
			if (&editor() != &textEdit)
				textEdit.set_margins(neogfx::margins{});
			optional_colour textColour = presentation_model().cell_colour(newIndex, item_cell_colour_type::Foreground);
			if (textColour == boost::none)
				textColour = has_foreground_colour() ? foreground_colour() : app::instance().current_style().palette().text_colour();
			optional_colour backgroundColour = presentation_model().cell_colour(newIndex, item_cell_colour_type::Background);
			textEdit.set_default_style(text_edit::style{ presentation_model().cell_font(newIndex), *textColour, backgroundColour != boost::none ? colour_or_gradient{ *backgroundColour } : colour_or_gradient{} });
			textEdit.set_text(presentation_model().cell_to_string(newIndex));
			textEdit.focus_event([this, newIndex](neogfx::focus_event fe)
			{
				if (fe == neogfx::focus_event::FocusLost && !has_focus() && (!root().has_focused_widget() || !root().focused_widget().is_descendent_of(*this) || !selection_model().has_current_index() || selection_model().current_index() != newIndex))
					end_edit(true);
			});
			textEdit.keyboard_event([this, &textEdit, newIndex](neogfx::keyboard_event ke)
			{
				if (ke.type() == neogfx::keyboard_event_type::KeyPressed && ke.scan_code() == ScanCode_ESCAPE && textEdit.cursor().position() == textEdit.cursor().anchor())
					end_edit(false);
			});
		}
		begin_edit();
	}

	void item_view::begin_edit()
	{
		if (editing() != boost::none && !ending_edit() && editor_has_text_edit())
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
		if (editing() == boost::none)
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
			iEditing = boost::none;
			iEditor = nullptr;
			if (!error)
				model().update_cell_data(modelIndex, cellData);
			else
				app::instance().basic_services().system_beep();
		}
		else
		{
			iEditing = boost::none;
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
		if (editing() != boost::none)
		{
			editor().move(cell_rect(*editing()).position());
			editor().resize(cell_rect(*editing()).extents());
		}
	}

	rect item_view::cell_rect(const item_presentation_model_index& aItemIndex, bool aBackground) const
	{
		const size cellSpacing = presentation_model().cell_spacing(*this);
		coordinate y = presentation_model().item_position(aItemIndex, *this);
		dimension h = presentation_model().item_height(aItemIndex, *this);
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
				if (aBackground)
				{
					if (col == presentation_model().columns() - 1)
					{
						result.x -= cellSpacing.cx / 2.0;
						result.cx += cellSpacing.cx / 2.0;
					}
					else
						result.inflate(size{ cellSpacing.cx / 2.0, 0.0 });
				}
				else
					result.deflate(size{ 0.0, cellSpacing.cy / 2.0 });
				return result;
			}
			x += column_width(col);
		}
		return rect{};
	}

	optional_item_presentation_model_index item_view::item_at(const point& aPosition) const
	{
		if (model().rows() == 0)
			return optional_item_presentation_model_index{};
		const size cellSpacing = presentation_model().cell_spacing(*this);
		point adjustedPos = aPosition.max(item_display_rect().top_left()).min(item_display_rect().bottom_right());
		item_presentation_model_index index = presentation_model().item_at(adjustedPos.y - item_display_rect().top() + vertical_scrollbar().position(), *this).first;
		for (uint32_t col = 0; col < presentation_model().columns(); ++col) // TODO: O(n) isn't good enough if lots of columns
		{
			index.set_column(col);
			rect cellRect = cell_rect(index);
			cellRect.extents() += cellSpacing;
			if (cellRect.contains_x(adjustedPos))
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
		return optional_item_presentation_model_index{};
	}
}