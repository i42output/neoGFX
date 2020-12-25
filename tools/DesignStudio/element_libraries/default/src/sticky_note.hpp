// sticky_note.hpp
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

#include <neogfx/neogfx.hpp>
#include <random>
#include <neogfx/app/action.hpp>
#include <neogfx/app/file_dialog.hpp>
#include <neogfx/gui/widget/widget.hpp>
#include <neogfx/gui/widget/text_edit.hpp>
#include <neogfx/gui/dialog/color_dialog.hpp>
#include <neogfx/gui/dialog/font_dialog.hpp>
#include <neogfx/tools/DesignStudio/i_element.hpp>

namespace neogfx::DesignStudio
{
    class sticky_note_text : public text_edit
    {
    public:
        using text_edit::text_edit;
    protected:
        color scrollbar_color(const i_scrollbar&) const override
        {
            return effective_background_color();
        }
    };

    class sticky_note : public image_widget, private default_clipboard_sink
    {
        typedef image_widget base_type;
    public:
        sticky_note(i_element const& aElement)
        {
            set_padding(neogfx::padding{ 0.0, 16.0_dip, 0.0, 0.0 });
            thread_local std::random_device tEntropy;
            thread_local std::mt19937 tGenerator(tEntropy());
            thread_local std::uniform_real_distribution<> tDistribution(0.0, 360.0);
            set_background_color(color::from_hsl(tDistribution(tGenerator), 1.0, 0.9));
            set_minimum_size(size{ 128.0_dip, 128.0_dip });
            iDefaultItem = make_ref<sticky_note_text>(*this, text_edit::MultiLine, frame_style::NoFrame);
            iDefaultItem->vertical_scrollbar().set_auto_hide(true);
            iDefaultItem->horizontal_scrollbar().set_auto_hide(true);
            iDefaultItem->set_ignore_non_client_mouse_events(false);
            iDefaultItem->set_focus_policy(iDefaultItem->focus_policy() | neogfx::focus_policy::ConsumeTabKey);
            iDefaultItem->set_background_opacity(0.0);
            iSink = iDefaultItem->Focus([&](neogfx::focus_event aEvent, focus_reason)
            {
                if (aEvent == neogfx::focus_event::FocusGained)
                {
                    set_ignore_mouse_events(false);
                }
                else if (aEvent == neogfx::focus_event::FocusLost)
                {
                    set_ignore_mouse_events(true);
                    if (service<i_clipboard>().sink_active() && &service<i_clipboard>().active_sink() == this)
                        service<i_clipboard>().deactivate(*this);
                }
            });
            iSink += ImageChanged([&]()
            {
                auto const placementRect = placement_rect();
                parent().move(parent().position() + (placementRect.top_left() - client_rect(false).top_left()));
                parent().set_extents(parent().minimum_size().max(parent().extents() + (placementRect.extents() - extents())));
                layout_items();
            });
            iSink += service<i_clipboard>().sink_activated([&]()
            {
                if (&service<i_clipboard>().active_sink() == &*iDefaultItem)
                    service<i_clipboard>().activate(*this);
            });
            iSink += aElement.context_menu([&](i_menu& aMenu)
            {
                auto noteColor = std::make_shared<action>("Sticky Note Color...");
                auto noteBackground = std::make_shared<action>("Sticky Note Background...");
                noteColor->Triggered([&]()
                {
                    auto oldColor = background_color();
                    color_dialog colorPicker{ *this, background_color() };
                    colorPicker.SelectionChanged([&]()
                    {
                        set_background_color(colorPicker.selected_color());
                    });
                    if (colorPicker.exec() == ng::dialog_result::Accepted)
                        set_background_color(colorPicker.selected_color());
                    else
                        set_background_color(oldColor);
                });
                noteBackground->Triggered([&]()
                {
                    auto imageFile = open_file_dialog(*this, file_dialog_spec{ "Open Image", {}, { "*.png" }, "Image Files" });
                    if (imageFile)
                        set_image(neogfx::image{ "file:///"_s + (*imageFile)[0], 1.0, texture_sampling::Normal });
                });
                aMenu.add_action(noteColor);
                aMenu.add_action(noteBackground);
                aMenu.add_separator();
            });
            iSink += iDefaultItem->ContextMenu([&](i_menu& aMenu)
            {
                aElement.context_menu().trigger(aMenu);
                auto fontFormat = std::make_shared<action>("Font...");
                auto paragraphFormat = std::make_shared<action>("Paragraph...");
                fontFormat->Triggered([&]()
                {
                    font_dialog fontPicker{ *this, iDefaultItem->font(), iDefaultItem->default_style().as_text_appearance() };
                    fontPicker.set_default_ink(iDefaultItem->default_text_color());
                    fontPicker.set_default_paper(iDefaultItem->effective_background_color());
                    fontPicker.SelectionChanged([&]()
                    {
                        iDefaultItem->set_font(fontPicker.selected_font());
                        auto s = iDefaultItem->default_style();
                        s.set_from_text_appearance(*fontPicker.selected_appearance());
                        iDefaultItem->set_default_style(s);
                    });
                    if (fontPicker.exec() == dialog_result::Accepted)
                    {
                        iDefaultItem->set_font(fontPicker.selected_font());
                        auto s = iDefaultItem->default_style();
                        s.set_from_text_appearance(*fontPicker.selected_appearance());
                        iDefaultItem->set_default_style(s);
                    }
                    else
                    {
                        iDefaultItem->set_font(fontPicker.current_font());
                        auto s = iDefaultItem->default_style();
                        s.set_from_text_appearance(*fontPicker.current_appearance());
                        iDefaultItem->set_default_style(s);
                    }
                });
                paragraphFormat->disable(); // todo
                aMenu.add_action(fontFormat);
                aMenu.add_action(paragraphFormat);
                aMenu.add_separator();
            });
            add(iDefaultItem);
        }
    protected:
        widget_part part(const point& aPosition) const override
        {
            if (!image().is_empty() && !placement_rect().contains(aPosition))
                return { *this, widget_part::Nowhere };
            return base_type::part(aPosition);
        }
    protected:
        void layout_items(bool aDefer = false) override
        {
            base_type::layout_items(aDefer);
            if (iDefaultItem != nullptr)
            {
                auto const placementRect = (image().is_empty() ? client_rect(false) : placement_rect().deflate(padding()));
                iDefaultItem->move(placementRect.top_left());
                iDefaultItem->resize(placementRect.extents());
            }
        }
    protected:
        void focus_gained(focus_reason) override
        {
            if (iDefaultItem)
                iDefaultItem->set_focus();
        }
    protected:
        void paint(i_graphics_context& aGc) const override
        {
            base_type::paint(aGc);
            auto placementRect = image().is_empty() ? client_rect() : placement_rect();
            placementRect.cy = padding().top;
            if (image().is_empty())
                aGc.fill_rect(placementRect, background_color().to_hsl().shade(0.05).to_rgb<color>());
            else
                aGc.fill_rect(placementRect, color::Black.with_alpha(0x80));
        }
    protected:
        bool has_background_opacity() const override
        {
            return image().is_empty() ? base_type::has_background_opacity() : true;
        }
        double background_opacity() const override
        {
            return image().is_empty() ? base_type::background_opacity() : 0.0;
        }
        color palette_color(color_role aColorRole) const override
        {
            if (aColorRole != color_role::Background || image().is_empty())
                return base_type::palette_color(aColorRole);
            return color::Black.shaded(0x20);
        }
    protected:
        const i_widget& get_widget_at(const point& aPosition) const override
        {
            auto& result = base_type::get_widget_at(aPosition);
            if (&result == this)
                return parent();
            return result;
        }
    protected:
        bool can_undo() const override
        {
            return iDefaultItem->can_undo();
        }
        bool can_redo() const override
        {
            return iDefaultItem->can_redo();
        }
        bool can_cut() const override
        {
            return iDefaultItem->can_cut();
        }
        bool can_copy() const override
        {
            return iDefaultItem->can_copy();
        }
        bool can_paste() const override
        {
            return service<i_clipboard>().has_image() || iDefaultItem->can_paste();
        }
        bool can_delete_selected() const override
        {
            return iDefaultItem->can_delete_selected();
        }
        bool can_select_all() const override
        {
            return iDefaultItem->can_delete_selected();
        }
        void undo(i_clipboard& aClipboard) override
        {
            iDefaultItem->undo(aClipboard);
        }
        void redo(i_clipboard& aClipboard) override
        {
            iDefaultItem->redo(aClipboard);
        }
        void cut(i_clipboard& aClipboard) override
        {
            iDefaultItem->cut(aClipboard);
        }
        void copy(i_clipboard& aClipboard) override
        {
            iDefaultItem->copy(aClipboard);
        }
        void paste(i_clipboard& aClipboard) override
        {
            if (aClipboard.has_image())
                set_image(neogfx::image{ aClipboard.image(), texture_sampling::Normal });
            else if (aClipboard.has_text())
                iDefaultItem->paste(aClipboard);
        }
        void delete_selected() override
        {
            iDefaultItem->delete_selected();
        }
        void select_all() override
        {
            iDefaultItem->select_all();
        }
    private:
        ref_ptr<sticky_note_text> iDefaultItem;
        sink iSink;
    };
}
