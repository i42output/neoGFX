// drop_list.hpp
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
#include <neogfx/gui/window/window.hpp>
#include <neogfx/gui/widget/list_view.hpp>
#include <neogfx/gui/widget/item_presentation_model.hpp>

namespace neogfx
{
    class drop_list;

    template <typename ItemModel = item_model>
    class default_drop_list_presentation_model : public basic_item_presentation_model<ItemModel>
    {
        typedef basic_item_presentation_model<ItemModel> base_type;
    public:
        using item_model_type = ItemModel;
    public:
        default_drop_list_presentation_model(drop_list& aDropList) : base_type{}, iDropList{ aDropList }
        {
        }
        default_drop_list_presentation_model(drop_list& aDropList, item_model_type& aModel) : base_type{ aModel }, iDropList{ aDropList }
        {
        }
    public:
        item_cell_flags column_flags(item_presentation_model_index::value_type aColumn) const override
        {
            return base_type::column_flags(aColumn) & ~item_cell_flags::Editable;
        }
    public:
        optional_color cell_color(const item_presentation_model_index& aIndex, color_role aColorRole) const override
        {
            if (aColorRole == color_role::Background && 
                (base_type::cell_meta(aIndex).selection & item_cell_selection_flags::Current) == item_cell_selection_flags::Current &&
                iDropList.view_created()) 
            {
                auto backgroundColor = iDropList.view().background_color().dark() ? color::Black : color::White;
                if (backgroundColor == iDropList.view().background_color())
                    backgroundColor = backgroundColor.shade(0x20);
                return backgroundColor;
            }
            else
                return base_type::cell_color(aIndex, aColorRole);
        }
    private:
        drop_list& iDropList;
    };

    class drop_list_view : public list_view
    {
        friend class drop_list_popup;
        friend class drop_list;
    public:
        drop_list_view(i_layout& aLayout, drop_list& aDropList);
        ~drop_list_view();
    public:
        bool changing_text() const;
    public:
        using list_view::total_item_area;
    protected:
        void items_filtered() override;
    protected:
        void current_index_changed(const optional_item_presentation_model_index& aCurrentIndex, const optional_item_presentation_model_index& aPreviousIndex) override;
    protected:
        void mouse_button_released(mouse_button aButton, const point& aPosition) override;
    protected:
        bool key_pressed(scan_code_e aScanCode, key_code_e aKeyCode, key_modifiers_e aKeyModifiers) override;
    public:
        color background_color() const override;
    private:
        drop_list& iDropList;
        bool iChangingText;
    };
    
    class drop_list_popup : public window
    {
        friend class drop_list;
        friend class drop_list_view;
    public:
        drop_list_popup(drop_list& aDropList);
        ~drop_list_popup();
    public:
        const drop_list_view& view() const;
        drop_list_view& view();
    protected:
        color frame_color() const override;
    protected:
        double rendering_priority() const override;
    public:
        using window::show;
        bool show(bool aVisible) override;
    protected:
        neogfx::size_policy size_policy() const override;
        size minimum_size(const optional_size& aAvailableSpace = optional_size()) const override;
    public:
        bool can_dismiss(const i_widget*) const override;
        dismissal_type_e dismissal_type() const override;
        bool dismissed() const override;
        void dismiss() override;
    private:
        size ideal_size() const;
        void update_placement();
    private:
        drop_list& iDropList;
        drop_list_view iView;
    };

    class i_drop_list_input_widget
    {
    public:
        declare_event(text_changed)
    public:
        class i_visitor
        {
        public:
            virtual void visit(i_drop_list_input_widget& aInputWidget, push_button& aButtonWidget) = 0;
            virtual void visit(i_drop_list_input_widget& aInputWidget, line_edit& aTextWidget) = 0;
        };
    public:
        virtual ~i_drop_list_input_widget() = default;
    public:
        virtual void accept(i_visitor& aVisitor) = 0;
    public:
        virtual const i_widget& as_widget() const = 0;
        virtual i_widget& as_widget() = 0;
    public:
        virtual bool editable() const = 0;
        virtual const i_widget& image_widget() const = 0;
        virtual i_widget& image_widget() = 0;
        virtual const i_widget& text_widget() const = 0;
        virtual i_widget& text_widget() = 0;
        virtual size spacing() const = 0;
        virtual void set_spacing(const size& aSpacing) = 0;
        virtual const i_texture& image() const = 0;
        virtual void set_image(const i_texture& aImage) = 0;
        virtual const std::string& text() const = 0;
        virtual void set_text(const std::string& aText) = 0;
    };

    class drop_list : public widget, private i_drop_list_input_widget::i_visitor
    {
        friend class drop_list_view;
    public:
        define_event(SelectionChanged, selection_changed, optional_item_model_index)
    public:
        enum class style : uint32_t
        {
            Normal                = 0x0000,
            Editable            = 0x0001,
            ListAlwaysVisible    = 0x0002,
            NoFilter            = 0x0004
        };
        friend style operator|(style aLhs, style aRhs)
        {
            return static_cast<style>(static_cast<uint32_t>(aLhs) | static_cast<uint32_t>(aRhs));
        }
        friend style operator&(style aLhs, style aRhs)
        {
            return static_cast<style>(static_cast<uint32_t>(aLhs) & static_cast<uint32_t>(aRhs));
        }
        friend style operator~(style aLhs)
        {
            return static_cast<style>(~static_cast<uint32_t>(aLhs));
        }
    public:
        struct no_selection : std::runtime_error { no_selection() : std::runtime_error("neogfx::drop_list::no_selection") {} };
    private:
        class list_proxy
        {
        private:
            class view_container : public framed_widget
            {
            public:
                view_container(i_layout& aLayout);
            public:
                color background_color() const override;
                color frame_color() const override;
            };
        public:
            struct no_view : std::logic_error { no_view() : std::logic_error("neogfx::drop_list::list_proxy::no_view") {} };
        public:
            list_proxy(drop_list& aDropList);
        public:
            bool view_created() const;
            drop_list_view& view() const;
        public:
            void show_view();
            void hide_view();
            void update_view_placement();
        private:
            drop_list& iDropList;
            mutable std::optional<drop_list_popup> iPopup;
            mutable std::optional<view_container> iViewContainer;
            mutable std::optional<drop_list_view> iView;
        };
    public:
        drop_list(style aStyle = style::Normal);
        drop_list(i_widget& aParent, style aStyle = style::Normal);
        drop_list(i_layout& aLayout, style aStyle = style::Normal);
        ~drop_list();
    public:
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
        bool has_selection_model() const;
        const i_item_selection_model& selection_model() const;
        i_item_selection_model& selection_model();
        void set_selection_model(i_item_selection_model& aSelectionModel);
        void set_selection_model(std::shared_ptr<i_item_selection_model> aSelectionModel);
    public:
        bool has_selection() const;
        const item_model_index& selection() const;
    public:
        bool view_created() const;
        void show_view();
        void hide_view();
        drop_list_view& view() const;
        void accept_selection();
        void cancel_selection();
        void cancel_and_restore_selection();
    public:
        bool editable() const;
        void set_editable(bool aEditable);
        bool list_always_visible() const;
        void set_list_always_visible(bool aListAlwaysVisible);
        bool filter_enabled() const;
        void enable_filter(bool aEnableFilter);
        const i_drop_list_input_widget& input_widget() const;
        i_drop_list_input_widget& input_widget();
    public:
        bool handling_text_change() const;
        bool accepting_selection() const;
        bool cancelling_selection() const;
    public:
        neogfx::size_policy size_policy() const override;
        size minimum_size(const optional_size& aAvailableSpace = optional_size()) const override;
    private:
        void visit(i_drop_list_input_widget& aInputWidget, push_button& aButtonWidget) override;
        void visit(i_drop_list_input_widget& aInputWidget, line_edit& aTextWidget) override;
    private:
        void init();
        void update_widgets(bool aForce = false);
        void update_arrow();
        void handle_clicked();
        void handle_cancel_selection(bool aRestoreSavedSelection, bool aUpdateEditor = true);
        bool handle_proxy_key_event(const neogfx::keyboard_event& aEvent);
    private:
        style iStyle;
        vertical_layout iLayout0;
        horizontal_layout iLayout1;
        std::unique_ptr<i_drop_list_input_widget> iInputWidget;
        std::shared_ptr<i_item_model> iModel;
        std::shared_ptr<i_item_presentation_model> iPresentationModel;
        std::shared_ptr<i_item_selection_model> iSelectionModel;
        sink iSink;
        mutable std::optional<std::pair<color, texture>> iDownArrowTexture;
        image_widget iDownArrow;
        list_proxy iListProxy;
        optional_item_model_index iSavedSelection;
        optional_item_model_index iSelection;
        bool iHandlingTextChange;
        bool iAcceptingSelection;
        bool iCancellingSelection;
    };
}