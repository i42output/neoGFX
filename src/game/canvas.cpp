// canvas.cpp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2015, 2020 Leigh Johnston.  All Rights Reserved.
  
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
#include <neogfx/game/animator.hpp>
#include <neogfx/game/collision_detector.hpp>
#include <neogfx/game/game_world.hpp>

namespace neogfx::game
{
    canvas::canvas() : 
        iLayers{ true },
        iEcsPaused{ false }
    {
        init();
    }

    canvas::canvas(game::i_ecs& aEcs) :
        iLayers{ true },
        iEcsPaused{ false }
    {
        init();
        set_ecs(aEcs);
    }

    canvas::canvas(std::shared_ptr<game::i_ecs> aEcs) :
        iLayers{ true },
        iEcsPaused{ false }
    {
        init();
        set_ecs(aEcs);
    }

    canvas::canvas(i_widget& aParent) :
        widget{ aParent }, 
        iLayers{ true },
        iEcsPaused{ false }
    {
        init();
    }

    canvas::canvas(i_widget& aParent, game::i_ecs& aEcs) :
        widget{ aParent },
        iLayers{ true },
        iEcsPaused{ false }
    {
        init();
        set_ecs(aEcs);
    }

    canvas::canvas(i_widget& aParent, std::shared_ptr<game::i_ecs> aEcs) :
        widget{ aParent },
        iLayers{ true },
        iEcsPaused{ false }
    {
        init();
        set_ecs(aEcs);
    }

    canvas::canvas(i_layout& aLayout) :
        widget{ aLayout }, 
        iLayers{ true },
        iEcsPaused{ false }
    {
        init();
    }

    canvas::canvas(i_layout& aLayout, game::i_ecs& aEcs) :
        widget{ aLayout },
        iLayers{ true },
        iEcsPaused{ false }
    {
        init();
        set_ecs(aEcs);
    }

    canvas::canvas(i_layout& aLayout, std::shared_ptr<game::i_ecs> aEcs) :
        widget{ aLayout },
        iLayers{ true },
        iEcsPaused{ false }
    {
        init();
        set_ecs(aEcs);
    }

    canvas::~canvas()
    {
    }

    bool canvas::have_ecs() const
    {
        return iEcs != nullptr;
    }

    game::i_ecs& canvas::ecs() const
    {
        if (have_ecs())
            return *iEcs;
        throw no_ecs();
    }

    void canvas::set_ecs(game::i_ecs& aEcs)
    {
        set_ecs(std::shared_ptr<game::i_ecs>{ std::shared_ptr<game::i_ecs>{}, &aEcs });
    }

    void canvas::set_ecs(const std::shared_ptr<game::i_ecs>& aEcs)
    {
        if (iEcs == aEcs)
            return;
        iSink.clear();
        iEcs = aEcs;
        if (have_ecs())
        {
            iEcsPaused = ecs().all_systems_paused();
            iSink += !ecs().system<game_world>().PhysicsApplied([this](step_time)
            {
                update();
            });
            iSink += !ecs().system<animator>().Animate([this](step_time)
            {
                update();
            });
        }
    }

    scene_layer canvas::layers()
    {
        return static_cast<scene_layer>(iLayers.size());
    }

    void canvas::set_layers(scene_layer aLayers)
    {
        iLayers.resize(aLayers, true);
    }

    bool canvas::layer_visible(scene_layer aLayer) const
    {
        if (aLayer >= iLayers.size())
            throw invalid_layer();
        return iLayers[aLayer];
    }

    void canvas::show_layer(scene_layer aLayer)
    {
        if (aLayer >= iLayers.size())
            throw invalid_layer();
        iLayers[aLayer] = true;
    }

    void canvas::hide_layer(scene_layer aLayer)
    {
        if (aLayer >= iLayers.size())
            throw invalid_layer();
        iLayers[aLayer] = false;
    }
        
    logical_coordinate_system canvas::logical_coordinate_system() const
    {
        if (widget::has_logical_coordinate_system())
            return widget::logical_coordinate_system();
        return neogfx::logical_coordinate_system::AutomaticGame;
    }

    void canvas::mouse_button_clicked(mouse_button aButton, const point& aPosition, key_modifier aKeyModifier)
    {
        widget::mouse_button_clicked(aButton, aPosition, aKeyModifier);
        if (aButton == mouse_button::Left && EntityClicked.has_slots())
        {
            /// @todo 3D
            ecs().system<collision_detector_2d>().run_cycle(collision_detection_cycle::Test);
            auto const hit = ecs().system<collision_detector_2d>().entity_at(aPosition.to_vec3());
            if (hit)
                EntityClicked(*hit);
        }
    }

    void canvas::mouse_button_double_clicked(mouse_button aButton, const point& aPosition, key_modifier aKeyModifier)
    {
        widget::mouse_button_double_clicked(aButton, aPosition, aKeyModifier);
        if (aButton == mouse_button::Left && (EntityClicked.has_slots() || EntityDoubleClicked.has_slots()))
        {
            /// @todo 3D
            ecs().system<collision_detector_2d>().run_cycle(collision_detection_cycle::Test);
            auto const hit = ecs().system<collision_detector_2d>().entity_at(aPosition.to_vec3());
            if (hit)
            {
                if (EntityDoubleClicked.has_slots())
                    EntityDoubleClicked(*hit);
                else
                    EntityClicked(*hit);
            }
        }
    }

    void canvas::init()
    {
        iUpdater.emplace(*this, [this](widget_timer& aTimer)
        {
            aTimer.again();
            if (!have_ecs())
                return;
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
        }, std::chrono::milliseconds{ 1000 });

        Painting([this](i_graphics_context& aGc)
        {
            if (have_ecs() && ecs().component_registered<mesh_renderer>())
            {
                aGc.clear_depth_buffer();
                scoped_component_data_lock<mesh_renderer> lgMeshRenderer{ ecs() };
                RenderingEntities(aGc, 0);
                aGc.draw_entities(ecs());
                EntitiesRendered(aGc, 0);
            }
        });

        Painted([this](i_graphics_context& aGc)
        {
            if (have_ecs() && ecs().component_registered<mesh_renderer>())
            {
                scoped_component_data_lock<mesh_renderer> lgMeshRenderer{ ecs() };
                for (scene_layer layer = 1; layer < layers(); ++layer)
                {
                    if (!layer_visible(layer))
                        continue;
                    RenderingEntities(aGc, layer);
                    aGc.draw_entities(ecs(), layer);
                    EntitiesRendered(aGc, layer);
                }
            }
        });
    }
}