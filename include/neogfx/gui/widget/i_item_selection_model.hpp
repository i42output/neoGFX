// i_item_selection_model.hpp
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
#include <neolib/core/i_map.hpp>
#include <neogfx/core/i_event.hpp>
#include <neogfx/core/i_property.hpp>
#include <neogfx/gui/widget/item_index.hpp>

namespace neogfx
{
    class i_item_presentation_model;

    enum class item_selection_mode
    {
        NoSelection,
        SingleSelection,
        MultipleSelection,
        ExtendedSelection
    };

    enum class item_selection_operation
    {
        None                 = 0x0000,
        Select               = 0x0001,
        Deselect             = 0x0002,
        Toggle               = 0x0004,
        Clear                = 0x0008,
        Row                  = 0x0100,
        Column               = 0x0200,
        Internal             = 0x1000,

        SelectRow            = Select | Row,
        SelectColumn         = Select | Column,
        DeselectRow          = Deselect | Row,
        DeselectColumn       = Deselect | Column,
        ToggleRow            = Toggle | Row,
        ToggleColumn         = Toggle | Column,
        ClearAndSelect       = Clear | Select,
        ClearAndSelectRow    = Clear | Select | Row,
        ClearAndSelectColumn = Clear | Select | Column,
        ClearAndToggle       = Clear | Toggle
    };

    enum class index_location
    {
        None,
        FirstCell,
        LastCell,
        PreviousCell,
        NextCell,
        StartOfCurrentRow,
        EndOfCurrentRow,
        StartOfCurrentColumn,
        EndOfCurrentColumn,
        CellToLeft,
        CellToRight,
        RowAbove,
        RowBelow
    };

    inline item_selection_operation operator|(item_selection_operation aLhs, item_selection_operation aRhs)
    {
        return static_cast<item_selection_operation>(static_cast<uint32_t>(aLhs) | static_cast<uint32_t>(aRhs));
    }

    inline item_selection_operation operator&(item_selection_operation aLhs, item_selection_operation aRhs)
    {
        return static_cast<item_selection_operation>(static_cast<uint32_t>(aLhs)& static_cast<uint32_t>(aRhs));
    }

    inline item_selection_operation operator~(item_selection_operation aLhs)
    {
        return static_cast<item_selection_operation>(~static_cast<uint32_t>(aLhs));
    }

    struct selection_area
    {
        typedef selection_area abstract_type; // todo: create abstract interface
        item_presentation_model_index topLeft;
        item_presentation_model_index bottomRight;
    };

    using item_selection = neolib::i_map<item_presentation_model_index, selection_area>;

    class selection_iterator
    {
        friend class i_item_selection_model;
    public:
        using value_type = item_presentation_model_index;
        using pointer = value_type const*;
        using reference = value_type const&;
        using difference_type = std::ptrdiff_t;
        using iterator_catagory = std::forward_iterator_tag;
    public:
        selection_iterator() : 
            iSelection{ nullptr }, iIterator {}
        {
        }
    private:
        selection_iterator(item_selection const& aSelection, item_selection::const_iterator aPosition) :
            iSelection{ &aSelection }, iIterator{ aPosition }
        {
        }
    public:
        bool operator==(const selection_iterator& aRhs) const
        {
            if (singular() || aRhs.singular())
                return false;
            else if (&selection() != &aRhs.selection())
                return false;
            else if (is_end() || aRhs.is_end())
                return is_end() == aRhs.is_end();
            else if (iterator() != aRhs.iterator())
                return false;
            else
                return index() == aRhs.index();
        }
        bool operator!=(const selection_iterator& aRhs) const
        {
            return !(*this == aRhs);
        }
    public:
        reference operator*() const
        {
            return index();
        }
        pointer operator->() const
        {
            return &index();
        }
    public:
        selection_iterator& operator++()
        {
            index() += item_presentation_model_index{ 0, 1 };
            if (index().column() > iIterator->second().bottomRight.column())
            {
                index().set_column(iIterator->second().topLeft.column());
                index() += item_presentation_model_index{ 1, 0 };
                if (index().row() > iIterator->second().bottomRight.row())
                {
                    ++iterator();
                    index() = {};
                }
            }
            return *this;
        }
    private:
        bool singular() const
        {
            return iSelection == nullptr;
        }
        bool is_end() const
        {
            return !singular() && iterator() == selection().end();
        }
        item_selection const& selection() const
        {
            return *iSelection;
        }
        item_selection::const_iterator const& iterator() const
        {
            return iIterator;
        }
        item_selection::const_iterator& iterator()
        {
            return iIterator;
        }
        item_presentation_model_index const& index() const
        {
            if (iIndex == std::nullopt)
                iIndex = iterator()->first();
            return *iIndex;
        }
        item_presentation_model_index& index()
        {
            return const_cast<item_presentation_model_index&>(const_cast<selection_iterator const&>(*this).index());
        }
    private:
        item_selection const* iSelection;
        item_selection::const_iterator iIterator;
        mutable std::optional<item_presentation_model_index> iIndex;
    };

    class i_item_selection_model : public i_reference_counted, public i_property_owner
    {
    public:
        declare_event(current_index_changed, const optional_item_presentation_model_index& /* aCurrentIndex */, const optional_item_presentation_model_index& /* aPreviousIndex */)
        declare_event(selection_changed, const item_selection&, const item_selection&)
        declare_event(presentation_model_added, i_item_presentation_model&)
        declare_event(presentation_model_changed, i_item_presentation_model&, i_item_presentation_model&)
        declare_event(presentation_model_removed, i_item_presentation_model&)
        declare_event(mode_changed, item_selection_mode)
    public:
        struct no_presentation_model : std::logic_error { no_presentation_model() : std::logic_error("neogfx::i_item_selection_model::no_presentation_model") {} };
        struct no_current_index : std::logic_error { no_current_index() : std::logic_error("neogfx::i_item_selection_model::no_current_index") {} };
    public:
        typedef i_item_selection_model abstract_type;
    public:
        virtual ~i_item_selection_model() = default;
    public:
        virtual bool has_presentation_model() const = 0;
        virtual i_item_presentation_model& presentation_model() const = 0;
        virtual void set_presentation_model(i_item_presentation_model& aModel) = 0;
    public:
        virtual item_selection_mode mode() const = 0;
        virtual void set_mode(item_selection_mode aType) = 0;
    public:
        virtual bool has_current_index() const = 0;
        virtual item_presentation_model_index const& current_index() const = 0;
        virtual void set_current_index(item_presentation_model_index const& aIndex) = 0;
        virtual void unset_current_index() = 0;
        virtual item_presentation_model_index relative_to_current_index(index_location aRelativeLocation, bool aSelectable = true, bool aEditable = false) const = 0;
        virtual item_presentation_model_index relative_to_index(item_presentation_model_index const& aIndex, index_location aRelativeLocation, bool aSelectable = true, bool aEditable = false) const = 0;
        virtual item_presentation_model_index next_cell() const = 0;
        virtual item_presentation_model_index next_cell(item_presentation_model_index const& aIndex) const = 0;
        virtual item_presentation_model_index previous_cell() const = 0;
        virtual item_presentation_model_index previous_cell(item_presentation_model_index const& aIndex) const = 0;
    public:
        virtual const item_selection& selection() const = 0;
        virtual bool is_selected(item_presentation_model_index const& aIndex) const = 0;
        virtual bool is_selectable(item_presentation_model_index const& aIndex) const = 0;
        virtual void select(item_presentation_model_index const& aIndex, item_selection_operation aOperation) = 0;
    public:
        virtual bool sorting() const = 0;
        virtual bool filtering() const = 0;
    public:
        virtual bool is_editable(item_presentation_model_index const& aIndex) const = 0;
        // helpers
    public:
        optional_item_presentation_model_index current_index_maybe() const
        {
            if (has_current_index())
                return current_index();
            return {};
        }
        void select(item_presentation_model_index const& aIndex)
        {
            switch (mode())
            {
            case item_selection_mode::NoSelection:
                break;
            case item_selection_mode::SingleSelection:
                select(aIndex, item_selection_operation::ClearAndSelect);
                break;
            case item_selection_mode::MultipleSelection:
                select(aIndex, item_selection_operation::ClearAndSelect);
                break;
            case item_selection_mode::ExtendedSelection:
                select(aIndex, item_selection_operation::Select);
                break;
            }
        }
        void clear(item_presentation_model_index const& aIndex)
        {
            select(aIndex, item_selection_operation::Clear);
        }
        selection_iterator begin() const
        {
            return selection_iterator{ selection(), selection().begin() };
        }
        selection_iterator end() const
        {
            return selection_iterator{ selection(), selection().end() };
        }
    };
}