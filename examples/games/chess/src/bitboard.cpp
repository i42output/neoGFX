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

#include <chess/bitboard.hpp>
#include <chess/node.hpp>
#include <chess/mailbox.hpp>

namespace chess
{
    template <>
    bitboard_position const& setup_position<bitboard_rep>()
    {
        static const bitboard_position position
        {
            // same information structured differently based on how the bitboard algorithms use the information...
            {
                {  // pieces
                    ( 0b1111111111111111000000000000000000000000000000001111111111111111ull )  // occupancy
                },
                {{ // byPieceColor
                    { 0b0000000000000000000000000000000000000000000000001111111111111111ull }, // white
                    { 0b1111111111111111000000000000000000000000000000000000000000000000ull }  // black
                }},
                {{ // byPieceType
                    { 0b0000000011111111000000000000000000000000000000001111111100000000ull }, // pawn
                    { 0b0100001000000000000000000000000000000000000000000000000001000010ull }, // knight
                    { 0b0010010000000000000000000000000000000000000000000000000000100100ull }, // bishop
                    { 0b1000000100000000000000000000000000000000000000000000000010000001ull }, // rook
                    { 0b0000100000000000000000000000000000000000000000000000000000001000ull }, // queen
                    { 0b0001000000000000000000000000000000000000000000000000000000010000ull }  // king
                }}, 
                {{ // bySquare
                    piece::WhiteRook, piece::WhiteKnight, piece::WhiteBishop, piece::WhiteQueen, piece::WhiteKing, piece::WhiteBishop, piece::WhiteKnight, piece::WhiteRook,
                    piece::WhitePawn, piece::WhitePawn, piece::WhitePawn, piece::WhitePawn, piece::WhitePawn, piece::WhitePawn, piece::WhitePawn, piece::WhitePawn,
                    {}, {}, {}, {}, {}, {}, {}, {},
                    {}, {}, {}, {}, {}, {}, {}, {},
                    {}, {}, {}, {}, {}, {}, {}, {},
                    {}, {}, {}, {}, {}, {}, {}, {},
                    piece::BlackPawn, piece::BlackPawn, piece::BlackPawn, piece::BlackPawn, piece::BlackPawn, piece::BlackPawn, piece::BlackPawn, piece::BlackPawn,
                    piece::BlackRook, piece::BlackKnight, piece::BlackBishop, piece::BlackQueen, piece::BlackKing, piece::BlackBishop, piece::BlackKnight, piece::BlackRook,
                }}
            },
            player::White
        };
        return position;
    }

    template<>
    move_tables<bitboard_rep> generate_move_tables<bitboard_rep>()
    {
        auto const mailboxMoveTables = generate_move_tables<mailbox_rep>();

        move_tables<bitboard_rep> result = {};

        for (std::size_t pieceColorIndex = 0u; pieceColorIndex < PIECE_COLORS; ++pieceColorIndex)
            for (std::size_t pieceTypeIndex = 0u; pieceTypeIndex < PIECE_TYPES; ++pieceTypeIndex)
                for (coordinate yFrom = 0u; yFrom <= 7u; ++yFrom)
                    for (coordinate xFrom = 0u; xFrom <= 7u; ++xFrom)
                        for (coordinate yTo = 0u; yTo <= 7u; ++yTo)
                            for (coordinate xTo = 0u; xTo <= 7u; ++xTo)
                            {
                                if (mailboxMoveTables.validMoves[pieceColorIndex][pieceTypeIndex][yFrom][xFrom][yTo][xTo])
                                    result.validMoves[pieceColorIndex][pieceTypeIndex][bit_position_from_coordinates({ xFrom, yFrom })] |= 
                                        bit_from_coordinates({ xTo, yTo });
                                if (mailboxMoveTables.validCaptureMoves[pieceColorIndex][pieceTypeIndex][yFrom][xFrom][yTo][xTo])
                                    result.validCaptureMoves[pieceColorIndex][pieceTypeIndex][bit_position_from_coordinates({ xFrom, yFrom })] |= 
                                        bit_from_coordinates({ xTo, yTo });
                                for (std::size_t pathSquareIndex = 0; pathSquareIndex < mailboxMoveTables.movePaths[yFrom][xFrom][yTo][xTo].first; ++pathSquareIndex)
                                    result.validPaths[bit_position_from_coordinates({ xFrom, yFrom })][bit_position_from_coordinates({ xTo, yTo })] |=
                                        bit_from_coordinates(mailboxMoveTables.movePaths[yFrom][xFrom][yTo][xTo].second[pathSquareIndex]);
                            }

        return result;
    }

    template <player Player>
    struct eval<bitboard_rep, Player>
    {
        eval_result operator()(move_tables<bitboard_rep> const& aTables, bitboard_position& aPosition, double aPly, eval_info* aEvalInfo = nullptr)
        {
            auto const start = !aEvalInfo ? std::chrono::steady_clock::time_point{} : std::chrono::steady_clock::now();

            eval_result result = {};

            double constexpr scaleMaterial = 100.0; // todo
            double constexpr scalePromotion = 0.01; // todo
            double constexpr scaleMobility = 0.01; // todo
            double constexpr scaleAttack = 0.04; // todo
            double constexpr scaleDefend = 0.02; // todo
            double constexpr scaleCheck = 1.0; // todo
            double constexpr scaleAttackAdvantage = 0.04; // todo
            double const scaleMate = 1.0 / std::pow(10.0, aPly);
            double constexpr stalemate = 0.0;
            double material = 0.0;
            double mobility = 0.0;
            double attack = 0.0;
            double defend = 0.0;
            bool mobilityPlayer = false;
            bool mobilityPlayerKing = false;
            double checkedPlayerKing = in_check<Player>(aTables, aPosition) ? 1.0 : 0.0;
            bool mobilityOpponent = false;
            bool mobilityOpponentKing = false;
            double checkedOpponentKing = in_check<opponent_v<Player>>(aTables, aPosition) ? 1.0 : 0.0;
            if (chess::draw(aPosition))
            {
                if (aEvalInfo)
                {
                    auto const end_us = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - start);
                    *aEvalInfo = eval_info{ material, mobility, attack, defend, mobilityPlayer, mobilityOpponent, mobilityPlayerKing, mobilityOpponentKing, checkedPlayerKing, checkedOpponentKing, result.eval, end_us };
                }
                return eval_result{ eval_node::Terminal, stalemate };
            }

            for (std::size_t playerPieceTypeIndex = 0; playerPieceTypeIndex < PIECE_TYPES; ++playerPieceTypeIndex)
            {
                auto const pieces = aPosition.rep.byPieceType[playerPieceTypeIndex];
                auto playerPieces = pieces & aPosition.rep.byPieceColor[as_cardinal<>(Player)];
                auto opponentPieces = pieces & aPosition.rep.byPieceColor[as_cardinal<>(opponent_v<Player>)];

                material += std::popcount(playerPieces) * (piece_value<Player>(cardinal_to_piece(playerPieceTypeIndex)));
                material -= std::popcount(opponentPieces) * (piece_value<opponent_v<Player>>(cardinal_to_piece(playerPieceTypeIndex)));

                // todo: pawn promotion value
            }

            thread_local game_tree_node playerNode;
            thread_local game_tree_node opponentNode;
            if (playerNode.children == std::nullopt)
                playerNode.children.emplace();
            if (opponentNode.children == std::nullopt)
                opponentNode.children.emplace();

            valid_moves<Player>(aTables, aPosition, playerNode);
            valid_moves<opponent_v<Player>>(aTables, aPosition, opponentNode);

            mobility = static_cast<double>(static_cast<int64_t>(as_valid_moves(playerNode).size()) - static_cast<int64_t>(as_valid_moves(opponentNode).size()));
            mobilityPlayer = !as_valid_moves(playerNode).empty();
            mobilityOpponent = !as_valid_moves(opponentNode).empty();
            mobilityPlayerKing = *playerNode.kingMobility;
            mobilityOpponentKing = *opponentNode.kingMobility;

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
        eval_result operator()(move_tables<bitboard_rep> const& aTables, bitboard_position& aPosition, double aPly, eval_info& aEvalInfo)
        {
            return eval{}(aTables, aPosition, aPly, &aEvalInfo);
        }
    };

    template struct eval<bitboard_rep, player::White>;
    template struct eval<bitboard_rep, player::Black>;
}