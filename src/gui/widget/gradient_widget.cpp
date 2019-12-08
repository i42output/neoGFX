// gradient_widget.cpp
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
#include <neogfx/app/i_app.hpp>
#include <neogfx/gfx/image.hpp>
#include <neogfx/gui/dialog/gradient_dialog.hpp>
#include <neogfx/gui/widget/slider.hpp>
#include <neogfx/gui/window/context_menu.hpp>
#include <neogfx/gui/widget/menu.hpp>
#include <neogfx/gui/widget/gradient_widget.hpp>
#include "../../hid/native/i_native_surface.hpp"

namespace neogfx
{
    namespace
    {
        static const dimension ALPHA_PATTERN_SIZE = 4;
        static const dimension SMALL_ALPHA_PATTERN_SIZE = 3;
        static const dimension BORDER_THICKNESS = 1;
        static const dimension BORDER_SPACER_THICKNESS = 1;
        static const dimension BAR_CONTENTS_HEIGHT = 16;
        static const dimension BAR_HEIGHT = BAR_CONTENTS_HEIGHT + (BORDER_THICKNESS * 2) + (BORDER_SPACER_THICKNESS * 2);
        static const dimension STOP_POINTER_HEIGHT = 6;
        static const dimension STOP_WIDTH = 11;
        static const dimension STOP_HEIGHT = STOP_WIDTH + STOP_POINTER_HEIGHT;
        static const dimension CONTROL_HEIGHT = BAR_HEIGHT + STOP_HEIGHT * 2;
    }

    void draw_alpha_background(i_graphics_context& aGraphicsContext, const rect& aRect, dimension aAlphaPatternSize = ALPHA_PATTERN_SIZE)
    {
        scoped_scissor scissor(aGraphicsContext, aRect);
        for (coordinate x = 0; x < aRect.width(); x += aAlphaPatternSize)
        {
            bool alt = false;
            if (static_cast<uint32_t>((x / aAlphaPatternSize)) % 2 == 1)
                alt = !alt;
            for (coordinate y = 0; y < aRect.height(); y += aAlphaPatternSize)
            {
                aGraphicsContext.fill_rect(rect{ aRect.top_left() + point{ x, y }, size{ aAlphaPatternSize, aAlphaPatternSize } }, alt ? colour{ 160, 160, 160 } : colour{ 255, 255, 255 });
                alt = !alt;
            }
        }
    }
    
    namespace
    {
        class alpha_dialog : public dialog
        {
        public:
            alpha_dialog(i_widget& aParent, colour::component aCurrentAlpha) :
                dialog(aParent, "Select Alpha (Opacity Level)", window_style::Modal | window_style::TitleBar | window_style::Close), iLayout{ layout() }, iLayout2{ client_layout() }, iSlider{ iLayout2 }, iSpinBox{ iLayout2 }
            {
                init(aCurrentAlpha);
            }
        public:
            colour::component selected_alpha() const
            {
                return static_cast<colour::component>(iSpinBox.value());
            }
        private:
            void init(colour::component aCurrentAlpha)
            {
                auto standardSpacing = set_standard_layout(size{ 16.0 });
                iLayout.set_margins(neogfx::margins{});
                iLayout.set_spacing(standardSpacing);
                iLayout2.set_margins(neogfx::margins{});
                iLayout2.set_spacing(standardSpacing);
                iSlider.set_minimum(0);
                iSlider.set_maximum(255);
                iSlider.set_step(1);
                iSpinBox.text_box().set_size_hint(size_hint{ "255" });
                iSpinBox.set_minimum(0);
                iSpinBox.set_maximum(255);
                iSpinBox.set_step(1);
                iSpinBox.ValueChanged([this]() {iSlider.set_value(iSpinBox.value()); update(); });
                iSlider.ValueChanged([this]() {iSpinBox.set_value(iSlider.value()); });
                iSlider.set_value(aCurrentAlpha);
                button_box().add_button(standard_button::Ok);
                button_box().add_button(standard_button::Cancel);
                centre_on_parent();
            }
        private:
            virtual void paint_non_client(i_graphics_context& aGraphicsContext) const
            {
                dialog::paint_non_client(aGraphicsContext);
                rect backgroundRect{ window::client_widget().position(), window::client_widget().extents() };
                scoped_scissor scissor(aGraphicsContext, update_rect());
                draw_alpha_background(aGraphicsContext, backgroundRect, spx(ALPHA_PATTERN_SIZE));
                aGraphicsContext.fill_rect(backgroundRect, background_colour().with_alpha(selected_alpha()));
            }
        private:
            vertical_layout iLayout;
            horizontal_layout iLayout2;
            slider iSlider;
            spin_box iSpinBox;
        };
    }

    gradient_widget::gradient_widget(const neogfx::gradient& aGradient) :
        iInGradientDialog{ false }, iTracking{ false }
    {
        set_margins(neogfx::margins{});
        set_gradient(aGradient);
    }

    gradient_widget::gradient_widget(i_widget& aParent, const neogfx::gradient& aGradient) :
        widget{ aParent }, iInGradientDialog{ false }, iTracking{ false }
    {
        set_margins(neogfx::margins{});
        set_gradient(aGradient);
    }

    gradient_widget::gradient_widget(i_layout& aLayout, const neogfx::gradient& aGradient) :
        widget{ aLayout }, iInGradientDialog{ false }, iTracking{ false }
    {
        set_margins(neogfx::margins{});
        set_gradient(aGradient);
    }

    gradient_widget::gradient_widget(gradient_dialog&, i_layout& aLayout, const neogfx::gradient& aGradient) :
        widget{ aLayout }, iInGradientDialog{ true }, iTracking{ false }
    {
        set_margins(neogfx::margins{});
        set_gradient(aGradient);
    }

    const gradient& gradient_widget::gradient() const
    {
        return iSelection;
    }

    void gradient_widget::set_gradient(const neogfx::gradient& aGradient)
    {
        if (iSelection != aGradient)
        {
            iSelection = aGradient;
            iCurrentColourStop = std::nullopt;
            iCurrentAlphaStop = std::nullopt;
            update();
            GradientChanged.trigger();
        }
    }

    const std::optional<colour_dialog::custom_colour_list>& gradient_widget::custom_colours() const
    {
        return iCustomColours;
    }

    std::optional<colour_dialog::custom_colour_list>& gradient_widget::custom_colours()
    {
        return iCustomColours;
    }

    size_policy gradient_widget::size_policy() const
    {
        return neogfx::size_policy{ size_constraint::Expanding, size_constraint::Minimum };
    }

    size gradient_widget::minimum_size(const optional_size& aAvailableSpace) const
    {
        if (has_minimum_size())
            return widget::minimum_size(aAvailableSpace);
        return size{ spx(CONTROL_HEIGHT) * 3, spx(CONTROL_HEIGHT) };
    }

    void gradient_widget::paint(i_graphics_context& aGraphicsContext) const
    {
        scoped_units su{ *this, units::Pixels };
        rect rectContents = contents_rect();
        colour frameColour = (background_colour().dark() ? background_colour().lighter(0x60) : background_colour().darker(0x60));
        draw_alpha_background(aGraphicsContext, rectContents, spx(ALPHA_PATTERN_SIZE));
        neogfx::gradient selection = iSelection;
        selection.set_direction(gradient_direction::Horizontal);
        aGraphicsContext.fill_rect(rectContents, selection);
        rectContents.inflate(size{ spx(BORDER_THICKNESS) });
        aGraphicsContext.draw_rect(rectContents, pen(frameColour.mid(background_colour()), spx(BORDER_THICKNESS)));
        rectContents.inflate(size{ spx(BORDER_THICKNESS) });
        aGraphicsContext.draw_rect(rectContents, pen(frameColour, spx(BORDER_THICKNESS)));
        for (gradient::colour_stop_list::const_iterator i = iSelection.colour_begin(); i != iSelection.colour_end(); ++i)
            draw_colour_stop(aGraphicsContext, *i);
        for (gradient::alpha_stop_list::const_iterator i = iSelection.alpha_begin(); i != iSelection.alpha_end(); ++i)
            draw_alpha_stop(aGraphicsContext, *i);
        if (iCurrentColourStop != std::nullopt)
            draw_colour_stop(aGraphicsContext, **iCurrentColourStop);
        if (iCurrentAlphaStop != std::nullopt)
            draw_alpha_stop(aGraphicsContext, **iCurrentAlphaStop);
    }

    void gradient_widget::mouse_button_pressed(mouse_button aButton, const point& aPosition, key_modifiers_e aKeyModifiers)
    {
        widget::mouse_button_pressed(aButton, aPosition, aKeyModifiers);
        iClicked = aPosition;
        if (aButton == mouse_button::Left)
        {
            if (contents_rect().contains(aPosition))
            {
                if (iCurrentColourStop != std::nullopt)
                    (**iCurrentColourStop).second = iSelection.colour_at(aPosition.x, contents_rect().left(), contents_rect().right() - 1.0);
                else if (iCurrentAlphaStop != std::nullopt)
                    (**iCurrentAlphaStop).second = iSelection.alpha_at(aPosition.x, contents_rect().left(), contents_rect().right() - 1.0);
                update();
                GradientChanged.trigger();
            }
            else
            {
                auto stopIter = stop_at(aPosition);
                if (std::holds_alternative<gradient::colour_stop_list::iterator>(stopIter))
                {
                    iCurrentColourStop = static_variant_cast<gradient::colour_stop_list::iterator>(stopIter);
                    iCurrentAlphaStop = std::nullopt;
                    iTracking = true;
                    update();
                }
                else if (std::holds_alternative<gradient::alpha_stop_list::iterator>(stopIter))
                {
                    iCurrentAlphaStop = static_variant_cast<gradient::alpha_stop_list::iterator>(stopIter);
                    iCurrentColourStop = std::nullopt;
                    iTracking = true;
                    update();
                }
                else
                {
                    if (aPosition.y < contents_rect().top())
                    {
                        iCurrentAlphaStop = iSelection.insert_alpha_stop(aPosition.x, contents_rect().left(), contents_rect().right() - 1.0);
                        iCurrentColourStop = std::nullopt;
                        update();
                        GradientChanged.trigger();
                    }
                    else if (aPosition.y >= contents_rect().bottom())
                    {
                        iCurrentColourStop = iSelection.insert_colour_stop(aPosition.x, contents_rect().left(), contents_rect().right() - 1.0);
                        iCurrentAlphaStop = std::nullopt;
                        update();
                        GradientChanged.trigger();
                    }
                }
            }
        }
    }

    void gradient_widget::mouse_button_double_clicked(mouse_button aButton, const point& aPosition, key_modifiers_e aKeyModifiers)
    {
        widget::mouse_button_double_clicked(aButton, aPosition, aKeyModifiers);
        if (aButton == mouse_button::Left)
        {
            auto stopIter = stop_at(aPosition);
            if (std::holds_alternative<gradient::colour_stop_list::iterator>(stopIter))
            {
                auto& stop = *static_variant_cast<gradient::colour_stop_list::iterator>(stopIter);
                colour_dialog cd{ *this, stop.second };
                if (iCustomColours != std::nullopt)
                    cd.set_custom_colours(*iCustomColours);
                if (cd.exec() == dialog_result::Accepted)
                {
                    stop.second = cd.selected_colour();
                    update();
                    GradientChanged.trigger();
                }
                iCustomColours = cd.custom_colours();
            }
            else if (std::holds_alternative<gradient::alpha_stop_list::iterator>(stopIter))
            {
                auto& stop = *static_variant_cast<gradient::alpha_stop_list::iterator>(stopIter);
                alpha_dialog ad{ *this, stop.second };
                if (ad.exec() == dialog_result::Accepted)
                {
                    stop.second = ad.selected_alpha();
                    update();
                    GradientChanged.trigger();
                }
            }
        }
    }

    void gradient_widget::mouse_button_released(mouse_button aButton, const point& aPosition)
    {
        widget::mouse_button_released(aButton, aPosition);
        if (aButton == mouse_button::Right)
        {
            auto moreAction = std::make_shared<action>("More..."_t);
            moreAction->Triggered([this]()
            {
                gradient_dialog gd{ *this, gradient() };
                if (iCustomColours != std::nullopt)
                    gd.gradient_selector().custom_colours() = iCustomColours;
                if (gd.exec() == dialog_result::Accepted)
                    set_gradient(gd.gradient());
                if (gd.gradient_selector().custom_colours() != std::nullopt)
                    iCustomColours = gd.gradient_selector().custom_colours();
            });
            auto stopIter = stop_at(aPosition);
            if (stopIter != neolib::none && stopIter == stop_at(*iClicked))
            {
                if (std::holds_alternative<gradient::colour_stop_list::iterator>(stopIter))
                {
                    auto iter = static_variant_cast<gradient::colour_stop_list::iterator>(stopIter);
                    auto selectColourAction = std::make_shared<action>("Select stop colour..."_t);
                    selectColourAction->Triggered([this, iter]()
                    {
                        auto& stop = *iter;
                        colour_dialog cd{ *this, stop.second };
                        if (iCustomColours != std::nullopt)
                            cd.set_custom_colours(*iCustomColours);
                        if (cd.exec() == dialog_result::Accepted)
                        {
                            stop.second = cd.selected_colour();
                            update();
                            GradientChanged.trigger();
                        }
                        iCustomColours = cd.custom_colours();
                    });
                    auto splitStopAction = std::make_shared<action>("Split stop"_t);
                    splitStopAction->Triggered([this, iter]()
                    {
                        auto prev = iter;
                        if (prev != iSelection.colour_begin())
                            --prev;
                        auto next = iter;
                        if (next != iSelection.colour_end() - 1)
                            ++next;
                        double p1 = (iter->first + (prev)->first) / 2.0;
                        double p2 = (iter->first + (next)->first) / 2.0;
                        colour c = iter->second;
                        iCurrentColourStop = std::nullopt;
                        if (iter != prev && iter != next)
                            iSelection.erase_stop(iter);
                        if (iter != prev)
                            iSelection.insert_colour_stop(p1)->second = c;
                        if (iter != next)
                            iSelection.insert_colour_stop(p2)->second = c;
                        update();
                        GradientChanged.trigger();
                    });
                    auto deleteStopAction = std::make_shared<action>("Delete stop"_t);
                    deleteStopAction->Triggered([this, iter]()
                    {
                        iCurrentColourStop = std::nullopt;
                        iSelection.erase_stop(iter);
                        update();
                        GradientChanged.trigger();
                    });
                    if (iSelection.colour_stop_count() <= 2)
                        deleteStopAction->disable();
                    iMenu = std::make_unique<context_menu>(*this, aPosition + non_client_rect().top_left() + root().window_position());
                    iMenu->menu().add_action(selectColourAction);
                    iMenu->menu().add_action(deleteStopAction);
                    iMenu->menu().add_action(splitStopAction);
                    if (!iInGradientDialog)
                        iMenu->menu().add_action(moreAction);
                    iMenu->exec();
                    iMenu.reset();
                }
                else if (std::holds_alternative<gradient::alpha_stop_list::iterator>(stopIter))
                {
                    auto selectAlphaAction = std::make_shared<action>("Select stop alpha (opacity level)..."_t);
                    selectAlphaAction->Triggered([this, stopIter]()
                    {
                        auto& stop = *static_variant_cast<gradient::alpha_stop_list::iterator>(stopIter);
                        alpha_dialog ad{ root().as_widget(), stop.second };
                        if (ad.exec() == dialog_result::Accepted)
                        {
                            stop.second = ad.selected_alpha();
                            update();
                            GradientChanged.trigger();
                        }
                    });
                    auto deleteStopAction = std::make_shared<action>("Delete stop");
                    deleteStopAction->Triggered([this, stopIter]()
                    {
                        if (iCurrentAlphaStop != std::nullopt && *iCurrentAlphaStop == static_variant_cast<gradient::alpha_stop_list::iterator>(stopIter))
                            iCurrentAlphaStop = std::nullopt;
                        iSelection.erase_stop(static_variant_cast<gradient::alpha_stop_list::iterator>(stopIter));
                        update();
                        GradientChanged.trigger();
                    });
                    if (iSelection.alpha_stop_count() <= 2)
                        deleteStopAction->disable();
                    iMenu = std::make_unique<context_menu>(*this, aPosition + non_client_rect().top_left() + root().window_position());
                    iMenu->menu().add_action(selectAlphaAction);
                    iMenu->menu().add_action(deleteStopAction);
                    if (!iInGradientDialog)
                        iMenu->menu().add_action(moreAction);
                    iMenu->exec();
                    iMenu.reset();
                }
            }
            else if (!iInGradientDialog)
            {
                iMenu = std::make_unique<context_menu>(*this, aPosition + non_client_rect().top_left() + root().window_position());
                iMenu->menu().add_action(moreAction);
                iMenu->exec();
                iMenu.reset();
            }
        }
        iClicked == std::nullopt;
        iTracking = false;
        update();
    }

    void gradient_widget::mouse_moved(const point& aPosition)
    {
        widget::mouse_moved(aPosition);
        if (iTracking)
        {
            double pos = gradient::normalized_position(aPosition.x, contents_rect().left(), contents_rect().right() - 1.0);
            const double min = 0.0001;
            if (iCurrentColourStop != std::nullopt)
            {
                auto leftStop = *iCurrentColourStop;
                if (leftStop != iSelection.colour_begin())
                    --leftStop;
                auto rightStop = *iCurrentColourStop;
                if (rightStop + 1 != iSelection.colour_end())
                    ++rightStop;
                (**iCurrentColourStop).first =
                    std::min(std::max(pos,
                        leftStop == *iCurrentColourStop ? 0.0 : leftStop->first + min),
                        rightStop == *iCurrentColourStop ? 1.0 : rightStop->first - min);
                update();
                GradientChanged.trigger();
            }
            else if (iCurrentAlphaStop != std::nullopt)
            {
                auto leftStop = *iCurrentAlphaStop;
                if (leftStop != iSelection.alpha_begin())
                    --leftStop;
                auto rightStop = *iCurrentAlphaStop;
                if (rightStop + 1 != iSelection.alpha_end())
                    ++rightStop;
                (**iCurrentAlphaStop).first =
                    std::min(std::max(pos,
                        leftStop == *iCurrentAlphaStop ? 0.0 : leftStop->first + min),
                        rightStop == *iCurrentAlphaStop ? 1.0 : rightStop->first - min);
                update();
                GradientChanged.trigger();
            }
        }
    }
    
    neogfx::mouse_cursor gradient_widget::mouse_cursor() const
    {
        point mousePos = root().mouse_position() - origin();
        if (contents_rect().contains(mousePos))
        {
            if (iCurrentColourStop != std::nullopt || iCurrentAlphaStop != std::nullopt)
                return mouse_system_cursor::Crosshair;
            else
                return mouse_system_cursor::Arrow;
        }
        else if (stop_at(mousePos) != neolib::none)
            return mouse_system_cursor::Arrow;
        else if (mousePos.x >= contents_rect().left() && mousePos.x < contents_rect().right())
            return mouse_system_cursor::Hand;
        return widget::mouse_cursor();
    }
    
    rect gradient_widget::contents_rect() const
    {
        rect r = client_rect(false);
        r.move(point{ std::floor(spx(STOP_WIDTH) / 2), spx(STOP_HEIGHT) });
        r.cx = r.width() - spx(STOP_WIDTH);
        r.cy = spx(BAR_HEIGHT);
        r.deflate(size{ spx(BORDER_THICKNESS) });
        r.deflate(size{ spx(BORDER_THICKNESS) });
        return r;
    }

    gradient_widget::stop_const_iterator gradient_widget::stop_at(const point& aPosition) const
    {
        for (auto i = iSelection.colour_begin(); i != iSelection.colour_end(); ++i)
            if (colour_stop_rect(*i).contains(aPosition))
                return i;
        for (auto i = iSelection.alpha_begin(); i != iSelection.alpha_end(); ++i)
            if (alpha_stop_rect(*i).contains(aPosition))
                return i;
        return stop_const_iterator{};
    }

    gradient_widget::stop_iterator gradient_widget::stop_at(const point& aPosition)
    {
        for (auto i = iSelection.colour_begin(); i != iSelection.colour_end(); ++i)
            if (colour_stop_rect(*i).contains(aPosition))
                return i;
        for (auto i = iSelection.alpha_begin(); i != iSelection.alpha_end(); ++i)
            if (alpha_stop_rect(*i).contains(aPosition))
                return i;
        return stop_iterator{};
    }

    rect gradient_widget::colour_stop_rect(const neogfx::gradient::colour_stop& aColourStop) const
    {
        rect result = contents_rect();
        result.x = result.left() + std::floor((result.width() - 1.0) * aColourStop.first) - std::floor(spx(STOP_WIDTH) / 2);
        result.y = result.bottom() + spx(BORDER_THICKNESS + BORDER_SPACER_THICKNESS);
        result.cx = spx(STOP_WIDTH);
        result.cy = spx(STOP_HEIGHT);
        return result;
    }

    rect gradient_widget::alpha_stop_rect(const neogfx::gradient::alpha_stop& aAlphaStop) const
    {
        rect result = contents_rect();
        result.x = result.left() + std::floor((result.width() - 1.0) * aAlphaStop.first) - std::floor(spx(STOP_WIDTH) / 2);
        result.y = result.top() - spx(BORDER_THICKNESS + BORDER_SPACER_THICKNESS + STOP_HEIGHT);
        result.cx = spx(STOP_WIDTH);
        result.cy = spx(STOP_HEIGHT);
        return result;
    }

    void gradient_widget::draw_colour_stop(i_graphics_context& aGraphicsContext, const neogfx::gradient::colour_stop& aColourStop) const
    {
        rect r = colour_stop_rect(aColourStop);
        draw_alpha_background(aGraphicsContext, rect{ r.top_left() + point{ 2.0, 8.0 }, size{ 7.0, 7.0 } }, spx(SMALL_ALPHA_PATTERN_SIZE));
        const char* stopGlpyhPattern =
        {
            "[11,17]"
            "{0,paper}"
            "{1,ink1}"
            "{2,ink2}"
            "{3,ink3}"
            "{4,ink4}"
            "{9,ink9}"

            "00000100000"
            "00001410000"
            "00014331000"
            "00143333100"
            "01433333310"
            "14333333331"
            "11111111111"
            "12222222221"
            "12999999921"
            "12999999921"
            "12999999921"
            "12999999921"
            "12999999921"
            "12999999921"
            "12999999921"
            "12222222221"
            "11111111111"
        };
        const char* stopGlpyhHighDpiPattern =
        {
            "[22,34]"
            "{0,paper}"
            "{1,ink1}"
            "{2,ink2}"
            "{3,ink3}"
            "{4,ink4}"
            "{9,ink9}"

            "0000000000110000000000"
            "0000000001431000000000"
            "0000000014333100000000"
            "0000000143333310000000"
            "0000001433333331000000"
            "0000014333333333100000"
            "0000143333333333310000"
            "0001433333333333331000"
            "0014333333333333333100"
            "0143333333333333333310"
            "1433333333333333333331"
            "1433333333333333333331"
            "1111111111111111111111"
            "1222222222222222222221"
            "1299999999999999999921"
            "1299999999999999999921"
            "1299999999999999999921"
            "1299999999999999999921"
            "1299999999999999999921"
            "1299999999999999999921"
            "1299999999999999999921"
            "1299999999999999999921"
            "1299999999999999999921"
            "1299999999999999999921"
            "1299999999999999999921"
            "1299999999999999999921"
            "1299999999999999999921"
            "1299999999999999999921"
            "1299999999999999999921"
            "1299999999999999999921"
            "1299999999999999999921"
            "1299999999999999999921"
            "1222222222222222222221"
            "1111111111111111111111"
        };
        colour transparentColour{ 255, 255, 255, 0 };
        colour backgroundColour = background_colour();
        colour frameColour = (background_colour().dark() ? background_colour().lighter(0x60) : background_colour().darker(0x60));
        image stopGlyph{
            dpi_select(stopGlpyhPattern, stopGlpyhHighDpiPattern),
            {
                {"paper", transparentColour},
                {"ink1", frameColour},
                {"ink2", frameColour.mid(backgroundColour)},
                {"ink3", iCurrentColourStop == std::nullopt || &**iCurrentColourStop != &aColourStop ? backgroundColour : service<i_app>().current_style().palette().selection_colour()},
                {"ink4", iCurrentColourStop == std::nullopt || &**iCurrentColourStop != &aColourStop ? backgroundColour : service<i_app>().current_style().palette().selection_colour().lighter(0x40)},
                {"ink9", aColourStop.second}} };
        auto stopGlyphTexture = iStopTextures.find(stopGlyph.hash());
        if (stopGlyphTexture == iStopTextures.end())
            stopGlyphTexture = iStopTextures.emplace(stopGlyph.hash(), stopGlyph).first;
        aGraphicsContext.draw_texture(r.top_left(), stopGlyphTexture->second);
    }

    void gradient_widget::draw_alpha_stop(i_graphics_context& aGraphicsContext, const neogfx::gradient::alpha_stop& aAlphaStop) const
    {
        rect r = alpha_stop_rect(aAlphaStop);
        draw_alpha_background(aGraphicsContext, rect{ r.top_left() + point{ 2.0, 2.0 }, dpi_select(size{ 7.0, 7.0 }, size{ 18.0, 18.0 }) }, spx(SMALL_ALPHA_PATTERN_SIZE));
        const char* stopGlpyhPattern =
        {
            "[11,17]"
            "{0,paper}"
            "{1,ink1}"
            "{2,ink2}"
            "{3,ink3}"
            "{4,ink4}"
            "{9,ink9}"

            "11111111111"
            "12222222221"
            "12999999921"
            "12999999921"
            "12999999921"
            "12999999921"
            "12999999921"
            "12999999921"
            "12999999921"
            "12222222221"
            "11111111111"
            "14333333331"
            "01433333310"
            "00143333100"
            "00014331000"
            "00001410000"
            "00000100000"
        };
        const char* stopGlpyhHighDpiPattern =
        {
            "[22,34]"
            "{0,paper}"
            "{1,ink1}"
            "{2,ink2}"
            "{3,ink3}"
            "{4,ink4}"
            "{9,ink9}"

            "1111111111111111111111"
            "1222222222222222222221"
            "1299999999999999999921"
            "1299999999999999999921"
            "1299999999999999999921"
            "1299999999999999999921"
            "1299999999999999999921"
            "1299999999999999999921"
            "1299999999999999999921"
            "1299999999999999999921"
            "1299999999999999999921"
            "1299999999999999999921"
            "1299999999999999999921"
            "1299999999999999999921"
            "1299999999999999999921"
            "1299999999999999999921"
            "1299999999999999999921"
            "1299999999999999999921"
            "1299999999999999999921"
            "1299999999999999999921"
            "1222222222222222222221"
            "1111111111111111111111"
            "1433333333333333333331"
            "1433333333333333333331"
            "0143333333333333333310"
            "0014333333333333333100"
            "0001433333333333331000"
            "0000143333333333310000"
            "0000014333333333100000"
            "0000001433333331000000"
            "0000000143333310000000"
            "0000000014333100000000"
            "0000000001331000000000"
            "0000000000110000000000"
        };
        colour transparentColour{ 255, 255, 255, 0 };
        colour backgroundColour = background_colour();
        colour frameColour = (background_colour().dark() ? background_colour().lighter(0x60) : background_colour().darker(0x60));
        image stopGlyph{
            dpi_select(stopGlpyhPattern, stopGlpyhHighDpiPattern),
            {
                { "paper", transparentColour },
                { "ink1", frameColour },
                { "ink2", frameColour.mid(backgroundColour) },
                { "ink3", iCurrentAlphaStop == std::nullopt || &**iCurrentAlphaStop != &aAlphaStop ? backgroundColour : service<i_app>().current_style().palette().selection_colour() },
                { "ink4", iCurrentAlphaStop == std::nullopt || &**iCurrentAlphaStop != &aAlphaStop ? backgroundColour : service<i_app>().current_style().palette().selection_colour().lighter(0x40) },
                { "ink9", colour::White.with_alpha(aAlphaStop.second) } } };
        auto stopGlyphTexture = iStopTextures.find(stopGlyph.hash());
        if (stopGlyphTexture == iStopTextures.end())
            stopGlyphTexture = iStopTextures.emplace(stopGlyph.hash(), stopGlyph).first;
        aGraphicsContext.draw_texture(r.top_left(), stopGlyphTexture->second);
    }
}