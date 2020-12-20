// font_dialog.cpp
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
#include <neogfx/gui/widget/item_presentation_model.hpp>
#include <neogfx/gui/dialog/font_dialog.hpp>
#include <neogfx/gui/dialog/message_box.hpp>

namespace neogfx
{
    void draw_alpha_background(i_graphics_context& aGc, const rect& aRect, dimension aAlphaPatternSize);

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
                    iFonts[modelRow] = font{ fm.font_family(modelRow), font_style::Normal, std::max(service<i_app>().current_style().font_info().size(), 12.0) };
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
                        existingStyle = static_variant_cast<std::string const&>(item_model().cell_data(to_item_model_index(iOurSelectionModel.current_index())));
                    item_model().clear();
                    iFonts.clear();
                    if (aCurrentIndex != std::nullopt)
                    {
                        auto fontFamilyIndex = iFamilyPickerSelectionModel.presentation_model().to_item_model_index(*aCurrentIndex).row();
                        auto& fm = service<i_font_manager>();
                        auto styleCount = fm.font_style_count(fontFamilyIndex);
                        std::optional<uint32_t> matchingStyle;
                        for (uint32_t s = 0; s < styleCount; ++s)
                        {
                            item_model().insert_item(item_model().end(), fm.font_style(fontFamilyIndex, s).to_std_string());
                            if (existingStyle && *existingStyle == fm.font_style(fontFamilyIndex, s))
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
                        static_variant_cast<std::string const&>(item_model().cell_data(to_item_model_index(aIndex))), 
                        std::max(service<i_app>().current_style().font_info().size(), 12.0) };
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

    font_dialog::font_dialog(const neogfx::font& aCurrentFont, std::optional<text_appearance> const& aCurrentAppearance) :
        dialog{ "Select Font", window_style::Dialog | window_style::Modal | window_style::TitleBar | window_style::Close },
        iUpdating{ false },
        iCurrentFont{ aCurrentFont },
        iSelectedFont{ aCurrentFont },
        iCurrentAppearance{ aCurrentAppearance },
        iSelectedAppearance{ aCurrentAppearance },
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

    font_dialog::font_dialog(i_widget& aParent, const neogfx::font& aCurrentFont, std::optional<text_appearance> const& aCurrentAppearance) :
        dialog{ aParent, "Select Font", window_style::Dialog | window_style::Modal | window_style::TitleBar | window_style::Close },
        iUpdating{ false },
        iCurrentFont{ aCurrentFont },
        iSelectedFont{ aCurrentFont },
        iCurrentAppearance{ aCurrentAppearance },
        iSelectedAppearance{ aCurrentAppearance },
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
        iEffects{ client_layout() },
        iLayoutEffects{ iEffects, neogfx::alignment::Top },
        iInkBox{ iLayoutEffects, "Ink" },
        iInkColor{ iInkBox.with_item_layout<vertical_layout>(), "Color" },
        iInkGradient{ iInkBox.item_layout(), "Gradient" },
        iPaperBox{ iLayoutEffects, "Paper" },
        iPaperColor{ iPaperBox.with_item_layout<vertical_layout>(), "Color" },
        iPaperGradient{ iPaperBox.item_layout(), "Gradient" },
        iTextEffectsBox{ iLayoutEffects, "Text Effects" },
        iTextEffectsOutline{ iTextEffectsBox.item_layout(), "Outline" },
        iTextEffectsShadow{ iTextEffectsBox.item_layout(), "Shadow" },
        iTextEffectsGlow{ iTextEffectsBox.item_layout(), "Glow" },
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

    std::optional<text_appearance> const& font_dialog::current_appearance() const
    {
        return iCurrentAppearance;
    }

    std::optional<text_appearance> const& font_dialog::selected_appearance() const
    {
        return iSelectedAppearance;
    }
    
    void font_dialog::select_font(const neogfx::font& aFont)
    {
        iSelectedFont = aFont;
        update_selected_font(*this);
    }

    void font_dialog::set_default_ink(const std::optional<color>& aColor)
    {
        iDefaultInk = aColor;
    }

    void font_dialog::set_default_paper(const std::optional<color>& aColor)
    {
        iDefaultPaper = aColor;
    }

    size font_dialog::minimum_size(optional_size const& aAvailableSpace) const
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
        iEffects.show(iSelectedAppearance != std::nullopt);
        iInkBox.set_checkable(true, true);
        iPaperBox.set_checkable(true, true);
        iTextEffectsBox.set_checkable(true, true);
        if (iSelectedAppearance)
        {
            if (iSelectedAppearance->ink() != neolib::none)
            {
                iInkBox.check_box().check();
                if (std::holds_alternative<color>(iSelectedAppearance->ink()))
                    iInkColor.check();
                else if (std::holds_alternative<gradient>(iSelectedAppearance->ink()))
                    iInkGradient.check();
            }
            if (iSelectedAppearance->paper() && iSelectedAppearance->paper() != neolib::none)
            {
                iPaperBox.check_box().check();
                if (std::holds_alternative<color>(*iSelectedAppearance->paper()))
                    iPaperColor.check();
                else if (std::holds_alternative<gradient>(*iSelectedAppearance->paper()))
                    iPaperGradient.check();
            }
            if (iSelectedAppearance->effect())
                iTextEffectsBox.check_box().check();
        }
        iSink += iInkBox.check_box().Checked([&]() { update_widgets(); });
        iSink += iInkBox.check_box().Unchecked([&]() { update_widgets(); });
        iSink += iPaperBox.check_box().Checked([&]() { update_widgets(); });
        iSink += iPaperBox.check_box().Unchecked([&]() { update_widgets(); });
        iSink += iTextEffectsBox.check_box().Checked([&]() { update_widgets(); });
        iSink += iTextEffectsBox.check_box().Unchecked([&]() { update_widgets(); });
        iSink += iInkColor.Checked([&]() { update_widgets(); });
        iSink += iInkGradient.Checked([&]() { update_widgets(); });
        iSink += iPaperColor.Checked([&]() { update_widgets(); });
        iSink += iPaperGradient.Checked([&]() { update_widgets(); });
        update_widgets();

        if (iSelectedAppearance)
        {
            iSink += iSample.Painting([&](i_graphics_context& aGc)
            {
                scoped_opacity so{ aGc, 0.25 };
                draw_alpha_background(aGc, iSample.client_rect(), dpi_scale(4.0));
            });
        }

        auto subpixelRendering = make_ref<push_button>("Subpixel Rendering..."_t);
        subpixelRendering->enable(false);
        button_box().option_layout().add(subpixelRendering).clicked([this]()
        {
            message_box::stop(*this, "neoGFX Feature"_t, "Sorry, this neoGFX feature (subpixel rendering settings dialog) has yet to be implemented."_t, standard_button::Ok);
        });

        iFamilyPicker.set_size_policy(neogfx::size_policy{ size_constraint::Minimum, size_constraint::Expanding });
        iStylePicker.set_size_policy(size_constraint::Expanding);
        iSizePicker.set_size_policy(size_constraint::Expanding);
        iLayout1.set_weight(neogfx::size{ 0.0, 1.0 });
        iLayout4.set_weight(neogfx::size{ 3.0, 1.0 });
        iLayout5.set_weight(neogfx::size{ 1.0, 1.0 });

        iSampleBox.set_size_policy(size_constraint::Expanding);
        iSample.set_size_policy(size_constraint::Expanding);
        iSample.set_minimum_size(size{ 192.0_dip, 48.0_dip });
        iSample.set_maximum_size(size{ size::max_dimension(), 48.0_dip });

        button_box().add_button(standard_button::Ok);
        button_box().add_button(standard_button::Cancel);

        iFamilyPicker.set_presentation_model(make_ref<family_picker_presentation_model>());
        iStylePicker.set_presentation_model(make_ref<style_picker_presentation_model>(iStylePicker.selection_model(), iFamilyPicker.selection_model()));
        iSizePicker.set_presentation_model(make_ref<picker_presentation_model>());

        iFamilyPicker.selection_model().current_index_changed([this](const optional_item_presentation_model_index&, const optional_item_presentation_model_index&)
        {
            update_selected_font(iFamilyPicker);
        });
        iFamilyPicker.SelectionChanged([this](const optional_item_model_index&)
        {
            update_selected_font(iFamilyPicker);
        });

        iStylePicker.selection_model().current_index_changed([this](const optional_item_presentation_model_index&, const optional_item_presentation_model_index&)
        {
            update_selected_font(iStylePicker);
        });
        iStylePicker.SelectionChanged([this](const optional_item_model_index&)
        {
            update_selected_font(iStylePicker);
        });

        iSizePicker.SelectionChanged([this](const optional_item_model_index&)
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

        auto& fm = service<i_font_manager>();

        for (uint32_t fi = 0; fi < fm.font_family_count(); ++fi)
            iFamilyPicker.model().insert_item(item_model_index{ fi }, fm.font_family(fi).to_std_string());

        center_on_parent();
        update_selected_font(*this);
        update_selected_appearance(*this);
        set_ready_to_render(true);
    }

    void font_dialog::update_selected_font(const i_widget& aUpdatingWidget)
    {
        if (iUpdating)
            return;
        neolib::scoped_flag sf{ iUpdating };

        auto oldFont = iSelectedFont;
        auto& fm = service<i_font_manager>();
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
            auto fontStyleIndex = iStylePicker.presentation_model().to_item_model_index(iStylePicker.selection_model().current_index()).row();
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
            iSizePicker.selection_model().clear_current_index();
        iSample.set_font(iSelectedFont);
        if (iSelectedFont != oldFont)
            SelectionChanged.trigger();
    }

    void font_dialog::update_selected_appearance(i_widget const& aUpdatingWidget)
    {
        if (!iSelectedAppearance || iUpdating)
            return;
        neolib::scoped_flag sf{ iUpdating };
        if (std::holds_alternative<color_widget>(iInk))
        {
            if (iSelectedAppearance->ink() != std::get<color_widget>(iInk).color())
            {
                iSelectedAppearance->set_ink(std::get<color_widget>(iInk).color());
                SelectionChanged.trigger();
            }
        }
        else if (std::holds_alternative<gradient_widget>(iInk))
        {
            if (iSelectedAppearance->ink() != std::get<gradient_widget>(iInk).gradient())
            {
                iSelectedAppearance->set_ink(std::get<gradient_widget>(iInk).gradient());
                SelectionChanged.trigger();
            }
        }
        if (std::holds_alternative<color_widget>(iPaper))
        {
            if (iSelectedAppearance->paper() != std::get<color_widget>(iPaper).color())
            {
                iSelectedAppearance->set_paper(std::get<color_widget>(iPaper).color());
                SelectionChanged.trigger();
            }
        }
        else if (std::holds_alternative<gradient_widget>(iPaper))
        {
            if (iSelectedAppearance->paper() != std::get<gradient_widget>(iPaper).gradient())
            {
                iSelectedAppearance->set_paper(std::get<gradient_widget>(iPaper).gradient());
                SelectionChanged.trigger();
            }
        }
        iSample.set_text_appearance(iSelectedAppearance);
    }
    
    void font_dialog::update_widgets()
    {
        auto oldSelection = iSelectedAppearance;
        if (iSelectedAppearance)
        {
            if (iInkBox.check_box().is_checked())
            {
                if (iInkColor.is_checked())
                {
                    if (iSelectedAppearance->ink() == neolib::none)
                        iSelectedAppearance->set_ink(iDefaultInk ? *iDefaultInk : service<i_app>().current_style().palette().color(color_role::Text));
                    else if (std::holds_alternative<gradient>(iSelectedAppearance->ink()))
                        iSelectedAppearance->set_ink(std::get<gradient>(iSelectedAppearance->ink()).color_at(0.0));
                }
                else if (iInkGradient.is_checked())
                {
                    if (iSelectedAppearance->ink() == neolib::none)
                        iSelectedAppearance->set_ink(gradient{ iDefaultInk ? *iDefaultInk : service<i_app>().current_style().palette().color(color_role::Text) });
                    else if (std::holds_alternative<color>(iSelectedAppearance->ink()))
                        iSelectedAppearance->set_ink(gradient{ std::get<color>(iSelectedAppearance->ink()) });
                }
            }
            else
                iSelectedAppearance->set_ink(neolib::none);
            if (iPaperBox.check_box().is_checked())
            {
                if (iPaperColor.is_checked())
                {
                    if (!iSelectedAppearance->paper() || iSelectedAppearance->paper() == neolib::none)
                        iSelectedAppearance->set_paper(iDefaultPaper ? *iDefaultPaper : service<i_app>().current_style().palette().color(color_role::Background));
                    else if (std::holds_alternative<gradient>(*iSelectedAppearance->paper()))
                        iSelectedAppearance->set_paper(std::get<gradient>(*iSelectedAppearance->paper()).color_at(0.0));
                }
                else if (iPaperGradient.is_checked())
                {
                    if (!iSelectedAppearance->paper() || iSelectedAppearance->paper() == neolib::none)
                        iSelectedAppearance->set_paper(gradient{ iDefaultPaper ? *iDefaultPaper : service<i_app>().current_style().palette().color(color_role::Background) });
                    else if (std::holds_alternative<color>(*iSelectedAppearance->paper()))
                        iSelectedAppearance->set_paper(gradient{ std::get<color>(*iSelectedAppearance->paper()) });
                }
            }
            else
                iSelectedAppearance->set_paper(std::nullopt);
            if (iSelectedAppearance->ink() != neolib::none)
            {
                iInkBox.check_box().check();
                if (std::holds_alternative<color>(iSelectedAppearance->ink()))
                {
                    iInkColor.check();
                    if (!std::holds_alternative<color_widget>(iInk))
                    {
                        iInk.emplace<color_widget>(iInkBox.item_layout());
                        iSink += std::get<color_widget>(iInk).ColorChanged([&]()
                        {
                            update_selected_appearance(std::get<color_widget>(iInk));
                        });
                    }
                    std::get<color_widget>(iInk).set_color(std::get<color>(iSelectedAppearance->ink()));
                }
                else if (std::holds_alternative<gradient>(iSelectedAppearance->ink()))
                {
                    iInkGradient.check();
                    if (!std::holds_alternative<gradient_widget>(iInk))
                    {
                        iInk.emplace<gradient_widget>(iInkBox.item_layout());
                        iSink += std::get<gradient_widget>(iInk).GradientChanged([&]()
                        {
                            update_selected_appearance(std::get<gradient_widget>(iInk));
                        });
                    }
                    std::get<gradient_widget>(iInk).set_gradient(std::get<gradient>(iSelectedAppearance->ink()));
                }
            }
            else
                iInkBox.check_box().uncheck();
            if (iSelectedAppearance->paper())
            {
                iPaperBox.check_box().check();
                if (std::holds_alternative<color>(*iSelectedAppearance->paper()))
                {
                    iPaperColor.check();
                    if (!std::holds_alternative<color_widget>(iPaper))
                    {
                        iPaper.emplace<color_widget>(iPaperBox.item_layout());
                        iSink += std::get<color_widget>(iPaper).ColorChanged([&]()
                        {
                            update_selected_appearance(std::get<color_widget>(iPaper));
                        });
                    }
                    std::get<color_widget>(iPaper).set_color(std::get<color>(*iSelectedAppearance->paper()));
                }
                else if (std::holds_alternative<gradient>(*iSelectedAppearance->paper()))
                {
                    iPaperGradient.check();
                    if (!std::holds_alternative<gradient_widget>(iPaper))
                    {
                        iPaper.emplace<gradient_widget>(iPaperBox.item_layout());
                        iSink += std::get<gradient_widget>(iPaper).GradientChanged([&]()
                        {
                            update_selected_appearance(std::get<gradient_widget>(iPaper));
                        });
                    }
                    std::get<gradient_widget>(iPaper).set_gradient(std::get<gradient>(*iSelectedAppearance->paper()));
                }
            }
            else
                iPaperBox.check_box().uncheck();
        }
        if (iSelectedAppearance != oldSelection)
            SelectionChanged.trigger();
    }
}