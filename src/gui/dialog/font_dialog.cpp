// font_dialog.cpp
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
#include <neolib/raii.hpp>
#include <neogfx/app/app.hpp>
#include <neogfx/gui/widget/item_presentation_model.hpp>
#include <neogfx/gui/dialog/font_dialog.hpp>
#include <neogfx/gui/dialog/message_box.hpp>

namespace neogfx
{
	namespace
	{
		class family_picker_presentation_model : public item_presentation_model
		{
		public:
			optional_font cell_font(const item_presentation_model_index& aIndex) const override
			{
				auto modelRow = to_item_model_index(aIndex).row();
				if (iFonts.size() <= modelRow)
					iFonts.resize(modelRow + 1);
				if (iFonts[modelRow] == std::nullopt)
				{
					auto& fm = service<i_font_manager>::instance();
					iFonts[modelRow] = font{ fm.font_family(modelRow), font::Normal, app::instance().current_style().font_info().size() };
				}
				return iFonts[modelRow];
			}
		public:
			item_cell_editable column_editable(item_presentation_model_index::value_type) const override
			{
				return item_cell_editable::No;
			}
		public:
			optional_colour cell_colour(const item_presentation_model_index& aIndex, item_cell_colour_type aColourType) const override
			{
				if (aColourType == item_cell_colour_type::Background && (cell_meta(aIndex).selection & item_cell_selection_flags::Current) == item_cell_selection_flags::Current)
				{
					auto backgroundColour = app::instance().current_style().palette().colour().dark() ? colour::Black : colour::White;
					if (backgroundColour == app::instance().current_style().palette().colour())
						backgroundColour = backgroundColour.dark() ? backgroundColour.lighter(0x20) : backgroundColour.darker(0x20);
					return backgroundColour;
				}
				else
					return item_presentation_model::cell_colour(aIndex, aColourType);
			}
		private:
			mutable std::vector<optional_font> iFonts;
		};

		class style_picker_presentation_model : public item_presentation_model
		{
		public:
			style_picker_presentation_model(i_item_selection_model& aOurSelectionModel, i_item_selection_model& aFamilyPickerSelectionModel) :
				iOurSelectionModel{ aOurSelectionModel }, iFamilyPickerSelectionModel {	aFamilyPickerSelectionModel	}
			{
				iSink += iFamilyPickerSelectionModel.current_index_changed([this](const optional_item_presentation_model_index& aCurrentIndex, const optional_item_presentation_model_index& /*aPreviousIndex*/)
				{
					item_model().clear();
					iFonts.clear();
					if (aCurrentIndex != std::nullopt)
					{
						auto fontFamilyIndex = iFamilyPickerSelectionModel.presentation_model().to_item_model_index(*aCurrentIndex).row();
						auto& fm = service<i_font_manager>::instance();
						auto styleCount = fm.font_style_count(fontFamilyIndex);
						for (uint32_t s = 0; s < styleCount; ++s)
							item_model().insert_item(item_model().end(), fm.font_style(fontFamilyIndex, s));
						iOurSelectionModel.set_current_index(item_presentation_model_index{});
					}
				});
			}
		public:
			optional_font cell_font(const item_presentation_model_index& aIndex) const override
			{
				if (!iFamilyPickerSelectionModel.has_current_index())
					return optional_font{};
				auto familyModelRow = iFamilyPickerSelectionModel.presentation_model().to_item_model_index(iFamilyPickerSelectionModel.current_index()).row();
				auto modelRow = to_item_model_index(aIndex).row();
				if (iFonts.size() <= modelRow)
					iFonts.resize(modelRow + 1);
				if (iFonts[modelRow] == std::nullopt)
				{
					auto& fm = service<i_font_manager>::instance();
					iFonts[modelRow] = font{ fm.font_family(familyModelRow), static_variant_cast<const std::string&>(item_model().cell_data(to_item_model_index(aIndex))), app::instance().current_style().font_info().size() };
				}
				return iFonts[modelRow];
			}
		public:
			item_cell_editable column_editable(item_presentation_model_index::value_type) const override
			{
				return item_cell_editable::No;
			}
		public:
			optional_colour cell_colour(const item_presentation_model_index& aIndex, item_cell_colour_type aColourType) const override
			{
				if (aColourType == item_cell_colour_type::Background && (cell_meta(aIndex).selection & item_cell_selection_flags::Current) == item_cell_selection_flags::Current)
				{
					auto backgroundColour = app::instance().current_style().palette().colour().dark() ? colour::Black : colour::White;
					if (backgroundColour == app::instance().current_style().palette().colour())
						backgroundColour = backgroundColour.dark() ? backgroundColour.lighter(0x20) : backgroundColour.darker(0x20);
					return backgroundColour;
				}
				else
					return item_presentation_model::cell_colour(aIndex, aColourType);
			}
		private:
			i_item_selection_model& iOurSelectionModel;
			i_item_selection_model& iFamilyPickerSelectionModel;
			sink iSink;
			mutable std::vector<optional_font> iFonts;
		};
	}

	font_dialog::font_dialog(const neogfx::font& aCurrentFont) : 
		dialog{ "Select Font", window_style::Modal | window_style::TitleBar | window_style::Close },
		iUpdating{ false },
		iCurrentFont{ aCurrentFont },
		iSelectedFont{ aCurrentFont },
		iLayout0{ client_layout() },
		iLayout1{ iLayout0 },
		iFamilyLabel{ iLayout1, "Family:" },
		iFamilyPicker{ iLayout1, drop_list::style::Editable | drop_list::style::ListAlwaysVisible | drop_list::style::NoFilter },
		iLayout2{ iLayout0 },
		iLayout3{ iLayout2 },
		iLayout4{ iLayout3 },
		iStyleLabel{ iLayout4, "Style:" },
		iStylePicker{ iLayout4, drop_list::style::Editable | drop_list::style::ListAlwaysVisible | drop_list::style::NoFilter },
		iLayout5{ iLayout3 },
		iSizeLabel{ iLayout5, "Size:" },
		iSizePicker{ iLayout5, drop_list::style::Editable | drop_list::style::ListAlwaysVisible | drop_list::style::NoFilter },
		iSampleBox{ iLayout2, "Sample" },
		iSample{ iSampleBox.with_item_layout<horizontal_layout>(), "AaBbYyZz 123" }
	{
		init();
	}

	font_dialog::font_dialog(i_widget& aParent, const neogfx::font& aCurrentFont) :
		dialog{ aParent, "Select Font", window_style::Modal | window_style::TitleBar | window_style::Close },
		iUpdating{ false },
		iCurrentFont{ aCurrentFont	},
		iSelectedFont{ aCurrentFont },
		iLayout0{ client_layout() },
		iLayout1{ iLayout0 },
		iFamilyLabel{ iLayout1, "Family:" },
		iFamilyPicker{ iLayout1, drop_list::style::Editable | drop_list::style::ListAlwaysVisible | drop_list::style::NoFilter },
		iLayout2{ iLayout0 },
		iLayout3{ iLayout2 },
		iLayout4{ iLayout3 },
		iStyleLabel{ iLayout4, "Style:" },
		iStylePicker{ iLayout4, drop_list::style::Editable | drop_list::style::ListAlwaysVisible | drop_list::style::NoFilter },
		iLayout5{ iLayout3 },
		iSizeLabel{ iLayout5, "Size:" },
		iSizePicker{ iLayout5, drop_list::style::Editable | drop_list::style::ListAlwaysVisible | drop_list::style::NoFilter },
		iSampleBox{ iLayout2, "Sample" },
		iSample{ iSampleBox.with_item_layout<horizontal_layout>(), "AaBbYyZz 123" }
	{
		init();
	}

	font_dialog::~font_dialog()
	{
	}

	font font_dialog::current_font() const
	{
		return iCurrentFont;
	}

	font font_dialog::selected_font() const
	{
		return iSelectedFont;
	}
	
	void font_dialog::select_font(const neogfx::font& aFont)
	{
		iSelectedFont = aFont;
		update_selected_font(*this);
	}

	size font_dialog::minimum_size(const optional_size& aAvailableSpace) const
	{
		auto result = dialog::minimum_size(aAvailableSpace);
		if (dialog::has_minimum_size())
			return result;
		result.cy += dpi_scale(std::min(font().height(), 16.0) * 8.0);
		result.cx += dpi_scale(std::min(font().height(), 16.0) * 6.0);
		return result;
	}

	void font_dialog::init()
	{
		button_box().option_layout().add(std::make_shared<push_button>("Subpixel Rendering..."_t)).clicked([this]()
		{
			message_box::stop(*this, "neoGFX Feature"_t, "Sorry, this neoGFX feature (subpixel rendering settings dialog) has yet to be implemented."_t, standard_button::Ok);
		});

		client_layout().set_size_policy(neogfx::size_policy{ neogfx::size_policy::Expanding, neogfx::size_policy::Maximum });
		iFamilyPicker.set_size_policy(neogfx::size_policy{ neogfx::size_policy::Minimum, neogfx::size_policy::Expanding });
		iStylePicker.set_size_policy(neogfx::size_policy::Expanding);
		iSizePicker.set_size_policy(neogfx::size_policy::Expanding);
		iLayout1.set_weight(neogfx::size{ 0.0, 1.0 });
		iLayout4.set_weight(neogfx::size{ 3.0, 1.0 });
		iLayout5.set_weight(neogfx::size{ 1.0, 1.0 });

		iSampleBox.set_size_policy(neogfx::size_policy::Expanding);
		iSample.set_size_policy(neogfx::size_policy::Expanding);
		iSample.set_minimum_size(dpi_scale(size{ 192.0, 48.0 }));
		iSample.set_maximum_size(dpi_scale(size{ size::max_dimension(), 48.0 }));

		set_standard_layout(16.0);
		button_box().add_button(standard_button::Ok);
		button_box().add_button(standard_button::Cancel);

		iFamilyPicker.model().set_column_read_only(0, true);
		iStylePicker.model().set_column_read_only(0, true);
		iSizePicker.model().set_column_read_only(0, true);

		iFamilyPicker.set_presentation_model(std::make_shared<family_picker_presentation_model>());
		iStylePicker.set_presentation_model(std::make_shared<style_picker_presentation_model>(iStylePicker.selection_model(), iFamilyPicker.selection_model()));

		iFamilyPicker.selection_model().current_index_changed([this](const optional_item_presentation_model_index&, const optional_item_presentation_model_index&)
		{
			update_selected_font(iFamilyPicker);
		});
		iFamilyPicker.selection_changed([this](const optional_item_model_index&)
		{
			update_selected_font(iFamilyPicker);
		});

		iStylePicker.selection_model().current_index_changed([this](const optional_item_presentation_model_index&, const optional_item_presentation_model_index&)
		{
			update_selected_font(iStylePicker);
		});
		iStylePicker.selection_changed([this](const optional_item_model_index&)
		{
			update_selected_font(iStylePicker);
		});

		iSizePicker.selection_changed([this](const optional_item_model_index&)
		{
			update_selected_font(iSizePicker);
		});
		iSizePicker.selection_model().current_index_changed([this](const optional_item_presentation_model_index&, const optional_item_presentation_model_index&)
		{
			update_selected_font(iSizePicker);
		});
		iSizePicker.input_widget().text_changed([this]()
		{
			update_selected_font(iSizePicker);
		});

		auto& fm = service<i_font_manager>::instance();

		for (uint32_t fi = 0; fi < fm.font_family_count(); ++fi)
			iFamilyPicker.model().insert_item(item_model_index{ fi }, fm.font_family(fi));

		centre_on_parent();

		update_selected_font(*this);
	}

	void font_dialog::update_selected_font(const i_widget& aUpdatingWidget)
	{
		if (iUpdating)
			return;
		neolib::scoped_flag sf{ iUpdating };

		auto oldFont = iSelectedFont;
		auto& fm = service<i_font_manager>::instance();
		if (&aUpdatingWidget == this || &aUpdatingWidget == &iFamilyPicker || &aUpdatingWidget == &iStylePicker)
		{
			iSizePicker.model().clear();
			if (!iSelectedFont.is_bitmap_font())
				for (auto sz : { 8, 9, 10, 11, 12, 14, 16, 18, 20, 22, 24, 26, 28, 36, 48, 72 })
					iSizePicker.model().insert_item(item_model_index{ iSizePicker.model().rows() }, sz);
			else
				for (uint32_t fsi = 0; fsi < iSelectedFont.num_fixed_sizes(); ++fsi)
					iSizePicker.model().insert_item(item_model_index{ iSizePicker.model().rows() }, iSelectedFont.fixed_size(fsi));
			iSizePicker.input_widget().set_text(boost::lexical_cast<std::string>(iSelectedFont.size()));
		}
		if (&aUpdatingWidget == this)
		{
			auto family = iFamilyPicker.presentation_model().find_item(iSelectedFont.family_name());
			if (family != std::nullopt)
				iFamilyPicker.selection_model().set_current_index(*family);
			auto style = iStylePicker.presentation_model().find_item(iSelectedFont.style_name());
			if (style != std::nullopt)
				iStylePicker.selection_model().set_current_index(*style);
			auto size = iSizePicker.presentation_model().find_item(boost::lexical_cast<std::string>(iSelectedFont.size()));
			if (size != std::nullopt)
				iSizePicker.selection_model().set_current_index(*size);
			iSizePicker.input_widget().set_text(boost::lexical_cast<std::string>(iSelectedFont.size()));
		}
		else if (iFamilyPicker.selection_model().has_current_index() && iStylePicker.selection_model().has_current_index())
		{
			auto fontFamilyIndex = iFamilyPicker.presentation_model().to_item_model_index(iFamilyPicker.selection_model().current_index()).row();
			auto fontStyleIndex = iFamilyPicker.presentation_model().to_item_model_index(iStylePicker.selection_model().current_index()).row();
			auto fontSize = iSelectedFont.size();
			try { fontSize = boost::lexical_cast<double>(iSizePicker.input_widget().text()); } catch (...) {}
			fontSize = std::min(std::max(fontSize, 1.0), 1638.0);
			iSelectedFont = neogfx::font{ 
				fm.font_family(fontFamilyIndex), 
				fm.font_style(fontFamilyIndex, fontStyleIndex), 
				fontSize };
		}
		else
			iSelectedFont = iCurrentFont;
		auto fontSizeIndex = iSizePicker.presentation_model().find_item(boost::lexical_cast<std::string>(static_cast<int>(iSelectedFont.size())));
		if (fontSizeIndex != std::nullopt)
			iSizePicker.selection_model().set_current_index(*fontSizeIndex);
		else
			iSizePicker.selection_model().unset_current_index();
		iSample.set_font(iSelectedFont);
		if (iSelectedFont != oldFont)
			selection_changed.trigger();
	}
}