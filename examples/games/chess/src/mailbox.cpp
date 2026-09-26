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

#include <vector>

#include <chess/mailbox.hpp>

namespace chess
{
    template<>
    mailbox_position const& setup_position<mailbox_rep>()
    {
        static const mailbox_position position
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
            }},
            player::White
        };
        return position;
    }

    template<>
    move_tables<mailbox_rep> generate_move_tables<mailbox_rep>()
    {
        typedef move_tables<mailbox_rep>::move_coordinates move_coordinates;
        move_tables<mailbox_rep> result
        {
            // unit moves
            {{
                {{
                    { { 0, 1 } }, // pawn
                    { { 2, 1 }, { 2, -1 }, { 1, 2 }, { 1, -2 }, { -1, 2 }, { -1, -2 }, { -2, 1 }, { -2, -1 } }, // knight
                    { { -1, 1 }, { 1, 1 }, { 1, -1 }, { -1, -1 } }, // bishop
                    { { 1, 0 }, { 0, 1 }, { -1, 0 }, { 0, -1 } }, // rook
                    { { 1, 0 }, { 0, 1 }, { -1, 0 }, { 0, -1 }, { 1, 1 }, { -1, 1 }, { -1, -1 }, { 1, -1 } }, // queen
                    { { 1, 0 }, { 0, 1 }, { -1, 0 }, { 0, -1 }, { 1, 1 }, { -1, 1 }, { -1, -1 }, { 1, -1 } } // king
                }},
                {{
                    { { 0, -1 } }, // pawn
                    { { 2, 1 }, { 2, -1 }, { 1, 2 }, { 1, -2 }, { -1, 2 }, { -1, -2 }, { -2, 1 }, { -2, -1 } }, // knight
                    { { -1, 1 }, { 1, 1 }, { 1, -1 }, { -1, -1 } }, // bishop
                    { { 1, 0 }, { 0, 1 }, { -1, 0 }, { 0, -1 } }, // rook
                    { { 1, 0 }, { 0, 1 }, { -1, 0 }, { 0, -1 }, { 1, 1 }, { -1, 1 }, { -1, -1 }, { 1, -1 } }, // queen
                    { { 1, 0 }, { 0, 1 }, { -1, 0 }, { 0, -1 }, { 1, 1 }, { -1, 1 }, { -1, -1 }, { 1, -1 } } // king
                }},
            }},
            // capture moves
            {{
                {{
                    { { -1, 1 }, { 1, 1 } }, // pawn
                    { { 2, 1 }, { 2, -1 }, { 1, 2 }, { 1, -2 }, { -1, 2 }, { -1, -2 }, { -2, 1 }, { -2, -1 } }, // knight
                    { { -1, 1 }, { 1, 1 }, { 1, -1 }, { -1, -1 } }, // bishop
                    { { 1, 0 }, { 0, 1 }, { -1, 0 }, { 0, -1 } }, // rook
                    { { 1, 0 }, { 0, 1 }, { -1, 0 }, { 0, -1 }, { 1, 1 }, { -1, 1 }, { -1, -1 }, { 1, -1 } }, // queen
                    { { 1, 0 }, { 0, 1 }, { -1, 0 }, { 0, -1 }, { 1, 1 }, { -1, 1 }, { -1, -1 }, { 1, -1 } } // king
                }},
                {{
                    { { -1, -1 }, { 1, -1 } }, // pawn
                    { { 2, 1 }, { 2, -1 }, { 1, 2 }, { 1, -2 }, { -1, 2 }, { -1, -2 }, { -2, 1 }, { -2, -1 } }, // knight
                    { { -1, 1 }, { 1, 1 }, { 1, -1 }, { -1, -1 } }, // bishop
                    { { 1, 0 }, { 0, 1 }, { -1, 0 }, { 0, -1 } }, // rook
                    { { 1, 0 }, { 0, 1 }, { -1, 0 }, { 0, -1 }, { 1, 1 }, { -1, 1 }, { -1, -1 }, { 1, -1 } }, // queen
                    { { 1, 0 }, { 0, 1 }, { -1, 0 }, { 0, -1 }, { 1, 1 }, { -1, 1 }, { -1, -1 }, { 1, -1 } } // king
                }}
            }},
            // can move multiple of unit move
            {{
                false, // pawn
                false, // knight
                true, // bishop
                true, // rook
                true, // queen
                false // king
            }}
        };
        // all pieces
        for (std::size_t pieceColorIndex = 0u; pieceColorIndex < PIECE_COLORS; ++pieceColorIndex)
            for (std::size_t pieceTypeIndex = 0u; pieceTypeIndex < PIECE_TYPES; ++pieceTypeIndex)
                for (coordinate yFrom = 0u; yFrom <= 7u; ++yFrom)
                    for (coordinate xFrom = 0u; xFrom <= 7u; ++xFrom)
                        for (coordinate yTo = 0u; yTo <= 7u; ++yTo)
                            for (coordinate xTo = 0u; xTo <= 7u; ++xTo)
                            {
                                auto calc_validity = [&](move_tables<mailbox_rep>::unit_moves const& aUnitMoves, bool& aResult) 
                                {
                                    aResult = false;
                                    auto const delta = move_coordinates{ static_cast<std::int32_t>(xTo), static_cast<std::int32_t>(yTo) } - move_coordinates{ static_cast<std::int32_t>(xFrom), static_cast<std::int32_t>(yFrom) };
                                    auto const& unitMoves = aUnitMoves[pieceColorIndex][pieceTypeIndex];
                                    if (std::find(unitMoves.begin(), unitMoves.end(), delta) != unitMoves.end())
                                        aResult = true;
                                    else if (result.canMoveMultiple[pieceTypeIndex] && (std::abs(delta.x) == std::abs(delta.y) || delta.x == 0 || delta.y == 0))
                                    {
                                        auto const& deltaUnity = neogfx::delta_i32{ delta.x != 0 ? delta.x / std::abs(delta.x) : 0, delta.y != 0 ? delta.y / std::abs(delta.y) : 0 };
                                        if (std::find(unitMoves.begin(), unitMoves.end(), deltaUnity) != unitMoves.end())
                                            aResult = true;
                                    }
                                };
                                calc_validity(result.unitMoves, result.validMoves[pieceColorIndex][pieceTypeIndex][yFrom][xFrom][yTo][xTo]);
                                calc_validity(result.unitCaptureMoves, result.validCaptureMoves[pieceColorIndex][pieceTypeIndex][yFrom][xFrom][yTo][xTo]);
                            }
        // pawn (first move)
        for (coordinate x = 0u; x <= 7u; ++x)
        {
            result.validMoves[static_cast<std::size_t>(piece_color_cardinal::White)][static_cast<std::size_t>(piece_cardinal::Pawn)][1u][x][3u][x] = true;
            result.validMoves[static_cast<std::size_t>(piece_color_cardinal::Black)][static_cast<std::size_t>(piece_cardinal::Pawn)][6u][x][4u][x] = true;
        }
        for (coordinate_i32 yFrom = 0; yFrom <= 7; ++yFrom)
            for (coordinate_i32 xFrom = 0; xFrom <= 7; ++xFrom)
                for (coordinate_i32 yTo = 0; yTo <= 7; ++yTo)
                    for (coordinate_i32 xTo = 0; xTo <= 7; ++xTo)
                    {
                        bool aKnight = false;
                        if (!can_move_trivial(coordinates_i32{ xFrom, yFrom }.as<coordinate>(), coordinates_i32{ xTo, yTo }.as<coordinate>(), aKnight))
                            continue;
                        result.trivialMoves[yFrom][xFrom][yTo][xTo] = true;
                        auto const start = coordinates_i32{ xFrom, yFrom };
                        auto const end = coordinates_i32{ xTo, yTo };
                        if (aKnight)
                        {
                            result.movePaths[yFrom][xFrom][yTo][xTo].second[result.movePaths[yFrom][xFrom][yTo][xTo].first++] = start;
                            result.movePaths[yFrom][xFrom][yTo][xTo].second[result.movePaths[yFrom][xFrom][yTo][xTo].first++] = end;
                            continue;
                        }
                        auto const delta = move_tables<mailbox_rep>::move_coordinates{ xTo, yTo } - move_tables<mailbox_rep>::move_coordinates{ xFrom, yFrom };
                        auto const& deltaUnity = neogfx::delta_i32{ delta.x != 0 ? delta.x / std::abs(delta.x) : 0, delta.y != 0 ? delta.y / std::abs(delta.y) : 0 };
                        auto pos = start;
                        for(;;)
                        {
                            result.movePaths[yFrom][xFrom][yTo][xTo].second[result.movePaths[yFrom][xFrom][yTo][xTo].first++] = pos;
                            if (pos == end)
                                break;
                            pos += deltaUnity;
                        }
                    }
        return result;
    }
}