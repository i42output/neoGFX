// text_widget.hpp
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
#include "widget.hpp"

namespace neogfx
{
	enum class text_widget_type
	{
		SingleLine,
		MultiLine
	};

	enum class text_widget_flags
	{
		None = 0x00,
		HideOnEmpty = 0x01,
		TakesSpaceWhenEmpty = 0x02
	};

	inline constexpr text_widget_flags operator|(text_widget_flags aLhs, text_widget_flags aRhs)
	{
		return static_cast<text_widget_flags>(static_cast<uint32_t>(aLhs) | static_cast<uint32_t>(aRhs));
	}

	inline constexpr text_widget_flags operator&(text_widget_flags aLhs, text_widget_flags aRhs)
	{
		return static_cast<text_widget_flags>(static_cast<uint32_t>(aLhs) & static_cast<uint32_t>(aRhs));
	}

	class text_widget : public widget
	{
	public:
		event<> text_changed;
	public:
		text_widget(const std::string& aText = std::string{}, text_widget_type aType = text_widget_type::SingleLine, text_widget_flags aFlags = text_widget_flags::None);
		text_widget(i_widget& aParent, const std::string& aText = std::string{}, text_widget_type aType = text_widget_type::SingleLine, text_widget_flags aFlags = text_widget_flags::None);
		text_widget(i_layout& aLayout, const std::string& aText = std::string{}, text_widget_type aType = text_widget_type::SingleLine, text_widget_flags aFlags = text_widget_flags::None);
		~text_widget();
	public:
		neogfx::size_policy size_policy() const override;
		size minimum_size(const optional_size& aAvailableSpace = optional_size()) const override;
	public:
		void paint(graphics_context& aGraphicsContext) const override;
	public:
		void set_font(const optional_font& aFont) override;
	public:
		bool visible() const override;
	public:
		const std::string& text() const;
		void set_text(const std::string& aText);
		void set_size_hint(const std::string& aSizeHint);
		bool multi_line() const;
		text_widget_flags flags() const;
		void set_flags(text_widget_flags aFlags);
		neogfx::alignment alignment() const;
		void set_alignment(neogfx::alignment aAlignment, bool aUpdateLayout = true);
		bool has_text_colour() const;
		colour text_colour() const;
		void set_text_colour(const optional_colour& aTextColour);
		bool has_text_appearance() const;
		neogfx::text_appearance text_appearance() const;
		void set_text_appearance(const optional_text_appearance& aTextAppearance);
	protected:
		size text_extent() const;
		size size_hint_extent() const;
	private:
		void init();
	private:
		sink iSink;
		std::string iText;
		mutable glyph_text iGlyphTextCache;
		mutable optional_size iTextExtent;
		std::string iSizeHint;
		mutable optional_size iSizeHintExtent;
		text_widget_type iType;
		text_widget_flags iFlags;
		neogfx::alignment iAlignment;
		optional_text_appearance iTextAppearance;
	};
}