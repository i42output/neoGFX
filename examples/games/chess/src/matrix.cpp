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

#include <chess/matrix.hpp>

namespace chess
{
    template<>
    matrix_board const& setup_position<matrix>()
    {
        static constexpr matrix_board position
        {
            {{
                { piece::WhiteRook, piece::WhiteKnight, piece::WhiteBishop, piece::WhiteQueen, piece::WhiteKing, piece::WhiteBishop, piece::WhiteKnight, piece::WhiteRook },
                { piece::WhitePawn, piece::WhitePawn, piece::WhitePawn, piece::WhitePawn, piece::WhitePawn, piece::WhitePawn, piece::WhitePawn, piece::WhitePawn },
                {}, {}, {}, {},
                // { piece::BlackPawn, piece::BlackPawn, piece::BlackPawn, piece::BlackPawn, piece::BlackPawn, piece::BlackPawn, piece::BlackPawn, piece::BlackPawn },
                // { piece::BlackRook, piece::BlackKnight, piece::BlackBishop, piece::BlackQueen, piece::BlackKing, piece::BlackBishop, piece::BlackKnight, piece::BlackRook },
                {},
                { piece::None, piece::None, piece::None, piece::None, piece::BlackKing, piece::None, piece::None, piece::None },
            }},
            {{
                { 4u, 0u }, { 4u, 7u }
            }},
            player::White
        };
        return position;
    }

    template<>
    move_tables<matrix> generate_move_tables<matrix>()
    {
        typedef move_tables<matrix>::move_coordinates move_coordinates;
        move_tables<matrix> result
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
        for (std::size_t pieceColorIndex = 0u; pieceColorIndex < static_cast<std::size_t>(piece_color_cardinal::COUNT); ++pieceColorIndex)
            for (std::size_t pieceIndex = 0u; pieceIndex < static_cast<std::size_t>(piece_cardinal::COUNT); ++pieceIndex)
                for (coordinate yFrom = 0u; yFrom <= 7u; ++yFrom)
                    for (coordinate xFrom = 0u; xFrom <= 7u; ++xFrom)
                        for (coordinate yTo = 0u; yTo <= 7u; ++yTo)
                            for (coordinate xTo = 0u; xTo <= 7u; ++xTo)
                            {
                                auto calc_validity = [&](move_tables<matrix>::unit_moves const& aUnitMoves, bool& aResult) 
                                {
                                    aResult = false;
                                    auto const delta = move_coordinates{ static_cast<int32_t>(xTo), static_cast<int32_t>(yTo) } - move_coordinates{ static_cast<int32_t>(xFrom), static_cast<int32_t>(yFrom) };
                                    auto const& unitMoves = aUnitMoves[pieceColorIndex][pieceIndex];
                                    if (std::find(unitMoves.begin(), unitMoves.end(), delta) != unitMoves.end())
                                        aResult = true;
                                    else if (result.canMoveMultiple[pieceIndex] && (std::abs(delta.dx) == std::abs(delta.dy) || delta.dx == 0 || delta.dy == 0))
                                    {
                                        auto const& deltaUnity = neogfx::delta_i32{ delta.dx != 0 ? delta.dx / std::abs(delta.dx) : 0, delta.dy != 0 ? delta.dy / std::abs(delta.dy) : 0 };
                                        if (std::find(unitMoves.begin(), unitMoves.end(), deltaUnity) != unitMoves.end())
                                            aResult = true;
                                    }
                                };
                                calc_validity(result.unitMoves, result.validMoves[pieceColorIndex][pieceIndex][yFrom][xFrom][yTo][xTo]);
                                calc_validity(result.unitCaptureMoves, result.validCaptureMoves[pieceColorIndex][pieceIndex][yFrom][xFrom][yTo][xTo]);
                            }
        // pawn (first move)
        for (coordinate x = 0u; x <= 7u; ++x)
        {
            result.validMoves[static_cast<std::size_t>(piece_color_cardinal::White)][static_cast<std::size_t>(piece_cardinal::Pawn)][1u][x][3u][x] = true;
            result.validMoves[static_cast<std::size_t>(piece_color_cardinal::Black)][static_cast<std::size_t>(piece_cardinal::Pawn)][6u][x][4u][x] = true;
        }
        // todo: corner cases
        return result;
    }
}