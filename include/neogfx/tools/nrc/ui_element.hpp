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
#include <neolib/vector.hpp>
#include <neogfx/tools/nrc/i_ui_element.hpp>

namespace neogfx::nrc
{
    template <typename Base = i_ui_element>
    class ui_element : public neolib::reference_counted<Base>
    {
    public:
        using i_ui_element::no_parent;
        using i_ui_element::wrong_type;
        using i_ui_element::ui_element_not_found;
    public:
        typedef neolib::vector<neolib::ref_ptr<i_ui_element>> children_t;
        using i_ui_element::data_t;
        using i_ui_element::array_data_t;
    public:
        ui_element(const i_ui_element_parser& aParser, const neolib::i_string& aId, ui_element_type aType) :
            iParser{ aParser }, iParent{ nullptr }, iId{ aId }, iType{ aType }
        {
        }
        ui_element(i_ui_element& aParent, const neolib::i_string& aId, ui_element_type aType) :
            iParser{ aParent.parser() }, iParent{ &aParent }, iId{ aId }, iType{ aType }
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
        const neolib::i_string& id() const override
        {
            return iId;
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
    private:
        const i_ui_element_parser& iParser;
        i_ui_element* iParent;
        neolib::string iId;
        ui_element_type iType;
        children_t iChildren;
    };
}
