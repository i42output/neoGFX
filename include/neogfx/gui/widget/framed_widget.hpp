// framed_widget.hpp
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
    enum class frame_style
    {
        NoFrame,
        DottedFrame,
        DashedFrame,
        SolidFrame,
        ContainerFrame,
        DoubleFrame,
        GrooveFrame,
        RidgeFrame,
        InsetFrame,
        OutsetFrame,
        HiddenFrame,
        WindowFrame
    };

    template <typename Base = widget<>>
    class framed_widget : public Base
    {
        meta_object(Base)
        typedef framed_widget<Base> self_type;
    public:
        template <typename... Args>
        framed_widget(Args&&... aArgs) :
            base_type{ std::forward<Args>(aArgs)... }, iStyle{ frame_style::SolidFrame }, iLineWidth{ 1.0 }
        {
            base_type::set_background_opacity(1.0);
        }
        template <typename... Args>
        framed_widget(i_widget& aParent, Args&&... aArgs) :
            base_type{ aParent, std::forward<Args>(aArgs)... }, iStyle{ frame_style::SolidFrame }, iLineWidth{ 1.0 }
        {
            base_type::set_background_opacity(1.0);
        }
        template <typename... Args>
        framed_widget(i_layout& aLayout, Args&&... aArgs) :
            base_type{ aLayout, std::forward<Args>(aArgs)... }, iStyle{ frame_style::SolidFrame }, iLineWidth{ 1.0 }
        {
            base_type::set_background_opacity(1.0);
        }
        template <typename... Args>
        framed_widget(dimension aLineWidth, Args&&... aArgs) :
            base_type{ std::forward<Args>(aArgs)... }, iStyle{ frame_style::SolidFrame }, iLineWidth{ aLineWidth }
        {
            base_type::set_background_opacity(1.0);
        }
        template <typename... Args>
        framed_widget(i_widget& aParent, dimension aLineWidth, Args&&... aArgs) :
            base_type{ aParent, std::forward<Args>(aArgs)... }, iStyle{ frame_style::SolidFrame }, iLineWidth{ aLineWidth }
        {
            base_type::set_background_opacity(1.0);
        }
        template <typename... Args>
        framed_widget(i_layout& aLayout, dimension aLineWidth, Args&&... aArgs) :
            base_type{ aLayout, std::forward<Args>(aArgs)... }, iStyle{ frame_style::SolidFrame }, iLineWidth{ aLineWidth }
        {
            base_type::set_background_opacity(1.0);
        }
        template <typename... Args>
        framed_widget(frame_style aStyle, Args&&... aArgs) :
            base_type{ std::forward<Args>(aArgs)... }, iStyle{ aStyle }, iLineWidth{ 1.0 }
        {
            base_type::set_background_opacity(1.0);
        }
        template <typename... Args>
        framed_widget(i_widget& aParent, frame_style aStyle, Args&&... aArgs) :
            base_type{ aParent, std::forward<Args>(aArgs)... }, iStyle{ aStyle }, iLineWidth{ 1.0 }
        {
            base_type::set_background_opacity(1.0);
        }
        template <typename... Args>
        framed_widget(i_layout& aLayout, frame_style aStyle, Args&&... aArgs) :
            base_type{ aLayout, std::forward<Args>(aArgs)... }, iStyle{ aStyle }, iLineWidth{ 1.0 }
        {
            base_type::set_background_opacity(1.0);
        }
        template <typename... Args>
        framed_widget(frame_style aStyle, dimension aLineWidth, Args&&... aArgs) :
            base_type{ std::forward<Args>(aArgs)... }, iStyle{ aStyle }, iLineWidth{ aLineWidth }
        {
            base_type::set_background_opacity(1.0);
        }
        template <typename... Args>
        framed_widget(i_widget& aParent, frame_style aStyle, dimension aLineWidth, Args&&... aArgs) :
            base_type{ aParent, std::forward<Args>(aArgs)... }, iStyle{ aStyle }, iLineWidth{ aLineWidth }
        {
            base_type::set_background_opacity(1.0);
        }
        template <typename... Args>
        framed_widget(i_layout& aLayout, frame_style aStyle, dimension aLineWidth, Args&&... aArgs) :
            base_type{ aLayout, std::forward<Args>(aArgs)... }, iStyle{ aStyle }, iLineWidth{ aLineWidth }
        {
            base_type::set_background_opacity(1.0);
        }
    public:
        using base_type::as_widget;
    public:
        neogfx::border border() const override;
    public:
        void paint_non_client(i_graphics_context& aGc) const override;
    public:
        void set_frame_style(frame_style aStyle);
    public:
        virtual bool has_frame_color() const;
        virtual color frame_color() const;
        virtual void set_frame_color(const optional_color& aFrameColor = optional_color{});
        virtual color inner_frame_color() const;
        virtual bool has_frame_radius() const;
        virtual vec4 frame_radius() const;
        virtual void set_frame_radius(const optional_vec4& aFrameRadius = optional_vec4{});
    public:
        dimension line_width() const;
        dimension effective_frame_width() const;
    private:
        frame_style iStyle;
        dimension iLineWidth;
        optional_color iFrameColor;
        optional_vec4 iFrameRadius;
    };
}