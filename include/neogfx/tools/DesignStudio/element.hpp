// element.hpp
/*
neoGFX Design Studio
Copyright(C) 2020 Leigh Johnston

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
#include <concepts>

#include <neolib/core/reference_counted.hpp>
#include <neolib/core/optional.hpp>
#include <neolib/core/vector.hpp>
#include <neolib/core/pair.hpp>
#include <neolib/core/string.hpp>
#include <neolib/task/event.hpp>

#include <neogfx/core/units.hpp>
#include <neogfx/app/app.hpp>
#include <neogfx/app/action.hpp>
#include <neogfx/hid/i_surface_manager.hpp>
#include <neogfx/gui/widget/i_widget.hpp>
#include <neogfx/gui/widget/i_menu.hpp>
#include <neogfx/gui/layout/i_layout.hpp>
#include <neogfx/gui/widget/progress_bar.hpp>
#include <neogfx/gui/widget/group_box.hpp>
#include <neogfx/gui/widget/tab_page.hpp>
#include <neogfx/gui/widget/tab_button.hpp>
#include <neogfx/gui/widget/i_tab_page_container.hpp>
#include <neogfx/gui/layout/vertical_layout.hpp>
#include <neogfx/gui/layout/spacer.hpp>
#include <neogfx/tools/DesignStudio/symbol.hpp>
#include <neogfx/tools/DesignStudio/i_project.hpp>
#include <neogfx/tools/DesignStudio/i_element.hpp>

namespace neogfx::DesignStudio
{
    struct user_interface {};

    template <typename Type>
    inline element_group default_element_group()
    {
        if constexpr (std::is_base_of_v<app, Type>)
            return element_group::App;
        else if constexpr (std::is_base_of_v<action, Type>)
            return element_group::Action;
        else if constexpr (std::is_base_of_v<i_menu, Type>)
            return element_group::Menu;
        else if constexpr (std::is_base_of_v<i_widget, Type>)
            return element_group::Widget;
        else if constexpr (std::is_base_of_v<i_layout, Type>)
            return element_group::Layout;
        else if constexpr (std::is_base_of_v<i_project, Type>)
            return element_group::Project;
        else if constexpr (std::is_base_of_v<user_interface, Type>)
            return element_group::UserInterface;
        else
            return element_group::Unknown;
    }

    // tab pages are created by (and owned by) their tab page container rather than wrapped in a caddy
    template <>
    struct element_traits<tab_page>
    {
        typedef i_element base;
        static constexpr bool needsCaddy = false;
    };

    template <typename BaseType>
    class element_variant
    {
    public:
        typedef BaseType base_type;
    };

    template <typename Type, typename Base = typename element_traits<Type>::base>
    class element : public neolib::reference_counted<Base>
    {
        typedef neolib::reference_counted<Base> base_type;
    public:
        define_declared_event(ModeChanged, mode_changed)
        define_declared_event(SelectionChanged, selection_changed)
        define_declared_event(ContextMenu, context_menu, i_menu&)
    public:
        using typename i_element::no_parent;
        using typename i_element::no_layout_item;
        using typename i_element::no_caddy;
        using typename i_element::no_child_layout;
    public:
        typedef maybe_abstract_t<base_type> abstract_type;
        typedef neolib::vector<ref_ptr<i_element>> children_t;
        typedef neolib::vector<neolib::pair<string, string>> attributes_t;
    public:
        element(i_element_library const& aLibrary, i_project& aProject, i_string const& aType, element_group aGroup = default_element_group<Type>()) :
            iLibrary{ aLibrary }, 
            iProject{ aProject },
            iParent { nullptr }, 
            iGroup{ aGroup }, 
            iType{ aType }
        {
        }
        element(i_element_library const& aLibrary, i_project& aProject, i_string const& aType, i_string const& aId, element_group aGroup = default_element_group<Type>()) :
            iLibrary{ aLibrary }, 
            iProject{ aProject },
            iParent{ nullptr },
            iGroup{ aGroup }, 
            iType{ aType }, 
            iId{ aId }
        {
        }
        element(i_element_library const& aLibrary, i_element& aParent, i_string const& aType, element_group aGroup = default_element_group<Type>()) :
            iLibrary{ aLibrary }, 
            iProject{ aParent.project() },
            iParent{ &aParent },
            iGroup{ aGroup }, 
            iType{ aType }
        {
            parent().add_child(*this);
        }
        element(i_element_library const& aLibrary, i_element& aParent, i_string const& aType, i_string const& aId, element_group aGroup = default_element_group<Type>()) :
            iLibrary{ aLibrary }, 
            iProject{ aParent.project() },
            iParent{ &aParent },
            iGroup{ aGroup },
            iType{ aType }, 
            iId{ aId }
        {
            parent().add_child(*this);
        }
        ~element()
        {
        }
    public:
        i_element_library const& library() const override
        {
            return iLibrary;
        }
        i_project& project() const override
        {
            return iProject;
        }
        element_group group() const override
        {
            return iGroup;
        }
        neolib::i_string const& type() const override
        {
            return iType;
        }
        neolib::i_string const& id() const override
        {
            return iId;
        }
    public:
        i_element const& root() const override
        {
            i_element const* e = this;
            while (e->has_parent())
                e = &e->parent();
            return *e;
        }
        i_element& root() override
        {
            return const_cast<i_element&>(to_const(*this).root());
        }
        bool has_parent() const override
        {
            return iParent != nullptr;
        }
        i_element const& parent() const override
        {
            if (has_parent())
                return *iParent;
            throw no_parent();
        }
        i_element& parent() override
        {
            return const_cast<i_element&>(to_const(*this).parent());
        }
        void set_parent(i_element& aParent) override
        {
            iParent = &aParent;
        }
        children_t const& children() const override
        {
            return iChildren;
        }
        children_t& children() override
        {
            return iChildren;
        }
        void add_child(i_element& aChild) override
        {
            if (aChild.group() != element_group::App || group() != element_group::UserInterface)
                children().push_back(neolib::ref_ptr<i_element>{ &aChild });
            else
            {
                aChild.set_parent(parent());
                parent().children().insert(std::prev(parent().children().end()), neolib::ref_ptr<i_element>{ &aChild });
            }
        }
        void remove_child(i_element& aChild) override
        {
            auto existing = std::find_if(children().begin(), children().end(), [&](auto&& e) { return &*e == &aChild; });
            if (existing != children().end())
                children().erase(existing);
        }
    public:
        attributes_t const& attributes() const override
        {
            return iAttributes;
        }
        attributes_t& attributes() override
        {
            return iAttributes;
        }
    public:
        void create_default_children() override
        {
            DesignStudio::create_default_children<Type>(*this);
        }
    public:
        bool needs_caddy() const override
        {
            return element_traits<Type>::needsCaddy;
        }
        bool has_caddy() const override
        {
            return iCaddy != nullptr;
        }
        i_element_caddy& caddy() const override
        {
            if (has_caddy())
                return *iCaddy;
            throw no_caddy();
        }
        void set_caddy(i_element_caddy& aCaddy) override
        {
            iCaddy = aCaddy;
        }
        bool has_layout_item() const override
        {
            return iLayoutItem != nullptr;
        }
        void create_layout_item(i_widget& aParent) override
        {
            if (!iLayoutItem)
            {
                if constexpr (std::is_base_of_v<i_tab_page, Type>)
                {
                    if (has_parent() && parent().has_layout_item())
                        parent().create_child_layout_item(*this, iLayoutItem);
                    if (iLayoutItem)
                    {
                        // clicking the page's tab on the design surface makes the page the current element
                        iSink = static_cast<Type&>(iLayoutItem->as_widget()).selected([this]()
                        {
                            if (!sRevealing)
                                set_mode(element_mode::Edit);
                        });
                        // double clicking the page's tab edits its text in place (tab pages have no caddy of their own so the container's is used)
                        iSink += static_cast<tab_button&>(static_cast<Type&>(iLayoutItem->as_widget()).tab()).DoubleClicked([this]()
                        {
                            if (has_parent() && parent().has_caddy())
                                parent().caddy().begin_text_edit(*this);
                        });
                    }
                }
                else if constexpr (std::is_base_of_v<i_widget, Type>)
                {
                    if constexpr (std::is_constructible_v<Type, i_element&>)
                        iLayoutItem = make_ref<Type>(*this);
                    else if constexpr (std::is_constructible_v<Type, i_widget&, window_style>)
                        iLayoutItem = make_ref<Type>(aParent, (window_style::Default | window_style::Nested));
                    else if constexpr (std::is_constructible_v<Type, i_element&, i_widget&, window_style>)
                        iLayoutItem = make_ref<Type>(*this, aParent, (window_style::Default | window_style::Nested));
                    else if constexpr (std::is_constructible_v<Type, i_widget&, neolib::i_string const&, window_style>)
                        iLayoutItem = make_ref<Type>(aParent, iId.to_std_string(), (window_style::Default | window_style::Nested));
                    else if constexpr (std::is_constructible_v<Type, i_element&, i_widget&, neolib::i_string const&, window_style>)
                        iLayoutItem = make_ref<Type>(*this, aParent, iId.to_std_string(), (window_style::Default | window_style::Nested));
                    else if constexpr (std::is_constructible_v<Type, i_widget&, neolib::i_string const&>)
                        iLayoutItem = make_ref<Type>(aParent, iId.to_std_string());
                    else if constexpr (std::is_constructible_v<Type, i_element&, i_widget&, neolib::i_string const&>)
                        iLayoutItem = make_ref<Type>(*this, aParent, iId.to_std_string());
                    else if constexpr (std::is_constructible_v<Type, window_style>)
                        iLayoutItem = make_ref<Type>((window_style::Default | window_style::Nested));
                    else if constexpr (std::is_constructible_v<Type, i_element&, window_style>)
                        iLayoutItem = make_ref<Type>(*this, (window_style::Default | window_style::Nested));
                    else if constexpr (std::is_constructible_v<Type, neolib::i_string const&, window_style>)
                        iLayoutItem = make_ref<Type>(iId.to_std_string(), (window_style::Default | window_style::Nested));
                    else if constexpr (std::is_constructible_v<Type, i_widget&, neolib::i_string const&, window_style>)
                        iLayoutItem = make_ref<Type>(*this, iId.to_std_string(), (window_style::Default | window_style::Nested));
                    else if constexpr (std::is_constructible_v<Type, neolib::i_string const&>)
                        iLayoutItem = make_ref<Type>(iId.to_std_string());
                    else if constexpr (std::is_constructible_v<Type, i_element&, neolib::i_string const&>)
                        iLayoutItem = make_ref<Type>(*this, iId.to_std_string());
                    else if constexpr (std::is_default_constructible_v<Type>)
                        iLayoutItem = make_ref<Type>();
                    else
                    {
                        // todo: widget creation for the other widget types
                    }
                    if (!iLayoutItem)
                        return;
                    if constexpr (std::is_base_of_v<i_tab_page_container, Type>)
                    {
                        // the element's widget ignores mouse events (the caddy handles them) but the tabs must remain clickable
                        static_cast<Type&>(iLayoutItem->as_widget()).tab_bar().as_widget().set_consider_ancestors_for_mouse_events(false);
                    }
                    if (std::is_same_v<Type, progress_bar>)
                    {
                        auto& progressBar = static_cast<progress_bar&>(iLayoutItem->as_widget());
                        progressBar.set_value(0.5);
                        progressBar.set_minimum(0.0);
                        progressBar.set_maximum(1.0);
                    }
                    if (iLayoutItem->is_widget() && iLayoutItem->as_widget().is_root() && iLayoutItem->as_widget().root().is_nested())
                        service<i_surface_manager>().nest_for(aParent, nest_type::Caddy).add(iLayoutItem->as_widget().root().native_window());
                }
                else if constexpr (std::is_base_of_v<i_layout, Type>)
                {
                    if constexpr (std::is_constructible_v<Type, i_element&>)
                        iLayoutItem = make_ref<Type>(*this);
                    else
                        iLayoutItem = make_ref<Type>();
                }
                else if constexpr (std::is_same_v<Type, spacer>)
                    iLayoutItem = make_ref<spacer>(static_cast<expansion_policy>(
                        static_cast<std::uint32_t>(expansion_policy::ExpandHorizontally) | static_cast<std::uint32_t>(expansion_policy::ExpandVertically)));
                else if constexpr (std::is_base_of_v<i_spacer, Type> && std::is_default_constructible_v<Type>)
                    iLayoutItem = make_ref<Type>();
            }
        }
        bool has_text() const override
        {
            // has visible text that can be edited in place
            if constexpr (std::is_base_of_v<i_tab_page, Type>)
                return true; // its tab's text
            else if constexpr (std::is_base_of_v<i_widget, Type>)
                return requires(Type& aWidget, string const& aText) { aWidget.set_title_text(aText); } || 
                    requires(Type& aWidget, string const& aText) { aWidget.set_text(aText); };
            else
                return false;
        }
        i_string const& text_attribute() const override
        {
            static string const sTabText{ "tab_text" };
            static string const sTitle{ "title" };
            static string const sText{ "text" };
            if constexpr (std::is_base_of_v<i_tab_page, Type>)
                return sTabText;
            else if constexpr (requires(Type& aWidget, string const& aText) { aWidget.set_title_text(aText); })
                return sTitle;
            else
                return sText;
        }
        i_widget& text_area() const override
        {
            if constexpr (std::is_base_of_v<i_tab_page, Type>)
                return static_cast<tab_button&>(static_cast<Type&>(layout_item().as_widget()).tab()).label().text_widget(); // tab pages have tab_button tabs
            else if constexpr (std::is_base_of_v<i_widget, Type>)
            {
                auto& widget = static_cast<Type&>(layout_item().as_widget());
                if constexpr (requires(Type& aWidget) { aWidget.input_box(); })
                    return widget.input_box(); // text fields: their text is the input box's
                else if constexpr (requires(Type& aWidget) { aWidget.label().text_widget(); })
                    return widget.label().text_widget(); // e.g. buttons, group boxes
                else if constexpr (requires(Type& aWidget) { aWidget.text_widget(); })
                    return widget.text_widget(); // e.g. labels
                else if constexpr (requires(Type& aWidget) { aWidget.title_bar().title_widget(); })
                {
                    try
                    {
                        return widget.title_bar().title_widget(); // windows: their title
                    }
                    catch (std::logic_error const&)
                    {
                        return widget; // no title bar
                    }
                }
                else
                    return widget;
            }
            else
                return layout_item().as_widget();
        }
        neogfx::alignment text_alignment() const override
        {
            if constexpr (std::is_base_of_v<i_tab_page, Type>)
                return static_cast<tab_button&>(static_cast<Type&>(layout_item().as_widget()).tab()).label().text_widget().alignment();
            else if constexpr (std::is_base_of_v<i_widget, Type>)
            {
                auto& widget = static_cast<Type&>(layout_item().as_widget());
                if constexpr (requires(Type& aWidget) { aWidget.input_box(); })
                    return widget.input_box().alignment();
                else if constexpr (requires(Type& aWidget) { aWidget.label().text_widget(); })
                    return widget.label().text_widget().alignment();
                else if constexpr (requires(Type& aWidget) { aWidget.text_widget(); })
                    return widget.text_widget().alignment();
                else if constexpr (requires(Type& aWidget) { aWidget.title_bar().title_widget(); })
                {
                    try
                    {
                        return widget.title_bar().title_widget().alignment();
                    }
                    catch (std::logic_error const&)
                    {
                        return neogfx::alignment::Left | neogfx::alignment::VCenter;
                    }
                }
                else if constexpr (requires(Type& aWidget) { { aWidget.alignment() } -> std::convertible_to<neogfx::alignment>; })
                    return widget.alignment(); // e.g. text widgets
                else
                    return neogfx::alignment::Left | neogfx::alignment::VCenter;
            }
            else
                return neogfx::alignment::Left | neogfx::alignment::VCenter;
        }
        void apply_attributes(bool aShowIds) override
        {
            // show the element's text (as in a running application) or, if requested, its id
            if (!has_layout_item())
                return;
            if constexpr (std::is_base_of_v<i_widget, Type>)
            {
                auto& widget = static_cast<Type&>(layout_item().as_widget());
                string const text{ aShowIds ? iId.to_std_string() : design_text() };
                if constexpr (std::is_base_of_v<i_tab_page, Type>)
                    widget.tab().set_text(text);
                else if constexpr (requires(Type& aWidget, string const& aText) { aWidget.set_title_text(aText); })
                    widget.set_title_text(text);
                else if constexpr (requires(Type& aWidget, string const& aText) { aWidget.set_text(aText); })
                    widget.set_text(text);
            }
        }
        i_layout_item& layout_item() const override
        {
            if (iLayoutItem != nullptr)
                return *iLayoutItem;
            throw no_layout_item();
        }
        bool has_child_layout() const override
        {
            if (!has_layout_item())
                return false;
            if constexpr (std::is_base_of_v<i_layout, Type>)
                return true;
            else if constexpr (std::is_base_of_v<i_widget, Type> && std::is_base_of_v<i_standard_layout_container, Type>)
                return true;
            else if constexpr (std::is_base_of_v<group_box, Type>)
                return true;
            else if constexpr (std::is_base_of_v<i_tab_page, Type>)
                return true;
            else
                return false;
        }
        i_layout& child_layout(neolib::i_string const& aChildType) const override
        {
            if (has_child_layout())
            {
                if constexpr (std::is_base_of_v<i_layout, Type>)
                    return layout_item().as_layout();
                else if constexpr (std::is_base_of_v<i_widget, Type> && std::is_base_of_v<i_standard_layout_container, Type>)
                {
                    i_standard_layout_container& container = static_cast<Type&>(layout_item().as_widget());
                    if (aChildType.to_std_string_view() == "menu_bar" && container.has_layout(standard_layout::Menu))
                        return container.menu_layout();
                    else if (aChildType.to_std_string_view() == "toolbar" && container.has_layout(standard_layout::Toolbar))
                        return container.toolbar_layout();
                    else if (aChildType.to_std_string_view() == "status_bar" && container.has_layout(standard_layout::StatusBar))
                        return container.status_bar_layout();
                    return container.client_layout();
                }
                else if constexpr (std::is_base_of_v<group_box, Type>)
                    return static_cast<Type&>(layout_item().as_widget()).item_layout();
                else if constexpr (std::is_base_of_v<i_tab_page, Type>)
                {
                    auto& page = layout_item().as_widget();
                    if (!page.has_layout())
                        page.set_layout(ref_ptr<i_layout>{ make_ref<vertical_layout>() });
                    return page.layout();
                }
            }
            throw no_child_layout();
        }
        void create_child_layout_item(i_element const& aChild, i_ref_ptr<i_layout_item>& aResult) override
        {
            if constexpr (std::is_base_of_v<i_widget, Type> && std::is_base_of_v<i_tab_page_container, Type>)
            {
                if (has_layout_item())
                {
                    std::string const tabText = aChild.id().to_std_string(); // design view shows ids (see apply_attributes)
                    auto& page = static_cast<Type&>(layout_item().as_widget()).add_tab_page(string{ tabText });
                    // the container owns the page so the element's reference is non-owning
                    aResult = ref_ptr<i_layout_item>{ ref_ptr<i_layout_item>{}, static_cast<i_layout_item*>(&page.as_widget()) };
                    return;
                }
            }
            throw no_child_layout();
        }
        void reveal() override
        {
            // make the element visible on the design surface (e.g. select the tab page it is on)
            if constexpr (std::is_base_of_v<i_tab_page, Type>)
            {
                if (has_layout_item())
                {
                    auto& tab = static_cast<Type&>(layout_item().as_widget()).tab();
                    if (!tab.is_selected())
                    {
                        neolib::scoped_flag sf{ sRevealing };
                        tab.select();
                    }
                }
            }
            if (has_parent())
                parent().reveal();
        }
    public:
        element_mode mode() const override
        {
            return iMode;
        }
        void set_mode(element_mode aMode) override
        {
            if (iMode != aMode)
            {
                iMode = aMode;
                if (mode() == element_mode::Edit)
                {
                    root().visit([&](i_element& aElement)
                    {
                        if (&aElement != this && aElement.mode() == element_mode::Edit)
                            aElement.set_mode(element_mode::None);
                    });
                }
                if (mode() == element_mode::Edit)
                    reveal();
                ModeChanged();
            }
        }
        bool is_selected() const override
        {
            return iSelected;
        }
        void select(bool aSelected = true, bool aDeselectRest = true) override
        {
            if (iSelected != aSelected)
            {
                iSelected = aSelected;
                if (iSelected)
                    reveal();
                SelectionChanged();
            }
            if (aDeselectRest)
            {
                root().visit([&](i_element& aElement)
                {
                    if (&aElement != this && aElement.is_selected())
                        aElement.select(false, false);
                });
            }
        }
    private:
        // value of the element's text attribute (or title/tab text) as plain text
        std::string design_text() const
        {
            for (std::string_view const name : { "text", "title", "tab_text" })
                for (auto const& attribute : iAttributes)
                    if (attribute.first().to_std_string_view() == name)
                    {
                        auto const value = attribute.second().to_std_string_view();
                        if (value.size() < 2u || value.front() != '"' || value.back() != '"')
                            return std::string{ value };
                        std::string result;
                        for (std::size_t i = 1u; i + 1u < value.size(); ++i)
                        {
                            char ch = value[i];
                            if (ch == '\\' && i + 2u < value.size())
                            {
                                ch = value[++i];
                                if (ch == 'n')
                                    ch = '\n';
                                else if (ch == 't')
                                    ch = '\t';
                                else if (ch == 'r')
                                    ch = '\r';
                            }
                            result += ch;
                        }
                        return result;
                    }
            return {};
        }
    private:
        const i_element_library& iLibrary;
        i_project& iProject;
        i_element* iParent;
        element_group iGroup;
        string iType;
        string iId;
        children_t iChildren;
        attributes_t iAttributes;
        sink iSink;
        inline static bool sRevealing = false;
        mutable ref_ptr<i_layout_item> iLayoutItem;
        ref_ptr<i_element_caddy> iCaddy;
        element_mode iMode = element_mode::None;
        bool iSelected = false;
    };
}
