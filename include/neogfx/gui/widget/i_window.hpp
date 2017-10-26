// i_window.hpp
/*
  neogfx C++ GUI Library
  Copyright(C) 2017 Leigh Johnston
  
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
#include <neogfx/gui/window/window_events.hpp>

namespace neogfx
{
	class i_widget;
	class i_layout;

	enum class window_style : uint32_t
	{
		Invalid =				0x00000000,
		None =					0x00000001,	// No decoration at all(useful for splash screens, for example); this style cannot be combined with others
		TitleBar =				0x00000002,	// The window has a titlebar
		NativeTitleBar =		0x00000004,	// The window has a native titlebar
		SystemMenu =			0x00000008,
		MinimizeBox =			0x00000010,
		MaximizeBox =			0x00000020,
		Resize =				0x00000040,	// The window can be resized and has a maximize button
		Close =					0x00000080,	// The window has a close button
		Fullscreen =			0x00000100,	// The window is shown in fullscreen mode; this style cannot be combined with others, and requires a valid video mode
		Modal =					0x00010000,
		ApplicationModal =		0x00020000,
		NoActivate =			0x00040000,
		RequiresOwnerFocus =	0x00080000,
		DismissOnOwnerClick =	0x00100000,
		DismissOnParentClick =	0x00200000,
		HideOnOwnerClick =		0x00400000,
		HideOnParentClick =		0x00800000,
		InitiallyHidden =		0x01000000,
		Weak =					0x10000000,
		Default = TitleBar | SystemMenu | MinimizeBox | MaximizeBox | Resize | Close
	};

	inline constexpr window_style operator|(window_style aLhs, window_style aRhs)
	{
		return static_cast<window_style>(static_cast<uint32_t>(aLhs) | static_cast<uint32_t>(aRhs));
	}

	inline constexpr window_style operator&(window_style aLhs, window_style aRhs)
	{
		return static_cast<window_style>(static_cast<uint32_t>(aLhs) & static_cast<uint32_t>(aRhs));
	}

	struct window_placement
	{
		enum state_e
		{
			Iconized,
			Restored,
			Maximized
		} state;
		rect iconizedGeometry;
		rect restoredGeometry;
	};

	class i_window
	{
	public:
		event<window_event&> window_event;
	public:
		virtual window_style style() const = 0;
		virtual void set_style(window_style aStyle) = 0;
	public:
		virtual const std::string& title_text() const = 0;
		virtual void set_title_text(const std::string& aTitleText) = 0;
	public:
		virtual bool is_active() const = 0;
		virtual void activate() = 0;
		virtual bool is_iconic() const = 0;
		virtual void iconize() = 0;
		virtual bool is_maximized() const = 0;
		virtual void maximize() = 0;
		virtual bool is_restored() const = 0;
		virtual void restore() = 0;
		virtual window_placement placement() const = 0;
		virtual void set_placement(const window_placement& aPlacement) = 0;
		virtual void centre() = 0;
		virtual void centre_on_parent() = 0;
		virtual bool window_enabled() const = 0;
		virtual void counted_window_enable(bool aEnable) = 0;
	public:
		virtual const i_layout& non_client_layout() const = 0;
		virtual i_layout& non_client_layout() = 0;
		virtual const i_layout& title_bar_layout() const = 0;
		virtual i_layout& title_bar_layout() = 0;
		virtual const i_layout& menu_layout() const = 0;
		virtual i_layout& menu_layout() = 0;
		virtual const i_layout& toolbar_layout() const = 0;
		virtual i_layout& toolbar_layout() = 0;
		virtual const i_layout& client_layout() const = 0;
		virtual i_layout& client_layout() = 0;
		virtual const i_layout& status_bar_layout() const = 0;
		virtual i_layout& status_bar_layout() = 0;
	public:
		virtual const i_widget& as_widget() const = 0;
		virtual i_widget& as_widget() = 0;
	};
}