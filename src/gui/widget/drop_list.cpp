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
#include <neogfx/app/i_basic_services.hpp>
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
        iDropList{ aDropList }
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
        if (presentation_model().attached_to(*this))
        {
            presentation_model().detach();
            presentation_model().attach(ref_ptr<i_widget>{ iDropList });
        }
    }

    void drop_list_view::items_filtered()
    {
        list_view::items_filtered();
        iDropList.iListProxy.update_view_placement();
    }

    void drop_list_view::current_index_changed(const optional_item_presentation_model_index& aCurrentIndex, const optional_item_presentation_model_index& aPreviousIndex)
    {
        list_view::current_index_changed(aCurrentIndex, aPreviousIndex);
    }

    void drop_list_view::mouse_button_released(mouse_button aButton, const point& aPosition)
    {
        bool const wasCapturing = capturing();
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

    bool drop_list_view::mouse_wheel_scrolled(mouse_wheel aWheel, const point& aPosition, delta aDelta, key_modifiers_e aKeyModifiers)
    {
        bool result = list_view::mouse_wheel_scrolled(aWheel, aPosition, aDelta, aKeyModifiers);
        return result || !iDropList.list_always_visible();
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
        case ScanCode_KEYPAD_ENTER:
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
        if (service<i_mouse>().is_mouse_grabbed_by(list_view()))
            service<i_mouse>().ungrab_mouse(list_view());
    }

    const drop_list_view& drop_list_popup::list_view() const
    {
        return iView;
    }

    drop_list_view& drop_list_popup::list_view()
    {
        return iView;
    }

    color drop_list_popup::frame_color() const
    {
        if (has_frame_color())
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
            if (!service<i_keyboard>().is_keyboard_grabbed_by(list_view()))
                service<i_keyboard>().grab_keyboard(list_view());
            if (!service<i_mouse>().is_mouse_grabbed_by(list_view()))
                service<i_mouse>().grab_mouse(list_view());
        }
        else if (!aVisible)
        {
            if (service<i_keyboard>().is_keyboard_grabbed_by(list_view()))
                service<i_keyboard>().ungrab_keyboard(list_view());
            if (service<i_mouse>().is_mouse_grabbed_by(list_view()))
                service<i_mouse>().ungrab_mouse(list_view());
        }
        return window::show(aVisible);
    }

    size_policy drop_list_popup::size_policy() const
    {
        if (has_size_policy())
            return window::size_policy();
        else if (has_fixed_size())
            return size_constraint::Fixed;
        else
            return size_constraint::Minimum;
    }

    bool drop_list_popup::has_ideal_size() const noexcept
    {
        return true;
    }

    size drop_list_popup::ideal_size(optional_size const&) const
    {
        auto totalArea = iView.total_item_area(*this);
        auto idealSize = internal_spacing().size() + totalArea + iView.internal_spacing().size();
        idealSize.cy = std::min(idealSize.cy, iDropList.root().as_widget().extents().cy / 2.0);
        if (idealSize.cy - (effective_frame_width() * 2.0 + internal_spacing().size().cy) < totalArea.cy)
            idealSize.cx += vertical_scrollbar().width();
        return idealSize.max(iDropList.minimum_size());
    }

    size drop_list_popup::minimum_size(optional_size const&) const
    {
        if (window::has_minimum_size())
            return window::minimum_size();
        auto result = ideal_size().max(iDropList.minimum_size());
        if (iDropList.size_policy().horizontal_constraint() == size_constraint::Fixed)
            result.cx = std::max(result.cx, iDropList.fixed_size().cx);
        return result;
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
        if (service<i_keyboard>().is_keyboard_grabbed_by(list_view()))
            service<i_keyboard>().ungrab_keyboard(list_view());
        close();
        // 'this' will be destroyed at this point...
        if (dropListForCancellation)
            dropListForCancellation->cancel_and_restore_selection();
    }

    void drop_list_popup::update_placement()
    {
        set_ready_to_render(false);

        list_view().set_padding(iDropList.input_widget().as_widget().internal_spacing() - list_view().presentation_model().cell_padding(*this) - list_view().effective_frame_width());

        // First stab at sizing ourselves...
        resize(minimum_size());
        
        // Workout ideal position whereby text for current item in drop list popup list_view is at same position as text in drop button or line edit...
        point currentItemPos;
        currentItemPos += list_view().presentation_model().cell_padding(*this).top_left();
        currentItemPos += list_view().presentation_model().cell_spacing(*this) / 2.0;
        currentItemPos -= point{ effective_frame_width(), effective_frame_width() };
        if (list_view().presentation_model().rows() > 0 && list_view().presentation_model().columns() > 0)
        {
            auto index = (list_view().selection_model().has_current_index() ?
                list_view().selection_model().current_index() :
                item_presentation_model_index{ 0, 0 });
            list_view().make_visible(index);
            currentItemPos += list_view().cell_rect(index).top_left();
            auto const& maybeCellImageSize = list_view().presentation_model().cell_image_size(index);
            if (maybeCellImageSize != std::nullopt)
                currentItemPos.x += (maybeCellImageSize->cx + list_view().presentation_model().cell_spacing(*this).cx);
        }
        point inputWidgetPos{ iDropList.non_client_rect().top_left() + iDropList.root().window_position() };
        point textWidgetPos{ iDropList.input_widget().text_widget().non_client_rect().top_left() + iDropList.input_widget().text_widget().internal_spacing().top_left() + iDropList.root().window_position() };
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

        set_ready_to_render(true);
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

    drop_list::list_proxy::~list_proxy()
    {
    }

    bool drop_list::list_proxy::view_created() const
    {
        return iPopup != std::nullopt || iView != std::nullopt;
    }

    bool drop_list::list_proxy::view_visible() const
    {
        return (iPopup != std::nullopt && iPopup->visible()) || (iView != std::nullopt && iView->visible());
    }

    drop_list_view& drop_list::list_proxy::list_view() const
    {
        if (iPopup != std::nullopt)
            return iPopup->list_view();
        else if (iView != std::nullopt)
            return *iView;
        throw no_view();
    }

    dimension drop_list::list_proxy::effective_frame_width() const
    {
        if (iViewContainer != std::nullopt)
            return iViewContainer->effective_frame_width();
        else
            return 0.0;
    }

    void drop_list::list_proxy::show_view()
    {
        if (iDropList.model().empty() && !iDropList.editable())
            return;
        if (!view_created())
        {
            if (iDropList.list_always_visible())
            {
                iViewContainer.emplace(iDropList.iLayout0);
                iViewContainer->set_padding(neogfx::padding{});
                iViewContainer->set_layout(make_ref<vertical_layout>());
                iViewContainer->layout().set_padding(neogfx::padding{});
                iView.emplace(iViewContainer->layout(), iDropList);
            }
            else if (!iDropList.model().empty())
            {
                iPopup.emplace(iDropList);
                iSink = iPopup->Closed([this]()
                {
                    iPopup = std::nullopt;
                    iSink.clear();
                });
                update_view_placement();
            }
        }
        else
        {
            if (iPopup != std::nullopt)
            {
                update_view_placement();
                iPopup->show();
            }
            else if (iViewContainer != std::nullopt)
                iViewContainer->show();
        }
    }

    void drop_list::list_proxy::hide_view()
    {
        if (iPopup != std::nullopt)
            iPopup->hide();
        else if (iViewContainer != std::nullopt)
            iViewContainer->hide();
    }

    void drop_list::list_proxy::close_view()
    {
        hide_view();
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
                image_widget().set_dpi_auto_scale(false);
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
            const neogfx::image_widget& image_widget() const override
            {
                return push_button::image_widget();
            }
            neogfx::image_widget& image_widget() override
            {
                return push_button::image_widget();
            }
            const neogfx::text_widget& text_widget() const override
            {
                return push_button::text_widget();
            }
            neogfx::text_widget& text_widget() override
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
            i_string const& text() const override
            {
                return push_button::text();
            }
            void set_text(i_string const& aText) override
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
                base_type{ aLayout, frame_style::SolidFrame, 2.0 },
                iLayout{ *this },
                iImage{ iLayout },
                iEditor{ iLayout, frame_style::NoFrame }
            {
                set_padding(neogfx::padding{});
                image_widget().set_padding(neogfx::padding{});
                text_widget().set_padding(neogfx::padding{});
                iImage.hide();
                set_background_opacity(1.0);
                iEditor.focus_event([&](neogfx::focus_event, focus_reason) { update(true); });
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
            i_string const& text() const override
            {
                return iEditor.text();
            }
            void set_text(i_string const& aText) override
            {
                iEditor.set_text(aText);
            }
        protected:
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

    drop_list::drop_list(drop_list_style aStyle) :
        iStyle { aStyle },
        iLayout0{ *this },
        iLayout1{ iLayout0 },
        iDownArrow{ texture{} }, 
        iListProxy{ *this },
        iChangingText{ false },
        iHandlingTextChange{ false },
        iAcceptingSelection{ false },
        iCancellingSelection{ false }
    {
        init();
    }

    drop_list::drop_list(i_widget& aParent, drop_list_style aStyle) :
        widget{ aParent },
        iStyle{ aStyle },
        iLayout0{ *this },
        iLayout1{ iLayout0 },
        iDownArrow{ texture{} },
        iListProxy{ *this },
        iChangingText{ false },
        iHandlingTextChange{ false },
        iAcceptingSelection{ false },
        iCancellingSelection{ false }
    {
        init();
    }

    drop_list::drop_list(i_layout& aLayout, drop_list_style aStyle) :
        widget{ aLayout },
        iStyle{ aStyle },
        iLayout0{ *this },
        iLayout1{ iLayout0 },
        iDownArrow{ texture{} },
        iListProxy{ *this },
        iChangingText{ false },
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
        set_model(ref_ptr<i_item_model>{ref_ptr<i_item_model>{}, &aModel});
    }

    void drop_list::set_model(i_ref_ptr<i_item_model> const& aModel)
    {
        if (iModel == aModel)
            return;

        iModel = aModel;

        if (has_model())
        {
            if (has_presentation_model())
            {
                try
                {
                    presentation_model().set_item_model(model());
                }
                catch (i_item_presentation_model::unrelated_item_model)
                {
                    set_presentation_model(ref_ptr<i_item_presentation_model>{});
                }
            }

            iSink += model().item_removed([&](item_model_index const& aItem)
            {
                if (iSavedSelection == aItem)
                    iSavedSelection = std::nullopt;
                if (has_selection() && selection() == aItem)
                    cancel_and_restore_selection(true);
                if (model().empty())
                {
                    model().item_removed().accept();
                    close_view();
                }
            });
        }
        
        if (view_created())
            list_view().set_model(aModel);

        update_layout();
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
        set_presentation_model(ref_ptr<i_item_presentation_model>{ref_ptr<i_item_presentation_model>{}, &aPresentationModel});
    }

    void drop_list::set_presentation_model(i_ref_ptr<i_item_presentation_model> const& aPresentationModel)
    {
        if (iPresentationModel == aPresentationModel)
            return;
        if (has_presentation_model() && presentation_model().attached())
            presentation_model().detach();
        iPresentationModel = aPresentationModel;
        if (has_presentation_model() && has_model())
            presentation_model().set_item_model(model());
        if (has_presentation_model() && has_selection_model())
            selection_model().set_presentation_model(*aPresentationModel);
        if (has_presentation_model())
            presentation_model().attach(ref_ptr<i_widget>{ *this });
        if (view_created())
            list_view().set_presentation_model(aPresentationModel);

        iSelectionSink = selection_model().current_index_changed([this](const optional_item_presentation_model_index& aCurrentIndex, const optional_item_presentation_model_index& /* aPreviousIndex */)
        {
            if (!presentation_model().filtering() && !handling_text_change())
            {
                neolib::scoped_flag sf{ iChangingText };
                texture image;
                string text;
                if (aCurrentIndex != std::nullopt)
                {
                    auto const& maybeImage = presentation_model().cell_image(*aCurrentIndex);
                    if (maybeImage != std::nullopt)
                        image = *maybeImage;
                    text = presentation_model().cell_to_string(*aCurrentIndex);
                }
                input_widget().set_spacing(presentation_model().cell_spacing(*this));
                input_widget().set_image(image);
                input_widget().set_text(text);
            }
        });

        update_layout();
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
        set_selection_model(ref_ptr<i_item_selection_model>{ref_ptr<i_item_selection_model>{}, &aSelectionModel});
    }

    void drop_list::set_selection_model(i_ref_ptr<i_item_selection_model> const& aSelectionModel)
    {
        if (iSelectionModel == aSelectionModel)
            return;
        iSelectionModel = aSelectionModel;
        if (has_presentation_model() && has_selection_model())
            selection_model().set_presentation_model(presentation_model());
        if (view_created())
            list_view().set_selection_model(aSelectionModel);
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

    bool drop_list::input_matches_current_item() const
    {
        return selection_model().has_current_index() &&
            model().cell_data(presentation_model().to_item_model_index(
                selection_model().current_index())).to_string() == input_widget().text();
    }

    bool drop_list::input_matches_selection() const
    {
        return has_selection() && model().cell_data(selection()).to_string() == input_widget().text();
    }

    bool drop_list::view_created() const
    {
        return iListProxy.view_created();
    }

    bool drop_list::view_visible() const
    {
        return iListProxy.view_visible();
    }

    void drop_list::show_view()
    {
        iListProxy.show_view();
        if (editable() && !accepting_selection())
        {
            if (filter_enabled())
                presentation_model().filter_by(0, input_widget().text());
            else if (presentation_model().rows() > 0)
            {
                auto findResult = presentation_model().find_item(input_widget().text());
                if (findResult != std::nullopt)
                    selection_model().set_current_index(*findResult);
                else
                    selection_model().clear_current_index();
            }
        }
    }

    void drop_list::hide_view()
    {
        if (!list_always_visible())
            iListProxy.hide_view();
    }

    void drop_list::close_view()
    {
        if (!list_always_visible())
            iListProxy.close_view();
    }

    drop_list_view& drop_list::list_view() const
    {
        return iListProxy.list_view();
    }

    void drop_list::accept_selection()
    {
        neolib::scoped_flag sf{ iAcceptingSelection };

        optional_item_model_index newSelection = (selection_model().has_current_index() ?
            presentation_model().to_item_model_index(selection_model().current_index()) : optional_item_model_index{});

        hide_view();
        presentation_model().reset_filter();

        if (newSelection != std::nullopt)
        {
            auto const presentationModelIndex = presentation_model().from_item_model_index(*newSelection);
            selection_model().set_current_index(presentationModelIndex);
            if (editable())
                input_widget().set_text(string{ presentation_model().cell_to_string(presentationModelIndex) });
        }
        else
            selection_model().clear_current_index();

        if (iSavedSelection != newSelection)
        {
            iSelection = newSelection;
            SelectionChanged.async_trigger(iSelection);
        }
        iSavedSelection = std::nullopt;

        update_widgets(true);
    }

    void drop_list::cancel_selection(bool aClearInput)
    {
        handle_cancel_selection(false);
        if (aClearInput && editable())
            input_widget().set_text(string{});
    }

    void drop_list::cancel_and_restore_selection(bool aOnlyRestoreIfViewCreated)
    {
        handle_cancel_selection(aOnlyRestoreIfViewCreated ? view_created() : true);
    }

    drop_list_style drop_list::style() const
    {
        return iStyle;
    }

    void drop_list::set_style(drop_list_style aStyle)
    {
        if (iStyle != aStyle)
        {
            iStyle = aStyle;
            update_widgets();
        }
    }

    bool drop_list::editable() const
    {
        return (iStyle & drop_list_style::Editable) == drop_list_style::Editable;
    }

    void drop_list::set_editable(bool aEditable)
    {
        if (editable() != aEditable)
        {
            if (aEditable)
                iStyle = (iStyle | drop_list_style::Editable);
            else
                iStyle = (iStyle & ~drop_list_style::Editable);
            update_widgets();
        }
    }

    bool drop_list::list_always_visible() const
    {
        return (iStyle & drop_list_style::ListAlwaysVisible) == drop_list_style::ListAlwaysVisible;
    }

    void drop_list::set_list_always_visible(bool aListAlwaysVisible)
    {
        if (list_always_visible() != aListAlwaysVisible)
        {
            if (aListAlwaysVisible)
                iStyle = (iStyle | drop_list_style::ListAlwaysVisible);
            else
                iStyle = (iStyle & ~drop_list_style::ListAlwaysVisible);
            if (view_created())
                close_view();
            update_widgets();
        }
    }

    bool drop_list::filter_enabled() const
    {
        return (iStyle & drop_list_style::NoFilter) != drop_list_style::NoFilter;
    }

    void drop_list::enable_filter(bool aEnableFilter)
    {
        if (filter_enabled() != aEnableFilter)
        {
            if (!aEnableFilter)
                iStyle = (iStyle | drop_list_style::NoFilter);
            else
                iStyle = (iStyle & ~drop_list_style::NoFilter);
            // todo
        }
    }

    bool drop_list::has_input() const
    {
        return !input_widget().text().empty();
    }

    const i_drop_list_input_widget& drop_list::input_widget() const
    {
        return *iInputWidget;
    }

    i_drop_list_input_widget& drop_list::input_widget()
    {
        return *iInputWidget;
    }

    bool drop_list::changing_text() const
    {
        return iChangingText;
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
        else if (list_always_visible())
            return neogfx::size_policy{ size_constraint::Minimum, size_constraint::Expanding };
        else if (has_fixed_size())
            return size_constraint::Fixed;
        else
            return size_constraint::Minimum;
    }

    size drop_list::minimum_size(optional_size const& aAvailableSpace) const
    {
        auto minimumSize = widget::minimum_size(aAvailableSpace);
        if (widget::has_minimum_size())
            return minimumSize;
        minimumSize.cx += iListProxy.effective_frame_width() * 2.0;
        minimumSize.cy += iListProxy.effective_frame_width() * 2.0;
        if (input_widget().image_widget().visible())
            minimumSize.cx -= input_widget().image_widget().minimum_size().cx;
        if (input_widget().image_widget().visible() && input_widget().text_widget().visible())
            minimumSize.cx -= input_widget().spacing().cx;
        if (input_widget().text_widget().visible())
        {
            minimumSize.cx -= input_widget().text_widget().minimum_size().cx;
            minimumSize.cx += input_widget().text_widget().internal_spacing().size().cx;
        }
        dimension modelWidth = 0.0;
        if (has_presentation_model() && (weight().cx == 0.0 || size_policy().horizontal_constraint() == size_constraint::Minimum))
        {
            graphics_context gc{ *this, graphics_context::type::Unattached };
            modelWidth = presentation_model().column_width(0, gc);
            if (list_always_visible())
                modelWidth += list_view().vertical_scrollbar().width();
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
                aInputWidget.text_changed()();
                if (aTextWidget.has_focus() && (!view_created() || !changing_text()))
                {
                    if (!aTextWidget.text().empty())
                        show_view();
                    else
                    {
                        if (view_created())
                            hide_view();
                        cancel_and_restore_selection(true);
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
        set_selection_model(ref_ptr<i_item_selection_model>(new item_selection_model{ item_selection_mode::NoSelection }));
        set_presentation_model(ref_ptr<i_item_presentation_model>(new default_drop_list_presentation_model<>{ *this }));
        set_model(ref_ptr<i_item_model>(new item_model{}));

        set_padding(neogfx::padding{});
        iLayout0.set_padding(neogfx::padding{});
        iLayout1.set_padding(neogfx::padding{});

        update_widgets();

        iSink += service<i_app>().current_style_changed([this](style_aspect aAspect) 
        { 
            if ((aAspect & (style_aspect::Geometry | style_aspect::Font)) != style_aspect::None)
            {
                if (view_created())
                {
                    if (list_view().capturing() && selection_model().has_current_index())
                    {
                        list_view().release_capture();
                        accept_selection();
                    }
                    else
                        hide_view();
                }
                update_widgets(true);
            }
        });

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

            inputWidget.set_size_policy(neogfx::size_policy{ size_constraint::Expanding, size_constraint::Minimum });

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

            inputWidget.set_size_policy(neogfx::size_policy{ size_constraint::Expanding, size_constraint::Minimum });

            inputWidget.layout().add(iDownArrow);

            changed = true;
        }

        if (changed || aForce)
        {
            update_arrow();

            if (changed)
                input_widget().accept(*this);

            texture image;
            string text;
            if (selection_model().has_current_index())
            {
                auto const& maybeImage = presentation_model().cell_image(selection_model().current_index());
                if (maybeImage != std::nullopt)
                    image = *maybeImage;
                text = presentation_model().cell_to_string(selection_model().current_index());
            }
            input_widget().set_spacing(presentation_model().cell_spacing(*this));
            input_widget().set_image(image);
            if (changed)
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

        if (iListProxy.view_created())
            iListProxy.update_view_placement();
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
            iDownArrowTexture.emplace(ink,
                image{
                    dpi_select("neogfx::drop_list::iDownArrowTexture::"s, "neogfx::drop_list::iDownArrowHighDpiTexture::"s) + ink.to_string(),
                    dpi_select(sDownArrowImagePattern, sDownArrowHighDpiImagePattern), { { "paper", color{} },{ "ink", ink } }, dpi_select(1.0, 2.0) });
        }
        iDownArrow.set_image(iDownArrowTexture->second);
    }

    void drop_list::handle_clicked()
    {
        if (!view_visible())
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

        auto const previousSelection = iSelection;
        if (aRestoreSavedSelection && iSavedSelection != std::nullopt)
        {
            iSelection = iSavedSelection;
            iSavedSelection = std::nullopt;
            selection_model().set_current_index(presentation_model().from_item_model_index(*iSelection));
        }
        else
        {
            iSelection = std::nullopt;
            selection_model().clear_current_index();
        }
        if (iSelection != previousSelection)
            SelectionChanged.async_trigger(iSelection);

        if (!editable() && aUpdateEditor)
        {
            texture image;
            string text;
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
                list_view().key_pressed(aEvent.scan_code(), aEvent.key_code(), KeyModifier_NONE);
                if (list_view().selection_model().has_current_index())
                    accept_selection();
            };
            switch (aEvent.scan_code())
            {
            case ScanCode_DOWN:
                if (view_created())
                    delegate_to_proxy(aEvent);
                else if (!view_created() && (aEvent.key_modifiers() & KeyModifier_ALT) != KeyModifier_NONE)
                {
                    if (selection_model().has_current_index())
                        iSavedSelection = presentation_model().to_item_model_index(selection_model().current_index());
                    show_view();
                }
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
                if ((iStyle & drop_list_style::Editable) != drop_list_style::Editable || (aEvent.key_modifiers() & KeyModifier_ALT) != KeyModifier_NONE)
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
            case ScanCode_KEYPAD_ENTER:
                if (view_created())
                {
                    accept_selection();
                    return true;
                }
                return false;
            case ScanCode_ESCAPE:
                if (has_selection())
                {
                    cancel_and_restore_selection(true);
                    return true;
                }
                return false;
            }
        }
        return false;
    }
}