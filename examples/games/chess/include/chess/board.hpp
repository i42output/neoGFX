/*
neogfx C++ App/Game Engine - Examples - Games - Chess
Copyright(C) 2020 Leigh Johnston

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
#include <neogfx/gfx/i_rendering_engine.hpp>
#include <neogfx/gfx/texture_atlas.hpp>

#include <chess/i_board.hpp>
#include <chess/i_move_validator.hpp>

namespace chess::gui
{
    class board : public i_board, public neogfx::widget<>
    {
    public:
        board(neogfx::i_layout& aLayout, i_move_validator const& aMoveValidator);
    protected:
        void paint(neogfx::i_graphics_context& aGc) const override;
    protected:
        void mouse_button_pressed(neogfx::mouse_button aButton, const neogfx::point& aPosition, neogfx::key_modifiers_e aKeyModifiers) override;
        void mouse_button_double_clicked(neogfx::mouse_button aButton, const neogfx::point& aPosition, neogfx::key_modifiers_e aKeyModifiers) override;
        void mouse_button_released(neogfx::mouse_button aButton, const neogfx::point& aPosition) override;
        void mouse_moved(const neogfx::point& aPosition, neogfx::key_modifiers_e aKeyModifiers) override;
        void mouse_entered(const neogfx::point& aPosition) override;
        void mouse_left() override;
    protected:
        void reset() override;
        void setup(player aTurn, chess::position const& aPosition) override;
        bool play(chess::move const& aMove) override;
    private:
        void animate();
        neogfx::rect square_rect(coordinates aCoordinates) const;
        std::optional<coordinates> at(neogfx::point const& aPosition) const;
    private:
        neolib::callback_timer iAnimator;
        std::optional<coordinates> iCursor;
        std::optional<coordinates> iSelection;
        std::optional<neogfx::point> iSelectionPosition;
        std::unordered_map<piece, neogfx::texture> iPieceTextures;
        chess::position iPosition;
        player iTurn;
        i_move_validator const& iMoveValidator;
    };
}