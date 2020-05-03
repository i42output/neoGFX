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
#include <neogfx/core/object_type.hpp>
#include <neogfx/core/units.hpp>
#include <neogfx/app/i_app.hpp>
#include <neogfx/gui/layout/i_geometry.hpp>
#include <neogfx/gui/widget/i_widget.hpp>
#include <neogfx/gui/layout/i_layout.hpp>
#include <neogfx/tools/nrc/i_ui_element_parser.hpp>

namespace neogfx::nrc
{
    typedef object_type ui_element_type;

    struct member_element_t {};
    const member_element_t member_element;

    inline constexpr bool is_widget_or_layout(ui_element_type aType)
    {
        return ((category(aType) & (ui_element_type::Widget | ui_element_type::Layout)) != ui_element_type::Invalid);
    }

    struct element_not_found : std::runtime_error { element_not_found(const std::string& aElement) : std::runtime_error{ "Element '" + aElement + "' not found." } {} };
    struct element_ill_formed : std::runtime_error { element_ill_formed(const std::string& aElement) : std::runtime_error{ "Element '" + aElement + "' ill-formed." } {} };
    struct unsupported_member_element : std::runtime_error { unsupported_member_element() : std::runtime_error{ "Unsupported member element." } {} };

    class i_ui_element : public neolib::i_reference_counted
    {
    public:
        typedef i_ui_element abstract_type;
        typedef neolib::i_vector<neolib::i_ref_ptr<i_ui_element>> children_t;
        typedef i_ui_element_parser::data_t data_t;
        typedef i_ui_element_parser::array_data_t array_data_t;
    public:
        struct no_parent : std::logic_error { no_parent() : std::logic_error{ "neogfx::nrc::i_ui_element::no_parent" } {} };
        struct wrong_type : std::logic_error { wrong_type() : std::logic_error{ "neogfx::nrc::i_ui_element::wrong_type" } {} };
    public:
        virtual ~i_ui_element() = default;
    public:
        virtual const i_ui_element_parser& parser() const = 0;
    public:
        virtual const neolib::i_string& header() const = 0;
    public:
        virtual bool is_member_element() const = 0;
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
            throw element_not_found(aId.to_std_string());
        }
        i_ui_element& find(const neolib::i_string& aId)
        {
            return const_cast<i_ui_element&>(to_const(*this).find(aId));
        }
    public:
        template <typename T>
        T get_enum(const data_t& aData) const
        {
            return neolib::string_to_enum<T>(aData.get<neolib::i_string>());
        }
        template <typename T>
        T get_enum(const array_data_t& aArrayData) const
        {
            bool ignore;
            return get_enum<T>(aArrayData, ignore);
        }
        template <typename T>
        T get_enum(const array_data_t& aArrayData, bool& aUseDefault, const std::optional<std::string>& aDefault = {}) const
        {
            T result = {};
            aUseDefault = false;
            bool first = true;
            for (auto const& a : aArrayData)
            {
                auto const& e = a.get<neolib::i_string>();
                if (aDefault && e == aDefault)
                {
                    aUseDefault = true;
                    continue;
                }
                if (first)
                    result = neolib::string_to_enum<T>(e);
                else
                    result = static_cast<T>(static_cast<std::underlying_type_t<T>>(result) | static_cast<std::underlying_type_t<T>>(neolib::string_to_enum<alignment>(e)));
                first = false;
            }
            return result;
        }
        template <typename T>
        T get_scalar(const data_t& aData) const
        {
            T result;
            std::visit([&result](auto&& v)
            {
// For some reason VS2019 is not suppressing warning C4244 for the conversion marked "Flibble cross" below even though an explicit static_cast is used.
#ifdef _MSC_VER
#pragma warning (push)
#pragma warning (disable: 4244 ) // warning C4244: 'argument': conversion from 'const T' to 'double', possible loss of data
#endif
// At the time of writing I am unsure if the line marked "Flibble very cross" below is legal or not: g++ and clang++ disagree.
                typedef std::decay_t<decltype(v)> vt;
                if constexpr (std::is_same_v<vt, double>)
                    result = static_cast<T>(v);
                else if constexpr (std::is_same_v<vt, int64_t>) 
                    result = static_cast<T>(v); // Flibble cross
                else if constexpr (std::is_same_v<vt, neolib::i_string> && std::is_class_v<T>)
                    result = T::from_string(v.to_std_string()); // Flibble very cross
                else
                    throw wrong_type();
            }, aData);
#ifdef _MSC_VER
#pragma warning (pop)
#endif
            return result;
        }
        template <typename T>
        std::vector<T> get_scalars(const array_data_t& aArrayData) const
        {
            std::vector<T> result;
            for (auto const& e : aArrayData)
                result.push_back(get_scalar<T>(e));
            return result;
        }
        template <typename T>
        T get_scalar(const std::string& aKey) const
        {
            return get_scalar<T>(parser().get_data(aKey));
        }
        template <typename T>
        std::vector<T> get_scalars(const std::string& aKey) const
        {
            return get_scalars<T>(parser().get_array_data(aKey));
        }
        template <typename T, typename Target>
        void emplace_2(const neolib::string& aKey, Target& aTarget) const
        {
            std::vector<T> scalars;
            if (parser().data_exists(aKey))
                scalars.push_back(get_scalar<T>(parser().get_data(aKey)));
            else if (parser().array_data_exists(aKey))
                scalars = get_scalars<T>(parser().get_array_data(aKey));
            else
                return;
            switch (scalars.size())
            {
            case 1:
                aTarget.emplace(scalars[0]);
                break;
            case 2:
                aTarget.emplace(scalars[0], scalars[1]);
                break;
            default:
                throw element_ill_formed(id().to_std_string());
            }
        }
        template <typename T, typename Target>
        void emplace_4(const neolib::string& aKey, Target& aTarget) const
        {
            std::vector<T> scalars;
            if (parser().data_exists(aKey))
                scalars.push_back(get_scalar<T>(parser().get_data(aKey)));
            else if (parser().array_data_exists(aKey))
                scalars = get_scalars<T>(parser().get_array_data(aKey));
            else
                return;
            switch (scalars.size())
            {
            case 1:
                aTarget.emplace(scalars[0]);
                break;
            case 2:
                aTarget.emplace(scalars[0], scalars[1]);
                break;
            case 4:
                aTarget.emplace(scalars[0], scalars[1], scalars[2], scalars[3]);
                break;
            default:
                throw element_ill_formed(id().to_std_string());
            }
        }
        color get_color(const data_t& aData) const
        {
            color result;
            std::visit([&result](auto&& v)
            {
                typedef std::decay_t<decltype(v)> vt;
                if constexpr (std::is_same_v<vt, int64_t>)
                    result = color{ static_cast<uint32_t>(v) };
                else if constexpr (std::is_same_v<vt, neolib::i_string>)
                    result = v.to_std_string();
                else
                    throw wrong_type();
            }, aData);
            return result;
        }
        color get_color(const array_data_t& aArrayData) const
        {
            if (aArrayData.size() == 3)
                return color{ neolib::get_as<uint8_t>(aArrayData[0]), neolib::get_as<uint8_t>(aArrayData[1]), neolib::get_as<uint8_t>(aArrayData[2]) };
            else if (aArrayData.size() == 4)
                return color{ neolib::get_as<uint8_t>(aArrayData[0]), neolib::get_as<uint8_t>(aArrayData[1]), neolib::get_as<uint8_t>(aArrayData[2]), neolib::get_as<uint8_t>(aArrayData[3]) };
            else
                throw element_ill_formed(id().to_std_string());
        }
        color_or_gradient get_color_or_gradient(const array_data_t& aArrayData) const
        {
            if (aArrayData[0].which() == neolib::simple_variant_type::String)
            {
                auto gradientDirection = neolib::try_string_to_enum<gradient_direction>(aArrayData[0].get<neolib::i_string>().to_std_string());
                if (gradientDirection != std::nullopt)
                {
                    gradient::color_stop_list stops;
                    // todo: full gradient specification support
                    auto interval = 1.0 / (aArrayData.size() - 2);
                    for (std::size_t i = 1; i < aArrayData.size(); ++i)
                        stops.push_back(gradient::color_stop{ (i - 1) * interval, get_color(aArrayData[i]) });
                    return gradient{ stops, *gradientDirection };
                }
                else
                    return get_color(aArrayData);
            }
            else
                return get_color(aArrayData);
        }
    protected:
        template <typename Enum>
        static std::string enum_to_string(const std::string& aEnumName, Enum aEnumValue) 
        {
            auto es = neolib::enum_to_string(aEnumValue);
            if (es[0] != '0')
                return aEnumName + "::" + es;
            else
                return "static_cast<" + aEnumName + ">(" + es + ")";
        }
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
        static std::string convert_emit_argument(const color& aArgument)
        {
            std::ostringstream result;
            result << "0x" << std::uppercase << std::hex << std::setfill('0') << std::setw(8) << aArgument.value() << "u";
            return result.str();
        }
        static std::string convert_emit_argument(const gradient& aArgument)
        {
            std::ostringstream result;
            // todo: full gradient specification support
            result << "gradient::color_stop_list{ ";
            for (auto s = aArgument.color_begin(); s != aArgument.color_end(); ++s)
            {
                if (s != aArgument.color_begin())
                    result << ", ";
                result << "gradient::color_stop{ " << s->first << ", color{ " << convert_emit_argument(s->second) << " } }";
            }
            result << " }, " << enum_to_string<gradient_direction>("gradient_direction", aArgument.direction());
            return result.str();
        }
    };
}
