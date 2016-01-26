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

#include "neogfx.hpp"
#include "app.hpp"
#include "item_view.hpp"

namespace neogfx
{
	namespace
	{
		struct scoped_counter
		{
			uint32_t& iCounter;
			scoped_counter(uint32_t& aCounter) : iCounter(aCounter) { ++iCounter; }
			~scoped_counter() { --iCounter; }
		};
	}

	item_view::item_view() :
		scrollable_widget(),
		iBatchUpdatesInProgress(0)
	{
		set_focus_policy(focus_policy::ClickTabFocus);
		set_margins(neogfx::margins(0.0));
	}

	item_view::item_view(i_widget& aParent) : 
		scrollable_widget(aParent),
		iBatchUpdatesInProgress(0)
	{
		set_focus_policy(focus_policy::ClickTabFocus);
		set_margins(neogfx::margins(0.0));
	}

	item_view::item_view(i_layout& aLayout) :
		scrollable_widget(aLayout),
		iBatchUpdatesInProgress(0)
	{
		set_focus_policy(focus_policy::ClickTabFocus);
		set_margins(neogfx::margins(0.0));
	}

	item_view::~item_view()
	{
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
			if (has_selection_model())
				selection_model().set_item_model(aModel);
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
			if (has_selection_model())
				selection_model().set_item_model(*aModel);
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
		iPresentationModel = std::shared_ptr<i_item_presentation_model>(std::shared_ptr<i_item_presentation_model>(), &aPresentationModel);
		if (has_model())
			presentation_model().set_item_model(model());
		presentation_model_changed();
		update_scrollbar_visibility();
		update();
	}

	void item_view::set_presentation_model(std::shared_ptr<i_item_presentation_model> aPresentationModel)
	{
		iPresentationModel = aPresentationModel;
		if (has_presentation_model() && has_model())
			presentation_model().set_item_model(model());
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
		iSelectionModel = std::shared_ptr<i_item_selection_model>(std::shared_ptr<i_item_selection_model>(), &aSelectionModel);
		if (has_model())
			selection_model().set_item_model(model());
		selection_model_changed();
		update_scrollbar_visibility();
		update();
	}

	void item_view::set_selection_model(std::shared_ptr<i_item_selection_model> aSelectionModel)
	{
		iSelectionModel = aSelectionModel;
		if (has_selection_model() && has_model())
			selection_model().set_item_model(model());
		selection_model_changed();
		update_scrollbar_visibility();
		update();
	}

	void item_view::start_batch_update()
	{
		if (++iBatchUpdatesInProgress == 1)
			batch_update_started();
	}
	void item_view::end_batch_update()
	{
		if (--iBatchUpdatesInProgress == 0)
		{
			batch_update_ended();
			update_scrollbar_visibility();
			update();
		}
	}

	std::pair<item_model_index::value_type, coordinate> item_view::first_visible_item(graphics_context& aGraphicsContext) const
	{
		return presentation_model().item_at(vertical_scrollbar().position(), aGraphicsContext);
	}

	std::pair<item_model_index::value_type, coordinate> item_view::last_visible_item(graphics_context& aGraphicsContext) const
	{
		return presentation_model().item_at(vertical_scrollbar().position() + item_display_rect().height(), aGraphicsContext);
	}

	void item_view::paint(graphics_context& aGraphicsContext) const
	{
		scrollable_widget::paint(aGraphicsContext);
		auto first = first_visible_item(aGraphicsContext);
		coordinate y = item_display_rect().top() + first.second;
		item_model_index::value_type row = first.first;
		while (y < client_rect().bottom() && row < model().rows())
		{
			optional_font of = presentation_model().cell_font(item_model_index(row));
			const neogfx::font& f = (of != boost::none ? *of : app::instance().current_style().default_font());
			optional_colour textColour = presentation_model().cell_colour(item_model_index(row), i_item_presentation_model::ForegroundColour);
			if (textColour == boost::none)
				textColour = has_foreground_colour() ? foreground_colour() : app::instance().current_style().default_text_colour();
			coordinate x = -static_cast<coordinate>(horizontal_scrollbar().position());
			dimension h = 0.0;
			for (uint32_t col = 0; col < model().columns(row); ++col)
			{
				dimension margin = units_converter(*this).from_device_units(1.0);
				x += margin;
				if (col != 0)
					x += cell_spacing().cx;
				size e = presentation_model().cell_extents(item_model_index(row, col), aGraphicsContext);
				dimension cw = column_width(col);
				rect cellRect(point(x, y), size(cw, e.cy));
				aGraphicsContext.scissor_on(default_clip_rect().intersection(cellRect));
				aGraphicsContext.draw_glyph_text(point(x, y), presentation_model().cell_glyph_text(item_model_index(row, col), aGraphicsContext), f, *textColour);
				if (iSelectionModel->has_current_index() && iSelectionModel->current_index() == item_model_index(row, col) && has_focus())
					aGraphicsContext.draw_focus_rect(cellRect, pen(background_colour().light() ? colour::Black : colour::White, 1));
				aGraphicsContext.scissor_off();
				x += cw;
				x += margin;
				h = std::max(h, e.cy);
			}
			y += h;
			++row;
		}
	}

	void item_view::focus_gained()
	{
		scrollable_widget::focus_gained();
		if (iModel->rows() > 0 && !iSelectionModel->has_current_index())
			iSelectionModel->set_current_index(item_model_index(0, 0));
	}

	void item_view::key_pressed(scan_code_e aScanCode, key_code_e aKeyCode, key_modifiers_e aKeyModifiers)
	{
		if (iModel->rows() == 0)
		{
			scrollable_widget::key_pressed(aScanCode, aKeyCode, aKeyModifiers);
			return;
		}
		if (iSelectionModel->has_current_index())
		{
			item_model_index currentIndex = iSelectionModel->current_index();
			item_model_index newIndex = currentIndex;
			switch (aScanCode)
			{
			case ScanCode_LEFT:
				if (currentIndex.column() > 0)
					newIndex.set_column(currentIndex.column() - 1);
				break;
			case ScanCode_RIGHT:
				if (currentIndex.column() < iModel->columns(currentIndex) - 1)
					newIndex.set_column(currentIndex.column() + 1);
				break;
			case ScanCode_UP:
				if (currentIndex.row() > 0)
					newIndex.set_row(currentIndex.row() - 1);
				break;
			case ScanCode_DOWN:
				if (currentIndex.row() < iModel->rows() - 1)
					newIndex.set_row(currentIndex.row() + 1);
				break;
			case ScanCode_PAGEUP:
				{
					graphics_context gc(*this);
					newIndex.set_row(
						iPresentationModel->item_at(
							iPresentationModel->item_position(currentIndex, gc) - 
							item_display_rect().height() + 
							iPresentationModel->item_height(currentIndex, gc), gc).first);
				}
				break;
			case ScanCode_PAGEDOWN:
				{
					graphics_context gc(*this);
					newIndex.set_row(
						iPresentationModel->item_at(
							iPresentationModel->item_position(currentIndex, gc) + 
							item_display_rect().height(), gc).first);
				}
				break;
			case ScanCode_HOME:
				if ((aKeyModifiers & KeyModifier_CTRL) == 0)
					newIndex.set_column(0);
				else
					newIndex = item_model_index(0, 0);
				break;
			case ScanCode_END:
				if ((aKeyModifiers & KeyModifier_CTRL) == 0)
					newIndex.set_column(iModel->columns() - 1);
				else
					newIndex = item_model_index(iModel->rows() - 1, iModel->columns() - 1);
				break;
			default:
				scrollable_widget::key_pressed(aScanCode, aKeyCode, aKeyModifiers);
				break;
			}
			if (newIndex != currentIndex)
			{
				iSelectionModel->set_current_index(newIndex);
				make_visible(iSelectionModel->current_index());
				update();
			}
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
				iSelectionModel->set_current_index(item_model_index(0, 0));
				make_visible(iSelectionModel->current_index());
				update();
				break;
			default:
				scrollable_widget::key_pressed(aScanCode, aKeyCode, aKeyModifiers);
				break;
			}
		}
	}

	item_view::child_widget_scrolling_disposition_e item_view::scrolling_disposition() const
	{
		return DontScrollChildWidget;
	}

	void item_view::update_scrollbar_visibility(usv_stage_e aStage)
	{
		if (!has_model() || !has_presentation_model())
		{
			scrollable_widget::update_scrollbar_visibility(aStage);
			return;
		}
		switch (aStage)
		{
		case UsvStageInit:
		{
			graphics_context gc(*this);
			i_scrollbar::value_type oldPosition = vertical_scrollbar().position();
			vertical_scrollbar().set_maximum(units_converter(gc).to_device_units(item_total_area(gc)).cy);
			vertical_scrollbar().set_step(font().height());
			vertical_scrollbar().set_page(units_converter(*this).to_device_units(item_display_rect()).cy);
			vertical_scrollbar().set_position(oldPosition);
			if (vertical_scrollbar().maximum() - vertical_scrollbar().page() > 0.0)
				vertical_scrollbar().show();
			else
				vertical_scrollbar().hide();
			oldPosition = horizontal_scrollbar().position();
			horizontal_scrollbar().set_maximum(units_converter(gc).to_device_units(item_total_area(gc)).cx);
			horizontal_scrollbar().set_step(font().height());
			horizontal_scrollbar().set_page(units_converter(*this).to_device_units(item_display_rect()).cx);
			horizontal_scrollbar().set_position(oldPosition);
			if (horizontal_scrollbar().maximum() - horizontal_scrollbar().page() > 0.0)
				horizontal_scrollbar().show();
			else
				horizontal_scrollbar().hide();
			scrollable_widget::update_scrollbar_visibility(aStage);
		}
		break;
		case UsvStageCheckVertical1:
		case UsvStageCheckVertical2:
		case UsvStageCheckHorizontal:
		case UsvStageDone:
			scrollable_widget::update_scrollbar_visibility(aStage);
			break;
		default:
			break;
		}
	}

	void item_view::column_info_changed(const i_item_model& aModel, item_model_index::value_type aColumnIndex)
	{
		if (iBatchUpdatesInProgress)
			return;
		update_scrollbar_visibility();
		update();
	}

	void item_view::item_added(const i_item_model& aModel, const item_model_index& aItemIndex)
	{
		if (iBatchUpdatesInProgress)
			return;
		update_scrollbar_visibility();
		update();
	}

	void item_view::item_changed(const i_item_model& aModel, const item_model_index& aItemIndex)
	{
		if (iBatchUpdatesInProgress)
			return;
		update_scrollbar_visibility();
		update();
	}

	void item_view::item_removed(const i_item_model& aModel, const item_model_index& aItemIndex)
	{
		if (iBatchUpdatesInProgress)
			return;
		update_scrollbar_visibility();
		update();
	}

	void item_view::items_sorted(const i_item_model& aModel)
	{
		update();
	}

	void item_view::model_destroyed(const i_item_model& aModel)
	{
		iModel.reset();
	}

	void item_view::make_visible(const item_model_index& aItemIndex)
	{
		graphics_context gc(*this);
		double itemHeight = presentation_model().item_height(aItemIndex, gc);
		if (itemHeight < item_display_rect().height())
		{
			double itemPosition = presentation_model().item_position(aItemIndex, gc);
			if (itemPosition < vertical_scrollbar().position())
				vertical_scrollbar().set_position(itemPosition);
			else if (itemPosition + itemHeight > vertical_scrollbar().position() + item_display_rect().height())
				vertical_scrollbar().set_position(itemPosition - item_display_rect().height() + itemHeight);
		}
	}
}