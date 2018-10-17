// canvas.hpp
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
#include <neogfx/gui/widget/widget.hpp>
#include <neogfx/game/ecs.hpp>

namespace neogfx::game
{
	class canvas : public widget
	{
	public:
		event<graphics_context&> rendering_entities;
		event<graphics_context&> entities_rendered;
		event<entity_id> entity_clicked;
	public:
		canvas();
		canvas(game::i_ecs& aEcs);
		canvas(std::shared_ptr<game::i_ecs> aEcs);
		canvas(i_widget& aParent);
		canvas(i_widget& aParent, game::i_ecs& aEcs);
		canvas(i_widget& aParent, std::shared_ptr<game::i_ecs> aEcs);
		canvas(i_layout& aLayout);
		canvas(i_layout& aLayout, game::i_ecs& aEcs);
		canvas(i_layout& aLayout, std::shared_ptr<game::i_ecs> aEcs);
		~canvas();
	public:
		game::i_ecs& ecs() const;
	public:
		virtual neogfx::logical_coordinate_system logical_coordinate_system() const;
		virtual void paint(graphics_context& aGraphicsContext) const;
	public:
		virtual void mouse_button_pressed(mouse_button aButton, const point& aPosition, key_modifiers_e aKeyModifiers);
	private:
		void init();
	private:
		std::shared_ptr<game::i_ecs> iEcs;
		sink iSink;
	};
}