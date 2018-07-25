// widget_bits.hpp
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

namespace neogfx
{
	enum class widget_part : uint32_t
	{
		Client,
		NonClientTitleBar,
		NonClientGrab,
		NonClient,
		NonClientBorder,
		NonClientBorderLeft,
		NonClientBorderTopLeft,
		NonClientBorderTop,
		NonClientBorderTopRight,
		NonClientBorderRight,
		NonClientBorderBottomRight,
		NonClientBorderBottom,
		NonClientBorderBottomLeft,
		NonClientGrowBox,
		NonClientCloseButton,
		NonClientMaximizeButton,
		NonClientMinimizeButton,
		NonClientRestoreButton,
		NonClientMenu,
		NonClientSystemMenu,
		NonClientVerticalScrollbar,
		NonClientHorizontalScrollbar,
		NonClientOther,
		Nowhere,
		NowhereError,
		Nested = 0x10000000
	};

	inline constexpr widget_part operator|(widget_part aLhs, widget_part aRhs)
	{
		return static_cast<widget_part>(static_cast<uint32_t>(aLhs) | static_cast<uint32_t>(aRhs));
	}

	inline constexpr widget_part operator&(widget_part aLhs, widget_part aRhs)
	{
		return static_cast<widget_part>(static_cast<uint32_t>(aLhs) & static_cast<uint32_t>(aRhs));
	}

	inline constexpr widget_part operator|=(widget_part& aLhs, widget_part aRhs)
	{
		return aLhs = static_cast<widget_part>(static_cast<uint32_t>(aLhs) | static_cast<uint32_t>(aRhs));
	}

	inline constexpr widget_part operator&=(widget_part& aLhs, widget_part aRhs)
	{
		return aLhs = static_cast<widget_part>(static_cast<uint32_t>(aLhs) & static_cast<uint32_t>(aRhs));
	}

	inline bool capture_ok(widget_part aWidgetPart)
	{
		switch (aWidgetPart)
		{
		case widget_part::Client:
		case widget_part::NonClientVerticalScrollbar:
		case widget_part::NonClientHorizontalScrollbar:
		case widget_part::NonClientCloseButton:
		case widget_part::NonClientMaximizeButton:
		case widget_part::NonClientMinimizeButton:
		case widget_part::NonClientRestoreButton: 
			return true;
		default:
			return false;
		}
	}

	enum class focus_policy : uint32_t
	{
		NoFocus				= 0x00000000,
		ClickFocus			= 0x00000001,
		TabFocus			= 0x00000002,
		ClickTabFocus		= ClickFocus | TabFocus,
		StrongFocus			= ClickTabFocus,
		WheelFocus			= 0x00000004, // todo
		PointerFocus		= 0x00000008, // todo
		ConsumeTabKey		= 0x10000000,
		ConsumeReturnKey	= 0x20000000,
		IgnoreNonClient		= 0x40000000
	};

	enum class focus_event
	{
		FocusGained,
		FocusLost
	};

	enum class focus_reason
	{
		ClickNonClient,
		ClickClient,
		Tab,
		Wheel,
		Pointer,
		WindowActivation,
		Other
	};

	enum class capture_reason
	{
		MouseEvent,
		Other
	};
	inline focus_policy operator|(focus_policy aLhs, focus_policy aRhs)
	{
		return static_cast<focus_policy>(static_cast<uint32_t>(aLhs) | static_cast<uint32_t>(aRhs));
	}

	inline focus_policy& operator|=(focus_policy& aLhs, focus_policy aRhs)
	{
		aLhs = static_cast<focus_policy>(static_cast<uint32_t>(aLhs) | static_cast<uint32_t>(aRhs));
		return aLhs;
	}

	inline focus_policy operator&(focus_policy aLhs, focus_policy aRhs)
	{
		return static_cast<focus_policy>(static_cast<uint32_t>(aLhs) & static_cast<uint32_t>(aRhs));
	}

	inline focus_policy& operator&=(focus_policy& aLhs, focus_policy aRhs)
	{
		aLhs = static_cast<focus_policy>(static_cast<uint32_t>(aLhs) & static_cast<uint32_t>(aRhs));
		return aLhs;
	}

	inline focus_policy operator~(focus_policy aLhs)
	{
		return static_cast<focus_policy>(~static_cast<uint32_t>(aLhs));
	}
}