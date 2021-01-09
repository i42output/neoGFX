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

#include <neogfx/core/geometrical.hpp>

#include <chess/piece.hpp>

namespace chess
{
    typedef neogfx::point_u32 coordinates;
    
    struct move
    {
        coordinates from;
        coordinates to;
        std::optional<piece> promoteTo;
    };

    typedef std::array<std::array<piece, 8>, 8> position;
    struct board
    {
        position position;
        std::array<coordinates, static_cast<std::size_t>(piece_color_cardinal::COUNT)> kings;
        mutable std::optional<move> checkTest;
    };

    inline piece piece_at(board const& aBoard, coordinates aPosition)
    {
        if (!aBoard.checkTest)
            return aBoard.position[aPosition.y][aPosition.x];
        if (aPosition == aBoard.checkTest->from)
            return piece::None;
        if (aPosition == aBoard.checkTest->to)
            return aBoard.position[aBoard.checkTest->from.y][aBoard.checkTest->from.x];
        return aBoard.position[aPosition.y][aPosition.x];
    }

    inline coordinates king_position(board const& aBoard, piece aKing)
    {
        auto const kingPosition = aBoard.kings[static_cast<std::size_t>(as_color_cardinal(aKing))];
        if (aBoard.checkTest && aBoard.checkTest->from == kingPosition)
            return aBoard.checkTest->to;
        return kingPosition;
    }

    inline void move_piece(board& aBoard, chess::move const& aMove)
    {
        auto& source = aBoard.position[aMove.from.y][aMove.from.x];
        auto& destination = aBoard.position[aMove.to.y][aMove.to.x];
        destination = source;
        source = piece::None;
        if (piece_type(destination) == piece::King)
            aBoard.kings[static_cast<std::size_t>(as_color_cardinal(destination))] = aMove.to;
    }

    struct invalid_uci_move : std::runtime_error { invalid_uci_move() : std::runtime_error{ "chess::invalid_uci_move" } {} };

    inline move parse_uci_move(std::string const& aMove)
    {
        if (aMove.size() != 4 && aMove.size() != 5)
            throw invalid_uci_move();
        if (aMove[0] < 'a' || aMove[0] > 'h'|| aMove[1] < '1' || aMove[1] > '8' || aMove[2] < 'a' || aMove[2] > 'h' || aMove[3] < '1' || aMove[3] > '8')
            throw invalid_uci_move();
        move result;
        result.from.x = aMove[0] - 'a';
        result.from.y = aMove[1] - '1';
        result.to.x = aMove[2] - 'a';
        result.to.y = aMove[3] - '1';
        if (aMove.size() == 5)
            result.promoteTo = parse_piece_character(aMove[4]);
        return result;
    }

    static constexpr board setup
    {
        {{
            { piece::WhiteRook, piece::WhiteKnight, piece::WhiteBishop, piece::WhiteQueen, piece::WhiteKing, piece::WhiteBishop, piece::WhiteKnight, piece::WhiteRook },
            { piece::WhitePawn, piece::WhitePawn, piece::WhitePawn, piece::WhitePawn, piece::WhitePawn, piece::WhitePawn, piece::WhitePawn, piece::WhitePawn },
            {}, {}, {}, {},
            { piece::BlackPawn, piece::BlackPawn, piece::BlackPawn, piece::BlackPawn, piece::BlackPawn, piece::BlackPawn, piece::BlackPawn, piece::BlackPawn },
            { piece::BlackRook, piece::BlackKnight, piece::BlackBishop, piece::BlackQueen, piece::BlackKing, piece::BlackBishop, piece::BlackKnight, piece::BlackRook },
        }},
        {{
            { 4u, 0u }, { 4u, 7u }
        }}
    };
}