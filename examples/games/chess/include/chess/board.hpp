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

namespace ng = neogfx;
using namespace ng::unit_literals;

namespace chess::gui
{
    struct animation
    {
        chess::move move;
        piece movingPiece;
        piece capturedPiece;
        mutable std::optional<std::chrono::steady_clock::time_point> startTime;
        mutable bool hold = false;
    };

    enum class square_identification : uint32_t
    {
        None,
        Outer,
        Inner,
        InnerExtra,

        COUNT
    };

    class board : public i_board, public ng::widget<>
    {
    public:
        board(ng::i_layout& aLayout, i_move_validator const& aMoveValidator);
    protected:
        void paint(ng::i_graphics_context& aGc) const override;
    protected:
        bool key_pressed(ng::scan_code_e aScanCode, ng::key_code_e aKeyCode, ng::key_modifiers_e aKeyModifiers) override;
    protected:
        ng::focus_policy focus_policy() const override;
    protected:
        void mouse_button_pressed(ng::mouse_button aButton, const ng::point& aPosition, ng::key_modifiers_e aKeyModifiers) override;
        void mouse_button_double_clicked(ng::mouse_button aButton, const ng::point& aPosition, ng::key_modifiers_e aKeyModifiers) override;
        void mouse_button_released(ng::mouse_button aButton, const ng::point& aPosition) override;
        void mouse_moved(const ng::point& aPosition, ng::key_modifiers_e aKeyModifiers) override;
        void mouse_entered(const ng::point& aPosition) override;
        void mouse_left() override;
    protected:
        void reset() override;
        void setup(player aTurn, chess::board const& aBoard) override;
        bool play(chess::move const& aMove) override;
        void edit(chess::move const& aMove) override;
    private:
        std::optional<std::pair<animation const*, ng::point>> animating_to(coordinates const& aMovePos, std::chrono::steady_clock::time_point const& aTime = std::chrono::steady_clock::now()) const;
        bool animating_from(coordinates const& aMovePos) const;
        void animate();
        ng::rect board_rect() const;
        ng::rect square_rect(coordinates aCoordinates) const;
        ng::scalar scale() const;
        ng::rect piece_rect(coordinates aCoordinates) const;
        std::optional<coordinates> at(ng::point const& aPosition) const;
    private:
        i_move_validator const& iMoveValidator;
        chess::board iBoard;
        player iTurn;
        std::unordered_map<piece, ng::texture> iPieceTextures;
        neolib::callback_timer iAnimator;
        std::optional<coordinates> iCursor;
        std::optional<coordinates> iSelection;
        std::optional<ng::point> iSelectionPosition;
        square_identification iSquareIdentification;
        bool iShowValidMoves;
        std::optional<std::chrono::steady_clock::time_point> iLastSelectionEventTime;
        bool iEditBoard;
        mutable std::deque<animation> iAnimations;
        std::optional<std::pair<bool, std::chrono::steady_clock::time_point>> iFlashCheck;
    };
}