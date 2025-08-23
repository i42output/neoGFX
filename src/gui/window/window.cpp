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

#include <neolib/core/string_utils.hpp>
#include <neolib/core/scoped.hpp>

#include <neogfx/app/i_basic_services.hpp>
#include <neogfx/app/i_app.hpp>
#include <neogfx/app/drag_drop.hpp>
#include <neogfx/hid/i_surface_manager.hpp>
#include <neogfx/hid/i_display.hpp>
#include <neogfx/hid/surface_window.hpp>
#include <neogfx/gui/window/i_native_window.hpp>
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

    pause_rendering::pause_rendering(pause_rendering&& aOther) :
        iSurface{ aOther.iSurface },
        iSurfaceDestroyed{ aOther.iSurfaceDestroyed },
        iWindowDestroyed{ aOther.iWindowDestroyed }
    {
        aOther.iSurface = nullptr;
    }

    pause_rendering::~pause_rendering()
    {
        if (iSurface != nullptr && !*iWindowDestroyed && !*iSurfaceDestroyed)
            iSurface->resume_rendering();
    }

    pause_rendering& pause_rendering::operator=(pause_rendering&& aOther)
    {
        iSurface = aOther.iSurface;
        aOther.iSurface = nullptr;
        iSurfaceDestroyed = std::move(aOther.iSurfaceDestroyed);
        iWindowDestroyed = std::move(aOther.iWindowDestroyed);
        return *this;
    }

    class window::client : public framed_scrollable_widget
    {
    public:
        client(i_widget& aParent, neogfx::scrollbar_style aScrollbarStyle);
    protected:
        bool is_managing_layout() const override;
    protected:
        neogfx::size_policy size_policy() const override;
        size minimum_size(optional_size const& aAvailableSpace) const override;
    private:
        vertical_layout iLayout;
    };

    window::client::client(i_widget& aParent, neogfx::scrollbar_style aScrollbarStyle) :
        framed_scrollable_widget{ aParent, aScrollbarStyle, frame_style::NoFrame },
        iLayout{ *this }
    {
        set_padding(neogfx::padding{});
        iLayout.set_padding(neogfx::padding{});
        set_background_opacity(1.0);
    }

    bool window::client::is_managing_layout() const
    {
        return true;
    }

    neogfx::size_policy window::client::size_policy() const
    {
        return iLayout.size_policy();
    }

    size window::client::minimum_size(optional_size const& aAvailableSpace) const
    {
        if (has_minimum_size() || (root().style() & window_style::Resize) != window_style::Resize || querying_ideal_size())
            return framed_scrollable_widget::minimum_size(aAvailableSpace);
        return service<i_app>().current_style().padding(padding_role::Window).size();
    }

    window::window(window_style aStyle, frame_style aFrameStyle, neogfx::scrollbar_style aScrollbarStyle) :
        window{ nullptr, window_placement::default_placement(), {}, aStyle, aFrameStyle, aScrollbarStyle }
    {
    }

    window::window(const window_placement& aPlacement, window_style aStyle, frame_style aFrameStyle, neogfx::scrollbar_style aScrollbarStyle) :
        window{ nullptr, aPlacement, {}, aStyle, aFrameStyle, aScrollbarStyle }
    {
    }

    window::window(const window_placement& aPlacement, std::string const& aWindowTitle, window_style aStyle, frame_style aFrameStyle, neogfx::scrollbar_style aScrollbarStyle) :
        window{ nullptr, aPlacement, aWindowTitle, aStyle, aFrameStyle, aScrollbarStyle }
    {
    }

    window::window(std::string const& aWindowTitle, window_style aStyle, frame_style aFrameStyle, neogfx::scrollbar_style aScrollbarStyle) :
        window{ nullptr, window_placement::default_placement(), aWindowTitle, aStyle, aFrameStyle, aScrollbarStyle }
    {
    }

    window::window(i_widget& aParent, window_style aStyle, frame_style aFrameStyle, neogfx::scrollbar_style aScrollbarStyle) :
        window{ &aParent, window_placement::default_placement(), {}, aStyle, aFrameStyle, aScrollbarStyle }
    {
    }

    window::window(i_widget& aParent, const window_placement& aPlacement, window_style aStyle, frame_style aFrameStyle, neogfx::scrollbar_style aScrollbarStyle) :
        window{ &aParent, aPlacement, {}, aStyle, aFrameStyle, aScrollbarStyle }
    {
    }

    window::window(i_widget& aParent, const window_placement& aPlacement, std::string const& aWindowTitle, window_style aStyle, frame_style aFrameStyle, neogfx::scrollbar_style aScrollbarStyle) :
        window{ &aParent, aPlacement, aWindowTitle, aStyle, aFrameStyle, aScrollbarStyle }
    {
    }

    window::window(i_widget& aParent, std::string const& aWindowTitle, window_style aStyle, frame_style aFrameStyle, neogfx::scrollbar_style aScrollbarStyle) :
        window{ &aParent, window_placement::default_placement(), aWindowTitle, aStyle, aFrameStyle, aScrollbarStyle }
    {
    }

    window::~window()
    {
        set_destroying();
        update_modality(true);
        if (window_manager().has_window(*this))
            window_manager().remove_window(*this);
        close();
        set_destroyed();
    }

    inline decoration_style window_style_to_decoration_style(window_style aStyle)
    {
        decoration_style result = (aStyle & window_style::NoDecoration) == window_style::NoDecoration ?
            decoration_style::None : 
            (aStyle & window_style::Dialog) == window_style::Dialog ? 
                decoration_style::Dialog : decoration_style::Window;
        if ((aStyle & window_style::Tool) == window_style::Tool)
            result |= decoration_style::Tool;
        if ((aStyle & window_style::Popup) == window_style::Popup)
            result |= decoration_style::Popup;
        if ((aStyle & window_style::Menu) == window_style::Menu)
            result |= decoration_style::Menu;
        if ((aStyle & window_style::Nested) == window_style::Nested)
            result |= decoration_style::Nested;
        if ((aStyle & window_style::Resize) == window_style::Resize)
            result |= decoration_style::Resizable;
        if ((aStyle & window_style::Main) == window_style::Main)
            result |= decoration_style::DockAreas;
        return result;
    }

    window::window(i_widget* aParent, const window_placement& aPlacement, const std::optional<std::string>& aWindowTitle, window_style aStyle, frame_style aFrameStyle, neogfx::scrollbar_style aScrollbarStyle) :
        base_type{ window_style_to_decoration_style(aStyle), aScrollbarStyle, aFrameStyle },
        iWindowManager{ service<i_window_manager>() },
        iParentWindow{ nullptr },
        iPlacement{ aPlacement },
        iCentering{ false },
        iClosed{ false },
        iReadyToRender{ (aStyle & window_style::InitiallyRenderable) == window_style::InitiallyRenderable },
        iTitleText{ aWindowTitle ? *aWindowTitle : service<i_app>().name() },
        iStyle{ aStyle },
        iCountedEnable{ 0 },
        iHandlingMouseEntered{ false },
        iHandlingMouseLeft{ false },
        iEnteredWidget{ nullptr },
        iFocusedWidget{ nullptr },
        iDismissingChildren{ false }
    {
        if ((iStyle & window_style::Nested) == window_style::Nested)
            iStyle &= ~window_style::MinimizeBox;

        set_root(*this);

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
            make_ref<surface_window>(
                *this,
                [&](i_surface_window& aProxy)
            {
                service<i_rendering_engine>().create_window(
                    service<i_surface_manager>(),
                    aProxy,
                    *aPlacement.video_mode(),
                    title_text(),
                    style());
            });
        }
        else
        {
            auto correctedPlacement = aPlacement;
            if (!has_parent_window() && service<i_surface_manager>().display().is_fullscreen())
            {
                iStyle |= window_style::Fullscreen;
                iStyle &= ~(window_style::Resize | window_style::MinimizeBox | window_style::MaximizeBox);
                correctedPlacement = service<i_surface_manager>().display().fullscreen_video_mode();
            }
            switch (correctedPlacement.state())
            {
            default:
            case window_state::Normal:
                if (!has_parent_window())
                    make_ref<surface_window>(
                        *this,
                        [&](i_surface_window& aProxy)
                        { 
                            service<i_rendering_engine>().create_window(
                                service<i_surface_manager>(), 
                                aProxy, 
                                correctedPlacement.normal_geometry()->top_left(), 
                                correctedPlacement.normal_geometry()->extents(), 
                                title_text(), 
                                style()); 
                        });
                else
                    make_ref<surface_window>(
                        *this,
                        [&](i_surface_window& aProxy)
                        { 
                            service<i_rendering_engine>().create_window(
                                service<i_surface_manager>(), 
                                aProxy, 
                                parent_window().native_window(),
                                correctedPlacement.normal_geometry()->top_left(), 
                                correctedPlacement.normal_geometry()->extents(), 
                                title_text(), 
                                style()); 
                        });
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
        if (window::is_surface())
            return true;
        return find_surface() != nullptr;
    }

    const i_surface_window& window::surface() const
    {
        if (window::is_surface())
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

    const i_surface_window& window::real_surface() const
    {
        if (window::is_surface() && !is_nested())
            return *iSurfaceWindow;
        auto s = find_real_surface();
        if (s != nullptr)
            return *s;
        throw no_surface();
    }

    i_surface_window& window::real_surface()
    {
        return const_cast<i_surface_window&>(to_const(*this).real_surface());
    }

    void window::set_surface(i_surface_window& aSurfaceWindow)
    {
        iSurfaceWindow.reset(&aSurfaceWindow);
        iSurfaceDestroyed.emplace(aSurfaceWindow.native_surface());
    }

    bool window::has_native_surface() const
    {
        return window::is_surface() && !*iSurfaceDestroyed;
    }

    const i_native_surface& window::native_surface() const
    {
        if (!window::has_native_surface())
            throw no_native_surface();
        return surface().native_surface();
    }

    i_native_surface& window::native_surface()
    {
        if (!window::has_native_surface())
            throw no_native_surface();
        return surface().native_surface();
    }

    bool window::has_native_window() const
    {
        return window::has_native_surface() && window::surface().surface_type() == neogfx::surface_type::Window;
    }

    const i_native_window& window::native_window() const
    {
        if (!window::has_native_window())
            throw no_native_window();
        return surface().as_surface_window().native_window();
    }

    i_native_window& window::native_window()
    {
        if (!window::has_native_window())
            throw no_native_window();
        return surface().as_surface_window().native_window();
    }

    bool window::has_parent_window() const
    {
        return iParentWindow != nullptr;
    }

    const i_window& window::parent_window() const
    {
        if (iParentWindow != nullptr)
            return *iParentWindow;
        throw no_parent_window();
    }

    i_window& window::parent_window()
    {
        return const_cast<i_window&>(to_const(*this).window::parent_window());
    }

    bool window::is_parent_of(const i_window& aChildWindow) const
    {
        return aChildWindow.has_parent_window() && &aChildWindow.parent_window() == this;
    }

    bool window::is_owner_of(const i_window& aChildWindow) const
    {
        const i_window* w = &aChildWindow;
        if (w == this)
            return false;
        while (w->has_parent_window())
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
        while (w->has_parent_window())
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
        Closed();
    }

    color window::frame_color() const
    {
        if (effectively_enabled() && !base_type::has_frame_color() && is_effectively_active())
        {
            if (!is_nested())
                return service<i_app>().current_style().palette().color(color_role::Selection);
            else
                return service<i_app>().current_style().palette().color(color_role::AlternateSelection);
        }
        else
            return base_type::frame_color().with_alpha(is_effectively_active() ? 1.0 : 0.25);
    }

    void window::set_parent(i_widget& aParent)
    {
        iParentWindow = &aParent.root();
        if ((style() & window_style::Nested) == window_style::Nested && !aParent.is_root())
            base_type::set_parent(aParent);
    }

    void window::resized()
    {
        base_type::resized();
        if (has_native_window())
        {
            bool const initiallyCentered = (style() & window_style::InitiallyCentered) == window_style::InitiallyCentered;
            bool const eventCauseExternal = native_window().event_cause_external();
            bool const resizingOrMoving = native_window().resizing_or_moving();
            if (initiallyCentered && !eventCauseExternal && !resizingOrMoving)
                center_on_parent((style() & window_style::Resize) != window_style::Resize);
        }
    }

    bool window::is_managing_layout() const
    {
        return true;
    }

    void window::layout_items_completed()
    {
        base_type::layout_items_completed();
        if (iEnteredWidget != nullptr)
        {
            i_widget& widgetUnderMouse = (!surface().has_capturing_widget() ? widget_for_mouse_event(mouse_position()) : surface().capturing_widget());
            if (iEnteredWidget != &widgetUnderMouse)
                mouse_entered(mouse_position());
        }
    }

    bool window::device_metrics_available() const
    {
        return base_type::device_metrics_available();
    }

    const i_device_metrics& window::device_metrics() const
    {
        return base_type::device_metrics();
    }

    layer_t window::layer() const
    {
        return (!is_nested() ? LayerWidget : LayerNestedWindow) + base_type::layer();
    }

    widget_part window::hit_test(const point& aPosition) const
    {
        auto result = base_type::hit_test(aPosition);
        if (result.part == widget_part::Client)
            result.part = widget_part::Grab;
        return result;
    }

    neogfx::size_policy window::size_policy() const
    {
        if (base_type::has_size_policy())
            return base_type::size_policy();
        else if (has_fixed_size())
            return size_constraint::Fixed;
        else
            return size_constraint::Manual;
    }

    size window::minimum_size(optional_size const& aAvailableSpace) const
    {
        if (has_minimum_size())
            return base_type::minimum_size(aAvailableSpace);
        if ((style() & window_style::Dialog) == window_style::Dialog)
            return base_type::minimum_size(aAvailableSpace);
        if ((style() & window_style::TitleBar) == window_style::TitleBar)
            return base_type::minimum_size(aAvailableSpace).max(size{ 200_dip, 200_dip });
        return base_type::minimum_size(aAvailableSpace);
    }

    bool window::update(const rect& aUpdateRect)
    {
        if (!base_type::update(aUpdateRect))
            return false;
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
                // todo
            }
        }
        aGc.set_extents(extents());
        aGc.set_origin(origin());
        base_type::render(aGc);
        PaintOverlay(aGc);
    }

    void window::paint(i_graphics_context& aGc) const
    {
        base_type::paint(aGc);
    }

    color  window::palette_color(color_role aColorRole) const
    {
        if (has_palette_color(aColorRole))
            return widget::palette_color(aColorRole);
        if (aColorRole == color_role::Background)
            return container_background_color();
        return widget::palette_color(aColorRole);
    }

    bool window::is_dismissing_children() const
    {
        return iDismissingChildren;
    }

    bool window::can_dismiss(const i_widget* aClickedWidget) const
    {
        switch (dismissal_type())
        {
        default:
        case CannotDismiss:
            return false;
        case CloseOnDismissal:
            return aClickedWidget == nullptr ||
                ((style() & window_style::DismissOnOwnerClick) == window_style::DismissOnOwnerClick &&
                    aClickedWidget->root().is_owner_of(*this)) ||
                ((style() & window_style::DismissOnParentClick) == window_style::DismissOnParentClick &&
                    aClickedWidget->root().is_parent_of(*this));
        case HideOnDismissal:
            return aClickedWidget == nullptr ||
                ((style() & window_style::HideOnOwnerClick) == window_style::HideOnOwnerClick &&
                    aClickedWidget->root().is_owner_of(*this)) ||
                ((style() & window_style::HideOnParentClick) == window_style::HideOnParentClick &&
                    aClickedWidget->root().is_parent_of(*this));
        }
    }

    window::dismissal_type_e window::dismissal_type() const
    {
        if ((style() & (window_style::DismissOnOwnerClick | window_style::DismissOnParentClick)) != window_style::Invalid)
            return CloseOnDismissal;
        else if ((style() & (window_style::HideOnOwnerClick | window_style::HideOnParentClick)) != window_style::Invalid)
            return HideOnDismissal;
        else
            return CannotDismiss;
    }

    bool window::dismissed() const
    {
        switch (dismissal_type())
        {
        case CloseOnDismissal:
            return is_closed();
        case HideOnDismissal:
            return hidden();
        default:
            return false;
        }
    }

    void window::dismiss()
    {
        if (service<i_keyboard>().is_keyboard_grabbed_by(*this))
            service<i_keyboard>().ungrab_keyboard(*this);
        switch (dismissal_type())
        {
        case CloseOnDismissal:
            close();
            break;
        case HideOnDismissal:
            hide();
            break;
        }
    }

    void window::center(bool aSetMinimumSize)
    {
        layout_items();
        if (aSetMinimumSize)
            resize(minimum_size());
        rect desktopRect{ window_manager().desktop_rect(*this) };
        move((desktopRect.extents() - window_manager().window_rect(*this).extents()) / 2.0);
    }

    void window::center_on_parent(bool aSetMinimumSize)
    {
        if (iCentering)
            return;
        neolib::scoped_flag sf{ iCentering };
        if (has_parent_window())
        {
            layout_items();
            if (aSetMinimumSize)
                resize(minimum_size());
            rect const desktopRect{ window_manager().desktop_rect(*this) };
            rect const parentRect{ window_manager().window_rect(parent_window()) };
            rect const ourRect{ window_manager().window_rect(*this) };
            point position = point{ (parentRect.extents() - ourRect.extents()) / 2.0 } + parentRect.top_left();
            if (position.x < 0.0)
                position.x = 0.0;
            if (position.y < 0.0)
                position.y = 0.0;
            if (position.x + ourRect.cx > desktopRect.right())
                position.x = desktopRect.right() - ourRect.cx;
            if (position.y + ourRect.cy > desktopRect.bottom())
                position.y = desktopRect.bottom() - ourRect.cy;
            move(position.ceil());
        }
        else
            center(aSetMinimumSize);
    }

    void window::widget_added(i_widget& aWidget)
    {
        if (aWidget.has_parent_layout())
            layout_items(true);
    }

    void window::widget_removed(i_widget& aWidget)
    {
        if (iEnteredWidget == &aWidget)
            iEnteredWidget = nullptr;
        if (!iSurfaceDestroyed && surface().has_capturing_widget() && &surface().capturing_widget() == &aWidget)
            surface().release_capture(aWidget);
        if (iFocusedWidget == &aWidget)
            iFocusedWidget = nullptr;
        if (aWidget.has_parent_layout())
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

    focus_policy window::focus_policy() const
    {
        if (has_focus_policy())
            return widget::focus_policy();
        if (is_nested())
            return neogfx::focus_policy::StrongFocus;
        return widget::focus_policy();
    }

    void window::focus_gained(focus_reason aFocusReason)
    {
        base_type::focus_gained(aFocusReason);

        if (is_nested() && !is_active())
            activate();
    }

    void window::focus_lost(focus_reason aFocusReason)
    {
        base_type::focus_lost(aFocusReason);

        if (is_nested() && is_active())
            deactivate();
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
        i_widget* previouslyFocused = iFocusedWidget;
        iFocusedWidget = &aWidget;
        if (previouslyFocused != nullptr && previouslyFocused != iFocusedWidget)
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
        if (aEnableAncestors && has_parent_window() && (style() & window_style::NoActivate) != window_style::NoActivate)
            parent_window().activate();
    }

    scrolling_disposition window::scrolling_disposition(const i_widget& aChildWidget) const
    {
        if (has_layout(standard_layout::TitleBar) && layout(standard_layout::TitleBar).find(aChildWidget) != std::nullopt)
            return neogfx::scrolling_disposition::DontScrollChildWidget;
        else if (has_layout(standard_layout::StatusBar) && layout(standard_layout::StatusBar).find(aChildWidget) != std::nullopt)
            return neogfx::scrolling_disposition::DontScrollChildWidget;
        return base_type::scrolling_disposition(aChildWidget);
    }

    i_string const& window::title_text() const
    {
        return iTitleText;
    }

    void window::set_title_text(i_string const& aTitleText)
    {
        if (iTitleText != aTitleText)
        {
            iTitleText = aTitleText;
            if ((style() & window_style::TitleBar) == window_style::TitleBar)
                title_bar().set_title(title_text());
            if (has_native_window())
                native_window().set_title_text(title_text());
        }
    }

    bool window::is_effectively_active() const
    {
        return has_native_window() && native_window().is_effectively_active();
    }

    bool window::is_active() const
    {
        return has_native_window() && native_window().is_active();
    }

    void window::activate()
    {
        window_manager().activate_window(*this);
    }

    void window::deactivate()
    {
        window_manager().deactivate_window(*this);
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
        if (is_surface() && has_native_window())
            native_window().enable(iCountedEnable >= 0);
        else if (!is_surface())
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
        base_type::init();

        set_decoration_style(window_style_to_decoration_style(style()));

        if ((style() & (window_style::SizeToContents | window_style::Main)) == (window_style::SizeToContents | window_style::Main))
            layout(standard_layout::Dock, layout_position::Center).parent_layout().set_minimum_size({});

        if (is_fullscreen() || (service<i_app>().program_options().nest() && &ultimate_ancestor() == this))
        {
            if (is_fullscreen())
                set_frame_style(frame_style::NoFrame);
        }

        if ((style() & window_style::InitiallyHidden) == window_style::InitiallyHidden)
            hide();

        update_modality(false);

        if ((style() & window_style::TitleBar) == window_style::TitleBar)
        {
            create_title_bar();
            title_bar().set_title(title_text());
        }

        resize(native_window().surface_extents());

        set_background_opacity(1.0);

        iSink += service<i_app>().current_style_changed([this](style_aspect aAspect)
        {
            if ((aAspect & style_aspect::Color) == style_aspect::Color)
                update(true);
        });

        init_scrollbars();

        if (!placement().video_mode())
        {
            if (placement().state() == window_state::Normal)
            {
                if (placement().position_specified())
                    move(placement().normal_geometry()->top_left());
                else if ((style() & window_style::InitiallyCentered) == window_style::InitiallyCentered &&
                    (style() & window_style::Dialog) != window_style::Dialog)
                    center_on_parent(false);
            }
        }

        set_client(make_ref<client>(*this, scrollbar_style()));

        if (has_native_window())
            native_window().initialisation_complete();
    }

    bool window::ready_to_render() const
    {
        return widget::ready_to_render() && iReadyToRender;
    }

    void window::set_ready_to_render(bool aReady)
    {
        bool wasReady = ready_to_render();
        iReadyToRender = aReady;
        if (!wasReady && ready_to_render())
            update();
    }

    double window::rendering_priority() const
    {
        if (is_active())
            return 1.0;
        return 0.1;
    }

    double window::fps() const
    {
        if (has_native_surface())
            return native_surface().fps();
        else
            return 0.0;
    }

    double window::potential_fps() const
    {
        if (has_native_surface())
            return native_surface().potential_fps();
        else
            return 0.0;
    }

    bool window::tracking_mouse() const
    {
        return is_tracking();
    }

    point window::mouse_position() const
    {
        return window_manager().mouse_position(*this) - origin();
    }

    border window::window_border() const
    {
        // todo: for win32 desktop windows we need a transparent window behind for resizing
        auto const result = is_surface() ? 
            native_window().border_thickness() :
            neogfx::border{ 4.0, 4.0, 4.0, 4.0 };
        return result;
    }

    rect window::widget_part_rect(widget_part_e aWidgetPart) const
    {
        switch (aWidgetPart)
        {
        case widget_part::TitleBar:
            if ((style() & window_style::TitleBar) == window_style::TitleBar)
                return to_client_coordinates(title_bar().as_widget().non_client_rect());
            else
                return rect{};
        default:
            return rect{};
        }
    }

    bool window::has_client_widget() const
    {
        return base_type::has_client_widget();
    }

    const i_widget& window::client_widget() const
    {
        return base_type::client_widget();
    }

    i_widget& window::client_widget()
    {
        return base_type::client_widget();
    }

    void window::set_client(i_widget& aClient)
    {
        base_type::set_client(aClient);
    }

    void window::set_client(i_ref_ptr<i_widget> const& aClient)
    {
        base_type::set_client(aClient);
    }

    bool window::has_layout(standard_layout aStandardLayout) const
    {
        return base_type::has_layout(aStandardLayout);
    }

    const i_layout& window::layout(standard_layout aStandardLayout, layout_position aPosition) const
    {
        return base_type::layout(aStandardLayout, aPosition);
    }

    i_layout& window::layout(standard_layout aStandardLayout, layout_position aPosition)
    {
        return base_type::layout(aStandardLayout, aPosition);
    }

    const i_drag_drop_target& window::default_drag_drop_target() const
    {
        // dynamic_cast? Something, Something, Something, Dark Side
        return dynamic_cast<const i_drag_drop_target&>(client_widget());
    }

    i_drag_drop_target& window::default_drag_drop_target()
    {
        // dynamic_cast? Something, Something, Something, Dark Side
        return dynamic_cast<i_drag_drop_target&>(client_widget());
    }

    void window::mouse_entered(const point& aPosition)
    {
        if (iHandlingMouseEntered)
            return;
        neolib::scoped_flag sf{ iHandlingMouseEntered };
        i_widget& widgetUnderMouse = (!surface().has_capturing_widget() ? widget_for_mouse_event(aPosition) : surface().capturing_widget());
        i_widget* newEnteredWidget = &widgetUnderMouse;
        i_widget* oldEnteredWidget = iEnteredWidget;
        if (newEnteredWidget != oldEnteredWidget)
        {
            if (oldEnteredWidget != nullptr)
            {
                if (!event_consumed(oldEnteredWidget->mouse_left_event()()))
                    oldEnteredWidget->mouse_left();
            }
            iEnteredWidget = newEnteredWidget;
            if (!event_consumed(iEnteredWidget->mouse_entered_event()(aPosition)))
                iEnteredWidget->mouse_entered(aPosition);
        }
    }

    void window::mouse_left()
    {
        if (iHandlingMouseLeft)
            return;
        neolib::scoped_flag sf{ iHandlingMouseLeft };
        i_widget* oldEnteredWidget = iEnteredWidget;
        if (oldEnteredWidget != nullptr)
        {
            iEnteredWidget = nullptr;
            if (!event_consumed(oldEnteredWidget->mouse_left_event()()))
                oldEnteredWidget->mouse_left();
        }
    }

    void window::dismiss_children(const i_widget* aClickedWidget)
    {
        DismissingChildren(aClickedWidget);
        neolib::scoped_flag sf{ iDismissingChildren };
        if ((iStyle & window_style::RequiresOwnerFocus) != window_style::RequiresOwnerFocus)
        {
            for (std::size_t i = 0; i < window_manager().window_count();)
            {
                i_window& w = window_manager().window(i);
                if (w.as_widget().visible() && !w.dismissed() && is_owner_of(w) && w.can_dismiss(aClickedWidget))
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
        bool const inClientArea = (aCandidateWidget.hit_test(aClickPos - aCandidateWidget.origin()).part == widget_part::Client);
        bool const ignoreNonClientArea = (aCandidateWidget.focus_policy() & focus_policy::IgnoreNonClient) != focus_policy::IgnoreNonClient;
        focus_reason const focusReason = (inClientArea ? focus_reason::ClickClient : focus_reason::ClickNonClient);
        if (childHasFocus)
        {
            if (focused_widget().client_rect().contains(aClickPos - focused_widget().origin()))
                return;
            if ((aCandidateWidget.focus_policy() & focus_policy::KeepChildFocus) == focus_policy::KeepChildFocus)
                return;
        }
        if (aCandidateWidget.enabled() && aCandidateWidget.can_set_focus(focusReason))
        {
            if ((inClientArea || (!ignoreNonClientArea && !childHasFocus)))
                aCandidateWidget.set_focus(focusReason);
        }
        else if (aCandidateWidget.has_parent() && !childHasFocus)
            update_click_focus(aCandidateWidget.parent(), aClickPos);
    }
}