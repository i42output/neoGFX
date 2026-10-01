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

namespace neolib
{
    class i_setting;
}

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
        bool has_item() const;
        i_layout_item& item() const;
    public:
        bool nested() const;
    public:
        neogfx::size_policy size_policy() const override;
        size minimum_size(optional_size const& aAvailableSpace = {}) const override;
    protected:
        neogfx::widget_type widget_type() const override;
        neogfx::padding padding() const override;
    protected:
        void layout_items(bool aDefer = false) override;
    protected:
        layer_t layer() const override;
    protected:
        layer_t render_layer() const override;
        void paint(i_graphics_context& aGc) const override;
        void paint_non_client_after(i_graphics_context& aGc) const override;
    protected:
        neogfx::focus_policy focus_policy() const override;
        void focus_gained(focus_reason aFocusReason) override;
        void focus_lost(focus_reason aFocusReason) override;
    protected:
        bool key_pressed(scan_code_e aScanCode, key_code_e aKeyCode, key_modifier aKeyModifier) override;
    protected:
        bool ignore_mouse_events(bool aConsiderAncestors = true) const override;
        void mouse_button_clicked(mouse_button aButton, const point& aPosition, key_modifier aKeyModifier) override;
        void mouse_button_double_clicked(mouse_button aButton, const point& aPosition, key_modifier aKeyModifier) override;
        void mouse_button_released(mouse_button aButton, const point& aPosition) override;
        void mouse_moved(const point& aPosition, key_modifier aKeyModifier) override;
        void mouse_entered(const point& aPosition) override;
        void mouse_left() override;
        neogfx::mouse_cursor mouse_cursor() const override;
    private:
        void apply_preview_mode();
        void end_text_edit(bool aCommit);
        bool can_be_dropped() const;
        bool capturing_drop() const;
        void update_drop_target(point const& aPosition);
        void move_to(i_element& aContainer, point const& aDropPosition);
        void move_to_canvas(point const& aDropPosition);
    public:
        void begin_text_edit() override;
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
        std::optional<cardinal> cardinal_at(point const& aPosition) const;
        std::optional<cardinal> cardinal_at(point const& aPosition, key_modifier aKeyModifier) const;
        rect cardinal_rect(cardinal aPart, bool aForHitTest = true) const;
    private:
        sink iSink;
        i_project& iProject;
        weak_ref_ptr<i_element> iElement;
        weak_ref_ptr<i_layout_item> iItem;
        widget_timer iAnimator;
        std::optional<drag_info> iDragInfo;
        neolib::i_setting* iShowLayoutIcons = nullptr;
        i_element* iDropTarget = nullptr;
        point iDropPosition;
        ref_ptr<i_widget> iDropHighlight;
        bool iDropCandidate = false;
        ref_ptr<i_widget> iTextEditor;
        std::optional<bool> iEndTextEdit; // end in-place text edit (true: commit) at next opportunity
    };

    // preview mode: widgets behave as in a running application (no editing)
    bool preview_mode();
    void set_preview_mode(bool aPreview);
    neolib::i_event<> const& preview_mode_changed();
    // display ids rather than text in the editor (never in preview mode)
    bool display_ids();
    void set_display_ids(bool aDisplayIds);
    neolib::i_event<> const& display_ids_changed();
    bool show_ids();
    void set_text_attribute(i_element& aElement, std::string const& aText);

    // design surface helpers
    rect design_rect(i_widget const& aWidget);
    i_element* find_drop_container(i_element& aRoot, i_element const& aDropped, point const& aDropPosition);
    void add_to_container(i_project& aProject, i_element& aElement, optional_point const& aDropPosition = {});
    void create_caddies(i_project& aProject, i_widget& aWorkspace);
    void show_drop_highlight(i_element& aContainer, i_string const& aChildType, ref_ptr<i_widget>& aHighlight, point const& aDropPosition, i_widget const* aExclude = nullptr);
    void hide_drop_highlight(ref_ptr<i_widget>& aHighlight);
    bool can_be_moved(i_element const& aElement);
    void move_element_to_container(i_project& aProject, i_element& aElement, i_element& aContainer, i_element const* aBefore = nullptr);
    void move_element_to_canvas(i_project& aProject, i_element& aElement, i_element& aNewParent, i_widget& aWorkspace, point const& aDropPosition);
    void remove_caddies(i_project& aProject);
}
