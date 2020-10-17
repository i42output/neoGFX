// html.hpp
/*
neogfx C++ App/Game Engine
Copyright (c) 2015, 2020 Leigh Johnston.  All Rights Reserved.

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
#include <neolib/core/quick_string.hpp>
#include <neolib/core/variant.hpp>
#include <neolib/core/vecarray.hpp>
#include <neogfx/core/geometrical.hpp>
#include <neogfx/gfx/hsl_color.hpp>
#include <neogfx/gfx/hsv_color.hpp>
#include <neogfx/gfx/color.hpp>

namespace neogfx
{
    template <typename CharT, typename Alloc = std::allocator<CharT> >
    class html_node
    {
    public:
        // types
        enum type_e { Document = 0x1, Element = 0x2, Text = 0x4, Comment = 0x8, All = 0xFF };
        typedef Alloc allocator_type;
        typedef neolib::basic_quick_string<CharT> string;
        typedef html_node<CharT, allocator_type> node;
        typedef node* node_ptr;
    private:
        // types
        typedef std::list<node_ptr, typename allocator_type::template rebind<node_ptr>::other> node_list;
        /* Why std::list of pointers instead of std::vector of pointers?  std::vector is not compatible with chunk allocator
        and timings indicate performance benefit of std::list with chunk allocator compared to std::vector when parsing large (~10MB) html files. */
    public:
        // types
        class const_iterator;
        class iterator
        {
            friend class const_iterator;
        public:
            iterator() : iNode(0), iIterator(), iFilter(node::All) {}
            iterator(node& aNode, typename node::node_list::iterator aIterator, typename node::type_e aFilter = node::All) : iNode(&aNode), iIterator(aIterator), iFilter(aFilter) {}
            iterator(const iterator& aOther) : iNode(aOther.iNode), iIterator(aOther.iIterator), iFilter(aOther.iFilter) {}
            iterator& operator=(const iterator& aOther) { iNode = aOther.iNode; iIterator = aOther.iIterator; iFilter = aOther.iFilter; return *this; }
        public:
            node& operator*() const { return static_cast<node&>(**iIterator); }
            node* operator->() const { return static_cast<node*>(&**iIterator); }
            iterator& operator++()
            {
                ++iIterator;
                iterator endIterator = iNode->end(iFilter);
                while (*this != endIterator && !((*iIterator)->type() & iFilter))
                    ++iIterator;
                return *this;
            }
            iterator& operator--()
            {
                --iIterator;
                iterator beginIterator = iNode->begin(iFilter);
                while (*this != beginIterator && !((*iIterator)->type() & iFilter))
                    --iIterator;
                return *this;
            }
            iterator operator++(int) { iterator temp(*this); operator++(); return temp; }
            iterator operator--(int) { iterator temp(*this); operator--(); return temp; }
            bool operator==(const iterator& aOther) const { return iIterator == aOther.iIterator; }
            bool operator!=(const iterator& aOther) const { return !(*this == aOther); }
            typename node::node_list::iterator base() const { return iIterator; }
        private:
            node* iNode;
            typename node::node_list::iterator iIterator;
            typename node::type_e iFilter;
        };
        class const_iterator
        {
        public:
            const_iterator() : iNode(0), iIterator(), iFilter(node::All) {}
            const_iterator(const node& aNode, typename node::node_list::const_iterator aIterator, typename node::type_e aFilter = node::All) : iNode(&aNode), iIterator(aIterator), iFilter(aFilter) {}
            const_iterator(const const_iterator& aOther) : iNode(aOther.iNode), iIterator(aOther.iIterator), iFilter(aOther.iFilter) {}
            const_iterator(const iterator aIterator) : iNode(aIterator.iNode), iIterator(typename node::node_list::const_iterator(aIterator.iIterator)), iFilter(aIterator.iFilter) {}
            const_iterator& operator=(const const_iterator& aOther) { iNode = aOther.iNode; iIterator = aOther.iIterator; iFilter = aOther.iFilter; return *this; }
            const_iterator& operator=(const iterator& aOther) { iNode = aOther.iNode; iIterator = aOther.iIterator; iFilter = aOther.iFilter; return *this; }
        public:
            const node& operator*() const { return static_cast<const node&>(**iIterator); }
            const node* operator->() const { return static_cast<const node*>(&**iIterator); }
            const_iterator& operator++()
            {
                ++iIterator;
                const_iterator endIterator = iNode->end(iFilter);
                while (*this != endIterator && !((*iIterator)->type() & iFilter))
                    ++iIterator;
                return *this;
            }
            const_iterator& operator--()
            {
                --iIterator;
                const_iterator beginIterator = iNode->begin(iFilter);
                while (*this != beginIterator && !((*iIterator)->type() & iFilter))
                    --iIterator;
                return *this;
            }
            const_iterator operator++(int) { const_iterator temp(*this); operator++(); return temp; }
            const_iterator operator--(int) { const_iterator temp(*this); operator--(); return temp; }
            bool operator==(const const_iterator& aOther) const { return iIterator == aOther.iIterator; }
            bool operator!=(const const_iterator& aOther) const { return !(*this == aOther); }
            typename node::node_list::const_iterator base() const { return iIterator; }
        private:
            const node* iNode;
            typename node::node_list::const_iterator iIterator;
            typename node::type_e iFilter;
        };

    public:
        // construction
        html_node(type_e aType = Document) : iType(aType) {}
        virtual ~html_node() { clear(); }

    public:
        // operations
        type_e type() const { return iType; }
        // access
        bool empty() const { return iContent.empty(); }
        const node& back() const { return *iContent.back(); }
        node& back() { return *iContent.back(); }
        const_iterator begin(type_e aFilter = All) const;
        const_iterator end(type_e aFilter = All) const;
        iterator begin(type_e aFilter = All);
        iterator end(type_e aFilter = All);
        const_iterator find(const string& aName) const;
        template <typename Exception>
        const_iterator find_or_throw(const string& aName) const;
        iterator find(const string& aName);
        iterator find_or_append(const string& aName);
        template <typename Exception>
        iterator find_or_throw(const string& aName);
        // modifiers
        void push_back(node_ptr aNode)
        {
            std::unique_ptr<node> newNode(aNode);
            iContent.push_back(0);
            iContent.back() = newNode.release();
        }
        iterator insert(iterator aIterator, node_ptr aNode)
        {
            std::unique_ptr<node> newNode(aNode);
            typename node_list::iterator i = iContent.insert(aIterator.base(), 0);
            *i = newNode.release();
            return iterator(*this, i);
        }
        void erase(iterator aIterator)
        {
            delete *aIterator.base();
            iContent.erase(aIterator.base());
        }
        void clear()
        {
            for (typename node_list::iterator i = iContent.begin(); i != iContent.end(); ++i)
                delete *i;
            iContent.clear();
        }

    private:
        // implementation
        const node_list& content() const { return iContent; }
        node_list& content() { return iContent; }
        html_node(const html_node&) = delete; // not allowed
        html_node& operator=(const html_node&) = delete; // not allowed

    private:
        // attributes
        type_e iType;
        node_list iContent;
    };

    template <typename CharT, typename Alloc = std::allocator<CharT> >
    class html_element : public html_node<CharT, Alloc>
    {
    public:
        enum type_e
        {
            Doctype,
            Html,
            Head,
            Body,
            Heading1,
            Heading2,
            Heading3,
            Heading4,
            Heading5,
            Heading6,
            Paragraph
        };

    public:
        // allocation
        static void* operator new(std::size_t) { return typename Alloc::template rebind<html_element>::other().allocate(1); }
        static void operator delete(void* ptr) { return typename Alloc::template rebind<html_element>::other().deallocate(static_cast<html_element*>(ptr), 1); }

    public:
        // types
        typedef html_node<CharT, Alloc> node;
        typedef typename node::allocator_type allocator_type;
        typedef typename node::string string;
        typedef std::pair<const string, string> attribute;
        typedef std::map<string, string, std::less<string>, typename allocator_type::template rebind<attribute>::other> attribute_list;
        class iterator : public node::iterator
        {
        public:
            iterator(typename node::iterator aIterator) : node::iterator(aIterator) {}
            html_element& operator*() const { return static_cast<html_element&>(node::iterator::operator*()); }
            html_element* operator->() const { return static_cast<html_element*>(node::iterator::operator->()); }
        };
        class const_iterator : public node::const_iterator
        {
        public:
            const_iterator(typename node::const_iterator aIterator) : node::const_iterator(aIterator) {}
            const_iterator(const iterator& aIterator) : node::const_iterator(aIterator) {}
            const html_element& operator*() const { return static_cast<const html_element&>(node::const_iterator::operator*()); }
            const html_element* operator->() const { return static_cast<const html_element*>(node::const_iterator::operator->()); }
        };

    public:
        // construction
        html_element(type_e aType) : node(node::Element), iType(aType), iUseEmptyElementTag(true) {}

    public:
        // operations
        type_e type() const { return iType; }
        using node::insert;
        typename node::iterator insert(typename node::iterator aPosition, const CharT* aName) { return node::insert(aPosition, new html_element(aName)); }
        html_element& append(const CharT* aName) { node::push_back(new html_element(aName)); return static_cast<html_element&>(node::back()); }
        const attribute_list& attributes() const { return iAttributes; }
        bool has_attribute(const string& aAttributeName) const;
        const string& attribute_value(const string& aAttributeName) const;
        const string& attribute_value(const string& aNewAttributeName, const string& aOldAttributeName) const;
        const_iterator begin() const { return const_iterator(node::begin(node::Element)); }
        const_iterator end() const { return const_iterator(node::end(node::Element)); }
        iterator begin() { return iterator(node::begin(node::Element)); }
        iterator end() { return iterator(node::end(node::Element)); }
        const string& text() const;
        bool use_empty_element_tag() const { return iUseEmptyElementTag; }
        type_e& type() { return iType; }
        attribute_list& attributes() { return iAttributes; }
        void set_attribute(const string& aAttributeName, const string& aAttributeValue);
        void append_text(const string& aText);
        void set_use_empty_element_tag(bool aUseEmptyElementTag) { iUseEmptyElementTag = aUseEmptyElementTag; }

    private:
        // attributes
        type_e iType;
        attribute_list iAttributes;
        mutable string iText;
        bool iUseEmptyElementTag;
    };

    template <typename CharT, typename Alloc = std::allocator<CharT> >
    class html_text : public html_node<CharT, Alloc>
    {
    public:
        // allocation
        static void* operator new(std::size_t) { return typename Alloc::template rebind<html_text>::other().allocate(1); }
        static void operator delete(void* ptr) { return typename Alloc::template rebind<html_text>::other().deallocate(static_cast<html_text*>(ptr), 1); }

    public:
        // types
        typedef html_node<CharT, Alloc> node;
        typedef typename node::string string;

    public:
        // construction
        html_text(const string& aContent = string()) : node(node::Text), iContent(aContent) {}

    public:
        // operations
        const string& content() const { return iContent; }
        string& content() { return iContent; }

    private:
        // attributes
        string iContent;
    };

    template <typename CharT, typename Alloc = std::allocator<CharT> >
    class html_comment : public html_node<CharT, Alloc>
    {
    public:
        // allocation
        static void* operator new(std::size_t) { return typename Alloc::template rebind<html_comment>::other().allocate(1); }
        static void operator delete(void* ptr) { return typename Alloc::template rebind<html_comment>::other().deallocate(static_cast<html_comment*>(ptr), 1); }

    public:
        // types
        typedef html_node<CharT, Alloc> node;
        typedef typename node::string string;

    public:
        // construction
        html_comment(const string& aContent = string()) : node(node::Comment), iContent(aContent) {}

    public:
        // operations
        const string& content() const { return iContent; }
        string& content() { return iContent; }

    private:
        // attributes
        string iContent;
    };

    class html
    {
    public:
        struct failed_to_open_html : std::runtime_error { failed_to_open_html() : std::runtime_error("neogfx::html::failed_to_open_html") {} };
    public:
        html(std::string const& aFragment);
        html(std::istream& aDocument);
    private:
        void parse();
    private:
        std::shared_ptr<std::istream> iDocument;
    };
}
