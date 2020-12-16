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
#include <neogfx/gui/widget/widget.hpp>
#include <neogfx/gui/widget/text_edit.hpp>
#include <neogfx/gui/dialog/color_dialog.hpp>
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

    class sticky_note : public widget<>
    {
        typedef widget<> base_type;
    public:
        sticky_note(i_element const& aElement)
        {
            thread_local std::random_device tEntropy;
            thread_local std::mt19937 tGenerator(tEntropy());
            thread_local std::uniform_real_distribution<> tDistribution(0.0, 360.0);
            set_background_color(color::from_hsl(tDistribution(tGenerator), 1.0, 0.9));
            set_minimum_size(size{ 128.0_dip, 128.0_dip });
            auto defaultItem = make_ref<sticky_note_text>(*this, text_edit::MultiLine, frame_style::NoFrame);
            defaultItem->set_ignore_non_client_mouse_events(false);
            defaultItem->set_focus_policy(defaultItem->focus_policy() | neogfx::focus_policy::ConsumeTabKey);
            defaultItem->set_background_opacity(0.0);
            iSink = defaultItem->Focus([&](neogfx::focus_event aEvent, focus_reason)
            {
                if (aEvent == neogfx::focus_event::FocusGained)
                {
                    set_ignore_mouse_events(false);
                }
                else if (aEvent == neogfx::focus_event::FocusLost)
                {
                    set_ignore_mouse_events(true);
                }
            });
            iSink += aElement.context_menu([&](i_menu& aMenu)
            {
                auto noteColor = std::make_shared<action>("Sticky Note Color...");
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
                aMenu.add_action(noteColor);
                aMenu.add_separator();
            });
            iSink += defaultItem->ContextMenu([&](i_menu& aMenu)
            {
                aElement.context_menu().trigger(aMenu);
                auto fontFormat = std::make_shared<action>("Font...");
                auto paragraphFormat = std::make_shared<action>("Paragraph...");
                paragraphFormat->disable(); // todo
                aMenu.add_action(fontFormat);
                aMenu.add_action(paragraphFormat);
                aMenu.add_separator();
            });
            iDefaultItem = defaultItem;
            add(defaultItem);
        }
    protected:
        neogfx::padding padding() const override
        {
            if (has_padding())
                return widget::padding();
            auto result = widget::padding();
            result.top = 16.0_dip;
            return result;
        }
        void layout_items(bool aDefer = false) override
        {
            widget::layout_items(aDefer);
            if (iDefaultItem != nullptr)
            {
                iDefaultItem->move(client_rect(false).top_left());
                iDefaultItem->resize(client_rect(false).extents());
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
            auto cr = client_rect(true);
            cr.cy = padding().top;
            aGc.fill_rect(cr, background_color().to_hsl().shade(0.05).to_rgb<color>());
        }
    protected:
        const i_widget& get_widget_at(const point& aPosition) const override
        {
            auto& result = base_type::get_widget_at(aPosition);
            if (&result == this)
                return parent();
            return result;
        }
    private:
        ref_ptr<i_widget> iDefaultItem;
        sink iSink;
    };
}
