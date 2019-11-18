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

#include <neogfx/neogfx.hpp>
#include <neolib/neolib.hpp>
#include <neolib/i_reference_counted.hpp>
#include <neolib/i_optional.hpp>
#include <neolib/i_string.hpp>
#include <neolib/i_vector.hpp>
#include <neogfx/app/i_app.hpp>
#include <neogfx/gui/widget/i_widget.hpp>
#include <neogfx/gui/layout/i_layout.hpp>
#include <neogfx/tools/nrc/i_ui_element_parser.hpp>

namespace neogfx::nrc
{
    enum class ui_element_type : uint64_t
    {
        Invalid         = 0x0000000000000000,

        App             = 0x0000000000000001,
        Action          = 0x0000000000000002,
        Widget          = 0x0000000000000004,
        Layout          = 0x0000000000000008,
        Menu            = 0x0000000000000010,
        Custom          = 0x0000000080000000,

        Window          = 0x0000000100000000 | Widget,
        MenuBar         = 0x0000000200000000 | Widget | Menu,

        Reference       = 0x1000000000000000,

        MASK_CATEGORY   = 0x00000000FFFFFFFF,
        MASK_TYPE       = 0x0FFFFFFF00000000,
        MASK_CONTEXT    = 0xF000000000000000
    };

    inline constexpr ui_element_type operator|(ui_element_type aLhs, ui_element_type aRhs)
    {
        return static_cast<ui_element_type>(static_cast<uint64_t>(aLhs) | static_cast<uint64_t>(aRhs));
    }

    inline constexpr ui_element_type operator&(ui_element_type aLhs, ui_element_type aRhs)
    {
        return static_cast<ui_element_type>(static_cast<uint64_t>(aLhs) & static_cast<uint64_t>(aRhs));
    }

    inline constexpr ui_element_type category(ui_element_type aType)
    {
        return aType & ui_element_type::MASK_CATEGORY;
    }

    inline constexpr bool is_widget_or_layout(ui_element_type aType)
    {
        return ((category(aType) & (ui_element_type::Widget | ui_element_type::Layout)) != ui_element_type::Invalid);
    }

    class i_ui_element : public neolib::i_reference_counted
    {
    public:
        struct element_is_anonymous : std::logic_error { element_is_anonymous() : std::logic_error{ "neogfx::nrc::i_ui_element::element_is_anonymous" } {} };
        struct no_parent : std::logic_error { no_parent() : std::logic_error{ "neogfx::nrc::i_ui_element::no_parent" } {} };
        struct wrong_type : std::logic_error { wrong_type() : std::logic_error{ "neogfx::nrc::i_ui_element::wrong_type" } {} };
        struct ui_element_not_found : std::runtime_error { ui_element_not_found() : std::runtime_error{ "neogfx::nrc::i_ui_element::ui_element_not_found" } {} };
    public:
        typedef neolib::i_vector<neolib::i_ref_ptr<i_ui_element>> children_t;
        typedef i_ui_element_parser::data_t data_t;
        typedef i_ui_element_parser::array_data_t array_data_t;
    public:
        virtual ~i_ui_element() {}
    public:
        virtual const i_ui_element_parser& parser() const = 0;
    public:
        virtual bool anonymous() const = 0;
        virtual const neolib::i_string& id() const = 0;
        virtual const neolib::i_string& anonymous_id() const = 0;
        virtual void generate_anonymous_id(neolib::i_string& aNewAnonymousId) const = 0;
        virtual ui_element_type type() const = 0;
    public:
        virtual bool has_parent() const = 0;
        virtual const i_ui_element& parent() const = 0;
        virtual i_ui_element& parent() = 0;
        virtual const children_t& children() const = 0;
        virtual children_t& children() = 0;
    public:
        virtual void parse(const neolib::i_string& aType, const data_t& aData) = 0;
        virtual void parse(const neolib::i_string& aType, const array_data_t& aData) = 0;
        virtual void emit() const = 0;
        virtual void emit_preamble() const = 0;
        virtual void emit_ctor() const = 0;
        virtual void emit_body() const = 0;
    public:
        virtual void instantiate(i_app& aApp) = 0;
        virtual void instantiate(i_widget& aWidget) = 0;
        virtual void instantiate(i_layout& aLayout) = 0;
    public:
        neolib::string generate_anonymous_id() const
        {
            neolib::string newAnonymousId;
            generate_anonymous_id(newAnonymousId);
            return newAnonymousId;
        }
        const i_ui_element& find(const neolib::i_string& aId) const
        {
            if (!anonymous() && id() == aId)
                return *this;
            if (has_parent())
                return parent().find(aId);
            throw ui_element_not_found();
        }
        i_ui_element& find(const neolib::i_string& aId)
        {
            return const_cast<i_ui_element&>(to_const(*this).find(aId));
        }
    };
}
