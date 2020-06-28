// i_item_presentation_model.hpp
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
#include <boost/format.hpp>

#include <neogfx/core/color.hpp>
#include <neogfx/app/i_palette.hpp>
#include <neogfx/gfx/text/font.hpp>
#include <neogfx/gfx/text/glyph.hpp>
#include <neogfx/gui/widget/i_button.hpp>
#include <neogfx/gui/widget/item_index.hpp>
#include <neogfx/gui/widget/i_item_model.hpp>

namespace neogfx
{
    enum class item_cell_flags : uint32_t
    {
        Invalid                 = 0x00000000,
        Enabled                 = 0x00000001,
        Selectable              = 0x00000002,
        Editable                = 0x00000004,
        EditableWhenFocused     = 0x00000008,
        EditableOnInputEvent    = 0x00000010,
        Checkable               = 0x00000020,
        CheckableBiState        = 0x00000040,
        CheckableTriState       = 0x00000080,
        Draggable               = 0x00001000,
        Droppable               = 0x00002000,

        Default = Enabled | Selectable | Editable | EditableOnInputEvent
    };

    typedef std::optional<item_cell_flags> optional_item_cell_flags;

    enum class item_cell_selection_flags : uint8_t
    {
        None = 0x00,
        Current = 0x01,
        Selected = 0x02
    };
}

template <>
const neolib::enum_enumerators_t<neogfx::item_cell_flags> neolib::enum_enumerators_v<neogfx::item_cell_flags>
{
    declare_enum_string(neogfx::item_cell_flags, Enabled)
    declare_enum_string(neogfx::item_cell_flags, Selectable)
    declare_enum_string(neogfx::item_cell_flags, Editable)
    declare_enum_string(neogfx::item_cell_flags, EditableWhenFocused)
    declare_enum_string(neogfx::item_cell_flags, EditableOnInputEvent)
    declare_enum_string(neogfx::item_cell_flags, Checkable)
    declare_enum_string(neogfx::item_cell_flags, CheckableBiState)
    declare_enum_string(neogfx::item_cell_flags, CheckableTriState)
    declare_enum_string(neogfx::item_cell_flags, Draggable)
    declare_enum_string(neogfx::item_cell_flags, Droppable)
};

namespace neogfx
{
    inline item_cell_flags operator|(item_cell_flags lhs, item_cell_flags rhs)
    {
        return static_cast<item_cell_flags>(static_cast<uint32_t>(lhs) | static_cast<uint32_t>(rhs));
    }

    inline item_cell_flags operator&(item_cell_flags lhs, item_cell_flags rhs)
    {
        return static_cast<item_cell_flags>(static_cast<uint32_t>(lhs)& static_cast<uint32_t>(rhs));
    }

    inline item_cell_flags& operator|=(item_cell_flags& lhs, item_cell_flags rhs)
    {
        lhs = lhs | rhs;
        return lhs;
    }

    inline item_cell_flags& operator&=(item_cell_flags& lhs, item_cell_flags rhs)
    {
        lhs = lhs & rhs;
        return lhs;
    }

    inline item_cell_flags operator~(item_cell_flags flags)
    {
        return static_cast<item_cell_flags>(~static_cast<uint32_t>(flags));
    }

    inline item_cell_selection_flags operator|(item_cell_selection_flags aLhs, item_cell_selection_flags aRhs)
    {
        return static_cast<item_cell_selection_flags>(static_cast<uint32_t>(aLhs) | static_cast<uint32_t>(aRhs));
    }

    inline item_cell_selection_flags operator&(item_cell_selection_flags aLhs, item_cell_selection_flags aRhs)
    {
        return static_cast<item_cell_selection_flags>(static_cast<uint32_t>(aLhs) & static_cast<uint32_t>(aRhs));
    }

    inline item_cell_selection_flags operator^(item_cell_selection_flags aLhs, item_cell_selection_flags aRhs)
    {
        return static_cast<item_cell_selection_flags>(static_cast<uint32_t>(aLhs) ^ static_cast<uint32_t>(aRhs));
    }

    inline item_cell_selection_flags& operator|=(item_cell_selection_flags& lhs, item_cell_selection_flags rhs)
    {
        lhs = lhs | rhs;
        return lhs;
    }

    inline item_cell_selection_flags& operator&=(item_cell_selection_flags& lhs, item_cell_selection_flags rhs)
    {
        lhs = lhs & rhs;
        return lhs;
    }

    inline item_cell_selection_flags& operator^=(item_cell_selection_flags& lhs, item_cell_selection_flags rhs)
    {
        lhs = lhs ^ rhs;
        return lhs;
    }

    inline item_cell_selection_flags operator~(item_cell_selection_flags aLhs)
    {
        return static_cast<item_cell_selection_flags>(~static_cast<uint32_t>(aLhs));
    }

    struct item_presentation_model_index : item_index<item_presentation_model_index> 
    { 
        typedef item_presentation_model_index abstract_type; // todo: create abstract interface
        using item_index::item_index; 
    };
    typedef std::optional<item_presentation_model_index> optional_item_presentation_model_index;

    class i_item_presentation_model
    {
    public:
        declare_event(visual_appearance_changed)
        declare_event(column_info_changed, item_presentation_model_index::column_type)
        declare_event(item_model_changed, const i_item_model&)
        declare_event(item_added, const item_presentation_model_index&)
        declare_event(item_changed, const item_presentation_model_index&)
        declare_event(item_removed, const item_presentation_model_index&)
        declare_event(item_expanding, const item_presentation_model_index&)
        declare_event(item_collapsing, const item_presentation_model_index&)
        declare_event(item_expanded, const item_presentation_model_index&)
        declare_event(item_collapsed, const item_presentation_model_index&)
        declare_event(item_toggled, const item_presentation_model_index&)
        declare_event(item_checked, const item_presentation_model_index&)
        declare_event(item_unchecked, const item_presentation_model_index&)
        declare_event(item_indeterminate, const item_presentation_model_index&)
        declare_event(items_sorting)
        declare_event(items_sorted)
        declare_event(items_filtering)
        declare_event(items_filtered)
    public:
        struct cell_meta_type
        {
            optional_item_cell_flags flags;
            item_cell_selection_flags selection = item_cell_selection_flags::None;
            button_checked_state checked = false;
            bool expanded = false;
            optional_glyph_text text;
            optional_size extents;
        };
        class i_meta_visitor
        {
        public:
            virtual void visit(cell_meta_type& aMeta) = 0;
        };
        enum class sort_direction
        {
            Ascending,
            Descending
        };
        typedef std::optional<sort_direction> optional_sort_direction;
        typedef std::pair<item_presentation_model_index::column_type, sort_direction> sort;
        typedef std::optional<sort> optional_sort;
        typedef std::string filter_search_key;
        enum class filter_search_type
        {
            Prefix,
            Glob,
            Regex
        };
        enum class case_sensitivity
        {
            CaseInsensitive,
            CaseSensitive
        };
        typedef std::tuple<item_presentation_model_index::column_type, filter_search_key, filter_search_type, case_sensitivity> filter;
        typedef std::optional<filter> optional_filter;
    public:
        struct no_item_model : std::logic_error { no_item_model() : std::logic_error("neogfx::i_item_presentation_model::no_item_model") {} };
        struct bad_index : std::logic_error { bad_index() : std::logic_error("neogfx::i_item_presentation_model::bad_index") {} };
        struct no_mapped_row : std::logic_error { no_mapped_row() : std::logic_error("neogfx::i_item_presentation_model::no_mapped_row") {} };
    public:
        virtual ~i_item_presentation_model() = default;
    public:
        virtual bool initializing() const = 0;
        virtual bool has_item_model() const = 0;
        virtual i_item_model& item_model() const = 0;
        virtual void set_item_model(i_item_model& aItemModel, bool aSortable = false) = 0;
        virtual item_model_index to_item_model_index(const item_presentation_model_index& aIndex) const = 0;
        virtual bool has_item_model_index(const item_model_index& aIndex) const = 0;
        virtual item_presentation_model_index from_item_model_index(const item_model_index& aIndex, bool aIgnoreColumn = false) const = 0;
    public:
        virtual uint32_t rows() const = 0;
        virtual uint32_t columns() const = 0;
        virtual uint32_t columns(const item_presentation_model_index& aIndex) const = 0;
    public:
        virtual void accept(i_meta_visitor& aVisitor, bool aIgnoreCollapsedState = false) = 0;
    public:
        virtual dimension column_width(item_presentation_model_index::column_type aColumnIndex, const i_graphics_context& aGc, bool aIncludePadding = true) const = 0;
        virtual const std::string& column_heading_text(item_presentation_model_index::column_type aColumnIndex) const = 0;
        virtual size column_heading_extents(item_presentation_model_index::column_type aColumnIndex, const i_graphics_context& aGc) const = 0;
        virtual void set_column_heading_text(item_presentation_model_index::column_type aColumnIndex, const std::string& aHeadingText) = 0;
        virtual item_cell_flags column_flags(item_presentation_model_index::column_type aColumnIndex) const = 0;
        virtual void set_column_flags(item_presentation_model_index::column_type aColumnIndex, item_cell_flags aFlags) = 0;
        virtual optional_size column_image_size(item_presentation_model_index::column_type aColumnIndex) const = 0;
        virtual void set_column_image_size(item_presentation_model_index::column_type aColumnIndex, const optional_size& aImageSize) = 0;
    public:
        virtual void toggle_expanded(const item_presentation_model_index& aIndex) = 0;
    public:
        virtual const button_checked_state& checked_state(const item_presentation_model_index& aIndex) = 0;
        virtual bool is_checked(const item_presentation_model_index& aIndex) const = 0;
        virtual bool is_unchecked(const item_presentation_model_index& aIndex) const = 0;
        virtual bool is_indeterminate(const item_presentation_model_index& aIndex) const = 0;
        virtual void set_checked_state(const item_presentation_model_index& aIndex, const button_checked_state& aState) = 0;
        virtual void check(const item_presentation_model_index& aIndex) = 0;
        virtual void uncheck(const item_presentation_model_index& aIndex) = 0;
        virtual void set_indeterminate(const item_presentation_model_index& aIndex) = 0;
        virtual void set_checked(const item_presentation_model_index& aIndex, bool aChecked) = 0;
        virtual void toggle_check(const item_presentation_model_index& aIndex) = 0;
    public:
        virtual const font& default_font() const = 0;
        virtual void set_default_font(const optional_font& aDefaultFont) = 0;
        virtual size cell_spacing(const i_units_context& aUnitsContext) const = 0;
        virtual void set_cell_spacing(const optional_size& aSpacing, const i_units_context& aUnitsContext) = 0;
        virtual neogfx::padding cell_padding(const i_units_context& aUnitsContext) const = 0;
        virtual void set_cell_padding(const optional_padding& aPadding, const i_units_context& aUnitsContext) = 0;
    public:
        virtual dimension item_height(const item_presentation_model_index& aIndex, const i_units_context& aUnitsContext) const = 0;
        virtual double total_height(const i_units_context& aUnitsContext) const = 0;
        virtual double item_position(const item_presentation_model_index& aIndex, const i_units_context& aUnitsContext) const = 0;
        virtual std::pair<item_presentation_model_index::value_type, coordinate> item_at(double aPosition, const i_units_context& aUnitsContext) const = 0;
    public:
        virtual item_cell_flags cell_flags(const item_presentation_model_index& aIndex) const = 0;
        virtual void set_cell_flags(const item_presentation_model_index& aIndex, item_cell_flags aFlags) = 0;
        virtual cell_meta_type& cell_meta(const item_presentation_model_index& aIndex) const = 0;
    public:
        virtual std::string cell_to_string(const item_presentation_model_index& aIndex) const = 0;
        virtual item_cell_data string_to_cell_data(const item_presentation_model_index& aIndex, const std::string& aString) const = 0;
        virtual item_cell_data string_to_cell_data(const item_presentation_model_index& aIndex, const std::string& aString, bool& aError) const = 0;
        virtual boost::basic_format<char> cell_format(const item_presentation_model_index& aIndex) const = 0;
        virtual optional_color cell_color(const item_presentation_model_index& aIndex, color_role aColorRole) const = 0;
        virtual optional_font cell_font(const item_presentation_model_index& aIndex) const = 0;
        virtual optional_size cell_image_size(const item_presentation_model_index& aIndex) const = 0;
        virtual optional_size cell_check_box_size(const item_presentation_model_index& aIndex, const i_graphics_context& aGc) const = 0;
        virtual optional_size cell_tree_expander_size(const item_presentation_model_index& aIndex, const i_graphics_context& aGc) const = 0;
        virtual optional_texture cell_image(const item_presentation_model_index& aIndex) const = 0;
        virtual neogfx::glyph_text& cell_glyph_text(const item_presentation_model_index& aIndex, const i_graphics_context& aGc) const = 0;
        virtual size cell_extents(const item_presentation_model_index& aIndex, const i_graphics_context& aGc) const = 0;
        virtual dimension indent(const item_presentation_model_index& aIndex, const i_graphics_context& aGc) const = 0;
    public:
        virtual bool sortable() const = 0;
        virtual optional_sort sorting_by() const = 0;
        virtual void sort_by(item_presentation_model_index::column_type aColumnIndex, const optional_sort_direction& aSortDirection = optional_sort_direction{}) = 0;
        virtual void reset_sort() = 0;
    public:
        virtual optional_item_presentation_model_index find_item(const filter_search_key& aFilterSearchKey, item_presentation_model_index::column_type aColumnIndex = 0, filter_search_type aFilterSearchType = filter_search_type::Prefix, case_sensitivity aCaseSensitivity = case_sensitivity::CaseInsensitive) const = 0;
    public:
        virtual bool filtering() const = 0;
        virtual optional_filter filtering_by() const = 0;
        virtual void filter_by(item_presentation_model_index::column_type aColumnIndex, const filter_search_key& aFilterSearchKey, filter_search_type aFilterSearchType = filter_search_type::Prefix, case_sensitivity aCaseSensitivity = case_sensitivity::CaseInsensitive) = 0;
        virtual void reset_filter() = 0;
        // helpers
    public:
        bool column_editable(item_presentation_model_index::column_type aColumnIndex) const
        {
            return (column_flags(aColumnIndex) & item_cell_flags::Editable) == item_cell_flags::Editable;
        }
        void set_column_editable(item_presentation_model_index::column_type aColumnIndex, bool aEditable = true)
        {
            if (aEditable)
            {
                set_column_flags(aColumnIndex, column_flags(aColumnIndex) | item_cell_flags::Editable);
                if ((column_flags(aColumnIndex) & (item_cell_flags::EditableWhenFocused | item_cell_flags::EditableOnInputEvent)) == item_cell_flags::Invalid)
                    set_column_flags(aColumnIndex, column_flags(aColumnIndex) | item_cell_flags::EditableWhenFocused);
            }
            else
                set_column_flags(aColumnIndex, column_flags(aColumnIndex) & ~item_cell_flags::Editable);
        }
        bool column_editable_when_focused(item_presentation_model_index::column_type aColumnIndex) const
        {
            return column_editable(aColumnIndex) && (column_flags(aColumnIndex) & item_cell_flags::EditableWhenFocused) == item_cell_flags::EditableWhenFocused;
        }
        void set_column_editable_when_focused(item_presentation_model_index::column_type aColumnIndex, bool aEditableWhenFocused = true)
        {
            if (aEditableWhenFocused)
                set_column_flags(aColumnIndex, (column_flags(aColumnIndex) | item_cell_flags::Editable | item_cell_flags::EditableWhenFocused) & ~item_cell_flags::EditableOnInputEvent);
            else
                set_column_flags(aColumnIndex, column_flags(aColumnIndex) & ~(item_cell_flags::Editable | item_cell_flags::EditableWhenFocused));
        }
        bool column_editable_on_input_event(item_presentation_model_index::column_type aColumnIndex) const
        {
            return column_editable(aColumnIndex) && (column_flags(aColumnIndex) & item_cell_flags::EditableOnInputEvent) == item_cell_flags::EditableOnInputEvent;
        }
        void set_column_editable_on_input_event(item_presentation_model_index::column_type aColumnIndex, bool aEditableOnInputEvent = true)
        {
            if (aEditableOnInputEvent)
                set_column_flags(aColumnIndex, (column_flags(aColumnIndex) | item_cell_flags::Editable | item_cell_flags::EditableOnInputEvent) & ~item_cell_flags::EditableWhenFocused);
            else
                set_column_flags(aColumnIndex, column_flags(aColumnIndex) & ~(item_cell_flags::Editable | item_cell_flags::EditableOnInputEvent));
        }
        bool column_selectable(item_model_index::column_type aColumnIndex) const
        {
            return (column_flags(aColumnIndex) & item_cell_flags::Selectable) == item_cell_flags::Selectable;
        }
        void set_column_selectable(item_model_index::column_type aColumnIndex, bool aSelectable = true)
        {
            if (aSelectable)
                set_column_flags(aColumnIndex, column_flags(aColumnIndex) | item_cell_flags::Selectable);
            else
                set_column_flags(aColumnIndex, column_flags(aColumnIndex) & ~item_cell_flags::Selectable);
        }
        bool column_read_only(item_model_index::column_type aColumnIndex) const
        {
            return (column_flags(aColumnIndex) & item_cell_flags::Editable) != item_cell_flags::Editable;
        }
        void set_column_read_only(item_model_index::column_type aColumnIndex, bool aReadOnly = true)
        {
            if (aReadOnly)
                set_column_flags(aColumnIndex, column_flags(aColumnIndex) & ~item_cell_flags::Editable);
            else
                set_column_flags(aColumnIndex, column_flags(aColumnIndex) | item_cell_flags::Editable);
        }
        bool column_checkable(item_model_index::column_type aColumnIndex) const
        {
            return (column_flags(aColumnIndex) & item_cell_flags::Checkable) == item_cell_flags::Checkable;
        }
        void set_column_checkable(item_model_index::column_type aColumnIndex, bool aCheckable = true)
        {
            if (aCheckable)
            {
                set_column_flags(aColumnIndex, column_flags(aColumnIndex) | item_cell_flags::Checkable);
                if ((column_flags(aColumnIndex) & (item_cell_flags::CheckableBiState | item_cell_flags::CheckableTriState)) == item_cell_flags::Invalid)
                    set_column_flags(aColumnIndex, column_flags(aColumnIndex) | item_cell_flags::CheckableBiState);
            }
            else
                set_column_flags(aColumnIndex, column_flags(aColumnIndex) & ~item_cell_flags::Checkable);
        }
        bool column_bi_state_checkable(item_model_index::column_type aColumnIndex) const
        {
            return column_checkable(aColumnIndex) && (column_flags(aColumnIndex) & item_cell_flags::CheckableBiState) == item_cell_flags::CheckableBiState;
        }
        void set_column_bi_state_checkable(item_model_index::column_type aColumnIndex, bool aCheckableBiState = true)
        {
            if (aCheckableBiState)
                set_column_flags(aColumnIndex, (column_flags(aColumnIndex) | item_cell_flags::Checkable | item_cell_flags::CheckableBiState) & ~item_cell_flags::CheckableTriState);
            else
                set_column_flags(aColumnIndex, column_flags(aColumnIndex) & ~(item_cell_flags::Checkable | item_cell_flags::CheckableBiState));
        }
        bool column_tri_state_checkable(item_model_index::column_type aColumnIndex) const
        {
            return column_checkable(aColumnIndex) && (column_flags(aColumnIndex) & item_cell_flags::CheckableTriState) == item_cell_flags::CheckableTriState;
        }
        void set_column_tri_state_checkable(item_model_index::column_type aColumnIndex, bool aCheckableTriState = true)
        {
            if (aCheckableTriState)
                set_column_flags(aColumnIndex, (column_flags(aColumnIndex) | item_cell_flags::Checkable | item_cell_flags::CheckableTriState) & ~item_cell_flags::CheckableBiState);
            else
                set_column_flags(aColumnIndex, column_flags(aColumnIndex) & ~(item_cell_flags::Checkable | item_cell_flags::CheckableTriState));
        }
        bool cell_editable(const item_presentation_model_index& aIndex) const
        {
            return (cell_flags(aIndex) & item_cell_flags::Editable) == item_cell_flags::Editable;
        }
        void set_cell_editable(const item_presentation_model_index& aIndex, bool aEditable = true)
        {
            if (aEditable)
            {
                set_cell_flags(aIndex, cell_flags(aIndex) | item_cell_flags::Editable);
                if ((cell_flags(aIndex) & (item_cell_flags::EditableWhenFocused | item_cell_flags::EditableOnInputEvent)) == item_cell_flags::Invalid)
                    set_cell_flags(aIndex, cell_flags(aIndex) | item_cell_flags::EditableWhenFocused);
            }
            else
                set_cell_flags(aIndex, cell_flags(aIndex) & ~item_cell_flags::Editable);
        }
        bool cell_editable_when_focused(const item_presentation_model_index& aIndex) const
        {
            return cell_editable(aIndex) && (cell_flags(aIndex) & item_cell_flags::EditableWhenFocused) == item_cell_flags::EditableWhenFocused;
        }
        void set_cell_editable_when_focused(const item_presentation_model_index& aIndex, bool aEditableWhenFocused = true)
        {
            if (aEditableWhenFocused)
                set_cell_flags(aIndex, (cell_flags(aIndex) | item_cell_flags::Editable | item_cell_flags::EditableWhenFocused) & ~item_cell_flags::EditableOnInputEvent);
            else
                set_cell_flags(aIndex, cell_flags(aIndex) & ~(item_cell_flags::Editable | item_cell_flags::EditableWhenFocused));
        }
        bool cell_editable_on_input_event(const item_presentation_model_index& aIndex) const
        {
            return cell_editable(aIndex) && (cell_flags(aIndex) & item_cell_flags::EditableOnInputEvent) == item_cell_flags::EditableOnInputEvent;
        }
        void set_cell_editable_on_input_event(const item_presentation_model_index& aIndex, bool aEditableOnInputEvent = true)
        {
            if (aEditableOnInputEvent)
                set_cell_flags(aIndex, (cell_flags(aIndex) | item_cell_flags::Editable | item_cell_flags::EditableOnInputEvent) & ~item_cell_flags::EditableWhenFocused);
            else
                set_cell_flags(aIndex, cell_flags(aIndex) & ~(item_cell_flags::Editable | item_cell_flags::EditableOnInputEvent));
        }
        bool cell_selectable(const item_presentation_model_index& aIndex) const
        {
            return (cell_flags(aIndex) & item_cell_flags::Selectable) == item_cell_flags::Selectable;
        }
        void set_cell_selectable(const item_presentation_model_index& aIndex, bool aSelectable = true)
        {
            if (aSelectable)
                set_cell_flags(aIndex, cell_flags(aIndex) | item_cell_flags::Selectable);
            else
                set_cell_flags(aIndex, cell_flags(aIndex) & ~item_cell_flags::Selectable);
        }
        bool cell_read_only(const item_presentation_model_index& aIndex) const
        {
            return (cell_flags(aIndex) & item_cell_flags::Editable) != item_cell_flags::Editable;
        }
        void set_cell_read_only(const item_presentation_model_index& aIndex, bool aReadOnly = true)
        {
            if (aReadOnly)
                set_cell_flags(aIndex, cell_flags(aIndex) & ~item_cell_flags::Editable);
            else
                set_cell_flags(aIndex, cell_flags(aIndex) | item_cell_flags::Editable);
        }
        bool cell_checkable(const item_presentation_model_index& aIndex) const
        {
            return (cell_flags(aIndex) & item_cell_flags::Checkable) == item_cell_flags::Checkable;
        }
        void set_cell_checkable(const item_presentation_model_index& aIndex, bool aCheckable = true)
        {
            if (aCheckable)
            {
                set_cell_flags(aIndex, cell_flags(aIndex) | item_cell_flags::Checkable);
                if ((cell_flags(aIndex) & (item_cell_flags::CheckableBiState | item_cell_flags::CheckableTriState)) == item_cell_flags::Invalid)
                    set_cell_flags(aIndex, cell_flags(aIndex) | item_cell_flags::CheckableBiState);
            }
            else
                set_cell_flags(aIndex, cell_flags(aIndex) & ~item_cell_flags::Checkable);
        }
        bool cell_bi_state_checkable(const item_presentation_model_index& aIndex) const
        {
            return cell_checkable(aIndex) && (cell_flags(aIndex) & item_cell_flags::CheckableBiState) == item_cell_flags::CheckableBiState;
        }
        void set_cell_bi_state_checkable(const item_presentation_model_index& aIndex, bool aCheckableBiState = true)
        {
            if (aCheckableBiState)
                set_cell_flags(aIndex, (cell_flags(aIndex) | item_cell_flags::Checkable | item_cell_flags::CheckableBiState) & ~item_cell_flags::CheckableTriState);
            else
                set_cell_flags(aIndex, cell_flags(aIndex) & ~(item_cell_flags::Checkable | item_cell_flags::CheckableBiState));
        }
        bool cell_tri_state_checkable(const item_presentation_model_index& aIndex) const
        {
            return cell_checkable(aIndex) && (cell_flags(aIndex) & item_cell_flags::CheckableTriState) == item_cell_flags::CheckableTriState;
        }
        void set_cell_tri_state_checkable(const item_presentation_model_index& aIndex, bool aCheckableTriState = true)
        {
            if (aCheckableTriState)
                set_cell_flags(aIndex, (cell_flags(aIndex) | item_cell_flags::Checkable | item_cell_flags::CheckableTriState) & ~item_cell_flags::CheckableBiState);
            else
                set_cell_flags(aIndex, cell_flags(aIndex) & ~(item_cell_flags::Checkable | item_cell_flags::CheckableTriState));
        }
    };
}