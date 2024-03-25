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

#include <boost/lexical_cast.hpp>

#include <video_poker/video_poker.hpp>

#include <neogfx/gui/dialog/message_box.hpp>
#include <neogfx/gfx/graphics_context.hpp>
#include <neogfx/gfx/i_texture_manager.hpp>
#include <neogfx/game/ecs.hpp>
#include <neogfx/game/clock.hpp>
#include <neogfx/game/rigid_body.hpp>
#include <neogfx/game/mesh_renderer.hpp>
#include <neogfx/game/text_mesh.hpp>
#include <neogfx/game/simple_physics.hpp>
#include <neogfx/game/entity_life_span.hpp>

#include <video_poker/table.hpp>
#include <video_poker/poker.hpp>

using namespace ng::unit_literals;

namespace video_poker
{
    using namespace neogames::card_games;

    const credit_t STARTING_CREDIT = 10;
    const credit_t MAX_BET = 100;

    const std::map<poker_hand, credit_t> PAY_TABLE = 
    {
        { Pair, 1 },
        { TwoPair, 2 },
        { ThreeOfAKind, 3 },
        { Straight, 4 },
        { Flush, 6 },
        { FullHouse, 9 },
        { FourOfAKind, 25 },
        { StraightFlush, 50 },
        { RoyalFlush, 250 }
    };

    class outcome : public ng::game::shape::text
    {
    public:
        outcome(ng::game::canvas& aCanvas, std::string const& aOutcome, const ng::color& aColor) :
            ng::game::shape::text{
                aCanvas.ecs(),
                ng::graphics_context{ aCanvas },
                aOutcome,
                ng::font{ "Exo 2", "Black", 48.0 }.with_outline({ 1.0_dp }),
                ng::text_format{aColor, ng::text_effect{ ng::text_effect_type::Outline, ng::color::Black } },
                ng::alignment::Center}
        {
            ng::game::scoped_component_lock<ng::game::mesh_renderer, ng::game::entity_info, ng::game::rigid_body> lock{ aCanvas.ecs() };
            aCanvas.ecs().component<ng::game::mesh_renderer>().entity_record(id()).layer = 1; 
            aCanvas.ecs().populate(id(), ng::game::entity_life_span{ ng::game::to_step_time(aCanvas.ecs(), 10.0) });
            auto const& boundingBox = ng::game::bounding_rect(*aCanvas.ecs().component<ng::game::mesh_filter>().entity_record(id()).mesh);
            aCanvas.ecs().populate(id(), ng::game::rigid_body{ ng::vec3{ (aCanvas.extents().cx - boundingBox.cx) / 2.0, (aCanvas.extents().cy - boundingBox.cy) / 2.0, 0.9 }, 1.0, ng::vec3{ 0.0, aCanvas.dpi_scale(-300.0), 0.0 } });
        }
    };
    
    table::table(ng::i_layout& aLayout) :
        ng::game::canvas{ aLayout, ng::game::make_ecs<ng::game::simple_physics>(ng::game::ecs_flags::Default | ng::game::ecs_flags::CreatePaused) },
        iState{ table_state::TakeBet },
        iCredit{ STARTING_CREDIT },
        iStake{ 0 },
        iMainLayout{ *this, ng::alignment::Center },
        iLabelTitle{ iMainLayout, "VIDEO POKER" },
        iSpacer1{ iMainLayout },
        iSpacesLayout{ iMainLayout },
        iSpacer2{ iSpacesLayout },
        iSpaces{
            std::make_shared<card_space>(iSpacesLayout, *this, *this),
            std::make_shared<card_space>(iSpacesLayout, *this, *this),
            std::make_shared<card_space>(iSpacesLayout, *this, *this),
            std::make_shared<card_space>(iSpacesLayout, *this, *this),
            std::make_shared<card_space>(iSpacesLayout, *this, *this) },
        iSpacer3{ iSpacesLayout },
        iSpacer4{ iMainLayout },
        iGambleLayout{ iMainLayout },
        iAddCredit{ iGambleLayout, ng::image{":/video_poker/resources/coin.png"} },
        iBetMinus{ iGambleLayout, "BET\n-" },
        iBetPlus{ iGambleLayout, "BET\n+" },
        iBetMax{ iGambleLayout, "MAX\nBET" },
        iSpacerGamble{ iGambleLayout },
        iDeal{ iGambleLayout, "DEAL" },
        iInfoBarLayout{ iMainLayout },
        iLabelCredits{ iInfoBarLayout, "Credits: " },
        iLabelCreditsValue{ iInfoBarLayout, "" },
        iSpacer5{ iInfoBarLayout },
        iLabelStake{ iInfoBarLayout, "Stake: " },
        iLabelStakeValue{ iInfoBarLayout, "" }
    {
        set_logical_coordinate_system(ng::logical_coordinate_system::AutomaticGui);

        set_layers(2);

        iMainLayout.set_spacing(ng::size{ 16.0 });
        iSpacesLayout.set_spacing(ng::size{ 16.0 });
        auto shiny_text = [](const ng::color& aColor)
        {
            return ng::text_format{
                ng::gradient{ { ng::color::Black, aColor, ng::color::Black } },
                ng::text_effect{ng::text_effect_type::Outline, ng::color::White } };
        };
        iLabelTitle.text_widget().set_font(ng::font{ "Exo 2", "Black", 48.0 }.with_outline({ 1.0_dp }));
        iLabelTitle.text_widget().set_text_format(shiny_text(ng::color::Green));
        iSpacer1.set_weight(ng::size{ 0.1 });
        iSpacer2.set_weight(ng::size{ 0.25 });
        iSpacer3.set_weight(ng::size{ 0.25 });
        iSpacer4.set_weight(ng::size{ 0.1 });
        iGambleLayout.set_size_policy(ng::size_policy{ ng::size_constraint::Expanding, ng::size_constraint::Minimum });
        iAddCredit.image_widget().set_minimum_size(ng::size{64.0_dip, 64.0_dip});
        iAddCredit.set_size_policy(ng::size_constraint::Expanding, ng::visibility_constraint::Consider, ng::size{ 1.0 });
        iAddCredit.set_weight(ng::size{ 0.0 });
        auto set_bet_button_appearance = [](ng::push_button& aButton)
        {
            aButton.set_size_policy(ng::size_constraint::Expanding, ng::visibility_constraint::Consider, ng::size{ 1.0 });
            aButton.set_weight(ng::size{ 0.0 });
            aButton.set_base_color(ng::color::White);
            aButton.text_widget().set_size_hint(ng::size_hint{ "MAX\nBET" });
            aButton.text_widget().set_text_color(ng::color::Black);
            aButton.text_widget().set_font(ng::font{ "Exo 2", "Black", 24.0 }.with_outline({ 1.0_dp }));
        };
        set_bet_button_appearance(iBetMinus);
        set_bet_button_appearance(iBetPlus);
        set_bet_button_appearance(iBetMax);
        set_bet_button_appearance(iDeal);
        ng::layout_as_same_size(iAddCredit, iBetMinus);
        iLabelCredits.text_widget().set_font(ng::font{ "Exo 2", "Black", 36.0 }.with_outline({ 1.0_dp }));
        iLabelCredits.text_widget().set_text_format(shiny_text(ng::color::Yellow));
        iLabelCreditsValue.text_widget().set_font(ng::font{ "Exo 2", "Black", 36.0 }.with_outline({ 1.0_dp }));
        iLabelCreditsValue.text_widget().set_text_format(shiny_text(ng::color::White));
        iLabelStake.text_widget().set_font(ng::font{ "Exo 2", "Black", 36.0 }.with_outline({ 1.0_dp }));
        iLabelStake.text_widget().set_text_format(shiny_text(ng::color::Yellow));
        iLabelStakeValue.text_widget().set_font(ng::font{ "Exo 2", "Black", 36.0 }.with_outline({ 1.0_dp }));
        iLabelStakeValue.text_widget().set_text_format(shiny_text(ng::color::White));

        iAddCredit.clicked([this]() { add_credit(STARTING_CREDIT); });
        iBetMinus.clicked([this]() { bet(-1); });
        iBetPlus.clicked([this]() { bet(+1); });
        iBetMax.clicked([this]() { bet(MAX_BET); });
        iDeal.clicked([this]() { deal(); });

        iTextures = ng::service<ng::i_rendering_engine>().texture_manager().create_texture_atlas();
        // Assumes font "Card Characters" by "Harold's Fonts" has been installed.
        auto const& values = { 'A', 'K', 'Q', 'J', '=', '9', '8', '7', '6', '5', '4', '3', '2', ' ' };
        ng::size const valueDimensions = { 72.0, 72.0 };
        auto& valueTextures = iTextures->create_sub_texture(ng::size{ valueDimensions.cx, values.size() * valueDimensions.cy }, 1.0, ng::texture_sampling::Multisample);
        ng::font valueFont{ "Card Characters", "Regular", -valueDimensions.cy };
        if (valueFont.family_name() != "Card Characters" &&
            ng::message_box::question(*this, "Setup", "Please install font \"CARD CHARACTERS\" by \"Harold's Fonts\" before playing neoGFX Video Poker", "Exit game?") == ng::standard_button::Yes)
            ng::app::instance().quit(EXIT_FAILURE);
        ng::graphics_context gcValue{ valueTextures };
        auto cursor = valueTextures.atlas_location().position();
        for (auto value : values)
        {
            gcValue.draw_text(cursor, std::string( 1, value ), valueFont, ng::color::White);
            cursor.y += valueDimensions.cy;
        }
        iValueTextures.emplace(card::value::Joker, ng::sub_texture{ valueTextures, ng::rect{ valueTextures.atlas_location().position(), valueDimensions } });
        for (auto v = card::value::Two; v <= card::value::Ace; v = static_cast<card::value>(static_cast<uint32_t>(v) + 1))
            iValueTextures.emplace(v, ng::sub_texture{ valueTextures, ng::rect{ valueTextures.atlas_location().position() + ng::point{0.0, (static_cast<uint32_t>(card::value::Ace) - static_cast<uint32_t>(v)) * valueDimensions.cy}, valueDimensions } });
        iSuitTextures.emplace(card::suit::Club, iTextures->create_sub_texture(ng::image{ ":/video_poker/resources/club.png" }));
        iSuitTextures.emplace(card::suit::Diamond, iTextures->create_sub_texture(ng::image{ ":/video_poker/resources/diamond.png" }));
        iSuitTextures.emplace(card::suit::Spade, iTextures->create_sub_texture(ng::image{ ":/video_poker/resources/spade.png" }));
        iSuitTextures.emplace(card::suit::Heart, iTextures->create_sub_texture(ng::image{ ":/video_poker/resources/heart.png" }));
        iFaceTextures.emplace(card::value::Jack, iTextures->create_sub_texture(ng::image{ ":/video_poker/resources/jack.png" }));
        iFaceTextures.emplace(card::value::Queen, iTextures->create_sub_texture(ng::image{ ":/video_poker/resources/queen.png" }));
        iFaceTextures.emplace(card::value::King, iTextures->create_sub_texture(ng::image{ ":/video_poker/resources/king.png" }));

        update_widgets();

        // Instantiate physics...
        ecs().system<ng::game::simple_physics>();
        // Instantiate non-default components...
        ecs().component<ng::game::text_mesh>();

        ecs().resume_all_systems();
    }

    table::~table()
    {
    }

    table_state table::state() const
    {
        return iState;
    }

    const i_card_textures& table::textures() const
    {
        return *this;
    }

    const ng::i_texture& table::value_texture(const card& aCard) const
    {
        auto vt = iValueTextures.find(aCard);
        if (vt != iValueTextures.end())
            return vt->second;
        throw texture_not_found();
    }

    const ng::i_texture& table::suit_texture(const card& aCard) const
    {
        auto st = iSuitTextures.find(aCard);
        if (st != iSuitTextures.end())
            return st->second;
        throw texture_not_found();
    }

    const ng::i_texture& table::face_texture(const card& aCard) const
    {
        if (aCard == card::value::Ace || aCard < card::value::Jack)
            return suit_texture(aCard);
        auto ft = iFaceTextures.find(aCard);
        if (ft != iFaceTextures.end())
            return ft->second;
        throw texture_not_found();
    }

    void table::add_credit(credit_t aCredit)
    {
        iCredit += aCredit;
        update_widgets();
    }

    void table::bet(credit_t aBet)
    {
        const credit_t minBet = -iStake;
        const credit_t maxBet = std::min(iCredit, MAX_BET - iStake);
        aBet = std::max(minBet, std::min(maxBet, aBet));
        if (aBet != 0)
        {
            iCredit -= aBet;
            iStake += aBet;
            update_widgets();
        }
    }

    void table::deal()
    {
        switch (iState)
        {
        case table_state::TakeBet:
            iDeck.emplace();
            iDeck->shuffle();
            iHand.emplace();
            iDeck->deal_hand(*iHand);
            for (std::size_t i = 0; i < 5; ++i)
            {
                auto& card = iHand->card_at(i);
                iSpaces[i]->set_card(card);
                card.discard();
            }
            change_state(table_state::DealtFirst);
            break;
        case table_state::DealtFirst:
            iDeck->exchange_cards(*iHand);
            for (std::size_t i = 0; i < 5; ++i)
                iSpaces[i]->set_card(iHand->card_at(i));
            change_state(table_state::DealtSecond);
            break;
        }
    }

    void table::win(credit_t aWinnings)
    {
        iOutcome = std::make_unique<outcome>(
            *this, 
            to_string(video_poker::to_poker_hand(*iHand)) + ng::to_string(u8"\nWIN £") + boost::lexical_cast<std::string>(aWinnings) + "!",
            ng::color::Goldenrod.with_lightness(0.8));
        iCredit += aWinnings;
    }

    void table::no_win()
    {
        iOutcome = std::make_unique<outcome>(
            *this,
            "No Win",
            ng::color::Blue.with_lightness(0.8));
    }

    void table::change_state(table_state aNewState)
    {
        if (iState != aNewState)
        {
            iState = aNewState;
            state_changed.trigger(iState);
            update_widgets();
            switch (iState)
            {
            case table_state::DealtSecond:
                {
                    auto w = PAY_TABLE.find(video_poker::to_poker_hand(*iHand));
                    if (w != PAY_TABLE.end() && (w->first != video_poker::poker_hand::Pair || most_frequent_card(*iHand) >= card::value::Jack))
                        win(w->second * iStake);
                    else
                        no_win();
                    auto lastStake = iStake;
                    iStake = 0;
                    bet(lastStake);
                    /* todo win/lose animation */
                    change_state(iCredit + iStake > 0 ? table_state::TakeBet : table_state::GameOver);
                }
                break;
            case table_state::GameOver:
                if (ng::message_box::question(*this, "Out Of Credit - Game Over", "You have run out of credit!\n\nPlay again?") == ng::standard_button::Yes)
                {
                    iCredit = STARTING_CREDIT;
                    iHand.emplace();
                    change_state(table_state::TakeBet);
                }
                else
                    ng::app::instance().quit();
                break;
            default:
                // do nothing
                break;
            }
        }
    }

    void table::update_widgets()
    {
        iLabelCreditsValue.set_text( ng::to_string(u8"£") + boost::lexical_cast<std::string>(iCredit));
        iLabelStakeValue.set_text( ng::to_string(u8"£") + boost::lexical_cast<std::string>(iStake));
        iBetMinus.enable(iState == table_state::TakeBet && iStake > 0);
        iBetPlus.enable(iState == table_state::TakeBet && iCredit > 0 && iStake < MAX_BET);
        iBetMax.enable(iState == table_state::TakeBet && iCredit > 0 && iStake < MAX_BET);
        iDeal.enable(iState != table_state::DealtSecond && iStake > 0);
    }
}