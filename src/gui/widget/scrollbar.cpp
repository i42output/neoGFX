// scrollbar.cpp
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
#include <neogfx/gui/widget/i_skin_manager.hpp>
#include <neogfx/gui/widget/scrollbar.hpp>

namespace neogfx
{
    scrollbar::scrollbar(i_scrollbar_container& aContainer, scrollbar_type aType, scrollbar_style aStyle, bool aIntegerPositions) :
        iContainer(aContainer), 
        iType(aType), 
        iStyle(aStyle),
        iIntegerPositions(aIntegerPositions),
        iVisible(false),
        iMinimum(0.0),
        iMaximum(0.0),
        iStep(1.0),
        iPage(0.0),
        iClickedElement(scrollbar_element::None),
        iHoverElement(scrollbar_element::None),
        iPaused(false)
    {
        Position.changed_from_to([this](const value_type& aFrom, const value_type& aTo)
        {
            update_reason_e updateReason = (aFrom < aTo ? ScrolledDown : ScrolledUp);
            iContainer.scrollbar_updated(*this, updateReason);
        });
        set_alive();
    }

    scrollbar::~scrollbar()
    {
        if (iTransition)
            service<i_animator>().remove_transition(*iTransition);
    }

    i_scrollbar_container& scrollbar::container() const
    {
        return iContainer;
    }

    scrollbar_type scrollbar::type() const
    {
        return iType;
    }

    scrollbar_style scrollbar::style() const
    {
        return iStyle;
    }

    void scrollbar::show()
    {
        if (!iVisible && style() != scrollbar_style::Invisible)
        {
            iVisible = true;
            iContainer.scrollbar_updated(*this, Shown);
        }
    }

    void scrollbar::hide()
    {
        if (iVisible)
        {
            iVisible = false;
            iContainer.scrollbar_updated(*this, Hidden);
        }
    }

    bool scrollbar::visible() const
    {
        return iVisible;
    }

    scrollbar::value_type scrollbar::position() const
    {
        auto result = Position.value();
        if (iIntegerPositions)
            result = std::ceil(result);
        return result;
    }

    bool scrollbar::set_position(value_type aPosition, const optional_easing& aTransition, double aTransitionDuration)
    {
        aPosition = std::max(std::min(aPosition, maximum() - page()), minimum());
        if (iIntegerPositions)
            aPosition = std::ceil(aPosition);
        bool changed = false;
        if (Position != aPosition)
        {
            changed = true;
            if (aTransition == std::nullopt || *aTransition == easing::One || std::abs(aPosition - Position.value()) < page())
            {
                if (have_transition())
                {
                    if (transition().active())
                        transition().reset(easing::One);
                    else if (!transition().paused())
                        transition().disable();
                }
            }
            else
            {
                if (!iTransition)
                    iTransition = service<i_animator>().add_transition(Position, *aTransition, aTransitionDuration);
                else
                    transition().reset(*aTransition);
            }
            Position = aPosition;
        }
        return changed;
    }

    scrollbar::value_type scrollbar::minimum() const
    {
        return iMinimum;
    }

    void scrollbar::set_minimum(value_type aMinimum)
    {
        if (iMinimum != aMinimum)
        {
            iMinimum = aMinimum;
            iContainer.scrollbar_updated(*this, AttributeChanged);
        }
    }

    scrollbar::value_type scrollbar::maximum() const
    {
        return iMaximum;
    }

    void scrollbar::set_maximum(value_type aMaximum)
    {
        if (iMaximum != aMaximum)
        {
            iMaximum = aMaximum;
            iContainer.scrollbar_updated(*this, AttributeChanged);
        }
    }

    scrollbar::value_type scrollbar::step() const
    {
        return iStep;
    }

    void scrollbar::set_step(value_type aStep)
    {
        if (iStep != aStep)
        {
            iStep = aStep;
            iContainer.scrollbar_updated(*this, AttributeChanged);
        }
    }

    scrollbar::value_type scrollbar::page() const
    {
        return iPage;
    }

    void scrollbar::set_page(value_type aPage)
    {
        if (iPage != aPage)
        {
            iPage = aPage;
            iContainer.scrollbar_updated(*this, AttributeChanged);
        }
    }

    bool scrollbar::locked() const
    {
        return iLockedPosition != std::nullopt;
    }

    void scrollbar::lock(value_type aPosition)
    {
        if (iLockedPosition != std::nullopt)
            throw already_locked();
        iLockedPosition = position();
        if (have_transition() && transition().active())
            transition().pause();
        set_position(aPosition);
    }

    void scrollbar::unlock()
    {
        if (iLockedPosition == std::nullopt)
            throw not_locked();
        set_position(*iLockedPosition);
        iLockedPosition = std::nullopt;
        if (have_transition() && transition().paused())
            transition().resume();
    }

    dimension scrollbar::width() const
    {
        return width(style());
    }

    void scrollbar::render(i_graphics_context& aGc) const
    {
        if (style() == scrollbar_style::Invisible)
            return;
        service<i_skin_manager>().active_skin().draw_scrollbar(aGc, *this, *this);
    }

    rect scrollbar::element_geometry(scrollbar_element aElement) const
    {
        if (style() == scrollbar_style::Invisible)
            return rect{};
        scoped_units su(units::Pixels);
        rect g = iContainer.scrollbar_geometry(*this);
        const dimension margin = 3.0_dip;
        switch (aElement)
        {
        case scrollbar_element::Scrollbar:
            return g;
        case scrollbar_element::UpButton:
            if (iType == scrollbar_type::Vertical)
            {
                if (iStyle == scrollbar_style::Normal)
                    g.cy = std::ceil((g.cx - margin * 2.0) / 2.0 + margin * 2.0);
                else if (iStyle == scrollbar_style::Menu)
                    g.cy = std::ceil(width());
                else if (iStyle == scrollbar_style::Scroller)
                {
                    g.y = g.bottom() - std::ceil(width()) * 2.0;
                    g.cy = std::ceil(width());
                }
            }
            else
            {
                if (iStyle == scrollbar_style::Normal)
                    g.cx = std::ceil((g.cy - margin * 2.0) / 2.0 + margin * 2.0);
                else if (iStyle == scrollbar_style::Menu)
                    g.cx = std::ceil(width());
                else if (iStyle == scrollbar_style::Scroller)
                {
                    g.x = g.right() - std::ceil(width()) * 2.0;
                    g.cx = std::ceil(width());;
                }
            }
            break;
        case scrollbar_element::DownButton:
            if (iType == scrollbar_type::Vertical)
            {
                if (iStyle == scrollbar_style::Normal)
                {
                    g.y = g.bottom() - std::ceil((g.cx - margin * 2.0) / 2.0 + margin * 2.0);
                    g.cy = std::ceil((g.cx - margin * 2.0) / 2.0 + margin * 2.0);
                }
                else if (iStyle == scrollbar_style::Menu || iStyle == scrollbar_style::Scroller)
                {
                    g.y = g.bottom() - std::ceil(width());
                    g.cy = std::ceil(width());
                }
            }
            else
            {
                if (iStyle == scrollbar_style::Normal)
                {
                    g.x = g.right() - std::ceil((g.cy - margin * 2.0) / 2.0 + margin * 2.0);
                    g.cx = std::ceil((g.cy - margin * 2.0) / 2.0 + margin * 2.0);
                }
                else if (iStyle == scrollbar_style::Menu || iStyle == scrollbar_style::Scroller)
                {
                    g.x = g.right() - std::ceil(width());
                    g.cx = std::ceil(width());
                }
            }
            break;
        case scrollbar_element::PageUpArea:
            if (iType == scrollbar_type::Vertical)
            {
                g.y = element_geometry(scrollbar_element::UpButton).bottom() + 1.0;
                g.cy = element_geometry(scrollbar_element::Thumb).top() - 1.0 - g.y;
            }
            else
            {
                g.x = element_geometry(scrollbar_element::LeftButton).right() + 1.0;
                g.cx = element_geometry(scrollbar_element::Thumb).left() - 1.0 - g.x;
            }
            break;
        case scrollbar_element::PageDownArea:
            if (iType == scrollbar_type::Vertical)
            {
                g.y = element_geometry(scrollbar_element::Thumb).bottom() + 1.0;
                g.cy = element_geometry(scrollbar_element::DownButton).top() - 1.0 - g.y;
            }
            else
            {
                g.x = element_geometry(scrollbar_element::Thumb).right() + 1.0;
                g.cx = element_geometry(scrollbar_element::RightButton).left() - 1.0 - g.x;
            }
            break;
        case scrollbar_element::Thumb:
            if (iType == scrollbar_type::Vertical)
            {
                g.y = element_geometry(scrollbar_element::UpButton).bottom() + 1.0;
                dimension available = element_geometry(scrollbar_element::DownButton).top() - 1.0 - g.y;
                if ((maximum() - minimum()) != 0.0)
                {
                    g.cy = available * static_cast<dimension>(page() / (maximum() - minimum()));
                    dimension const minimumThumbHeight = rasterize(2.0_mm);
                    if (g.cy < minimumThumbHeight)
                    {
                        available -= (minimumThumbHeight - g.cy);
                        g.cy = minimumThumbHeight;
                    }
                    g.y += static_cast<dimension>(position() / (maximum() - minimum())) * available;
                }
                else
                {
                    g.y = 0;
                    g.cy = 0.0;
                }
                g.y = std::ceil(g.y);
                g.cy = std::ceil(g.cy);
            }
            else
            {
                g.x = element_geometry(scrollbar_element::LeftButton).right() + 1.0;
                dimension available = element_geometry(scrollbar_element::RightButton).left() - 1.0 - g.x;
                if ((maximum() - minimum()) != 0.0)
                {
                    g.cx = available * static_cast<dimension>(page() / (maximum() - minimum()));
                    dimension const minimumThumbWidth = rasterize(2.0_mm);
                    if (g.cx < minimumThumbWidth)
                    {
                        available -= (minimumThumbWidth - g.cx);
                        g.cx = minimumThumbWidth;
                    }
                    g.x += static_cast<dimension>(position() / (maximum() - minimum())) * available;
                }
                else
                {
                    g.x = 0;
                    g.cx = 0.0;
                }
                g.x = std::ceil(g.x);
                g.cx = std::ceil(g.cx);
            }
            break;
        }
        return to_units(su.saved_units(), g);
    }

    scrollbar_element scrollbar::element_at(const point& aPosition) const
    {
        if (style() == scrollbar_style::Invisible)
            return scrollbar_element::None;
        else if (element_geometry(scrollbar_element::UpButton).contains(aPosition))
            return scrollbar_element::UpButton;
        else if (element_geometry(scrollbar_element::DownButton).contains(aPosition))
            return scrollbar_element::DownButton;
        else if (style() == scrollbar_style::Normal)
        {
            if (element_geometry(scrollbar_element::PageUpArea).contains(aPosition))
                return scrollbar_element::PageUpArea;
            else if (element_geometry(scrollbar_element::PageDownArea).contains(aPosition))
                return scrollbar_element::PageDownArea;
            else if (element_geometry(scrollbar_element::Thumb).contains(aPosition))
                return scrollbar_element::Thumb;
            else
                return scrollbar_element::None;
        }
        else
            return scrollbar_element::None;
    }

    void scrollbar::update(const update_params_t& aUpdateParams)
    {
        if (!visible())
            return;
        if (clicked_element() != scrollbar_element::None && clicked_element() != element_at(iContainer.as_widget().root().mouse_position()))
            pause();
        else
            resume();
        if (clicked_element() == scrollbar_element::Thumb)
        {
            point delta = (std::holds_alternative<point>(aUpdateParams) ? static_variant_cast<point>(aUpdateParams) : iContainer.as_widget().root().mouse_position()) - iThumbClickedPosition;
            scoped_units su(units::Pixels);
            rect g = iContainer.scrollbar_geometry(*this);
            if (iType == scrollbar_type::Vertical)
            {
                g.y = element_geometry(scrollbar_element::UpButton).bottom() + 1.0;
                g.cy = element_geometry(scrollbar_element::DownButton).top() - 1.0 - g.y;
                g.cy -= (element_geometry(scrollbar_element::Thumb).cy - std::ceil(g.cy * static_cast<dimension>(page() / (maximum() - minimum()))));
                set_position(static_cast<value_type>(delta.y / g.height()) * (maximum() - minimum()) + iThumbClickedValue);
            }
            else
            {
                g.x = element_geometry(scrollbar_element::LeftButton).right() + 1.0;
                g.cx = element_geometry(scrollbar_element::RightButton).left() - 1.0 - g.x;
                g.cx -= (element_geometry(scrollbar_element::Thumb).cx - std::ceil(g.cx * static_cast<dimension>(page() / (maximum() - minimum()))));
                set_position(static_cast<value_type>(delta.x / g.width()) * (maximum() - minimum()) + iThumbClickedValue);
            }
            iContainer.scrollbar_updated(*this, Updated);
        }
        if (clicked_element() == scrollbar_element::None && iContainer.as_widget().entered())
            hover_element(element_at(iContainer.as_widget().root().mouse_position()));
        else
            unhover_element();
    }

    scrollbar_element scrollbar::clicked_element() const
    {
        return iClickedElement;
    }

    void scrollbar::click_element(scrollbar_element aElement)
    {
        if (iClickedElement != scrollbar_element::None)
            throw element_already_clicked();
        iClickedElement = aElement;
        switch (aElement)
        {
        case scrollbar_element::UpButton:
            set_position(position() - step());
            iTimer = std::make_shared<neolib::callback_timer>(service<async_task>(), [this](neolib::callback_timer& aTimer)
            {
                aTimer.set_duration(50);
                aTimer.again();
                if (!iPaused)
                    set_position(position() - step());
            }, 500);
            break;
        case scrollbar_element::DownButton:
            set_position(position() + step());
            iTimer = std::make_shared<neolib::callback_timer>(service<async_task>(), [this](neolib::callback_timer& aTimer)
            {
                aTimer.set_duration(50);
                aTimer.again();
                if (!iPaused)
                    set_position(position() + step());
            }, 500);
            break;
        case scrollbar_element::PageUpArea:
            set_position(position() - page());
            iTimer = std::make_shared<neolib::callback_timer>(service<async_task>(), [this](neolib::callback_timer& aTimer)
            {
                aTimer.set_duration(50);
                aTimer.again();
                if (!iPaused)
                    set_position(position() - page());
            }, 500);
            break;
        case scrollbar_element::PageDownArea:
            set_position(position() + page());
            iTimer = std::make_shared<neolib::callback_timer>(service<async_task>(), [this](neolib::callback_timer& aTimer)
            {
                aTimer.set_duration(50);
                aTimer.again();
                if (!iPaused)
                    set_position(position() + page());
            }, 500);
            break;
        case scrollbar_element::Thumb:
            iThumbClickedPosition = iContainer.as_widget().root().mouse_position();
            iThumbClickedValue = position();
            break;
        default:
            break;
        }
    }

    void scrollbar::unclick_element()
    {
        if (iClickedElement == scrollbar_element::None)
            throw element_not_clicked();
        iClickedElement = scrollbar_element::None;
        iTimer.reset();
        iPaused = false;
    }

    scrollbar_element scrollbar::hovering_element() const
    {
        return iHoverElement;
    }

    void scrollbar::hover_element(scrollbar_element aElement)
    {
        if (iHoverElement != aElement)
        {
            iHoverElement = aElement;
            iContainer.scrollbar_updated(*this, Updated);
        }
    }

    void scrollbar::unhover_element()
    {
        if (iHoverElement != scrollbar_element::None)
        {
            iHoverElement = scrollbar_element::None;
            iContainer.scrollbar_updated(*this, Updated);
        }
    }

    void scrollbar::pause()
    {
        iPaused = true;
    }

    void scrollbar::resume()
    {
        iPaused = false;
    }

    void scrollbar::track()
    {
        if (iScrollTrackPosition == std::nullopt)
        {
            iScrollTrackPosition = iContainer.as_widget().root().mouse_position();
            iTimer = std::make_shared<neolib::callback_timer>(service<async_task>(), [this](neolib::callback_timer& aTimer)
            {
                aTimer.again();
                point delta = iContainer.as_widget().root().mouse_position() - *iScrollTrackPosition;
                scoped_units su(iContainer.as_widget(), units::Pixels);
                rect g = iContainer.scrollbar_geometry(*this);
                if (iType == scrollbar_type::Vertical)
                {
                    g.y = element_geometry(scrollbar_element::UpButton).bottom() + 1.0;
                    g.cy = element_geometry(scrollbar_element::DownButton).top() - 1.0 - g.y;
                    set_position(position() + static_cast<value_type>(delta.y * 0.25f / g.height()) * (maximum() - minimum()));
                }
                else
                {
                    g.x = element_geometry(scrollbar_element::UpButton).right() + 1.0;
                    g.cx = element_geometry(scrollbar_element::DownButton).left() - 1.0 - g.x;
                    set_position(position() + static_cast<value_type>(delta.x * 0.25f / g.width()) * (maximum() - minimum()));
                }
            }, 50);
        }
    }

    void scrollbar::untrack()
    {
        if (iScrollTrackPosition != std::nullopt)
        {
            iScrollTrackPosition.reset();
            iTimer.reset();
        }
    }

    dimension scrollbar::width(scrollbar_style aStyle)
    {
        if (aStyle == scrollbar_style::Invisible)
            return 0.0;
        dimension w = rasterize(4.0_mm);
        if (aStyle == scrollbar_style::Menu || aStyle == scrollbar_style::Scroller)
        {
            dimension const margin = 3.0_dip;
            w -= margin * 2.0;
        }
        if (to_px<uint32_t>(w) % 2u == 0u)
            w = from_px<dimension>(to_px<uint32_t>(w) + 1u);
        return w;
    }

    bool scrollbar::is_widget() const
    {
        return true;
    }

    const i_widget& scrollbar::as_widget() const
    {
        return iContainer.as_widget();
    }

    rect scrollbar::element_rect(skin_element aElement) const
    {
        switch (aElement)
        {
        case skin_element::ClickableArea:
            return element_geometry(scrollbar_element::Scrollbar);
        case skin_element::Scrollbar:
            return element_geometry(scrollbar_element::Scrollbar);
        case skin_element::ScrollbarUpArrow:
            return element_geometry(scrollbar_element::UpButton);
        case skin_element::ScrollbarLeftArrow:
            return element_geometry(scrollbar_element::LeftButton);
        case skin_element::ScrollbarDownArrow:
            return element_geometry(scrollbar_element::DownButton);
        case skin_element::ScrollbarRightArrow:
            return element_geometry(scrollbar_element::RightButton);
        case skin_element::ScrollbarPageUpArea:
            return element_geometry(scrollbar_element::PageUpArea);
        case skin_element::ScrollbarPageLeftArea:
            return element_geometry(scrollbar_element::PageLeftArea);
        case skin_element::ScrollbarPageDownArea:
            return element_geometry(scrollbar_element::PageDownArea);
        case skin_element::ScrollbarPageRightArea:
            return element_geometry(scrollbar_element::PageRightArea);
        case skin_element::ScrollbarThumb:
            return element_geometry(scrollbar_element::Thumb);
        default:
            return as_widget().element_rect(aElement);
        }
    }

    bool scrollbar::have_transition() const
    {
        return iTransition != std::nullopt;
    }

    i_transition& scrollbar::transition() const
    {
        if (have_transition())
            return service<i_animator>().transition(*iTransition);
        throw no_transition();
    }
}