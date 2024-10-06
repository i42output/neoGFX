// font_widget.cpp
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

#include <boost/lexical_cast.hpp>

#include <neolib/core/scoped.hpp>

#include <neogfx/app/i_app.hpp>
#include <neogfx/gui/widget/item_presentation_model.hpp>
#include <neogfx/gui/dialog/font_dialog.hpp>
#include <neogfx/gui/widget/font_widget.hpp>

namespace neogfx
{
    namespace
    {
        class picker_presentation_model : public item_presentation_model
        {
        public:
            item_cell_flags column_flags(item_presentation_model_index::value_type aColumn) const override
            {
                return item_presentation_model::column_flags(aColumn) & ~item_cell_flags::Editable;
            }
        public:
            optional_font cell_font(item_presentation_model_index const& aIndex) const override
            {
                return service<i_app>().current_style().font_info().with_size(12.0);
            }
            optional_color cell_color(item_presentation_model_index const& aIndex, color_role aColorRole) const override
            {
                if (aColorRole == color_role::Background && (cell_meta(aIndex).selection & item_cell_selection_flags::Current) == item_cell_selection_flags::Current)
                {
                    auto backgroundColor = service<i_app>().current_style().palette().color(color_role::Void);
                    if (backgroundColor == service<i_app>().current_style().palette().color(color_role::Theme))
                        backgroundColor = backgroundColor.shaded(0x20);
                    return backgroundColor;
                }
                else
                    return item_presentation_model::cell_color(aIndex, aColorRole);
            }
        };

        class family_picker_presentation_model : public picker_presentation_model
        {
        public:
            optional_font cell_font(item_presentation_model_index const& aIndex) const override
            {
                auto modelRow = to_item_model_index(aIndex).row();
                if (iFonts.size() <= modelRow)
                    iFonts.resize(modelRow + 1);
                if (iFonts[modelRow] == std::nullopt)
                {
                    auto& fm = service<i_font_manager>();
                    auto const pointSize = std::max(service<i_app>().current_style().font_info().size(), 12.0);
                    iFonts[modelRow] = font{
                        fm.font_family(modelRow),
                        font_style::Normal,
                        -font{ service<i_app>().current_style().font().with_size(pointSize) }.height() };
                }
                return iFonts[modelRow];
            }
        private:
            mutable std::vector<optional_font> iFonts;
        };

        class style_picker_presentation_model : public picker_presentation_model
        {
        public:
            style_picker_presentation_model(i_item_selection_model& aOurSelectionModel, i_item_selection_model& aFamilyPickerSelectionModel) :
                iOurSelectionModel{ aOurSelectionModel }, iFamilyPickerSelectionModel { aFamilyPickerSelectionModel }
            {
                iSink += iFamilyPickerSelectionModel.current_index_changed([this](const optional_item_presentation_model_index& aCurrentIndex, const optional_item_presentation_model_index& /*aPreviousIndex*/)
                {
                    std::optional<std::string> existingStyle;
                    if (iOurSelectionModel.has_current_index())
                        existingStyle = static_variant_cast<string const&>(item_model().cell_data(to_item_model_index(iOurSelectionModel.current_index())));
                    item_model().clear();
                    iFonts.clear();
                    if (aCurrentIndex != std::nullopt)
                    {
                        auto fontFamilyIndex = iFamilyPickerSelectionModel.presentation_model().to_item_model_index(*aCurrentIndex).row();
                        auto& fm = service<i_font_manager>();
                        auto styleCount = fm.font_style_count(fontFamilyIndex);
                        std::optional<std::uint32_t> matchingStyle;
                        for (std::uint32_t s = 0; s < styleCount; ++s)
                        {
                            item_model().insert_item(item_model().end(), fm.font_style_name(fontFamilyIndex, s));
                            if (existingStyle && *existingStyle == fm.font_style_name(fontFamilyIndex, s))
                                matchingStyle = s;
                        }
                        if (!matchingStyle)
                            iOurSelectionModel.set_current_index(item_presentation_model_index{});
                        else
                            iOurSelectionModel.set_current_index(item_presentation_model_index{ *matchingStyle, 0u });
                    }
                });
            }
        public:
            optional_font cell_font(item_presentation_model_index const& aIndex) const override
            {
                if (!iFamilyPickerSelectionModel.has_current_index())
                    return optional_font{};
                auto familyModelRow = iFamilyPickerSelectionModel.presentation_model().to_item_model_index(iFamilyPickerSelectionModel.current_index()).row();
                auto modelRow = to_item_model_index(aIndex).row();
                if (iFonts.size() <= modelRow)
                    iFonts.resize(modelRow + 1);
                if (iFonts[modelRow] == std::nullopt)
                {
                    auto& fm = service<i_font_manager>();
                    iFonts[modelRow] = font{ 
                        fm.font_family(familyModelRow), 
                        static_variant_cast<string const&>(item_model().cell_data(to_item_model_index(aIndex))), 
                        12.0 };
                }
                return iFonts[modelRow];
            }
        private:
            i_item_selection_model& iOurSelectionModel;
            i_item_selection_model& iFamilyPickerSelectionModel;
            sink iSink;
            mutable std::vector<optional_font> iFonts;
        };
    }

    font_widget::font_picker::font_picker(font_widget& aParent) : 
        button<>{ aParent.iLayout0 },
        iParent{ aParent }
    {
        iParent.SelectionChanged([&]
        {
            iSampleText = std::nullopt;
            update_layout(true, true);
            update();
        });
    }

    size_policy font_widget::font_picker::size_policy() const
    {
        if (has_size_policy())
            return button<>::size_policy();
        return size_constraint::Minimum;
    }

    size font_widget::font_picker::minimum_size(optional_size const& aAvailableSpace) const
    {
        auto const result = internal_spacing().size() + 
            units_converter{ *this }.from_device_units(sample_text().extents()) + 
            units_converter{ *this }.from_device_units(size{ 2.0 * (1.0 + 2.0_dip), 2.0 * (1 + 2.0_dip) });
        return result;
    }

    void font_widget::font_picker::paint(i_graphics_context& aGc) const
    {
        button<>::paint(aGc);

        auto r = client_rect(false);

        aGc.draw_rounded_rect(r, 3.0_dip, pen{ palette_color(color_role::Void), 1.0 });
        r.deflate(2.0_dip, 2.0_dip);
        aGc.draw_rect(r, pen{ palette_color(color_role::Void), 1.0 });
        r.deflate(1.0, 1.0);

        if (iParent.has_format())
            aGc.draw_glyph_text(r.top_left(), sample_text(), iParent.selected_format());
        else
            aGc.draw_glyph_text(r.top_left(), sample_text(), text_color());
    }

    glyph_text const& font_widget::font_picker::sample_text() const
    {
        if (iSampleText == std::nullopt)
        {
            graphics_context gc{ *this, graphics_context::type::Unattached };
            iSampleText = gc.to_glyph_text("AaBbYyZz 123", 
                iParent.selected_font().with_size(
                    service<i_app>().current_style().font_info(font_role::Widget).size()));
        }
        return *iSampleText;
    }

    font_widget::font_widget(const neogfx::font& aCurrentFont, font_widget_style aStyle) :
        framed_widget<>{},
        iUpdating{ false },
        iStyle{ aStyle },
        iCurrentFont{ aCurrentFont },
        iSelectedFont{ aCurrentFont },
        iLayout0{ *this }
    {
        init();
    }

    font_widget::font_widget(const neogfx::font& aCurrentFont, const text_format& aCurrentTextFormat, font_widget_style aStyle) :
        framed_widget<>{},
        iUpdating{ false },
        iStyle{ aStyle },
        iCurrentFont{ aCurrentFont },
        iSelectedFont{ aCurrentFont },
        iCurrentTextFormat{ aCurrentTextFormat },
        iSelectedTextFormat{ aCurrentTextFormat },
        iLayout0{ *this }
    {
        init();
    }

    font_widget::font_widget(i_widget& aParent, const neogfx::font& aCurrentFont, font_widget_style aStyle) :
        framed_widget<>{ aParent },
        iUpdating{ false },
        iStyle{ aStyle },
        iCurrentFont{ aCurrentFont },
        iSelectedFont{ aCurrentFont },
        iLayout0{ *this }
    {
        init();
    }

    font_widget::font_widget(i_widget& aParent, const neogfx::font& aCurrentFont, const text_format& aCurrentTextFormat, font_widget_style aStyle) :
        framed_widget<>{ aParent },
        iUpdating{ false },
        iStyle{ aStyle },
        iCurrentFont{ aCurrentFont },
        iSelectedFont{ aCurrentFont },
        iCurrentTextFormat{ aCurrentTextFormat },
        iSelectedTextFormat{ aCurrentTextFormat },
        iLayout0{ *this }
    {
        init();
    }

    font_widget::font_widget(i_layout& aLayout, const neogfx::font& aCurrentFont, font_widget_style aStyle) :
        framed_widget<>{ aLayout },
        iUpdating{ false },
        iStyle{ aStyle },
        iCurrentFont{ aCurrentFont },
        iSelectedFont{ aCurrentFont },
        iLayout0{ *this }
    {
        init();
    }

    font_widget::font_widget(i_layout& aLayout, const neogfx::font& aCurrentFont, const text_format& aCurrentTextFormat, font_widget_style aStyle) :
        framed_widget<>{ aLayout },
        iUpdating{ false },
        iStyle{ aStyle },
        iCurrentFont{ aCurrentFont },
        iSelectedFont{ aCurrentFont },
        iCurrentTextFormat{ aCurrentTextFormat },
        iSelectedTextFormat{ aCurrentTextFormat },
        iLayout0{ *this }
    {
        init();
    }

    font_widget::~font_widget()
    {
    }

    font font_widget::current_font() const
    {
        return iCurrentFont;
    }

    font font_widget::selected_font() const
    {
        return iSelectedFont;
    }

    bool font_widget::has_format() const
    {
        return iSelectedTextFormat.has_value();
    }

    text_format font_widget::current_format() const
    {
        return iCurrentTextFormat.has_value() ? *iCurrentTextFormat : text_format{};
    }

    text_format font_widget::selected_format() const
    {
        return iSelectedTextFormat.has_value() ? *iSelectedTextFormat : text_format{};
    }
    
    void font_widget::select_font(const neogfx::font& aFont)
    {
        iSelectedFont = aFont;
        update_selected_font(*this);
    }

    void font_widget::select_format(const text_format& aTextFormat)
    {
        iSelectedTextFormat = aTextFormat;
        update_selected_format(*this);
    }

    void font_widget::init()
    {
        if ((iStyle & font_widget_style::Dialog) == font_widget_style::Dialog)
        {
            iFontPicker.emplace(*this);
            iFontPicker->Clicked([&]()
            {
                font_dialog fd{ *this, selected_font(), has_format() ? 
                    optional<text_format>{ selected_format() } : std::nullopt };
                fd.SelectionChanged([&]
                {
                    select_font(fd.selected_font());
                    if (has_format())
                        select_format(fd.selected_format().value());
                });
                if (fd.exec() == dialog_result::Accepted)
                {
                    select_font(fd.selected_font());
                    if (has_format())
                        select_format(fd.selected_format().value());
                }
                else
                {
                    select_font(current_font());
                    if (has_format())
                        select_format(current_format());
                }
            });
        }
        else if ((iStyle & font_widget_style::DropList) == font_widget_style::DropList)
        {
            iFamilyPicker.emplace(iLayout0, drop_list_style::Editable | drop_list_style::NoFilter);
            iStylePicker.emplace(iLayout0, drop_list_style::Editable | drop_list_style::NoFilter);
            iSizePicker.emplace(iLayout0, drop_list_style::Editable | drop_list_style::NoFilter);

            iFamilyPicker->set_id("neogfx::font_widget::family_picker"_s);
            iStylePicker->set_id("neogfx::font_widget::style_picker"_s);
            iSizePicker->set_id("neogfx::font_widget::size_picker"_s);

            iFamilyPicker->set_size_policy(neogfx::size_policy{ size_constraint::Fixed, size_constraint::Minimum });
            iStylePicker->set_size_policy(neogfx::size_policy{ size_constraint::Fixed, size_constraint::Minimum });
            iSizePicker->set_size_policy(neogfx::size_policy{ size_constraint::Fixed, size_constraint::Minimum });
            iFamilyPicker->set_fixed_size(size{ 160.0_dip });
            iStylePicker->set_fixed_size(size{ 128.0_dip });
            iSizePicker->set_fixed_size(size{ 48.0_dip });

            iFamilyPicker->set_presentation_model(make_ref<family_picker_presentation_model>());
            iStylePicker->set_presentation_model(make_ref<style_picker_presentation_model>(iStylePicker->selection_model(), iFamilyPicker->selection_model()));
            iSizePicker->set_presentation_model(make_ref<picker_presentation_model>());

            iFamilyPicker->selection_model().current_index_changed([this](const optional_item_presentation_model_index&, const optional_item_presentation_model_index&)
                {
                    update_selected_font(*iFamilyPicker);
                });

            iStylePicker->selection_model().current_index_changed([this](const optional_item_presentation_model_index&, const optional_item_presentation_model_index&)
                {
                    update_selected_font(*iStylePicker);
                });

            iSizePicker->selection_model().current_index_changed([this](const optional_item_presentation_model_index&, const optional_item_presentation_model_index&)
                {
                    update_selected_font(*iSizePicker);
                });

            iSizePicker->input_widget().text_changed([this]()
                {
                    update_selected_font(*iSizePicker);
                });

            auto& fm = service<i_font_manager>();

            for (std::uint32_t fi = 0; fi < fm.font_family_count(); ++fi)
                iFamilyPicker->model().insert_item(item_model_index{ fi }, fm.font_family(fi));
        }

        update_selected_font(*this);
    }

    void font_widget::update_selected_font(const i_widget& aUpdatingWidget)
    {
        if (iUpdating)
            return;
        neolib::scoped_flag sf{ iUpdating };

        auto oldFont = iSelectedFont;
        auto& fm = service<i_font_manager>();

        if ((iStyle & font_widget_style::DropList) == font_widget_style::DropList)
        {
            if (&aUpdatingWidget == this || &aUpdatingWidget == &*iFamilyPicker || &aUpdatingWidget == &*iStylePicker)
            {
                iSizePicker->model().clear();
                if (!iSelectedFont.is_bitmap_font())
                    for (auto sz : { 8, 9, 10, 11, 12, 14, 16, 18, 20, 22, 24, 26, 28, 36, 48, 72 })
                        iSizePicker->model().insert_item(item_model_index{ iSizePicker->model().rows() }, sz);
                else
                    for (std::uint32_t fsi = 0; fsi < iSelectedFont.num_fixed_sizes(); ++fsi)
                        iSizePicker->model().insert_item(item_model_index{ iSizePicker->model().rows() }, iSelectedFont.fixed_size(fsi));
                iSizePicker->input_widget().set_text(string{ boost::lexical_cast<std::string>(iSelectedFont.size()) });
            }
            if (&aUpdatingWidget == this)
            {
                auto family = iFamilyPicker->presentation_model().find_item(iSelectedFont.family_name());
                if (family != std::nullopt)
                    iFamilyPicker->selection_model().set_current_index(*family);
                auto style = iStylePicker->presentation_model().find_item(iSelectedFont.style_name());
                if (style != std::nullopt)
                    iStylePicker->selection_model().set_current_index(*style);
                auto size = iSizePicker->presentation_model().find_item(boost::lexical_cast<std::string>(iSelectedFont.size()));
                if (size != std::nullopt)
                    iSizePicker->selection_model().set_current_index(*size);
                iSizePicker->input_widget().set_text(string{ boost::lexical_cast<std::string>(iSelectedFont.size()) });
            }
            else if (iFamilyPicker->selection_model().has_current_index() && iStylePicker->selection_model().has_current_index())
            {
                auto fontFamilyIndex = iFamilyPicker->presentation_model().to_item_model_index(iFamilyPicker->selection_model().current_index()).row();
                auto fontStyleIndex = iStylePicker->presentation_model().to_item_model_index(iStylePicker->selection_model().current_index()).row();
                auto fontSize = iSelectedFont.size();
                try { fontSize = boost::lexical_cast<double>(iSizePicker->input_widget().text()); }
                catch (...) {}
                fontSize = std::min(std::max(fontSize, 1.0), 1638.0);
                iSelectedFont = neogfx::font{
                    fm.font_family(fontFamilyIndex),
                    fm.font_style_name(fontFamilyIndex, fontStyleIndex),
                    fontSize };
            }
            else
                iSelectedFont = iCurrentFont;
            auto fontSizeIndex = iSizePicker->presentation_model().find_item(boost::lexical_cast<std::string>(static_cast<int>(iSelectedFont.size())));
            if (fontSizeIndex != std::nullopt)
                iSizePicker->selection_model().set_current_index(*fontSizeIndex);
            else
                iSizePicker->selection_model().clear_current_index();

            if (iSelectedFont != oldFont)
                SelectionChanged();
        }
        else
            SelectionChanged();
    }

    void font_widget::update_selected_format(const i_widget& aUpdatingWidget)
    {
        if (iUpdating)
            return;
        neolib::scoped_flag sf{ iUpdating };

        SelectionChanged();
    }
}