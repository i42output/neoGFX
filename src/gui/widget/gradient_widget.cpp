// gradient_widget.cpp
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
        static const dimension BAR_CONTENTS_HEIGHT = 26;
        static const dimension BAR_HEIGHT = BAR_CONTENTS_HEIGHT + (BORDER_THICKNESS * 2) + (BORDER_SPACER_THICKNESS * 2);
        static const dimension STOP_POINTER_HEIGHT = 6;
        static const dimension STOP_WIDTH = 11;
        static const dimension STOP_HEIGHT = STOP_WIDTH + STOP_POINTER_HEIGHT;
        static const dimension STOP_OFFSET = STOP_WIDTH / 2 + STOP_POINTER_HEIGHT;
        static const dimension CONTROL_HEIGHT = BAR_HEIGHT + STOP_WIDTH * 2;
    }

    void draw_alpha_background(i_graphics_context& aGc, const rect& aRect, dimension aAlphaPatternSize = ALPHA_PATTERN_SIZE)
    {
        scoped_scissor scissor(aGc, aRect);
        for (coordinate x = 0; x < aRect.width(); x += aAlphaPatternSize)
        {
            bool alt = ((static_cast<int32_t>(x / aAlphaPatternSize) % 2) == 1);
            for (coordinate y = 0; y < aRect.height(); y += aAlphaPatternSize)
            {
                aGc.fill_rect(rect{ aRect.top_left() + point{ x, y }, size{ aAlphaPatternSize, aAlphaPatternSize } }, alt ? color{ 160, 160, 160 } : color{ 255, 255, 255 });
                alt = !alt;
            }
        }
    }
    
    namespace
    {
        class alpha_dialog : public dialog
        {
        public:
            alpha_dialog(i_widget& aParent, color::component aCurrentAlpha) :
                dialog(aParent, "Select Alpha (Opacity Level)", window_style::Dialog | window_style::Modal | window_style::TitleBar | window_style::Close), iLayout{ layout() }, iLayout2{ client_layout() }, iSlider{ iLayout2 }, iSpinBox{ iLayout2 }
            {
                init(aCurrentAlpha);
            }
        public:
            color::component selected_alpha() const
            {
                return static_cast<color::component>(iSpinBox.value());
            }
        private:
            void init(color::component aCurrentAlpha)
            {
                iLayout.set_padding(neogfx::padding{});
                iLayout2.set_padding(neogfx::padding{});
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
                center_on_parent();
                set_ready_to_render(true);
            }
        private:
            virtual void paint_non_client(i_graphics_context& aGc) const
            {
                dialog::paint_non_client(aGc);
                rect backgroundRect{ window::client_widget().position(), window::client_widget().extents() };
                scoped_scissor scissor(aGc, update_rect());
                draw_alpha_background(aGc, backgroundRect, dip(ALPHA_PATTERN_SIZE));
                aGc.fill_rect(backgroundRect, background_color().with_alpha(selected_alpha()));
            }
        private:
            vertical_layout iLayout;
            horizontal_layout iLayout2;
            slider iSlider;
            spin_box iSpinBox;
        };
    }

    gradient_widget::gradient_widget(i_gradient const& aGradient) :
        iInGradientDialog{ false }, iTracking{ false }
    {
        set_padding(neogfx::padding{});
        set_gradient(aGradient);
    }

    gradient_widget::gradient_widget(i_widget& aParent, i_gradient const& aGradient) :
        widget{ aParent }, iInGradientDialog{ false }, iTracking{ false }
    {
        set_padding(neogfx::padding{});
        set_gradient(aGradient);
    }

    gradient_widget::gradient_widget(i_layout& aLayout, i_gradient const& aGradient) :
        widget{ aLayout }, iInGradientDialog{ false }, iTracking{ false }
    {
        set_padding(neogfx::padding{});
        set_gradient(aGradient);
    }

    gradient_widget::gradient_widget(gradient_dialog&, i_layout& aLayout, i_gradient const& aGradient) :
        widget{ aLayout }, iInGradientDialog{ true }, iTracking{ false }
    {
        set_padding(neogfx::padding{});
        set_gradient(aGradient);
    }

    i_gradient const& gradient_widget::gradient() const
    {
        return iSelection;
    }

    void gradient_widget::set_gradient(i_gradient const& aGradient)
    {
        if (iSelection != aGradient)
        {
            iSelection = aGradient;
            set_current_color_stop(std::nullopt);
            set_current_alpha_stop(std::nullopt);
            update();
            GradientChanged.trigger();
        }
    }

    void gradient_widget::set_gradient(i_ref_ptr<i_gradient> const& aGradient)
    {
        set_gradient(*aGradient);
    }

    const std::optional<color_dialog::custom_color_list>& gradient_widget::custom_colors() const
    {
        return iCustomColors;
    }

    std::optional<color_dialog::custom_color_list>& gradient_widget::custom_colors()
    {
        return iCustomColors;
    }

    gradient::color_stop_list::const_iterator gradient_widget::selected_color_stop() const
    {
        if (iCurrentColorStop)
            return *iCurrentColorStop;
        return iSelection.color_stops().end();
    }

    gradient::alpha_stop_list::const_iterator gradient_widget::selected_alpha_stop() const
    {
        if (iCurrentAlphaStop)
            return *iCurrentAlphaStop;
        return iSelection.alpha_stops().end();
    }

    size_policy gradient_widget::size_policy() const
    {
        if (has_size_policy())
            return widget::size_policy();
        return neogfx::size_policy{ size_constraint::Expanding, size_constraint::Minimum };
    }

    size gradient_widget::minimum_size(optional_size const& aAvailableSpace) const
    {
        if (has_minimum_size())
            return widget::minimum_size(aAvailableSpace);
        return size{ dip(CONTROL_HEIGHT) * 3, dip(CONTROL_HEIGHT) };
    }

    void gradient_widget::paint(i_graphics_context& aGc) const
    {
        scoped_units su{ *this, units::Pixels };
        rect rectContents = contents_rect();
        color frameColor = background_color().shaded(0x20);
        draw_alpha_background(aGc, rectContents, dip(ALPHA_PATTERN_SIZE));
        neogfx::gradient selection = gradient();
        selection.set_direction(gradient_direction::Horizontal);
        aGc.fill_rect(rectContents, selection);
        rectContents.inflate(size{ dip(BORDER_THICKNESS) });
        aGc.draw_rect(rectContents, pen(frameColor.mid(background_color()), dip(BORDER_THICKNESS)));
        rectContents.inflate(size{ dip(BORDER_THICKNESS) });
        aGc.draw_rect(rectContents, pen(frameColor, dip(BORDER_THICKNESS)));
        if (iCurrentColorStop != std::nullopt)
        {
            auto const r = color_stop_rect(**iCurrentColorStop);
            aGc.line_stipple_on(1.0_dip, 0x5555, 0.0);
            aGc.push_logical_operation(neogfx::logical_operation::Xor);
            aGc.draw_line(point{ std::floor(r.center().x), r.top() }, point{ std::floor(r.center().x), contents_rect().top() }, color::White);
            aGc.line_stipple_off();
            aGc.pop_logical_operation();
        }
        else if (iCurrentAlphaStop != std::nullopt)
        {
            auto const r = alpha_stop_rect(**iCurrentAlphaStop);
            aGc.line_stipple_on(1.0_dip, 0x5555, 0.0);
            aGc.push_logical_operation(neogfx::logical_operation::Xor);
            aGc.draw_line(point{ std::floor(r.center().x), r.bottom() }, point{ std::floor(r.center().x), contents_rect().bottom() }, color::White);
            aGc.line_stipple_off();
            aGc.pop_logical_operation();
        }
        for (gradient::color_stop_list::const_iterator i = iSelection.color_stops().begin(); i != iSelection.color_stops().end(); ++i)
            draw_color_stop(aGc, *i);
        for (gradient::alpha_stop_list::const_iterator i = iSelection.alpha_stops().begin(); i != iSelection.alpha_stops().end(); ++i)
            draw_alpha_stop(aGc, *i);
        if (iCurrentColorStop != std::nullopt)
            draw_color_stop(aGc, **iCurrentColorStop);
        if (iCurrentAlphaStop != std::nullopt)
            draw_alpha_stop(aGc, **iCurrentAlphaStop);
    }

    void gradient_widget::mouse_button_pressed(mouse_button aButton, point const& aPosition, key_modifiers_e aKeyModifiers)
    {
        widget::mouse_button_pressed(aButton, aPosition, aKeyModifiers);
        iClicked = aPosition;
        if (aButton == mouse_button::Left)
        {
            if (is_stop_at(aPosition))
            {
                auto stopIter = stop_at(aPosition);
                if (std::holds_alternative<gradient::color_stop_list::iterator>(stopIter))
                {
                    set_current_color_stop(static_variant_cast<gradient::color_stop_list::iterator>(stopIter));
                    set_current_alpha_stop(std::nullopt);
                    iTracking = true;
                    update();
                }
                else if (std::holds_alternative<gradient::alpha_stop_list::iterator>(stopIter))
                {
                    set_current_alpha_stop(static_variant_cast<gradient::alpha_stop_list::iterator>(stopIter));
                    set_current_color_stop(std::nullopt);
                    iTracking = true;
                    update();
                }
            }
            else
            {
                if (contents_rect().contains(aPosition))
                {
                    if (iCurrentColorStop != std::nullopt)
                        (**iCurrentColorStop).second() = iSelection.color_at(aPosition.x, contents_rect().left(), contents_rect().right() - 1.0);
                    else if (iCurrentAlphaStop != std::nullopt)
                        (**iCurrentAlphaStop).second() = iSelection.alpha_at(aPosition.x, contents_rect().left(), contents_rect().right() - 1.0);
                    update();
                    GradientChanged.trigger();
                }
                else if (aPosition.y < contents_rect().top())
                {
                    set_current_alpha_stop(iSelection.insert_alpha_stop(aPosition.x, contents_rect().left(), contents_rect().right() - 1.0));
                    set_current_color_stop(std::nullopt);
                    update();
                    GradientChanged.trigger();
                }
                else if (aPosition.y >= contents_rect().bottom())
                {
                    set_current_color_stop(iSelection.insert_color_stop(aPosition.x, contents_rect().left(), contents_rect().right() - 1.0));
                    set_current_alpha_stop(std::nullopt);
                    update();
                    GradientChanged.trigger();
                }
            }
        }
    }

    void gradient_widget::mouse_button_double_clicked(mouse_button aButton, point const& aPosition, key_modifiers_e aKeyModifiers)
    {
        widget::mouse_button_double_clicked(aButton, aPosition, aKeyModifiers);
        if (aButton == mouse_button::Left)
        {
            auto stopIter = stop_at(aPosition);
            if (std::holds_alternative<gradient::color_stop_list::iterator>(stopIter))
            {
                auto& stop = *static_variant_cast<gradient::color_stop_list::iterator>(stopIter);
                auto const originalColor = stop.second();
                color_dialog cd{ *this, stop.second() };
                if (iCustomColors != std::nullopt)
                    cd.set_custom_colors(*iCustomColors);
                auto update_stop = [&](color updatedColor)
                {
                    auto const color = updatedColor;
                    stop.second() = color.with_alpha(1.0);
                    if (color.alpha() != 0xFF)
                    {
                        iCurrentAlphaStop = std::nullopt;
                        auto as = iSelection.find_alpha_stop(stop.first());
                        if (as == iSelection.alpha_stops().end() || as->first() != stop.first())
                            as = iSelection.insert_alpha_stop(stop.first());
                        as->second() = color.alpha();
                    }
                    update();
                    GradientChanged.trigger();
                };
                cd.SelectionChanged([&]()
                {
                    update_stop(cd.selected_color());
                });
                if (cd.exec() == dialog_result::Accepted)
                    update_stop(cd.selected_color());
                else
                    update_stop(originalColor);
                iCustomColors = cd.custom_colors();
            }
            else if (std::holds_alternative<gradient::alpha_stop_list::iterator>(stopIter))
            {
                auto& stop = *static_variant_cast<gradient::alpha_stop_list::iterator>(stopIter);
                alpha_dialog ad{ *this, stop.second() };
                if (ad.exec() == dialog_result::Accepted)
                {
                    stop.second() = ad.selected_alpha();
                    update();
                    GradientChanged.trigger();
                }
            }
        }
    }

    void gradient_widget::mouse_button_released(mouse_button aButton, point const& aPosition)
    {
        widget::mouse_button_released(aButton, aPosition);
        if (aButton == mouse_button::Right)
        {
            auto moreAction = std::make_shared<action>("More..."_t);
            moreAction->Triggered([this]()
            {
                neogfx::gradient const originalGradient = gradient();
                gradient_dialog gd{ *this, gradient() };
                if (iCustomColors != std::nullopt)
                    gd.gradient_selector().custom_colors() = iCustomColors;
                gd.gradient_selector().GradientChanged([&]()
                {
                    set_gradient(gd.gradient_selector().gradient());
                });
                if (gd.exec() == dialog_result::Accepted)
                    set_gradient(gd.gradient());
                else
                    set_gradient(originalGradient);
                if (gd.gradient_selector().custom_colors() != std::nullopt)
                    iCustomColors = gd.gradient_selector().custom_colors();
            });
            auto stopIter = stop_at(aPosition);
            if (stopIter != neolib::none && stopIter == stop_at(*iClicked))
            {
                if (std::holds_alternative<gradient::color_stop_list::iterator>(stopIter))
                {
                    auto iter = static_variant_cast<gradient::color_stop_list::iterator>(stopIter);
                    auto selectColorAction = std::make_shared<action>("Select stop color..."_t);
                    selectColorAction->Triggered([this, iter]()
                    {
                        auto& stop = *iter;
                        auto const originalColor = stop.second();
                        color_dialog cd{ *this, stop.second() };
                        if (iCustomColors != std::nullopt)
                            cd.set_custom_colors(*iCustomColors);
                        auto update_stop = [&](color updatedColor)
                        {
                            auto const color = updatedColor;
                            stop.second() = color.with_alpha(1.0);
                            if (color.alpha() != 0xFF)
                            {
                                iCurrentAlphaStop = std::nullopt;
                                auto as = iSelection.find_alpha_stop(stop.first());
                                if (as == iSelection.alpha_stops().end() || as->first() != stop.first())
                                    as = iSelection.insert_alpha_stop(stop.first());
                                as->second() = color.alpha();
                            }
                            update();
                            GradientChanged.trigger();
                        };
                        cd.SelectionChanged([&]()
                        {
                            update_stop(cd.selected_color());
                        });
                        if (cd.exec() == dialog_result::Accepted)
                            update_stop(cd.selected_color());
                        else
                            update_stop(originalColor);
                        iCustomColors = cd.custom_colors();
                    });
                    auto splitStopAction = std::make_shared<action>("Split stop"_t);
                    splitStopAction->Triggered([this, iter]()
                    {
                        auto prev = iter;
                        if (prev != iSelection.color_stops().begin())
                            --prev;
                        auto next = iter;
                        if (next != iSelection.color_stops().end() - 1)
                            ++next;
                        double p1 = (iter->first() + (prev)->first()) / 2.0;
                        double p2 = (iter->first() + (next)->first()) / 2.0;
                        auto const c = iter->second();
                        set_current_color_stop(std::nullopt);
                        if (iter != prev && iter != next)
                            iSelection.color_stops().erase(iter);
                        if (iter != prev)
                            iSelection.insert_color_stop(p1)->second() = c;
                        if (iter != next)
                            iSelection.insert_color_stop(p2)->second() = c;
                        update();
                        GradientChanged.trigger();
                    });
                    auto deleteStopAction = std::make_shared<action>("Delete stop"_t);
                    deleteStopAction->Triggered([this, iter]()
                    {
                        set_current_color_stop(std::nullopt);
                        iSelection.color_stops().erase(iter);
                        update();
                        GradientChanged.trigger();
                    });
                    if (iSelection.color_stops().size() <= 2)
                        deleteStopAction->disable();
                    iMenu = std::make_unique<context_menu>(*this, aPosition + non_client_rect().top_left() + root().window_position());
                    iMenu->menu().add_action(selectColorAction);
                    iMenu->menu().add_action(deleteStopAction);
                    iMenu->menu().add_action(splitStopAction);
                    if (!iInGradientDialog)
                        iMenu->menu().add_action(moreAction);
                    iMenu->exec();
                    iMenu.reset();
                }
                else if (std::holds_alternative<gradient::alpha_stop_list::iterator>(stopIter))
                {
                    auto iter = static_variant_cast<gradient::alpha_stop_list::iterator>(stopIter);
                    auto selectAlphaAction = std::make_shared<action>("Select stop alpha (opacity level)..."_t);
                    selectAlphaAction->Triggered([this, stopIter]()
                    {
                        auto& stop = *static_variant_cast<gradient::alpha_stop_list::iterator>(stopIter);
                        alpha_dialog ad{ root().as_widget(), stop.second() };
                        if (ad.exec() == dialog_result::Accepted)
                        {
                            stop.second() = ad.selected_alpha();
                            update();
                            GradientChanged.trigger();
                        }
                    });
                    auto splitStopAction = std::make_shared<action>("Split stop"_t);
                    splitStopAction->Triggered([this, iter]()
                    {
                        auto prev = iter;
                        if (prev != iSelection.alpha_stops().begin())
                            --prev;
                        auto next = iter;
                        if (next != iSelection.alpha_stops().end() - 1)
                            ++next;
                        double p1 = (iter->first() + (prev)->first()) / 2.0;
                        double p2 = (iter->first() + (next)->first()) / 2.0;
                        auto const a = iter->second();
                        set_current_alpha_stop(std::nullopt);
                        if (iter != prev && iter != next)
                            iSelection.alpha_stops().erase(iter);
                        if (iter != prev)
                            iSelection.insert_alpha_stop(p1)->second() = a;
                        if (iter != next)
                            iSelection.insert_alpha_stop(p2)->second() = a;
                        update();
                        GradientChanged.trigger();
                    });
                    auto deleteStopAction = std::make_shared<action>("Delete stop");
                    deleteStopAction->Triggered([this, stopIter]()
                    {
                        if (iCurrentAlphaStop != std::nullopt && *iCurrentAlphaStop == static_variant_cast<gradient::alpha_stop_list::iterator>(stopIter))
                            set_current_alpha_stop(std::nullopt);
                        iSelection.alpha_stops().erase(static_variant_cast<gradient::alpha_stop_list::iterator>(stopIter));
                        update();
                        GradientChanged.trigger();
                    });
                    if (iSelection.alpha_stops().size() <= 2)
                        deleteStopAction->disable();
                    iMenu = std::make_unique<context_menu>(*this, aPosition + non_client_rect().top_left() + root().window_position());
                    iMenu->menu().add_action(selectAlphaAction);
                    iMenu->menu().add_action(deleteStopAction);
                    iMenu->menu().add_action(splitStopAction);
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

    void gradient_widget::mouse_moved(point const& aPosition, key_modifiers_e aKeyModifiers)
    {
        widget::mouse_moved(aPosition, aKeyModifiers);
        if (iTracking)
        {
            double pos = gradient::normalized_position(aPosition.x, contents_rect().left(), contents_rect().right() - 1.0);
            const double min = 0.0001;
            if (iCurrentColorStop != std::nullopt)
            {
                auto leftStop = *iCurrentColorStop;
                if (leftStop != iSelection.color_stops().begin())
                    --leftStop;
                auto rightStop = *iCurrentColorStop;
                if (rightStop + 1 != iSelection.color_stops().end())
                    ++rightStop;
                (**iCurrentColorStop).first() =
                    std::min(std::max(pos,
                        leftStop == *iCurrentColorStop ? 0.0 : leftStop->first() + min),
                        rightStop == *iCurrentColorStop ? 1.0 : rightStop->first() - min);
                update();
                GradientChanged.trigger();
            }
            else if (iCurrentAlphaStop != std::nullopt)
            {
                auto leftStop = *iCurrentAlphaStop;
                if (leftStop != iSelection.alpha_stops().begin())
                    --leftStop;
                auto rightStop = *iCurrentAlphaStop;
                if (rightStop + 1 != iSelection.alpha_stops().end())
                    ++rightStop;
                (**iCurrentAlphaStop).first() =
                    std::min(std::max(pos,
                        leftStop == *iCurrentAlphaStop ? 0.0 : leftStop->first() + min),
                        rightStop == *iCurrentAlphaStop ? 1.0 : rightStop->first() - min);
                update();
                GradientChanged.trigger();
            }
        }
    }
    
    neogfx::mouse_cursor gradient_widget::mouse_cursor() const
    {
        point mousePos = root().mouse_position() - origin();
        if (stop_at(mousePos) != neolib::none)
            return mouse_system_cursor::Arrow;
        else if (contents_rect().contains(mousePos))
        {
            if (iCurrentColorStop != std::nullopt || iCurrentAlphaStop != std::nullopt)
                return mouse_system_cursor::Crosshair;
            else
                return mouse_system_cursor::Arrow;
        }
        else if (mousePos.x >= contents_rect().left() && mousePos.x < contents_rect().right())
            return mouse_system_cursor::Hand;
        return widget::mouse_cursor();
    }
    
    rect gradient_widget::contents_rect() const
    {
        rect r = client_rect(false);
        r.translate(point{ std::floor(dip(STOP_WIDTH) / 2), dip(STOP_WIDTH) });
        r.cx = r.width() - dip(STOP_WIDTH);
        r.cy = dip(BAR_HEIGHT);
        r.deflate(size{ dip(BORDER_THICKNESS) });
        r.deflate(size{ dip(BORDER_THICKNESS) });
        return r;
    }

    bool gradient_widget::is_stop_at(point const& aPosition) const
    {
        return stop_at(aPosition) != neolib::none;
    }

    gradient_widget::stop_const_iterator gradient_widget::stop_at(point const& aPosition) const
    {
        for (auto i = iSelection.color_stops().begin(); i != iSelection.color_stops().end(); ++i)
            if (color_stop_rect(*i).contains(aPosition))
                return i;
        for (auto i = iSelection.alpha_stops().begin(); i != iSelection.alpha_stops().end(); ++i)
            if (alpha_stop_rect(*i).contains(aPosition))
                return i;
        return stop_const_iterator{};
    }

    gradient_widget::stop_iterator gradient_widget::stop_at(point const& aPosition)
    {
        iCurrentColorStop = std::nullopt;
        for (auto i = iSelection.color_stops().begin(); i != iSelection.color_stops().end(); ++i)
            if (color_stop_rect(*i).contains(aPosition))
                return i;
        iCurrentAlphaStop = std::nullopt;
        for (auto i = iSelection.alpha_stops().begin(); i != iSelection.alpha_stops().end(); ++i)
            if (alpha_stop_rect(*i).contains(aPosition))
                return i;
        return stop_iterator{};
    }

    void gradient_widget::set_current_color_stop(std::optional<gradient::color_stop_list::iterator> const& aStop)
    {
        if (iCurrentColorStop != aStop)
        {
            iCurrentColorStop = aStop;
            if (iCurrentColorStop)
                ColorStopSelected.trigger();
            else
                ColorStopDeselected.trigger();
        }
    }

    void gradient_widget::set_current_alpha_stop(std::optional<gradient::alpha_stop_list::iterator> const& aStop)
    {
        if (iCurrentAlphaStop != aStop)
        {
            iCurrentAlphaStop = aStop;
            if (iCurrentAlphaStop)
                AlphaStopSelected.trigger();
            else
                AlphaStopDeselected.trigger();
        }
    }

    rect gradient_widget::color_stop_rect(neogfx::gradient::color_stop const& aColorStop) const
    {
        rect result = contents_rect();
        result.x = result.left() + std::floor((result.width() - 1.0) * aColorStop.first()) - std::floor(dip(STOP_WIDTH) / 2);
        result.y = result.bottom() + dip(BORDER_THICKNESS + BORDER_SPACER_THICKNESS - STOP_OFFSET) + 1.0_dip;
        result.cx = dip(STOP_WIDTH);
        result.cy = dip(STOP_HEIGHT);
        return result;
    }

    rect gradient_widget::alpha_stop_rect(neogfx::gradient::alpha_stop const& aAlphaStop) const
    {
        rect result = contents_rect();
        result.x = result.left() + std::floor((result.width() - 1.0) * aAlphaStop.first()) - std::floor(dip(STOP_WIDTH) / 2);
        result.y = result.top() - dip(BORDER_THICKNESS + BORDER_SPACER_THICKNESS + (STOP_HEIGHT - STOP_OFFSET));
        result.cx = dip(STOP_WIDTH);
        result.cy = dip(STOP_HEIGHT);
        return result;
    }

    void gradient_widget::draw_color_stop(i_graphics_context& aGc, neogfx::gradient::color_stop const& aColorStop) const
    {
        rect r = color_stop_rect(aColorStop);
        draw_alpha_background(aGc, rect{ r.top_left() + point{ 2.0, 8.0 }, size{ 7.0, 7.0 } }, dip(SMALL_ALPHA_PATTERN_SIZE));
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
        color transparentColor{ 255, 255, 255, 0 };
        color backgroundColor = background_color();
        color frameColor = background_color().shaded(0x20);
        bool const selected = (iCurrentColorStop != std::nullopt && (**iCurrentColorStop).first() == aColorStop.first());
        image stopGlyph{
            dpi_select(stopGlpyhPattern, stopGlpyhHighDpiPattern),
            {
                {"paper", transparentColor},
                {"ink1", frameColor},
                {"ink2", frameColor.mid(backgroundColor)},
                {"ink3", !selected ? backgroundColor : service<i_app>().current_style().palette().color(color_role::Selection)},
                {"ink4", !selected ? backgroundColor : service<i_app>().current_style().palette().color(color_role::Selection).lighter(0x40)},
                {"ink9", aColorStop.second()}} };
        auto stopGlyphTexture = iStopTextures.find(stopGlyph.hash());
        if (stopGlyphTexture == iStopTextures.end())
            stopGlyphTexture = iStopTextures.emplace(stopGlyph.hash(), stopGlyph).first;
        aGc.draw_texture(r.top_left(), stopGlyphTexture->second);
    }

    void gradient_widget::draw_alpha_stop(i_graphics_context& aGc, neogfx::gradient::alpha_stop const& aAlphaStop) const
    {
        rect r = alpha_stop_rect(aAlphaStop);
        draw_alpha_background(aGc, rect{ r.top_left() + point{ 2.0, 2.0 }, dpi_select(size{ 7.0, 7.0 }, size{ 18.0, 18.0 }) }, dip(SMALL_ALPHA_PATTERN_SIZE));
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
        color transparentColor{ 255, 255, 255, 0 };
        color backgroundColor = background_color();
        color frameColor = background_color().shaded(0x20);
        bool const selected = (iCurrentAlphaStop != std::nullopt && (**iCurrentAlphaStop).first() == aAlphaStop.first());
        image stopGlyph{
            dpi_select(stopGlpyhPattern, stopGlpyhHighDpiPattern),
            {
                { "paper", transparentColor },
                { "ink1", frameColor },
                { "ink2", frameColor.mid(backgroundColor) },
                { "ink3", !selected ? backgroundColor : service<i_app>().current_style().palette().color(color_role::Selection) },
                { "ink4", !selected ? backgroundColor : service<i_app>().current_style().palette().color(color_role::Selection).lighter(0x40) },
                { "ink9", color::White.with_alpha(aAlphaStop.second()) } } };
        auto stopGlyphTexture = iStopTextures.find(stopGlyph.hash());
        if (stopGlyphTexture == iStopTextures.end())
            stopGlyphTexture = iStopTextures.emplace(stopGlyph.hash(), stopGlyph).first;
        aGc.draw_texture(r.top_left(), stopGlyphTexture->second);
    }
}