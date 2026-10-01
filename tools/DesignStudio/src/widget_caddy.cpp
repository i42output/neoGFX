    // widget_caddy.cpp
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

#include <neogfx/tools/DesignStudio/DesignStudio.hpp>
#include <cmath>
#include <numbers>
#include <neogfx/app/i_app.hpp>
#include <neogfx/app/settings.hpp>
#include <neogfx/gui/window/i_window.hpp>
#include <neogfx/gui/widget/line_edit.hpp>
#include <neolib/task/event.hpp>
#include <neogfx/gui/widget/widget.ipp>
#include <neogfx/gui/window/context_menu.hpp>
#include <neogfx/tools/DesignStudio/i_element_library.hpp>
#include "widget_caddy.hpp"

namespace neogfx
{
    template class widget<DesignStudio::i_element_caddy>;
}

namespace neogfx::DesignStudio
{
    namespace
    {
        bool sPreviewMode = false;
        neolib::event<> sPreviewModeChanged;
        bool sDisplayIds = false;
        neolib::event<> sDisplayIdsChanged;
        bool sDesignDragActive = false;
        neolib::event<> sDesignDragActiveChanged;
    }

    bool design_drag_active()
    {
        return sDesignDragActive;
    }

    void set_design_drag_active(bool aActive)
    {
        if (sDesignDragActive != aActive)
        {
            sDesignDragActive = aActive;
            sDesignDragActiveChanged();
        }
    }

    neolib::i_event<> const& design_drag_active_changed()
    {
        return sDesignDragActiveChanged;
    }

    bool display_ids()
    {
        return sDisplayIds;
    }

    void set_display_ids(bool aDisplayIds)
    {
        if (sDisplayIds != aDisplayIds)
        {
            sDisplayIds = aDisplayIds;
            sDisplayIdsChanged();
        }
    }

    neolib::i_event<> const& display_ids_changed()
    {
        return sDisplayIdsChanged;
    }

    bool show_ids()
    {
        return sDisplayIds && !sPreviewMode;
    }

    void set_text_attribute(i_element& aElement, std::string const& aText)
    {
        std::string quoted = "\"";
        for (auto ch : aText)
            switch (ch)
            {
            case '\"': quoted += "\\\""; break;
            case '\\': quoted += "\\\\"; break;
            case '\n': quoted += "\\n"; break;
            case '\r': quoted += "\\r"; break;
            case '\t': quoted += "\\t"; break;
            default: quoted += ch; break;
            }
        quoted += "\"";
        for (auto& attribute : aElement.attributes())
            if (attribute.first().to_std_string_view() == aElement.text_attribute().to_std_string_view())
            {
                attribute.second() = string{ quoted };
                return;
            }
        aElement.attributes().push_back(neolib::pair<string, string>{ string{ aElement.text_attribute() }, string{ quoted } });
    }

    namespace
    {
        bool is_design_element(i_element const& aElement)
        {
            return aElement.group() == element_group::Widget || aElement.group() == element_group::Layout;
        }

        std::size_t element_depth(i_element const& aElement)
        {
            std::size_t result = 0;
            for (i_element const* e = &aElement; e->has_parent(); e = &e->parent())
                ++result;
            return result;
        }

        bool is_descendant_of(i_element const& aElement, i_element const& aAncestor)
        {
            for (i_element const* e = &aElement; e->has_parent(); e = &e->parent())
                if (&e->parent() == &aAncestor)
                    return true;
            return false;
        }

        rect layout_design_rect(i_layout const& aLayout)
        {
            if (!aLayout.has_parent_widget())
                return rect{};
            return rect{ aLayout.position(), aLayout.extents() } + design_rect(aLayout.parent_widget()).top_left();
        }

        struct insertion_point
        {
            layout_item_index index;
            i_widget const* next; // item to insert before (nullptr: append)
            std::pair<point, point> line; // insertion marker (design coordinates)
        };

        // where an item dropped at aDropPosition (design coordinates) would be inserted; aExclude is the item being moved (if any)
        insertion_point find_insertion_point(i_layout const& aLayout, point const& aDropPosition, i_widget const* aExclude = nullptr)
        {
            bool const horizontal = (aLayout.direction() == layout_direction::Horizontal);
            std::vector<std::pair<layout_item_index, rect>> items;
            for (layout_item_index i = 0; i < aLayout.count(); ++i)
                if (aLayout.is_widget_at(i) && &aLayout.get_widget_at(i) != aExclude && !aLayout.get_widget_at(i).effectively_hidden())
                    items.emplace_back(i, design_rect(aLayout.get_widget_at(i)));
            std::size_t nextItem = items.size();
            for (std::size_t k = 0; k < items.size(); ++k)
                if (horizontal ? aDropPosition.x < items[k].second.center().x : aDropPosition.y < items[k].second.center().y)
                {
                    nextItem = k;
                    break;
                }
            insertion_point result{ aLayout.count(), nullptr, {} };
            if (nextItem < items.size())
            {
                result.index = items[nextItem].first;
                result.next = &aLayout.get_widget_at(result.index);
            }
            auto const layoutRect = layout_design_rect(aLayout);
            auto const spacing = aLayout.spacing();
            if (horizontal)
            {
                scalar x = layoutRect.left();
                if (!items.empty())
                {
                    if (nextItem == 0u)
                        x = items.front().second.left() - spacing.cx / 2.0;
                    else if (nextItem == items.size())
                        x = items.back().second.right() + spacing.cx / 2.0;
                    else
                        x = (items[nextItem - 1u].second.right() + items[nextItem].second.left()) / 2.0;
                }
                x = std::clamp(x, layoutRect.left(), std::max(layoutRect.left(), layoutRect.right() - 1.0));
                result.line = { point{ x, layoutRect.top() }, point{ x, layoutRect.bottom() } };
            }
            else
            {
                scalar y = layoutRect.top();
                if (!items.empty())
                {
                    if (nextItem == 0u)
                        y = items.front().second.top() - spacing.cy / 2.0;
                    else if (nextItem == items.size())
                        y = items.back().second.bottom() + spacing.cy / 2.0;
                    else
                        y = (items[nextItem - 1u].second.bottom() + items[nextItem].second.top()) / 2.0;
                }
                y = std::clamp(y, layoutRect.top(), std::max(layoutRect.top(), layoutRect.bottom() - 1.0));
                result.line = { point{ layoutRect.left(), y }, point{ layoutRect.right(), y } };
            }
            return result;
        }

        // keep element order (which is the order saved to .nrc) in step with layout order
        void sync_element_order(i_element& aElement, i_layout& aLayout)
        {
            if (!aElement.has_caddy() || !aElement.has_parent())
                return;
            auto const index = aLayout.find(aElement.caddy());
            if (!index || *index + 1u >= aLayout.count() || !aLayout.is_widget_at(*index + 1u))
                return;
            i_widget const& nextWidget = aLayout.get_widget_at(*index + 1u);
            auto& siblings = aElement.parent().children();
            auto self = std::find_if(siblings.begin(), siblings.end(), [&](auto const& e) { return &*e == &aElement; });
            if (self == siblings.end())
                return;
            ref_ptr<i_element> keep{ &aElement };
            siblings.erase(self);
            auto next = std::find_if(siblings.begin(), siblings.end(), [&](auto const& e)
            {
                return e->has_caddy() && static_cast<i_widget const*>(&e->caddy()) == &nextWidget;
            });
            siblings.insert(next, keep);
        }

        // grow top level caddy if its content no longer fits
        void grow_top_level_caddy(i_element& aElement)
        {
            i_element* top = &aElement;
            while (top->is_nested())
                top = &top->parent();
            if (top->has_caddy())
            {
                auto& topCaddy = top->caddy();
                auto const minimumSize = topCaddy.minimum_size();
                auto const currentSize = topCaddy.extents();
                if (currentSize.cx < minimumSize.cx || currentSize.cy < minimumSize.cy)
                    topCaddy.resize(size{ std::max(currentSize.cx, minimumSize.cx), std::max(currentSize.cy, minimumSize.cy) });
            }
        }

        // place an in-place text editor (a child of aHost) exactly over aTextArea (its text widget is kept showing the edited text so
        // it is the right size, including its lines); a little wider for the cursor
        void position_text_editor(i_widget& aEditor, i_widget const& aHost, i_widget const& aTextArea)
        {
            auto const textRect = design_rect(aTextArea);
            scalar const cursorRoom = 2.0_dip;
            size const editorSize{ textRect.cx + cursorRoom, textRect.cy };
            aEditor.move(point{ textRect.x, textRect.center().y - editorSize.cy / 2.0 } - design_rect(aHost).top_left());
            aEditor.resize(editorSize);
        }

        thread_local std::optional<std::pair<point, point>> tDropLine; // insertion marker (drop highlight coordinates)
        thread_local std::optional<widget_timer> tDropHighlightAnimator; // repaints the drop highlight while its fill fades in and out
        thread_local std::chrono::steady_clock::time_point tDropHighlightStart;
    }

    bool preview_mode()
    {
        return sPreviewMode;
    }

    void set_preview_mode(bool aPreview)
    {
        if (sPreviewMode != aPreview)
        {
            sPreviewMode = aPreview;
            sPreviewModeChanged();
        }
    }

    neolib::i_event<> const& preview_mode_changed()
    {
        return sPreviewModeChanged;
    }

    widget_caddy::widget_caddy(i_project& aProject, i_element& aElement, i_widget& aParent, const point& aPosition) :
        widget{ aParent },
        iProject{ aProject },
        iElement{ aElement },
        iAnimator{ *this, [this](widget_timer& aAnimator) 
        {    
            aAnimator.again();
            if (iEndTextEdit)
                end_text_edit(*iEndTextEdit);
            if (iTextEditor && iTextEditor->has_parent() && iTextElement != nullptr && iTextElement->has_layout_item())
                position_text_editor(*iTextEditor, iTextEditor->parent(), iTextElement->text_area()); // keep it over the text (it may not have been laid out yet)
            if (has_element() && (element().mode() != element_mode::None || element().is_selected() || entered()))
                update(); 
        }, std::chrono::milliseconds{ 20 } }
    {
        set_minimum_size(size{ 96.0_dip, 32.0_dip });
        bring_to_front();
        move(aPosition);
        iSink = ChildAdded([&](i_widget& aChild)
        {
            // only the element's own widget ignores mouse events; nested element caddies must not
            if (has_item() && item().is_widget() && &item().as_widget() == &aChild)
            {
                // the caddy, not the element's widget, handles moving/resizing (e.g. a window's title bar and borders)
                aChild.set_ignore_mouse_events(true);
                aChild.set_ignore_non_client_mouse_events(true);
            }
        });
        iSink += ChildRemoved([&](i_widget& aChild)
        {
            if (has_item() && item().is_widget() && &item().as_widget() == &aChild)
            {
                aChild.set_ignore_mouse_events(false);
                aChild.set_ignore_non_client_mouse_events(false);
            }
        });
        ref_ptr<i_settings> settings{ aElement.library().application() };
        iShowLayoutIcons = &settings->setting("environment.workspace.show_layout_icons"_s);
        iSink += iShowLayoutIcons->changing([&]()
        {
            update();
        });
        iSink += iShowLayoutIcons->changed([&]()
        {
            update();
        });
        iSink += element().mode_changed([&]()
        {
            update();
        });
        iSink += element().selection_changed([&]()
        {
            update();
        });
        iSink += iProject.element_removed([&](i_element& aElement)
        {
            if (&aElement == iElement)
            {
                if (has_parent_layout())
                    parent_layout().remove(*this);
                if (has_parent())
                    parent().remove(*this);
            }
        });
        iItem = aElement.has_layout_item() ? aElement.layout_item() : (aElement.create_layout_item(*this), aElement.layout_item());
        // only register as the element's caddy once it has a layout item (layout_item() throws if the element type can't be created)
        element().set_caddy(*this);
        if (item().is_widget())
        {
            auto& itemAsWidget = item().as_widget();
            add(itemAsWidget);
            if (itemAsWidget.is_root())
            {
                // a window being designed (nested window) mustn't handle mouse events itself (e.g. move/resize via its 
                // title bar/borders) so filter (consume) them and handle them as the caddy's
                auto forward = [this](auto const& aEvent)
                {
                    auto const position = mouse_position();
                    auto const modifiers = service<i_keyboard>().modifiers();
                    switch (aEvent.type())
                    {
                    case mouse_event_type::ButtonClicked:
                        if (!has_focus())
                            set_focus();
                        mouse_button_clicked(aEvent.mouse_button(), position, modifiers);
                        break;
                    case mouse_event_type::ButtonDoubleClicked:
                        mouse_button_double_clicked(aEvent.mouse_button(), position, modifiers);
                        break;
                    case mouse_event_type::ButtonReleased:
                        mouse_button_released(aEvent.mouse_button(), position);
                        break;
                    case mouse_event_type::Moved:
                        mouse_moved(position, modifiers);
                        break;
                    default:
                        break;
                    }
                };
                iSink += itemAsWidget.mouse_event([this, forward](const neogfx::mouse_event& aEvent)
                {
                    auto& window = item().as_widget();
                    if (preview_mode())
                    {
                        // previewing: the window's contents are live but its frame (title bar, borders etc.) still belongs 
                        // to the caddy (so dragging it moves the caddy, not the nested window)
                        auto const part = window.part(window.mouse_position()).part;
                        bool const framePart = (part >= widget_part::TitleBar && part <= widget_part::SystemMenu);
                        if (!framePart && !capturing())
                            return;
                    }
                    window.mouse_event().accept();
                    forward(aEvent);
                });
                iSink += itemAsWidget.non_client_mouse_event([this, forward](const neogfx::non_client_mouse_event& aEvent)
                {
                    item().as_widget().non_client_mouse_event().accept();
                    forward(aEvent);
                });
                // title bar buttons (close etc.) mustn't work: title bar clicks go to (and are filtered by) the window
                i_standard_layout_container& window = static_cast<i_window&>(itemAsWidget);
                if (window.has_layout(standard_layout::TitleBar))
                {
                    try
                    {
                        window.title_bar().set_ignore_mouse_events(true);
                    }
                    catch (...) {}
                }
            }
        }
        else if (item().is_layout())
            set_layout(item().as_layout());
        else
            item().set_parent_widget(this);
        if (nested())
        {
            // nested caddies live inside another element's widget/layout which ignores mouse events
            set_ignore_mouse_events(false);
            set_consider_ancestors_for_mouse_events(false);
        }
        iSink += preview_mode_changed()([this]()
        {
            apply_preview_mode();
        });
        iSink += display_ids_changed()([this]()
        {
            apply_preview_mode();
        });
        iSink += design_drag_active_changed()([this]()
        {
            // layouts only have editor padding, spacing, guidelines and icons while dragging (to show where things can be dropped)
            if (has_item() && (nested() || item().is_layout() || item().is_spacer()))
            {
                update_layout();
                update();
            }
            else if (has_item() && !nested())
            {
                // a top level element grows (centred on its centre) while dragging if its content (with editor padding/spacing) no 
                // longer fits and shrinks back afterwards (but not below what its content then needs, e.g. after something was dropped in it)
                auto const centre = position() + point{ extents() / 2.0 };
                size newSize;
                if (design_drag_active())
                {
                    iPreDragSize = extents();
                    newSize = extents().max(minimum_size());
                }
                else if (iPreDragSize)
                {
                    newSize = iPreDragSize->max(minimum_size());
                    iPreDragSize = std::nullopt;
                }
                else
                    return;
                if (newSize != extents())
                {
                    move(centre - point{ newSize / 2.0 });
                    resize(newSize);
                }
            }
        });
        apply_preview_mode();
    }

    void widget_caddy::apply_preview_mode()
    {
        bool const preview = preview_mode();
        if (preview && has_element())
        {
            element().set_mode(element_mode::None);
            element().select(false, false);
            iDragInfo = std::nullopt;
            iDropTarget = nullptr;
            hide_drop_highlight(iDropHighlight);
        }
        if (has_item() && item().is_widget())
        {
            // previewing: the element's widget handles mouse events as in a running application (the caddy ignores them)
            auto& itemWidget = item().as_widget();
            itemWidget.set_ignore_mouse_events(!preview);
            itemWidget.set_consider_ancestors_for_mouse_events(!preview);
            if (!itemWidget.is_root())
                itemWidget.set_ignore_non_client_mouse_events(!preview);
        }
        if (has_element())
        {
            // text (as in a running application) or ids
            element().apply_attributes(show_ids());
            for (auto& child : element().children())
                if (!child->needs_caddy())
                    child->apply_attributes(show_ids()); // e.g. tab pages (which have no caddy of their own)
        }
        if (preview)
            iEndTextEdit = false;
        if (preview && has_focus())
            release_focus();
        update();
    }

    void widget_caddy::begin_text_edit()
    {
        if (has_element())
            begin_text_edit(element());
    }

    void widget_caddy::begin_text_edit(i_element& aElement)
    {
        if (preview_mode() || !aElement.has_layout_item() || !aElement.layout_item().is_widget() || !aElement.has_text())
            return;
        end_text_edit(false);
        iTextElement = &aElement;
        // in-place editor in place of the element's text widget: frameless and in the same font so the element still looks as it is;
        // it is a sibling of the text widget (so it is in the same window, e.g. a title bar of a window being designed)
        auto& textArea = aElement.text_area();
        i_widget& host = textArea.has_parent() && &textArea != &aElement.layout_item().as_widget() ? textArea.parent() : static_cast<i_widget&>(*this);
        auto editor = make_ref<text_edit>(host, text_edit_caps::MultiLine, frame_style::NoFrame); // multi-line: Shift+Return inserts a new line
        editor->set_consider_ancestors_for_mouse_events(false); // the element's widgets ignore mouse events (the caddy handles them)
        editor->set_font(textArea.font());
        editor->set_padding(neogfx::padding{}); // so its text is where the text widget's text is
        editor->set_alignment(aElement.text_alignment()); // e.g. a button's text is centred within its text widget
        // Return (and Escape) must reach the editor's keyboard event handler below rather than be left for the window
        editor->set_focus_policy(editor->focus_policy() | neogfx::focus_policy::ConsumeReturnKey | neogfx::focus_policy::ConsumeEscapeKey);
        // hide the text being edited (without affecting layout) so only the editor's text is seen
        iTextAreaOpacity = textArea.opacity();
        textArea.set_opacity(0.0);
        iTextEditor = ref_ptr<i_widget>{ editor };
        string text;
        for (auto const& attribute : aElement.attributes())
            if (attribute.first().to_std_string_view() == aElement.text_attribute().to_std_string_view())
                text = attribute.second();
        auto const quotedText = text.to_std_string();
        std::string plainText = quotedText;
        if (quotedText.size() >= 2u && quotedText.front() == '"' && quotedText.back() == '"')
        {
            plainText.clear();
            for (std::size_t i = 1u; i + 1u < quotedText.size(); ++i)
            {
                char ch = quotedText[i];
                if (ch == '\\' && i + 2u < quotedText.size())
                {
                    ch = quotedText[++i];
                    if (ch == 'n')
                        ch = '\n';
                    else if (ch == 't')
                        ch = '\t';
                    else if (ch == 'r')
                        ch = '\r';
                }
                plainText += ch;
            }
        }
        editor->set_text(string{ plainText });
        position_text_editor(*editor, host, textArea);
        editor->bring_to_front();
        editor->keyboard_event([this, &editorRef = *editor](const neogfx::keyboard_event& aEvent)
        {
            // Return commits; Shift+Return is left to the editor (a new line)
            bool const shift = (service<i_keyboard>().modifiers() & key_modifier::SHIFT) != key_modifier::None;
            if (aEvent.type() == keyboard_event_type::TextInput)
            {
                auto const text = aEvent.text();
                if (!shift && (text.to_std_string() == "\r" || text.to_std_string() == "\n"))
                    editorRef.keyboard_event().accept(); // not a new line
                return;
            }
            if (aEvent.type() != keyboard_event_type::KeyPressed)
                return;
            if ((aEvent.scan_code() == ScanCode_RETURN || aEvent.scan_code() == ScanCode_KEYPAD_ENTER) && !shift)
            {
                iEndTextEdit = true;
                editorRef.keyboard_event().accept();
            }
            else if (aEvent.scan_code() == ScanCode_ESCAPE)
            {
                iEndTextEdit = false;
                editorRef.keyboard_event().accept();
            }
        });
        editor->TextChanged([this, &editorRef = *editor]()
        {
            // keep the element's (transparent) text widget showing the text being edited so the element and its layout follow it;
            // the element's text attribute itself only changes when the edit is committed
            if (iTextElement == nullptr || !iTextElement->has_layout_item())
                return;
            auto& textElement = *iTextElement;
            auto& attributes = textElement.attributes();
            auto existing = std::find_if(attributes.begin(), attributes.end(), [&](auto const& attribute) { return attribute.first().to_std_string_view() == textElement.text_attribute().to_std_string_view(); });
            std::optional<string> const previous = existing != attributes.end() ? std::optional<string>{ existing->second() } : std::nullopt;
            set_text_attribute(textElement, editorRef.text().to_std_string());
            textElement.apply_attributes(false);
            for (auto attribute = attributes.begin(); attribute != attributes.end(); ++attribute)
                if (attribute->first().to_std_string_view() == textElement.text_attribute().to_std_string_view())
                {
                    if (previous)
                        attribute->second() = *previous;
                    else
                        attributes.erase(attribute);
                    break;
                }
        });
        editor->focus_event([this](neogfx::focus_event aEvent, focus_reason)
        {
            if (aEvent == neogfx::focus_event::FocusLost && !iEndTextEdit)
                iEndTextEdit = true;
        });
        // a window being designed is a nested window which is only activated when it gains focus but the caddy filters its mouse events 
        // so it never does; it must be active for its focused widget (the editor) to get keyboard input and show a cursor
        auto& editorRoot = editor->root();
        if (editorRoot.is_nested() && !editorRoot.is_active())
            editorRoot.activate();
        editor->set_focus();
        editor->select_all();
    }

    void widget_caddy::end_text_edit(bool aCommit)
    {
        iEndTextEdit = std::nullopt;
        if (!iTextEditor)
            return;
        auto editor = iTextEditor;
        iTextEditor = {};
        auto textElement = iTextElement;
        iTextElement = nullptr;
        if (textElement != nullptr && textElement->has_layout_item())
        {
            if (iTextAreaOpacity)
                textElement->text_area().set_opacity(*iTextAreaOpacity);
            if (aCommit)
            {
                set_text_attribute(*textElement, static_cast<text_edit&>(*editor).text().to_std_string());
                iProject.set_dirty();
            }
            textElement->apply_attributes(show_ids()); // committed text, or undo the text shown while editing
        }
        iTextAreaOpacity = std::nullopt;
        if (editor->has_parent())
            editor->parent().remove(*editor);
    }
    
    widget_caddy::~widget_caddy()
    {
        if (iTextEditor && iTextEditor->has_parent())
            iTextEditor->parent().remove(*iTextEditor); // the in-place editor may be a child of one of the element's widgets
        end_rubber_band();
        if (service<i_clipboard>().sink_active() && &service<i_clipboard>().active_sink() == this)
            service<i_clipboard>().deactivate(*this);
        if (has_item())
        {
            if (item().is_widget())
                remove(item().as_widget());
            else
            {
                item().set_parent_widget(nullptr);
                if (item().is_layout() && has_layout())
                    set_layout(ref_ptr<i_layout>{});
            }
        }
    }

    bool widget_caddy::has_element() const
    {
        return iElement.valid();
    }

    i_element& widget_caddy::element() const
    {
        return *iElement;
    }

    bool widget_caddy::has_item() const
    {
        return iItem.valid();
    }

    i_layout_item& widget_caddy::item() const
    {
        return *iItem;
    }

    bool widget_caddy::nested() const
    {
        return has_element() && element().is_nested();
    }

    neogfx::size_policy widget_caddy::size_policy() const
    {
        if (nested() && has_item())
            return item().size_policy();
        return widget::size_policy();
    }

    size widget_caddy::minimum_size(optional_size const& aAvailableSpace) const
    {
        size result = item().minimum_size(aAvailableSpace != std::nullopt ? *aAvailableSpace - internal_spacing().size() : aAvailableSpace);
        if (result.cx != 0.0)
            result.cx += internal_spacing().size().cx;
        if (result.cy != 0.0)
            result.cy += internal_spacing().size().cy;
        return result != size{} ? result : widget::minimum_size(aAvailableSpace);
    }

    neogfx::widget_type widget_caddy::widget_type() const
    {
        // top level caddies float on the workspace; nested caddies are managed by their container's layout
        if (nested())
            return widget::widget_type();
        return widget::widget_type() | neogfx::widget_type::Floating;
    }

    neogfx::padding widget_caddy::padding() const
    {
        // a layout's own padding and spacing (as specified in its properties) are all it has except while dragging when it gets editor 
        // padding and spacing too (the latter being the padding of the caddies of the items within it)
        if (has_item() && (nested() || item().is_layout() || item().is_spacer()) && !design_drag_active())
            return neogfx::padding{};
        return neogfx::padding{ 4.0_dip };
    }

    void widget_caddy::layout_items(bool aDefer)
    {
        widget::layout_items(aDefer);
        if (item().is_widget())
        {
            item().as_widget().move(client_rect(false).top_left());
            item().as_widget().resize(client_rect(false).extents());
        }
    }

    layer_t widget_caddy::layer() const
    {
        switch (element().mode())
        {
        case element_mode::None:
        default:
            return LayerWidget + 0;
        case element_mode::Drag:
            return LayerWidget - 1;
        case element_mode::Edit:
            return LayerWidget + 1;
        }
    }

    layer_t widget_caddy::render_layer() const
    {
        switch (element().mode())
        {
        case element_mode::None:
        default:
            return LayerWidget + 0;
        case element_mode::Drag:
        case element_mode::Edit:
            return LayerWidget + 1;
        }
    }

    void widget_caddy::paint(i_graphics_context& aGc) const
    {
        widget::paint(aGc);
        if ((item().is_layout() || item().is_spacer()) && !preview_mode() && design_drag_active())
        {
            auto const r = client_rect(false);
            if (iShowLayoutIcons != nullptr && iShowLayoutIcons->value<bool>(true))
            {
                scoped_opacity so{ aGc, aGc.opacity() * 0.25 };
                scoped_scissor ss{ aGc, r };
                size const iconSize{ std::min<scalar>(r.cx, std::min<scalar>(r.cy, 8.0_dip)), std::min<scalar>(r.cx, std::min<scalar>(r.cy, 8.0_dip)) };
                // nested layouts/spacers (higher z order) draw their own icons so don't draw ours where they intersect
                std::vector<rect> nestedRects;
                for (auto const& child : element().children())
                    if (child->has_caddy() && child->has_layout_item() && (child->layout_item().is_layout() || child->layout_item().is_spacer()) &&
                        child->caddy().has_parent() && !child->caddy().effectively_hidden())
                        nestedRects.push_back(to_client_coordinates(child->caddy().non_client_rect()));
                for (std::int32_t y = 0; y < r.height() / iconSize.cy; ++y)
                {
                    for (std::int32_t x = 0; x < r.width() / iconSize.cx; ++x)
                    {
                        if (y % 2 == 1 || x % 3 != y % 3)
                            continue;
                        rect const iconRect{ r.top_left() + point{ iconSize } / 4.0 + basic_point<std::int32_t>{ x, y }.as<scalar>() * iconSize, iconSize };
                        // clip the icon to the parts of it not covered by nested layouts/spacers
                        std::vector<rect> pieces{ iconRect.intersection(r) };
                        for (auto const& nestedRect : nestedRects)
                        {
                            std::vector<rect> remaining;
                            for (auto const& piece : pieces)
                            {
                                auto const covered = piece.intersection(nestedRect);
                                if (covered.empty())
                                {
                                    remaining.push_back(piece);
                                    continue;
                                }
                                auto add = [&](scalar aLeft, scalar aTop, scalar aRight, scalar aBottom)
                                {
                                    if (aRight > aLeft && aBottom > aTop)
                                        remaining.push_back(rect{ point{ aLeft, aTop }, size{ aRight - aLeft, aBottom - aTop } });
                                };
                                add(piece.left(), piece.top(), piece.right(), covered.top());
                                add(piece.left(), covered.bottom(), piece.right(), piece.bottom());
                                add(piece.left(), covered.top(), covered.left(), covered.bottom());
                                add(covered.right(), covered.top(), piece.right(), covered.bottom());
                            }
                            pieces = std::move(remaining);
                        }
                        for (auto const& piece : pieces)
                        {
                            if (piece.empty())
                                continue;
                            scoped_scissor pieceScissor{ aGc, piece };
                            aGc.draw_texture(iconRect, element().library().element_icon(element().type()));
                        }
                    }
                }
            }
            aGc.draw_rect(r, pen{ color::PowderBlue.lighter(0x20), line_dash{ 0xCCCC, 4.0 } });
        }
    }

    void widget_caddy::paint_non_client_after(i_graphics_context& aGc) const
    {
        widget::paint_non_client_after(aGc);
        if (opacity() == 1.0 && !preview_mode())
        {
            auto draw_selected_rect = [&]()
            {
                auto const cr = client_rect(false);
                if (element().is_selected() && element().group() != element_group::Workflow && !iTextEditor) // not while editing text so the element is seen as it is
                    aGc.fill_rect(cr, service<i_app>().current_style().palette().color(color_role::Selection).with_alpha(0.5));
                aGc.draw_rect(cr, pen{ color::White.with_alpha(0.75), 2.0 });
                aGc.draw_rect(cr, pen{ color::Black.with_alpha(0.75), 2.0, 
                    line_dash{ 0xCCCC, 2.0, (7.0 - std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count() / 100 % 8) } });
            };
            auto draw_resizer_rects = [&]()
            {
                aGc.draw_rect(cardinal_rect(cardinal::NorthWest, false), color::NavyBlue, color::White.with_alpha(0.75));
                aGc.draw_rect(cardinal_rect(cardinal::North, false), color::NavyBlue, color::White.with_alpha(0.75));
                aGc.draw_rect(cardinal_rect(cardinal::NorthEast, false), color::NavyBlue, color::White.with_alpha(0.75));
                aGc.draw_rect(cardinal_rect(cardinal::East, false), color::NavyBlue, color::White.with_alpha(0.75));
                aGc.draw_rect(cardinal_rect(cardinal::SouthEast, false), color::NavyBlue, color::White.with_alpha(0.75));
                aGc.draw_rect(cardinal_rect(cardinal::South, false), color::NavyBlue, color::White.with_alpha(0.75));
                aGc.draw_rect(cardinal_rect(cardinal::SouthWest, false), color::NavyBlue, color::White.with_alpha(0.75));
                aGc.draw_rect(cardinal_rect(cardinal::West, false), color::NavyBlue, color::White.with_alpha(0.75));
            };
            switch (element().mode())
            {
            case element_mode::None:
            default:
                if (element().is_selected() || entered())
                    draw_selected_rect();
                if (entered() && !nested())
                    draw_resizer_rects();
                break;
            case element_mode::Drag:
                draw_selected_rect();
                break;
            case element_mode::Edit:
                draw_selected_rect();
                if (!nested())
                    draw_resizer_rects();
                break;
            }
        }
    }

    focus_policy widget_caddy::focus_policy() const
    {
        return preview_mode() ? neogfx::focus_policy::NoFocus : neogfx::focus_policy::StrongFocus;
    }

    void widget_caddy::focus_gained(focus_reason aFocusReason)
    {
        widget::focus_gained(aFocusReason);
        element().set_mode(element_mode::Edit);
        service<i_clipboard>().activate(*this);
        if (element().group() == element_group::Workflow)
            bring_to_front();
    }

    void widget_caddy::focus_lost(focus_reason aFocusReason)
    {
        widget::focus_lost(aFocusReason);
        if (has_element())
            element().set_mode(element_mode::None);
        if (service<i_clipboard>().sink_active() && &service<i_clipboard>().active_sink() == this)
            service<i_clipboard>().deactivate(*this);
    }

    bool widget_caddy::key_pressed(scan_code_e aScanCode, key_code_e aKeyCode, key_modifier aKeyModifier)
    {
        widget::key_pressed(aScanCode, aKeyCode, aKeyModifier);
        if (aScanCode == ScanCode_ESCAPE)
        {
            element().root().select(false, true);
            return true;
        }
        return false;
    }

    bool widget_caddy::ignore_mouse_events(bool aConsiderAncestors) const
    {
        if (preview_mode() || element().mode() == element_mode::Drag)
            return true;
        return widget::ignore_mouse_events(aConsiderAncestors);
    }

    void widget_caddy::mouse_button_clicked(mouse_button aButton, const point& aPosition, key_modifier aKeyModifier)
    {
        widget::mouse_button_clicked(aButton, aPosition, aKeyModifier);
        if (aButton == mouse_button::Left)
        {
            bool const toggleSelect = ((aKeyModifier & key_modifier::CTRL) != key_modifier::None);
            auto clickLocation = cardinal_at(aPosition, aKeyModifier);
            if (!clickLocation)
                clickLocation = cardinal::Center;
            if (clickLocation == cardinal::Center && (aKeyModifier & key_modifier::SHIFT) != key_modifier::None && !preview_mode())
            {
                // shift+click+move: rubber band select (as on the empty canvas)
                end_rubber_band();
                element().root().select(false, true);
                element().root().visit([&](i_element& aElement) { aElement.set_mode(element_mode::None); });
                iRubberBandAnchor = design_position(aPosition);
                update_rubber_band(*iRubberBandAnchor);
                return;
            }
            if (clickLocation == cardinal::Center)
            {
                element().select(toggleSelect ? !element().is_selected() : true, !toggleSelect && element().root().selected_child_count() <= 1);
                // the clicked element is the current one (Object Explorer selects and scrolls to it, Properties shows it); this doesn't 
                // rely on the caddy gaining focus as that doesn't happen for caddies in a window being designed (which is never active)
                if (element().is_selected())
                    element().set_mode(element_mode::Edit);
            }
            if (element().is_selected() && clickLocation == cardinal::Center)
            {
                element().root().visit([&](i_element& aElement)
                {
                    if (aElement.is_selected() && aElement.has_caddy())
                        aElement.caddy().start_drag(cardinal::Center, aPosition);
                });
            }
            else if (clickLocation != cardinal::Center)
                start_drag(*clickLocation, aPosition);
        }
    }

    void widget_caddy::mouse_button_double_clicked(mouse_button aButton, const point& aPosition, key_modifier aKeyModifier)
    {
        widget::mouse_button_double_clicked(aButton, aPosition, aKeyModifier);
        // double clicking an element with visible text edits it in place
        if (aButton == mouse_button::Left && !preview_mode() && has_element() && element().has_text())
        {
            auto const location = cardinal_at(aPosition, aKeyModifier);
            if (!location || *location == cardinal::Center)
            {
                end_drag();
                begin_text_edit();
            }
        }
    }

    void widget_caddy::mouse_button_released(mouse_button aButton, const point& aPosition)
    {
        bool const wasCapturing = capturing();
        widget::mouse_button_released(aButton, aPosition);
        if (aButton == mouse_button::Left && iRubberBandAnchor)
        {
            end_rubber_band();
            return;
        }
        if (aButton == mouse_button::Left && wasCapturing)
        {
            if (iDragInfo && !iDragInfo->wasDragged && iDragInfo->part == cardinal::Center)
            {
                if (element().group() == element_group::Workflow && element().has_widget())
                {
                    element().select(false, false);
                    element().widget().set_focus();
                }
            }
            iElement->root().visit([&](i_element& aElement)
            {
                if (aElement.is_selected() && aElement.has_caddy())
                    aElement.caddy().end_drag();
            });
        }
        else if (aButton == mouse_button::Right)
        {
            if (!iElement->is_selected())
                iElement->select(true);
            display_element_context_menu(*this, *iElement);
        }
    }

    void widget_caddy::mouse_moved(const point& aPosition, key_modifier aKeyModifier)
    {
        widget::mouse_moved(aPosition, aKeyModifier);
        if (iRubberBandAnchor)
        {
            update_rubber_band(design_position(aPosition));
            return;
        }
        if (capturing() && iDragInfo)
        {
            bool const ignoreConstraints = ((aKeyModifier & key_modifier::SHIFT) != key_modifier::None);
            if (iDragInfo->part == cardinal::Center)
            {
                element().root().visit([&](i_element& aElement)
                {
                    if (aElement.is_selected() && aElement.has_caddy())
                        aElement.caddy().drag( aPosition, ignoreConstraints);
                });
            }
            else
                drag(aPosition, ignoreConstraints);
        }
    }

    point widget_caddy::design_position(point const& aPosition) const
    {
        return to_window_coordinates(aPosition) + root().window_position();
    }

    void widget_caddy::update_rubber_band(point const& aDesignPosition)
    {
        if (!iRubberBandAnchor)
            return;
        rect const band{ iRubberBandAnchor->min(aDesignPosition), iRubberBandAnchor->max(aDesignPosition) };
        // the overlay is in the window the elements are in (a window being designed is a nested window which is drawn above its caddy)
        i_widget& host = has_item() && item().is_widget() && item().as_widget().is_root() ? item().as_widget() : root().as_widget();
        if (!iRubberBand)
        {
            iRubberBand = ref_ptr<i_widget>{ make_ref<widget<>>(host) };
            iRubberBand->set_ignore_mouse_events(true);
            iRubberBand->set_ignore_non_client_mouse_events(true);
            i_widget* rubberBand = &*iRubberBand;
            iRubberBand->painted([rubberBand](i_graphics_context& aGc)
            {
                auto const& palette = service<i_app>().current_style().palette();
                aGc.draw_rect(rect{ point{}, rubberBand->extents() }, palette.color(color_role::Selection), palette.color(color_role::Selection).with_alpha(0.25));
            });
        }
        iRubberBand->move(host.to_client_coordinates(band.top_left() - host.root().window_position()));
        iRubberBand->resize(band.extents());
        iRubberBand->bring_to_front();
        iRubberBand->update();
        // select the elements whose centres are within the band
        element().root().visit([&](i_element& aElement)
        {
            if (!aElement.has_caddy() || &aElement == &element().root() || aElement.caddy().effectively_hidden() ||
                (aElement.group() != element_group::Widget && aElement.group() != element_group::Layout))
                return;
            aElement.select(band.contains(design_rect(aElement.caddy()).center()), false);
        });
    }

    void widget_caddy::end_rubber_band()
    {
        iRubberBandAnchor = std::nullopt;
        if (iRubberBand)
        {
            if (iRubberBand->has_parent())
                iRubberBand->parent().remove(*iRubberBand);
            iRubberBand = {};
        }
    }

    void widget_caddy::mouse_entered(const point& aPosition)
    {
        update();
    }

    void widget_caddy::mouse_left()
    {
        update();
    }

// For some reason Visual Studio 2026 C++ optimiser fucks up in widget_caddy::mouse_cursor() (access violation)...
#ifdef _MSC_VER
#pragma optimize("", off) 
#endif

    mouse_cursor widget_caddy::mouse_cursor() const
    {
        if (nested())
            return widget::mouse_cursor();
        auto const cursorLocation = cardinal_at(mouse_position(), service<i_keyboard>().modifiers());
        if (cursorLocation.has_value())
            switch (cursorLocation.value())
            {
            case cardinal::NorthWest:
            case cardinal::SouthEast:
                return mouse_system_cursor::SizeNWSE;
            case cardinal::NorthEast:
            case cardinal::SouthWest:
                return mouse_system_cursor::SizeNESW;
            case cardinal::North:
            case cardinal::South:
                return mouse_system_cursor::SizeNS;
            case cardinal::West:
            case cardinal::East:
                return mouse_system_cursor::SizeWE;
            case cardinal::Center:
                return mouse_system_cursor::SizeAll;
            };
        return widget::mouse_cursor();
    }

#ifdef _MSC_VER
#pragma optimize("", on) 
#endif

    void widget_caddy::start_drag(cardinal aPart, point const& aPosition)
    {
        iDragInfo.emplace(aPart, aPosition - cardinal_rect(aPart).center());
    }

    bool widget_caddy::can_be_dropped() const
    {
        return has_element() && can_be_moved(element());
    }

    bool widget_caddy::capturing_drop() const
    {
        return iDropCandidate;
    }

    void widget_caddy::update_drop_target(point const& aPosition)
    {
        iDropPosition = to_window_coordinates(aPosition) + root().window_position();
        iDropTarget = find_drop_container(iProject.root(), element(), iDropPosition);
        if (iDropTarget != nullptr)
        {
            try
            {
                show_drop_highlight(*iDropTarget, element().type(), iDropHighlight, iDropPosition, this);
                return;
            }
            catch (...)
            {
                iDropTarget = nullptr;
            }
        }
        hide_drop_highlight(iDropHighlight);
    }

    void widget_caddy::move_to(i_element& aContainer, point const& aDropPosition)
    {
        auto& layout = aContainer.child_layout(element().type());
        auto const insertion = find_insertion_point(layout, aDropPosition, this);
        if (has_parent_layout() && &parent_layout() == &layout)
        {
            // already there?
            auto const current = layout.find(*this);
            i_widget const* after = (current && *current + 1u < layout.count() && layout.is_widget_at(*current + 1u)) ?
                &layout.get_widget_at(*current + 1u) : nullptr;
            if (insertion.next == after)
                return;
        }
        i_element const* before = nullptr;
        if (insertion.next != nullptr)
            for (auto const& sibling : aContainer.children())
                if (sibling->has_caddy() && static_cast<i_widget const*>(&sibling->caddy()) == insertion.next)
                    before = &*sibling;
        move_element_to_container(iProject, element(), aContainer, before);
    }

    void widget_caddy::move_to_canvas(point const& aDropPosition)
    {
        // only widgets can be top level (layouts must be within a window/dialog)
        if (element().group() != element_group::Widget)
            return;
        i_element* top = &element();
        while (top->is_nested())
            top = &top->parent();
        if (top == &element() || !top->has_caddy() || !top->caddy().has_parent())
            return;
        auto& workspace = top->caddy().parent();
        if (!design_rect(workspace).contains(aDropPosition))
            return;
        move_element_to_canvas(iProject, element(), iProject.root(), workspace, aDropPosition);
    }

    void widget_caddy::drag(point const& aPosition, bool aIgnoreConstraints)
    {
        if (!iDragInfo)
            return;
        if (iDragInfo->part == cardinal::Center && capturing() && can_be_dropped())
        {
            // dragging an element onto a layout moves it there (on release)
            auto const startPosition = cardinal_rect(iDragInfo->part).center() + iDragInfo->dragFrom;
            if (iDragInfo->wasDragged || (aPosition - startPosition).to_vec2().magnitude() >= 4.0_dip)
            {
                iDragInfo->wasDragged = true;
                iDropCandidate = true;
                set_design_drag_active(true);
                update_drop_target(aPosition);
            }
        }
        if (nested())
            return; // position and size of nested elements are managed by their container's layout
        auto const adjust = point{ aPosition - cardinal_rect(iDragInfo->part).center() } - iDragInfo->dragFrom;
        auto r = non_client_rect();
        switch (iDragInfo->part)
        {
        case cardinal::NorthWest:
            r.x += adjust.x;
            r.y += adjust.y;
            r.cx -= adjust.x;
            r.cy -= adjust.y;
            break;
        case cardinal::NorthEast:
            r.y += adjust.y;
            r.cx += adjust.x;
            r.cy -= adjust.y;
            break;
        case cardinal::North:
            r.y += adjust.y;
            r.cy -= adjust.y;
            break;
        case cardinal::West:
            r.x += adjust.x;
            r.cx -= adjust.x;
            break;
        case cardinal::Center:
            r.x += adjust.x;
            r.y += adjust.y;
            break;
        case cardinal::East:
            r.cx += adjust.x;
            break;
        case cardinal::SouthWest:
            r.x += adjust.x;
            r.cx -= adjust.x;
            r.cy += adjust.y;
            break;
        case cardinal::SouthEast:
            r.cx += adjust.x;
            r.cy += adjust.y;
            break;
        case cardinal::South:
            r.cy += adjust.y;
            break;
        }
        auto const minSize = aIgnoreConstraints ? internal_spacing().size() : minimum_size();
        if (r.cx < minSize.cx)
        {
            r.cx = minSize.cx;
            if (non_client_rect().x < r.x)
                r.x = non_client_rect().right() - r.cx;
        }
        if (r.cy < minSize.cy)
        {
            r.cy = minSize.cy;
            if (non_client_rect().y < r.y)
                r.y = non_client_rect().bottom() - r.cy;
        }
        move(r.top_left() - parent().origin());
        resize(r.extents());
        iDragInfo->wasDragged = true;
    }

    void widget_caddy::end_drag()
    {
        bool const wasDragged = iDragInfo && iDragInfo->wasDragged;
        bool const droppable = wasDragged && capturing_drop() ;
        iDragInfo = std::nullopt;
        auto const target = iDropTarget;
        iDropTarget = nullptr;
        hide_drop_highlight(iDropHighlight);
        if (droppable && has_element())
        {
            try
            {
                if (target != nullptr)
                    move_to(*target, iDropPosition);
                else if (nested())
                    move_to_canvas(iDropPosition); // dragged out of its layout onto empty canvas
            }
            catch (...)
            {
                // not droppable there
            }
        }
        iDropCandidate = false;
        set_design_drag_active(false);
    }

    bool widget_caddy::can_undo() const
    {
        // todo
        return false;
    }

    bool widget_caddy::can_redo() const
    {
        // todo
        return false;
    }

    bool widget_caddy::can_cut() const
    {
        // todo
        return false;
    }

    bool widget_caddy::can_copy() const
    {
        // todo
        return false;
    }

    bool widget_caddy::can_paste() const
    {
        // todo
        return false;
    }

    bool widget_caddy::can_delete_selected() const
    {
        if (preview_mode())
            return false;
        bool someSelected = false;
        iProject.root().visit([&](i_element& aElement)
        {
            if (aElement.is_selected())
                someSelected = true;
        });
        return someSelected;
    }

    bool widget_caddy::can_select_all() const
    {
        return !preview_mode();
    }

    void widget_caddy::undo(i_clipboard& aClipboard)
    {
        // todo
    }

    void widget_caddy::redo(i_clipboard& aClipboard)
    {
        // todo
    }

    void widget_caddy::cut(i_clipboard& aClipboard)
    {
        // todo
    }

    void widget_caddy::copy(i_clipboard& aClipboard)
    {
        // todo
    }

    void widget_caddy::paste(i_clipboard& aClipboard)
    {
        // todo
    }

    void widget_caddy::delete_selected()
    {
        thread_local std::vector<weak_ref_ptr<i_element>> tToDelete;
        iProject.root().visit([&](i_element& aElement)
        {
            if (aElement.is_selected())
                tToDelete.push_back(aElement);
        });
        for (auto& e : tToDelete)
        {
            if (e.valid())
                iProject.remove_element(*e);
        }
        tToDelete.clear();
    }

    void widget_caddy::select_all()
    {
        iProject.root().visit([&](i_element& aElement)
        {
            if (aElement.has_layout_item())
                aElement.select(true, false);
        });
    }

    std::optional<cardinal> widget_caddy::cardinal_at(point const& aPosition, key_modifier aKeyModifier) const
    {
        return ((aKeyModifier & key_modifier::CTRL) == key_modifier::None) ? 
            cardinal_at(aPosition) : cardinal::Center;
    }

    std::optional<cardinal> widget_caddy::cardinal_at(point const& aPosition) const
    {
        if (cardinal_rect(cardinal::NorthWest).contains(aPosition))
            return cardinal::NorthWest;
        else if (cardinal_rect(cardinal::SouthEast).contains(aPosition))
            return cardinal::SouthEast;
        else if (cardinal_rect(cardinal::NorthEast).contains(aPosition))
            return cardinal::NorthEast;
        else if (cardinal_rect(cardinal::SouthWest).contains(aPosition))
            return cardinal::SouthWest;
        else if (cardinal_rect(cardinal::North).contains(aPosition))
            return cardinal::North;
        else if (cardinal_rect(cardinal::South).contains(aPosition))
            return cardinal::South;
        else if (cardinal_rect(cardinal::West).contains(aPosition))
            return cardinal::West;
        else if (cardinal_rect(cardinal::East).contains(aPosition))
            return cardinal::East;
        else if (cardinal_rect(cardinal::Center).contains(aPosition))
            return cardinal::Center;
        else
            return {};
    }

    rect widget_caddy::cardinal_rect(cardinal aPart, bool aForHitTest) const
    {
        auto const pw = internal_spacing().left * 2.0;
        auto const cr = client_rect(false).inflated(pw / 2.0);
        rect result;
        switch (aPart)
        {
        case cardinal::NorthWest:
            result = rect{ cr.top_left(), size{ pw } };
            break;
        case cardinal::North:
            result = rect{ point{ cr.center().x - pw / 2.0, cr.top() }, size{ pw } };
            break;
        case cardinal::NorthEast:
            result = rect{ point{ cr.right() - pw, cr.top() }, size{ pw } };
            break;
        case cardinal::West:
            result = rect{ point{ cr.left(), cr.center().y - pw / 2.0 }, size{ pw } };
            break;
        case cardinal::Center:
            result = cr.deflated(size{ pw });
            break;
        case cardinal::East:
            result = rect{ point{ cr.right() - pw, cr.center().y - pw / 2.0 }, size{ pw } };
            break;
        case cardinal::SouthWest:
            result = rect{ point{ cr.left(), cr.bottom() - pw }, size{ pw } };
            break;
        case cardinal::South:
            result = rect{ point{ cr.center().x - pw / 2.0, cr.bottom() - pw }, size{ pw } };
            break;
        case cardinal::SouthEast:
            result = rect{ point{ cr.right() - pw, cr.bottom() - pw }, size{ pw } };
            break;
        default:
            result = cr;
            break;
        }
        if (aForHitTest && aPart != cardinal::Center)
            result.inflate(result.extents() / 2.0);
        return result;
    }

    rect design_rect(i_widget const& aWidget)
    {
        return aWidget.non_client_rect() + aWidget.root().window_position();
    }


    i_element* find_drop_container(i_element& aRoot, i_element const& aDropped, point const& aDropPosition)
    {
        i_element* result = nullptr;
        std::size_t resultDepth = 0;
        aRoot.visit([&](i_element& aElement)
        {
            if (&aElement == &aDropped || !aElement.has_child_layout())
                return;
            // elements without a caddy (e.g. tab pages) are drop targets via their own widget
            i_widget const* target = aElement.has_caddy() ? static_cast<i_widget const*>(&aElement.caddy()) :
                aElement.has_widget() ? &aElement.widget() : nullptr;
            if (target == nullptr || !target->has_parent() || target->effectively_hidden())
                return;
            if (is_descendant_of(aElement, aDropped))
                return;
            if (!design_rect(*target).contains(aDropPosition))
                return;
            auto const depth = element_depth(aElement);
            if (result == nullptr || depth > resultDepth)
            {
                result = &aElement;
                resultDepth = depth;
            }
        });
        return result;
    }

    void add_to_container(i_project& aProject, i_element& aElement, optional_point const& aDropPosition)
    {
        auto& container = aElement.parent();
        if (!aElement.needs_caddy())
        {
            // created by (and within) its container (e.g. a tab page)
            if (container.has_widget())
                aElement.create_layout_item(container.widget());
            if (!aElement.has_layout_item())
                throw i_element::no_layout_item();
            aElement.apply_attributes(show_ids());
            return;
        }
        auto& layout = container.child_layout(aElement.type());
        i_widget& parentWidget = layout.has_parent_widget() ? layout.parent_widget() : static_cast<i_widget&>(container.caddy());
        auto caddy = make_ref<widget_caddy>(aProject, aElement, parentWidget, point{});
        auto const index = aDropPosition ? find_insertion_point(layout, *aDropPosition).index : layout.count();
        layout.add_at(index, caddy);
        sync_element_order(aElement, layout);
        grow_top_level_caddy(aElement);
    }

    void show_drop_highlight(i_element& aContainer, i_string const& aChildType, ref_ptr<i_widget>& aHighlight, point const& aDropPosition, i_widget const* aExclude)
    {
        auto& layout = aContainer.child_layout(aChildType);
        if (!layout.has_parent_widget())
        {
            hide_drop_highlight(aHighlight);
            return;
        }
        auto& parentWidget = layout.parent_widget();
        if (aHighlight && (!aHighlight->has_parent() || &aHighlight->parent() != &parentWidget))
            hide_drop_highlight(aHighlight);
        if (!aHighlight)
        {
            // a non-layout-managed child of the layout's widget (so it works inside nested windows too) that just paints the highlight
            aHighlight = ref_ptr<i_widget>{ make_ref<widget<>>(parentWidget) };
            aHighlight->set_ignore_mouse_events(true);
            aHighlight->set_ignore_non_client_mouse_events(true);
            i_widget* highlight = &*aHighlight;
            aHighlight->painted([highlight](i_graphics_context& aGc)
            {
                // fill alpha fades 0.0 -> 0.5 -> 0.0 over two seconds, repeating
                auto const elapsed = std::chrono::duration<double>(std::chrono::steady_clock::now() - tDropHighlightStart).count();
                auto const alpha = 0.25 * (1.0 - std::cos(std::numbers::pi * elapsed));
                aGc.fill_rect(rect{ point{}, highlight->extents() }, color::Yellow.with_alpha(alpha));
                // insertion line flashes like a caret: black for half a second, white for half a second
                if (tDropLine)
                    aGc.draw_line(tDropLine->first, tDropLine->second, pen{ std::fmod(elapsed, 1.0) < 0.5 ? color::Black : color::White, 2.0_dip });
            });
            tDropHighlightStart = std::chrono::steady_clock::now();
            tDropHighlightAnimator.emplace(*aHighlight, [highlight](widget_timer& aAnimator)
            {
                aAnimator.again();
                highlight->update();
            }, std::chrono::milliseconds{ 20 });
        }
        auto const insertion = find_insertion_point(layout, aDropPosition, aExclude);
        auto const origin = design_rect(parentWidget).top_left() + layout.position();
        tDropLine = std::make_pair(insertion.line.first - origin, insertion.line.second - origin);
        aHighlight->move(layout.position());
        aHighlight->resize(layout.extents());
        aHighlight->bring_to_front();
        aHighlight->update();
    }

    void hide_drop_highlight(ref_ptr<i_widget>& aHighlight)
    {
        tDropLine = std::nullopt;
        tDropHighlightAnimator = std::nullopt;
        if (aHighlight)
        {
            if (aHighlight->has_parent())
                aHighlight->parent().remove(*aHighlight);
            aHighlight = {};
        }
    }

    bool can_be_moved(i_element const& aElement)
    {
        if (aElement.group() != element_group::Widget && aElement.group() != element_group::Layout)
            return false;
        if (!aElement.has_caddy() || !aElement.has_layout_item())
            return false;
        return !(aElement.layout_item().is_widget() && aElement.layout_item().as_widget().is_root()); // windows stay top level
    }

    void move_element_to_container(i_project& aProject, i_element& aElement, i_element& aContainer, i_element const* aBefore)
    {
        if (!can_be_moved(aElement) || &aElement == &aContainer || is_descendant_of(aContainer, aElement))
            return;
        auto& caddy = aElement.caddy();
        auto& layout = aContainer.child_layout(aElement.type());
        ref_ptr<i_widget> keep{ &caddy };
        aProject.move_element(aElement, aContainer, aBefore);
        if (caddy.has_parent_layout())
            caddy.parent_layout().remove(caddy);
        i_widget& parentWidget = layout.has_parent_widget() ? layout.parent_widget() : static_cast<i_widget&>(aContainer.caddy());
        parentWidget.add(keep);
        optional_layout_item_index index;
        if (aBefore != nullptr && aBefore->has_caddy())
            index = layout.find(aBefore->caddy());
        layout.add_at(index ? *index : layout.count(), ref_ptr<i_layout_item>{ keep });
        // now nested: managed by the container's layout and must receive mouse events itself
        caddy.set_ignore_mouse_events(false);
        caddy.set_consider_ancestors_for_mouse_events(false);
        grow_top_level_caddy(aElement);
    }

    void move_element_to_canvas(i_project& aProject, i_element& aElement, i_element& aNewParent, i_widget& aWorkspace, point const& aDropPosition)
    {
        if (!can_be_moved(aElement) || aElement.group() != element_group::Widget)
            return;
        auto& caddy = aElement.caddy();
        ref_ptr<i_widget> keep{ &caddy };
        aProject.move_element(aElement, aNewParent);
        if (caddy.has_parent_layout())
            caddy.parent_layout().remove(caddy);
        aWorkspace.add(keep);
        // now top level: floats on the workspace
        caddy.set_consider_ancestors_for_mouse_events(true);
        auto const idealSize = caddy.transformed_ideal_size();
        auto const minimumSize = caddy.minimum_size();
        caddy.resize(size{ std::max(idealSize.cx, minimumSize.cx), std::max(idealSize.cy, minimumSize.cy) });
        caddy.move(aDropPosition - design_rect(aWorkspace).top_left() - point{ caddy.extents() / 2.0 });
        caddy.bring_to_front();
    }

    void create_caddies(i_project& aProject, i_widget& aWorkspace)
    {
        point position{ 32.0_dip, 32.0_dip };
        std::function<void(i_element&)> create_nested = [&](i_element& aParent)
        {
            for (auto& child : aParent.children())
                if (is_design_element(*child) && !child->has_caddy() && !child->has_layout_item())
                {
                    try
                    {
                        add_to_container(aProject, *child);
                    }
                    catch (...)
                    {
                        continue; // not representable on the design surface (it is still preserved in the project and saved)
                    }
                    create_nested(*child);
                }
        };
        auto create_top_level = [&](i_element& aElement)
        {
            if (!is_design_element(aElement) || aElement.has_caddy())
                return;
            // widget(i_widget& aParent) adds itself to its parent with a non-owning reference so, as with a toolbox drop, create 
            // the caddy on the root and then move it to the workspace passing an owning reference (otherwise the caddy (which 
            // is also the window nest) is destroyed when 'caddy' goes out of scope)
            auto caddy = make_ref<widget_caddy>(aProject, aElement, aWorkspace.root().as_widget(), point{});
            aWorkspace.add(ref_ptr<i_widget>{ caddy });
            caddy->move(position);
            create_nested(aElement);
            auto const idealSize = caddy->transformed_ideal_size();
            auto const minimumSize = caddy->minimum_size();
            caddy->resize(size{ std::max(idealSize.cx, minimumSize.cx), std::max(idealSize.cy, minimumSize.cy) });
            position += point{ 32.0_dip, 32.0_dip };
        };
        for (auto& e : aProject.root().children())
        {
            if (e->group() == element_group::UserInterface)
            {
                for (auto& uiElement : e->children())
                    create_top_level(*uiElement);
            }
            else
                create_top_level(*e);
        }
    }

    void remove_caddies(i_project& aProject)
    {
        aProject.root().visit([](i_element& aElement)
        {
            if (aElement.has_caddy() && !aElement.is_nested() && aElement.caddy().has_parent())
                aElement.caddy().parent().remove(aElement.caddy());
        });
    }
}
