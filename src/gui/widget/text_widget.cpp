// text_widget.cpp
/*
  neogfx C++ GUI Library
  Copyright (c) 2015-present, Leigh Johnston.  All Rights Reserved.
  
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
#include <neogfx/app/app.hpp>
#include <neogfx/gfx/graphics_context.hpp>
#include <neogfx/gui/layout/i_layout.hpp>
#include <neogfx/gui/widget/text_widget.hpp>

namespace neogfx
{
	text_widget::text_widget(const std::string& aText, text_widget_type aType, text_widget_flags aFlags) :
		widget{}, iText{ aText }, iGlyphTextCache{ neogfx::font{} }, iType{ aType }, iFlags{ aFlags }, iAlignment {	neogfx::alignment::Centre | neogfx::alignment::VCentre
	}
	{
		init();
	}

	text_widget::text_widget(i_widget& aParent, const std::string& aText, text_widget_type aType, text_widget_flags aFlags) :
		widget{ aParent }, iText{ aText }, iGlyphTextCache{ neogfx::font{} }, iType{ aType }, iFlags{ aFlags }, iAlignment{ neogfx::alignment::Centre | neogfx::alignment::VCentre }
	{
		init();
	}

	text_widget::text_widget(i_layout& aLayout, const std::string& aText, text_widget_type aType, text_widget_flags aFlags) :
		widget{ aLayout }, iText{ aText }, iGlyphTextCache{ neogfx::font{} }, iType{ aType }, iFlags{ aFlags }, iAlignment{ neogfx::alignment::Centre | neogfx::alignment::VCentre }
	{
		init();
	}

	text_widget::~text_widget()
	{
	}

	neogfx::size_policy text_widget::size_policy() const
	{
		if (widget::has_size_policy())
			return widget::size_policy();
		return neogfx::size_policy::Minimum;
	}

	size text_widget::minimum_size(const optional_size& aAvailableSpace) const
	{
		if (widget::has_minimum_size())
			return widget::minimum_size(aAvailableSpace);
		else
		{
			size extent = units_converter(*this).to_device_units(text_extent().max(size_hint_extent()));
			size result = extent + units_converter(*this).to_device_units(margins().size());
			if (has_maximum_size())
			{
				result.cx = std::min(std::ceil(result.cx), maximum_size().cx);
				result.cy = std::min(std::ceil(result.cy), maximum_size().cy);
			}
			if (result.cx == 0.0 && (flags() & text_widget_flags::TakesSpaceWhenEmpty) == text_widget_flags::TakesSpaceWhenEmpty)
				result.cx = 1.0;
			return units_converter(*this).from_device_units(result);
		}
	}

	void text_widget::paint(graphics_context& aGraphicsContext) const
	{
		scoped_mnemonics sm(aGraphicsContext, app::instance().keyboard().is_key_pressed(ScanCode_LALT) || app::instance().keyboard().is_key_pressed(ScanCode_RALT));
		if (iGlyphTextCache.font() != font())
		{
			iTextExtent = boost::none;
			iGlyphTextCache = glyph_text{ font() };
		}
		aGraphicsContext.set_glyph_text_cache(iGlyphTextCache);
		size textSize = text_extent();
		point textPosition;
		switch (iAlignment & neogfx::alignment::Horizontal)
		{
		case neogfx::alignment::Left:
		case neogfx::alignment::Justify:
			textPosition.x = 0.0;
			break;
		case neogfx::alignment::Centre:
			textPosition.x = std::floor((client_rect().width() - textSize.cx) / 2.0);
			break;
		case neogfx::alignment::Right:
			textPosition.x = std::floor((client_rect().width() - textSize.cx));
			break;
		default:
			break;
		}
		switch (iAlignment & neogfx::alignment::Vertical)
		{
		case neogfx::alignment::Top:
			textPosition.y = 0.0;
			break;
		case neogfx::alignment::VCentre:
			textPosition.y = std::floor((client_rect().height() - textSize.cy) / 2.0);
			break;
		case neogfx::alignment::Bottom:
			textPosition.y = std::floor((client_rect().height() - textSize.cy));
			break;
		default:
			break;
		}
		auto appearance = text_appearance();
		if (effectively_disabled())
			appearance = appearance.with_alpha(static_cast<colour::component>(appearance.ink().alpha() / 2));
		if (multi_line())
			aGraphicsContext.draw_multiline_text(textPosition, text(), font(), textSize.cx, appearance, iAlignment & neogfx::alignment::Horizontal, UseGlyphTextCache);
		else
			aGraphicsContext.draw_text(textPosition, text(), font(), appearance, UseGlyphTextCache);
	}

	void text_widget::set_font(const optional_font& aFont)
	{
		widget::set_font(aFont);
		iTextExtent = boost::none;
		iSizeHintExtent = boost::none;
		iGlyphTextCache = glyph_text(font());
	}

	bool text_widget::visible() const
	{
		if (iText.empty() && (iFlags & text_widget_flags::HideOnEmpty) == text_widget_flags::HideOnEmpty)
			return false;
		return widget::visible();
	}

	const std::string& text_widget::text() const
	{
		return iText;
	}

	void text_widget::set_text(const std::string& aText)
	{
		if (iText != aText)
		{
			size oldSize = minimum_size();
			iText = aText;
			iTextExtent = boost::none;
			iGlyphTextCache = glyph_text{ font() };
			text_changed.trigger();
			if (has_parent_layout())
				parent_layout().invalidate();
			if (oldSize != minimum_size() && has_managing_layout())
				managing_layout().layout_items();
			update();
		}
	}

	void text_widget::set_size_hint(const std::string& aSizeHint)
	{
		if (iSizeHint != aSizeHint)
		{
			size oldSize = minimum_size();
			iSizeHint = aSizeHint;
			iSizeHintExtent = boost::none;
			if (has_parent_layout())
				parent_layout().invalidate();
			if (oldSize != minimum_size() && has_managing_layout())
				managing_layout().layout_items();
		}
	}

	bool text_widget::multi_line() const
	{
		return iType == text_widget_type::MultiLine;
	}

	text_widget_flags text_widget::flags() const
	{
		return iFlags;
	}
	
	void text_widget::set_flags(text_widget_flags aFlags)
	{
		iFlags = aFlags;
	}

	neogfx::alignment text_widget::alignment() const
	{
		return iAlignment;
	}

	void text_widget::set_alignment(neogfx::alignment aAlignment, bool aUpdateLayout)
	{
		if (iAlignment != aAlignment)
		{
			iAlignment = aAlignment;
			if (aUpdateLayout)
				ultimate_ancestor().layout_items(true);
		}
	}

	bool text_widget::has_text_colour() const
	{
		return has_text_appearance() && text_appearance().ink() != boost::none && text_appearance().ink().is<colour>();
	}

	colour text_widget::text_colour() const
	{
		if (has_text_colour())
			return static_variant_cast<colour>(iTextAppearance->ink());
		optional_colour textColour;
		const i_widget* w = 0;
		do
		{
			if (w == 0)
				w = this;
			else
				w = &w->parent();
			if (w->has_background_colour())
			{
				textColour = w->background_colour().luma() >= 0.5 ? colour::Black : colour::White;
				break;
			}
			else if (w->has_foreground_colour())
			{
				textColour = w->foreground_colour().luma() >= 0.5 ? colour::Black : colour::White;
				break;
			}
		} while (w->has_parent());
		colour defaultTextColour = app::instance().current_style().palette().text_colour();
		if (textColour == boost::none || textColour->similar_intensity(defaultTextColour))
			textColour = defaultTextColour;
		return *textColour;
	}

	void text_widget::set_text_colour(const optional_colour& aTextColour)
	{
		if (has_text_appearance())
			set_text_appearance(neogfx::text_appearance{ aTextColour != boost::none ? *aTextColour : neogfx::text_colour{}, iTextAppearance->paper(), iTextAppearance->effect() });
		else
			set_text_appearance(neogfx::text_appearance{ aTextColour != boost::none ? *aTextColour : neogfx::text_colour{} });
	}

	bool text_widget::has_text_appearance() const
	{
		return iTextAppearance != boost::none;
	}

	text_appearance text_widget::text_appearance() const
	{
		if (has_text_appearance())
			return *iTextAppearance;
		return neogfx::text_appearance{ text_colour() };
	}

	void text_widget::set_text_appearance(const optional_text_appearance& aTextAppearance)
	{
		iTextAppearance = aTextAppearance;
		update();
	}

	size text_widget::text_extent() const
	{
		if (iGlyphTextCache.font() != font())
		{
			iTextExtent = boost::none;
			iSizeHintExtent = boost::none;
			iGlyphTextCache = glyph_text{ font() };
		}
		if (iTextExtent != boost::none)
			return *iTextExtent;
		else if (!has_surface())
			return size{};
		graphics_context gc{ *this, graphics_context::type::Unattached };
		scoped_mnemonics sm{ gc, app::instance().keyboard().is_key_pressed(ScanCode_LALT) || app::instance().keyboard().is_key_pressed(ScanCode_RALT) };
		gc.set_glyph_text_cache(iGlyphTextCache);
		if (multi_line())
		{
			if (widget::has_minimum_size() && widget::minimum_size().cx != 0 && widget::minimum_size().cy == 0)
				iTextExtent = gc.multiline_text_extent(iText, font(), widget::minimum_size().cx - margins().size().cx, UseGlyphTextCache);
			else if (widget::has_maximum_size() && widget::maximum_size().cx != size::max_dimension())
				iTextExtent = gc.multiline_text_extent(iText, font(), widget::maximum_size().cx - margins().size().cx, UseGlyphTextCache);
			else
				iTextExtent = gc.multiline_text_extent(iText, font(), UseGlyphTextCache);
		}
		else
			iTextExtent = gc.text_extent(iText, font(), UseGlyphTextCache);
		return *iTextExtent;
	}

	size text_widget::size_hint_extent() const
	{
		if (iGlyphTextCache.font() != font())
		{
			iTextExtent = boost::none;
			iSizeHintExtent = boost::none;
			iGlyphTextCache = glyph_text{ font() };
		}
		if (iSizeHintExtent != boost::none)
			return *iSizeHintExtent;
		else if (!has_surface())
			return size{};
		else
		{
			graphics_context gc{ *this, graphics_context::type::Unattached };
			scoped_mnemonics sm{ gc, app::instance().keyboard().is_key_pressed(ScanCode_LALT) || app::instance().keyboard().is_key_pressed(ScanCode_RALT) };
			if (multi_line())
			{
				if (widget::has_minimum_size() && widget::minimum_size().cx != 0 && widget::minimum_size().cy == 0)
					iSizeHintExtent = gc.multiline_text_extent(iSizeHint, font(), widget::minimum_size().cx - margins().size().cx, DontUseGlyphTextCache);
				else if (widget::has_maximum_size() && widget::maximum_size().cx != size::max_dimension())
					iSizeHintExtent = gc.multiline_text_extent(iSizeHint, font(), widget::maximum_size().cx - margins().size().cx, DontUseGlyphTextCache);
				else
					iSizeHintExtent = gc.multiline_text_extent(iSizeHint, font(), DontUseGlyphTextCache);
			}
			else
				iSizeHintExtent = gc.text_extent(iSizeHint, font(), DontUseGlyphTextCache);
		}
		return *iSizeHintExtent;
	}

	void text_widget::init()
	{
		set_margins(neogfx::margins{ 0.0 });
		set_ignore_mouse_events(true);
		iSink += app::instance().current_style_changed([this](style_aspect aAspect)
		{
			if (!has_font() && (aAspect & style_aspect::Font) == style_aspect::Font)
			{
				iTextExtent = boost::none;
				iSizeHintExtent = boost::none;
				iGlyphTextCache = glyph_text{ font() };
				if (has_parent_layout())
					parent_layout().invalidate();
				update();
			}
		});
		iSink += app::instance().rendering_engine().subpixel_rendering_changed([this]()
		{
			iTextExtent = boost::none;
			iSizeHintExtent = boost::none;
			iGlyphTextCache = glyph_text{ font() };
			if (has_parent_layout())
				parent_layout().invalidate();
			update();
		});
	}
}