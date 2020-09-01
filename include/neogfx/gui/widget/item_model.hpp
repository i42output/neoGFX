// item_model.hpp
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
#include <vector>
#include <deque>
#include <boost/algorithm/string.hpp>
#include <neolib/core/vecarray.hpp>
#include <neolib/core/segmented_array.hpp>
#include <neolib/core/tree.hpp>
#include <neogfx/core/object.hpp>
#include <neogfx/gfx/i_graphics_context.hpp>
#include <neogfx/gui/widget/i_item_model.hpp>
#include <neogfx/gui/widget/i_basic_item_model.hpp>

namespace neogfx
{
    template <typename T, typename CellType, uint32_t Columns, bool CellsCached>
    class item_row_traits;

    template <typename T, typename CellType, uint32_t Columns>
    class item_row_traits<T, CellType, Columns, false>
    {
    public:
        typedef T value_type;
        typedef std::allocator<value_type> allocator_type;
        typedef CellType cell_type;
        typedef neolib::vecarray<cell_type, Columns, Columns, neolib::check<neolib::vecarray_overflow>, typename std::allocator_traits<allocator_type>::template rebind_alloc<cell_type>> row_cell_array;
        struct row
        {
            value_type value;
            row_cell_array cells;
        };
    };

    template <typename T, typename CellType, uint32_t Columns>
    class item_row_traits<T, CellType, Columns, true>
    {
    public:
        typedef T value_type;
        typedef std::allocator<value_type> allocator_type;
        typedef CellType cell_type;
        typedef neolib::vecarray<cell_type, Columns, Columns, neolib::check<neolib::vecarray_overflow>, typename std::allocator_traits<allocator_type>::template rebind_alloc<cell_type>> row_cell_array;
        struct row
        {
            value_type value;
            mutable row_cell_array cells;
        };
    };

    template <typename T, typename CellType>
    class item_row_traits<T, CellType, 0, false>
    {
    public:
        typedef T value_type;
        typedef std::allocator<value_type> allocator_type;
        typedef CellType cell_type;
        typedef std::vector<cell_type, typename std::allocator_traits<allocator_type>::template rebind_alloc<cell_type>> row_cell_array;
        struct row
        {
            value_type value;
            row_cell_array cells;
        };
    };

    template <typename T, typename CellType>
    class item_row_traits<T, CellType, 0, true>
    {
    public:
        typedef T value_type;
        typedef std::allocator<value_type> allocator_type;
        typedef CellType cell_type;
        typedef std::vector<cell_type, typename std::allocator_traits<allocator_type>::template rebind_alloc<cell_type>> row_cell_array;
        struct row
        {
            value_type value;
            mutable row_cell_array cells;
        };
    };

    template <typename T, typename CellType, uint32_t Columns, bool CellsCached = false>
    class item_flat_container_traits
    {
    public:
        static constexpr bool is_flat = true;
        static constexpr bool is_tree = false;
    public:
        typedef T value_type;
        typedef std::allocator<value_type> allocator_type;
        typedef CellType cell_type;
        typedef typename item_row_traits<T, CellType, Columns, CellsCached>::row_cell_array row_cell_array;
        typedef typename item_row_traits<T, CellType, Columns, CellsCached>::row row;
        typedef std::vector<row, typename std::allocator_traits<allocator_type>::template rebind_alloc<row>> container_type;
        typedef typename container_type::iterator iterator;
        typedef typename container_type::const_iterator const_iterator;
        typedef typename iterator sibling_iterator;
        typedef typename const_iterator const_sibling_iterator;
        typedef typename iterator skip_iterator;
        typedef typename const_iterator const_skip_iterator;
    public:
        template <typename T2, typename CellType2, bool CellsCached2 = CellsCached>
        struct rebind
        {
            typedef item_flat_container_traits<T2, CellType2, Columns, CellsCached2> other;
        };
    };

    template <typename T, typename CellType, uint32_t Columns, bool CellsCached = false>
    class item_tree_container_traits
    {
    public:
        static constexpr bool is_flat = false;
        static constexpr bool is_tree = true;
    public:
        typedef T value_type;
        typedef std::allocator<value_type> allocator_type;
        typedef CellType cell_type;
        typedef typename item_row_traits<T, CellType, Columns, CellsCached>::row_cell_array row_cell_array;
        typedef typename item_row_traits<T, CellType, Columns, CellsCached>::row row;
        typedef neolib::tree<row, 64, typename std::allocator_traits<allocator_type>::template rebind_alloc<row>> container_type;
        typedef typename container_type::iterator iterator;
        typedef typename container_type::const_iterator const_iterator;
        typedef typename container_type::sibling_iterator sibling_iterator;
        typedef typename container_type::const_sibling_iterator const_sibling_iterator;
        typedef typename container_type::skip_iterator skip_iterator;
        typedef typename container_type::const_skip_iterator const_skip_iterator;
    public:
        template <typename T2, typename CellType2, bool CellsCached2 = CellsCached>
        struct rebind
        {
            typedef item_tree_container_traits<T2, CellType2, Columns, CellsCached2> other;
        };
    };

    template <typename T, typename CellType, bool CellsCached>
    class item_flat_container_traits<T, CellType, 0, CellsCached>
    {
    public:
        static constexpr bool is_flat = true;
        static constexpr bool is_tree = false;
    public:
        typedef T value_type;
        typedef std::allocator<value_type> allocator_type;
        typedef CellType cell_type;
        typedef typename item_row_traits<T, CellType, 0, CellsCached>::row_cell_array row_cell_array;
        typedef typename item_row_traits<T, CellType, 0, CellsCached>::row row;
        typedef std::vector<row, typename std::allocator_traits<allocator_type>::template rebind_alloc<row>> container_type;
        typedef typename container_type::iterator iterator;
        typedef typename container_type::const_iterator const_iterator;
        typedef typename iterator sibling_iterator;
        typedef typename const_iterator const_sibling_iterator;
        typedef typename iterator skip_iterator;
        typedef typename const_iterator const_skip_iterator;
    public:
        template <typename T2, typename CellType2, bool CellsCached2 = CellsCached>
        struct rebind
        {
            typedef item_flat_container_traits<T2, CellType2, 0, CellsCached2> other;
        };
    };

    template <typename T, typename CellType, bool CellsCached>
    class item_tree_container_traits<T, CellType, 0, CellsCached>
    {
    public:
        static constexpr bool is_flat = false;
        static constexpr bool is_tree = true;
    public:
        typedef T value_type;
        typedef std::allocator<value_type> allocator_type;
        typedef CellType cell_type;
        typedef typename item_row_traits<T, CellType, 0, CellsCached>::row_cell_array row_cell_array;
        typedef typename item_row_traits<T, CellType, 0, CellsCached>::row row;
        typedef neolib::tree<row, 64, typename std::allocator_traits<allocator_type>::template rebind_alloc<row>> container_type;
        typedef typename container_type::iterator iterator;
        typedef typename container_type::const_iterator const_iterator;
        typedef typename container_type::sibling_iterator sibling_iterator;
        typedef typename container_type::const_sibling_iterator const_sibling_iterator;
        typedef typename container_type::skip_iterator skip_iterator;
        typedef typename container_type::const_skip_iterator const_skip_iterator;
    public:
        template <typename T2, typename CellType2, bool CellsCached2 = CellsCached>
        struct rebind
        {
            typedef item_tree_container_traits<T2, CellType2, 0, CellsCached2> other;
        };
    };

    template <typename T, uint32_t Columns = 0, typename CellType = item_cell_data, typename ContainerTraits = item_flat_container_traits<T, CellType, Columns>>
    class basic_item_model : public object<i_basic_item_model<T>>
    {
        typedef object<i_basic_item_model<T>> base_type;
    public:
        define_declared_event(ColumnInfoChanged, column_info_changed, item_model_index::column_type)
        define_declared_event(ItemAdded, item_added, const item_model_index&)
        define_declared_event(ItemChanged, item_changed, const item_model_index&)
        define_declared_event(ItemRemoved, item_removed, const item_model_index&)
        define_declared_event(Cleared, cleared)
    public:
        typedef ContainerTraits container_traits;
        typedef typename container_traits::value_type value_type;
        typedef typename container_traits::allocator_type allocator_type;
        typedef typename container_traits::row_cell_array row_cell_array;
        typedef typename container_traits::container_type container_type;
        typedef typename container_type::value_type row_type;
        typedef typename container_traits::cell_type cell_type;
        typedef typename container_type::iterator iterator;
        typedef typename container_type::const_iterator const_iterator;
        typedef neolib::specialized_generic_iterator<iterator> base_iterator;
        typedef neolib::specialized_generic_iterator<const_iterator> const_base_iterator;
        typedef typename container_traits::sibling_iterator sibling_iterator;
        typedef typename container_traits::const_sibling_iterator const_sibling_iterator;
        typedef typename row_cell_array::iterator column_iterator;
        typedef typename row_cell_array::const_iterator const_column_iterator;
    private:
        struct column_info
        {
            std::string name;
            mutable optional_item_cell_info defaultDataInfo;
        };
        typedef typename container_traits::template rebind<item_model_index::row_type, column_info>::other::row_cell_array column_info_array;
    public:
        basic_item_model()
        {
            base_type::set_alive();
        }
        ~basic_item_model()
        {
            base_type::set_destroying();
        }
    public:
        bool is_tree() const override
        {
            return container_traits::is_tree;
        }
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
            return static_cast<uint32_t>(row(aIndex).cells.size());
        }
        const std::string& column_name(item_model_index::value_type aColumnIndex) const override
        {
            if (iColumns.size() < aColumnIndex + 1u)
                throw base_type::bad_column_index();
            return iColumns[aColumnIndex].name;
        }
        void set_column_name(item_model_index::value_type aColumnIndex, const std::string& aName) override
        {
            if (iColumns.size() < aColumnIndex + 1u)
                iColumns.resize(aColumnIndex + 1u);
            iColumns[aColumnIndex].name = aName;
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
            return base_iterator{ std::next(iItems.begin(), aIndex.row()) };
        }
        i_item_model::const_iterator index_to_iterator(const item_model_index& aIndex) const override
        {
            return const_base_iterator{ std::next(iItems.begin(), aIndex.row()) };
        }
        item_model_index iterator_to_index(i_item_model::const_iterator aPosition) const override
        {
            return item_model_index{ static_cast<uint32_t>(std::distance(iItems.begin(), const_base_iterator{ aPosition }.get<const_iterator, const_iterator, iterator, const_sibling_iterator, sibling_iterator>())), 0 };
        }
        i_item_model::iterator begin() override
        {
            return base_iterator{ iItems.begin() };
        }
        i_item_model::const_iterator begin() const override
        {
            return const_base_iterator{ iItems.begin() };
        }
        i_item_model::iterator end() override
        {
            return base_iterator{ iItems.end() };
        }
        i_item_model::const_iterator end() const override
        {
            return const_base_iterator{ iItems.end() };
        }
        i_item_model::iterator sbegin() override
        {
            if constexpr (container_traits::is_tree)
                return base_iterator{ iItems.sbegin() };
            else
                throw base_type::wrong_model_type();
        }
        i_item_model::const_iterator sbegin() const override
        {
            if constexpr (container_traits::is_tree)
                return const_base_iterator{ iItems.sbegin() };
            else
                throw base_type::wrong_model_type();
        }
        i_item_model::iterator send() override
        {
            if constexpr (container_traits::is_tree)
                return base_iterator{ iItems.send() };
            else
                throw base_type::wrong_model_type();
        }
        i_item_model::const_iterator send() const override
        {
            if constexpr (container_traits::is_tree)
                return const_base_iterator{ iItems.send() };
            else
                throw base_type::wrong_model_type();
        }
        bool has_children(i_item_model::const_iterator aParent) const override
        {
            if constexpr (container_traits::is_tree)
                return aParent.get<const_iterator, const_iterator, iterator, const_sibling_iterator, sibling_iterator>().descendent_count() != 0;
            else
                throw base_type::wrong_model_type();
        }
        bool has_children(const item_model_index& aParentIndex) const override
        {
            if constexpr (container_traits::is_tree)
                return has_children(index_to_iterator(aParentIndex));
            else
                throw base_type::wrong_model_type();
        }
        bool has_parent(i_item_model::const_iterator aChild) const override
        {
            if constexpr (container_traits::is_tree)
                return !aChild.get<const_iterator, const_iterator, iterator, const_sibling_iterator, sibling_iterator>().parent_is_root();
            else
                throw base_type::wrong_model_type();
        }
        bool has_parent(const item_model_index& aChildIndex) const override
        {
            if constexpr (container_traits::is_tree)
                return has_parent(index_to_iterator(aChildIndex));
            else
                throw base_type::wrong_model_type();
        }
        i_item_model::iterator parent(i_item_model::iterator aChild) override
        {
            if constexpr (container_traits::is_tree)
                return base_iterator{ aChild.get<iterator, iterator, sibling_iterator>().parent() };
            else
                throw base_type::wrong_model_type();
        }
        i_item_model::const_iterator parent(i_item_model::const_iterator aChild) const override
        {
            if constexpr (container_traits::is_tree)
                return const_base_iterator{ aChild.get<const_iterator, const_iterator, iterator, const_sibling_iterator, sibling_iterator>().parent() };
            else
                throw base_type::wrong_model_type();
        }
        item_model_index parent(const item_model_index& aChildIndex) const override
        {
            if constexpr (container_traits::is_tree)
                return iterator_to_index(parent(index_to_iterator(aChildIndex)));
            else
                throw base_type::wrong_model_type();
        }
        i_item_model::iterator sbegin(i_item_model::iterator aParent) override
        {
            if constexpr (container_traits::is_tree)
                return base_iterator{ aParent.get<iterator, iterator, sibling_iterator>().begin() };
            else
                throw base_type::wrong_model_type();
        }
        i_item_model::const_iterator sbegin(i_item_model::const_iterator aParent) const override
        {
            if constexpr (container_traits::is_tree)
                return const_base_iterator{ aParent.get<const_iterator, const_iterator, iterator, const_sibling_iterator, sibling_iterator>().begin() };
            else
                throw base_type::wrong_model_type();
        }
        i_item_model::iterator send(i_item_model::iterator aParent) override
        {
            if constexpr (container_traits::is_tree)
                return base_iterator{ aParent.get<iterator, iterator, sibling_iterator>().end() };
            else
                throw base_type::wrong_model_type();
        }
        i_item_model::const_iterator send(i_item_model::const_iterator aParent) const override
        {
            if constexpr (container_traits::is_tree)
                return const_base_iterator{ aParent.get<const_iterator, const_iterator, iterator, const_sibling_iterator, sibling_iterator>().end() };
            else
                throw base_type::wrong_model_type();
        }
    public:
        const item_cell_data& cell_data(const item_model_index& aIndex) const override
        {
            if (aIndex.column() < row(aIndex).cells.size())
                return row(aIndex).cells[aIndex.column()];
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
            if constexpr (container_traits::is_flat)
                iItems.reserve(aItemCount);
            else
                throw base_type::wrong_model_type();
        }
        uint32_t capacity() const override
        {
            if constexpr (container_traits::is_flat)
                return static_cast<uint32_t>(iItems.capacity());
            else
                throw base_type::wrong_model_type();
        }
        i_item_model::iterator insert_item(i_item_model::const_iterator aPosition, const value_type& aValue) override
        {
            auto result = base_iterator{ iItems.insert(aPosition.get<const_sibling_iterator, const_iterator, iterator, const_sibling_iterator, sibling_iterator>(), row_type{ aValue, row_cell_array{} }) };
            ItemAdded.trigger(iterator_to_index(result));
            return result;
        }
        i_item_model::iterator insert_item(i_item_model::const_iterator aPosition, const value_type& aValue, const item_cell_data& aCellData) override
        {
            auto result = base_iterator{ iItems.insert(aPosition.get<const_sibling_iterator, const_iterator, iterator, const_sibling_iterator, sibling_iterator>(), row_type{ aValue, row_cell_array{} }) };
            do_insert_cell_data(result, 0, aCellData);
            ItemAdded.trigger(iterator_to_index(result));
            ItemChanged.trigger(iterator_to_index(result));
            return result;
        }
        i_item_model::iterator insert_item(i_item_model::const_iterator aPosition, const item_cell_data& aCellData) override
        {
            return insert_item(aPosition, value_type{}, aCellData);
        }
        i_item_model::iterator insert_item(const item_model_index& aIndex, const value_type& aValue) override
        {
            return insert_item(index_to_iterator(aIndex), aValue);
        }
        i_item_model::iterator insert_item(const item_model_index& aIndex, const value_type& aValue, const item_cell_data& aCellData) override
        {
            return insert_item(index_to_iterator(aIndex), aValue, aCellData);
        }
        i_item_model::iterator insert_item(const item_model_index& aIndex, const item_cell_data& aCellData) override
        {
            return insert_item(index_to_iterator(aIndex), aCellData);
        }
        i_item_model::iterator append_item(i_item_model::const_iterator aParent, const value_type& aValue) override
        {
            if constexpr (container_traits::is_tree)
            {
                auto result = base_iterator{ iItems.insert(aParent.get<const_sibling_iterator, const_iterator, iterator, const_sibling_iterator, sibling_iterator>().end(), row_type{ aValue, row_cell_array{} }) };
                ItemAdded.trigger(iterator_to_index(result));
                return result;
            }
            else
                throw base_type::wrong_model_type();
        }
        i_item_model::iterator append_item(i_item_model::const_iterator aParent, const value_type& aValue, const item_cell_data& aCellData) override
        {
            auto result = base_iterator{ append_item(aParent, aValue) };
            insert_cell_data(result, 0, aCellData);
            return result;
        }
        i_item_model::iterator append_item(i_item_model::const_iterator aParent, const item_cell_data& aCellData) override
        {
            return append_item(aParent, value_type(), aCellData);
        }
        i_item_model::iterator append_item(const item_model_index& aIndex, const value_type& aValue) override
        {
            return append_item(index_to_iterator(aIndex), aValue);
        }
        i_item_model::iterator append_item(const item_model_index& aIndex, const value_type& aValue, const item_cell_data& aCellData) override
        {
            return append_item(index_to_iterator(aIndex), aValue, aCellData);
        }
        i_item_model::iterator append_item(const item_model_index& aIndex, const item_cell_data& aCellData) override
        {
            return append_item(index_to_iterator(aIndex), aCellData);
        }
        void clear() override
        {
            iItems.clear();
            Cleared.trigger();
        }
        i_item_model::iterator erase(i_item_model::const_iterator aPosition) override
        {
            ItemRemoved.trigger(iterator_to_index(aPosition));
            auto result = base_iterator{ iItems.erase(aPosition.get<const_iterator, const_iterator, iterator, const_sibling_iterator, sibling_iterator>()) };
            return result;
        }
        void insert_cell_data(i_item_model::iterator aItem, item_model_index::value_type aColumnIndex, const item_cell_data& aCellData) override
        {
            if (do_insert_cell_data(aItem, aColumnIndex, aCellData))
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
            if (std::holds_alternative<std::string>(aCellData) && std::get<std::string>(aCellData).empty())
            {
                update_cell_data(aIndex, {});
                return;
            }
            if (row(aIndex).cells[aIndex.column()] == aCellData)
                return;
            row(aIndex).cells[aIndex.column()] = aCellData;
            if (default_cell_info(aIndex.column()).dataType == item_data_type::Unknown)
                default_cell_info(aIndex.column()).dataType = static_cast<item_data_type>(aCellData.index());
            ItemChanged.trigger(aIndex);
        }
    public:
        using base_type::item;
        value_type& item(const item_model_index& aIndex) override
        {
            return row(aIndex).value;
        }
        const value_type& item(const item_model_index& aIndex) const override
        {
            return row(aIndex).value;
        }
    private:
        row_type& row(const item_model_index& aIndex)
        {
            return *std::next(iItems.begin(), aIndex.row());
        }
        const row_type& row(const item_model_index& aIndex) const
        {
            return *std::next(iItems.begin(), aIndex.row());
        }
        const item_cell_info& default_cell_info(item_model_index::column_type aColumnIndex) const
        {
            if (iColumns.size() < aColumnIndex + 1)
                throw base_type::bad_column_index();
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
        bool do_insert_cell_data(i_item_model::iterator aItem, item_model_index::value_type aColumnIndex, const item_cell_data& aCellData)
        {
            if (std::holds_alternative<std::string>(aCellData) && std::get<std::string>(aCellData).empty())
                return do_insert_cell_data(aItem, aColumnIndex, {});
            bool changed = false;
            auto ri = aItem.get<iterator, iterator, sibling_iterator>();
            if (ri->cells.size() < aColumnIndex + 1)
            {
                ri->cells.resize(aColumnIndex + 1);
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
            if (ri->cells[aColumnIndex] != aCellData)
            {
                ri->cells[aColumnIndex] = aCellData;
                changed = true;
            }
            return changed;
        }
    private:
        container_type iItems;
        column_info_array iColumns;
    };

    typedef basic_item_model<void*> item_model;
    typedef basic_item_model<void*, 0, item_cell_data, item_tree_container_traits<void*, item_cell_data, 0>> item_tree_model;

    template <typename T, uint32_t Columns = 0>
    using basic_item_tree_model = basic_item_model<T, Columns, item_cell_data, item_tree_container_traits<T, item_cell_data, Columns>>;
}