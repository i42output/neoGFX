// text_mesh.cpp
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

#include <neogfx/game/rectangle.hpp>
#include <neogfx/game/text_mesh.hpp>

namespace neogfx::game
{
    namespace shape
    {
        text::text(i_ecs& aEcs, i_graphics_context const& aGc, i_string const& aText, const neogfx::font& aFont, const neogfx::text_format& aTextFormat, neogfx::alignment aAlignment) :
            entity{ aEcs, archetype().id() }
        {
            neogfx::game::scoped_component_data_lock<game::text_mesh> lock{ aEcs };
            auto const& font = aEcs.shared_component<game::font>().populate(to_string(neolib::generate_uuid()).to_std_string(), game::font{{service<i_font_manager>(), aFont.id()}, aFont.family_name(), aFont.style_name(), aFont.size(), aFont.underline()});
            auto& textMesh = aEcs.component<game::text_mesh>().populate(id(), game::text_mesh
                {
                    aText,
                    {},
                    {},
                    {},
                    aAlignment,
                    { font },
                    { to_ecs_component(aTextFormat.ink()) },
                    aTextFormat.effect() ? aTextFormat.effect()->type() : text_effect_type::None,
                    { aTextFormat.effect() ? to_ecs_component(aTextFormat.effect()->color()) : game::material{} },
                    aTextFormat.effect() ? static_cast<float>(aTextFormat.effect()->width()) : 0.0f
                });
            game::text_mesh::meta::update(textMesh, aEcs, aGc, id());
        }

        text::text(const text& aOther) :
            entity{ aOther.ecs(), archetype().id() }
        {
            // todo: clone entity
        }

        text::~text()
        {
        }
    }
}