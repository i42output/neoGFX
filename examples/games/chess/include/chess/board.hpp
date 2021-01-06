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

#include <neogfx/gui/widget/widget.hpp>
#include <neogfx/gfx/i_rendering_engine.hpp>
#include <neogfx/gfx/texture_atlas.hpp>

namespace chess
{
    enum class piece_cardinal : uint8_t
    {
        Pawn    = 0x00,
        Knight  = 0x01,
        Bishop  = 0x02,
        Rook    = 0x03,
        Queen   = 0x04,
        King    = 0x05
    };

    enum class piece : uint8_t
    {
        None        = 0x00,

        Pawn        = 0x01,
        Knight      = 0x02,
        Bishop      = 0x04,
        Rook        = 0x08,
        Queen       = 0x10,
        King        = 0x20,

        White       = 0x40,
        Black       = 0x80,

        WhitePawn   = White | Pawn,
        WhiteKnight = White | Knight,
        WhiteBishop = White | Bishop,
        WhiteRook   = White | Rook,
        WhiteQueen  = White | Queen,
        WhiteKing   = White | King,

        BlackPawn   = Black | Pawn,
        BlackKnight = Black | Knight,
        BlackBishop = Black | Bishop,
        BlackRook   = Black | Rook,
        BlackQueen  = Black | Queen,
        BlackKing   = Black | King,

        TYPE_MASK   = Pawn | Knight | Bishop | Rook | Queen | King,
        COLOR_MASK  = White | Black
    };

    inline piece operator&(piece lhs, piece rhs)
    {
        return static_cast<piece>(static_cast<uint8_t>(lhs) & static_cast<uint8_t>(rhs));
    }

    inline piece operator|(piece lhs, piece rhs)
    {
        return static_cast<piece>(static_cast<uint8_t>(lhs) & static_cast<uint8_t>(rhs));
    }

    inline piece piece_type(piece p)
    {
        return p & piece::TYPE_MASK;
    }

    inline piece piece_color(piece p)
    {
        return p & piece::COLOR_MASK;
    }

    class board : public neogfx::widget<>
    {
    public:
        typedef neogfx::point_u32 square_coordinates;
        typedef std::array<std::array<piece, 8>, 8> squares;
    public:
        board(neogfx::i_layout& aLayout) :
            neogfx::widget<>{ aLayout }
        {
            neogfx::image const piecesImage{ ":/chess/resources/pieces.png" };
            neogfx::size const pieceExtents{ piecesImage.extents().cy / 2.0 };
            iPieceTextures.emplace(piece::Pawn, neogfx::texture{ piecesImage, neogfx::rect{ neogfx::point{ pieceExtents.cx * static_cast<double>(piece_cardinal::Pawn), 0.0 }, pieceExtents } });
            iPieceTextures.emplace(piece::Knight, neogfx::texture{ piecesImage, neogfx::rect{ neogfx::point{ pieceExtents.cx * static_cast<double>(piece_cardinal::Knight), 0.0 }, pieceExtents } });
            iPieceTextures.emplace(piece::Bishop, neogfx::texture{ piecesImage, neogfx::rect{ neogfx::point{ pieceExtents.cx * static_cast<double>(piece_cardinal::Bishop), 0.0 }, pieceExtents } });
            iPieceTextures.emplace(piece::Rook, neogfx::texture{ piecesImage, neogfx::rect{ neogfx::point{ pieceExtents.cx * static_cast<double>(piece_cardinal::Rook), 0.0 }, pieceExtents } });
            iPieceTextures.emplace(piece::Queen, neogfx::texture{ piecesImage, neogfx::rect{ neogfx::point{ pieceExtents.cx * static_cast<double>(piece_cardinal::Queen), 0.0 }, pieceExtents } });
            iPieceTextures.emplace(piece::King, neogfx::texture{ piecesImage, neogfx::rect{ neogfx::point{ pieceExtents.cx * static_cast<double>(piece_cardinal::King), 0.0 }, pieceExtents } });

            reset_board();
        }
    protected:
        void paint(neogfx::i_graphics_context& aGc) const override
        {
            for (square_coordinates::coordinate_type y = 0u; y <= 7u; ++y)
                for (square_coordinates::coordinate_type x = 0u; x <= 7u; ++x)
                {
                    auto const squareRect = square_rect({ x, y });
                    aGc.fill_rect(squareRect, (x + y) % 2 == 0 ? neogfx::color::Gray25 : neogfx::color::White);
                    auto const occupier = iSquares[y][x];
                    if (occupier != piece::None)
                    {
                        auto pieceColor = piece_color(occupier) == piece::White ? neogfx::color::Goldenrod : neogfx::color::Silver;
                        aGc.draw_texture(squareRect, iPieceTextures.at(piece_type(occupier)), neogfx::gradient{ pieceColor.lighter(0x80), pieceColor }, neogfx::shader_effect::Colorize);
                    }
                }
        }
    private:
        void reset_board()
        {
            iSquares = squares
            { {
                { piece::WhiteRook, piece::WhiteKnight, piece::WhiteBishop, piece::WhiteQueen, piece::WhiteKing, piece::WhiteBishop, piece::WhiteKnight, piece::WhiteRook },
                { piece::WhitePawn, piece::WhitePawn, piece::WhitePawn, piece::WhitePawn, piece::WhitePawn, piece::WhitePawn, piece::WhitePawn, piece::WhitePawn },
                {}, {}, {}, {},
                { piece::BlackPawn, piece::BlackPawn, piece::BlackPawn, piece::BlackPawn, piece::BlackPawn, piece::BlackPawn, piece::BlackPawn, piece::BlackPawn },
                { piece::BlackRook, piece::BlackKnight, piece::BlackBishop, piece::BlackQueen, piece::BlackKing, piece::BlackBishop, piece::BlackKnight, piece::BlackRook },
            } };
        }
        neogfx::rect square_rect(square_coordinates aCoordinates) const
        {
            auto boardRect = client_rect(false);
            if (boardRect.cx > boardRect.cy)
                boardRect.deflate((boardRect.cx - boardRect.cy) / 2.0, 0.0);
            else
                boardRect.deflate(0.0, (boardRect.cy - boardRect.cx) / 2.0);
            neogfx::size squareDimensions{ boardRect.extents() / 8.0 };
            return neogfx::rect{ boardRect.top_left() + neogfx::point{ squareDimensions * neogfx::size_u32{ aCoordinates.x, 7u - aCoordinates.y }.as<neogfx::scalar>() }, squareDimensions };
        }
    private:
        std::unordered_map<piece, neogfx::texture> iPieceTextures;
        squares iSquares;
    };
}