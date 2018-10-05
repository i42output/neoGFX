// canvas.cpp
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

#include <neogfx/neogfx.hpp>
#include <neogfx/game/canvas.hpp>

namespace neogfx::game
{
	canvas::canvas() : 
		iEcs{ std::make_shared<game::ecs>() }
	{
	}

	canvas::canvas(game::i_ecs& aEcs) :
		iEcs{ std::shared_ptr<game::i_ecs>{}, &aEcs }
	{
	}

	canvas::canvas(std::shared_ptr<game::i_ecs> aEcs) :
		iEcs{ aEcs }
	{
	}

	canvas::canvas(i_widget& aParent) :
		widget{ aParent }, 
		iEcs{ std::make_shared<game::ecs>() }
	{
	}

	canvas::canvas(i_widget& aParent, game::i_ecs& aEcs) :
		widget{ aParent },
		iEcs{ std::shared_ptr<game::i_ecs>{}, &aEcs }
	{
	}

	canvas::canvas(i_widget& aParent, std::shared_ptr<game::i_ecs> aEcs) :
		widget{ aParent },
		iEcs{ aEcs }
	{
	}

	canvas::canvas(i_layout& aLayout) :
		widget{ aLayout }, 
		iEcs{ std::make_shared<game::ecs>() }
	{
	}

	canvas::canvas(i_layout& aLayout, game::i_ecs& aEcs) :
		widget{ aLayout },
		iEcs{ std::shared_ptr<game::i_ecs>{}, &aEcs }
	{
	}

	canvas::canvas(i_layout& aLayout, std::shared_ptr<game::i_ecs> aEcs) :
		widget{ aLayout },
		iEcs{ aEcs }
	{
	}

	canvas::~canvas()
	{
	}

	game::i_ecs& canvas::ecs() const
	{
		return *iEcs;
	}

	logical_coordinate_system canvas::logical_coordinate_system() const
	{
		if (widget::has_logical_coordinate_system())
			return widget::logical_coordinate_system();
		return neogfx::logical_coordinate_system::AutomaticGame;
	}

	void canvas::paint(graphics_context& aGraphicsContext) const
	{	
		aGraphicsContext.clear_depth_buffer();
		painting_entities.trigger(aGraphicsContext);
		// todo
		/* sort_shapes();
		for (auto s : iRenderBuffer)
		{
			if (s->killed())
				continue;
			if (s->bounding_box_2d().intersection(client_rect()).empty())
				continue;
			s->paint(aGraphicsContext);
		} */
		aGraphicsContext.flush();
		entities_painted.trigger(aGraphicsContext);
	}

	void canvas::mouse_button_pressed(mouse_button aButton, const point& aPosition, key_modifiers_e)
	{
		if (aButton == mouse_button::Left)
		{
			// todo
		}
	}
}