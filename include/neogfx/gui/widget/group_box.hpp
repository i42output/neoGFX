// group_box.hpp
/*
  neogfx C++ GUI Library
  Copyright (c) 2015 Leigh Johnston.  All Rights Reserved.
  
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
#include <neogfx/gui/layout/vertical_layout.hpp>
#include <neogfx/gui/widget/label.hpp>
#include <neogfx/gui/widget/check_box.hpp>
#include "widget.hpp"

namespace neogfx
{
    class group_box : public widget
    {
    public:
        typedef group_box abstract_type; // todo: create i_group_box
    private:
        typedef abstract_type property_context_type;
    private:
        typedef std::unique_ptr<neogfx::label> label_ptr;
        typedef std::unique_ptr<neogfx::check_box> check_box_ptr;
    public:
        struct not_checkable : std::logic_error { not_checkable() : std::logic_error("neogfx::group_box::not_checkable") {} };
    public:
        group_box(const std::string& aText = std::string());
        group_box(i_widget& aParent, const std::string& aText = std::string());
        group_box(i_layout& aLayout, const std::string& aText = std::string());
    public:
        const std::string& text() const;
        void set_text(const std::string& aText);
        bool is_checkable() const;
        void set_checkable(bool aCheckable);
        const neogfx::label& label() const;
        neogfx::label& label();
        bool has_check_box() const;
        const neogfx::check_box& check_box() const;
        neogfx::check_box& check_box();
        void set_item_layout(i_layout& aItemLayout);
        void set_item_layout(std::shared_ptr<i_layout> aItemLayout);
        const i_layout& item_layout() const;
        i_layout& item_layout();
        template <typename LayoutT, typename... Args>
        LayoutT& with_item_layout(Args&&... args)
        {
            set_item_layout(std::make_shared<LayoutT>(std::forward<Args>(args)...));
            return static_cast<LayoutT&>(item_layout());
        }
    public:
        neogfx::size_policy size_policy() const override;
    public:
        void paint(i_graphics_context& aGraphicsContext) const override;
    public:
        color background_color() const override;
    public:
        virtual bool has_border_color() const;
        virtual color border_color() const;
        virtual void set_border_color(const optional_color& aBorderColor);
        virtual bool has_fill_color() const;
        virtual color fill_color() const;
        virtual void set_fill_color(const optional_color& aFillColor);
        virtual double fill_opacity() const;
        virtual void set_fill_opacity(double aFillOpacity);
    private:
        void init();
    private:
        vertical_layout iLayout;
        neolib::variant<label_ptr, check_box_ptr> iTitle;
        std::shared_ptr<i_layout> iItemLayout;
        define_property(property_category::color, optional_color, BorderColor, border_color)
        define_property(property_category::color, optional_color, FillColor, fill_color)
        define_property(property_category::other_appearance, double, FillOpacity, fill_opacity, 1.0)
        sink iSink;
    };
}