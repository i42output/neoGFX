// i_ui_element.hpp
/*
neoGFX Resource Compiler
Copyright(C) 2019 Leigh Johnston

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

#include <neolib/neolib.hpp>
#include <neolib/i_reference_counted.hpp>
#include <neolib/i_string.hpp>

#include <neogfx/app/i_app.hpp>
#include <neogfx/gui/widget/i_widget.hpp>
#include <neogfx/gui/layout/i_layout.hpp>

#include <neogfx/tools/nrc/i_ui_element_parser.hpp>

namespace neogfx::nrc
{
    enum class ui_element_type
    {
        App,
        Widget,
        Layout
    };

    class i_ui_element : public neolib::i_reference_counted
    {
    public:
        struct no_parent : std::logic_error { no_parent() : std::logic_error{ "neogfx::nrc::i_ui_element::no_parent" } {} };
        struct wrong_type : std::logic_error { wrong_type() : std::logic_error{ "neogfx::nrc::i_ui_element::wrong_type" } {} };
        struct ui_element_not_found : std::runtime_error { ui_element_not_found() : std::runtime_error{ "neogfx::nrc::i_ui_element::ui_element_not_found" } {} };
    public:
        typedef i_ui_element_parser::data_type data_type;
        typedef i_ui_element_parser::array_data_type array_data_type;
    public:
        virtual ~i_ui_element() {}
    public:
        virtual const neolib::i_string& name() const = 0;
        virtual ui_element_type type() const = 0;
    public:
        virtual bool has_parent() const = 0;
        virtual const i_ui_element& parent() const = 0;
        virtual i_ui_element& parent() = 0;
    public:
        virtual i_ui_element& parse(const neolib::i_string& aName) = 0;
        virtual void parse(const neolib::i_string& aName, const data_type& aData) = 0;
        virtual void parse(const neolib::i_string& aName, const array_data_type& aData) = 0;
    public:
        virtual void instantiate(i_app& aApp) = 0;
        virtual void instantiate(i_widget& aWidget) = 0;
        virtual void instantiate(i_layout& aLayout) = 0;
    public:
        const i_ui_element& find(const neolib::i_string& aName) const
        {
            if (name() == aName)
                return *this;
            if (has_parent())
                return parent().find(aName);
            throw ui_element_not_found();
        }
        i_ui_element& find(const neolib::i_string& aName)
        {
            return const_cast<i_ui_element&>(to_const(*this).find(aName));
        }
    }
}
