// spliter.hpp
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
#include <neogfx/gui/widget/widget.hpp>

namespace neogfx
{
    enum class splitter_style : uint32_t
    {
        None                = 0x00000000,
        Horizontal          = 0x00000001,
        Vertical            = 0x00000002,
        ResizeSinglePane    = 0x00001000,
        DrawGrip            = 0x10000000
    };

    inline constexpr splitter_style operator|(splitter_style aLhs, splitter_style aRhs)
    {
        return static_cast<splitter_style>(static_cast<uint32_t>(aLhs) | static_cast<uint32_t>(aRhs));
    }

    inline constexpr splitter_style operator&(splitter_style aLhs, splitter_style aRhs)
    {
        return static_cast<splitter_style>(static_cast<uint32_t>(aLhs) & static_cast<uint32_t>(aRhs));
    }

    class splitter : public widget<>
    {
    private:
        typedef std::pair<uint32_t, uint32_t> separator_type;
    public:
        splitter(splitter_style aStyle = splitter_style::Horizontal | splitter_style::DrawGrip);
        splitter(i_widget& aParent, splitter_style aStyle = splitter_style::Horizontal | splitter_style::DrawGrip);
        splitter(i_layout& aLayout, splitter_style aStyle = splitter_style::Horizontal | splitter_style::DrawGrip);
        ~splitter();
    public:
        neogfx::size_policy size_policy() const override;
    public:
        i_widget& get_widget_at(const point& aPosition) override;
    public:
        void mouse_button_pressed(mouse_button aButton, const point& aPosition, key_modifiers_e aKeyModifiers) override;
        void mouse_button_double_clicked(mouse_button aButton, const point& aPosition, key_modifiers_e aKeyModifiers) override;
        void mouse_moved(const point& aPosition, key_modifiers_e aKeyModifiers) override;
        void mouse_entered(const point& aPosition) override;
        void mouse_left() override;
        neogfx::mouse_cursor mouse_cursor() const override;
        void capture_released() override;
    public:
        virtual void panes_resized();
        virtual void reset_pane_sizes_requested(const std::optional<uint32_t>& aPane = {});
    protected:
        void paint(i_graphics_context& aGc) const override;
    private:
        void init();
        std::optional<separator_type> separator_at(const point& aPosition) const;
    private:
        splitter_style iStyle;
        std::optional<separator_type> iTracking;
        std::pair<size, size> iSizeBeforeTracking;
        point iTrackFrom;
    };
}