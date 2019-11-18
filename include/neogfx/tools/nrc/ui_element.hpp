// ui_element.hpp
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
#include <boost/format.hpp>
#include <neolib/reference_counted.hpp>
#include <neolib/optional.hpp>
#include <neolib/vector.hpp>
#include <neogfx/core/units.hpp>
#include <neogfx/tools/nrc/i_ui_element.hpp>

namespace neogfx::nrc
{
    template <typename Base = i_ui_element>
    class ui_element : public neolib::reference_counted<Base>
    {
        typedef ui_element<Base> self_type;
        typedef neolib::reference_counted<Base> base_type;
    public:
        using i_ui_element::element_is_anonymous;
        using i_ui_element::no_parent;
        using i_ui_element::wrong_type;
        using i_ui_element::ui_element_not_found;
    public:
        typedef neolib::ref_ptr<i_ui_element> element_ptr_t;
        typedef neolib::vector<element_ptr_t> children_t;
        using i_ui_element::data_t;
        using i_ui_element::array_data_t;
    public:
        ui_element(const i_ui_element_parser& aParser, const neolib::optional<neolib::string>& aId, ui_element_type aType) :
            iParser{ aParser }, iParent{ nullptr }, iId{ aId }, iAnonymousIdCounter{ 0u }, iType{ aType }
        {
        }
        ui_element(i_ui_element& aParent, const neolib::optional<neolib::string>& aId, ui_element_type aType) :
            iParser{ aParent.parser() }, iParent{ &aParent }, iAnonymousIdCounter{ 0u }, iId{ aId }, iType{ aType }
        {
            parent().children().push_back(neolib::ref_ptr<i_ui_element>{ this });
        }
        ~ui_element()
        {
        }
    public:
        const i_ui_element_parser& parser() const override
        {
            return iParser;
        }
    public:
        bool anonymous() const override
        {
            return !iId;
        }
        const neolib::i_string& id() const override
        {
            if (!anonymous())
                return *iId;
            return anonymous_id();
        }
        const neolib::i_string& anonymous_id() const override
        {
            if (!iAnonymousId)
            {
                if (has_parent())
                    iAnonymousId = parent().generate_anonymous_id();
                else
                    iAnonymousId = parser().generate_anonymous_id();
            }
            return *iAnonymousId;
        }
        using base_type::generate_anonymous_id;
        void generate_anonymous_id(neolib::i_string& aNewAnonymousId) const override
        {
            aNewAnonymousId = neolib::string{ id() + "_" + boost::lexical_cast<std::string>(++iAnonymousIdCounter) };
        }
        ui_element_type type() const override
        {
            return iType;
        }
    public:
        bool has_parent() const override
        {
            return iParent != nullptr;
        }
        const i_ui_element& parent() const override
        {
            if (has_parent())
                return *iParent;
            throw no_parent();
        }
        i_ui_element& parent() override
        {
            return const_cast<i_ui_element&>(to_const(*this).parent());
        }
        const children_t& children() const override
        {
            return iChildren;
        }
        children_t& children() override
        {
            return iChildren;
        }
    public:
        void instantiate(i_app& aApp) override
        {
        }
        void instantiate(i_widget& aWidget) override
        {
        }
        void instantiate(i_layout& aLayout) override
        {
        }
    protected:
        void emit_preamble() const override
        {
            for (auto const& child : children())
                child->emit_preamble();
        }
        void emit_ctor() const override
        {
            for (auto const& child : children())
                child->emit_ctor();
        }
        void emit_body() const override
        {
            for (auto const& child : children())
                child->emit_body();
        }
    protected:
        void emit(const std::string& aArgument) const
        {
            parser().emit(neolib::string{ aArgument });
        }
        template <typename T>
        void emit(const std::string& aFormat, const T& aArgument) const
        {
            parser().emit(neolib::string{ (boost::format(aFormat) % convert_emit_argument(aArgument)).str() });
        }
        template <typename T1, typename T2>
        void emit(const std::string& aFormat, const T1& aArgument1, const T2& aArgument2) const
        {
            parser().emit(neolib::string{ (boost::format(aFormat) % convert_emit_argument(aArgument1) % convert_emit_argument(aArgument2)).str() });
        }
        template <typename T1, typename T2, typename T3>
        void emit(const std::string& aFormat, const T1& aArgument1, const T2& aArgument2, const T3& aArgument3) const
        {
            parser().emit(neolib::string{ (boost::format(aFormat) % convert_emit_argument(aArgument1) % convert_emit_argument(aArgument2) % convert_emit_argument(aArgument3)).str() });
        }
        template <typename T1, typename T2, typename T3, typename T4>
        void emit(const std::string& aFormat, const T1& aArgument1, const T2& aArgument2, const T3& aArgument3, const T4& aArgument4) const
        {
            parser().emit(neolib::string{ (boost::format(aFormat) % convert_emit_argument(aArgument1) % convert_emit_argument(aArgument2) % convert_emit_argument(aArgument3) % convert_emit_argument(aArgument4)).str() });
        }
    private:
        template <typename T>
        static const T& convert_emit_argument(const T& aArgument)
        {
            return aArgument;
        }
        static std::string convert_emit_argument(const bool& aArgument)
        {
            return aArgument ? "true" : "false";
        }
        static std::string convert_emit_argument(const length& aArgument)
        {
            return aArgument.to_string();
        }
    private:
        const i_ui_element_parser& iParser;
        i_ui_element* iParent;
        neolib::optional<neolib::string> iId;
        mutable neolib::optional<neolib::string> iAnonymousId;
        mutable uint32_t iAnonymousIdCounter;
        ui_element_type iType;
        children_t iChildren;
    };
}
