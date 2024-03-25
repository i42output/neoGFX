// progress_bar.hpp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2022 Leigh Johnston.  All Rights Reserved.
  
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
#include <neogfx/gui/layout/spacer.hpp>
#include <neogfx/gui/layout/grid_layout.hpp>
#include <neogfx/gui/widget/label.hpp>
#include <neogfx/gui/widget/i_progress_bar.hpp>

namespace neogfx
{   
    class progress_bar : public widget<>, public i_progress_bar
    {
        meta_object(widget<>)
    public:
        class bar : public widget<>
        {
        public:
            bar(progress_bar& aOwner);
        public:
            size minimum_size(optional_size const& aAvailableSpace = optional_size{}) const override;
        public:
            void paint(i_graphics_context& aGc) const override;
        private:
            progress_bar& iOwner;
        };
    public:
        progress_bar(progress_bar_style aStyle = progress_bar_style::Default);
        progress_bar(i_widget& aParent, progress_bar_style aStyle = progress_bar_style::Default);
        progress_bar(i_layout& aLayout, progress_bar_style aStyle = progress_bar_style::Default);
    public:
        progress_bar_style style() const override;
        void set_style(progress_bar_style aStyle) override;
        i_string const& text() const override;
        void set_text(i_string const& aText) override;
    public:
        scalar value() const override;
        void set_value(scalar aValue) override;
        scalar minimum() const override;
        void set_minimum(scalar aMinimum) override;
        scalar maximum() const override;
        void set_maximum(scalar aMaximum) override;
    public:
        rect bar_rect() const override;
        i_text_widget const& text_label() const override;
        i_text_widget& text_label() override;
        i_string const& value_as_text() const override;
    private:
        void init();
        void placement_changed();
        void changed();
        void animate();
    private:
        progress_bar_style iStyle;
        grid_layout iLayout;
        bar iBar;
        label iLabel;
        string iText = "%pct%%%";
        scalar iValue = 0.0;
        scalar iMinimum = 0.0;
        scalar iMaximum = 100.0;
        string iValueAsText;
        widget_timer iAnimator;
    };
}