// widget_caddy.hpp
/*
  neoGFX Design Studio
  Copyright(C) 2020 Leigh Johnston
  
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

#include <neogfx/tools/DesignStudio/DesignStudio.hpp>
#include <neogfx/app/i_clipboard.hpp>
#include <neogfx/gui/layout/vertical_layout.hpp>
#include <neogfx/gui/widget/widget.hpp>
#include <neogfx/tools/DesignStudio/i_element.hpp>
#include <neogfx/tools/DesignStudio/i_project.hpp>
#include <neogfx/tools/DesignStudio/context_menu.hpp>

namespace neogfx
{
    extern template class widget<DesignStudio::i_element_caddy>;
}

namespace neogfx::DesignStudio
{
    class widget_caddy : public widget<i_element_caddy>, private i_clipboard_sink
    {
    private:
        struct drag_info
        {
            cardinal part;
            point dragFrom;
            bool wasDragged;
        };
    public:
        widget_caddy(i_project& aProject, i_element& aElement, i_widget& aParent, const point& aPosition);
        ~widget_caddy();
    public:
        bool has_element() const;
        i_element& element() const;
        i_layout_item& item() const;
    public:
        size minimum_size(optional_size const& aAvailableSpace = {}) const override;
    protected:
        neogfx::widget_type widget_type() const override;
        neogfx::padding padding() const override;
    protected:
        void layout_items(bool aDefer = false) override;
    protected:
        int32_t layer() const override;
    protected:
        int32_t render_layer() const override;
        void paint_non_client_after(i_graphics_context& aGc) const override;
    protected:
        neogfx::focus_policy focus_policy() const override;
        void focus_gained(focus_reason aFocusReason) override;
        void focus_lost(focus_reason aFocusReason) override;
    protected:
        bool key_pressed(scan_code_e aScanCode, key_code_e aKeyCode, key_modifiers_e aKeyModifiers) override;
    protected:
        bool ignore_mouse_events(bool aConsiderAncestors = true) const override;
        void mouse_button_pressed(mouse_button aButton, const point& aPosition, key_modifiers_e aKeyModifiers) override;
        void mouse_button_released(mouse_button aButton, const point& aPosition) override;
        void mouse_moved(const point& aPosition, key_modifiers_e aKeyModifiers) override;
        void mouse_entered(const point& aPosition) override;
        void mouse_left() override;
        neogfx::mouse_cursor mouse_cursor() const override;
    protected:
        void start_drag(cardinal aPart, point const& aPosition) override;
        void drag(point const& aPosition, bool aIgnoreConstraints) override;
        void end_drag() override;
    protected:
        bool can_undo() const override;
        bool can_redo() const override;
        bool can_cut() const override;
        bool can_copy() const override;
        bool can_paste() const override;
        bool can_delete_selected() const override;
        bool can_select_all() const override;
        void undo(i_clipboard& aClipboard) override;
        void redo(i_clipboard& aClipboard) override;
        void cut(i_clipboard& aClipboard) override;
        void copy(i_clipboard& aClipboard) override;
        void paste(i_clipboard& aClipboard) override;
        void delete_selected() override;
        void select_all() override;
    private:
        std::optional<cardinal> resize_part_at(point const& aPosition) const;
        rect resizer_part_rect(cardinal aPart, bool aForHitTest = true) const;
    private:
        sink iSink;
        i_project& iProject;
        weak_ref_ptr<i_element> iElement;
        weak_ref_ptr<i_layout_item> iItem;
        neolib::callback_timer iAnimator;
        std::optional<drag_info> iDragInfo;
    };
}
