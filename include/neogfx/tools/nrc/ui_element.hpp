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
#include <map>
#include <boost/lexical_cast.hpp>

#include <neolib/core/reference_counted.hpp>
#include <neolib/core/optional.hpp>
#include <neolib/core/vector.hpp>
#include <neolib/core/string.hpp>

#include <neogfx/core/units.hpp>
#include <neogfx/gui/layout/i_geometry.hpp>
#include <neogfx/gui/widget/i_widget.hpp>
#include <neogfx/gui/widget/label_bits.hpp>
#include <neogfx/gui/widget/text_field_bits.hpp>
#include <neogfx/tools/nrc/i_ui_element.hpp>

namespace neogfx::nrc
{
    template <typename Base = i_ui_element>
    class ui_element : public neolib::reference_counted<Base>
    {
        typedef neolib::reference_counted<Base> base_type;
    public:
        using i_ui_element::no_parent;
        using i_ui_element::wrong_type;
    public:
        typedef neolib::ref_ptr<i_ui_element> element_ptr_t;
        typedef neolib::vector<element_ptr_t> children_t;
        using i_ui_element::data_t;
        using i_ui_element::array_data_t;
        typedef neolib::vector<neolib::simple_variant> concrete_array_data_t;
    protected:
        typedef std::set<neolib::string> data_names_t;
    public:
        ui_element(const i_ui_element_parser& aParser, ui_element_type aType) :
            iParser{ aParser }, iParent{ nullptr }, iName{ aParser.current_element() }, iFragmentName{ aParser.current_fragment() }, iTypeName{ aParser.current_element() }, iMemberElement{ false }, iId{ aParser.get_optional<neolib::string>("id") }, iAnonymousIdCounter{ 0u }, iType{ aType }
        {
            init();
        }
        ui_element(const i_ui_element_parser& aParser, ui_element_type aType, const neolib::optional<neolib::string>& aId) :
            iParser{ aParser }, iParent{ nullptr }, iName{ aParser.current_element() }, iFragmentName{ aParser.current_fragment() }, iTypeName{ aParser.current_element() }, iMemberElement{ false }, iId{ aId }, iAnonymousIdCounter{ 0u }, iType{ aType }
        {
            init();
        }
        ui_element(const i_ui_element_parser& aParser, i_ui_element& aParent, ui_element_type aType) :
            iParser{ aParser }, iParent{ &aParent }, iName{ aParser.current_element() }, iFragmentName{ aParser.current_fragment() }, iTypeName{ aParser.current_element() }, iMemberElement{ false }, iAnonymousIdCounter{ 0u }, iId{ aParser.get_optional<neolib::string>("id") }, iType{ aType }
        {
            init();
            parent().children().push_back(neolib::ref_ptr<i_ui_element>{ this });
        }
        ui_element(const i_ui_element_parser& aParser, i_ui_element& aParent, ui_element_type aType, const neolib::optional<neolib::string>& aId) :
            iParser{ aParser }, iParent{ &aParent }, iName{ aParser.current_element() }, iFragmentName{ aParser.current_fragment() }, iTypeName{ aParser.current_element() }, iMemberElement{ false }, iAnonymousIdCounter{ 0u }, iId{ aId }, iType{ aType }
        {
            init();
            parent().children().push_back(neolib::ref_ptr<i_ui_element>{ this });
        }
        ui_element(const i_ui_element_parser& aParser, i_ui_element& aParent, member_element_t, ui_element_type aType) :
            iParser{ aParser }, iParent{ &aParent }, iName{ aParser.current_element() }, iFragmentName{ aParser.current_fragment() }, iTypeName{ aParser.current_element() }, iMemberElement{ true }, iAnonymousIdCounter{ 0u }, iId{}, iType{ aType }
        {
            init();
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
        const neolib::i_string& name() const override
        {
            return iName;
        }
        const neolib::i_string& fragment_name() const override
        {
            return iFragmentName;
        }
        const neolib::i_string& type_name() const override
        {
            if (iDragDrop != std::nullopt && *iDragDrop)
            {
                thread_local neolib::string result;
                result = "drag_drop_target<"_s + iTypeName + ">"_s;
                return result;
            }
            return iTypeName;
        }
        void set_type_name(const neolib::i_string& aTypeName) override
        {
            iTypeName = aTypeName;
        }
        const neolib::i_vector<neolib::i_string>& headers() const override
        {
            return iHeaders;
        }
        void add_header(std::string const& aHeader)
        {
            iHeaders.push_back(neolib::string{ aHeader });
        }
    public:
        bool is_member_element() const override
        {
            return iMemberElement;
        }
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
                if (!is_member_element())
                {
                    if (has_parent())
                        iAnonymousId = parent().generate_anonymous_id();
                    else
                        iAnonymousId = parser().generate_anonymous_id();
                }
                else
                {
                    switch (type() & ui_element_type::MASK_RESERVED_SPECIFIC)
                    {
                    case ui_element_type::Label:
                        iAnonymousId = parent().id() + ".label()";
                        break;
                    case ui_element_type::LineEdit:
                        iAnonymousId = parent().id() + ".input_box()";
                        break;
                    case ui_element_type::TextWidget:
                        iAnonymousId = parent().id() + ".text_widget()";
                        break;
                    case ui_element_type::ImageWidget:
                        iAnonymousId = parent().id() + ".image_widget()";
                        break;
                    default:
                        throw unsupported_member_element();
                    }
                }
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
        using base_type::check_element_ref;
    public:
        const i_ui_element& fragment() const override
        {
            auto e = static_cast<const i_ui_element*>(this);
            while (e->has_parent())
                e = &e->parent();
            return *e;
        }
        i_ui_element& fragment() override
        {
            return const_cast<i_ui_element&>(to_const(*this).fragment());
        }
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
        using i_ui_element::enum_to_string;
        using i_ui_element::get_enum;
        using i_ui_element::get_scalar;
        using i_ui_element::get_scalars;
        using i_ui_element::emplace_2;
        using i_ui_element::emplace_4;
        using i_ui_element::get_color;
    protected:
        const data_names_t& data_names() const
        {
            return iDataNames;
        }
        void add_data_names(data_names_t aNames)
        {
            iDataNames.merge(aNames);
        }
        void parse(const neolib::i_string& aName, const data_t& aData) override
        {
            if (data_names().find(aName) == data_names().end())
            {
                std::cerr << parser().source_location() << ": warning: nrc: Unknown element key '" << aName << "' in element '" << id() << "'." << std::endl;
                return;
            }
            if (aName == "drag_drop")
            {
                iDragDrop = aData.get<bool>();
                if (*iDragDrop)
                    add_header("neogfx/app/drag_drop.hpp");
            }
            if (aName == "type")
                iWidgetType = get_enum<widget_type>(aData);
            // (attributes that are object properties have the property's name; a composite property's component is "Property.Component", 
            // e.g. "MaximumSize.Width" or, in the .nrc, "MaximumSize: { Width: 100 }")
            if (aName == "Weight")
                iWeight.emplace(get_scalar<double>(aData));
            else if (aName == "SizePolicy")
                iSizePolicy = get_enum<size_constraint>(aData);
            else if (aName == "Alignment")
                iAlignment = get_enum<alignment>(aData);
            else if (aName == "FixedSize")
                iFixedSize.emplace(get_scalar<length>(aData));
            else if (aName == "MinimumSize")
                iMinimumSize.emplace(get_scalar<length>(aData));
            else if (aName == "MinimumSize.Width")
                iMinimumWidth.emplace(get_scalar<length>(aData));
            else if (aName == "MinimumSize.Height")
                iMinimumHeight.emplace(get_scalar<length>(aData));
            else if (aName == "MaximumSize")
                iMaximumSize.emplace(get_scalar<length>(aData));
            else if (aName == "MaximumSize.Width")
                iMaximumWidth.emplace(get_scalar<length>(aData));
            else if (aName == "MaximumSize.Height")
                iMaximumHeight.emplace(get_scalar<length>(aData));
            else if (aName == "FixedSize.Width")
                iFixedWidth.emplace(get_scalar<length>(aData));
            else if (aName == "FixedSize.Height")
                iFixedHeight.emplace(get_scalar<length>(aData));
            else if (aName == "Weight.Width")
                iWeightWidth.emplace(get_scalar<double>(aData));
            else if (aName == "Weight.Height")
                iWeightHeight.emplace(get_scalar<double>(aData));
            else if (aName == "Padding.Left")
                iPaddingLeft.emplace(get_scalar<length>(aData));
            else if (aName == "Padding.Top")
                iPaddingTop.emplace(get_scalar<length>(aData));
            else if (aName == "Padding.Right")
                iPaddingRight.emplace(get_scalar<length>(aData));
            else if (aName == "Padding.Bottom")
                iPaddingBottom.emplace(get_scalar<length>(aData));
            else if (aName == "SizePolicy.Horizontal")
                iHorizontalSizePolicy = get_enum<size_constraint>(aData);
            else if (aName == "SizePolicy.Vertical")
                iVerticalSizePolicy = get_enum<size_constraint>(aData);
            else if (aName == "Padding")
                iPadding.emplace(get_scalar<length>(aData));
            else if (aName == "Enabled")
                iEnabled = aData.get<bool>();
            else if (aName == "FocusPolicy")
                iFocusPolicy.first = get_enum<focus_policy>(aData);
            else if (aName == "text")
                iText = aData.get<neolib::i_string>();
            else if (aName == "label")
                iLabelText = aData.get<neolib::i_string>();
            else if (aName == "image" || (aName == "uri" && (type() & ui_element_type::MASK_RESERVED_SPECIFIC) == ui_element_type::ImageWidget))
                iImage = aData.get<neolib::i_string>();
            else if (aName == "AspectRatio")
                iAspectRatio = get_enum<aspect_ratio>(aData);
            else if (aName == "Placement" || aName == "placement") // (a label's or image's Placement property; a text field's placement)
            {
                if ((type() & ui_element_type::MASK_RESERVED_SPECIFIC) != ui_element_type::TextField)
                {
                    if ((type() & ui_element_type::MASK_RESERVED_SPECIFIC) == ui_element_type::Label || (type() & ui_element_type::HasLabel) == ui_element_type::HasLabel)
                        iLabelPlacement = get_enum<label_placement>(aData);
                    else if ((type() & ui_element_type::MASK_RESERVED_SPECIFIC) == ui_element_type::ImageWidget || (type() & ui_element_type::HasImage) == ui_element_type::HasImage)
                        iImagePlacement = get_enum<cardinal>(aData);
                }
                else
                    iTextFieldPlacement = get_enum<text_field_placement>(aData);
            }
            else if (aName.to_std_string_view().starts_with("Palette."))
                iPaletteColors[aName.to_std_string().substr(8u)] = get_color(aData);
            else if (aName == "Opacity")
                iOpacity = aData.get<double>();
            else if (aName == "default_focus")
                iDefaultFocus = aData.get<neolib::i_string>();
        }
        void parse(const neolib::i_string& aName, const array_data_t& aArrayData) override
        {
            if (data_names().find(aName) == data_names().end())
            {
                std::cerr << parser().source_location() << ": warning: nrc: Unknown element key '" << aName << "' in element '" << id() << "'." << std::endl;
                return;
            }
            if (aName == "SizePolicy" && !aArrayData.empty())
                iSizePolicy = size_policy::from_string(
                    aArrayData[0u].get<neolib::i_string>().to_std_string(),
                    aArrayData[std::min<std::size_t>(1u, aArrayData.size() - 1u)].get<neolib::i_string>().to_std_string());
            else if (aName == "Alignment")
                iAlignment = get_enum<alignment>(aArrayData);
            else if (aName == "FixedSize")
                emplace_2<length>("FixedSize", iFixedSize);
            else if (aName == "MinimumSize")
                emplace_2<length>("MinimumSize", iMinimumSize);
            else if (aName == "MaximumSize")
                emplace_2<length>("MaximumSize", iMaximumSize);
            else if (aName == "Padding")
                emplace_4<length>("Padding", iPadding);
            else if (aName == "Weight")
                emplace_2<double>("Weight", iWeight);
            else if (aName == "FocusPolicy")
                iFocusPolicy.first = get_enum<focus_policy>(aArrayData, iFocusPolicy.second, "Default");
            else if (aName.to_std_string_view().starts_with("Palette."))
                iPaletteColors[aName.to_std_string().substr(8u)] = get_color(aArrayData);
        }
        void add_element_ref(const neolib::i_string& aRef) override
        {
            auto const& fullRef = aRef.to_std_string_view();
            auto part = fullRef.find_first_of('.');
            auto ref = fullRef.substr(0, part);
            auto resolved = parser().find(neolib::string{ ref });
            if (!resolved || (part == std::string::npos && resolved->fragment_name() != fragment_name()))
                throw element_not_found(std::string{ ref });
            if (part != std::string::npos)
                iRefs.insert(neolib::string{ ref });
        }
        const neolib::i_set<neolib::i_string>& element_refs() const override
        {
            return iRefs;
        }
        const neolib::i_string& generate_ctor_params(bool aParamsAfter = false) const override
        {
            std::string temp;
            if (iWidgetType && *iWidgetType == widget_type::Child)
                temp += "i_widget& aParent";
            for (auto const& ref : iRefs)
            {
                if (!temp.empty())
                    temp += ", ";
                auto const& e = parser().at(ref);
                temp += e.fragment_name().to_std_string() + "& " + e.id().to_std_string();
            }
            if (aParamsAfter && !temp.empty())
                temp += ", ";
            thread_local neolib::string result;
            result = temp;
            return result;
        }
        const neolib::i_string& generate_base_ctor_args(bool aArgsAfter = false) const override
        {
            std::string temp;
            if (iWidgetType && *iWidgetType == widget_type::Child)
                temp += "aParent";
            if (!temp.empty())
            {
                if (aArgsAfter)
                    temp += ", ";
                else
                    temp = " " + temp + " ";
            }
            thread_local neolib::string result;
            result = temp;
            return result;
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
            if (iAlignment)
                emit("   %1%.set_alignment(%2%);\n", id(), enum_to_string("alignment", *iAlignment));
            if (iFixedSize)
                emit("   %1%.set_fixed_size(size{ %2%, %3% });\n", id(), iFixedSize->cx, iFixedSize->cy);
            if (iMinimumSize)
                emit("   %1%.set_minimum_size(size{ %2%, %3% });\n", id(), iMinimumSize->cx, iMinimumSize->cy);
            if (iMinimumWidth)
                emit("   %1%.set_minimum_width(%2%);\n", id(), *iMinimumWidth);
            if (iMinimumHeight)
                emit("   %1%.set_minimum_height(%2%);\n", id(), *iMinimumHeight);
            if (iMaximumSize)
                emit("   %1%.set_maximum_size(size{ %2%, %3% });\n", id(), iMaximumSize->cx, iMaximumSize->cy);
            if (iMaximumWidth)
                emit("   %1%.set_maximum_width(%2%);\n", id(), *iMaximumWidth);
            if (iMaximumHeight)
                emit("   %1%.set_maximum_height(%2%);\n", id(), *iMaximumHeight);
            if (iWeight)
                emit("   %1%.set_weight(size{ %2%, %3% });\n", id(), iWeight->cx, iWeight->cy);
            // (a composite property's components given on their own (e.g. "FixedSize: { Width: 24dip }"): the others are as they are)
            if (iHorizontalSizePolicy || iVerticalSizePolicy)
            {
                emit("   { auto sizePolicy = %1%.size_policy();", id());
                if (iHorizontalSizePolicy)
                    emit(" sizePolicy.set_horizontal_constraint(%1%);", enum_to_string("size_constraint", *iHorizontalSizePolicy));
                if (iVerticalSizePolicy)
                    emit(" sizePolicy.set_vertical_constraint(%1%);", enum_to_string("size_constraint", *iVerticalSizePolicy));
                emit(" %1%.set_size_policy(sizePolicy); }\n", id());
            }
            if (iFixedWidth || iFixedHeight)
            {
                emit("   { auto fixedSize = %1%.fixed_size();", id());
                if (iFixedWidth)
                    emit(" fixedSize.cx = %1%;", *iFixedWidth);
                if (iFixedHeight)
                    emit(" fixedSize.cy = %1%;", *iFixedHeight);
                emit(" %1%.set_fixed_size(fixedSize); }\n", id());
            }
            if (iWeightWidth || iWeightHeight)
            {
                emit("   { auto weight = %1%.weight();", id());
                if (iWeightWidth)
                    emit(" weight.cx = %1%;", *iWeightWidth);
                if (iWeightHeight)
                    emit(" weight.cy = %1%;", *iWeightHeight);
                emit(" %1%.set_weight(weight); }\n", id());
            }
            if (iPaddingLeft || iPaddingTop || iPaddingRight || iPaddingBottom)
            {
                emit("   { auto padding = %1%.padding();", id());
                if (iPaddingLeft)
                    emit(" padding.left = %1%;", *iPaddingLeft);
                if (iPaddingTop)
                    emit(" padding.top = %1%;", *iPaddingTop);
                if (iPaddingRight)
                    emit(" padding.right = %1%;", *iPaddingRight);
                if (iPaddingBottom)
                    emit(" padding.bottom = %1%;", *iPaddingBottom);
                emit(" %1%.set_padding(padding); }\n", id());
            }
            if (iPadding)
            {
                auto const& padding = *iPadding;
                if (padding.left == padding.right && padding.top == padding.bottom)
                {
                    if (padding.left == padding.top)
                        emit("   %1%.set_padding(neogfx::padding{ %2% });\n", id(), padding.left);
                    else
                        emit("   %1%.set_padding(neogfx::padding{ %2%, %3% });\n", id(), padding.left, padding.top);
                }
                else 
                    emit("   %1%.set_padding(neogfx::padding{ %2%, %3%, %4%, %5% });\n", id(), padding.left, padding.top, padding.right, padding.bottom);
            }
            if (iEnabled)
                emit("   %1%.%2%();\n", id(), *iEnabled ? "enable" : "disable");
            if (iFocusPolicy.first)
            {
                if (!iFocusPolicy.second)
                    emit("   %1%.set_focus_policy(%2%);\n", id(), enum_to_string("focus_policy", *iFocusPolicy.first));
                else
                    emit("   %1%.set_focus_policy(%1%.focus_policy() | %2%);\n", id(), enum_to_string("focus_policy", *iFocusPolicy.first));
            }
            if (iLabelPlacement)
                emit("   %1%.set_placement(%2%);\n", id(), enum_to_string("label_placement", *iLabelPlacement));
            if (iTextFieldPlacement)
                emit("   %1%.set_placement(%2%);\n", id(), enum_to_string("text_field_placement", *iTextFieldPlacement));
            if (iImage)
                emit("   %1%.set_image(\"%2%\"_s);\n", id(), *iImage);
            if (iAspectRatio)
                emit("   %1%.set_aspect_ratio(%2%);\n", id(), enum_to_string("aspect_ratio", *iAspectRatio));
            if (iImagePlacement)
                emit("   %1%.set_placement(%2%);\n", id(), enum_to_string("cardinal", *iImagePlacement));
            if (iText)
                emit("   %1%.set_text(\"%2%\"_t);\n", id(), *iText);
            if (iLabelText)
                emit("   %1%.label().set_text(\"%2%\"_t);\n", id(), *iLabelText);
            if (iOpacity)
                emit("   %1%.set_opacity(%2%);\n", id(), *iOpacity);
            for (auto const& [role, paletteColor] : iPaletteColors)
                emit("   %1%.set_palette_color(color_role::%2%, %3%);\n", id(), role, std::string_view{ paletteColor });
            if (iDefaultFocus)
            {
                check_element_ref(*iDefaultFocus);
                emit("   %1%.set_focus();\n", *iDefaultFocus);
            }
            if ((type() & ui_element_type::Widget) == ui_element_type::Widget &&
                (type() & ui_element_type::Separator) != ui_element_type::Separator && has_parent() &&
                (parent().type() & ui_element_type::MASK_RESERVED_SPECIFIC) == ui_element_type::StatusBar)
            {
                if (!iPadding && !iPaddingLeft && !iPaddingTop && !iPaddingRight && !iPaddingBottom)
                    emit("   %1%.set_padding(neogfx::padding{});\n", id());
                emit("   %1%.set_font_role(font_role::StatusBar);\n", id());
            }
            for (auto const& child : children())
                child->emit_body();
        }
    protected:
        neolib::string layout() const
        {
            if ((parent().type() & ui_element_type::MASK_RESERVED_GENERIC) == ui_element_type::Window)
            {
                switch (type() & ui_element_type::MASK_RESERVED_SPECIFIC)
                {
                case ui_element_type::MenuBar:
                    return ".menu_layout()";
                case ui_element_type::Toolbar:
                    return ".toolbar_layout()";
                case ui_element_type::StatusBar:
                    return "";
                default:
                    return ".client_layout()";
                }
            }
            else if ((parent().type() & ui_element_type::MASK_RESERVED_SPECIFIC) == ui_element_type::GroupBox)
                return ".item_layout()";
            else if ((parent().type() & ui_element_type::MASK_RESERVED_SPECIFIC) == ui_element_type::StatusBar)
                return ".normal_layout()";
            else
                return "";
        }
        void emit_generic_ctor() const
        {
            if (is_member_element())
                return;
            emit(",\n"
                "   %1%{ %2%%3% }", id(), parent().id(), layout());
        }
        void emit_generic_ctor(const std::optional<neolib::string>& aText) const
        {
            if (is_member_element())
                return;
            emit(",\n"
                "   %1%{ %2%%3%, \"%4%\"_t }", id(), parent().id(), layout(), *aText);
        }
        template <typename Enum>
        std::enable_if_t<std::is_enum_v<Enum>, void> emit_generic_ctor(std::string const& aEnumName, Enum aEnum) const
        {
            if (is_member_element())
                return;
            emit(",\n"
                "   %1%{ %2%%3%, %4% }", id(), parent().id(), layout(), enum_to_string(aEnumName, aEnum));
        }
        template <typename T>
        void emit_generic_ctor(const T& aArgument) const
        {
            if (is_member_element())
                return;
            emit(",\n"
                "   %1%{ %2%%3%, %4% }", id(), parent().id(), layout(), aArgument);
        }
    protected:
        void emit(std::string const& aArgument) const
        {
            parser().emit(aArgument);
        }
        template <typename... Args>
        void emit(std::string const& aFormat, const Args&... aArguments) const
        {
            parser().emit(aFormat, base_type::convert_emit_argument(aArguments)...);
        }
    protected:
    private:
        void init()
        {
            if (!anonymous())
                parser().index(id(), *this);
            if (!has_parent() && (type() & ui_element_type::Widget) == ui_element_type::Widget)
                add_data_names({ "type", "default_focus" });
            if ((type() & ui_element_type::Widget) == ui_element_type::Widget)
                add_data_names({ "drag_drop", "Enabled", "FocusPolicy" });
            if ((type() & ui_element_type::HasGeometry) == ui_element_type::HasGeometry)
                add_data_names({ "SizePolicy", "Padding", "MinimumSize", "MaximumSize", "FixedSize", "MinimumSize.Width", "MinimumSize.Height", "MaximumSize.Width", "MaximumSize.Height", "Weight", 
                    "FixedSize.Width", "FixedSize.Height", "Weight.Width", "Weight.Height", "Padding.Left", "Padding.Top", "Padding.Right", "Padding.Bottom", 
                    "SizePolicy.Horizontal", "SizePolicy.Vertical" });
            if ((type() & ui_element_type::HasAlignment) == ui_element_type::HasAlignment)
                add_data_names({ "Alignment" });
            if ((type() & (ui_element_type::HasText | ui_element_type::HasLabel)) != ui_element_type::None)
                add_data_names({ "text" });
            if ((type() & (ui_element_type::HasImage | ui_element_type::HasLabel)) != ui_element_type::None)
                add_data_names({ "image", "AspectRatio", (type() & ui_element_type::MASK_RESERVED_SPECIFIC) == ui_element_type::TextField ? "placement" : "Placement" });
            if ((type() & ui_element_type::MASK_RESERVED_SPECIFIC) == ui_element_type::ImageWidget)
                add_data_names({ "uri" });
            if ((type() & ui_element_type::HasColor) == ui_element_type::HasColor)
                add_data_names({ "Palette.Theme", "Palette.Background", "Palette.Foreground", "Palette.Base", "Palette.AlternateBase", "Palette.Text", 
                    "Palette.Selection", "Palette.AlternateSelection", "Palette.SelectedText", "Palette.Focus", "Palette.Hover", "Palette.PrimaryAccent", 
                    "Palette.SecondaryAccent", "Palette.Void", "Opacity" });
        }
    private:
        const i_ui_element_parser& iParser;
        i_ui_element* iParent;
        neolib::string iName;
        neolib::string iFragmentName;
        neolib::string iTypeName;
        neolib::vector<neolib::string> iHeaders;
        bool iMemberElement;
        data_names_t iDataNames;
        neolib::optional<neolib::string> iId;
        mutable neolib::optional<neolib::string> iAnonymousId;
        mutable std::uint32_t iAnonymousIdCounter;
        ui_element_type iType;
        children_t iChildren;
        neolib::set<neolib::string> iRefs;
        std::optional<bool> iDragDrop;
        std::optional<widget_type> iWidgetType;
        std::optional<size_policy> iSizePolicy;
        std::optional<alignment> iAlignment;
        std::optional<basic_size<length>> iFixedSize;
        std::optional<basic_size<length>> iMinimumSize;
        std::optional<basic_size<length>> iMaximumSize;
        std::optional<length> iMinimumWidth;
        std::optional<length> iMinimumHeight;
        std::optional<length> iMaximumWidth;
        std::optional<length> iMaximumHeight;
        std::optional<size> iWeight;
        std::optional<basic_padding<length>> iPadding;
        std::optional<length> iFixedWidth; // (a composite property's components given on their own, e.g. "FixedSize: { Width: 24dip }")
        std::optional<length> iFixedHeight;
        std::optional<double> iWeightWidth;
        std::optional<double> iWeightHeight;
        std::optional<length> iPaddingLeft;
        std::optional<length> iPaddingTop;
        std::optional<length> iPaddingRight;
        std::optional<length> iPaddingBottom;
        std::optional<size_constraint> iHorizontalSizePolicy;
        std::optional<size_constraint> iVerticalSizePolicy;
        std::optional<bool> iEnabled;
        std::pair<std::optional<focus_policy>, bool> iFocusPolicy;
        std::optional<label_placement> iLabelPlacement;
        std::optional<text_field_placement> iTextFieldPlacement;
        std::optional<string> iText;
        std::optional<string> iLabelText;
        std::optional<string> iImage;
        std::optional<aspect_ratio> iAspectRatio;
        std::optional<cardinal> iImagePlacement;
        std::optional<double> iOpacity;
        std::map<std::string, std::string> iPaletteColors; // (by color_role name, e.g. "Base"; the colour to emit, see get_color)
        std::optional<string> iDefaultFocus;
    };
}
