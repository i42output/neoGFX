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

    template <player Player>
    struct eval<matrix, Player>
    {
        eval_result operator()(move_tables<matrix> const& aTables, matrix_board const& aBoard, double aPly, eval_info* aEvalInfo = nullptr)
        {
            auto const start = !aEvalInfo ? std::chrono::steady_clock::time_point{} : std::chrono::steady_clock::now();

            eval_result result = {};

            double constexpr scaleMaterial = 100.0; // todo
            double constexpr scalePromotion = 1.0; // todo
            double constexpr scaleMobility = 1.0; // todo
            double constexpr scaleAttack = 10.0; // todo
            double constexpr scaleDefend = 1.0; // todo
            double constexpr scaleCheck = 9.0; // todo
            double constexpr scaleAttackAdvantage = 2.0; // todo
            double const scaleMate = 1.0 / aPly;
            double constexpr stalemate = 0.0;
            double material = 0.0;
            double mobility = 0.0;
            double attack = 0.0;
            double defend = 0.0;
            double checkedPlayerKing = 0.0;
            bool mobilityPlayer = false;
            bool mobilityOpponent = false;
            bool mobilityPlayerKing = false;
            double checkedOpponentKing = 0.0;
            bool mobilityOpponentKing = false;
            for (coordinate yFrom = 0u; yFrom <= 7u; ++yFrom)
                for (coordinate xFrom = 0u; xFrom <= 7u; ++xFrom)
                {
                    auto const from = piece_at(aBoard, coordinates{ xFrom, yFrom });
                    if (from == piece::None)
                        continue;
                    auto const playerFrom = static_cast<chess::player>(piece_color(from));
                    auto const valueFrom = piece_value<Player>(from);
                    material += valueFrom;
                    for (coordinate yTo = 0u; yTo <= 7u; ++yTo)
                        for (coordinate xTo = 0u; xTo <= 7u; ++xTo)
                        {
                            if (yFrom == yTo && xFrom == xTo)
                                continue;
                            auto const to = piece_at(aBoard, coordinates{ xTo, yTo });
                            auto const playerTo = static_cast<chess::player>(piece_color(to));
                            auto const valueTo = piece_value<Player>(to);
                            if (can_move(aTables, Player, aBoard, move{ { xFrom, yFrom }, { xTo, yTo } }, true, true))
                            {
                                if (playerFrom != playerTo)
                                {
                                    if (from == (piece::King | static_cast<piece>(Player)))
                                        mobilityPlayerKing = true;
                                    else
                                        mobilityPlayer = true;
                                    if (to == (piece::King | static_cast<piece>(opponent_v<Player>)))
                                        checkedOpponentKing = 1.0;
                                    if (from == (piece::Pawn | static_cast<piece>(Player)) && yTo == promotion_rank_v<Player>)
                                        material += (piece_value<Player>(piece::Queen) * scalePromotion);
                                    mobility += 1.0;
                                    if (playerTo == opponent_v<Player>)
                                        attack -= valueTo * scaleAttackAdvantage;
                                }
                                else
                                    defend += valueTo;
                            }
                            else if (can_move(aTables, opponent_v<Player>, aBoard, move{ { xFrom, yFrom }, { xTo, yTo } }, true, true))
                            {
                                if (playerFrom != playerTo)
                                {
                                    if (from == (piece::King | static_cast<piece>(opponent_v<Player>)))
                                        mobilityOpponentKing = true;
                                    else
                                        mobilityOpponent = true;
                                    if (to == (piece::King | static_cast<piece>(Player)))
                                        checkedPlayerKing = 1.0;
                                    if (from == (piece::Pawn | static_cast<piece>(opponent_v<Player>)) && yTo == promotion_rank_v<opponent_v<Player>>)
                                        material -= (piece_value<Player>(piece::Queen) * scalePromotion);
                                    mobility -= 1.0;
                                    if (playerTo == Player)
                                        attack -= valueTo;
                                }
                                else
                                    defend += valueTo;
                            }
                        }
                }
            material *= scaleMaterial;
            mobility *= scaleMobility;
            attack *= scaleAttack;
            defend *= scaleDefend;
            result.eval = material + mobility + attack + defend;
            result.eval -= (checkedPlayerKing * scaleCheck);
            result.eval += (checkedOpponentKing * scaleCheck);
            if (!mobilityPlayerKing)
            {
                if (checkedPlayerKing != 0.0)
                {
                    if (!mobilityPlayer)
                    {
                        result.node = eval_node::Terminal;
                        result.eval = -std::numeric_limits<double>::max() * scaleMate;
                    }
                }
                else if (!mobilityPlayer)
                {
                    result.node = eval_node::Terminal;
                    result.eval = stalemate;
                }
            }
            if (!mobilityOpponentKing)
            {
                if (checkedOpponentKing != 0.0)
                {
                    if (!mobilityOpponent)
                    {
                        result.node = eval_node::Terminal;
                        result.eval = +std::numeric_limits<double>::max() * scaleMate;
                    }
                }
                else if (!mobilityOpponent)
                {
                    result.node = eval_node::Terminal;
                    result.eval = stalemate;
                }
            }

            if (aEvalInfo)
            {
                auto const end_us = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - start);
                *aEvalInfo = eval_info{ material, mobility, attack, defend, mobilityPlayer, mobilityOpponent, mobilityPlayerKing, mobilityOpponentKing, checkedPlayerKing, checkedOpponentKing, result.eval, end_us };
            }

            return result;
        }
        eval_result operator()(move_tables<matrix> const& aTables, matrix_board const& aBoard, double aPly, eval_info& aEvalInfo)
        {
            return eval{}(aTables, aBoard, aPly, &aEvalInfo);
        }
    };


    template struct eval<matrix, player::White>;
    template struct eval<matrix, player::Black>;
}