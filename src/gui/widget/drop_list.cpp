// drop_list.cpp
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

#include <neogfx/neogfx.hpp>
#include <neogfx/app/i_app.hpp>
#include <neogfx/gfx/graphics_context.hpp>
#include <neogfx/gui/layout/spacer.hpp>
#include <neogfx/gui/widget/item_model.hpp>
#include <neogfx/gui/widget/item_presentation_model.hpp>
#include <neogfx/gui/widget/item_selection_model.hpp>
#include <neogfx/gui/widget/push_button.hpp>
#include <neogfx/gui/widget/line_edit.hpp>
#include <neogfx/gui/widget/drop_list.hpp>

namespace neogfx
{
    drop_list_view::drop_list_view(i_layout& aLayout, drop_list& aDropList) :
        list_view{ aLayout, false, frame_style::NoFrame },
        iDropList{ aDropList },
        iChangingText{ false }
    {
        set_padding(neogfx::padding{});
        if (!iDropList.editable())
            enable_hot_tracking();
        set_selection_model(iDropList.iSelectionModel);
        set_presentation_model(iDropList.iPresentationModel);
        set_model(iDropList.iModel);
    }

    drop_list_view::~drop_list_view()
    {
    }

    bool drop_list_view::changing_text() const
    {
        return iChangingText;
    }

    void drop_list_view::items_filtered()
    {
        list_view::items_filtered();
        iDropList.iListProxy.update_view_placement();
    }

    void drop_list_view::current_index_changed(const optional_item_presentation_model_index& aCurrentIndex, const optional_item_presentation_model_index& aPreviousIndex)
    {
        list_view::current_index_changed(aCurrentIndex, aPreviousIndex);
        if (!presentation_model().filtering() && !iDropList.handling_text_change())
        {
            neolib::scoped_flag sf{ iChangingText };
            texture image;
            std::string text;
            if (aCurrentIndex != std::nullopt)
            {
                auto const& maybeImage = presentation_model().cell_image(*aCurrentIndex);
                if (maybeImage != std::nullopt)
                    image = *maybeImage;
                text = presentation_model().cell_to_string(*aCurrentIndex);
            }
            iDropList.input_widget().set_spacing(presentation_model().cell_spacing(*this));
            iDropList.input_widget().set_image(image);
            iDropList.input_widget().set_text(text);
        }
    }

    void drop_list_view::mouse_button_released(mouse_button aButton, const point& aPosition)
    {
        bool wasCapturing = capturing();
        list_view::mouse_button_released(aButton, aPosition);
        if (aButton == mouse_button::Left && wasCapturing)
        {
            if (selection_model().has_current_index())
            {
                auto item = item_at(aPosition);
                if (item != std::nullopt && row_rect(*item).contains(aPosition) && item->row() == selection_model().current_index().row())
                    iDropList.accept_selection();
            }
        }
    }

    bool drop_list_view::key_pressed(scan_code_e aScanCode, key_code_e aKeyCode, key_modifiers_e aKeyModifiers)
    {
        bool handled = false;
        switch (aScanCode)
        {
        case ScanCode_ESCAPE:
            iDropList.cancel_and_restore_selection();
            handled = true;
            break;
        case ScanCode_RETURN:
            if (selection_model().has_current_index())
                iDropList.accept_selection();
            else
                service<i_basic_services>().system_beep();
            handled = true;
            break;
        case ScanCode_LEFT:
        case ScanCode_RIGHT:
            handled = false;
            break;
        case ScanCode_DOWN:
            handled = list_view::key_pressed(aScanCode, aKeyCode, aKeyModifiers);
            if (!handled && !selection_model().has_current_index() && presentation_model().rows() > 0)
            {
                selection_model().set_current_index(item_presentation_model_index{ 0, 0 });
                handled = true;
            }
            break;
        default:
            handled = list_view::key_pressed(aScanCode, aKeyCode, aKeyModifiers);
            break;
        }
        return handled;
    }

    drop_list_popup::drop_list_popup(drop_list& aDropList) :
        window{ 
            aDropList,
            window_placement{ 
                aDropList.non_client_rect().bottom_left() + aDropList.root().window_position(),
                aDropList.non_client_rect().extents() },
            window_style::NoDecoration | window_style::NoActivate | window_style::RequiresOwnerFocus | window_style::DismissOnOwnerClick | window_style::InitiallyHidden | window_style::InitiallyRenderable | window_style::DropShadow },
        iDropList{ aDropList },
        iView{ client_layout(), aDropList }
    {
        set_padding(neogfx::padding{});
        client_layout().set_padding(neogfx::padding{});
        client_layout().set_size_policy(size_constraint::Expanding);
        update_placement();
        show();
    }

    drop_list_popup::~drop_list_popup()
    {
    }

    const drop_list_view& drop_list_popup::view() const
    {
        return iView;
    }

    drop_list_view& drop_list_popup::view()
    {
        return iView;
    }

    color drop_list_popup::frame_color() const
    {
        if (window::has_frame_color())
            return window::frame_color();
        auto viewBackgroundColor = iView.background_color();
        auto backgroundColor = viewBackgroundColor.unshaded(0x20);
        if (backgroundColor == viewBackgroundColor)
            backgroundColor = viewBackgroundColor.shaded(0x20);
        return backgroundColor;
    }

    double drop_list_popup::rendering_priority() const
    {
        if (visible())
            return 1.0;
        else
            return window::rendering_priority();
    }

    bool drop_list_popup::show(bool aVisible)
    {
        if (aVisible && !visible())
        {
            update_placement();
            if (!service<i_keyboard>().is_keyboard_grabbed_by(view()))
                service<i_keyboard>().grab_keyboard(view());
        }
        else if (!aVisible)
        {
            if (service<i_keyboard>().is_keyboard_grabbed_by(view()))
                service<i_keyboard>().ungrab_keyboard(view());
        }
        return window::show(aVisible);
    }

    size_policy drop_list_popup::size_policy() const
    {
        if (has_size_policy())
            return window::size_policy();
        return size_constraint::Minimum;
    }

    size drop_list_popup::minimum_size(const optional_size&) const
    {
        if (window::has_minimum_size())
            return window::minimum_size();
        return ideal_size().max(iDropList.minimum_size());
    }

    bool drop_list_popup::can_dismiss(const i_widget*) const
    {
        return true;
    }

    window::dismissal_type_e drop_list_popup::dismissal_type() const
    {
        return CloseOnDismissal;
    }

    bool drop_list_popup::dismissed() const
    {
        return surface().is_closed();
    }

    void drop_list_popup::dismiss()
    {
        // dismissal may be for reasons other than explicit acceptance or cancellation (e.g. clicking outside of list popup)...
        auto* dropListForCancellation = iDropList.handling_text_change() || iDropList.accepting_selection() || iDropList.cancelling_selection() ? nullptr : &iDropList;
        if (service<i_keyboard>().is_keyboard_grabbed_by(view()))
            service<i_keyboard>().ungrab_keyboard(view());
        close();
        // 'this' will be destroyed at this point...
        if (dropListForCancellation)
            dropListForCancellation->cancel_and_restore_selection();
    }

    size drop_list_popup::ideal_size() const
    {
        auto totalArea = iView.total_item_area(*this);
        auto idealSize = size{ effective_frame_width() * 2.0 } + padding().size() + totalArea + iView.padding().size();
        idealSize.cy = std::min(idealSize.cy, iDropList.root().as_widget().extents().cy / 2.0);
        if (idealSize.cy - (effective_frame_width() * 2.0 + padding().size().cy) < totalArea.cy)
            idealSize.cx += vertical_scrollbar().width();
        return idealSize.max(iDropList.minimum_size());
    }

    void drop_list_popup::update_placement()
    {
        view().set_padding(iDropList.input_widget().as_widget().padding() - view().presentation_model().cell_padding(*this) - view().effective_frame_width());

        // First stab at sizing ourselves...
        resize(minimum_size());
        
        // Workout ideal position whereby text for current item in drop list popup view is at same position as text in drop button or line edit...
        point currentItemPos;
        currentItemPos += view().presentation_model().cell_padding(*this).top_left();
        currentItemPos += view().presentation_model().cell_spacing(*this) / 2.0;
        currentItemPos -= point{ effective_frame_width(), effective_frame_width() };
        if (view().presentation_model().rows() > 0 && view().presentation_model().columns() > 0)
        {
            auto index = (view().selection_model().has_current_index() ?
                view().selection_model().current_index() :
                item_presentation_model_index{ 0, 0 });
            view().make_visible(index);
            currentItemPos += view().cell_rect(index).top_left();
            auto const& maybeCellImageSize = view().presentation_model().cell_image_size(index);
            if (maybeCellImageSize != std::nullopt)
                currentItemPos.x += (maybeCellImageSize->cx + view().presentation_model().cell_spacing(*this).cx);
        }
        point inputWidgetPos{ iDropList.non_client_rect().top_left() + iDropList.root().window_position() };
        point textWidgetPos{ iDropList.input_widget().text_widget().non_client_rect().top_left() + iDropList.input_widget().text_widget().padding().top_left() + iDropList.root().window_position() };
        point popupPos = -currentItemPos + textWidgetPos;
        
        // Popup goes below line edit if editable or on top of drop button if not...
        if (iDropList.editable())
            popupPos.y = inputWidgetPos.y + iDropList.extents().cy;

        size popupExtents = extents() + size{ textWidgetPos.x + iDropList.extents().cx - popupPos.x - extents().cx + currentItemPos.x * 2.0, 0.0 };
        popupExtents = popupExtents.max(iDropList.extents());

        rect rectPopup{ popupPos, popupExtents };
        
        // Check we are not out of bounds of desktop window and correct if we are...
        auto correctedRect = corrected_popup_rect(*this, rectPopup);
        if (iDropList.editable() && correctedRect.y < inputWidgetPos.y)
            correctedRect.y = inputWidgetPos.y - extents().cy;

        surface().move_surface(correctedRect.top_left());
        resize(correctedRect.extents());
    }

    drop_list::list_proxy::view_container::view_container(i_layout& aLayout) :
        base_type{ aLayout }
    {
    }

    color  drop_list::list_proxy::view_container::palette_color(color_role aColorRole) const
    {
        if (has_palette_color(aColorRole))
            return base_type::palette_color(aColorRole);
        if (aColorRole == color_role::Background)
        {
            if (container_background_color().light())
                return parent().palette_color(aColorRole).darker(24);
            else
                return parent().palette_color(aColorRole).lighter(24);
        }
        return base_type::palette_color(aColorRole);
    }

    color drop_list::list_proxy::view_container::frame_color() const
    {
        if (has_frame_color())
            return base_type::frame_color();
        else if (container_background_color().light())
            return background_color().darker(24);
        else
            return background_color().lighter(24);
    }

    drop_list::list_proxy::list_proxy(drop_list& aDropList) :
        iDropList{ aDropList }
    {
    }

    bool drop_list::list_proxy::view_created() const
    {
        return iPopup != std::nullopt || iView != std::nullopt;
    }

    drop_list_view& drop_list::list_proxy::view() const
    {
        if (iPopup != std::nullopt)
            return iPopup->view();
        else if (iView != std::nullopt)
            return *iView;
        throw no_view();
    }

    void drop_list::list_proxy::show_view()
    {
        if (!view_created())
        {
            if (iDropList.list_always_visible())
            {
                iViewContainer.emplace(iDropList.iLayout0);
                iViewContainer->set_padding(neogfx::padding{});
                iViewContainer->set_layout(std::make_shared<vertical_layout>());
                iViewContainer->layout().set_padding(neogfx::padding{});
                iView.emplace(iViewContainer->layout(), iDropList);
            }
            else
            {
                iPopup.emplace(iDropList);
                iPopup->Closed([this]()
                {
                    iPopup = std::nullopt;
                });
                update_view_placement();
            }
        }
    }

    void drop_list::list_proxy::hide_view()
    {
        if (iPopup != std::nullopt)
            iPopup->close();
        else if (iView != std::nullopt)
        {
            iView = std::nullopt;
            iViewContainer = std::nullopt;
        }
    }

    void drop_list::list_proxy::update_view_placement()
    {
        if (iPopup != std::nullopt)
            iPopup->update_placement();
    }

    namespace
    {
        class non_editable_input_widget : public push_button, public i_drop_list_input_widget
        {
        public:
            define_declared_event(TextChanged, text_changed)
        public:
            non_editable_input_widget(i_layout& aLayout) :
                push_button{ aLayout, push_button_style::DropList }
            {
                image_widget().set_padding(neogfx::padding{});
                text_widget().set_padding(neogfx::padding{});
            }
        public:
            void accept(i_drop_list_input_widget::i_visitor& aVisitor) override
            {
                aVisitor.visit(*this, *this);
            }
        public:
            const i_widget& as_widget() const override
            {
                return *this;
            }
            i_widget& as_widget() override
            {
                return *this;
            }
        public:
            bool editable() const override
            {
                return false;
            }
            const i_widget& image_widget() const override
            {
                return push_button::image_widget();
            }
            i_widget& image_widget() override
            {
                return push_button::image_widget();
            }
            const i_widget& text_widget() const override
            {
                return push_button::text_widget();
            }
            i_widget& text_widget() override
            {
                return push_button::text_widget();
            }
            size spacing() const override
            {
                return label().layout().spacing();
            }
            void set_spacing(const size& aSpacing) override
            {
                label().layout().set_spacing(aSpacing);
            }
            const i_texture& image() const override
            {
                return push_button::image();
            }
            void set_image(const i_texture& aImage) override
            {
                push_button::set_image(aImage);
                push_button::image_widget().show(!aImage.is_empty());
            }
            const std::string& text() const override
            {
                return push_button::text();
            }
            void set_text(const std::string& aText) override
            {
                return push_button::set_text(aText);
            }
        };

        class editable_input_widget : public framed_widget<>, public i_drop_list_input_widget
        {
            typedef framed_widget<> base_type;
        public:
            define_declared_event(TextChanged, text_changed)
        public:
            editable_input_widget(i_layout& aLayout) :
                base_type{ aLayout, frame_style::SolidFrame },
                iLayout{ *this },
                iImage{ iLayout },
                iEditor{ iLayout, frame_style::NoFrame }
            {
                set_padding(neogfx::padding{});
                image_widget().set_padding(neogfx::padding{});
                text_widget().set_padding(neogfx::padding{});
                iImage.hide();
            }
        public:
            void accept(i_drop_list_input_widget::i_visitor& aVisitor) override
            {
                aVisitor.visit(*this, iEditor);
            }
        public:
            const i_widget& as_widget() const override
            {
                return *this;
            }
            i_widget& as_widget() override
            {
                return *this;
            }
        public:
            bool editable() const override
            {
                return true;
            }
            const i_widget& image_widget() const override
            {
                return iImage;
            }
            i_widget& image_widget() override
            {
                return iImage;
            }
            const i_widget& text_widget() const override
            {
                return iEditor;
            }
            i_widget& text_widget() override
            {
                return iEditor;
            }
            size spacing() const override
            {
                return layout().spacing();
            }
            void set_spacing(const size& aSpacing) override
            {
                layout().set_spacing(aSpacing);
            }
            const i_texture& image() const override
            {
                return iImage.image();
            }
            void set_image(const i_texture& aImage) override
            {
                iImage.set_image(aImage);
                iImage.show(!aImage.is_empty());

            }
            const std::string& text() const override
            {
                return iEditor.text();
            }
            void set_text(const std::string& aText) override
            {
                iEditor.set_text(aText);
            }
        protected:
            bool transparent_background() const override
            {
                return false;
            }
            color palette_color(color_role aColorRole) const override
            {
                if (has_palette_color(aColorRole))
                    return base_type::palette_color(aColorRole);
                if (aColorRole == color_role::Background)
                    return palette_color(color_role::Base);
                return base_type::palette_color(aColorRole);
            }
        protected:
            color frame_color() const override
            {
                return iEditor.frame_color();
            }
        private:
            horizontal_layout iLayout;
            neogfx::image_widget iImage;
            line_edit iEditor;
        };
    }

    drop_list::drop_list(style aStyle) :
        iStyle { aStyle },
        iLayout0{ *this },
        iLayout1{ iLayout0 },
        iDownArrow{ texture{} }, 
        iListProxy{ *this },
        iHandlingTextChange{ false },
        iAcceptingSelection{ false },
        iCancellingSelection{ false }
    {
        init();
    }

    drop_list::drop_list(i_widget& aParent, style aStyle) :
        widget{ aParent },
        iStyle{ aStyle },
        iLayout0{ *this },
        iLayout1{ iLayout0 },
        iDownArrow{ texture{} },
        iListProxy{ *this },
        iHandlingTextChange{ false },
        iAcceptingSelection{ false },
        iCancellingSelection{ false }
    {
        init();
    }

    drop_list::drop_list(i_layout& aLayout, style aStyle) :
        widget{ aLayout },
        iStyle{ aStyle },
        iLayout0{ *this },
        iLayout1{ iLayout0 },
        iDownArrow{ texture{} },
        iListProxy{ *this },
        iHandlingTextChange{ false },
        iAcceptingSelection{ false },
        iCancellingSelection{ false }
    {
        init();
    }

    drop_list::~drop_list()
    {
    }

    bool drop_list::has_model() const
    {
        if (iModel)
            return true;
        else
            return false;
    }

    const i_item_model& drop_list::model() const
    {
        return *iModel;
    }

    i_item_model& drop_list::model()
    {
        return *iModel;
    }

    void drop_list::set_model(i_item_model& aModel)
    {
        set_model(std::shared_ptr<i_item_model>{std::shared_ptr<i_item_model>{}, &aModel});
    }

    void drop_list::set_model(std::shared_ptr<i_item_model> aModel)
    {
        if (iModel == aModel)
            return;
        iModel = aModel;
        if (has_model())
        {
            if (has_presentation_model())
                presentation_model().set_item_model(*aModel);
        }
        if (view_created())
            view().set_model(aModel);
        layout_manager().layout_items(true);
        update();
    }

    bool drop_list::has_presentation_model() const
    {
        if (iPresentationModel)
            return true;
        else
            return false;
    }

    const i_item_presentation_model& drop_list::presentation_model() const
    {
        return *iPresentationModel;
    }

    i_item_presentation_model& drop_list::presentation_model()
    {
        return *iPresentationModel;
    }

    void drop_list::set_presentation_model(i_item_presentation_model& aPresentationModel)
    {
        set_presentation_model(std::shared_ptr<i_item_presentation_model>{std::shared_ptr<i_item_presentation_model>{}, &aPresentationModel});
    }

    void drop_list::set_presentation_model(std::shared_ptr<i_item_presentation_model> aPresentationModel)
    {
        if (iPresentationModel == aPresentationModel)
            return;
        iPresentationModel = aPresentationModel;
        if (has_presentation_model() && has_model())
            presentation_model().set_item_model(model());
        if (has_presentation_model() && has_selection_model())
            selection_model().set_presentation_model(*aPresentationModel);
        if (view_created())
            view().set_presentation_model(aPresentationModel);
        if (has_layout_manager())
            layout_manager().layout_items(true);
        update();
    }

    bool drop_list::has_selection_model() const
    {
        if (iSelectionModel)
            return true;
        else
            return false;
    }

    const i_item_selection_model& drop_list::selection_model() const
    {
        return *iSelectionModel;
    }

    i_item_selection_model& drop_list::selection_model()
    {
        return *iSelectionModel;
    }

    void drop_list::set_selection_model(i_item_selection_model& aSelectionModel)
    {
        set_selection_model(std::shared_ptr<i_item_selection_model>{std::shared_ptr<i_item_selection_model>{}, &aSelectionModel});
    }

    void drop_list::set_selection_model(std::shared_ptr<i_item_selection_model> aSelectionModel)
    {
        if (iSelectionModel == aSelectionModel)
            return;
        iSelectionModel = aSelectionModel;
        if (has_presentation_model() && has_selection_model())
            selection_model().set_presentation_model(presentation_model());
        if (view_created())
            view().set_selection_model(aSelectionModel);
        update();
    }

    bool drop_list::has_selection() const
    {
        return iSelection != std::nullopt;
    }

    const item_model_index& drop_list::selection() const
    {
        if (has_selection())
            return *iSelection;
        throw no_selection();
    }

    bool drop_list::view_created() const
    {
        return iListProxy.view_created();
    }

    void drop_list::show_view()
    {
        iListProxy.show_view();
        if (editable() && !accepting_selection())
        {
            if ((iStyle & style::NoFilter) != style::NoFilter)
                presentation_model().filter_by(0, input_widget().text());
            else if (presentation_model().rows() > 0)
            {
                auto findResult = presentation_model().find_item(input_widget().text());
                if (findResult != std::nullopt)
                    selection_model().set_current_index(*findResult);
                else
                    selection_model().unset_current_index();
            }
        }
    }

    void drop_list::hide_view()
    {
        if (!list_always_visible())
            iListProxy.hide_view();
    }

    drop_list_view& drop_list::view() const
    {
        return iListProxy.view();
    }

    void drop_list::accept_selection()
    {
        neolib::scoped_flag sf{ iAcceptingSelection };

        optional_item_model_index newSelection = (selection_model().has_current_index() ?
            presentation_model().to_item_model_index(selection_model().current_index()) : optional_item_model_index{});

        hide_view();
        presentation_model().reset_filter();

        if (newSelection != std::nullopt)
            selection_model().set_current_index(presentation_model().from_item_model_index(*newSelection));
        else
            selection_model().unset_current_index();

        if (iSavedSelection != newSelection)
        {
            iSelection = newSelection;
            SelectionChanged.async_trigger(iSelection);
        }
        iSavedSelection = std::nullopt;

        update_widgets(true);
    }

    void drop_list::cancel_selection()
    {
        handle_cancel_selection(view_created());
    }

    void drop_list::cancel_and_restore_selection()
    {
        handle_cancel_selection(true);
    }

    bool drop_list::editable() const
    {
        return (iStyle & style::Editable) == style::Editable;
    }

    void drop_list::set_editable(bool aEditable)
    {
        if (editable() != aEditable)
        {
            if (aEditable)
                iStyle = (iStyle | style::Editable);
            else
                iStyle = (iStyle & ~style::Editable);
            update_widgets();
        }
    }

    bool drop_list::list_always_visible() const
    {
        return (iStyle & style::ListAlwaysVisible) == style::ListAlwaysVisible;
    }

    void drop_list::set_list_always_visible(bool aListAlwaysVisible)
    {
        if (list_always_visible() != aListAlwaysVisible)
        {
            if (aListAlwaysVisible)
                iStyle = (iStyle | style::ListAlwaysVisible);
            else
                iStyle = (iStyle & ~style::ListAlwaysVisible);
            if (view_created())
                hide_view();
            update_widgets();
        }
    }

    bool drop_list::filter_enabled() const
    {
        return (iStyle & style::NoFilter) != style::NoFilter;
    }

    void drop_list::enable_filter(bool aEnableFilter)
    {
        if (filter_enabled() != aEnableFilter)
        {
            if (!aEnableFilter)
                iStyle = (iStyle | style::NoFilter);
            else
                iStyle = (iStyle & ~style::NoFilter);
            // todo
        }
    }

    const i_drop_list_input_widget& drop_list::input_widget() const
    {
        return *iInputWidget;
    }

    i_drop_list_input_widget& drop_list::input_widget()
    {
        return *iInputWidget;
    }

    bool drop_list::handling_text_change() const
    {
        return iHandlingTextChange;
    }

    bool drop_list::accepting_selection() const
    {
        return iAcceptingSelection;
    }

    bool drop_list::cancelling_selection() const
    {
        return iCancellingSelection;
    }

    neogfx::size_policy drop_list::size_policy() const
    {
        if (has_size_policy())
            return widget::size_policy();
        if (list_always_visible())
            return neogfx::size_policy{ size_constraint::Minimum, size_constraint::Expanding };
        else
            return size_constraint::Minimum;
    }

    size drop_list::minimum_size(const optional_size& aAvailableSpace) const
    {
        auto minimumSize = widget::minimum_size(aAvailableSpace);
        if (widget::has_minimum_size())
            return minimumSize;
        if (input_widget().image_widget().visible())
            minimumSize.cx -= input_widget().image_widget().minimum_size().cx;
        if (input_widget().image_widget().visible() && input_widget().text_widget().visible())
            minimumSize.cx -= input_widget().spacing().cx;
        if (input_widget().text_widget().visible())
        {
            minimumSize.cx -= input_widget().text_widget().minimum_size().cx;
            minimumSize.cx += input_widget().text_widget().padding().size().cx;
        }
        dimension modelWidth = 0.0;
        if (has_presentation_model())
        {
            graphics_context gc{ *this, graphics_context::type::Unattached };
            modelWidth = presentation_model().column_width(0, gc, false);
            if (list_always_visible())
                modelWidth += view().vertical_scrollbar().width();
        }
        minimumSize.cx += modelWidth;
        return minimumSize;
    }

    void drop_list::visit(i_drop_list_input_widget& aInputWidget, line_edit& aTextWidget)
    {
        aTextWidget.TextChanged([this, &aInputWidget, &aTextWidget]()
        {
            if (!accepting_selection())
            {
                neolib::scoped_flag sf{ iHandlingTextChange };
                iSavedSelection = std::nullopt;
                aInputWidget.text_changed().trigger();
                if (aTextWidget.has_focus() && (!view_created() || !view().changing_text()))
                {
                    if (!aTextWidget.text().empty())
                        show_view();
                    else
                    {
                        if (view_created())
                            hide_view();
                        cancel_selection();
                    }
                }
            }
        });
        aTextWidget.Keyboard([this, &aTextWidget](const neogfx::keyboard_event& aEvent)
        {
            if (handle_proxy_key_event(aEvent))
                aTextWidget.Keyboard.accept();
        });
    }

    void drop_list::visit(i_drop_list_input_widget&, push_button& aButtonWidget)
    {
        aButtonWidget.Keyboard([this, &aButtonWidget](const neogfx::keyboard_event& aEvent)
        {
            if (handle_proxy_key_event(aEvent))
                aButtonWidget.Keyboard.accept();
        });
    }

    void drop_list::init()
    {
        set_selection_model(std::shared_ptr<i_item_selection_model>(new item_selection_model{ item_selection_mode::NoSelection }));
        set_presentation_model(std::shared_ptr<i_item_presentation_model>(new default_drop_list_presentation_model<>{ *this }));
        set_model(std::shared_ptr<i_item_model>(new item_model{}));

        set_padding(neogfx::padding{});
        iLayout0.set_padding(neogfx::padding{});
        iLayout1.set_padding(neogfx::padding{});

        update_widgets();

        iSink += service<i_app>().current_style_changed([this](style_aspect) { update_widgets(true); });

        presentation_model().set_cell_padding(neogfx::padding{ 3.0, 3.0 }, *this);
    }

    void drop_list::update_widgets(bool aForce)
    {
        bool changed = false;

        if (!editable() && (iInputWidget == nullptr || iInputWidget->editable()))
        {
            iInputWidget = std::make_unique<non_editable_input_widget>(iLayout1);
            auto& inputWidget = static_cast<non_editable_input_widget&>(iInputWidget->as_widget());

            inputWidget.clicked([this]() { handle_clicked(); });

            inputWidget.set_size_policy(size_constraint::Expanding);
            
            auto& inputLabelLayout = inputWidget.label().layout();
            inputLabelLayout.set_alignment(neogfx::alignment::Left | neogfx::alignment::VCenter);
            auto& s1 = inputWidget.layout().add_spacer();
            s1.set_minimum_width(inputLabelLayout.spacing().cx);
            inputWidget.layout().add(iDownArrow);
            auto& s2 = inputWidget.layout().add_spacer();
            s2.set_fixed_size(size{ inputLabelLayout.spacing().cx, 0.0 });

            changed = true;
        }
        else if (editable() && (iInputWidget == nullptr || !iInputWidget->editable()))
        {
            iInputWidget = std::make_unique<editable_input_widget>(iLayout1);
            auto& inputWidget = static_cast<editable_input_widget&>(iInputWidget->as_widget());

            inputWidget.Mouse([&inputWidget, this](const neogfx::mouse_event& aEvent)
            {
                if (aEvent.type() == mouse_event_type::ButtonReleased &&
                    aEvent.mouse_button() == mouse_button::Left &&
                    inputWidget.client_rect().contains(aEvent.position() - non_client_rect().top_left()) &&
                    inputWidget.capturing())
                {
                    inputWidget.text_widget().set_focus();
                    handle_clicked();
                }
            });

            inputWidget.set_size_policy(size_constraint::Expanding);

            inputWidget.layout().add(iDownArrow);

            changed = true;
        }

        if (changed || aForce)
        {
            update_arrow();

            if (changed)
                input_widget().accept(*this);

            texture image;
            std::string text;
            if (selection_model().has_current_index())
            {
                auto const& maybeImage = presentation_model().cell_image(selection_model().current_index());
                if (maybeImage != std::nullopt)
                    image = *maybeImage;
                text = presentation_model().cell_to_string(selection_model().current_index());
            }
            input_widget().set_spacing(presentation_model().cell_spacing(*this));
            input_widget().set_image(image);
            input_widget().set_text(text);
        }

        if (list_always_visible())
        {
            iDownArrow.hide();
            show_view();
        }
        else
        {
            iDownArrow.show();
        }
    }

    void drop_list::update_arrow()
    {
        auto ink = service<i_app>().current_style().palette().color(color_role::Text);
        if (iDownArrowTexture == std::nullopt || iDownArrowTexture->first != ink)
        {
            const char* sDownArrowImagePattern
            {
                "[10,4]"
                "{0,paper}"
                "{1,ink}"

                "0111111110"
                "0011111100"
                "0001111000"
                "0000110000"
            };
            const char* sDownArrowHighDpiImagePattern
            {
                "[20,8]"
                "{0,paper}"
                "{1,ink}"

                "00111111111111111100"
                "00011111111111111000"
                "00001111111111110000"
                "00000111111111100000"
                "00000011111111000000"
                "00000001111110000000"
                "00000000111100000000"
                "00000000011000000000"
            };
            iDownArrowTexture.emplace(
                ink, !high_dpi() ?
                    neogfx::image{ "neogfx::drop_list::iDownArrowTexture::" + ink.to_string(), sDownArrowImagePattern, { { "paper", color{} }, { "ink", ink } } } :
                    neogfx::image{ "neogfx::drop_list::iDownArrowHighDpiTexture::" + ink.to_string(), sDownArrowHighDpiImagePattern, { { "paper", color{} }, { "ink", ink } }, 2.0 });
        }
        iDownArrow.set_image(iDownArrowTexture->second);
    }

    void drop_list::handle_clicked()
    {
        if (!view_created())
        {
            if (selection_model().has_current_index())
                iSavedSelection = presentation_model().to_item_model_index(selection_model().current_index());
            show_view();
        }
        else
            hide_view();
    }

    void drop_list::handle_cancel_selection(bool aRestoreSavedSelection, bool aUpdateEditor)
    {
        neolib::scoped_flag sf{ iCancellingSelection };

        if (view_created())
            hide_view();

        presentation_model().reset_filter();

        if (aRestoreSavedSelection)
        {
            if (iSavedSelection != std::nullopt)
                selection_model().set_current_index(presentation_model().from_item_model_index(*iSavedSelection));
            else
                selection_model().unset_current_index();
        }
        else
        {
            selection_model().unset_current_index();
            if (iSelection != std::nullopt)
            {
                iSelection = std::nullopt;
                SelectionChanged.async_trigger(iSelection);
            }
        }
        iSavedSelection = std::nullopt;

        if (!editable() && aUpdateEditor)
        {
            texture image;
            std::string text;
            if (iSelection)
            {
                auto const& maybeImage = presentation_model().cell_image(presentation_model().from_item_model_index(*iSelection));
                if (maybeImage != std::nullopt)
                    image = *maybeImage;
                text = model().cell_data(*iSelection).to_string();
            }
            input_widget().set_spacing(presentation_model().cell_spacing(*this));
            input_widget().set_image(image);
            input_widget().set_text(text);
        }
    }

    bool drop_list::handle_proxy_key_event(const neogfx::keyboard_event& aEvent)
    {
        if (presentation_model().rows() == 0)
            return false;
        if (aEvent.type() == keyboard_event_type::KeyPressed)
        {
            auto delegate_to_proxy = [this](const neogfx::keyboard_event& aEvent)
            {
                view().key_pressed(aEvent.scan_code(), aEvent.key_code(), KeyModifier_NONE);
                if (view().selection_model().has_current_index())
                    accept_selection();
            };
            switch (aEvent.scan_code())
            {
            case ScanCode_DOWN:
                if (!view_created() && (aEvent.key_modifiers() & KeyModifier_ALT) != KeyModifier_NONE)
                {
                    if (selection_model().has_current_index())
                        iSavedSelection = presentation_model().to_item_model_index(selection_model().current_index());
                    show_view();
                    return true;
                }
                if (view_created())
                    delegate_to_proxy(aEvent);
                else if (has_selection() && presentation_model().from_item_model_index(selection()).row() < presentation_model().rows() - 1)
                {
                    selection_model().set_current_index(presentation_model().from_item_model_index(selection()) += item_presentation_model_index{ 1u, 0u });
                    accept_selection();
                }
                else if (!has_selection())
                {
                    selection_model().set_current_index(item_presentation_model_index{ 0u, 0u });
                    accept_selection();
                }
                return true;
            case ScanCode_UP:
                if (view_created())
                    delegate_to_proxy(aEvent);
                else if (has_selection() && presentation_model().from_item_model_index(selection()).row() > 0u)
                {
                    selection_model().set_current_index(presentation_model().from_item_model_index(selection()) -= item_presentation_model_index{ 1u, 0u });
                    accept_selection();
                }
                else if (!has_selection())
                {
                    selection_model().set_current_index(item_presentation_model_index{ 0u, 0u });
                    accept_selection();
                }
                return true;
            case ScanCode_PAGEUP:
                if (view_created())
                    delegate_to_proxy(aEvent);
                else
                    return false;
                return true;
            case ScanCode_PAGEDOWN:
                if (view_created())
                    delegate_to_proxy(aEvent);
                else
                    return false;
                return true;
            case ScanCode_HOME:
            case ScanCode_END:
                if ((iStyle & style::Editable) != style::Editable || (aEvent.key_modifiers() & KeyModifier_ALT) != KeyModifier_NONE)
                {
                    if (view_created())
                        delegate_to_proxy(aEvent);
                    else
                    {
                        selection_model().set_current_index(aEvent.scan_code() == ScanCode_HOME ? item_presentation_model_index{ 0u, 0u } : item_presentation_model_index{ presentation_model().rows() - 1u, 0u });
                        accept_selection();
                    }
                    return true;
                }
                break;
            case ScanCode_RETURN:
                if (view_created())
                {
                    accept_selection();
                    return true;
                }
                return false;
            case ScanCode_ESCAPE:
                if (has_selection())
                {
                    cancel_selection();
                    return true;
                }
                return false;
            }
        }
        return false;
    }
}