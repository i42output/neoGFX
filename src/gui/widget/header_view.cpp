// header_view.cpp
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
#include <neolib/timer.hpp>
#include <neolib/destroyable.hpp>
#include <neogfx/app/app.hpp>
#include <neogfx/gui/layout/i_layout.hpp>
#include <neogfx/gui/widget/header_view.hpp>
#include <neogfx/gui/widget/push_button.hpp>

namespace neogfx
{
	class header_view::updater : private neolib::callback_timer
	{
	public:
		updater(header_view& aParent) :
			neolib::callback_timer(app::instance(), [this, &aParent](neolib::callback_timer&)
			{
				neolib::destroyable::destroyed_flag destroyed(*this);
				iParent.layout().set_spacing(iParent.separator_width());
				iParent.iSectionWidths.resize(iParent.model().columns());
				while (iParent.layout().item_count() > iParent.model().columns() + 1)
					iParent.layout().remove_item_at(iParent.layout().item_count() - 1);
				while (iParent.layout().item_count() < iParent.model().columns() + 1)
				{
					iParent.layout().add_item(std::make_shared<push_button>("", push_button_style::ItemViewHeader));
				}
				for (std::size_t i = 0; i < iParent.layout().item_count(); ++i)
				{
					push_button& button = iParent.layout().get_widget_at<push_button>(i);
					if (i < iParent.model().columns())
					{
						button.text().set_text(iParent.model().column_heading_text(i));
						button.set_size_policy(iParent.iType == header_view::HorizontalHeader ?
							neogfx::size_policy{neogfx::size_policy::Fixed, neogfx::size_policy::Minimum} :
							neogfx::size_policy{neogfx::size_policy::Minimum, neogfx::size_policy::Fixed});
						button.set_minimum_size(optional_size{});
						button.enable(true);
						aParent.iSink += button.clicked([&aParent, i]()
						{
							aParent.surface().save_mouse_cursor();
							aParent.surface().set_mouse_cursor(mouse_system_cursor::Wait);
							aParent.model().sort_by(i);
							aParent.surface().restore_mouse_cursor();
						}, aParent);
					}
					else
					{
						button.text().set_text(std::string());
						button.set_size_policy(iParent.iType == header_view::HorizontalHeader ? 
							neogfx::size_policy{neogfx::size_policy::Expanding, neogfx::size_policy::Minimum} :
							neogfx::size_policy{neogfx::size_policy::Minimum, neogfx::size_policy::Expanding});
						button.set_minimum_size(size{});
						button.enable(false);
					}
				}
				uint64_t since = app::instance().program_elapsed_ms();
				app::event_processing_context epc(app::instance(), "neogfx::header_view::updater");
				for (uint32_t c = 0; c < 1000 && iRow < iParent.model().rows(); ++c, ++iRow)
				{
					iParent.update_from_row(iRow, false);
					if (c % 25 == 0 && app::instance().program_elapsed_ms() - since > 20)
					{
						iParent.iOwner.header_view_updated(iParent);
						app::instance().process_events(epc);
						if (destroyed || iParent.surface().destroyed())
							return;
						since = app::instance().program_elapsed_ms();
					}
				}
				if (iRow == iParent.model().rows())
					iParent.iOwner.header_view_updated(iParent);
				else
					again();
			}, 10),
			iParent(aParent),
			iRow(0)
		{
		}
		~updater()
		{
			cancel();
		}
	private:
		header_view& iParent;
		uint32_t iRow;
	};

	header_view::header_view(i_owner& aOwner, type_e aType) :
		splitter(aType == HorizontalHeader ? HorizontalSplitter : VerticalSplitter),
		iOwner(aOwner),
		iType(aType),
		iBatchUpdatesInProgress(0)
	{
	}

	header_view::header_view(i_widget& aParent, i_owner& aOwner, type_e aType) :
		splitter(aParent, aType == HorizontalHeader ? HorizontalSplitter : VerticalSplitter),
		iOwner(aOwner),
		iType(aType),
		iBatchUpdatesInProgress(0)
	{
	}

	header_view::header_view(i_layout& aLayout, i_owner& aOwner, type_e aType) :
		splitter(aLayout, aType == HorizontalHeader ? HorizontalSplitter : VerticalSplitter),
		iOwner(aOwner),
		iType(aType),
		iBatchUpdatesInProgress(0)
	{
	}

	header_view::~header_view()
	{
		if (has_model())
			model().unsubscribe(*this);
	}

	bool header_view::has_model() const
	{
		if (iModel)
			return true;
		else
			return false;
	}
	
	const i_item_model& header_view::model() const
	{
		return *iModel;
	}

	i_item_model& header_view::model()
	{
		return *iModel;
	}

	void header_view::set_model(i_item_model& aModel)
	{
		if (has_model())
			model().unsubscribe(*this);
		iModel = std::shared_ptr<i_item_model>(std::shared_ptr<i_item_model>(), &aModel);
		iSectionWidths.resize(model().columns());
		if (has_model())
			model().subscribe(*this);
		if (has_presentation_model())
			presentation_model().set_item_model(aModel);
		iUpdater.reset();
		iUpdater.reset(new updater(*this));
		update();
	}

	void header_view::set_model(std::shared_ptr<i_item_model> aModel)
	{
		if (has_model())
			model().unsubscribe(*this);
		iModel = aModel;
		iSectionWidths.resize(model().columns());
		if (has_model())
			model().subscribe(*this);
		if (has_presentation_model())
			presentation_model().set_item_model(*aModel);
		iUpdater.reset();
		iUpdater.reset(new updater(*this));
		update();
	}

	bool header_view::has_presentation_model() const
	{
		if (iPresentationModel)
			return true;
		else
			return false;
	}

	const i_item_presentation_model& header_view::presentation_model() const
	{
		return *iPresentationModel;
	}

	i_item_presentation_model& header_view::presentation_model()
	{
		return *iPresentationModel;
	}

	void header_view::set_presentation_model(i_item_presentation_model& aPresentationModel)
	{
		iPresentationModel = std::shared_ptr<i_item_presentation_model>(std::shared_ptr<i_item_presentation_model>(), &aPresentationModel);
		if (has_model())
			presentation_model().set_item_model(model());
	}

	void header_view::set_presentation_model(std::shared_ptr<i_item_presentation_model> aPresentationModel)
	{
		iPresentationModel = aPresentationModel;
		if (has_presentation_model() && has_model())
			presentation_model().set_item_model(model());
	}

	void header_view::start_batch_update()
	{
		++iBatchUpdatesInProgress;
	}

	void header_view::end_batch_update()
	{
		if (--iBatchUpdatesInProgress == 0)
		{
			iSectionWidths.resize(model().columns());
			iUpdater.reset();
			iUpdater.reset(new updater(*this));
			update();
		}
	}

	void header_view::column_info_changed(const i_item_model&, item_model_index::value_type)
	{
		if (iBatchUpdatesInProgress)
			return;
		iSectionWidths.resize(model().columns());
		iUpdater.reset();
		iUpdater.reset(new updater(*this));
	}

	void header_view::item_added(const i_item_model&, const item_model_index&)
	{
		if (iBatchUpdatesInProgress)
			return;
		iSectionWidths.resize(model().columns());
		iUpdater.reset();
		iUpdater.reset(new updater(*this));
	}

	void header_view::item_changed(const i_item_model&, const item_model_index&)
	{
		if (iBatchUpdatesInProgress)
			return;
		iSectionWidths.resize(model().columns());
		iUpdater.reset();
		iUpdater.reset(new updater(*this));
	}

	void header_view::item_removed(const i_item_model&, const item_model_index&)
	{
		if (iBatchUpdatesInProgress)
			return;
		iSectionWidths.resize(model().columns());
		iUpdater.reset();
		iUpdater.reset(new updater(*this));
	}

	void header_view::items_sorted(const i_item_model&)
	{
		iUpdater.reset();
		iUpdater.reset(new updater(*this));
	}

	void header_view::model_destroyed(const i_item_model&)
	{
		iModel.reset();
	}

	dimension header_view::separator_width() const
	{
		if (iSeparatorWidth != boost::none)
			return units_converter(*this).from_device_units(*iSeparatorWidth);
		else
		{
			dimension millimetre = as_units(*this, UnitsMillimetres, 1.0);
			return units_converter(*this).from_device_units(std::ceil(units_converter(*this).to_device_units(millimetre)));
		}
	}

	void header_view::set_separator_width(const optional_dimension& aWidth)
	{
		if (aWidth != boost::none)
			iSeparatorWidth = std::ceil(units_converter(*this).to_device_units(*aWidth));
		else
			iSeparatorWidth = boost::none;
	}

	uint32_t header_view::section_count() const
	{
		return iSectionWidths.size();
	}

	dimension header_view::section_width(uint32_t aSectionIndex) const
	{
		return units_converter(*this).from_device_units(iSectionWidths[aSectionIndex].first != boost::none ? 
			*iSectionWidths[aSectionIndex].first : 
			iSectionWidths[aSectionIndex].second);
	}

	dimension header_view::total_width() const
	{
		dimension result = 0.0;
		for (uint32_t col = 0; col < model().columns(); ++col)
		{
			if (col != 0)
				result += separator_width();
			result += section_width(col);
		}
		return result;
	}

	bool header_view::can_defer_layout() const
	{
		return true;
	}

	bool header_view::is_managing_layout() const
	{
		return true;
	}

	void header_view::panes_resized()
	{
		for (uint32_t col = 0; col < model().columns(); ++col)
		{
			dimension oldSectionWidth = section_width(col);
			dimension newSectionWidth = layout().get_widget_at(col).extents().cx;
			if (newSectionWidth != oldSectionWidth)
				iSectionWidths[col].first = newSectionWidth;
		}
		iOwner.header_view_updated(*this);
	}

	void header_view::reset_pane_sizes_requested(const boost::optional<uint32_t>& aPane)
	{
		for (uint32_t col = 0; col < model().columns(); ++col)
		{
			if (aPane != boost::none && *aPane != col)
				continue;
			iSectionWidths[col].first = boost::none;
			layout().get_widget_at(col).set_fixed_size(size(std::max(section_width(col), layout().spacing().cx * 3.0), layout().get_widget_at(col).minimum_size().cy), false);
		}
		layout_items();
		iOwner.header_view_updated(*this);
	}

	void header_view::update_from_row(uint32_t aRow, bool aUpdateOwner)
	{
		graphics_context gc(*this);
		bool updated = false;
		for (uint32_t col = 0; col < model().columns(item_model_index(aRow)); ++col)
		{
			dimension headingWidth = model().column_heading_extents(col, gc).cx + iOwner.cell_margins().size().cx * 2.0;
			dimension cellWidth = presentation_model().cell_extents(item_model_index(aRow, col), gc).cx + iOwner.cell_margins().size().cx * 2.0;
			dimension oldSectionWidth = iSectionWidths[col].second;
			iSectionWidths[col].second = std::max(iSectionWidths[col].second, units_converter(*this).to_device_units(std::max(headingWidth, cellWidth)));
			if (section_width(col) != oldSectionWidth || layout().get_widget_at(col).minimum_size().cx != section_width(col))
			{
				layout().get_widget_at(col).set_fixed_size(size(std::max(section_width(col), layout().spacing().cx * 3.0), layout().get_widget_at(col).minimum_size().cy));
				updated = true;
			}
		}
		if (updated)
			layout_items();
		if (aUpdateOwner)
			iOwner.header_view_updated(*this);
	}
}