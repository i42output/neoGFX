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
#include <neogfx/game/mesh_renderer.hpp>
#include <neogfx/game/mesh_filter.hpp>

namespace neogfx::game
{
	canvas::canvas() : 
		iEcs{ std::make_shared<game::ecs>() }
	{
		init();
	}

	canvas::canvas(game::i_ecs& aEcs) :
		iEcs{ std::shared_ptr<game::i_ecs>{}, &aEcs }
	{
		init();
	}

	canvas::canvas(std::shared_ptr<game::i_ecs> aEcs) :
		iEcs{ aEcs }
	{
		init();
	}

	canvas::canvas(i_widget& aParent) :
		widget{ aParent }, 
		iEcs{ std::make_shared<game::ecs>() }
	{
		init();
	}

	canvas::canvas(i_widget& aParent, game::i_ecs& aEcs) :
		widget{ aParent },
		iEcs{ std::shared_ptr<game::i_ecs>{}, &aEcs }
	{
		init();
	}

	canvas::canvas(i_widget& aParent, std::shared_ptr<game::i_ecs> aEcs) :
		widget{ aParent },
		iEcs{ aEcs }
	{
		init();
	}

	canvas::canvas(i_layout& aLayout) :
		widget{ aLayout }, 
		iEcs{ std::make_shared<game::ecs>() }
	{
		init();
	}

	canvas::canvas(i_layout& aLayout, game::i_ecs& aEcs) :
		widget{ aLayout },
		iEcs{ std::shared_ptr<game::i_ecs>{}, &aEcs }
	{
		init();
	}

	canvas::canvas(i_layout& aLayout, std::shared_ptr<game::i_ecs> aEcs) :
		widget{ aLayout },
		iEcs{ aEcs }
	{
		init();
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
		if (ecs().component_registered<mesh_renderer>())
		{
			aGraphicsContext.clear_depth_buffer();
			component_lock_guard<rigid_body> lgRigidBody{ ecs() };
			component_lock_guard<mesh_renderer> lgMeshRenderer{ ecs() };
			rendering_entities.trigger(aGraphicsContext);
			for (auto entity : ecs().component<mesh_renderer>().entities())
				aGraphicsContext.draw_entity(ecs(), entity);
			aGraphicsContext.flush();
			entities_rendered.trigger(aGraphicsContext);
		}
	}

	void canvas::mouse_button_pressed(mouse_button aButton, const point& aPosition, key_modifiers_e)
	{
		if (aButton == mouse_button::Left)
		{
			// todo
		}
	}

	void canvas::init()
	{
		iSink += ecs().physics_applied([this](step_time)
		{
			update();
		});
	}
}