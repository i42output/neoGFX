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

#include <array>
#include <bit>
#include <chess/primitives.hpp>

namespace chess
{
    class bitboard_iterator
    {
    public:
        bitboard_iterator() :
            iValue{}, iBit{}
        {
        }
        bitboard_iterator(bitboard aBitboard) :
            iValue{ aBitboard }, iBit{ next_bit(aBitboard) }
        {
        }
    public:
        bool operator==(bitboard_iterator const& aRhs) const
        {
            return iValue == aRhs.iValue;
        }
        bool operator!=(bitboard_iterator const& aRhs) const
        {
            return iValue != aRhs.iValue;
        }
        bit_position const& operator*() const
        {
            return iBit;
        }
        bit_position const& operator->() const
        {
            return iBit;
        }
        bitboard_iterator& operator++()
        {
            iValue ^= (1ull << iBit);
            iBit = next_bit(iValue);
            return *this;
        }
    private:
        static bit_position next_bit(bitboard aValue)
        {
            return static_cast<bit_position>(std::countr_zero(aValue));
        }
    private:
        bitboard iValue;
        bit_position iBit;
    };

    class bitboard_as_range
    {
    public:
        bitboard_as_range(bitboard aBitboard) :
            iBitboard{ aBitboard }
        {
        }
    public:
        bitboard_iterator begin() const
        {
            return iBitboard;
        }
        bitboard_iterator end() const
        {
            return {};
        }
    private:
        bitboard iBitboard;
    };

    template<>
    struct move_tables<bitboard_rep>
    {
        typedef std::array<std::array<std::array<bitboard, SQUARES>, PIECE_TYPES>, PIECE_COLORS> valid_moves;
        typedef std::array<std::array<bitboard, SQUARES>, SQUARES> valid_paths;

        valid_moves validMoves;
        valid_moves validCaptureMoves;
        valid_paths validPaths;
    };

    template <player Player>
    inline bool in_check(move_tables<bitboard_rep> const& aTables, bitboard_position const& aPosition)
    {
        auto const playerColorIndex = as_cardinal<>(Player);
        auto const opponentColorIndex = as_cardinal<>(opponent(Player));
        auto const playerPieces = aPosition.rep.byPieceColor[playerColorIndex];
        auto const playerKing = aPosition.rep.byPieceType[as_cardinal<>(piece::King)] & playerPieces;

        for (std::size_t opponentPieceTypeIndex = 0; opponentPieceTypeIndex < PIECE_TYPES; ++opponentPieceTypeIndex)
        {
            auto const pieces = aPosition.rep.byPieceType[opponentPieceTypeIndex];
            auto opponentPieces = pieces & aPosition.rep.byPieceColor[opponentColorIndex];
            for (auto const& opponentPiece : bitboard_as_range{ opponentPieces })
            {
                auto const opponentPieceCaptureMoves = aTables.validCaptureMoves[opponentColorIndex][opponentPieceTypeIndex][opponentPiece];
                auto const captures = aPosition.rep.pieces & opponentPieceCaptureMoves & aTables.validPaths[opponentPiece][bit_position_from_bitboard(playerKing)];
                if (captures == playerKing)
                    return true;
            }
        }

        return false;
    }

    template <player Player, typename ResultContainer>
    inline void sort_nodes(move_tables<bitboard_rep> const& aTables, bitboard_position const& aPosition, ResultContainer& aResult)
    {
        std::sort(as_valid_moves(aResult).begin(), as_valid_moves(aResult).end(), [&](auto const& lhs, auto const& rhs)
            {
                bitboard_position lhsBoard = aPosition;
                bitboard_position rhsBoard = aPosition;
                make(lhsBoard, as_move(lhs));
                make(rhsBoard, as_move(rhs));
                return eval<bitboard_rep, Player>{}(aTables, lhsBoard, 2.0).eval < eval<bitboard_rep, Player>{}(aTables, rhsBoard, 2.0).eval;
            });
    }

    template <player Player>
    inline void valid_moves(move_tables<bitboard_rep> const& aTables, bitboard_position& aPosition, game_tree_node& aResult)
    {
        as_valid_moves(aResult).clear();

        auto const playerColorIndex = as_cardinal<>(Player);
        auto const opponentColorIndex = as_cardinal<>(opponent(Player));

        aResult.kingMobility = false;

        for (std::size_t playerPieceTypeIndex = 0; playerPieceTypeIndex < PIECE_TYPES; ++playerPieceTypeIndex)
        {
            auto const pieces = aPosition.rep.byPieceType[playerPieceTypeIndex];
            auto playerPieces = pieces & aPosition.rep.byPieceColor[as_cardinal<>(Player)];
            for (auto const& playerPiece : bitboard_as_range{ playerPieces })
            {
                auto const playerPieceBit = bit_from_bit_position(playerPiece);
                auto const playerPieceCoordinates = coordinates_from_bit_position(playerPiece);
                auto const playerPieceMoves = aTables.validMoves[playerColorIndex][playerPieceTypeIndex][playerPiece];
                for (auto const& playerMoveTo : bitboard_as_range{ playerPieceMoves })
                {
                    auto const playerMovePath = aTables.validPaths[playerPiece][playerMoveTo];
                    if ((aPosition.rep.pieces & playerMovePath) == playerPieceBit)
                    {
                        move const candidateMove{ playerPieceCoordinates, coordinates_from_bit_position(playerMoveTo) };
                        make(aPosition, candidateMove);
                        bool const inCheck = in_check<Player>(aTables, aPosition);
                        unmake(aPosition);
                        if (!inCheck)
                        {
                            as_valid_moves(aResult).emplace_back(candidateMove);
                            if (playerPieceTypeIndex == as_cardinal<>(piece::King))
                                aResult.kingMobility = true;
                        }
                    }
                    // todo: castling
                }
                auto const playerPieceCaptureMoves = aTables.validCaptureMoves[playerColorIndex][playerPieceTypeIndex][playerPiece];
                for (auto const& playerMoveTo : bitboard_as_range{ playerPieceCaptureMoves })
                {
                    auto const capture = aPosition.rep.byPieceColor[opponentColorIndex] & playerMoveTo;
                    if (capture != playerMoveTo)
                    {
                        // todo: en passant
                        continue;
                    }
                    if (aPosition.rep.byPieceType[as_cardinal<>(piece::King)] & playerMoveTo)
                        continue;
                    auto const playerMovePath = aTables.validPaths[playerPiece][playerMoveTo];
                    if ((aPosition.rep.pieces & playerMovePath) == (playerPieceBit | bit_from_bit_position(playerMoveTo)))
                    {
                        move const candidateMove{ playerPieceCoordinates, coordinates_from_bit_position(playerMoveTo) };
                        make(aPosition, candidateMove);
                        bool const inCheck = in_check<Player>(aTables, aPosition);
                        unmake(aPosition);
                        if (!inCheck)
                        {
                            as_valid_moves(aResult).emplace_back(candidateMove);
                            if (playerPieceTypeIndex == as_cardinal<>(piece::King))
                                aResult.kingMobility = true;
                        }
                    }
                }
            }
        }
    }
}