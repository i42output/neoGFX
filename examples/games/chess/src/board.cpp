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

#include <chess/board.hpp>

namespace chess::gui
{
    board::board(neogfx::i_layout& aLayout, i_move_validator const& aMoveValidator) :
        neogfx::widget<>{ aLayout },
        iTurn{ player::Invalid },
        iMoveValidator{ aMoveValidator }
    {
        neogfx::image const piecesImage{ ":/chess/resources/pieces.png" };
        neogfx::size const pieceExtents{ piecesImage.extents().cy / 2.0 };
        iPieceTextures.emplace(piece::Pawn, neogfx::texture{ piecesImage, neogfx::rect{ neogfx::point{ pieceExtents.cx * static_cast<double>(piece_cardinal::Pawn), 0.0 }, pieceExtents } });
        iPieceTextures.emplace(piece::Knight, neogfx::texture{ piecesImage, neogfx::rect{ neogfx::point{ pieceExtents.cx * static_cast<double>(piece_cardinal::Knight), 0.0 }, pieceExtents } });
        iPieceTextures.emplace(piece::Bishop, neogfx::texture{ piecesImage, neogfx::rect{ neogfx::point{ pieceExtents.cx * static_cast<double>(piece_cardinal::Bishop), 0.0 }, pieceExtents } });
        iPieceTextures.emplace(piece::Rook, neogfx::texture{ piecesImage, neogfx::rect{ neogfx::point{ pieceExtents.cx * static_cast<double>(piece_cardinal::Rook), 0.0 }, pieceExtents } });
        iPieceTextures.emplace(piece::Queen, neogfx::texture{ piecesImage, neogfx::rect{ neogfx::point{ pieceExtents.cx * static_cast<double>(piece_cardinal::Queen), 0.0 }, pieceExtents } });
        iPieceTextures.emplace(piece::King, neogfx::texture{ piecesImage, neogfx::rect{ neogfx::point{ pieceExtents.cx * static_cast<double>(piece_cardinal::King), 0.0 }, pieceExtents } });

        reset();
    }

    void board::paint(neogfx::i_graphics_context& aGc) const
    {
        for (coordinates::coordinate_type y = 0u; y <= 7u; ++y)
            for (coordinates::coordinate_type x = 0u; x <= 7u; ++x)
            {
                auto const squareRect = square_rect({ x, y });
                aGc.fill_rect(squareRect, (x + y) % 2 == 0 ? neogfx::color::Gray25 : neogfx::color::White);
                auto const occupier = iPosition[y][x];
                if (occupier != piece::None)
                {
                    auto pieceColor = piece_color(occupier) == piece::White ? neogfx::color::Goldenrod : neogfx::color::Silver;
                    aGc.draw_texture(squareRect, iPieceTextures.at(piece_type(occupier)), neogfx::gradient{ pieceColor.lighter(0x80), pieceColor }, neogfx::shader_effect::Colorize);
                }
            }
    }

    void board::reset()
    {
        setup(player::White, chess::setup);
    }

    void board::setup(player aTurn, chess::position const& aPosition)
    {
        iPosition = aPosition;
        iTurn = aTurn;
    }

    bool board::play(chess::move const& aMove)
    {
        if (!iMoveValidator.can_move(iTurn, iPosition, aMove))
            return false;
        // todo
        return true;
    }

    neogfx::rect board::square_rect(coordinates aCoordinates) const
    {
        auto boardRect = client_rect(false);
        if (boardRect.cx > boardRect.cy)
            boardRect.deflate((boardRect.cx - boardRect.cy) / 2.0, 0.0);
        else
            boardRect.deflate(0.0, (boardRect.cy - boardRect.cx) / 2.0);
        neogfx::size squareDimensions{ boardRect.extents() / 8.0 };
        return neogfx::rect{ boardRect.top_left() + neogfx::point{ squareDimensions * neogfx::size_u32{ aCoordinates.x, 7u - aCoordinates.y }.as<neogfx::scalar>() }, squareDimensions };
    }
}