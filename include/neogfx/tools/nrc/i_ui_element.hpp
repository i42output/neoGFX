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
#include <neogfx/core/i_object.hpp>
#include <neogfx/core/units.hpp>
#include <neogfx/app/i_app.hpp>
#include <neogfx/gui/layout/i_geometry.hpp>
#include <neogfx/gui/widget/i_widget.hpp>
#include <neogfx/gui/layout/i_layout.hpp>
#include <neogfx/tools/nrc/i_ui_element_parser.hpp>

namespace neogfx::nrc
{
    typedef object_type ui_element_type;

    inline constexpr bool is_widget_or_layout(ui_element_type aType)
    {
        return ((category(aType) & (ui_element_type::Widget | ui_element_type::Layout)) != ui_element_type::Invalid);
    }

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
        virtual bool consume_element(const neolib::i_string& aElementType) = 0;
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
                    throw wrong_type();
            }, aVariant);
            return result;
        }
        static std::vector<length> get_lengths(const array_data_t& aVariantArray)
        {
            std::vector<length> result;
            for (auto const& e : aVariantArray)
                result.push_back(get_length(e));
            return result;
        }
        static colour get_colour(const data_t& aVariant)
        {
            colour result;
            std::visit([&result](auto&& v)
            {
                typedef std::remove_const_t<std::remove_reference_t<decltype(v)>> vt;
                if constexpr (std::is_same_v<vt, int64_t>)
                    result = colour{ static_cast<uint32_t>(v) };
                else if constexpr (std::is_same_v<vt, neolib::i_string>)
                    result = v.to_std_string();
                else
                    throw wrong_type();
            }, aVariant);
            return result;
        }
        static colour get_colour(const array_data_t& aVariantArray)
        {
            if (aVariantArray.size() == 3)
                return colour{ neolib::get_as<uint8_t>(aVariantArray[0]), neolib::get_as<uint8_t>(aVariantArray[1]), neolib::get_as<uint8_t>(aVariantArray[2]) };
            else if (aVariantArray.size() == 4)
                return colour{ neolib::get_as<uint8_t>(aVariantArray[0]), neolib::get_as<uint8_t>(aVariantArray[1]), neolib::get_as<uint8_t>(aVariantArray[2]), neolib::get_as<uint8_t>(aVariantArray[3]) };
            else
                throw element_ill_formed();
        }
    protected:
        template <typename T>
        static const T& convert_emit_argument(const T& aArgument)
        {
            return aArgument;
        }
        static std::string convert_emit_argument(const bool& aArgument)
        {
            return aArgument ? "true" : "false";
        }
        static std::string convert_emit_argument(const neolib::i_string& aArgument)
        {
            std::string result;
            for (auto const ch : aArgument)
            {
                switch (ch)
                {
                case '\b':
                    result += "\\b";
                    continue;
                case '\f':
                    result += "\\f";
                    continue;
                case '\n':
                    result += "\\n";
                    continue;
                case '\r':
                    result += "\\r";
                    continue;
                case '\t':
                    result += "\\t";
                    continue;
                case '\\':
                case '\"':
                case '\'':
                    result += '\\';
                    break;
                }
                result += ch;
            }
            return result;
        }
        static std::string convert_emit_argument(const neolib::string& aArgument)
        {
            return convert_emit_argument(static_cast<const neolib::i_string&>(aArgument));
        }
        static std::string convert_emit_argument(const std::string& aArgument)
        {
            return convert_emit_argument(neolib::string{ aArgument });
        }
        static std::string convert_emit_argument(const length& aArgument)
        {
            return aArgument.to_string(true);
        }
        static std::string convert_emit_argument(const size_policy& aArgument)
        {
            std::string hp, vp;
            aArgument.to_string(hp, vp);
            if (hp == vp)
                return "size_constraint::" + hp;
            else
                return "size_constraint::" + hp + ", size_constraint::" + vp;
        }
        static std::string convert_emit_argument(const colour& aArgument)
        {
            std::ostringstream result;
            result << "0x" << std::uppercase << std::hex << std::setfill('0') << std::setw(8) << aArgument.value() << "u";
            return result.str();
        }
    };
}
