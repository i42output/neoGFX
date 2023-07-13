/*
neogfx C++ App/Game Engine - Examples - Games - Video Poker
Copyright(C) 2017 Leigh Johnston

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

#include <video_poker/video_poker.hpp>

#include <neogfx/game/entity_info.hpp>

#include <card_games/card_sprite.hpp>
#include <video_poker/card_space.hpp>

namespace video_poker
{
    card_widget::card_widget(ng::i_layout& aLayout, ng::game::canvas& aCanvas, const i_card_textures& aCardTextures) :
        widget<>{ aLayout },
        iCanvas{ aCanvas },
        iCardTextures{ aCardTextures },
        iCard{ nullptr },
        iCardSprite{ ng::game::null_entity }
    {
        set_padding(ng::padding{});
        set_size_policy(ng::size_constraint::Expanding, ng::visibility_constraint::Ignore, kBridgeCardSize);
        set_ignore_mouse_events(true);
        iCanvas.LayoutCompleted([this]() { update_sprite_geometry(); });
        iCanvas.EntityClicked([this](ng::game::entity_id aEntity)
        { 
            if (aEntity == iCardSprite)
                toggle_hold(); 
        });
    }

    ng::size card_widget::minimum_size(const ng::optional_size& aAvailableSpace) const
    {
        return ceil_rasterized(ng::from_mm(kBridgeCardSize * 0.5));
    }

    ng::size card_widget::maximum_size(const ng::optional_size& aAvailableSpace) const
    {
        return ceil_rasterized(minimum_size(aAvailableSpace) * 2.0);
    }
        
    void card_widget::paint(ng::i_graphics_context& aGc) const
    {
        auto rect = client_rect();
        aGc.fill_rounded_rect(rect, rect.cx / 10.0, ng::color::DarkGreen);
        rect.deflate(ng::size{ 4.0 });
        aGc.fill_rounded_rect(rect, rect.cx / 10.0, background_color());
    }

    bool card_widget::has_card() const
    {
        return iCard != nullptr;
    }

    card& card_widget::card() const
    {
        return *iCard;
    }

    void card_widget::set_card(video_poker::card& aCard)
    {
        iCard = &aCard;
        iSink.clear();
        iSink += card().changed([this](video_poker::card&) 
        { 
                update_sprite_geometry();  
                iCanvas.update(); 
        });
        if (iCardSprite != ng::game::null_entity)
            iCanvas.ecs().destroy_entity(iCardSprite);
        iCardSprite = create_card_sprite(iCanvas.ecs(), iCardTextures, aCard);
        update_sprite_geometry();
        iCanvas.update();
    }

    void card_widget::clear_card()
    {
        iCard = nullptr;
        if (iCardSprite != ng::game::null_entity)
        {
            iCanvas.ecs().destroy_entity(iCardSprite);
            iCardSprite = ng::game::null_entity;
        }
        iCanvas.update();
    }

    void card_widget::update_sprite_geometry()
    {
        if (iCardSprite != ng::game::null_entity)
        {
            auto xy = iCanvas.to_client_coordinates(to_window_coordinates(client_rect().center()));
            if (iCard->discarded())
                xy += ng::point{ -8.0, -16.0 };
            auto& meshFilter = iCanvas.ecs().component<ng::game::mesh_filter>().entity_record(iCardSprite);
            meshFilter.transformation = ng::mat44{ 
                { extents().cx, 0.0, 0.0, 0.0 },
                { 0.0, extents().cy * kBridgeCardSize.cx / kBridgeCardSize.cy, 0.0, 0.0 },
                { 0.0, 0.0, 1.0, 0.0 },
                { xy.x, xy.y, 0.8, 1.0 } };
            ng::game::set_render_cache_dirty(iCanvas.ecs(), iCardSprite);
            iCanvas.update();
        }
    }

    void card_widget::toggle_hold()
    {
        if (has_card())
        {
            if (!card().discarded())
                card().discard();
            else
                card().undiscard();
        }
    }

    card_space::card_space(ng::i_layout& aLayout, ng::game::canvas& aCanvas, i_table& aTable) :
        widget<>{ aLayout },
        iCanvas{ aCanvas },
        iTable{ aTable },
        iVerticalLayout{ *this, ng::alignment::Center | ng::alignment::VCenter },
        iCardWidget{ iVerticalLayout, aCanvas, aTable.textures() },
        iHoldButton{ iVerticalLayout, "HOLD\n CANCEL " },
        iCard{ nullptr }
    {
        set_size_policy(ng::size_constraint::ExpandingUniform);
        set_ignore_mouse_events(true);
        iVerticalLayout.set_spacing(ng::size{ 8.0 });
        iHoldButton.set_consider_ancestors_for_mouse_events(false);
        iHoldButton.set_size_policy(ng::size_constraint::Minimum);
        iHoldButton.set_base_color(ng::color::Black);
        iHoldButton.text_widget().set_font(ng::font{ "Exo 2", "Black", 16.0 });
        iHoldButton.text_widget().set_text_format(ng::text_format{ ng::color::White, ng::text_effect{ ng::text_effect_type::Outline, ng::color::Black.with_alpha(0.5) } });
        iHoldButton.set_checkable();
        auto update_hold = [this]() 
        { 
            if (has_card() && iTable.state() == table_state::DealtFirst)
            {
                if (iHoldButton.is_checked())
                    card().undiscard();
                else
                    card().discard();
                update_widgets();
            }
        };
        iHoldButton.Toggled(update_hold);
        iTable.state_changed([this](table_state) { update_widgets(); });
        update_widgets();
    }

    bool card_space::has_card() const
    {
        return iCard != nullptr;
    }

    const video_poker::card& card_space::card() const
    {
        if (has_card())
            return *iCard;
        throw no_card();
    }

    video_poker::card& card_space::card()
    {
        if (has_card())
            return *iCard;
        throw no_card();
    }

    void card_space::set_card(video_poker::card& aCard)
    {
        iCard = &aCard;
        iCardWidget.set_card(card());
        iSink.clear();
        iSink += card().changed([this](video_poker::card&) { update_widgets(); });
        iSink += card().destroyed([this](video_poker::card&) { clear_card(); });
        update_widgets();
    }

    void card_space::clear_card()
    {
        iCard = nullptr;
        iCardWidget.clear_card();
        update_widgets();
    }

    void card_space::update_widgets()
    {
        iCardWidget.enable(has_card() && iTable.state() == table_state::DealtFirst);
        iHoldButton.set_base_color(has_card() && !card().discarded() && iTable.state() == table_state::DealtFirst ? ng::color::LightYellow1 : ng::color::Black.with_alpha(0.5));
        iHoldButton.enable(has_card() && iTable.state() == table_state::DealtFirst);
        iHoldButton.set_checked(has_card() && !card().discarded() && iTable.state() == table_state::DealtFirst);
    }
}