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
#include <algorithm>
#include <boost/format.hpp>
#include <neolib/reference_counted.hpp>
#include <neolib/optional.hpp>
#include <neolib/vector.hpp>
#include <neolib/string.hpp>
#include <neogfx/core/units.hpp>
#include <neogfx/gui/layout/i_geometry.hpp>
#include <neogfx/gui/widget/label.hpp>
#include <neogfx/tools/nrc/i_ui_element.hpp>

namespace neogfx::nrc
{
    template <typename Base = i_ui_element>
    class ui_element : public neolib::reference_counted<Base>
    {
        typedef ui_element<Base> self_type;
        typedef neolib::reference_counted<Base> base_type;
    public:
        using i_ui_element::no_parent;
        using i_ui_element::wrong_type;
        using i_ui_element::element_not_found;
        using i_ui_element::element_ill_formed;
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
        ui_element(const i_ui_element_parser& aParser, i_ui_element& aParent, const neolib::optional<neolib::string>& aId, ui_element_type aType) :
            iParser{ aParser }, iParent{ &aParent }, iAnonymousIdCounter{ 0u }, iId{ aId }, iType{ aType }
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
        bool consume_element(const neolib::i_string& aElementType) override
        {
            if (aElementType == "label" && (type() & ui_element_type::HasLabel) == ui_element_type::HasLabel)
            {
                if (parser().data_exists("size"))
                    emplace_lengths_2(iLabelFixedSize, get_lengths("size"));
                if (parser().data_exists("minimum_size"))
                    emplace_lengths_2(iLabelMinimumSize, get_lengths("minimum_size"));
                if (parser().data_exists("maximum_size"))
                    emplace_lengths_2(iLabelMaximumSize, get_lengths("maximum_size"));
                iLabelPlacement = parser().get_optional_enum<label_placement>("placement");
                return true;
            }
            else if (aElementType == "image" && (type() & ui_element_type::HasImage) == ui_element_type::HasImage)
            {
                if (parser().data_exists("size"))
                    emplace_lengths_2(iImageFixedSize, get_lengths("size"));
                if (parser().data_exists("minimum_size"))
                    emplace_lengths_2(iImageMinimumSize, get_lengths("minimum_size"));
                if (parser().data_exists("maximum_size"))
                    emplace_lengths_2(iImageMaximumSize, get_lengths("maximum_size"));
                iImageUri = parser().get_optional<neolib::string>("uri");
                return true;
            }
            return false;
        }
        void parse(const neolib::i_string& aName, const data_t& aData) override
        {
            bool handled = false;
            if ((type() & ui_element_type::HasGeometry) == ui_element_type::HasGeometry)
            {
                if (aName == "size_policy")
                    handled = true, iSizePolicy = neolib::string_to_enum<size_constraint>(aData.get<neolib::i_string>());
                if (aName == "margin")
                    handled = true, iMargin.emplace(get_length(aData));
            }
            if ((type() & ui_element_type::HasText) == ui_element_type::HasText)
                if (aName == "text")
                    handled = true, iText = aData.get<neolib::i_string>();
            if ((type() & ui_element_type::HasImage) == ui_element_type::HasImage)
                if (aName == "image")
                    handled = true, iImageUri = aData.get<neolib::i_string>();
            if ((type() & ui_element_type::HasColour) == ui_element_type::HasColour)
            {
                if (aName == "foreground_colour")
                    handled = true, iForegroundColour = get_colour(aData);
                else if (aName == "background_colour")
                    handled = true, iBackgroundColour = get_colour(aData);
                else if (aName == "opacity")
                    handled = true, iOpacity = aData.get<double>();
                else if (aName == "transparency")
                    handled = true, iOpacity = 1.0 - aData.get<double>();
            }
            if (!handled)
                std::cerr << "warning: nrc: Unknown element key '" << aName << "' in element '" << id() << "'." << std::endl;
        }
        void parse(const neolib::i_string& aName, const array_data_t& aData) override
        {
            bool handled = false;
            if ((type() & ui_element_type::HasGeometry) == ui_element_type::HasGeometry)
            {
                if (aName == "size_policy" && !aData.empty())
                    handled = true, iSizePolicy = size_policy::from_string(
                        aData[0u].get<neolib::i_string>().to_std_string(),
                        aData[std::min<std::size_t>(1u, aData.size() - 1u)].get<neolib::i_string>().to_std_string());
                else if (aName == "margin")
                    handled = true, emplace_lengths_4(iMargin, get_lengths(aData));
                else if (aName == "size")
                    handled = true, emplace_lengths_2(iFixedSize, get_lengths(aData));
                else if (aName == "minimum_size")
                    handled = true, emplace_lengths_2(iMinimumSize, get_lengths(aData));
                else if (aName == "maximum_size")
                    handled = true, emplace_lengths_2(iMaximumSize, get_lengths(aData));
            }
            if ((type() & ui_element_type::HasColour) == ui_element_type::HasColour)
            {
                if (aName == "foreground_colour")
                    handled = true, iForegroundColour = get_colour(aData);
                else if (aName == "background_colour")
                    handled = true, iBackgroundColour = get_colour(aData);
            }
            if (!handled)
                std::cerr << "warning: nrc: Unknown element key '" << aName << "' in element '" << id() << "'." << std::endl;
        }
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
            if (iSizePolicy)
                emit("   %1%.set_size_policy(%2%);\n", id(), *iSizePolicy);
            if (iFixedSize)
                emit("   %1%.set_fixed_size(size{ %2%, %3 });\n", id(), iFixedSize->cx, iFixedSize->cy);
            if (iMinimumSize)
                emit("   %1%.set_minimum_size(size{ %2%, %3 });\n", id(), iMinimumSize->cx, iMinimumSize->cy);
            if (iMaximumSize)
                emit("   %1%.set_maximum_size(size{ %2%, %3 });\n", id(), iMaximumSize->cx, iMaximumSize->cy);
            if (iMargin)
            {
                auto const& margin = *iMargin;
                if (margin.left == margin.right && margin.top == margin.bottom)
                {
                    if (margin.left == margin.top)
                        emit("   %1%.set_margins(margins{ %2% });\n", id(), margin.left);
                    else
                        emit("   %1%.set_margins(margins{ %2%, %3% });\n", id(), margin.left, margin.top);
                }
                else 
                    emit("   %1%.set_margins(margins{ %2%, %3%, %4%, %5% });\n", id(), margin.left, margin.top, margin.right, margin.bottom);
            }
            if (iLabelFixedSize)
                emit("   %1%.label().set_fixed_size(size{ %2%, %3% });\n", id(), iLabelFixedSize->cx, iLabelFixedSize->cy);
            if (iLabelMinimumSize)
                emit("   %1%.label().set_minimum_size(size{ %2%, %3% });\n", id(), iLabelMinimumSize->cx, iLabelMinimumSize->cy);
            if (iLabelMaximumSize)
                emit("   %1%.label().set_maximum_size(size{ %2%, %3% });\n", id(), iLabelMaximumSize->cx, iLabelMaximumSize->cy);
            if (iLabelPlacement)
                emit("   %1%.label().set_placement(label_placement::%2%);\n", id(), neolib::enum_to_string(*iLabelPlacement));
            if (iImageFixedSize)
                emit("   %1%.image().set_fixed_size(size{ %2%, %3% });\n", id(), iImageFixedSize->cx, iImageFixedSize->cy);
            if (iImageMinimumSize)
                emit("   %1%.image().set_minimum_size(size{ %2%, %3% });\n", id(), iImageMinimumSize->cx, iImageMinimumSize->cy);
            if (iImageMaximumSize)
                emit("   %1%.image().set_maximum_size(size{ %2%, %3% });\n", id(), iImageMaximumSize->cx, iImageMaximumSize->cy);
            if (iImageUri)
                emit("   %1%.image().set_image(image{ \"%2%\" });\n", id(), *iImageUri);
            if (iText)
                emit("   %1%.text().set_text(\"%2%\"_t);\n", id(), *iText);
            if (iOpacity)
                emit("   %1%.set_opacity(%2%);\n", id(), *iOpacity);
            if (iForegroundColour)
                emit("   %1%.set_foreground_colour(colour{ %2% });\n", id(), *iForegroundColour);
            if (iForegroundColour)
                emit("   %1%.set_background_colour(colour{ %2% });\n", id(), *iBackgroundColour);
            for (auto const& child : children())
                child->emit_body();
        }
    protected:
        using base_type::get_length;
        using base_type::get_lengths;
        using base_type::get_colour;
    protected:
        void emit(const std::string& aArgument) const
        {
            parser().emit(aArgument);
        }
        template <typename... Args>
        void emit(const std::string& aFormat, const Args&... aArguments) const
        {
            parser().emit(aFormat, base_type::convert_emit_argument(aArguments)...);
        }
    private:
        template <typename Target>
        void emplace_lengths_2(Target& aTarget, const std::vector<length>& aLengths)
        {
            switch (aLengths.size())
            {
            case 1:
                aTarget.emplace(aLengths[0]);
                break;
            case 2:
                aTarget.emplace(aLengths[0], aLengths[1]);
                break;
            default:
                throw element_ill_formed();
            }
        }
        template <typename Target>
        void emplace_lengths_4(Target& aTarget, const std::vector<length>& aLengths)
        {
            switch (aLengths.size())
            {
            case 1:
                aTarget.emplace(aLengths[0]);
                break;
            case 2:
                aTarget.emplace(aLengths[0], aLengths[1]);
                break;
            case 4:
                aTarget.emplace(aLengths[0], aLengths[1], aLengths[2], aLengths[3]);
                break;
            default:
                throw element_ill_formed();
            }
        }
    private:
        const i_ui_element_parser& iParser;
        i_ui_element* iParent;
        neolib::optional<neolib::string> iId;
        mutable neolib::optional<neolib::string> iAnonymousId;
        mutable uint32_t iAnonymousIdCounter;
        ui_element_type iType;
        children_t iChildren;
        std::optional<size_policy> iSizePolicy;
        std::optional<basic_size<length>> iFixedSize;
        std::optional<basic_size<length>> iMinimumSize;
        std::optional<basic_size<length>> iMaximumSize;
        std::optional<basic_margins<length>> iMargin;
        std::optional<basic_size<length>> iLabelFixedSize;
        std::optional<basic_size<length>> iLabelMinimumSize;
        std::optional<basic_size<length>> iLabelMaximumSize;
        std::optional<label_placement> iLabelPlacement;
        std::optional<string> iText;
        std::optional<string> iImageUri;
        std::optional<basic_size<length>> iImageFixedSize;
        std::optional<basic_size<length>> iImageMinimumSize;
        std::optional<basic_size<length>> iImageMaximumSize;
        std::optional<double> iOpacity;
        std::optional<colour> iForegroundColour;
        std::optional<colour> iBackgroundColour;
    };
}
