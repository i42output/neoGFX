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
#include <neogfx/gui/window/context_menu.hpp>

namespace neogfx
{
	class header_view::updater : private neolib::callback_timer
	{
	public:
		updater(header_view& aParent) :
			neolib::callback_timer{ app::instance(), [this, &aParent](neolib::callback_timer&)
			{
				neolib::destroyable::destroyed_flag destroyed(*this);
				if (iRow == 0)
				{
					aParent.layout().set_spacing(aParent.separator_width(), false);
					aParent.iSectionWidths.resize(aParent.presentation_model().columns());
					for (auto& sw : aParent.iSectionWidths)
						sw.second = 0.0;
					while (aParent.layout().item_count() > aParent.presentation_model().columns() + (aParent.expand_last_column() ? 0 : 1))
						aParent.layout().remove_item_at(aParent.layout().item_count() - 1);
					while (aParent.layout().item_count() < aParent.presentation_model().columns() + (aParent.expand_last_column() ? 0 : 1))
					{
						aParent.layout().add_item(std::make_shared<push_button>("", push_button_style::ItemViewHeader));
					}
					if (aParent.iButtonSinks.size() < aParent.layout().item_count())
						aParent.iButtonSinks.resize(aParent.layout().item_count());
					for (std::size_t i = 0; i < aParent.layout().item_count(); ++i)
					{
						push_button& button = aParent.layout().get_widget_at<push_button>(i);
						if (i == 0)
						{
							auto m = button.margins();
							m.left = aParent.separator_width() / 2.0 + 1.0;
							button.set_margins(m);
						}
						if (i < aParent.presentation_model().columns())
						{
							button.text().set_text(aParent.presentation_model().column_heading_text(i));
							if (!aParent.expand_last_column() || i != aParent.presentation_model().columns() - 1)
								button.set_size_policy(aParent.iType == header_view::HorizontalHeader ?
									neogfx::size_policy{ neogfx::size_policy::Fixed, neogfx::size_policy::Expanding } :
									neogfx::size_policy{ neogfx::size_policy::Expanding, neogfx::size_policy::Fixed });
							else
								button.set_size_policy(aParent.iType == header_view::HorizontalHeader ?
									neogfx::size_policy{ neogfx::size_policy::Expanding, neogfx::size_policy::Minimum } :
									neogfx::size_policy{ neogfx::size_policy::Minimum, neogfx::size_policy::Expanding });
							button.set_minimum_size(optional_size{});
							button.set_maximum_size(optional_size{});
							button.enable(true);
							aParent.iButtonSinks[i][0] = button.clicked([&aParent, i]()
							{
								aParent.surface().save_mouse_cursor();
								aParent.surface().set_mouse_cursor(mouse_system_cursor::Wait);
								aParent.presentation_model().sort_by(i);
								aParent.surface().restore_mouse_cursor();
							}, aParent);
							aParent.iButtonSinks[i][1] = button.right_clicked([&aParent, i]()
							{
								context_menu menu{ aParent, aParent.surface().mouse_position() + aParent.surface().surface_position() };
								action sortAscending{ "Sort Ascending" };
								action sortDescending{ "Sort Descending" };
								action applySort{ "Apply Sort" };
								action resetSort{ "Reset Sort" };
								menu.menu().add_action(sortAscending).set_checkable(true);
								menu.menu().add_action(sortDescending).set_checkable(true);
								menu.menu().add_action(applySort);
								menu.menu().add_action(resetSort);
								if (aParent.presentation_model().sorting_by() != boost::none)
								{
									auto const& sortingBy = *aParent.presentation_model().sorting_by();
									if (sortingBy.first == i)
									{
										if (sortingBy.second == i_item_presentation_model::SortAscending)
											sortAscending.check();
										else if (sortingBy.second == i_item_presentation_model::SortDescending)
											sortDescending.check();
									}
								}
								sortAscending.checked([&aParent, &sortDescending, i]()
								{
									aParent.presentation_model().sort_by(i, i_item_presentation_model::SortAscending);
									sortDescending.uncheck();
								});
								sortDescending.checked([&aParent, &sortAscending, i]()
								{
									aParent.presentation_model().sort_by(i, i_item_presentation_model::SortDescending);
									sortAscending.uncheck();
								});
								resetSort.triggered([&aParent]()
								{
									aParent.presentation_model().reset_sort();
								});
								menu.exec();
							}, aParent);
						}
						else if (!aParent.expand_last_column())
						{
							button.text().set_text(std::string());
							button.set_size_policy(aParent.iType == header_view::HorizontalHeader ?
								neogfx::size_policy{ neogfx::size_policy::Expanding, neogfx::size_policy::Minimum } :
								neogfx::size_policy{ neogfx::size_policy::Minimum, neogfx::size_policy::Expanding });
							button.set_minimum_size(size{});
							button.enable(false);
						}
					}
				}
				uint64_t since = app::instance().program_elapsed_ms();
				app::event_processing_context epc(app::instance(), "neogfx::header_view::updater");
				graphics_context gc{ aParent };
				for (uint32_t c = 0; c < 1000 && iRow < aParent.presentation_model().rows(); ++c, ++iRow)
				{
					aParent.update_from_row(iRow, gc);
					if (c % 25 == 0 && app::instance().program_elapsed_ms() - since > 20)
					{
						aParent.iOwner.header_view_updated(aParent, header_view_update_reason::FullUpdate);
						app::instance().process_events(epc);
						if (destroyed || aParent.surface().destroyed())
							return;
						since = app::instance().program_elapsed_ms();
					}
				}
				if (iRow == aParent.presentation_model().rows())
				{
					aParent.iOwner.header_view_updated(aParent, header_view_update_reason::FullUpdate);
				}
				else
					again();
			}, 10 },
			iRow{ 0 }
		{
		}
		~updater()
		{
			cancel();
		}
		uint32_t iRow;
	};

	header_view::header_view(i_owner& aOwner, type_e aType) :
		splitter{ aType == HorizontalHeader ? HorizontalSplitter : VerticalSplitter },
		iOwner{ aOwner },
		iType{ aType },
		iExpandLastColumn{ false }
	{
		iSink += app::instance().current_style_changed([this]()
		{
			iUpdater.reset();
			iUpdater.reset(new updater(*this));
		});
	}

	header_view::header_view(i_widget& aParent, i_owner& aOwner, type_e aType) :
		splitter{ aParent, aType == HorizontalHeader ? HorizontalSplitter : VerticalSplitter },
		iOwner{ aOwner },
		iType{ aType },
		iExpandLastColumn{ false }
	{
		iSink += app::instance().current_style_changed([this]()
		{
			iUpdater.reset();
			iUpdater.reset(new updater(*this));
		});
	}

	header_view::header_view(i_layout& aLayout, i_owner& aOwner, type_e aType) :
		splitter{ aLayout, aType == HorizontalHeader ? HorizontalSplitter : VerticalSplitter },
		iOwner{ aOwner },
		iType{ aType },
		iExpandLastColumn{ false }
	{
		iSink += app::instance().current_style_changed([this]()
		{
			iUpdater.reset();
			iUpdater.reset(new updater(*this));
		});
	}

	header_view::~header_view()
	{
		if (has_presentation_model())
			presentation_model().unsubscribe(*this);
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
			presentation_model().unsubscribe(*this);
		iModel = std::shared_ptr<i_item_model>(std::shared_ptr<i_item_model>(), &aModel);
		iSectionWidths.resize(presentation_model().columns());
		if (has_presentation_model())
			presentation_model().set_item_model(aModel);
		iUpdater.reset();
		iUpdater.reset(new updater(*this));
		update();
	}

	void header_view::set_model(std::shared_ptr<i_item_model> aModel)
	{
		if (has_model())
			presentation_model().unsubscribe(*this);
		iModel = aModel;
		iSectionWidths.resize(presentation_model().columns());
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
		if (has_presentation_model())
			presentation_model().unsubscribe(*this);
		iPresentationModel = std::shared_ptr<i_item_presentation_model>(std::shared_ptr<i_item_presentation_model>(), &aPresentationModel);
		presentation_model().subscribe(*this);
		if (has_model())
			presentation_model().set_item_model(model());
	}

	void header_view::set_presentation_model(std::shared_ptr<i_item_presentation_model> aPresentationModel)
	{
		if (has_presentation_model())
			presentation_model().unsubscribe(*this);
		iPresentationModel = aPresentationModel;
		if (has_presentation_model())
		{
			presentation_model().subscribe(*this);
			if (has_model())
				presentation_model().set_item_model(model());
		}
	}

	bool header_view::expand_last_column() const
	{
		return iExpandLastColumn;
	}

	void header_view::set_expand_last_column(bool aExpandLastColumn)
	{
		if (iExpandLastColumn != aExpandLastColumn)
		{
			iExpandLastColumn = aExpandLastColumn;
			iUpdater.reset();
			iUpdater.reset(new updater(*this));
		}
	}

	void header_view::column_info_changed(const i_item_presentation_model&, item_presentation_model_index::column_type)
	{
		iSectionWidths.resize(presentation_model().columns());
		iUpdater.reset();
		iUpdater.reset(new updater(*this));
	}

	void header_view::item_model_changed(const i_item_presentation_model&, const i_item_model&)
	{
		iSectionWidths.resize(presentation_model().columns());
		iUpdater.reset();
		iUpdater.reset(new updater(*this));
	}

	void header_view::item_added(const i_item_presentation_model&, const item_presentation_model_index&)
	{
		iSectionWidths.resize(presentation_model().columns());
		iUpdater.reset();
		iUpdater.reset(new updater(*this));
	}

	void header_view::item_changed(const i_item_presentation_model&, const item_presentation_model_index&)
	{
		iSectionWidths.resize(presentation_model().columns());
		iUpdater.reset();
		iUpdater.reset(new updater(*this));
	}

	void header_view::item_removed(const i_item_presentation_model&, const item_presentation_model_index&)
	{
		iSectionWidths.resize(presentation_model().columns());
		iUpdater.reset();
		iUpdater.reset(new updater(*this));
	}

	void header_view::items_sorting(const i_item_presentation_model&)
	{
	}

	void header_view::items_sorted(const i_item_presentation_model&)
	{
		iUpdater.reset();
		iUpdater.reset(new updater(*this));
	}

	void header_view::model_destroyed(const i_item_presentation_model&)
	{
		iModel.reset();
	}

	dimension header_view::separator_width() const
	{
		if (iSeparatorWidth != boost::none)
			return units_converter(*this).from_device_units(*iSeparatorWidth);
		else if (has_presentation_model())
			return presentation_model().cell_spacing(*this).cx;
		else
		{
			dimension millimetre = as_units(*this, units::Millimetres, 1.0);
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

	dimension header_view::section_width(uint32_t aSectionIndex, bool aForHeaderButton) const
	{
		if (!expand_last_column() || aSectionIndex != iSectionWidths.size() - 1)
		{
			auto result = units_converter(*this).from_device_units(iSectionWidths[aSectionIndex].first != boost::none ?
				*iSectionWidths[aSectionIndex].first :
				iSectionWidths[aSectionIndex].second);
			if (aForHeaderButton && aSectionIndex == 0)
				result += presentation_model().cell_spacing(*this).cx / 2.0;
			return result;
		}
		else
			return layout().get_widget_at(aSectionIndex).extents().cx;
	}

	dimension header_view::total_width() const
	{
		if (!has_presentation_model())
			return 0.0;
		dimension result = 0.0;
		for (uint32_t col = 0; col < presentation_model().columns(); ++col)
		{
			if (col != 0)
				result += separator_width();
			else
				result += separator_width() / 2.0;
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
		for (uint32_t col = 0; col < presentation_model().columns(); ++col)
		{
			dimension oldSectionWidth = section_width(col);
			dimension newSectionWidth = layout().get_widget_at(col).extents().cx;
			if (col == 0)
				newSectionWidth -= presentation_model().cell_spacing(*this).cx / 2.0;
			if (newSectionWidth != oldSectionWidth)
				iSectionWidths[col].first = newSectionWidth;
		}
		layout_items();
		iOwner.header_view_updated(*this, header_view_update_reason::PanesResized);
	}

	void header_view::reset_pane_sizes_requested(const boost::optional<uint32_t>& aPane)
	{
		for (uint32_t col = 0; col < presentation_model().columns(); ++col)
		{
			if (aPane != boost::none && *aPane != col)
				continue;
			iSectionWidths[col].first = boost::none;
			layout().get_widget_at(col).set_minimum_size(size(std::max(section_width(col), layout().spacing().cx * 3.0), layout().get_widget_at(col).minimum_size().cy), false);
		}
		layout_items();
		iOwner.header_view_updated(*this, header_view_update_reason::PanesResized);
	}

	void header_view::update_from_row(uint32_t aRow, graphics_context& aGc)
	{
		bool updated = false;
		for (uint32_t col = 0; col < presentation_model().columns(item_presentation_model_index{ aRow }); ++col)
		{
			dimension headingWidth = presentation_model().column_heading_extents(col, aGc).cx + presentation_model().cell_margins(*this).size().cx * 2.0;
			dimension cellWidth = presentation_model().cell_extents(item_presentation_model_index{ aRow, col }, aGc).cx + presentation_model().cell_margins(*this).size().cx * 2.0;
			dimension oldSectionWidth = iSectionWidths[col].second;
			iSectionWidths[col].second = std::max(iSectionWidths[col].second, units_converter(*this).to_device_units(std::max(headingWidth, cellWidth)));
			if (section_width(col) != oldSectionWidth || layout().get_widget_at(col).minimum_size().cx != section_width(col, true))
			{
				if (!expand_last_column() || col != presentation_model().columns(item_presentation_model_index{ aRow }) - 1)
					layout().get_widget_at(col).set_minimum_size(size(std::max(section_width(col, true), layout().spacing().cx * 3.0), layout().get_widget_at(col).minimum_size().cy));
				else
					layout().get_widget_at(col).set_minimum_size(size(std::max(iSectionWidths[col].second, layout().spacing().cx * 3.0), layout().get_widget_at(col).minimum_size().cy));
				updated = true;
			}
		}
		if (updated)
			layout_items();
	}
}