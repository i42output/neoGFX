// color_dialog.hpp
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
#include <neogfx/gfx/color.hpp>
#include <neogfx/gui/dialog/dialog.hpp>
#include <neogfx/gui/widget/group_box.hpp>
#include <neogfx/gui/widget/radio_button.hpp>
#include <neogfx/gui/widget/drop_list.hpp>
#include <neogfx/gui/widget/spin_box.hpp>

namespace neogfx
{
    class color_dialog : public dialog
    {
    public:
        define_event(SelectionChanged, selection_changed)
    protected:
        define_event(ColorSpaceChanged, color_space_changed)
    public:
        enum mode_e
        {
            ModeHSV,
            ModeRGB
        };
        enum channel_e
        {
            ChannelHue,
            ChannelSaturation,
            ChannelValue,
            ChannelRed,
            ChannelGreen,
            ChannelBlue,
            ChannelAlpha
        };
        typedef std::array<optional_color, 24> custom_color_list;
    private:
        typedef neolib::variant<color, hsv_color> representations;
        typedef std::optional<custom_color_list::iterator> optional_custom_color_list_iterator;
        class color_box : public framed_widget<>
        {
            typedef framed_widget<> base_type;
        public:
            color_box(color_dialog& aOwner, const optional_color& aColor, const optional_custom_color_list_iterator& aCustomColor = optional_custom_color_list_iterator());
        public:
            virtual size minimum_size(optional_size const& aAvailableSpace = optional_size{}) const;
            virtual size maximum_size(optional_size const& aAvailableSpace = optional_size{}) const;
        public:
            virtual void paint(i_graphics_context& aGc) const;
        public:
            virtual void mouse_button_pressed(mouse_button aButton, const point& aPosition, key_modifiers_e aKeyModifiers);
        private:
            color_dialog& iOwner;
            optional_color iColor;
            optional_custom_color_list_iterator iCustomColor;
        };
        class x_picker : public framed_widget<>
        {
            typedef framed_widget<> base_type;
        private:
            class cursor_widget : public image_widget
            {
            public:
                enum type_e
                {
                    LeftCursor,
                    RightCursor
                };
            public:
                cursor_widget(x_picker& aParent, type_e aType);
            public:
                void mouse_button_pressed(mouse_button aButton, const point& aPosition, key_modifiers_e aKeyModifiers) override;
                void mouse_button_released(mouse_button aButton, const point& aPosition) override;
                void mouse_moved(const point& aPosition, key_modifiers_e aKeyModifiers) override;
            private:
                x_picker& iParent;
                optional_point iDragOffset;
            };
        public:
            x_picker(color_dialog& aOwner);
        public:
            scalar cursor_width() const;
        public:
            size minimum_size(optional_size const& aAvailableSpace = optional_size{}) const override;
            size maximum_size(optional_size const& aAvailableSpace = optional_size{}) const override;
        public:
            void moved() override;
            void resized() override;
        public:
            void paint(i_graphics_context& aGc) const override;
        public:
            void mouse_button_pressed(mouse_button aButton, const point& aPosition, key_modifiers_e aKeyModifiers) override;
            void mouse_button_released(mouse_button aButton, const point& aPosition) override;
            void mouse_moved(const point& aPosition, key_modifiers_e aKeyModifiers) override;
        public:
            neogfx::mouse_cursor mouse_cursor() const override;
        public:
            using framed_widget::effective_frame_width;
        private:
            void select(const point& aPosition);
            representations color_at_position(const point& aCursorPos) const;
            void update_cursors();
            point current_cursor_position() const;
        private:
            color_dialog& iOwner;
            sink iSink;
            bool iTracking;
            cursor_widget iLeftCursor;
            cursor_widget iRightCursor;
        };
        class yz_picker : public framed_scrollable_widget
        {
        public:
            yz_picker(color_dialog& aOwner);
        public:
            void set_image(image&& aImage);
            void clear_image();
        public:
            void mouse_button_pressed(mouse_button aButton, const point& aPosition, key_modifiers_e aKeyModifiers) override;
            void mouse_button_released(mouse_button aButton, const point& aPosition) override;
            void mouse_moved(const point& aPosition, key_modifiers_e aKeyModifiers) override;
        public:
            neogfx::mouse_cursor mouse_cursor() const override;
        private:
            void select(const point& aPosition);
            representations color_at_position(const point& aCursorPos) const;
            point current_cursor_position() const;
            void update_texture();
        private:
            color_dialog& iOwner;
            vertical_layout iLayout;
            std::optional<image> iImage;
            mutable std::array<std::array<avec4u8, 256>, 256> iPixels;
            image_widget iCanvas;
            mutable texture iTexture;
            bool iTracking;
            std::optional<point> iCursorPosition;
        };
        class color_selection : public framed_widget<>
        {
            typedef framed_widget<> base_type;
        public:
            color_selection(color_dialog& aOwner);
        public:
            size minimum_size(optional_size const& aAvailableSpace = optional_size{}) const override;
            size maximum_size(optional_size const& aAvailableSpace = optional_size{}) const override;
        public:
            void paint(i_graphics_context& aGc) const override;
        private:
            color_dialog& iOwner;
        };
    public:
        color_dialog(const color& aCurrentColor = color::Black);
        color_dialog(i_widget& aParent, const color& aCurrentColor = color::Black);
        ~color_dialog();
    public:
        color current_color() const;
        color selected_color() const;
        hsv_color selected_color_as_hsv() const;
        void select_color(const color& aColor);
        const custom_color_list& custom_colors() const;
        void set_custom_colors(const custom_color_list& aCustomColors);
    protected:
        void mouse_button_pressed(mouse_button aButton, const point& aPosition, key_modifiers_e aKeyModifiers) override;
    private:
        void init();
        scalar color_space_coefficient() const;
        void change_color_space(color_space aColorSpace);
        mode_e current_mode() const;
        channel_e current_channel() const;
        void set_current_channel(channel_e aChannel);
        hsv_color selected_color_as_hsv(bool aChangeRepresentation) const;
        vec4 selected_color_in_color_space() const;
        void select_color(const representations& aColor, const i_widget& aUpdatingWidget);
        void select_color_in_color_space(const vec4& aColor, const i_widget& aUpdatingWidget);
        custom_color_list::iterator current_custom_color() const;
        void set_current_custom_color(custom_color_list::iterator aCustomColor);
        void update_widgets(const i_widget& aUpdatingWidget);
    private:
        sink iSink;
        channel_e iCurrentChannel;
        color iCurrentColor;
        mutable representations iSelectedColor;
        custom_color_list iCustomColors;
        custom_color_list::iterator iCurrentCustomColor;
        bool iUpdatingWidgets;
        bool iScreenPickerActive;
        vertical_layout iLayout;
        horizontal_layout iLayout2;
        vertical_layout iLeftLayout;
        vertical_layout iRightLayout;
        horizontal_layout iRightTopLayout;
        horizontal_layout iRightBottomLayout;
        color_selection iColorSelection;
        push_button iScreenPicker;
        horizontal_spacer iSpacer0;
        grid_layout iChannelLayout;
        group_box iBasicColorsGroup;
        grid_layout iBasicColorsGrid;
        vertical_spacer iSpacer;
        group_box iCustomColorsGroup;
        grid_layout iCustomColorsGrid;
        horizontal_spacer iSpacer2;
        yz_picker iYZPicker;
        x_picker iXPicker;
        horizontal_layout iModelLayout;
        horizontal_spacer iSpacer3;
        std::optional<color_space> iColorSpace;
        drop_list iColorSpaceSelector;
        std::pair<radio_button, double_spin_box> iH;
        std::pair<radio_button, double_spin_box> iS;
        std::pair<radio_button, double_spin_box> iV;
        std::pair<radio_button, double_spin_box> iR;
        std::pair<radio_button, double_spin_box> iG;
        std::pair<radio_button, double_spin_box> iB;
        std::pair<radio_button, double_spin_box> iA;
        line_edit iRgb;
        push_button iAddToCustomColors;
    };
}