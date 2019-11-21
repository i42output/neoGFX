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
        Invalid             = 0x0000000000000000,

        MASK_RESERVED       = 0x0000FFFF0000FFFF,
        MASK_USER           = 0x0FFFFFFFFFFF0000,

        MASK_TYPE           = 0x00000000FFFFFFFF,
        MASK_SUBCATEGORY    = 0x0000000F00000000,
        MASK_CATEGORY       = 0x0FFFFFF000000000,
        MASK_CONTEXT        = 0xF000000000000000,

        HasGeometry         = 0x0000000100000000,
        LayoutItem          = 0x0000000200000000,

        App                 = 0x0000001000000000,
        Action              = 0x0000002000000000,
        Widget              = 0x0000004000000000 | HasGeometry | LayoutItem,
        Layout              = 0x0000008000000000 | HasGeometry | LayoutItem,
        Menu                = 0x0000010000000000,

        Window              = 0x0000000000000001 | Widget,
        MenuBar             = 0x0000000000000002 | Widget | Menu,
        TabPageContainer    = 0x0000000000000003 | Widget,
        VerticalLayout      = 0x0000000000000004 | Layout,
        HorizontalLayout    = 0x0000000000000005 | Layout,
        GridLayout          = 0x0000000000000006 | Layout,
        FlowLayout          = 0x0000000000000007 | Layout,
        StackLayout         = 0x0000000000000008 | Layout,
        BorderLayout        = 0x0000000000000009 | Layout,

        Reference           = 0x1000000000000000,
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

    class i_ui_element_library;

    class i_ui_element : public neolib::i_reference_counted
    {
    public:
        struct no_parent : std::logic_error { no_parent() : std::logic_error{ "neogfx::nrc::i_ui_element::no_parent" } {} };
        struct wrong_type : std::logic_error { wrong_type() : std::logic_error{ "neogfx::nrc::i_ui_element::wrong_type" } {} };
        struct element_not_found : std::runtime_error { element_not_found() : std::runtime_error{ "neogfx::nrc::i_ui_element::element_not_found" } {} };
        struct element_ill_formed : std::runtime_error { element_ill_formed() : std::runtime_error{ "neogfx::nrc::i_ui_element::element_ill_formed" } {} };
    public:
        typedef neolib::i_vector<neolib::i_ref_ptr<i_ui_element>> children_t;
        typedef i_ui_element_parser::data_t data_t;
        typedef i_ui_element_parser::array_data_t array_data_t;
    public:
        virtual ~i_ui_element() {}
    public:
        virtual const i_ui_element_library& library() const = 0;
        virtual const i_ui_element_parser& parser() const = 0;
    public:
        virtual const neolib::i_string& header() const = 0;
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
            throw element_not_found();
        }
        i_ui_element& find(const neolib::i_string& aId)
        {
            return const_cast<i_ui_element&>(to_const(*this).find(aId));
        }
    public:
        length get_length(const std::string& aKey) const
        {
            return get_length(parser().get_data(aKey));
        }
        std::vector<length> get_lengths(const std::string& aKey) const
        {
            return get_lengths(parser().get_array_data(aKey));
        }
        static length get_length(const data_t& aVariant)
        {
            length result;
            std::visit([&result](auto&& v)
            {
                typedef std::remove_const_t<std::remove_reference_t<decltype(v)>> vt;
                if constexpr (std::is_same_v<vt, double>)
                    result = length{ v };
                else if constexpr (std::is_same_v<vt, int64_t>)
                    result = length{ static_cast<double>(v) };
                else if constexpr (std::is_same_v<vt, neolib::i_string>)
                    result = length::from_string(v.to_std_string());
                else
                    throw i_ui_element::wrong_type();
            }, aVariant);
            return result;
        }
        static std::vector<length> get_lengths(const array_data_t& aVariantArray)
        {
            std::vector<length> result;
            for (auto const& e : aVariantArray)
                result.push_back(i_ui_element::get_length(e));
            return result;
        }
    };
}
