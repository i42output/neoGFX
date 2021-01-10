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

#include <neogfx/core/easing.hpp>
#include <neogfx/app/i_app.hpp>
#include <neogfx/app/action.hpp>
#include <neogfx/gui/window/context_menu.hpp>

#include <chess/board.hpp>

namespace chess::gui
{
    constexpr std::chrono::seconds SHOW_VALID_MOVES_AFTER_s{ 2 };

    board::board(ng::i_layout& aLayout, i_move_validator const& aMoveValidator) :
        ng::widget<>{ aLayout },
        iMoveValidator{ aMoveValidator },
        iTurn{ player::Invalid },
        iAnimator{ ng::service<ng::i_async_task>(), [this](neolib::callback_timer&) { animate(); }, std::chrono::milliseconds{ 20 } },
        iShowValidMoves{ false },
        iEditBoard{ false }
    {
        ng::image const piecesImage{ ":/chess/resources/pieces.png" };
        ng::size const pieceExtents{ piecesImage.extents().cy / 2.0 };
        iPieceTextures.emplace(piece::Pawn, ng::texture{ piecesImage, ng::rect{ ng::point{ pieceExtents.cx * static_cast<double>(piece_cardinal::Pawn), 0.0 }, pieceExtents } });
        iPieceTextures.emplace(piece::Knight, ng::texture{ piecesImage, ng::rect{ ng::point{ pieceExtents.cx * static_cast<double>(piece_cardinal::Knight), 0.0 }, pieceExtents } });
        iPieceTextures.emplace(piece::Bishop, ng::texture{ piecesImage, ng::rect{ ng::point{ pieceExtents.cx * static_cast<double>(piece_cardinal::Bishop), 0.0 }, pieceExtents } });
        iPieceTextures.emplace(piece::Rook, ng::texture{ piecesImage, ng::rect{ ng::point{ pieceExtents.cx * static_cast<double>(piece_cardinal::Rook), 0.0 }, pieceExtents } });
        iPieceTextures.emplace(piece::Queen, ng::texture{ piecesImage, ng::rect{ ng::point{ pieceExtents.cx * static_cast<double>(piece_cardinal::Queen), 0.0 }, pieceExtents } });
        iPieceTextures.emplace(piece::King, ng::texture{ piecesImage, ng::rect{ ng::point{ pieceExtents.cx * static_cast<double>(piece_cardinal::King), 0.0 }, pieceExtents } });

        reset();
    }

    void board::paint(ng::i_graphics_context& aGc) const
    {
        if (iEditBoard)
            draw_alpha_background(aGc, board_rect(), std::min<ng::scalar>(square_rect({ 0, 0 }).width() / 4, 8.0_dip));
        auto const now = std::chrono::steady_clock::now();
        int32_t constexpr RENDER_BOARD                  = 1;
        int32_t constexpr RENDER_NON_SELECTED_PIECES    = 2;
        int32_t constexpr RENDER_SELECTED_PIECES        = 3;
        int32_t constexpr RENDER_ANIMATIONS             = 4;
        for (int32_t pass = RENDER_BOARD; pass <= RENDER_ANIMATIONS; ++pass)
            for (coordinates::coordinate_type y = 0u; y <= 7u; ++y)
                for (coordinates::coordinate_type x = 0u; x <= 7u; ++x)
                {
                    auto const squareRect = square_rect({ x, y });
                    switch (pass)
                    {
                    case RENDER_BOARD:
                        {
                            bool const canMove = iSelection && iMoveValidator.can_move(iTurn, iBoard, chess::move{ *iSelection, coordinates{x, y} });
                            auto squareColor = (x + y) % 2 == 0 ? ng::color::Gray25 : ng::color::Burlywood;
                            if (iCursor && *iCursor == coordinates{ x, y } && iCursor != iSelection)
                                squareColor = palette_color(ng::color_role::Selection).with_alpha(canMove || !iSelection ? 1.0 : 0.5);
                            else if (iSelection && *iSelection == coordinates{ x, y })
                                squareColor = ng::color::White;
                            aGc.fill_rect(squareRect, squareColor.with_combined_alpha(iEditBoard ? 0.75 : 1.0));
                            if (canMove && iLastSelectionEventTime && entered())
                            {
                                auto since = std::chrono::steady_clock::now() - *iLastSelectionEventTime;
                                if (since > SHOW_VALID_MOVES_AFTER_s)
                                {
                                    auto constexpr flashInterval_ms = 1000;
                                    auto const normalizedFrameTime = ((std::chrono::duration_cast<std::chrono::milliseconds>(since).count() + flashInterval_ms / 2) % flashInterval_ms) / ((flashInterval_ms - 1) * 1.0);
                                    auto const cursorAlpha = ng::partitioned_ease(ng::easing::InvertedInOutQuint, ng::easing::InOutQuint, normalizedFrameTime) * 0.75;
                                    aGc.fill_rect(squareRect, palette_color(ng::color_role::Selection).with_alpha(cursorAlpha));
                                }
                            }
                        }
                        break;
                    case RENDER_NON_SELECTED_PIECES:
                    case RENDER_SELECTED_PIECES:
                        if (!animating(coordinates{ x, y }, now))
                        {
                            bool selectedOccupier = (iSelection  && *iSelection == coordinates{ x, y });
                            if ((pass == RENDER_NON_SELECTED_PIECES && selectedOccupier) || (pass == RENDER_SELECTED_PIECES && !selectedOccupier))
                                continue;
                            auto const occupier = iBoard.position[y][x];
                            if (occupier != piece::None)
                            {
                                auto pieceColor = piece_color(occupier) == piece::White ? ng::color::Goldenrod : ng::color::Silver;
                                ng::point mousePosition = root().mouse_position() - origin();
                                auto adjust = (!selectedOccupier || !iSelectionPosition || (mousePosition - *iSelectionPosition).magnitude() < 8.0 ?
                                    ng::point{} : mousePosition - *iSelectionPosition);
                                aGc.draw_texture(squareRect + adjust, iPieceTextures.at(piece_type(occupier)), ng::gradient{ pieceColor.lighter(0x80), pieceColor }, ng::shader_effect::Colorize);
                            }
                        }
                        else
                        {
                            auto const occupier = iAnimations.front().capturedPiece;
                            if (occupier != piece::None)
                            {
                                auto pieceColor = piece_color(occupier) == piece::White ? ng::color::Goldenrod : ng::color::Silver;
                                aGc.draw_texture(squareRect, iPieceTextures.at(piece_type(occupier)), ng::gradient{ pieceColor.lighter(0x80), pieceColor }, ng::shader_effect::Colorize);
                            }
                        }
                        break;
                    case RENDER_ANIMATIONS:
                        if (auto pos = animating(coordinates{ x, y }, now))
                        {
                            auto const& animation = iAnimations.front();
                            auto const occupier = iBoard.position[animation.move.to.y][animation.move.to.x];
                            if (occupier != piece::None)
                            {
                                auto pieceColor = piece_color(occupier) == piece::White ? ng::color::Goldenrod : ng::color::Silver;
                                aGc.draw_texture(ng::rect{ *pos, squareRect.extents() }, iPieceTextures.at(piece_type(occupier)), ng::gradient{ pieceColor.lighter(0x80), pieceColor }, ng::shader_effect::Colorize);
                            }
                        }
                        break;
                    }
                }
    }

    void board::mouse_button_pressed(ng::mouse_button aButton, const ng::point& aPosition, ng::key_modifiers_e aKeyModifiers)
    {
        widget<>::mouse_button_pressed(aButton, aPosition, aKeyModifiers);
        if (!iAnimations.empty())
        {
            iAnimations.pop_front();
            update();
        }
        if (aButton == ng::mouse_button::Left && capturing())
        {
            auto const pos = at(aPosition);
            auto const pieceColor = piece_color(iBoard.position[pos->y][pos->x]);
            if (!iSelection)
            {
                if (pos && pieceColor == static_cast<piece>(iTurn) || (iEditBoard && pieceColor != piece::None))
                {
                    iSelectionPosition = aPosition;
                    iSelection = pos;
                    iLastSelectionEventTime = std::chrono::steady_clock::now();
                }
            }
            else
            {
                if (pos == iSelection)
                {
                    iSelectionPosition = std::nullopt;
                    iSelection = std::nullopt;
                    iLastSelectionEventTime = std::nullopt;
                }
                else if (pieceColor == static_cast<piece>(iTurn) && !iEditBoard)
                {
                    iSelectionPosition = aPosition;
                    iSelection = pos;
                    iLastSelectionEventTime = std::chrono::steady_clock::now();
                }
                else if (iMoveValidator.can_move(iTurn, iBoard, chess::move{ *iSelection, *pos }) || iEditBoard)
                {
                    chess::move move{ *iSelection, *pos };
                    iSelectionPosition = std::nullopt;
                    iSelection = std::nullopt;
                    iLastSelectionEventTime = std::nullopt;
                    iAnimations.emplace_back(move, iBoard.position[move.to.y][move.to.x]);
                    if (piece_type(iBoard.position[move.from.y][move.from.x]) == piece::King)
                    {
                        if (move.from.x - move.to.x == 2u)
                        {
                            // queenside castling
                            iAnimations.emplace_back(chess::move{ coordinates{ 0u, move.from.y }, coordinates{ 3u, move.to.y } });
                        }
                        else if (move.to.x - move.from.x == 2u)
                        {
                            // kingside castling
                            iAnimations.emplace_back(chess::move{ coordinates{ 7u, move.from.y }, coordinates{ 5u, move.to.y } });
                        }
                    }
                    play(move);
                }
            }
        }
        else if (aButton == ng::mouse_button::Right)
        {
            iSelectionPosition = std::nullopt;
            iSelection = std::nullopt;
            iLastSelectionEventTime = std::nullopt;

            ng::context_menu contextMenu{ *this, aPosition + non_client_rect().top_left() + root().window_position() };
            ng::action actionEditBoard{ "Edit Board"_t };
            actionEditBoard.set_checkable(true);
            actionEditBoard.set_checked(iEditBoard);
            contextMenu.menu().add_action(actionEditBoard);
            actionEditBoard.Checked([&]() { iEditBoard = true; });
            actionEditBoard.Unchecked([&]() { iEditBoard = false; });
            contextMenu.exec();
        }
        update();
    }

    void board::mouse_button_double_clicked(ng::mouse_button aButton, const ng::point& aPosition, ng::key_modifiers_e aKeyModifiers)
    {
        widget<>::mouse_button_double_clicked(aButton, aPosition, aKeyModifiers);
    }

    void board::mouse_button_released(ng::mouse_button aButton, const ng::point& aPosition)
    {
        bool wasCapturing = capturing();
        widget<>::mouse_button_released(aButton, aPosition);
        if (wasCapturing)
        {
            if (iSelection)
            {
                iSelectionPosition = std::nullopt;
                auto pos = at(aPosition);
                if (pos && pos != *iSelection && (iMoveValidator.can_move(iTurn, iBoard, chess::move{ *iSelection, *pos }) || iEditBoard))
                {
                    play(chess::move{ *iSelection, *pos });
                    iSelection = std::nullopt;
                    iLastSelectionEventTime = std::nullopt;
                }
            }
        }
        update();
    }

    void board::mouse_moved(const ng::point& aPosition, ng::key_modifiers_e aKeyModifiers)
    {
        auto oldCursor = iCursor;
        iCursor = at(aPosition);
        if (iLastSelectionEventTime && iCursor != oldCursor)
            iLastSelectionEventTime = std::chrono::steady_clock::now();
        update();
    }

    void board::mouse_entered(const ng::point& aPosition)
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
        setup(player::White, chess::matrix_board_setup);
    }

    void board::setup(player aTurn, chess::board const& aBoard)
    {
        iBoard = aBoard;
        iTurn = aTurn;
    }

    bool board::play(chess::move const& aMove)
    {
        if (!iEditBoard)
        {
            // todo: update engine
            if (!iMoveValidator.can_move(iTurn, iBoard, aMove))
                return false;
            move_piece(iBoard, aMove);
            iTurn = next_player(iTurn);
            update();
        }
        else
            edit(aMove);
        return true;
    }

    void board::edit(chess::move const& aMove)
    {
        auto& source = iBoard.position[aMove.from.y][aMove.from.x];
        auto const movingPiece = source;
        auto& destination = iBoard.position[aMove.to.y][aMove.to.x];
        auto const targetPiece = destination;
        destination = source;
        source = piece::None;
        if (piece_type(destination) == piece::King)
            iBoard.kings[as_color_cardinal<>(destination)] = aMove.to;
        iBoard.lastMove = std::nullopt;
        update();
    }
        
    std::optional<ng::point> board::animating(coordinates const& aMovePos, std::chrono::steady_clock::time_point const& aTime) const
    {
        if (!iAnimations.empty())
        {
            auto const& animation = iAnimations.front();
            if (animation.move.to == aMovePos)
            {
                if (!animation.startTime)
                    animation.startTime = std::chrono::steady_clock::now();
                auto const start = square_rect(animation.move.from).top_left();
                auto const end = square_rect(animation.move.to).top_left();
                auto const maxDistance = (coordinates{ 0, 0 }.as<ng::scalar>() - coordinates{ 8, 8 }.as<ng::scalar>()).magnitude();
                auto const distance = (animation.move.to.as<ng::scalar>() - animation.move.from.as<ng::scalar>()).magnitude();
                auto const elapsed = std::chrono::duration_cast<std::chrono::duration<ng::scalar>>(aTime - *animation.startTime).count() * (maxDistance / distance);
                if (elapsed <= 1.0)
                    return start + (end - start) * elapsed;
            }
        }
        return {};
    }
    
    void board::animate()
    {
        iAnimator.again();
        if (iLastSelectionEventTime || !iAnimations.empty())
        {
            if (!iAnimations.empty())
            {
                auto& animation = iAnimations.front();
                if (animation.startTime)
                {
                    auto const maxDistance = (coordinates{ 0, 0 }.as<ng::scalar>() - coordinates{ 8, 8 }.as<ng::scalar>()).magnitude();
                    auto const distance = (animation.move.to.as<ng::scalar>() - animation.move.from.as<ng::scalar>()).magnitude();
                    auto const elapsed = std::chrono::duration_cast<std::chrono::duration<ng::scalar>>(std::chrono::steady_clock::now() - *animation.startTime).count() * (maxDistance / distance);
                    if (elapsed > 1.0)
                        iAnimations.pop_front();
                }
            }
            update();
        }
    }

    ng::rect board::board_rect() const
    {
        auto result = client_rect(false);
        if (result.cx > result.cy)
            result.deflate((result.cx - result.cy) / 2.0, 0.0);
        else
            result.deflate(0.0, (result.cy - result.cx) / 2.0);
        return result;
    }

    ng::rect board::square_rect(coordinates aCoordinates) const
    {
        auto const boardRect = board_rect();
        ng::size squareDimensions{ boardRect.extents() / 8.0 };
        return ng::rect{ boardRect.top_left() + ng::point{ squareDimensions * ng::size_u32{ aCoordinates.x, 7u - aCoordinates.y }.as<ng::scalar>() }, squareDimensions };
    }

    std::optional<coordinates> board::at(ng::point const& aPosition) const
    {
        for (coordinates::coordinate_type y = 0u; y <= 7u; ++y)
            for (coordinates::coordinate_type x = 0u; x <= 7u; ++x)
                if (square_rect(coordinates{ x, y }).contains(aPosition))
                    return coordinates{ x, y };
        return {};
    }
}