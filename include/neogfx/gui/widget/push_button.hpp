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

#include <neogfx/neogfx.hpp>
#include <neolib/timer.hpp>
#include "button.hpp"

namespace neogfx
{
	enum class push_button_style
	{
		Normal,
		ButtonBox,
		ItemViewHeader,
		Tab,
		DropList,
		SpinBox,
		Toolbar
	};

	class push_button : public button
	{
	public:
		static const uint32_t kMaxAnimationFrame = 10;
	public:
		push_button(const std::string& aText = std::string(), push_button_style aStyle = push_button_style::Normal);
		push_button(i_widget& aParent, const std::string& aText = std::string(), push_button_style aStyle = push_button_style::Normal);
		push_button(i_layout& aLayout, const std::string& aText = std::string(), push_button_style aStyle = push_button_style::Normal);
	public:
		size minimum_size(const optional_size& aAvailableSpace = optional_size()) const override;
		size maximum_size(const optional_size& aAvailableSpace = optional_size()) const override;
	public:
		void paint_non_client(graphics_context& aGraphicsContext) const override;
		void paint(graphics_context& aGraphicsContext) const override;
	public:
		void mouse_entered() override;
		void mouse_left() override;
	protected:
		virtual rect path_bounding_rect() const;
		virtual neogfx::path path() const;
		virtual bool spot_colour() const;
		virtual colour border_mid_colour() const;
		virtual bool perform_hover_animation() const;
		virtual bool has_hover_colour() const;
		virtual colour hover_colour() const;
		virtual void set_hover_colour(const optional_colour& aHoverColour = optional_colour());
		virtual void animate();
		virtual colour animation_colour() const;
		virtual colour animation_colour(uint32_t aAnimationFrame) const;
	private:
		neolib::callback_timer iAnimator;
		uint32_t iAnimationFrame;
		push_button_style iStyle;
		optional_colour iHoverColour;
		mutable boost::optional<std::pair<neogfx::font, size>> iStandardButtonWidth;
	};
}