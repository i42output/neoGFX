// font_dialog.cpp
/*
  neogfx C++ GUI Library
  Copyright (c) 2015-present, Leigh Johnston.  All Rights Reserved.
  
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
#include <neogfx/app/app.hpp>
#include <neogfx/gui/widget/item_presentation_model.hpp>
#include <neogfx/gui/dialog/font_dialog.hpp>

namespace neogfx
{
	font_dialog::font_dialog(const neogfx::font& aCurrentFont) : 
		dialog{ "Select Font", window_style::Modal | window_style::TitleBar | window_style::Close },
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
		update_widgets(*this);
	}

	class family_picker_presentation_model : public item_presentation_model
	{
	public:
		optional_font cell_font(const item_presentation_model_index& aIndex) const override
		{
			auto modelRow = to_item_model_index(aIndex).row();
			if (iFonts.size() <= modelRow)
				iFonts.resize(modelRow + 1);
			if (iFonts[modelRow] == boost::none)
			{
				auto& fm = app::instance().rendering_engine().font_manager();
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

	size font_dialog::minimum_size(const optional_size& aAvailableSpace) const
	{
		auto result = dialog::minimum_size(aAvailableSpace);
		if (dialog::has_minimum_size())
			return result;
		result.cy += font().height() * 6.0;
		return result;
	}

	void font_dialog::init()
	{
		client_layout().set_size_policy(neogfx::size_policy{ neogfx::size_policy::Minimum, neogfx::size_policy::Maximum });

		iSample.set_fixed_size(dpi_scale(size{ 192.0, 48.0 }));

		set_standard_layout(16.0);
		button_box().add_button(standard_button::Ok);
		button_box().add_button(standard_button::Cancel);

		iFamilyPicker.set_presentation_model(std::make_shared<family_picker_presentation_model>());

		iFamilyPicker.selection_model().current_index_changed([this](const optional_item_presentation_model_index&, const optional_item_presentation_model_index&)
		{
			update_widgets(iFamilyPicker);
		});

		auto& fm = app::instance().rendering_engine().font_manager();

		uint32_t familyPickerCurrentIndex = 0;
		for (uint32_t fi = 0; fi < fm.font_family_count(); ++fi)
		{
			iFamilyPicker.model().insert_item(item_model_index{ fi }, fm.font_family(fi));
			if (fm.font_family(fi) == app::instance().current_style().font_info().family_name())
				familyPickerCurrentIndex = fi;
		}
		iFamilyPicker.model().set_column_read_only(0, true);

		uint32_t sizePickerCurrentIndex = 0;
		for (auto sz : { 8, 9, 10, 11, 12, 14, 16, 18, 20, 22, 24, 26, 28, 36, 48, 72 })
		{
			if (sz == static_cast<int>(app::instance().current_style().font_info().size()))
				sizePickerCurrentIndex = iSizePicker.model().rows();
			iSizePicker.model().insert_item(item_model_index{ iSizePicker.model().rows() }, sz);
		}
		iSizePicker.input_widget().set_text(boost::lexical_cast<std::string>(app::instance().current_style().font_info().size()));

		centre_on_parent();

		iFamilyPicker.selection_model().set_current_index(iFamilyPicker.presentation_model().from_item_model_index(familyPickerCurrentIndex));
		iSizePicker.selection_model().set_current_index(iSizePicker.presentation_model().from_item_model_index(sizePickerCurrentIndex));
	}

	void font_dialog::update_widgets(const i_widget& aUpdatingWidget)
	{
		auto& fm = app::instance().rendering_engine().font_manager();
		auto fontFamilyIndex = iFamilyPicker.presentation_model().to_item_model_index(iFamilyPicker.selection_model().current_index()).row();
		if (&aUpdatingWidget == &iFamilyPicker)
		{
			auto styleCount = fm.font_style_count(fontFamilyIndex);
		}
		optional_font selectedFont;
		if (iFamilyPicker.selection_model().has_current_index() && 
			iStylePicker.selection_model().has_current_index())
			selectedFont = neogfx::font{ fm.font_family(fontFamilyIndex), font::Normal, boost::lexical_cast<double>(iSizePicker.input_widget().text()) };
		iSample.set_font(selectedFont);
	}
}