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
#include <neogfx/app/i_app.hpp>
#include <neogfx/game/canvas.hpp>
#include <neogfx/game/mesh_renderer.hpp>
#include <neogfx/game/mesh_filter.hpp>
#include <neogfx/game/game_world.hpp>

namespace neogfx::game
{
    canvas::canvas() : 
        iEcs{ std::make_shared<game::ecs>() },
        iEcsPaused{ false }
    {
        init();
    }

    canvas::canvas(game::i_ecs& aEcs) :
        iEcs{ std::shared_ptr<game::i_ecs>{}, &aEcs },
        iEcsPaused{ false }
    {
        init();
    }

    canvas::canvas(std::shared_ptr<game::i_ecs> aEcs) :
        iEcs{ aEcs },
        iEcsPaused{ false }
    {
        init();
    }

    canvas::canvas(i_widget& aParent) :
        widget{ aParent }, 
        iEcs{ std::make_shared<game::ecs>() },
        iEcsPaused{ false }
    {
        init();
    }

    canvas::canvas(i_widget& aParent, game::i_ecs& aEcs) :
        widget{ aParent },
        iEcs{ std::shared_ptr<game::i_ecs>{}, &aEcs },
        iEcsPaused{ false }
    {
        init();
    }

    canvas::canvas(i_widget& aParent, std::shared_ptr<game::i_ecs> aEcs) :
        widget{ aParent },
        iEcs{ aEcs },
        iEcsPaused{ false }
    {
        init();
    }

    canvas::canvas(i_layout& aLayout) :
        widget{ aLayout }, 
        iEcs{ std::make_shared<game::ecs>() },
        iEcsPaused{ false }
    {
        init();
    }

    canvas::canvas(i_layout& aLayout, game::i_ecs& aEcs) :
        widget{ aLayout },
        iEcs{ std::shared_ptr<game::i_ecs>{}, &aEcs },
        iEcsPaused{ false }
    {
        init();
    }

    canvas::canvas(i_layout& aLayout, std::shared_ptr<game::i_ecs> aEcs) :
        widget{ aLayout },
        iEcs{ aEcs },
        iEcsPaused{ false }
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

    void canvas::paint(i_graphics_context& aGc) const
    {    
        if (ecs().component_registered<mesh_renderer>())
        {
            aGc.clear_depth_buffer();
            component_scoped_lock<mesh_renderer> lgMeshRenderer{ ecs() };
            RenderingEntities.trigger(aGc);
            aGc.draw_entities(ecs());
            EntitiesRendered.trigger(aGc);
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
        iUpdater.emplace(service<async_task>(), [this](neolib::callback_timer& aTimer)
        {
            aTimer.again();
            if (!iEcsPaused && effectively_hidden())
            {
                ecs().pause_all_systems();
                iEcsPaused = true;
            }
            else if (iEcsPaused && effectively_visible())
            {
                ecs().resume_all_systems();
                iEcsPaused = false;
            }
        }, 1000u);

        iSink += ecs().system<game_world>().PhysicsApplied([this](step_time)
        {
            update();
        });
    }
}