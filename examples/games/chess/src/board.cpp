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

#include <neogfx/app/i_app.hpp>

#include <chess/board.hpp>

namespace chess::gui
{
    board::board(neogfx::i_layout& aLayout, i_move_validator const& aMoveValidator) :
        neogfx::widget<>{ aLayout },
        iAnimator{ neogfx::service<neogfx::i_async_task>(), [this](neolib::callback_timer&) { animate(); }, std::chrono::milliseconds{ 20 } },
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
        for (int32_t pass = 1; pass <= 3; ++pass)
            for (coordinates::coordinate_type y = 0u; y <= 7u; ++y)
                for (coordinates::coordinate_type x = 0u; x <= 7u; ++x)
                {
                    auto const squareRect = square_rect({ x, y });
                    switch (pass)
                    {
                    case 1: // board
                        aGc.fill_rect(squareRect, (x + y) % 2 == 0 ? neogfx::color::Gray25 : neogfx::color::Burlywood);
                        if (iCursor && *iCursor == coordinates{ x, y } && iCursor != iSelection)
                            aGc.fill_rect(squareRect, palette_color(neogfx::color_role::Selection));
                        else if (iSelection && *iSelection == coordinates{ x, y })
                            aGc.fill_rect(squareRect, neogfx::color::White);
                        break;
                    case 2: // non-selected pieces
                    case 3: // selected piece
                        {
                            bool selectedOccupier = (iSelection  && *iSelection == coordinates{ x, y });
                            if ((pass == 2 && selectedOccupier) || (pass == 3 && !selectedOccupier))
                                continue;
                            auto const occupier = iPosition[y][x];
                            if (occupier != piece::None)
                            {
                                auto pieceColor = piece_color(occupier) == piece::White ? neogfx::color::Goldenrod : neogfx::color::Silver;
                                neogfx::point mousePosition = root().mouse_position() - origin();
                                auto adjust = (!selectedOccupier || !iSelectionPosition || (mousePosition - *iSelectionPosition).magnitude() < 8.0 ?
                                    neogfx::point{} : mousePosition - *iSelectionPosition);
                                aGc.draw_texture(squareRect + adjust, iPieceTextures.at(piece_type(occupier)), neogfx::gradient{ pieceColor.lighter(0x80), pieceColor }, neogfx::shader_effect::Colorize);
                            }
                        }
                        break;
                    }
                }
    }

    void board::mouse_button_pressed(neogfx::mouse_button aButton, const neogfx::point& aPosition, neogfx::key_modifiers_e aKeyModifiers)
    {
        widget<>::mouse_button_pressed(aButton, aPosition, aKeyModifiers);
        if (capturing())
        {
            if (!iSelection)
            {
                auto pos = at(aPosition);
                if (pos && piece_color(iPosition[pos->y][pos->x]) == static_cast<piece>(iTurn))
                {
                    iSelectionPosition = aPosition;
                    iSelection = pos;
                }
            }
            else
            {
                auto pos = at(aPosition);
                if (pos == iSelection)
                {
                    iSelectionPosition = std::nullopt;
                    iSelection = std::nullopt;
                }
                else if (piece_color(iPosition[pos->y][pos->x]) == static_cast<piece>(iTurn))
                {
                    iSelectionPosition = aPosition;
                    iSelection = pos;
                }
                else if (iMoveValidator.can_move(iTurn, iPosition, chess::move{ *iSelection, *pos }))
                {
                    play(chess::move{ *iSelection, *pos });
                    iSelectionPosition = std::nullopt;
                    iSelection = std::nullopt;
                }
            }
        }
        update();
    }

    void board::mouse_button_double_clicked(neogfx::mouse_button aButton, const neogfx::point& aPosition, neogfx::key_modifiers_e aKeyModifiers)
    {
        widget<>::mouse_button_double_clicked(aButton, aPosition, aKeyModifiers);
    }

    void board::mouse_button_released(neogfx::mouse_button aButton, const neogfx::point& aPosition)
    {
        bool wasCapturing = capturing();
        widget<>::mouse_button_released(aButton, aPosition);
        if (wasCapturing)
        {
            if (iSelection)
            {
                iSelectionPosition = std::nullopt;
                auto pos = at(aPosition);
                if (pos && pos != *iSelection)
                {
                    play(chess::move{ *iSelection, *pos });
                    iSelection = std::nullopt;
                }
            }
        }
        update();
    }

    void board::mouse_moved(const neogfx::point& aPosition, neogfx::key_modifiers_e aKeyModifiers)
    {
        iCursor = at(aPosition);
        update();
    }

    void board::mouse_entered(const neogfx::point& aPosition)
    {
        iCursor = at(aPosition);
        update();
    }

    void board::mouse_left()
    {
        iCursor = std::nullopt;
        update();
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
        // todo: update engine
        iPosition[aMove.to.y][aMove.to.x] = iPosition[aMove.from.y][aMove.from.x];
        iPosition[aMove.from.y][aMove.from.x] = piece::None;
        iTurn = (iTurn == player::White ? player::Black : player::White);
        update();
        return true;
    }

    void board::animate()
    {
        iAnimator.again();
        // todo
        update();
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

    std::optional<coordinates> board::at(neogfx::point const& aPosition) const
    {
        for (coordinates::coordinate_type y = 0u; y <= 7u; ++y)
            for (coordinates::coordinate_type x = 0u; x <= 7u; ++x)
                if (square_rect(coordinates{ x, y }).contains(aPosition))
                    return coordinates{ x, y };
        return {};
    }
}