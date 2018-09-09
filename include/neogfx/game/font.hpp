// font.hpp
/*
  neogfx C++ GUI Library
  Copyright (c) 2018 Leigh Johnston.  All Rights Reserved.
  
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
#include <neolib/uuid.hpp>
#include <neolib/string.hpp>
#include <neogfx/game/ecs_ids.hpp>
#include <neogfx/game/i_ecs.hpp>
#include <neogfx/game/i_component_data.hpp>
#include <neogfx/gfx/text/font_manager.hpp>
#include <neogfx/app/app.hpp>

namespace neogfx::game
{
	struct font
	{
		string familyName;
		string styleName;
		scalar pointSize;
		bool underline;
		id_t id;

		struct meta : i_component_data::meta
		{
			static const neolib::uuid& id()
			{
				static const neolib::uuid sId = { 0x13edb5d4, 0x9fae, 0x4dc1, 0xb9e4, { 0x15, 0x49, 0x8b, 0xf, 0xff, 0xe } };
				return sId;
			}
			static const neolib::i_string& name()
			{
				static const neolib::string sName = "Font";
				return sName;
			}
			static uint32_t field_count()
			{
				return 5;
			}
			static component_data_field_type field_type(uint32_t aFieldIndex)
			{
				switch (aFieldIndex)
				{
				case 0:
				case 1:
					return component_data_field_type::String;
				case 2:
					return component_data_field_type::Scalar;
				case 3:
					return component_data_field_type::Bool;
				case 4:
					return component_data_field_type::Id;
				default:
					throw invalid_field_index();
				}
			}
			static const neolib::i_string& field_name(uint32_t aFieldIndex)
			{
				static const neolib::string sFieldNames[] =
				{
					"Family Name",
					"Style Name",
					"Point Size",
					"Underline",
					"Id"
				};
				return sFieldNames[aFieldIndex];
			}
			static constexpr bool has_updater = true;
			static void update(font& aData, i_ecs& aEcs, entity_id)
			{
				auto& fontManager = app::instance().rendering_engine().font_manager();
				neogfx::font_info fontInfo{ aData.familyName, aData.styleName, aData.pointSize };
				fontInfo.set_underline(aData.underline);
				if (aData.id != null_id)
					fontManager.return_token(aEcs.update_handle<neogfx::font>(aData.id, neogfx::font{ fontInfo }.get_token()));
				else
					aData.id = aEcs.add_handle<neogfx::font>(neogfx::font{ fontInfo }.get_token());
			}
			static constexpr bool has_handles = true;
			static void free_handles(font& aData, i_ecs& aEcs)
			{
				if (aData.id != null_id)
				{
					auto& fontManager = app::instance().rendering_engine().font_manager();
					fontManager.return_token(aEcs.release_handle<neogfx::font::token>(aData.id));
					aData.id = null_id;
				}
			}
		};
	};
}