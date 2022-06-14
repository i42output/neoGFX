// i_status_bar.hpp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2020 Leigh Johnston.  All Rights Reserved.
  
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
#include <neogfx/gui/widget/i_widget.hpp>

namespace neogfx
{
    class i_status_bar : public i_widget
    {
    public:
        typedef i_status_bar abstract_type;
        typedef uint32_t widget_index;
    public:
        virtual ~i_status_bar() = default;
    public:
        virtual const i_widget& as_widget() const = 0;
        virtual i_widget& as_widget() = 0;
    public:
        virtual const i_widget& size_grip() const = 0;
    public:
        virtual bool have_message() const = 0;
        virtual i_string const& message() const = 0;
        virtual void set_message(i_string const& aMessage) = 0;
        virtual void clear_message() = 0;
        virtual void add_normal_widget(i_widget& aWidget) = 0;
        virtual void add_normal_widget_at(widget_index aPosition, i_widget& aWidget) = 0;
        virtual void add_normal_widget(i_ref_ptr<i_widget> const& aWidget) = 0;
        virtual void add_normal_widget_at(widget_index aPosition, i_ref_ptr<i_widget> const& aWidget) = 0;
        virtual void add_permanent_widget(i_widget& aWidget) = 0;
        virtual void add_permanent_widget_at(widget_index aPosition, i_widget& aWidget) = 0;
        virtual void add_permanent_widget(i_ref_ptr<i_widget> const& aWidget) = 0;
        virtual void add_permanent_widget_at(widget_index aPosition, i_ref_ptr<i_widget> const& aWidget) = 0;
    public:
        virtual i_layout& normal_layout() = 0;
        virtual i_layout& permanent_layout() = 0;
    public:
        virtual i_widget& message_widget() = 0;
        virtual i_widget& idle_widget() = 0;
    public:
        template<typename WidgetType>
        WidgetType& add_normal_widget()
        {
            auto w = make_ref<WidgetType>();
            add_normal_widget(w);
            return *w;
        }
        template<typename WidgetType>
        WidgetType& add_normal_widget_at(widget_index aPosition)
        {
            auto w = make_ref<WidgetType>();
            add_normal_widget_at(aPosition, w);
            return *w;
        }
        template<typename WidgetType>
        WidgetType& add_permanent_widget()
        {
            auto w = make_ref<WidgetType>();
            add_permanent_widget(w);
            return *w;
        }
        template<typename WidgetType>
        WidgetType& add_permanent_widget_at(widget_index aPosition)
        {
            auto w = make_ref<WidgetType>();
            add_permanent_widget_at(aPosition, w);
            return *w;
        }
    };
}