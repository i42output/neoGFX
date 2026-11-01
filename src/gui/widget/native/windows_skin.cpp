// windows_skin.cpp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2026 Leigh Johnston.  All Rights Reserved.
  
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

#include <algorithm>
#include <cmath>
#include <vector>

#include <Uxtheme.h>
#include <vsstyle.h>
#include <vssym32.h>
#pragma comment(lib, "UxTheme.lib")

#include <neogfx/gfx/i_texture_manager.hpp>
#include <neogfx/gfx/i_graphics_context.hpp>
#include <neogfx/gui/widget/i_widget.hpp>
#include "native_skin.hpp"
#include "windows_skin.hpp"

namespace neogfx
{
    namespace native
    {
        i_skin& native_skin()
        {
            // Intentionally never destroyed: the skin caches textures, which must not be
            // released during static destruction (after the rendering engine has gone).
            static native::windows::skin* const sNativeSkin = new native::windows::skin{};
            return *sNativeSkin;
        }
    }

    namespace native::windows
    {
        namespace
        {
            constexpr std::size_t kMaxCachedParts = 512u;

            std::uint32_t system_dpi()
            {
                HDC const screen = ::GetDC(nullptr);
                std::uint32_t const dpi = static_cast<std::uint32_t>(::GetDeviceCaps(screen, LOGPIXELSX));
                ::ReleaseDC(nullptr, screen);
                return dpi != 0u ? dpi : 96u;
            }

            std::uint32_t item_dpi(i_skinnable_item const& aItem)
            {
                if (aItem.is_widget() && aItem.as_widget().device_metrics_available())
                {
                    auto const dpi = static_cast<std::uint32_t>(std::round(aItem.as_widget().device_metrics().horizontal_dpi()));
                    if (dpi != 0u)
                        return dpi;
                }
                return system_dpi();
            }

            // The dark mode variants of the visual styles (Windows 10 1809 or later) are used for a widget on a dark background; the
            // class list falls back to the normal class where there is no dark variant (or on older versions of Windows).
            bool dark_background(i_skinnable_item const& aItem)
            {
                return aItem.is_widget() && aItem.as_widget().container_background_color().dark();
            }

            std::wstring class_list(i_skinnable_item const& aItem, std::wstring const& aClass, std::wstring const& aClassList)
            {
                if (dark_background(aItem))
                    return L"DarkMode_Explorer::" + aClass + L";" + aClassList;
                return aClassList;
            }

            std::wstring class_list(i_skinnable_item const& aItem, std::wstring const& aClass)
            {
                return class_list(aItem, aClass, aClass);
            }

            // Theme part state offsets: normal, hot, pressed, disabled.
            int state_offset(bool aEnabled, bool aHot, bool aPressed)
            {
                if (!aEnabled)
                    return 3;
                if (aPressed)
                    return 2;
                if (aHot)
                    return 1;
                return 0;
            }

            int scrollbar_part_state(bool aEnabled, bool aHot, bool aPressed)
            {
                return SCRBS_NORMAL + state_offset(aEnabled, aHot, aPressed);
            }

            int push_button_state(const i_skinnable_item& aItem, const i_push_button& aButton, const push_button_appearance& aAppearance)
            {
                auto const& widget = aItem.as_widget();
                bool const enabled = widget.effectively_enabled();
                bool const hot = enabled && widget.entered();
                bool const pressed = enabled && ((hot && widget.capturing()) || aButton.is_checked());
                if (!enabled)
                    return PBS_DISABLED;
                if (pressed)
                    return PBS_PRESSED;
                if (hot)
                    return PBS_HOT;
                if (aAppearance.defaultButton || widget.has_focus())
                    return PBS_DEFAULTED;
                return PBS_NORMAL;
            }
        }

        skin::skin()
        {
        }

        skin::~skin()
        {
            close_themes();
        }

        std::string const& skin::name() const
        {
            static const std::string sName = "Native";
            return sName;
        }

        bool skin::has_fallback() const
        {
            return true;
        }

        i_skin& skin::fallback()
        {
            return iFallback;
        }

        void skin::activate()
        {
            // Pick up any change to the system theme made while we were inactive.
            close_themes();
        }

        void skin::deactivate()
        {
            close_themes();
        }

        size skin::preferred_size(skin_element aElement, optional_size const& aDesiredSize) const
        {
            int part = 0;
            int state = 0;
            switch (aElement)
            {
            case skin_element::CheckBox:
                part = BP_CHECKBOX;
                state = CBS_UNCHECKEDNORMAL;
                break;
            case skin_element::RadioButton:
                part = BP_RADIOBUTTON;
                state = RBS_UNCHECKEDNORMAL;
                break;
            default:
                return iFallback.preferred_size(aElement, aDesiredSize);
            }
            HTHEME const buttonTheme = theme(L"BUTTON", system_dpi());
            SIZE partSize{};
            if (buttonTheme == nullptr || FAILED(::GetThemePartSize(buttonTheme, nullptr, part, state, nullptr, TS_TRUE, &partSize)))
                return iFallback.preferred_size(aElement, aDesiredSize);
            return size{ length{ static_cast<dimension>(partSize.cx) }, length{ static_cast<dimension>(partSize.cy) } };
        }

        dimension skin::push_button_border_width(const i_skinnable_item& aItem, bool aDefaultButton, dimension aBorderWidth) const
        {
            // Push buttons are drawn by neoGFX itself, so keep their appearance consistent with it.
            return iFallback.push_button_border_width(aItem, aDefaultButton, aBorderWidth);
        }

        dimension skin::window_frame_width(const i_skinnable_item& aItem, dimension aFrameWidth) const
        {
            // Window frames and title bars are drawn by neoGFX itself, so keep their appearance consistent with it.
            return iFallback.window_frame_width(aItem, aFrameWidth);
        }

        void skin::adjust_title_bar_appearance(const i_skinnable_item& aItem, title_bar_appearance& aAppearance) const
        {
            iFallback.adjust_title_bar_appearance(aItem, aAppearance);
        }

        void skin::draw_push_button(i_graphics_context& aGc, const i_skinnable_item& aItem, const i_push_button& aButton, const push_button_appearance& aAppearance) const
        {
            HTHEME const buttonTheme = push_button_theme(aItem, aButton, aAppearance);
            if (buttonTheme == nullptr)
            {
                iFallback.draw_push_button(aGc, aItem, aButton, aAppearance);
                return;
            }
            auto const& widget = aItem.as_widget();
            scoped_units su{ widget, units::Pixels };
            int const state = push_button_state(aItem, aButton, aAppearance);
            rect buttonRect = aAppearance.boundingRect;
            buttonRect.inflate(aAppearance.borderWidth / 2.0);
            draw_part(aGc, buttonTheme, BP_PUSHBUTTON, state, buttonRect);
            if (widget.has_focus())
            {
                RECT const bounds{ 0, 0, static_cast<LONG>(std::ceil(buttonRect.cx)), static_cast<LONG>(std::ceil(buttonRect.cy)) };
                RECT content = bounds;
                if (FAILED(::GetThemeBackgroundContentRect(buttonTheme, nullptr, BP_PUSHBUTTON, state, &bounds, &content)))
                    ::InflateRect(&content, -3, -3);
                aGc.draw_focus_rect(rect{ 
                    point{ std::floor(buttonRect.x) + content.left, std::floor(buttonRect.y) + content.top }, 
                    size{ static_cast<dimension>(content.right - content.left), static_cast<dimension>(content.bottom - content.top) } });
            }
        }

        void skin::push_button_text_color(const i_skinnable_item& aItem, const i_push_button& aButton, const push_button_appearance& aAppearance, i_optional<color>& aTextColor) const
        {
            HTHEME const buttonTheme = push_button_theme(aItem, aButton, aAppearance);
            // (the dark mode push button doesn't supply a usable text color so the button's usual (style) text color is used)
            if (buttonTheme == nullptr || dark_background(aItem))
            {
                iFallback.push_button_text_color(aItem, aButton, aAppearance, aTextColor);
                return;
            }
            COLORREF textColor = 0;
            if (FAILED(::GetThemeColor(buttonTheme, BP_PUSHBUTTON, push_button_state(aItem, aButton, aAppearance), TMT_TEXTCOLOR, &textColor)))
                textColor = ::GetSysColor(COLOR_BTNTEXT);
            aTextColor = color{ GetRValue(textColor) / 255.0, GetGValue(textColor) / 255.0, GetBValue(textColor) / 255.0, 1.0 };
        }

        void skin::draw_scrollbar(i_graphics_context& aGc, const i_skinnable_item& aItem, const i_scrollbar& aScrollbar) const
        {
            if (!aScrollbar.visible() || aScrollbar.auto_hidden())
                return;
            HTHEME const scrollbarTheme = aScrollbar.type() == scrollbar_style::Normal ? theme(class_list(aItem, L"SCROLLBAR"), item_dpi(aItem)) : nullptr;
            if (scrollbarTheme == nullptr)
            {
                iFallback.draw_scrollbar(aGc, aItem, aScrollbar);
                return;
            }

            scoped_units su{ aItem.as_widget(), units::Pixels };

            bool const vertical = aScrollbar.orientation() == scrollbar_orientation::Vertical;
            bool const enabled = aItem.as_widget().effectively_enabled();
            bool const noThumb = (aScrollbar.maximum() - aScrollbar.minimum() <= aScrollbar.page());
            auto const hovering = aScrollbar.hovering_element();
            auto const clicked = aScrollbar.clicked_element();

            // track
            draw_part(aGc, scrollbarTheme, vertical ? SBP_LOWERTRACKVERT : SBP_LOWERTRACKHORZ,
                scrollbar_part_state(enabled && !noThumb, false, false), aItem.element_rect(skin_element::Scrollbar));
            if (!noThumb)
            {
                if (hovering == scrollbar_element::PageUpArea || clicked == scrollbar_element::PageUpArea)
                    draw_part(aGc, scrollbarTheme, vertical ? SBP_UPPERTRACKVERT : SBP_UPPERTRACKHORZ,
                        scrollbar_part_state(enabled, true, clicked == scrollbar_element::PageUpArea), aItem.element_rect(skin_element::ScrollbarPageUpArea));
                if (hovering == scrollbar_element::PageDownArea || clicked == scrollbar_element::PageDownArea)
                    draw_part(aGc, scrollbarTheme, vertical ? SBP_LOWERTRACKVERT : SBP_LOWERTRACKHORZ,
                        scrollbar_part_state(enabled, true, clicked == scrollbar_element::PageDownArea), aItem.element_rect(skin_element::ScrollbarPageDownArea));
            }

            // arrow buttons
            bool const upEnabled = enabled && !noThumb && aScrollbar.position() != aScrollbar.minimum();
            bool const downEnabled = enabled && !noThumb && aScrollbar.position() != aScrollbar.maximum() - aScrollbar.page();
            int const upBase = vertical ? ABS_UPNORMAL : ABS_LEFTNORMAL;
            int const downBase = vertical ? ABS_DOWNNORMAL : ABS_RIGHTNORMAL;
            draw_part(aGc, scrollbarTheme, SBP_ARROWBTN,
                upBase + state_offset(upEnabled, hovering == scrollbar_element::UpButton, clicked == scrollbar_element::UpButton),
                aItem.element_rect(skin_element::ScrollbarUpArrow));
            draw_part(aGc, scrollbarTheme, SBP_ARROWBTN,
                downBase + state_offset(downEnabled, hovering == scrollbar_element::DownButton, clicked == scrollbar_element::DownButton),
                aItem.element_rect(skin_element::ScrollbarDownArrow));

            // thumb
            if (!noThumb)
                draw_part(aGc, scrollbarTheme, vertical ? SBP_THUMBBTNVERT : SBP_THUMBBTNHORZ,
                    scrollbar_part_state(enabled, hovering == scrollbar_element::Thumb, clicked == scrollbar_element::Thumb),
                    aItem.element_rect(skin_element::ScrollbarThumb));
        }

        void skin::draw_check_box(i_graphics_context& aGc, const i_skinnable_item& aItem, const button_checked_state& aCheckedState) const
        {
            HTHEME const buttonTheme = theme(class_list(aItem, L"BUTTON"), item_dpi(aItem));
            if (buttonTheme == nullptr)
            {
                iFallback.draw_check_box(aGc, aItem, aCheckedState);
                return;
            }
            auto const& widget = aItem.as_widget();
            scoped_units su{ widget, units::Pixels };
            bool const enabled = widget.effectively_enabled();
            bool const hot = enabled && !widget.ignore_mouse_events() && 
                aItem.element_rect(skin_element::ClickableArea).contains(widget.capture_position() ? *widget.capture_position() : widget.mouse_position());
            bool const pressed = hot && widget.capturing();
            int const base = aCheckedState == std::nullopt ? CBS_MIXEDNORMAL : *aCheckedState ? CBS_CHECKEDNORMAL : CBS_UNCHECKEDNORMAL;
            draw_part_true_size(aGc, buttonTheme, BP_CHECKBOX, base + state_offset(enabled, hot, pressed), aItem.element_rect(skin_element::CheckBox));
        }

        void skin::draw_radio_button(i_graphics_context& aGc, const i_skinnable_item& aItem, const button_checked_state& aCheckedState) const
        {
            HTHEME const buttonTheme = theme(class_list(aItem, L"BUTTON"), item_dpi(aItem));
            if (buttonTheme == nullptr)
            {
                iFallback.draw_radio_button(aGc, aItem, aCheckedState);
                return;
            }
            auto const& widget = aItem.as_widget();
            scoped_units su{ widget, units::Pixels };
            bool const enabled = widget.effectively_enabled();
            bool const hot = enabled && !widget.ignore_mouse_events() && 
                aItem.element_rect(skin_element::ClickableArea).contains(widget.mouse_position());
            bool const pressed = hot && widget.capturing();
            int const base = aCheckedState != std::nullopt && *aCheckedState ? RBS_CHECKEDNORMAL : RBS_UNCHECKEDNORMAL;
            draw_part_true_size(aGc, buttonTheme, BP_RADIOBUTTON, base + state_offset(enabled, hot, pressed), aItem.element_rect(skin_element::RadioButton));
        }

        void skin::draw_tree_expander(i_graphics_context& aGc, const i_skinnable_item& aItem, bool aExpandedState) const
        {
            HTHEME const treeTheme = theme(class_list(aItem, L"TreeView", L"Explorer::TreeView;TREEVIEW"), item_dpi(aItem));
            if (treeTheme == nullptr)
            {
                iFallback.draw_tree_expander(aGc, aItem, aExpandedState);
                return;
            }
            draw_part_true_size(aGc, treeTheme, TVP_GLYPH, aExpandedState ? GLPS_OPENED : GLPS_CLOSED, aItem.element_rect(skin_element::TreeExpander));
        }

        void skin::draw_slider(i_graphics_context& aGc, const i_skinnable_item& aItem, slider_orientation aOrientation, double aNormalizedValue, bool aDragging, i_optional<color_or_gradient::abstract_type> const& aBarColor) const
        {
            HTHEME const trackbarTheme = theme(class_list(aItem, L"TRACKBAR"), item_dpi(aItem));
            if (trackbarTheme == nullptr)
            {
                iFallback.draw_slider(aGc, aItem, aOrientation, aNormalizedValue, aDragging, aBarColor);
                return;
            }
            auto const& widget = aItem.as_widget();
            scoped_units su{ widget, units::Pixels };
            bool const vertical = aOrientation == slider_orientation::Vertical;
            rect const track = aItem.element_rect(skin_element::SliderTrack);
            rect const thumb = aItem.element_rect(skin_element::SliderThumb);
            // the track at its themed thickness, centred across the slider's track
            int const trackPart = vertical ? TKP_TRACKVERT : TKP_TRACK;
            int const trackState = vertical ? TRVS_NORMAL : TRS_NORMAL;
            SIZE trackSize{};
            if (FAILED(::GetThemePartSize(trackbarTheme, nullptr, trackPart, trackState, nullptr, TS_TRUE, &trackSize)))
                trackSize = SIZE{ 4, 4 };
            rect trackRect = track;
            if (!vertical)
            {
                trackRect.cy = std::max<dimension>(1.0, static_cast<dimension>(trackSize.cy));
                trackRect.y = std::floor(track.center().y - trackRect.cy / 2.0);
            }
            else
            {
                trackRect.cx = std::max<dimension>(1.0, static_cast<dimension>(trackSize.cx));
                trackRect.x = std::floor(track.center().x - trackRect.cx / 2.0);
            }
            draw_part(aGc, trackbarTheme, trackPart, trackState, trackRect);
            // the thumb
            bool const enabled = widget.effectively_enabled();
            bool const hot = enabled && widget.entered() && thumb.contains(widget.mouse_position());
            int thumbState = TUS_NORMAL;
            if (!enabled)
                thumbState = TUS_DISABLED;
            else if (aDragging)
                thumbState = TUS_PRESSED;
            else if (hot)
                thumbState = TUS_HOT;
            else if (widget.has_focus())
                thumbState = TUS_FOCUSED;
            // (TUVS_* (the vertical thumb's states) have the same values as TUS_*)
            int const thumbPart = vertical ? TKP_THUMBVERT : TKP_THUMB;
            SIZE thumbSize{};
            if (FAILED(::GetThemePartSize(trackbarTheme, nullptr, thumbPart, thumbState, nullptr, TS_TRUE, &thumbSize)) || thumbSize.cx <= 0 || thumbSize.cy <= 0)
            {
                draw_part(aGc, trackbarTheme, thumbPart, thumbState, thumb);
                return;
            }
            // some themes (e.g. Windows 11's) give the vertical thumb the horizontal thumb's (tall) size rather than a wide one
            if (vertical && thumbSize.cx < thumbSize.cy)
                std::swap(thumbSize.cx, thumbSize.cy);
            size const thumbExtents{ static_cast<dimension>(thumbSize.cx), static_cast<dimension>(thumbSize.cy) };
            point const thumbPosition{ 
                std::floor(thumb.x + (thumb.cx - thumbExtents.cx) / 2.0), 
                std::floor(thumb.y + (thumb.cy - thumbExtents.cy) / 2.0) };
            draw_part(aGc, trackbarTheme, thumbPart, thumbState, rect{ thumbPosition, thumbExtents });
        }

        void skin::draw_progress_bar(i_graphics_context& aGc, const i_skinnable_item& aItem, const i_progress_bar& aProgressBar) const
        {
            std::uint32_t const dpi = item_dpi(aItem);
            HTHEME const progressTheme = theme(class_list(aItem, L"PROGRESS"), dpi);
            if (progressTheme == nullptr)
            {
                iFallback.draw_progress_bar(aGc, aItem, aProgressBar);
                return;
            }
            scoped_units su{ aItem.as_widget(), units::Pixels };
            rect const barRect = aProgressBar.bar_rect();
            draw_part(aGc, progressTheme, PP_BAR, 0, barRect);
            RECT const bounds{ 0, 0, static_cast<LONG>(std::ceil(barRect.cx)), static_cast<LONG>(std::ceil(barRect.cy)) };
            RECT content = bounds;
            if (FAILED(::GetThemeBackgroundContentRect(progressTheme, nullptr, PP_BAR, 0, &bounds, &content)))
                content = bounds;
            auto const range = aProgressBar.maximum() - aProgressBar.minimum();
            auto const fraction = range > 0.0 ? std::clamp((aProgressBar.value() - aProgressBar.minimum()) / range, 0.0, 1.0) : 0.0;
            rect fillRect{ 
                point{ barRect.x + content.left, barRect.y + content.top }, 
                size{ std::floor((content.right - content.left) * fraction), static_cast<dimension>(content.bottom - content.top) } };
            if (fillRect.cx >= 1.0 && fillRect.cy >= 1.0)
                draw_part(aGc, progressTheme, PP_FILL, PBFS_NORMAL, fillRect);
        }

        void skin::draw_separators(i_graphics_context& aGc, const i_skinnable_item& aItem, const i_layout& aLayout) const
        {
            // Windows has no themed splitter grip, so use the default skin's.
            iFallback.draw_separators(aGc, aItem, aLayout);
        }

        void skin::draw_window_frame(i_graphics_context& aGc, const i_skinnable_item& aItem, const rect& aFrameRect, dimension aFrameWidth, const color& aFrameColor) const
        {
            iFallback.draw_window_frame(aGc, aItem, aFrameRect, aFrameWidth, aFrameColor);
        }

        void skin::draw_title_bar(i_graphics_context& aGc, const i_skinnable_item& aItem, const title_bar_appearance& aAppearance) const
        {
            iFallback.draw_title_bar(aGc, aItem, aAppearance);
        }

        void skin::draw_title_bar_button(i_graphics_context& aGc, const i_skinnable_item& aItem, widget_part_e aButton, const title_bar_appearance& aAppearance) const
        {
            iFallback.draw_title_bar_button(aGc, aItem, aButton, aAppearance);
        }

        void skin::draw_container_frame(i_graphics_context& aGc, const i_skinnable_item& aItem, const rect& aFrameRect, dimension aFrameWidth, const color& aFrameColor, const color& aInnerFrameColor) const
        {
            iFallback.draw_container_frame(aGc, aItem, aFrameRect, aFrameWidth, aFrameColor, aInnerFrameColor);
        }

        void skin::draw_tab_page_join(i_graphics_context& aGc, const i_skinnable_item& aItem, const i_tab_page& aPage, dimension aFrameWidth, const color& aInnerFrameColor) const
        {
            iFallback.draw_tab_page_join(aGc, aItem, aPage, aFrameWidth, aInnerFrameColor);
        }

        HTHEME skin::theme(std::wstring const& aClassList, std::uint32_t aDpi) const
        {
            if (!::IsAppThemed())
                return nullptr;
            auto existing = iThemes.find(theme_key{ aClassList, aDpi });
            if (existing != iThemes.end())
                return existing->second;
            // OpenThemeDataForDpi requires Windows 10 1703 or later so look it up at runtime.
            typedef HTHEME(WINAPI* open_theme_data_for_dpi_t)(HWND, LPCWSTR, UINT);
            static auto const sOpenThemeDataForDpi = reinterpret_cast<open_theme_data_for_dpi_t>(
                ::GetProcAddress(::GetModuleHandleW(L"uxtheme.dll"), "OpenThemeDataForDpi"));
            HTHEME const newTheme = sOpenThemeDataForDpi != nullptr ? 
                sOpenThemeDataForDpi(nullptr, aClassList.c_str(), aDpi) : 
                ::OpenThemeData(nullptr, aClassList.c_str());
            iThemes.emplace(theme_key{ aClassList, aDpi }, newTheme);
            return newTheme;
        }

        HTHEME skin::push_button_theme(const i_skinnable_item& aItem, const i_push_button& aButton, const push_button_appearance& aAppearance) const
        {
            // Only buttons that look like Windows push buttons are drawn natively; a button whose
            // appearance has been customized (e.g. a colour swatch) keeps its neoGFX look.
            bool const pushButtonStyle = aButton.style() == push_button_style::Normal || aButton.style() == push_button_style::ButtonBox;
            return pushButtonStyle && !aAppearance.customized ? theme(class_list(aItem, L"BUTTON"), item_dpi(aItem)) : nullptr;
        }

        void skin::draw_part(i_graphics_context& aGc, HTHEME aTheme, int aPart, int aState, rect const& aRect) const
        {
            auto const cx = static_cast<std::int32_t>(std::ceil(aRect.cx));
            auto const cy = static_cast<std::int32_t>(std::ceil(aRect.cy));
            if (cx <= 0 || cy <= 0)
                return;

            part_key const key{ aTheme, aPart, aState, cx, cy };
            auto existing = iParts.find(key);
            if (existing == iParts.end())
            {
                if (iParts.size() >= kMaxCachedParts)
                    iParts.clear();

                BITMAPINFO bmi = {};
                bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
                bmi.bmiHeader.biWidth = cx;
                bmi.bmiHeader.biHeight = -cy; // top-down
                bmi.bmiHeader.biPlanes = 1;
                bmi.bmiHeader.biBitCount = 32;
                bmi.bmiHeader.biCompression = BI_RGB;
                HDC const screen = ::GetDC(nullptr);
                HDC const dc = ::CreateCompatibleDC(screen);
                ::ReleaseDC(nullptr, screen);
                void* bits = nullptr;
                HBITMAP const dib = ::CreateDIBSection(dc, &bmi, DIB_RGB_COLORS, &bits, nullptr, 0);
                if (dib == nullptr || bits == nullptr)
                {
                    ::DeleteDC(dc);
                    return;
                }
                HGDIOBJ const oldBitmap = ::SelectObject(dc, dib);
                RECT const partRect{ 0, 0, cx, cy };
                std::size_t const pixelCount = static_cast<std::size_t>(cx) * static_cast<std::size_t>(cy);
                auto* const pixels = static_cast<std::uint32_t*>(bits);

                // GDI does not produce usable alpha, so render the part onto black and onto white
                // and recover (straight) alpha and colour from the difference.
                std::fill(pixels, pixels + pixelCount, 0x00000000u);
                ::DrawThemeBackground(aTheme, dc, aPart, aState, &partRect, nullptr);
                ::GdiFlush();
                std::vector<std::uint32_t> onBlack(pixels, pixels + pixelCount);
                std::fill(pixels, pixels + pixelCount, 0x00FFFFFFu);
                ::DrawThemeBackground(aTheme, dc, aPart, aState, &partRect, nullptr);
                ::GdiFlush();

                std::vector<std::uint32_t> bgra(pixelCount);
                for (std::size_t i = 0; i < pixelCount; ++i)
                {
                    auto const black = onBlack[i];
                    auto const white = pixels[i];
                    std::int32_t alpha = 255;
                    for (std::uint32_t shift = 0u; shift <= 16u; shift += 8u)
                        alpha = std::min<std::int32_t>(alpha, 255 - (static_cast<std::int32_t>((white >> shift) & 0xFFu) - static_cast<std::int32_t>((black >> shift) & 0xFFu)));
                    alpha = std::clamp(alpha, 0, 255);
                    std::uint32_t result = 0u;
                    if (alpha != 0)
                    {
                        result = static_cast<std::uint32_t>(alpha) << 24u;
                        for (std::uint32_t shift = 0u; shift <= 16u; shift += 8u)
                        {
                            auto const channel = std::min<std::uint32_t>(255u, ((black >> shift) & 0xFFu) * 255u / static_cast<std::uint32_t>(alpha));
                            result |= channel << shift;
                        }
                    }
                    // (memory order B, G, R, A; rows bottom up as texture set_pixels expects (cf. set_pixels(const i_image&)))
                    bgra[(static_cast<std::size_t>(cy) - 1u - i / static_cast<std::size_t>(cx)) * static_cast<std::size_t>(cx) + i % static_cast<std::size_t>(cx)] = result;
                }

                ::SelectObject(dc, oldBitmap);
                ::DeleteObject(dib);
                ::DeleteDC(dc);

                auto texture = service<i_texture_manager>().create_texture(
                    size{ static_cast<dimension>(cx), static_cast<dimension>(cy) }, 1.0, texture_sampling::Nearest, texture_data_format::BGRA);
                texture->set_pixels(rect{ point{}, size{ static_cast<dimension>(cx), static_cast<dimension>(cy) } }, bgra.data(), static_cast<std::uint32_t>(cx));
                existing = iParts.emplace(key, texture).first;
            }
            aGc.draw_texture(rect{ point{ std::floor(aRect.x), std::floor(aRect.y) }, size{ static_cast<dimension>(cx), static_cast<dimension>(cy) } }, *existing->second);
        }

        void skin::draw_part_true_size(i_graphics_context& aGc, HTHEME aTheme, int aPart, int aState, rect const& aRect) const
        {
            SIZE partSize{};
            if (FAILED(::GetThemePartSize(aTheme, nullptr, aPart, aState, nullptr, TS_TRUE, &partSize)) || partSize.cx <= 0 || partSize.cy <= 0)
            {
                draw_part(aGc, aTheme, aPart, aState, aRect);
                return;
            }
            size const extents{ static_cast<dimension>(partSize.cx), static_cast<dimension>(partSize.cy) };
            point const position{ 
                std::floor(aRect.x + (aRect.cx - extents.cx) / 2.0), 
                std::floor(aRect.y + (aRect.cy - extents.cy) / 2.0) };
            draw_part(aGc, aTheme, aPart, aState, rect{ position, extents });
        }

        void skin::close_themes() const
        {
            iParts.clear();
            for (auto const& t : iThemes)
                if (t.second != nullptr)
                    ::CloseThemeData(t.second);
            iThemes.clear();
        }
    }
}
