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

#include <neogfx/neogfx.hpp>
#include <card_games/card_sprite.hpp>
#include <video_poker/card_space.hpp>

#include <neogfx/game/entity_info.hpp>

namespace video_poker
{
    card_widget::card_widget(neogfx::i_layout& aLayout, neogfx::game::canvas& aCanvas, const i_card_textures& aCardTextures) :
        widget{ aLayout },
        iCanvas{ aCanvas },
        iCardTextures{ aCardTextures },
        iCard{ nullptr },
        iCardSprite{ neogfx::game::null_entity }
    {
        set_margins(neogfx::margins{});
        set_size_policy(neogfx::size_constraint::Expanding, kBridgeCardSize);
        set_ignore_mouse_events(true);
        iCanvas.LayoutCompleted([this]() { update_sprite_geometry(); });
        iCanvas.EntityClicked([this](neogfx::game::entity_id aEntity)
        { 
            if (aEntity == iCardSprite)
                toggle_hold(); 
        });
    }

    neogfx::size card_widget::minimum_size(const neogfx::optional_size& aAvailableSpace) const
    {
        return rasterize(neogfx::from_mm(kBridgeCardSize * 0.5));
    }

    neogfx::size card_widget::maximum_size(const neogfx::optional_size& aAvailableSpace) const
    {
        return rasterize(minimum_size(aAvailableSpace) * 2.0);
    }
        
    void card_widget::paint(neogfx::i_graphics_context& aGc) const
    {
        auto rect = client_rect();
        aGc.fill_rounded_rect(rect, rect.cx / 10.0, neogfx::color::DarkGreen);
        rect.deflate(neogfx::size{ 4.0 });
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
        iSink += card().changed([this](video_poker::card&) { update_sprite_geometry();  iCanvas.update(); });
        if (iCardSprite != neogfx::game::null_entity)
            iCanvas.ecs().destroy_entity(iCardSprite);
        iCardSprite = create_card_sprite(iCanvas.ecs(), iCardTextures, aCard);
        update_sprite_geometry();
        iCanvas.update();
    }

    void card_widget::clear_card()
    {
        iCard = nullptr;
        if (iCardSprite != neogfx::game::null_entity)
        {
            iCanvas.ecs().destroy_entity(iCardSprite);
            iCardSprite = neogfx::game::null_entity;
        }
        iCanvas.update();
    }

    void card_widget::update_sprite_geometry()
    {
        if (iCardSprite != neogfx::game::null_entity)
        {
            auto xy = iCanvas.to_client_coordinates(to_window_coordinates(client_rect().center()));
            if (iCard->discarded())
                xy += neogfx::point{ -8.0, -16.0 };
            auto& meshFilter = iCanvas.ecs().component<neogfx::game::mesh_filter>().entity_record(iCardSprite);
            meshFilter.transformation = neogfx::mat44{ 
                { extents().cx, 0.0, 0.0, 0.0 },
                { 0.0, extents().cy * kBridgeCardSize.cx / kBridgeCardSize.cy, 0.0, 0.0 },
                { 0.0, 0.0, 1.0, 0.0 },
                { xy.x, xy.y, 0.8, 1.0 } };
            neogfx::game::set_render_cache_dirty(iCanvas.ecs(), iCardSprite);
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

    card_space::card_space(neogfx::i_layout& aLayout, neogfx::game::canvas& aCanvas, i_table& aTable) :
        widget{ aLayout },
        iCanvas{ aCanvas },
        iTable{ aTable },
        iVerticalLayout{ *this, neogfx::alignment::Center | neogfx::alignment::VCenter },
        iCardWidget{ iVerticalLayout, aCanvas, aTable.textures() },
        iHoldButton{ iVerticalLayout, "HOLD\n CANCEL " },
        iCard{ nullptr }
    {
        set_size_policy(neogfx::size_constraint::ExpandingUniform);
        iVerticalLayout.set_spacing(neogfx::size{ 8.0 });
        set_ignore_mouse_events(true);
        iHoldButton.set_size_policy(neogfx::size_constraint::Minimum);
        iHoldButton.set_foreground_color(neogfx::color::Black);
        iHoldButton.text_widget().set_font(neogfx::font{ "Exo 2", "Black", 16.0 });
        iHoldButton.text_widget().set_text_appearance(neogfx::text_appearance{ neogfx::color::White, neogfx::text_effect{ neogfx::text_effect_type::Outline, neogfx::color::Black.with_alpha(0.5) } });
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
        iHoldButton.set_foreground_color(has_card() && !card().discarded() && iTable.state() == table_state::DealtFirst ? neogfx::color::LightYellow1 : neogfx::color::Black.with_alpha(0.5));
        iHoldButton.enable(has_card() && iTable.state() == table_state::DealtFirst);
        iHoldButton.set_checked(has_card() && !card().discarded() && iTable.state() == table_state::DealtFirst);
    }
}