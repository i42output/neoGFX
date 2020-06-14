// window.cpp
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
#include <boost/format.hpp>
#include <neolib/core/string_utils.hpp>
#include <neolib/core/scoped.hpp>
#include <neogfx/app/i_basic_services.hpp>
#include <neogfx/app/i_app.hpp>
#include <neogfx/hid/i_surface_manager.hpp>
#include <neogfx/hid/i_display.hpp>
#include <neogfx/hid/surface_window_proxy.hpp>
#include "native/i_native_window.hpp"
#include <neogfx/gui/layout/i_layout.hpp>
#include <neogfx/gui/window/window.hpp>

namespace neogfx
{
    rect corrected_popup_rect(i_window& aPopup, rect aPopupRect)
    {
        auto desktopRect = service<i_window_manager>().desktop_rect(aPopup);
        if (aPopupRect.x < desktopRect.x)
            aPopupRect.x += (desktopRect.x - aPopupRect.x);
        if (aPopupRect.y < desktopRect.y)
            aPopupRect.y += (desktopRect.y - aPopupRect.y);
        if (aPopupRect.right() > desktopRect.right())
            aPopupRect.position().x += (desktopRect.right() - aPopupRect.right());
        if (aPopupRect.bottom() > desktopRect.bottom())
            aPopupRect.position().y += (desktopRect.bottom() - aPopupRect.bottom());
        aPopupRect.x = std::max(aPopupRect.x, desktopRect.x);
        aPopupRect.y = std::max(aPopupRect.y, desktopRect.y);
        if (aPopupRect.right() > desktopRect.right())
            aPopupRect.cx += (desktopRect.right() - aPopupRect.right());
        if (aPopupRect.bottom() > desktopRect.bottom())
            aPopupRect.cy += (desktopRect.bottom() - aPopupRect.bottom());
        return aPopupRect;
    }

    rect corrected_popup_rect(i_window& aPopup)
    {
        return corrected_popup_rect(aPopup, service<i_window_manager>().window_rect(aPopup));
    }

    void correct_popup_rect(i_window& aPopup)
    {
        auto correctedRect = corrected_popup_rect(aPopup);
        auto& wm = service<i_window_manager>();
        wm.move_window(aPopup, correctedRect.position());
        wm.resize_window(aPopup, correctedRect.extents());
    }

    pause_rendering::pause_rendering(i_window& aWindow) :
        iSurface{ service<i_window_manager>().has_window(aWindow) && aWindow.has_native_surface() ? &aWindow.surface() : nullptr }
    {
        if (iSurface != nullptr)
        {
            iSurfaceDestroyed.emplace(*iSurface);
            iWindowDestroyed.emplace(aWindow.as_widget());
            iSurface->pause_rendering();
        }
    }

    pause_rendering::~pause_rendering()
    {
        if (iSurface != nullptr && !*iWindowDestroyed && !*iSurfaceDestroyed)
            iSurface->resume_rendering();
    }

    class window::nested_details : public i_nested_window
    {
    public:
        nested_details(i_window& aSurrogate) : iSurrogate{ aSurrogate }
        {
            nested_root().add(*this);
        }
        ~nested_details()
        {
            nested_root().remove(*this);
        }
    public:
        const i_nest& nested_root() const override
        {
            return iSurrogate.nest();
        }
        i_nest& nested_root() override
        {
            return iSurrogate.nest();
        }
        bool has_nested_parent() const override
        {
            return iSurrogate.has_parent_window() && iSurrogate.parent_window().is_nested();
        }
        const i_nested_window& nested_parent() const override
        {
            if (has_nested_parent())
                return iSurrogate.parent_window().as_nested();
            throw no_nested_parent();
        }
        i_nested_window& nested_parent() override
        {
            return const_cast<i_nested_window&>(to_const(*this).nested_parent());
        }
    public:
        const i_window& as_window() const override
        {
            return iSurrogate;
        }
        i_window& as_window() override
        {
            return iSurrogate;
        }
    private:
        i_window& iSurrogate;
    };

    class window::client : public scrollable_widget
    {
    public:
        client(i_layout& aLayout, scrollbar_style aScrollbarStyle);
    protected:
        bool can_defer_layout() const override;
        bool is_managing_layout() const override;
    protected:
        neogfx::size_policy size_policy() const override;
        size minimum_size(const optional_size& aAvailableSpace) const override;
    protected:
        bool transparent_background() const override;
    private:
        vertical_layout iLayout;
    };

    window::client::client(i_layout& aLayout, scrollbar_style aScrollbarStyle) :
        scrollable_widget{ aLayout, frame_style::NoFrame, aScrollbarStyle },
        iLayout{ *this }
    {
        set_margins(neogfx::margins{});
        iLayout.set_margins(neogfx::margins{});
    }

    bool window::client::can_defer_layout() const
    {
        return true;
    }

    bool window::client::is_managing_layout() const
    {
        return true;
    }

    neogfx::size_policy window::client::size_policy() const
    {
        return iLayout.size_policy();
    }

    size window::client::minimum_size(const optional_size& aAvailableSpace) const
    {
        if (has_minimum_size() || (static_cast<const window&>(parent()).style() & window_style::Resize) != window_style::Resize)
            return scrollable_widget::minimum_size(aAvailableSpace);
        else
            return service<i_app>().current_style().margins().size();
    }

    bool window::client::transparent_background() const
    {
        return true;
    }

    window::window(window_style aStyle, frame_style aFrameStyle, scrollbar_style aScrollbarStyle) : 
        window{ nullptr, window_placement::default_placement(), {}, aStyle, aFrameStyle, aScrollbarStyle }
    {
    }

    window::window(const window_placement& aPlacement, window_style aStyle, frame_style aFrameStyle, scrollbar_style aScrollbarStyle) :
        window{ nullptr, aPlacement, {}, aStyle, aFrameStyle, aScrollbarStyle }
    {
    }

    window::window(const window_placement& aPlacement, const std::string& aWindowTitle, window_style aStyle, frame_style aFrameStyle, scrollbar_style aScrollbarStyle) :
        window{ nullptr, aPlacement, aWindowTitle, aStyle, aFrameStyle, aScrollbarStyle }
    {
    }

    window::window(const std::string& aWindowTitle, window_style aStyle, frame_style aFrameStyle, scrollbar_style aScrollbarStyle) :
        window{ nullptr, window_placement::default_placement(), aWindowTitle, aStyle, aFrameStyle, aScrollbarStyle }
    {
    }

    window::window(i_widget& aParent, window_style aStyle, frame_style aFrameStyle, scrollbar_style aScrollbarStyle) :
        window{ &aParent, window_placement::default_placement(), {}, aStyle, aFrameStyle, aScrollbarStyle }
    {
    }

    window::window(i_widget& aParent, const window_placement& aPlacement, window_style aStyle, frame_style aFrameStyle, scrollbar_style aScrollbarStyle) :
        window{ &aParent, aPlacement, {}, aStyle, aFrameStyle, aScrollbarStyle }
    {
    }

    window::window(i_widget& aParent, const window_placement& aPlacement, const std::string& aWindowTitle, window_style aStyle, frame_style aFrameStyle, scrollbar_style aScrollbarStyle) :
        window{ &aParent, aPlacement, aWindowTitle, aStyle, aFrameStyle, aScrollbarStyle }
    {
    }

    window::window(i_widget& aParent, const std::string& aWindowTitle, window_style aStyle, frame_style aFrameStyle, scrollbar_style aScrollbarStyle) :
        window{ &aParent, window_placement::default_placement(), aWindowTitle, aStyle, aFrameStyle, aScrollbarStyle }
    {
    }

    window::~window()
    {
        update_modality(true);
        window_manager().remove_window(*this);
        set_destroyed();
    }

    window::window(i_widget* aParent, const window_placement& aPlacement, const std::optional<std::string>& aWindowTitle, window_style aStyle, frame_style aFrameStyle, scrollbar_style aScrollbarStyle) :
        scrollable_widget{ aFrameStyle, aScrollbarStyle },
        iWindowManager{ service<i_window_manager>() },
        iParentWindow{ nullptr },
        iPlacement{ aPlacement },
        iClosed{ false },
        iTitleText{ aWindowTitle ? *aWindowTitle : service<i_app>().name() },
        iStyle{ aStyle },
        iCountedEnable{ 0 },
        iEnteredWidget{ nullptr },
        iFocusedWidget{ nullptr },
        iDismissingChildren{ false },
        iNonClientLayout{ *this },
        iTitleBarLayout{ iNonClientLayout },
        iMenuLayout{ iNonClientLayout },
        iToolbarLayout{ iNonClientLayout },
        iDockLayout{ iToolbarLayout.center() },
        iClientWidget{ std::make_unique<client>(iDockLayout.center(), aScrollbarStyle) },
        iClientLayout{ iClientWidget->layout() },
        iStatusBarLayout{ iNonClientLayout }
    {
        if (aParent)
            set_parent(*aParent);

        window_manager().add_window(*this);

        if (is_nested() && !has_parent())
            throw parentless_window_cannot_nest();

        if (has_parent() && (ultimate_ancestor().is_fullscreen() || service<i_app>().program_options().nest()) && &ultimate_ancestor() != this)
            iStyle |= window_style::Nested;

        if (aPlacement.video_mode())
        {
            if (is_nested())
                throw fullscreen_window_cannot_nest();
            iStyle |= window_style::Fullscreen;
            iStyle &= ~(window_style::Resize | window_style::MinimizeBox | window_style::MaximizeBox);
            iSurfaceWindow = std::make_unique<surface_window_proxy>(
                *this,
                [&](i_surface_window& aProxy) 
                    { 
                        return service<i_rendering_engine>().create_window(
                            service<i_surface_manager>(), 
                            aProxy, 
                            *aPlacement.video_mode(),
                            title_text(), 
                            style()); 
                    });
        }
        else if (!is_nested())
        {
            auto correctedPlacement = aPlacement;
            // todo: multiple displays
            if (!has_parent_window(false) && service<i_surface_manager>().display().is_fullscreen())
            {
                iStyle |= window_style::Fullscreen;
                iStyle &= ~(window_style::Resize | window_style::MinimizeBox | window_style::MaximizeBox);
                correctedPlacement = service<i_surface_manager>().display().fullscreen_video_mode();
            }
            switch (correctedPlacement.state())
            {
            default:
            case window_state::Normal:
                if (!has_parent_window(false))
                    iSurfaceWindow = std::make_unique<surface_window_proxy>(
                        *this,
                        [&](i_surface_window& aProxy) 
                            { return service<i_rendering_engine>().create_window(service<i_surface_manager>(), aProxy, correctedPlacement.normal_geometry()->top_left(), correctedPlacement.normal_geometry()->extents(), title_text(), style()); });
                else
                    iSurfaceWindow = std::make_unique<surface_window_proxy>(
                        *this,
                        [&](i_surface_window& aProxy) 
                            { return service<i_rendering_engine>().create_window(service<i_surface_manager>(), aProxy, parent_window().surface().native_surface(), correctedPlacement.normal_geometry()->top_left(), correctedPlacement.normal_geometry()->extents(), title_text(), style()); });
                break;
            case window_state::Iconized:
                // todo
                throw not_yet_implemented();
                break;
            case window_state::Maximized:
                // todo
                throw not_yet_implemented();
                break;
            }
        }

        init();
    }

    window_style window::style() const
    {
        return iStyle;
    }

    void window::set_style(window_style aStyle)
    {
        iStyle = aStyle;
    }

    const i_window_manager& window::window_manager() const
    {
        return iWindowManager;
    }

    i_window_manager& window::window_manager()
    {
        return iWindowManager;
    }

    bool window::is_surface() const
    {
        return iSurfaceWindow != nullptr;
    }

    bool window::has_surface() const
    {
        if (is_surface())
            return true;
        return find_surface() != nullptr;
    }

    const i_surface_window& window::surface() const
    {
        if (is_surface())
            return *iSurfaceWindow;
        auto s = find_surface();
        if (s != nullptr)
            return *s;
        throw no_surface();
    }

    i_surface_window& window::surface()
    {
        return const_cast<i_surface_window&>(to_const(*this).surface());
    }

    bool window::has_native_surface() const
    {
        return is_surface() && !*iSurfaceDestroyed;
    }

    const i_native_surface& window::native_surface() const
    {
        if (!has_native_surface())
            throw no_native_surface();
        return surface().native_surface();
    }

    i_native_surface& window::native_surface()
    {
        return const_cast<i_native_surface&>(to_const(*this).native_surface());
    }

    bool window::has_native_window() const
    {
        return has_native_surface() && surface().surface_type() == neogfx::surface_type::Window;
    }

    const i_native_window& window::native_window() const
    {
        return static_cast<const i_native_window&>(native_surface());
    }

    i_native_window& window::native_window()
    {
        return const_cast<i_native_window&>(to_const(*this).native_window());
    }

    bool window::has_parent_window(bool aSameSurface) const
    {
        return iParentWindow != nullptr &&
            (!aSameSurface ||
            (has_surface() && iParentWindow->has_surface() && &surface() == &iParentWindow->surface()));
    }

    const i_window& window::parent_window() const
    {
        if (iParentWindow != nullptr)
            return *iParentWindow;
        throw no_parent_window();
    }

    i_window& window::parent_window()
    {
        return const_cast<i_window&>(to_const(*this).parent_window());
    }

    bool window::is_owner_of(const i_window& aChildWindow) const
    {
        const i_window* w = &aChildWindow;
        if (w == this)
            return false;
        while (w->has_parent_window(false))
        {
            w = &w->parent_window();
            if (w == this)
                return true;
        }
        return false;
    }

    const i_window& window::ultimate_ancestor() const
    {
        const i_window* w = this;
        while (w->has_parent_window(false))
            w = &w->parent_window();
        return *w;
    }

    i_window& window::ultimate_ancestor()
    {
        return const_cast<i_window&>(to_const(*this).ultimate_ancestor());
    }

    bool window::is_nested() const
    {
        return (style() & window_style::Nested) == window_style::Nested;
    }

    const i_nest& window::nest() const
    {
        if (is_nest())
            return as_nest();
        if (!has_parent_window())
            throw not_in_nest();
        const i_window* w = &parent_window();
        while (!w->is_nest() && w->has_parent_window())
            w = &w->parent_window();
        if (w->is_nest())
            return w->as_nest();
        throw not_in_nest();
    }

    i_nest& window::nest()
    {
        return const_cast<i_nest&>(to_const(*this).nest());
    }

    const i_nested_window& window::as_nested() const
    {
        return *iNestedWindowDetails;
    }

    i_nested_window& window::as_nested()
    {
        return const_cast<i_nested_window&>(to_const(*this).as_nested());
    }

    bool window::is_nest() const
    {
        return iNest != std::nullopt;
    }

    const i_nest& window::as_nest() const
    {
        if (is_nest())
            return *iNest;
        throw not_a_nest();
    }

    i_nest& window::as_nest()
    {
        return const_cast<i_nest&>(to_const(*this).as_nest());
    }

    bool window::is_strong() const
    {
        return !is_weak();
    }

    bool window::is_weak() const
    {
        return (style() & window_style::Weak) == window_style::Weak;
    }

    bool window::can_close() const
    {
        if (is_strong() && !service<i_app>().in_exec())
            return false;
        return true;
    }

    bool window::is_closed() const
    {
        return iClosed;
    }

    void window::close()
    {
        if (iClosed)
            return;
        if (has_layout())
            layout().remove_all();
        remove_all();
        destroyed_flag destroyed{ *this };
        if (is_surface())
            surface().close();
        if (destroyed)
            return;
        iClosed = true;
        Closed.trigger();
    }

    color window::frame_color() const
    {
        if (effectively_enabled() && !scrollable_widget::has_frame_color() && is_active())
        {
            if (!is_nested())
                return service<i_app>().current_style().palette().color(color_role::Selection);
            else
                return service<i_app>().current_style().palette().color(color_role::SecondaryAccent);
        }
        else
            return scrollable_widget::frame_color().with_alpha(is_active() ? 1.0 : 0.25);
    }

    bool window::is_root() const
    {
        return true;
    }

    const i_window& window::root() const
    {
        return *this;
    }

    i_window& window::root()
    {
        return *this;
    }

    void window::set_parent(i_widget& aParent)
    {
        iParentWindow = &aParent.root();
    }

    bool window::can_defer_layout() const
    {
        return true;
    }

    bool window::is_managing_layout() const
    {
        return true;
    }

    void window::layout_items_completed()
    {
        scrollable_widget::layout_items_completed();
        if (iEnteredWidget != nullptr)
        {
            i_widget& widgetUnderMouse = (!surface().has_capturing_widget() ? widget_for_mouse_event(mouse_position()) : surface().capturing_widget());
            if (iEnteredWidget != &widgetUnderMouse)
                mouse_entered(mouse_position());
        }
    }

    bool window::device_metrics_available() const
    {
        return scrollable_widget::device_metrics_available();
    }

    const i_device_metrics& window::device_metrics() const
    {
        return scrollable_widget::device_metrics();
    }

    void window::resized()
    {
        window_manager().resize_window(*this, widget::extents());
        scrollable_widget::resized();
        update(true);
    }

    widget_part window::hit_test(const point& aPosition) const
    {
        auto result = scrollable_widget::hit_test(aPosition);
        if (result == widget_part::Client)
            result = widget_part::Grab;
        return result;
    }

    neogfx::size_policy window::size_policy() const
    {
        if (scrollable_widget::has_size_policy())
            return scrollable_widget::size_policy();
        return size_constraint::Manual;
    }

    bool window::update(const rect& aUpdateRect)
    {
        if (!scrollable_widget::update(aUpdateRect))
            return false;
        if (is_nest())
            for (std::size_t nw = 0; nw < as_nest().nested_window_count(); ++nw)
                if (!as_nest().nested_window(nw).as_window().as_widget().non_client_rect().intersection(aUpdateRect).empty())
                    as_nest().nested_window(nw).as_window().as_widget().update(true);
        return true;
    }

    void window::render(i_graphics_context& aGc) const
    {
        aGc.set_extents(extents());
        aGc.set_origin(origin());
        if ((style() & window_style::DropShadow) == window_style::DropShadow)
        {
            if (!is_nested())
            {
                // todo
            }
            else
            {
                rect shadowRect = to_client_coordinates(non_client_rect());
                shadowRect.position() += point{ 4.0_dip, 4.0_dip };
                aGc.fill_rounded_rect(shadowRect, 4.0_dip, color::Yellow);
            }
        }
        aGc.set_extents(extents());
        aGc.set_origin(origin());
        scrollable_widget::render(aGc);
        if (is_nest())
            for (std::size_t nw = 0; nw < as_nest().nested_window_count(); ++nw)
                as_nest().nested_window(nw).as_window().as_widget().render(aGc);
        PaintOverlay.trigger(aGc);
    }

    void window::paint(i_graphics_context& aGc) const
    {
        scrollable_widget::paint(aGc);
    }

    color window::background_color() const
    {
        if (has_background_color())
            return scrollable_widget::background_color();
        else
            return container_background_color();
    }

    bool window::is_dismissing_children() const
    {
        return iDismissingChildren;
    }

    bool window::can_dismiss(const i_widget*) const
    {
        return true;
    }

    window::dismissal_type_e window::dismissal_type() const
    {
        return CloseOnDismissal;
    }

    bool window::dismissed() const
    {
        return iClosed;
    }

    void window::dismiss()
    {
        close();
    }

    void window::center(bool aSetMinimumSize)
    {
        layout_items();
        if (aSetMinimumSize)
            window_manager().resize_window(*this, minimum_size());
        rect desktopRect{ window_manager().desktop_rect(*this) };
        window_manager().move_window(*this, (desktopRect.extents() - window_manager().window_rect(*this).extents()) / 2.0);
    }

    void window::center_on_parent(bool aSetMinimumSize)
    {
        if (has_parent_window(false))
        {
            layout_items();
            if (aSetMinimumSize)
                window_manager().resize_window(*this, minimum_size());
            rect desktopRect{ window_manager().desktop_rect(*this) };
            rect parentRect{ window_manager().window_rect(parent_window()) };
            rect ourRect{ window_manager().window_rect(*this) };
            point position = point{ (parentRect.extents() - ourRect.extents()) / 2.0 } + parentRect.top_left();
            if (position.x < 0.0)
                position.x = 0.0;
            if (position.y < 0.0)
                position.y = 0.0;
            if (position.x + ourRect.cx > desktopRect.right())
                position.x = desktopRect.right() - ourRect.cx;
            if (position.y + ourRect.cy > desktopRect.bottom())
                position.y = desktopRect.bottom() - ourRect.cy;
            window_manager().move_window(*this, position.ceil());
        }
        else
            center(aSetMinimumSize);
    }

    void window::widget_added(i_widget&)
    {
        layout_items(true);
    }

    void window::widget_removed(i_widget& aWidget)
    {
        if (iEnteredWidget == &aWidget)
            iEnteredWidget = nullptr;
        if (surface().has_capturing_widget() && &surface().capturing_widget() == &aWidget)
            surface().release_capture(aWidget);
        if (iFocusedWidget == &aWidget)
            iFocusedWidget = nullptr;
        layout_items(true);
    }

    bool window::show(bool aVisible)
    {
        bool result = widget::show(aVisible);
        if (result && has_native_surface())
        {
            if (aVisible)
                native_window().show();
            else
                native_window().hide();
        }
        return result;
    }

    bool window::requires_owner_focus() const
    {
        return (iStyle & window_style::RequiresOwnerFocus) == window_style::RequiresOwnerFocus;
    }

    bool window::has_entered_widget() const
    {
        return iEnteredWidget != nullptr;
    }

    i_widget& window::entered_widget() const
    {
        if (iEnteredWidget == nullptr)
            throw widget_not_entered();
        return *iEnteredWidget;
    }

    bool window::has_focused_widget() const
    {
        return iFocusedWidget != nullptr;
    }

    i_widget& window::focused_widget() const
    {
        return *iFocusedWidget;
    }

    void window::set_focused_widget(i_widget& aWidget, focus_reason aFocusReason)
    {
        if (iFocusedWidget == &aWidget)
            return;
        i_widget* previouslyFocused = iFocusedWidget;
        iFocusedWidget = &aWidget;
        if (previouslyFocused != nullptr)
            previouslyFocused->focus_lost(aFocusReason);
        iFocusedWidget->focus_gained(aFocusReason);
    }

    void window::release_focused_widget(i_widget& aWidget)
    {
        if (iFocusedWidget != &aWidget)
            throw widget_not_focused();
        iFocusedWidget = nullptr;
        aWidget.focus_lost(focus_reason::Other);
    }

    void window::update_modality(bool aEnableAncestors)
    {
        for (std::size_t i = 0; i < window_manager().window_count(); ++i)
        {
            i_window& w = window_manager().window(i);
            if (&w != this)
            {
                if ((iStyle & window_style::ApplicationModal) == window_style::ApplicationModal)
                    w.counted_window_enable(aEnableAncestors);
                else if ((iStyle & window_style::Modal) == window_style::Modal && w.is_ancestor_of(*this))
                    w.counted_window_enable(aEnableAncestors);
            }
        }
        if (aEnableAncestors && has_parent_window(false) && (style() & window_style::NoActivate) != window_style::NoActivate)
            parent_window().activate();
    }

    scrolling_disposition window::scrolling_disposition(const i_widget& aChildWidget) const
    {
        if (iTitleBar != std::nullopt && &aChildWidget == &*iTitleBar)
            return neogfx::scrolling_disposition::DontScrollChildWidget;
        else if (iStatusBarLayout.find(aChildWidget) != std::nullopt)
            return neogfx::scrolling_disposition::DontScrollChildWidget;
        return scrollable_widget::scrolling_disposition(aChildWidget);
    }

    const std::string& window::title_text() const
    {
        return iTitleText;
    }

    void window::set_title_text(const std::string& aTitleText)
    {
        if (iTitleText != aTitleText)
        {
            iTitleText = aTitleText;
            if (iTitleBar != std::nullopt)
                iTitleBar->set_title(iTitleText);
            if (has_native_window())
                native_window().set_title_text(aTitleText);
        }
    }

    bool window::is_active() const
    {
        return has_native_window() && native_window().is_active();
    }

    void window::activate()
    {
        if (has_native_window())
            native_window().activate();
    }

    bool window::is_iconic() const
    {
        return has_native_window() && native_window().is_iconic();
    }

    void window::iconize()
    {
        if (has_native_window())
            native_window().iconize();
    }

    bool window::is_maximized() const
    {
        return has_native_window() && native_window().is_maximized();
    }

    void window::maximize()
    {
        if (has_native_window())
            native_window().maximize();
    }

    bool window::is_restored() const
    {
        return has_native_window() && native_window().is_restored();
    }

    void window::restore()
    {
        if (has_native_window())
            native_window().restore();
    }

    bool window::is_fullscreen() const
    {
        return has_native_window() && native_window().is_fullscreen();
    }

    void window::enter_fullscreen(const video_mode& aVideoMode)
    {
        native_window().enter_fullscreen(aVideoMode);
    }

    point window::window_position() const
    {
        return window_manager().window_rect(*this).position();
    }

    const window_placement& window::placement() const
    {
        // todo: update cache
        return iPlacement;
    }

    void window::set_placement(const neogfx::window_placement& aPlacement)
    {
        iPlacement = aPlacement;
        // todo: update native window state
    }

    bool window::window_enabled() const
    {
        if (is_surface())
            return has_native_window() && native_window().enabled();
        else
            return enabled();
    }

    void window::counted_window_enable(bool aEnable)
    {
        if (aEnable)
            ++iCountedEnable;
        else
            --iCountedEnable;
        if (is_surface() && has_native_window() && !is_nest())
            native_window().enable(iCountedEnable >= 0);
        else if (!is_surface() || is_nest())
            enable(iCountedEnable >= 0);
    }

    void window::modal_enable(bool aEnable)
    {
        counted_window_enable(aEnable);
        update_modality(aEnable);
        if (aEnable && (style() & window_style::NoActivate) != window_style::NoActivate)
            activate();
    }

    void window::init()
    {
        iSurfaceDestroyed.emplace(surface().native_surface());

        if (is_fullscreen() || (service<i_app>().program_options().nest() && &ultimate_ancestor() == this))
        {
            if (is_fullscreen())
                set_frame_style(frame_style::NoFrame);
            iNest.emplace(*this);
        }

        if ((style() & window_style::InitiallyHidden) == window_style::InitiallyHidden)
            hide();

        if (is_nested())
            iNestedWindowDetails = std::make_unique<nested_details>(*this);

        update_modality(false);

        if ((style() & window_style::TitleBar) == window_style::TitleBar)
            iTitleBar.emplace(*this, service<i_app>().default_window_icon(), title_text());

        set_margins({});
        iNonClientLayout.set_margins(neogfx::margins{});
        iNonClientLayout.set_spacing(size{});
        iTitleBarLayout.set_margins(neogfx::margins{});
        iMenuLayout.set_margins(neogfx::margins{});
        iToolbarLayout.set_margins(neogfx::margins{});
        iDockLayout.set_margins(neogfx::margins{});
        iDockLayout.top().set_margins(neogfx::margins{});
        iDockLayout.bottom().set_margins(neogfx::margins{});
        iDockLayout.left().set_margins(neogfx::margins{});
        iDockLayout.right().set_margins(neogfx::margins{});
        iDockLayout.center().set_margins(neogfx::margins{});
        iClientLayout.set_margins(neogfx::margins{});
        iStatusBarLayout.set_margins(neogfx::margins{});

        if (!is_nested())
            resize(native_surface().surface_size());
        else
            layout_items(true);

        iSink += service<i_app>().current_style_changed([this](style_aspect aAspect)
        {
            if ((aAspect & style_aspect::Color) == style_aspect::Color)
                surface().native_surface().invalidate(rect{ surface().surface_size() });
        });

        init_scrollbars();

        if (!placement().video_mode())
        {
            if (placement().state() == window_state::Normal)
            {
                if (placement().position_specified())
                    move(placement().normal_geometry()->top_left());
                else if ((style() & window_style::InitiallyCentered) == window_style::InitiallyCentered)
                    center_on_parent(false);
            }
        }

        if (has_native_window())
            native_window().initialisation_complete();
    }

    double window::rendering_priority() const
    {
        if (is_active())
            return 1.0;
        return 0.1;
    }

    double window::fps() const
    {
        if (has_surface())
            return native_window().fps();
        else
            return 0.0;
    }

    double window::potential_fps() const
    {
        if (has_surface())
            return native_window().potential_fps();
        else
            return 0.0;
    }

    point window::mouse_position() const
    {
        return window_manager().mouse_position(*this);
    }

    rect window::widget_part_rect(widget_part aWidgetPart) const
    {
        switch (aWidgetPart)
        {
        case widget_part::TitleBar:
            if (iTitleBar != std::nullopt)
                return to_client_coordinates(iTitleBar->non_client_rect());
            else
                return rect{};
        default:
            return rect{};
        }
    }

    const i_widget& window::client_widget() const
    {
        return *iClientWidget;
    }

    i_widget& window::client_widget()
    {
        return *iClientWidget;
    }

    bool window::has_layout(standard_layout aStandardLayout) const
    {
        switch (aStandardLayout)
        {
        case standard_layout::Client:
        case standard_layout::NonClient:
        case standard_layout::TitleBar:
        case standard_layout::Menu:
        case standard_layout::Toolbar:
        case standard_layout::Dock:
        case standard_layout::StatusBar:
            return true;
        case standard_layout::Default:
            return has_layout();
        default:
            return false;
        }
    }

    const i_layout& window::layout(standard_layout aStandardLayout, layout_position aPosition) const
    {
        switch (aStandardLayout)
        {
        case standard_layout::Client:
            return iClientLayout;
        case standard_layout::NonClient:
            return iNonClientLayout;
        case standard_layout::TitleBar:
            return iTitleBarLayout;
        case standard_layout::Menu:
            return iMenuLayout;
        case standard_layout::Toolbar:
            return iToolbarLayout.part(aPosition);
        case standard_layout::Dock:
            return iDockLayout.part(aPosition);
        case standard_layout::StatusBar:
            return iStatusBarLayout;
        case standard_layout::Default:
            return layout();
        default:
            throw standard_layout_not_found();
        }
    }

    i_layout& window::layout(standard_layout aStandardLayout, layout_position aPosition)
    {
        return const_cast<i_layout&>(to_const(*this).layout(aStandardLayout, aPosition));
    }

    bool window::is_widget() const
    {
        return true;
    }

    const i_widget& window::as_widget() const
    {
        return *this;
    }

    i_widget& window::as_widget()
    {
        return *this;
    }

    void window::mouse_entered(const point& aPosition)
    {
        i_widget& widgetUnderMouse = (!surface().has_capturing_widget() ? widget_for_mouse_event(aPosition) : surface().capturing_widget());
        i_widget* newEnteredWidget = &widgetUnderMouse;
        i_widget* oldEnteredWidget = iEnteredWidget;
        if (newEnteredWidget != oldEnteredWidget)
        {
            if (oldEnteredWidget != nullptr)
                oldEnteredWidget->mouse_left();
            iEnteredWidget = newEnteredWidget;
            iEnteredWidget->mouse_entered(aPosition);
        }
    }

    void window::mouse_left()
    {
        i_widget* oldEnteredWidget = iEnteredWidget;
        if (oldEnteredWidget != nullptr)
        {
            iEnteredWidget = nullptr;
            oldEnteredWidget->mouse_left();
        }
    }

    void window::dismiss_children(const i_widget* aClickedWidget)
    {
        DismissingChildren.trigger(aClickedWidget);
        neolib::scoped_flag sf{ iDismissingChildren };
        if ((iStyle & window_style::RequiresOwnerFocus) != window_style::RequiresOwnerFocus)
        {
            for (std::size_t i = 0; i < window_manager().window_count();)
            {
                i_window& w = window_manager().window(i);
                if (!w.dismissed() && is_owner_of(w) && w.can_dismiss(aClickedWidget))
                {
                    if (w.dismissal_type() == CloseOnDismissal)
                        i = 0;
                    else
                        ++i;
                    w.dismiss();
                }
                else
                    ++i;
            }
        }
    }

    void window::update_click_focus(i_widget& aCandidateWidget, const point& aClickPos)
    {
        bool const childHasFocus = has_focused_widget() && focused_widget().is_descendent_of(aCandidateWidget);
        if (childHasFocus && focused_widget().client_rect().contains(aClickPos - focused_widget().origin()))
            return;
        bool const inClientArea = (aCandidateWidget.hit_test(aClickPos - aCandidateWidget.origin()) == widget_part::Client);
        bool const ignoreNonClientArea = (aCandidateWidget.focus_policy() & focus_policy::IgnoreNonClient) != focus_policy::IgnoreNonClient;
        focus_reason const focusReason = (inClientArea ? focus_reason::ClickClient : focus_reason::ClickNonClient);
        if (aCandidateWidget.enabled() && aCandidateWidget.can_set_focus(focusReason))
        {
            if ((inClientArea || (!ignoreNonClientArea && !childHasFocus)))
                aCandidateWidget.set_focus(focusReason);
        }
        else if (aCandidateWidget.has_parent() && !childHasFocus)
            update_click_focus(aCandidateWidget.parent(), aClickPos);
    }
}