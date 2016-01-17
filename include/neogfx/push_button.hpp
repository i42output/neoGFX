// push_button.hpp
/*
  neogfx C++ GUI Library
  Copyright(C) 2016 Leigh Johnston
  
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

#include "neogfx.hpp"
#include <neolib/timer.hpp>
#include "button.hpp"

namespace neogfx
{
	class push_button : public button
	{
	public:
		enum style_e
		{
			ButtonStyleNormal,
			ButtonStyleItemViewHeader
		};
		static const uint32_t kMaxAnimationFrame = 20;
	public:
		push_button(const std::string& aText = std::string(), style_e aStyle = ButtonStyleNormal);
		push_button(i_widget& aParent, const std::string& aText = std::string(), style_e aStyle = ButtonStyleNormal);
		push_button(i_layout& aLayout, const std::string& aText = std::string(), style_e aStyle = ButtonStyleNormal);
	public:
		virtual void paint(graphics_context& aGraphicsContext) const;
	public:
		virtual void mouse_entered();
		virtual void mouse_left();
	private:
		path get_path() const;
		bool has_hover_colour() const;
		colour hover_colour() const;
		void set_hover_colour(const optional_colour& aHoverColour = optional_colour());
		void animate();
		colour animation_colour() const;
		colour animation_colour(uint32_t aAnimationFrame) const;
	private:
		neolib::callback_timer iAnimator;
		uint32_t iAnimationFrame;
		style_e iStyle;
		optional_colour iHoverColour;
	};
}