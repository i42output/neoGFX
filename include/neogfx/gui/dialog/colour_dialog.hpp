// colour_dialog.hpp
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
#include <neogfx/core/colour.hpp>
#include <neogfx/gui/dialog/dialog.hpp>
#include <neogfx/gui/widget/group_box.hpp>
#include <neogfx/gui/widget/radio_button.hpp>
#include <neogfx/gui/widget/spin_box.hpp>

namespace neogfx
{
	class colour_dialog : public dialog
	{
	public:
		event<> selection_changed;
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
		typedef std::array<colour, 24> custom_colour_list;
	private:
		typedef neolib::variant<colour, hsv_colour> representations;
		typedef std::optional<custom_colour_list::iterator> optional_custom_colour_list_iterator;
		class colour_box : public framed_widget
		{
		public:
			colour_box(colour_dialog& aOwner, const colour& aColour, const optional_custom_colour_list_iterator& aCustomColour = optional_custom_colour_list_iterator());
		public:
			virtual size minimum_size(const optional_size& aAvailableSpace = optional_size()) const;
			virtual size maximum_size(const optional_size& aAvailableSpace = optional_size()) const;
		public:
			virtual void paint(graphics_context& aGraphicsContext) const;
		public:
			virtual void mouse_button_pressed(mouse_button aButton, const point& aPosition, key_modifiers_e aKeyModifiers);
		private:
			colour_dialog& iOwner;
			colour iColour;
			optional_custom_colour_list_iterator iCustomColour;
		};
		class x_picker : public framed_widget
		{
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
				virtual void mouse_button_pressed(mouse_button aButton, const point& aPosition, key_modifiers_e aKeyModifiers);
				virtual void mouse_button_released(mouse_button aButton, const point& aPosition);
				virtual void mouse_moved(const point& aPosition);
			private:
				x_picker& iParent;
				optional_point iDragOffset;
			};
		public:
			x_picker(colour_dialog& aOwner);
		public:
			virtual size minimum_size(const optional_size& aAvailableSpace = optional_size()) const;
			virtual size maximum_size(const optional_size& aAvailableSpace = optional_size()) const;
		public:
			virtual void moved();
			virtual void resized();
		public:
			virtual void paint(graphics_context& aGraphicsContext) const;
		public:
			virtual void mouse_button_pressed(mouse_button aButton, const point& aPosition, key_modifiers_e aKeyModifiers);
			virtual void mouse_button_released(mouse_button aButton, const point& aPosition);
			virtual void mouse_moved(const point& aPosition);
		public:
			using framed_widget::effective_frame_width;
		private:
			void select(const point& aPosition);
			representations colour_at_position(const point& aCursorPos) const;
			void update_cursors();
			point x_picker::current_cursor_position() const;
		private:
			colour_dialog& iOwner;
			sink iSink;
			bool iTracking;
			cursor_widget iLeftCursor;
			cursor_widget iRightCursor;
		};
		class yz_picker : public framed_widget
		{
		public:
			yz_picker(colour_dialog& aOwner);
		public:
			virtual size minimum_size(const optional_size& aAvailableSpace = optional_size()) const;
			virtual size maximum_size(const optional_size& aAvailableSpace = optional_size()) const;
		public:
			virtual void paint(graphics_context& aGraphicsContext) const;
		public:
			virtual void mouse_button_pressed(mouse_button aButton, const point& aPosition, key_modifiers_e aKeyModifiers);
			virtual void mouse_button_released(mouse_button aButton, const point& aPosition);
			virtual void mouse_moved(const point& aPosition);
		private:
			void select(const point& aPosition);
			representations colour_at_position(const point& aCursorPos) const;
			point current_cursor_position() const;
		private:
			colour_dialog& iOwner;
			mutable std::array<std::array<std::array<uint8_t, 4>, 256>, 256> iPixels;
			mutable texture iTexture;
			mutable bool iUpdateTexture;
			bool iTracking;
		};
		class colour_selection : public framed_widget
		{
		public:
			colour_selection(colour_dialog& aOwner);
		public:
			virtual size minimum_size(const optional_size& aAvailableSpace = optional_size()) const;
			virtual size maximum_size(const optional_size& aAvailableSpace = optional_size()) const;
		public:
			virtual void paint(graphics_context& aGraphicsContext) const;
		private:
			colour_dialog& iOwner;
		};
	public:
		colour_dialog(const colour& aCurrentColour = colour::Black);
		colour_dialog(i_widget& aParent, const colour& aCurrentColour = colour::Black);
		~colour_dialog();
	public:
		colour current_colour() const;
		colour selected_colour() const;
		hsv_colour selected_colour_as_hsv() const;
		void select_colour(const colour& aColour);
		const custom_colour_list& custom_colours() const;
		custom_colour_list& custom_colours();
	protected:
		void mouse_button_pressed(mouse_button aButton, const point& aPosition, key_modifiers_e aKeyModifiers) override;
		neogfx::mouse_cursor mouse_cursor() const override;
	private:
		void init();
		mode_e current_mode() const;
		channel_e current_channel() const;
		void set_current_channel(channel_e aChannel);
		hsv_colour selected_colour_as_hsv(bool aChangeRepresentation) const;
		void select_colour(const representations& aColour, const i_widget& aUpdatingWidget);
		custom_colour_list::iterator current_custom_colour() const;
		void set_current_custom_colour(custom_colour_list::iterator aCustomColour);
		void update_widgets(const i_widget& aUpdatingWidget);
	private:
		sink iSink;
		channel_e iCurrentChannel;
		colour iCurrentColour;
		mutable representations iSelectedColour;
		custom_colour_list iCustomColours;
		custom_colour_list::iterator iCurrentCustomColour;
		bool iUpdatingWidgets;
		bool iScreenPickerActive;
		vertical_layout iLayout;
		horizontal_layout iLayout2;
		vertical_layout iLeftLayout;
		vertical_layout iRightLayout;
		horizontal_layout iRightTopLayout;
		horizontal_layout iRightBottomLayout;
		colour_selection iColourSelection;
		push_button iScreenPicker;
		grid_layout iChannelLayout;
		group_box iBasicColoursGroup;
		grid_layout iBasicColoursGrid;
		vertical_spacer iSpacer;
		group_box iCustomColoursGroup;
		grid_layout iCustomColoursGrid;
		yz_picker iYZPicker;
		x_picker iXPicker;
		std::pair<radio_button, double_spin_box> iH;
		std::pair<radio_button, double_spin_box> iS;
		std::pair<radio_button, double_spin_box> iV;
		std::pair<radio_button, spin_box> iR;
		std::pair<radio_button, spin_box> iG;
		std::pair<radio_button, spin_box> iB;
		std::pair<radio_button, spin_box> iA;
		line_edit iRgb;
		push_button iAddToCustomColours;
	};
}