// item_model.hpp
/*
  neogfx C++ GUI Library
  Copyright (c) 2015 Leigh Johnston.  All Rights Reserved.
  
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
#include <vector>
#include <deque>
#include <boost/algorithm/string.hpp>

#include <neolib/vecarray.hpp>
#include <neolib/segmented_array.hpp>

#include <neogfx/core/object.hpp>
#include <neogfx/gfx/i_graphics_context.hpp>
#include <neogfx/gui/widget/i_item_model.hpp>
#include <neogfx/gui/widget/i_basic_item_model.hpp>

namespace neogfx
{
    class default_item_flat_container_traits
    {
    public:
        struct operation_not_supported : std::logic_error { operation_not_supported() : std::logic_error("neogfx::default_item_flat_container_traits::operation_not_supported") {} };
    public:
        template <typename Container, typename T>
        static typename Container::iterator append(Container& aContainer, typename Container::const_iterator aPosition, const T& aValue)
        {
            return aContainer.insert(aPosition, aValue);
        }
        template <typename Container>
        static i_item_model::iterator sibling_begin(Container& aContainer)
        {
            return neolib::make_generic_iterator(aContainer.begin());
        }
        template <typename Container>
        static i_item_model::const_iterator sibling_begin(const Container& aContainer)
        {
            return neolib::make_generic_iterator(aContainer.begin());
        }
        template <typename Container>
        static i_item_model::iterator sibling_end(Container& aContainer)
        {
            return neolib::make_generic_iterator(aContainer.end());
        }
        template <typename Container>
        static i_item_model::const_iterator sibling_end(const Container& aContainer)
        {
            return neolib::make_generic_iterator(aContainer.end());
        }
        template <typename Container>
        static i_item_model::iterator parent(Container&, i_item_model::iterator)
        {
            throw operation_not_supported();
        }
        template <typename Container>
        static i_item_model::const_iterator parent(const Container&, i_item_model::const_iterator)
        {
            throw operation_not_supported();
        }
        template <typename Container>
        static i_item_model::iterator sibling_begin(Container&, i_item_model::iterator)
        {
            throw operation_not_supported();
        }
        template <typename Container>
        static i_item_model::const_iterator sibling_begin(const Container&, i_item_model::const_iterator)
        {
            throw operation_not_supported();
        }
        template <typename Container>
        static i_item_model::iterator sibling_end(Container&, i_item_model::iterator)
        {
            throw operation_not_supported();
        }
        template <typename Container>
        static i_item_model::const_iterator sibling_end(const Container&, i_item_model::const_iterator)
        {
            throw operation_not_supported();
        }
    };

    template <typename T, typename CellType, uint32_t Columns>
    class item_flat_container_traits : public default_item_flat_container_traits
    {
    public:
        typedef T value_type;
        typedef std::allocator<value_type> allocator_type;
        typedef CellType cell_type;
        typedef neolib::vecarray<cell_type, Columns, Columns, neolib::check<neolib::vecarray_overflow>, typename allocator_type::template rebind<cell_type>::other> row_container_type;
        typedef std::pair<value_type, row_container_type> row_type;
        typedef std::vector<row_type, typename allocator_type::template rebind<row_type>::other> container_type;
        typedef typename container_type::iterator sibling_iterator;
        typedef typename container_type::const_iterator const_sibling_iterator;
    public:
        template <typename T2, typename CellType2>
        struct rebind
        {
            typedef item_flat_container_traits<T2, CellType2, Columns> other;
        };
    };

    template <typename T, typename CellType>
    class item_flat_container_traits<T, CellType, 0> : public default_item_flat_container_traits
    {
    public:
        typedef T value_type;
        typedef std::allocator<value_type> allocator_type;
        typedef CellType cell_type;
        typedef std::vector<cell_type, typename allocator_type::template rebind<cell_type>::other> row_container_type;
        typedef std::pair<value_type, row_container_type> row_type;
        typedef std::vector<row_type, typename allocator_type::template rebind<row_type>::other> container_type;
        typedef typename container_type::iterator sibling_iterator;
        typedef typename container_type::const_iterator const_sibling_iterator;
    public:
        template <typename T2, typename CellType2>
        struct rebind
        {
            typedef item_flat_container_traits<T2, CellType2, 0> other;
        };
    };

    template <typename T, uint32_t Columns = 0, typename CellType = item_cell_data, typename ContainerTraits = item_flat_container_traits<T, CellType, Columns>>
    class basic_item_model : public object<i_basic_item_model<T>>
    {
    public:
        define_declared_event(ColumnInfoChanged, column_info_changed, item_model_index::column_type)
        define_declared_event(ItemAdded, item_added, const item_model_index&)
        define_declared_event(ItemChanged, item_changed, const item_model_index&)
        define_declared_event(ItemRemoved, item_removed, const item_model_index&)
    public:
        typedef ContainerTraits container_traits;
        typedef typename container_traits::value_type value_type;
        typedef typename container_traits::allocator_type allocator_type;
        typedef typename container_traits::row_container_type row_container_type;
        typedef typename container_traits::container_type container_type;
        typedef typename container_type::value_type row_type;
        typedef typename container_traits::cell_type cell_type;
        typedef typename container_type::iterator iterator;
        typedef typename container_type::const_iterator const_iterator;
        typedef neolib::specialized_generic_iterator<iterator> base_iterator;
        typedef neolib::specialized_generic_iterator<const_iterator> const_base_iterator;
        typedef typename container_traits::sibling_iterator sibling_iterator;
        typedef typename container_traits::const_sibling_iterator const_sibling_iterator;
        typedef typename row_type::second_type::iterator column_iterator;
        typedef typename row_type::second_type::const_iterator const_column_iterator;
    private:
        struct column_info
        {
            std::string name;
            mutable optional_item_cell_info defaultDataInfo;
        };
        typedef typename container_traits::template rebind<item_model_index::row_type, column_info>::other::row_container_type column_info_container_type;
    public:
        basic_item_model()
        {
        }
        ~basic_item_model()
        {
            set_destroying();
        }
    public:
        uint32_t rows() const override
        {
            return static_cast<uint32_t>(iItems.size());
        }
        uint32_t columns() const override
        {
            return static_cast<uint32_t>(iColumns.size());
        }
        uint32_t columns(const item_model_index& aIndex) const override
        {
            return static_cast<uint32_t>(iItems[aIndex.row()].second.size());
        }
        const std::string& column_name(item_model_index::value_type aColumnIndex) const override
        {
            if (iColumns.size() < aColumnIndex + 1u)
                throw bad_column_index();
            return iColumns[aColumnIndex].name;
        }
        void set_column_name(item_model_index::value_type aColumnIndex, const std::string& aName) override
        {
            if (iColumns.size() < aColumnIndex + 1u)
                iColumns.resize(aColumnIndex + 1u);
            iColumns[aColumnIndex].name = aName;
            ColumnInfoChanged.trigger(aColumnIndex);
        }
        bool column_selectable(item_model_index::column_type aColumnIndex) const override
        {
            return (default_cell_info(aColumnIndex).flags & item_cell_flags::Selectable) == item_cell_flags::Selectable;
        }
        void set_column_selectable(item_model_index::column_type aColumnIndex, bool aSelectable) override
        {
            if (aSelectable)
                default_cell_info(aColumnIndex).flags |= item_cell_flags::Selectable;
            else
                default_cell_info(aColumnIndex).flags &= ~item_cell_flags::Selectable;
            ColumnInfoChanged.trigger(aColumnIndex);
        }
        bool column_read_only(item_model_index::column_type aColumnIndex) const override
        {
            return (default_cell_info(aColumnIndex).flags & item_cell_flags::Editable) != item_cell_flags::Editable;
        }
        void set_column_read_only(item_model_index::column_type aColumnIndex, bool aReadOnly) override
        {
            if (aReadOnly)
                default_cell_info(aColumnIndex).flags &= ~item_cell_flags::Editable;
            else
                default_cell_info(aColumnIndex).flags |= item_cell_flags::Editable;
            ColumnInfoChanged.trigger(aColumnIndex);
        }
        bool column_checkable(item_model_index::column_type aColumnIndex) const override
        {
            return (default_cell_info(aColumnIndex).flags & item_cell_flags::Checkable) == item_cell_flags::Checkable;
        }
        void set_column_checkable(item_model_index::column_type aColumnIndex, bool aCheckable) override
        {
            if (aCheckable)
                default_cell_info(aColumnIndex).flags |= item_cell_flags::Checkable;
            else
                default_cell_info(aColumnIndex).flags &= ~item_cell_flags::Checkable;
            ColumnInfoChanged.trigger(aColumnIndex);
        }
        bool column_tri_state_checkable(item_model_index::column_type aColumnIndex) const override
        {
            return (default_cell_info(aColumnIndex).flags & item_cell_flags::CheckableTriState) == item_cell_flags::CheckableTriState;
        }
        void set_column_tri_state_checkable(item_model_index::column_type aColumnIndex, bool aCheckableTriState) override
        {
            if (aCheckableTriState)
                default_cell_info(aColumnIndex).flags |= item_cell_flags::CheckableTriState;
            else
                default_cell_info(aColumnIndex).flags &= ~item_cell_flags::CheckableTriState;
            ColumnInfoChanged.trigger(aColumnIndex);
        }
        item_data_type column_data_type(item_model_index::column_type aColumnIndex) const override
        {
            return default_cell_info(aColumnIndex).dataType;
        }
        void set_column_data_type(item_model_index::column_type aColumnIndex, item_data_type aType) override
        {
            default_cell_info(aColumnIndex).dataType = aType;
            ColumnInfoChanged.trigger(aColumnIndex);
        }
        const item_cell_data& column_min_value(item_model_index::column_type aColumnIndex) const override
        {
            return default_cell_info(aColumnIndex).dataMin;
        }
        void set_column_min_value(item_model_index::column_type aColumnIndex, const item_cell_data& aValue) override
        {
            default_cell_info(aColumnIndex).dataMin = aValue;
            ColumnInfoChanged.trigger(aColumnIndex);
        }
        const item_cell_data& column_max_value(item_model_index::column_type aColumnIndex) const override
        {
            return default_cell_info(aColumnIndex).dataMax;
        }
        void set_column_max_value(item_model_index::column_type aColumnIndex, const item_cell_data& aValue) override
        {
            default_cell_info(aColumnIndex).dataMax = aValue;
            ColumnInfoChanged.trigger(aColumnIndex);
        }
        const item_cell_data& column_step_value(item_model_index::column_type aColumnIndex) const override
        {
            return default_cell_info(aColumnIndex).dataStep;
        }
        void set_column_step_value(item_model_index::column_type aColumnIndex, const item_cell_data& aValue) override
        {
            default_cell_info(aColumnIndex).dataStep = aValue;
            ColumnInfoChanged.trigger(aColumnIndex);
        }
    public:
        i_item_model::iterator index_to_iterator(const item_model_index& aIndex) override
        {
            return base_iterator(std::next(iItems.begin(), aIndex.row()));
        }
        i_item_model::const_iterator index_to_iterator(const item_model_index& aIndex) const override
        {
            return const_base_iterator(std::next(iItems.begin(), aIndex.row()));
        }
        item_model_index iterator_to_index(i_item_model::const_iterator aPosition) const override
        {
            return item_model_index(std::distance(iItems.begin(), const_base_iterator(aPosition).get<const_iterator, const_iterator, iterator, const_sibling_iterator, sibling_iterator>()), 0);
        }
        i_item_model::iterator begin() override
        {
            return base_iterator(iItems.begin());
        }
        i_item_model::const_iterator begin() const override
        {
            return const_base_iterator(iItems.begin());
        }
        i_item_model::iterator end() override
        {
            return base_iterator(iItems.end());
        }
        i_item_model::const_iterator end() const override
        {
            return const_base_iterator(iItems.end());
        }
        i_item_model::iterator sibling_begin() override
        {
            return base_iterator(container_traits::sibling_begin(iItems));
        }
        i_item_model::const_iterator sibling_begin() const override
        {
            return const_base_iterator(container_traits::sibling_begin(iItems));
        }
        i_item_model::iterator sibling_end() override
        {
            return base_iterator(container_traits::sibling_end(iItems));
        }
        i_item_model::const_iterator sibling_end() const override
        {
            return const_base_iterator(container_traits::sibling_end(iItems));
        }
        i_item_model::iterator parent(i_item_model::iterator aChild) override
        {
            return base_iterator(container_traits::parent(iItems, aChild));
        }
        i_item_model::const_iterator parent(i_item_model::const_iterator aChild) const override
        {
            return const_base_iterator(container_traits::parent(iItems, aChild));
        }
        i_item_model::iterator sibling_begin(i_item_model::iterator aParent) override
        {
            return base_iterator(container_traits::sibling_begin(iItems, aParent));
        }
        i_item_model::const_iterator sibling_begin(i_item_model::const_iterator aParent) const override
        {
            return const_base_iterator(container_traits::sibling_begin(iItems, aParent));
        }
        i_item_model::iterator sibling_end(i_item_model::iterator aParent) override
        {
            return base_iterator(container_traits::sibling_end(iItems, aParent));
        }
        i_item_model::const_iterator sibling_end(i_item_model::const_iterator aParent) const override
        {
            return const_base_iterator(container_traits::sibling_end(iItems, aParent));
        }
    public:
        const item_cell_data& cell_data(const item_model_index& aIndex) const override
        {
            if (aIndex.column() < iItems[aIndex.row()].second.size())
                return iItems[aIndex.row()].second[aIndex.column()];
            static const item_cell_data sEmpty;
            return sEmpty;
        }
        const item_cell_info& cell_info(const item_model_index& aIndex) const override
        {
            return default_cell_info(aIndex.column());
        }
    public:
        bool empty() const override
        {
            return iItems.empty();
        }
        void reserve(uint32_t aItemCount) override
        {
            iItems.reserve(aItemCount);
        }
        uint32_t capacity() const override
        {
            return static_cast<uint32_t>(iItems.capacity());
        }
        i_item_model::iterator insert_item(i_item_model::const_iterator aPosition, const value_type& aValue) override
        {
            base_iterator i = iItems.insert(aPosition.get<const_iterator, const_iterator, iterator, const_sibling_iterator, sibling_iterator>(), row_type(aValue, row_container_type()));
            ItemAdded.trigger(iterator_to_index(i));
            return i;
        }
        i_item_model::iterator insert_item(i_item_model::const_iterator aPosition, const value_type& aValue, const item_cell_data& aCellData) override
        {
            auto i = insert_item(aPosition, aValue);
            insert_cell_data(i, 0, aCellData);
            return i;
        }
        i_item_model::iterator insert_item(i_item_model::const_iterator aPosition, const item_cell_data& aCellData) override
        {
            return insert_item(aPosition, value_type(), aCellData);
        }
        i_item_model::iterator insert_item(const item_model_index& aIndex, const item_cell_data& aCellData) override
        {
            return insert_item(index_to_iterator(aIndex), aCellData);
        }
        i_item_model::iterator append_item(i_item_model::const_iterator aParent, const value_type& aValue) override
        {
            base_iterator i = container_traits::append(iItems, aParent.get<const_iterator, const_iterator, iterator, const_sibling_iterator, sibling_iterator>(), row_type(aValue, row_container_type()));
            ItemAdded.trigger(iterator_to_index(i));
            return i;
        }
        i_item_model::iterator append_item(i_item_model::const_iterator aParent, const value_type& aValue, const item_cell_data& aCellData) override
        {
            auto i = append_item(aParent, aValue);
            insert_cell_data(i, 0, aCellData);
            return i;
        }
        i_item_model::iterator append_item(i_item_model::const_iterator aParent, const item_cell_data& aCellData) override
        {
            return append_item(aParent, value_type(), aCellData);
        }
        void clear() override
        {
            while (!empty())
                erase(begin());
        }
        void erase(i_item_model::const_iterator aPosition) override
        {
            ItemRemoved.trigger(iterator_to_index(aPosition));
            iItems.erase(aPosition.get<const_iterator, const_iterator, iterator, const_sibling_iterator, sibling_iterator>());
        }
        void insert_cell_data(i_item_model::iterator aItem, item_model_index::value_type aColumnIndex, const item_cell_data& aCellData) override
        {
            bool changed = false;
            auto ri = aItem.get<iterator, iterator, sibling_iterator>();
            if (ri->second.size() < aColumnIndex + 1)
            {
                ri->second.resize(aColumnIndex + 1);
                changed = true;
            }
            if (iColumns.size() < aColumnIndex + 1)
            {
                iColumns.resize(aColumnIndex + 1);
                changed = true;
            }
            if (default_cell_info(aColumnIndex).dataType == item_data_type::Unknown)
            {
                default_cell_info(aColumnIndex).dataType = static_cast<item_data_type>(aCellData.index());
                changed = true;
            }
            if (ri->second[aColumnIndex] != aCellData)
            {
                ri->second[aColumnIndex] = aCellData;
                changed = true;
            }
            if (changed)
            {
                item_model_index index = iterator_to_index(aItem);
                index.set_column(aColumnIndex);
                ItemChanged.trigger(index);
            }
        }
        void insert_cell_data(const item_model_index& aIndex, const item_cell_data& aCellData) override
        {
            insert_cell_data(index_to_iterator(aIndex), aIndex.column(), aCellData);
        }
        void update_cell_data(const item_model_index& aIndex, const item_cell_data& aCellData) override
        {
            if (iItems[aIndex.row()].second[aIndex.column()] == aCellData)
                return;
            iItems[aIndex.row()].second[aIndex.column()] = aCellData;
            if (default_cell_info(aIndex.column()).dataType == item_data_type::Unknown)
                default_cell_info(aIndex.column()).dataType = static_cast<item_data_type>(aCellData.index());
            ItemChanged.trigger(aIndex);
        }
    public:
        value_type& item(const item_model_index& aIndex) override
        {
            return iItems[aIndex.row()].first;
        }
        const value_type& item(const item_model_index& aIndex) const override
        {
            return iItems[aIndex.row()].first;
        }
    private:
        const item_cell_info& default_cell_info(item_model_index::column_type aColumnIndex) const
        {
            if (iColumns.size() < aColumnIndex + 1)
                throw bad_column_index();
            if (iColumns[aColumnIndex].defaultDataInfo != std::nullopt)
                return *iColumns[aColumnIndex].defaultDataInfo;
            else
            {
                static const item_cell_info sZero;
                return sZero;
            }
        }
        item_cell_info& default_cell_info(item_model_index::column_type aColumnIndex)
        {
            if (iColumns.size() < aColumnIndex + 1)
                iColumns.resize(aColumnIndex + 1);
            if (iColumns[aColumnIndex].defaultDataInfo == std::nullopt)
                iColumns[aColumnIndex].defaultDataInfo = item_cell_info{};
            return *iColumns[aColumnIndex].defaultDataInfo;
        }
    private:
        container_type iItems;
        column_info_container_type iColumns;
    };

    typedef basic_item_model<void*> item_model;
}