// header_view.hpp
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
#include <neogfx/gui/window/context_menu.hpp>
#include "splitter.hpp"
#include "i_item_model.hpp"
#include "i_item_presentation_model.hpp"

namespace neogfx
{
    enum class header_view_update_reason
    {
        FullUpdate,
        PanesResized
    };

    class header_view;

    class i_header_view_owner
    {
    public:
        virtual void header_view_updated(header_view& aHeaderView, header_view_update_reason aUpdateReason) = 0;
    };

    enum class header_view_type : uint32_t
    {
        Horizontal      = 0x0001,
        Vertical        = 0x0002
    };

    class header_view : public splitter
    {
    private:
        class updater;
    private:
        struct section_dimension
        {
            optional_dimension manual;
            dimension calculated;
            dimension max;
        };
    public:
        header_view(i_header_view_owner& aOwner, header_view_type aType = header_view_type::Horizontal);
        header_view(i_widget& aParent, i_header_view_owner& aOwner, header_view_type aType = header_view_type::Horizontal);
        header_view(i_layout& aLayout, i_header_view_owner& aOwner, header_view_type aType = header_view_type::Horizontal);
        ~header_view();
    public:
        header_view_type type() const;
        bool has_model() const;
        const i_item_model& model() const;
        i_item_model& model();
        void set_model(i_item_model& aModel);
        void set_model(std::shared_ptr<i_item_model> aModel);
        bool has_presentation_model() const;
        const i_item_presentation_model& presentation_model() const;
        i_item_presentation_model& presentation_model();
        void set_presentation_model(i_item_presentation_model& aPresentationModel);
        void set_presentation_model(std::shared_ptr<i_item_presentation_model> aPresentationModel);
        bool expand_last_column() const;
        void set_expand_last_column(bool aExpandLastColumn);
    public:
        dimension separator_width() const;
        void set_separator_width(const optional_dimension& aWidth);
        uint32_t section_count() const;
        dimension section_width(uint32_t aSectionIndex, bool aForHeaderButton = false) const;
        dimension total_width() const;
    public:
        bool is_managing_layout() const override;
    public:
        void panes_resized() override;
        void reset_pane_sizes_requested(const std::optional<uint32_t>& aPane = std::optional<uint32_t>()) override;
    protected:
        virtual void column_info_changed(item_presentation_model_index::column_type aColumnIndex);
        virtual void item_model_changed(const i_item_model& aItemModel);
        virtual void item_added(const item_presentation_model_index& aItemIndex);
        virtual void item_changed(const item_presentation_model_index& aItemIndex);
        virtual void item_removed(const item_presentation_model_index& aItemIndex);
        virtual void items_sorting();
        virtual void items_sorted();
        virtual void items_filtering();
        virtual void items_filtered();
    private:
        void init();
        void update_buttons();
        void update_from_row(uint32_t aRow, i_graphics_context& aGc);
        bool update_section_width(uint32_t aColumn, const size& aCellExtents, i_graphics_context& aGc);
    private:
        i_header_view_owner& iOwner;
        sink iSink;
        std::vector<std::array<sink, 2>> iButtonSinks;
        header_view_type iType;
        std::shared_ptr<i_item_model> iModel;
        std::shared_ptr<i_item_presentation_model> iPresentationModel;
        sink iPresentationModelSink;
        bool iExpandLastColumn;
        optional_dimension iSeparatorWidth;
        std::vector<section_dimension> iSectionWidths;
        std::unique_ptr<updater> iUpdater;
    };
}