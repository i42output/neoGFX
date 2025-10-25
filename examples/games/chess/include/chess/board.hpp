﻿/*
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

#include <chess/chess.hpp>
#include <chess/i_move_validator.hpp>
#include <chess/i_board.hpp>

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

    enum class square_identification : std::uint32_t
    {
        None,
        Outer,
        Inner,
        InnerExtra,
        Debug,

        COUNT
    };

    class board : public i_board, public ng::widget<>
    {
    public:
        define_declared_event(Changed, changed)
    public:
        struct no_player : std::logic_error { no_player() : std::logic_error{ "chess::gui::board::no_player" } {} };
    public:
        board(ng::i_layout& aLayout, i_move_validator const& aMoveValidator);
        ~board();
    protected:
        void paint(ng::i_graphics_context& aGc) const override;
    protected:
        bool key_pressed(ng::scan_code_e aScanCode, ng::key_code_e aKeyCode, ng::key_modifiers_e aKeyModifiers) override;
    protected:
        void resized() override;
    protected:
        ng::focus_policy focus_policy() const override;
    protected:
        void mouse_button_clicked(ng::mouse_button aButton, const ng::point& aPosition, ng::key_modifiers_e aKeyModifiers) override;
        void mouse_button_double_clicked(ng::mouse_button aButton, const ng::point& aPosition, ng::key_modifiers_e aKeyModifiers) override;
        void mouse_button_released(ng::mouse_button aButton, const ng::point& aPosition) override;
        void mouse_moved(const ng::point& aPosition, ng::key_modifiers_e aKeyModifiers) override;
        void mouse_entered(const ng::point& aPosition) override;
        void mouse_left() override;
    public:
        void new_game(i_player_factory& aPlayerFactory, player_type aWhitePlayer, player_type aBlackPlayer) override;
        void setup(chess::mailbox_position const& aPosition) override;
        bool play(chess::move const& aMove) override;
        void edit(chess::move const& aMove) override;
        bool can_undo() const override;
        void undo() override;
        bool can_redo() const override;
        void redo() override;  
        bool can_play() const override;
        void play() override;
        bool can_stop() const override;
        void stop() override;
    public:
        i_player const& current_player() const override;
        i_player& current_player() override;
        i_player const& next_player() const override;
        i_player& next_player() override;
        i_player const& white_player() const override;
        i_player& white_player() override;
        i_player const& black_player() const override;
        i_player& black_player() override;
    private:
        void moved(chess::move const& aMove);
        void display_eval() const;
        void display_query() const;
    private:
        void animate_move(chess::move const& aMove);
        std::optional<std::pair<animation const*, ng::point>> animating_to(coordinates const& aMovePos, std::chrono::steady_clock::time_point const& aTime = std::chrono::steady_clock::now()) const;
        bool animating_from(coordinates const& aMovePos) const;
        void animate();
        ng::rect board_rect() const;
        ng::rect square_rect(coordinates aCoordinates) const;
        ng::scalar scale() const;
        ng::rect piece_rect(coordinates aCoordinates) const;
        std::optional<coordinates> at(ng::point const& aPosition) const;
    private:
        ng::sink iSink;
        i_move_validator const& iMoveValidator;
        chess::mailbox_position iPosition;
        std::unique_ptr<i_player> iWhitePlayer;
        std::unique_ptr<i_player> iBlackPlayer;
        std::vector<chess::move> iUndoneMoves;
        bool iInRedo;
        bool iFlipped;
        ng::color iColorWhiteSquare;
        ng::color iColorBlackSquare;
        ng::color iColorWhitePiece;
        ng::color iColorBlackPiece;
        std::unordered_map<piece, ng::texture> iPieceTextures;
        bool iColorizePieces;
        ng::widget_timer iAnimator;
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