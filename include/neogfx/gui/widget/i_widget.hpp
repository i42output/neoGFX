// i_widget.hpp
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

#pragma once

#include <neogfx/neogfx.hpp>
#include <unordered_set>
#include <neogfx/core/event.hpp>
#include <neogfx/gfx/i_graphics_context.hpp>
#include <neogfx/hid/mouse.hpp>
#include <neogfx/hid/i_keyboard.hpp>
#include <neogfx/app/i_style.hpp>
#include <neogfx/app/palette.hpp>
#include <neogfx/gui/window/i_window.hpp>
#include <neogfx/gui/window/window_events.hpp>
#include <neogfx/gui/layout/i_layout_item.hpp>
#include <neogfx/gui/widget/i_skinnable_item.hpp>
#include <neogfx/gui/widget/widget_bits.hpp>

namespace neogfx
{
    class i_surface;
    class i_layout;

    class i_widget : public i_layout_item, public i_keyboard_handler, public virtual i_skinnable_item
    {
    public:
        declare_event(visibility_changed)
        declare_event(position_changed)
        declare_event(size_changed)
        declare_event(layout_completed)
        declare_event(painting, i_graphics_context&)
        declare_event(painting_children, i_graphics_context&)
        declare_event(painted, i_graphics_context&)
        declare_event(mouse_event, const neogfx::mouse_event&)
        declare_event(non_client_mouse_event, const neogfx::non_client_mouse_event&)
        declare_event(keyboard_event, const neogfx::keyboard_event&)
        declare_event(focus_event, neogfx::focus_event)
    public:
        typedef i_widget abstract_type;
        typedef std::vector<std::shared_ptr<i_widget>> widget_list;
    protected:
        typedef std::unordered_set<rect> update_rect_list;
    public:
        struct no_parent : std::logic_error { no_parent() : std::logic_error("neogfx::i_widget::no_parent") {} };
        struct no_root : std::logic_error { no_root() : std::logic_error("neogfx::i_widget::no_root") {} };
        struct no_surface : std::logic_error { no_surface() : std::logic_error("neogfx::i_widget::no_surface") {} };
        struct no_children : std::logic_error { no_children() : std::logic_error("neogfx::i_widget::no_children") {} };
        struct not_child : std::logic_error { not_child() : std::logic_error("neogfx::i_widget::not_child") {} };
        struct no_update_rect : std::logic_error { no_update_rect() : std::logic_error("neogfx::i_widget::no_update_rect") {} };
        struct widget_not_entered : std::logic_error { widget_not_entered() : std::logic_error("neogfx::i_widget::widget_not_entered") {} };
        struct widget_cannot_capture : std::logic_error { widget_cannot_capture() : std::logic_error("neogfx::i_widget::widget_cannot_capture") {} };
        struct widget_not_focused : std::logic_error { widget_not_focused() : std::logic_error("neogfx::i_widget::widget_not_focused") {} };
        struct widget_cannot_defer_layout : std::logic_error { widget_cannot_defer_layout() : std::logic_error("neogfx::i_widget::widget_cannot_defer_layout") {} };
        struct layout_already_set : std::logic_error { layout_already_set() : std::logic_error("neogfx::i_widget::layout_already_set") {} };
        struct no_layout : std::logic_error { no_layout() : std::logic_error("neogfx::i_widget::no_layout") {} };
    public:
        virtual ~i_widget() = default;
    public:
        virtual bool is_singular() const = 0;
        virtual void set_singular(bool aSingular) = 0;
        virtual bool has_root() const = 0;
        virtual bool is_root() const = 0;
        virtual const i_window& root() const = 0;
        virtual i_window& root() = 0;
        virtual bool has_parent() const = 0;
        virtual const i_widget& parent() const = 0;
        virtual i_widget& parent() = 0;
        virtual void set_parent(i_widget& aParent) = 0;
        virtual void parent_changed() = 0;
        virtual bool adding_child() const = 0;
        virtual i_widget& add(i_widget& aChild) = 0;
        virtual i_widget& add(std::shared_ptr<i_widget> aChild) = 0;
        virtual std::shared_ptr<i_widget> remove(i_widget& aChild, bool aSingular = false) = 0;
        virtual void remove_all() = 0;
        virtual bool has_children() const = 0;
        virtual const widget_list& children() const = 0;
        virtual widget_list::const_iterator last() const = 0;
        virtual widget_list::iterator last() = 0;
        virtual widget_list::const_iterator find(const i_widget& aChild, bool aThrowIfNotFound = true) const = 0;
        virtual widget_list::iterator find(const i_widget& aChild, bool aThrowIfNotFound = true) = 0;
    public:
        virtual const i_widget& before() const = 0;
        virtual i_widget& before() = 0;
        virtual const i_widget& after() const = 0;
        virtual i_widget& after() = 0;
        virtual void link_before(i_widget* aPreviousWidget) = 0;
        virtual void link_after(i_widget* aNextWidget) = 0;
        virtual void unlink() = 0;
    public:
        virtual bool has_layout() const = 0;
        virtual void set_layout(i_layout& aLayout, bool aMoveExistingItems = true) = 0;
        virtual void set_layout(std::shared_ptr<i_layout> aLayout, bool aMoveExistingItems = true) = 0;
        virtual const i_layout& layout() const = 0;
        virtual i_layout& layout() = 0;
        virtual bool can_defer_layout() const = 0;
        virtual bool has_layout_manager() const = 0;
        virtual const i_widget& layout_manager() const = 0;
        virtual i_widget& layout_manager() = 0;
        virtual bool is_managing_layout() const = 0;
        virtual bool has_parent_layout() const = 0;
        virtual const i_layout& parent_layout() const = 0;
        virtual i_layout& parent_layout() = 0;
        virtual void layout_items(bool aDefer = false) = 0;
        virtual void layout_items_started() = 0;
        virtual bool layout_items_in_progress() const = 0;
        virtual void layout_items_completed() = 0;
    public:
        virtual bool has_logical_coordinate_system() const = 0;
        virtual neogfx::logical_coordinate_system logical_coordinate_system() const = 0;
        virtual void set_logical_coordinate_system(const optional_logical_coordinate_system& aLogicalCoordinateSystem) = 0;
        virtual point position() const = 0;
        virtual point origin() const = 0;
        virtual rect non_client_rect() const = 0;
        virtual rect client_rect(bool aIncludePadding = true) const = 0;
        virtual void move(const point& aPosition) = 0;
        virtual void moved() = 0;
        virtual void parent_moved() = 0;
        virtual size extents() const = 0;
        virtual void resize(const size& aSize) = 0;
        virtual void resized() = 0;
        virtual const i_widget& get_widget_at(const point& aPosition) const = 0;
        virtual i_widget& get_widget_at(const point& aPosition) = 0;
        virtual neogfx::widget_type widget_type() const = 0;
        virtual bool part_active(widget_part aPart) const = 0;
        virtual widget_part part(const point& aPosition) const = 0;
        virtual widget_part hit_test(const point& aPosition) const = 0;
    public:
        virtual bool update(const rect& aUpdateRect) = 0;
        virtual bool requires_update() const = 0;
        virtual rect update_rect() const = 0;
        virtual rect default_clip_rect(bool aIncludeNonClient = false) const = 0;
        virtual bool ready_to_render() const = 0;
        virtual void render(i_graphics_context& aGc) const = 0;
        virtual bool transparent_background() const = 0;
        virtual void paint_non_client(i_graphics_context& aGc) const = 0;
        virtual void paint_non_client_after(i_graphics_context& aGc) const = 0;
        virtual void paint(i_graphics_context& aGc) const = 0;
    public:
        virtual double opacity() const = 0;
        virtual void set_opacity(double aOpacity) = 0;
        virtual double transparency() const = 0;
        virtual void set_transparency(double aTransparency) = 0;
        virtual bool has_palette() const = 0;
        virtual const i_palette& palette() const = 0;
        virtual void set_palette(const i_palette& aPalette) = 0;
        virtual bool has_palette_color(color_role aColorRole) const = 0;
        virtual color palette_color(color_role aColorRole) const = 0;
        virtual void set_palette_color(color_role aColorRole, const optional_color& aColor) = 0;
        virtual neogfx::color container_background_color() const = 0;
        virtual bool has_font_role() const = 0;
        virtual neogfx::font_role font_role() const = 0;
        virtual void set_font_role(const optional_font_role& aFontRole) = 0;
        virtual bool has_font() const = 0;
        virtual const neogfx::font& font() const = 0;
        virtual void set_font(const optional_font& aFont) = 0;
    public:
        virtual bool visible() const = 0;
        virtual bool effectively_visible() const = 0;
        virtual bool hidden() const = 0;
        virtual bool effectively_hidden() const = 0;
        virtual bool show(bool aVisible) = 0;
        virtual bool enabled() const = 0;
        virtual bool effectively_enabled() const = 0;
        virtual bool disabled() const = 0;
        virtual bool effectively_disabled() const = 0;
        virtual bool enable(bool aEnable) = 0;
        virtual bool entered(bool aChildEntered = false) const = 0;
        virtual bool can_capture() const = 0;
        virtual bool capturing() const = 0;
        virtual const optional_point& capture_position() const = 0;
        virtual void set_capture(capture_reason aReason = capture_reason::Other, const optional_point& aPosition = {}) = 0;
        virtual void release_capture(capture_reason aReason = capture_reason::Other) = 0;
        virtual void non_client_set_capture() = 0;
        virtual void non_client_release_capture() = 0;
        virtual void captured() = 0;
        virtual void capture_released() = 0;
        virtual neogfx::focus_policy focus_policy() const = 0;
        virtual void set_focus_policy(neogfx::focus_policy aFocusPolicy) = 0;
        virtual bool can_set_focus(focus_reason aFocusReason) const = 0;
        virtual bool has_focus() const = 0;
        virtual bool child_has_focus() const = 0;
        virtual void set_focus(focus_reason aFocusReason = focus_reason::Other) = 0;
        virtual void release_focus() = 0;
        virtual void focus_gained(focus_reason aFocusReason) = 0;
        virtual void focus_lost(focus_reason aFocusReason) = 0;
    public:
        virtual bool ignore_mouse_events() const = 0;
        virtual void set_ignore_mouse_events(bool aIgnoreMouseEvents) = 0;
        virtual bool ignore_non_client_mouse_events() const = 0;
        virtual void set_ignore_non_client_mouse_events(bool aIgnoreNonClientMouseEvents) = 0;
        virtual bool mouse_event_is_non_client() const = 0;
        virtual void mouse_wheel_scrolled(mouse_wheel aWheel, const point& aPosition, delta aDelta, key_modifiers_e aKeyModifiers) = 0;
        virtual void mouse_button_pressed(mouse_button aButton, const point& aPosition, key_modifiers_e aKeyModifiers) = 0;
        virtual void mouse_button_double_clicked(mouse_button aButton, const point& aPosition, key_modifiers_e aKeyModifiers) = 0;
        virtual void mouse_button_released(mouse_button aButton, const point& aPosition) = 0;
        virtual void mouse_moved(const point& aPosition, key_modifiers_e aKeyModifiers) = 0;
        virtual void mouse_entered(const point& aPosition) = 0;
        virtual void mouse_left() = 0;
        virtual neogfx::mouse_cursor mouse_cursor() const = 0;
    public:
        virtual const i_widget& widget_for_mouse_event(const point& aPosition, bool aForHitTest = false) const = 0;
        virtual i_widget& widget_for_mouse_event(const point& aPosition, bool aForHitTest = false) = 0;
        // helpers
    public:
        template <typename WidgetType, typename... Args>
        WidgetType& emplace(Args&&... args)
        {
            auto newWidget = std::make_shared<WidgetType>(std::forward<Args>(args)...);
            add(newWidget);
            return *newWidget;
        }
    public:
        bool is_surface() const
        {
            return is_root() && root().is_surface();
        }
        bool has_surface() const
        {
            return has_root() && root().has_surface();
        }
        const i_surface& surface() const
        {
            return root().surface();
        }
        i_surface& surface()
        {
            return root().surface();
        }
        bool same_surface(const i_widget& aWidget) const
        {
            return has_surface() && aWidget.has_surface() &&
                &surface() == &aWidget.surface();
        }
    public:
        bool is_parent_of(const i_widget& aWidget) const
        {
            return aWidget.has_parent() && &aWidget.parent() == this;
        }
        bool is_ancestor_of(const i_widget& aWidget) const
        {
            const i_widget* w = &aWidget;
            while (w->has_parent())
            {
                w = &w->parent();
                if (w == this)
                    return true;
            }
            return false;
        }
        bool is_descendent_of(const i_widget& aWidget) const
        {
            return aWidget.is_ancestor_of(*this);
        }
        bool is_sibling_of(const i_widget& aWidget) const
        {
            return has_parent() && aWidget.has_parent() && &parent() == &aWidget.parent();
        }
    public:
        void layout_root(bool aDefer = false)
        {
            if (has_root())
                root().as_widget().layout_items(aDefer);
        }
    public:
        point to_window_coordinates(const point& aClientCoordinates) const
        {
            return aClientCoordinates + non_client_rect().top_left();
        }
        rect to_window_coordinates(const rect& aClientCoordinates) const
        {
            return aClientCoordinates + non_client_rect().top_left();
        }
        point to_client_coordinates(const point& aWindowCoordinates) const
        {
            return aWindowCoordinates - non_client_rect().top_left();
        }
        rect to_client_coordinates(const rect& aWindowCoordinates) const
        {
            return aWindowCoordinates - non_client_rect().top_left();
        }
    public:
        bool can_update() const
        {
            return has_root() && (root().has_native_surface() || root().is_nested()) && !effectively_hidden() && !layout_items_in_progress();
        }
        bool update(bool aIncludeNonClient = false)
        {
            if (!can_update())
                return false;
            return update(aIncludeNonClient ? to_client_coordinates(non_client_rect()) : client_rect());
        }
    public:
        bool show()
        {
            return show(true);
        }
        bool hide()
        {
            return show(false);
        }
        bool enable()
        {
            return enable(true);
        }
        bool disable()
        {
            return enable(false);
        }
     public:
         bool has_background_color() const
         {
             return has_palette_color(color_role::Background);
         }
         color background_color() const
         {
             return palette_color(color_role::Background);
         }
         void set_background_color(const optional_color& aBackgroundColor = optional_color{})
         {
             set_palette_color(color_role::Background, aBackgroundColor);
         }
         bool has_foreground_color() const
         {
             return has_palette_color(color_role::Foreground);
         }
         color foreground_color() const
         {
             return palette_color(color_role::Foreground);
         }
         void set_foreground_color(const optional_color& aForegroundColor = optional_color{})
         {
             set_palette_color(color_role::Foreground, aForegroundColor);
         }
         bool has_base_color() const
         {
             return has_palette_color(color_role::Base);
         }
         color base_color() const
         {
             return palette_color(color_role::Base);
         }
         void set_base_color(const optional_color& aBaseColor = optional_color{})
         {
             set_palette_color(color_role::Base, aBaseColor);
         }
         bool has_alternate_base_color() const
         {
             return has_palette_color(color_role::AlternateBase);
         }
         color alternate_base_color() const
         {
             return palette_color(color_role::AlternateBase);
         }
         void set_alternate_base_color(const optional_color& aAlternateBaseColor = optional_color{})
         {
             set_palette_color(color_role::AlternateBase, aAlternateBaseColor);
         }
    };

    // todo: make being a layout manager a property?
    template <typename WidgetType, typename... OtherBases>
    class layout_manager : public WidgetType, public OtherBases...
    {
    public:
        typedef WidgetType widget_type;
    public:
        template <typename... Args>
        layout_manager(Args&&... aArgs) :
            widget_type{ std::forward<Args>(aArgs)... }
        {
        }
    public:
        bool is_managing_layout() const override
        {
            return true;
        }
    };
}