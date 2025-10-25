﻿/*
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
#include <neogfx/gui/widget/i_menu_item_widget.hpp>
#include <chess/board.hpp>

namespace chess::gui
{
    constexpr std::chrono::seconds SHOW_VALID_MOVES_AFTER_s{ 2 };
    constexpr ng::scalar BORDER = 24.0;

    board::board(ng::i_layout& aLayout, i_move_validator const& aMoveValidator) :
        ng::widget<>{ aLayout },
        iMoveValidator{ aMoveValidator },
        iWhitePlayer{ nullptr },
        iBlackPlayer{ nullptr },
        iInRedo{ false },
        iFlipped{ false },
        iColorWhiteSquare{ 235, 236, 208 },
        iColorBlackSquare{ 119, 149, 86 },
        iColorWhitePiece{ ng::color::Gray90 },
        iColorBlackPiece{ ng::color::Gray10 },
        iColorizePieces{ true },
        iAnimator{ *this, [this](ng::widget_timer&) { animate(); }, std::chrono::milliseconds{ 20 } },
        iSquareIdentification{ square_identification::None },
        iShowValidMoves{ false },
        iEditBoard{ false }
    {
        ng::image const piecesImage{ ":/chess/resources/pieces.png" };
        ng::size const pieceExtents{ piecesImage.extents().cy / 2.0 };
        iPieceTextures.emplace(piece::WhitePawn, ng::texture{ piecesImage, ng::rect{ ng::point{ pieceExtents.cx * static_cast<double>(piece_cardinal::Pawn), 0.0 }, pieceExtents } });
        iPieceTextures.emplace(piece::WhiteKnight, ng::texture{ piecesImage, ng::rect{ ng::point{ pieceExtents.cx * static_cast<double>(piece_cardinal::Knight), 0.0 }, pieceExtents } });
        iPieceTextures.emplace(piece::WhiteBishop, ng::texture{ piecesImage, ng::rect{ ng::point{ pieceExtents.cx * static_cast<double>(piece_cardinal::Bishop), 0.0 }, pieceExtents } });
        iPieceTextures.emplace(piece::WhiteRook, ng::texture{ piecesImage, ng::rect{ ng::point{ pieceExtents.cx * static_cast<double>(piece_cardinal::Rook), 0.0 }, pieceExtents } });
        iPieceTextures.emplace(piece::WhiteQueen, ng::texture{ piecesImage, ng::rect{ ng::point{ pieceExtents.cx * static_cast<double>(piece_cardinal::Queen), 0.0 }, pieceExtents } });
        iPieceTextures.emplace(piece::WhiteKing, ng::texture{ piecesImage, ng::rect{ ng::point{ pieceExtents.cx * static_cast<double>(piece_cardinal::King), 0.0 }, pieceExtents } });
        iPieceTextures.emplace(piece::BlackPawn, ng::texture{ piecesImage, ng::rect{ ng::point{ pieceExtents.cx * static_cast<double>(piece_cardinal::Pawn), iColorizePieces ? 0.0 : pieceExtents.cy }, pieceExtents } });
        iPieceTextures.emplace(piece::BlackKnight, ng::texture{ piecesImage, ng::rect{ ng::point{ pieceExtents.cx * static_cast<double>(piece_cardinal::Knight), iColorizePieces ? 0.0 : pieceExtents.cy }, pieceExtents } });
        iPieceTextures.emplace(piece::BlackBishop, ng::texture{ piecesImage, ng::rect{ ng::point{ pieceExtents.cx * static_cast<double>(piece_cardinal::Bishop), iColorizePieces ? 0.0 : pieceExtents.cy }, pieceExtents } });
        iPieceTextures.emplace(piece::BlackRook, ng::texture{ piecesImage, ng::rect{ ng::point{ pieceExtents.cx * static_cast<double>(piece_cardinal::Rook), iColorizePieces ? 0.0 : pieceExtents.cy }, pieceExtents } });
        iPieceTextures.emplace(piece::BlackQueen, ng::texture{ piecesImage, ng::rect{ ng::point{ pieceExtents.cx * static_cast<double>(piece_cardinal::Queen), iColorizePieces ? 0.0 : pieceExtents.cy }, pieceExtents } });
        iPieceTextures.emplace(piece::BlackKing, ng::texture{ piecesImage, ng::rect{ ng::point{ pieceExtents.cx * static_cast<double>(piece_cardinal::King), iColorizePieces ? 0.0 : pieceExtents.cy }, pieceExtents } });

        set_focus();
    }

    board::~board()
    {
        white_player().finish();
        black_player().finish();
    }

    void board::paint(ng::i_graphics_context& aGc) const
    {
        if (iEditBoard)
            draw_alpha_background(aGc, board_rect(), std::min<ng::scalar>(square_rect({ 0, 0 }).width() / 4, 8.0_dip));
        auto const now = std::chrono::steady_clock::now();
        std::int32_t constexpr RENDER_BOARD                  = 1;
        std::int32_t constexpr RENDER_NON_SELECTED_PIECES    = 2;
        std::int32_t constexpr RENDER_SELECTED_PIECES        = 3;
        std::int32_t constexpr RENDER_ANIMATIONS             = 4;
        coordinate const coordMin = (!iFlipped ? 0u : 7u);
        coordinate const coordMax = (!iFlipped ? 0u : 7u);
        for (std::int32_t pass = RENDER_BOARD; pass <= RENDER_ANIMATIONS; ++pass)
            for (coordinate y = 0u; y <= 7u; ++y)
                for (coordinate x = 0u; x <= 7u; ++x)
                {
                    auto const squareRect = square_rect({ x, y });
                    auto const labelPadding = 2.0_dip * scale();
                    auto const labelRect = squareRect.deflated(ng::size{ labelPadding });
                    auto const pieceRect = piece_rect({ x, y });
                    auto squareColor = (x + y) % 2 == 0 ? iColorBlackSquare : iColorWhiteSquare;
                    auto labelColor = (x + y) % 2 == 0 ? iColorWhiteSquare : iColorBlackSquare;
                    bool const colorizePieces = true;
                    switch (pass)
                    {
                    case RENDER_BOARD:
                        {
                            bool const canMove = iSelection && iMoveValidator.can_move(iPosition.turn, iPosition, chess::move{ *iSelection, coordinates{x, y} });
                            auto const& labelFont = font();
                            auto yLabel = std::string{ static_cast<char>(y + (iSquareIdentification != square_identification::Debug ? '1' : '0')) };
                            auto xLabel = std::string{ static_cast<char>(x + (iSquareIdentification != square_identification::Debug ? 'a' : '0')) };
                            auto yLabelExtents = aGc.text_extent(yLabel, labelFont);
                            auto xLabelExtents = aGc.text_extent(xLabel, labelFont);
                            bool labelCursor = false;
                            if (iCursor && *iCursor == coordinates{ x, y } && iCursor != iSelection)
                            {
                                squareColor = palette_color(ng::color_role::Selection).with_alpha(canMove || !iSelection ? 1.0 : 0.5);
                                labelColor = squareColor.shaded(0x60);
                                labelCursor = canMove && (iSquareIdentification == square_identification::InnerExtra);
                            }
                            else if (iSelection && *iSelection == coordinates{ x, y })
                            {
                                squareColor = ng::color::White;
                                labelColor = squareColor.shaded(0x60);
                                labelCursor = (iSquareIdentification == square_identification::InnerExtra);
                            }
                            aGc.fill_rect(squareRect, squareColor.with_combined_alpha(iEditBoard ? 0.75 : 1.0));
                            if (canMove && iLastSelectionEventTime && entered() && !labelCursor)
                            {
                                auto since = std::chrono::steady_clock::now() - *iLastSelectionEventTime;
                                if (since > SHOW_VALID_MOVES_AFTER_s)
                                {
                                    auto constexpr flashInterval_ms = 1000;
                                    auto const normalizedFrameTime = ((std::chrono::duration_cast<std::chrono::milliseconds>(since).count() + flashInterval_ms / 2) % flashInterval_ms) / ((flashInterval_ms - 1) * 1.0);
                                    auto const cursorAlpha = ng::partitioned_ease(ng::easing::InvertedInOutQuint, ng::easing::InOutQuint, normalizedFrameTime) * 0.75;
                                    auto cursorColor = palette_color(ng::color_role::Selection).with_alpha(cursorAlpha);
                                    labelColor = ng::mix(labelColor, cursorColor.with_alpha(1.0).shaded(0x60), cursorAlpha / 0.75);
                                    labelCursor = (iSquareIdentification == square_identification::InnerExtra);
                                    aGc.fill_rect(squareRect, cursorColor);
                                }
                            }
                            switch (iSquareIdentification)
                            {
                            case square_identification::None:
                                break;
                            case square_identification::Outer:
                                if (x == coordMin)
                                    aGc.draw_text(ng::point{ squareRect.left() - (dpi_scale(BORDER) * scale() - yLabelExtents.cx) / 2.0 - yLabelExtents.cx, squareRect.center().y - yLabelExtents.cy / 2.0 }, yLabel, labelFont, palette_color(ng::color_role::Text));
                                else if (x == coordMax)
                                    aGc.draw_text(ng::point{ squareRect.right() + (dpi_scale(BORDER) * scale() - yLabelExtents.cx) / 2.0, squareRect.center().y - yLabelExtents.cy / 2.0 }, yLabel, labelFont, palette_color(ng::color_role::Text));
                                if (y == coordMin)
                                    aGc.draw_text(ng::point{ squareRect.center().x - xLabelExtents.cx / 2.0, squareRect.bottom() + (dpi_scale(BORDER) * scale() - xLabelExtents.cy) / 2.0 }, xLabel, labelFont, palette_color(ng::color_role::Text));
                                else if (y == coordMax)
                                    aGc.draw_text(ng::point{ squareRect.center().x - xLabelExtents.cx / 2.0, squareRect.top() - (dpi_scale(BORDER) * scale() - xLabelExtents.cy) / 2.0 - xLabelExtents.cy }, xLabel, labelFont, palette_color(ng::color_role::Text));
                                break;
                            case square_identification::Inner:
                            case square_identification::InnerExtra:
                                if (labelCursor)
                                {
                                    aGc.draw_text(labelRect.top_left(), yLabel, labelFont, labelColor);
                                    aGc.draw_text(labelRect.bottom_right() - xLabelExtents, xLabel, labelFont, labelColor);
                                }
                                else
                                {
                                    if (x == coordMin)
                                        aGc.draw_text(labelRect.top_left(), yLabel, labelFont, labelColor);
                                    if (y == coordMin)
                                        aGc.draw_text(labelRect.bottom_right() - xLabelExtents, xLabel, labelFont, labelColor);
                                }
                                break;
                            case square_identification::Debug:
                                if (x == coordMin)
                                    aGc.draw_text(ng::point{ squareRect.left() - (dpi_scale(BORDER) * scale() - yLabelExtents.cx) / 2.0 - yLabelExtents.cx, squareRect.center().y - yLabelExtents.cy / 2.0 }, yLabel, labelFont, palette_color(ng::color_role::SecondaryAccent));
                                else if (x == coordMax)
                                    aGc.draw_text(ng::point{ squareRect.right() + (dpi_scale(BORDER) * scale() - yLabelExtents.cx) / 2.0, squareRect.center().y - yLabelExtents.cy / 2.0 }, yLabel, labelFont, palette_color(ng::color_role::SecondaryAccent));
                                if (y == coordMin)
                                    aGc.draw_text(ng::point{ squareRect.center().x - xLabelExtents.cx / 2.0, squareRect.bottom() + (dpi_scale(BORDER) * scale() - xLabelExtents.cy) / 2.0 }, xLabel, labelFont, palette_color(ng::color_role::SecondaryAccent));
                                else if (y == coordMax)
                                    aGc.draw_text(ng::point{ squareRect.center().x - xLabelExtents.cx / 2.0, squareRect.top() - (dpi_scale(BORDER) * scale() - xLabelExtents.cy) / 2.0 - xLabelExtents.cy }, xLabel, labelFont, palette_color(ng::color_role::SecondaryAccent));
                                break;
                            }
                        }
                        break;
                    case RENDER_NON_SELECTED_PIECES:
                    case RENDER_SELECTED_PIECES:
                        if (!animating_to(coordinates{ x, y }, now) && !animating_from(coordinates{ x, y }))
                        {
                            bool selectedOccupier = (iSelection  && *iSelection == coordinates{ x, y });
                            if ((pass == RENDER_NON_SELECTED_PIECES && selectedOccupier) || (pass == RENDER_SELECTED_PIECES && !selectedOccupier))
                                continue;
                            auto const occupier = iPosition.rep[y][x];
                            if (occupier != piece::None)
                            {
                                auto pieceColor = piece_color(occupier) == piece::White ? iColorWhitePiece : iColorBlackPiece;
                                bool useGradient = true;
                                if (iFlashCheck && (iMoveValidator.in_check(iPosition.turn, iPosition) || iFlashCheck->first) && piece_type(occupier) == piece::King && piece_color(occupier) == static_cast<piece>(iPosition.turn))
                                {
                                    useGradient = false;
                                    auto const since = std::chrono::steady_clock::now() - iFlashCheck->second;
                                    auto constexpr flashInterval_ms = 500;
                                    auto const normalizedFrameTime = ((std::chrono::duration_cast<std::chrono::milliseconds>(since).count() + flashInterval_ms / 2) % flashInterval_ms) / ((flashInterval_ms - 1) * 1.0);
                                    pieceColor = ng::mix(ng::color::Red, ng::color::White, ng::partitioned_ease(ng::easing::InvertedInOutQuint, ng::easing::InOutQuint, normalizedFrameTime));
                                }
                                ng::point const mousePosition = mouse_position();
                                auto adjust = (!selectedOccupier || !iSelectionPosition || (mousePosition - *iSelectionPosition).magnitude() < 8.0 ?
                                    ng::point{} : mousePosition - *iSelectionPosition);
                                if (iColorizePieces)
                                    aGc.draw_texture(pieceRect + adjust, iPieceTextures.at(occupier), useGradient ? ng::gradient{ pieceColor.lighter(0x80), pieceColor } : ng::color_or_gradient{ pieceColor }, ng::shader_effect::Colorize);
                                else
                                    aGc.draw_texture(pieceRect + adjust, iPieceTextures.at(occupier));
                            }
                        }
                        else if (animating_to(coordinates{ x, y }, now))
                        {
                            auto const& animation = animating_to(coordinates{ x, y }, now);
                            auto const occupier = animation->first->capturedPiece;
                            if (occupier != piece::None)
                            {
                                if (iColorizePieces)
                                {
                                    auto pieceColor = piece_color(occupier) == piece::White ? iColorWhitePiece : iColorBlackPiece;
                                    aGc.draw_texture(pieceRect, iPieceTextures.at(occupier), ng::gradient{ pieceColor.lighter(0x80), pieceColor }, ng::shader_effect::Colorize);
                                }
                                else
                                    aGc.draw_texture(pieceRect, iPieceTextures.at(occupier));
                            }
                        }
                        break;
                    case RENDER_ANIMATIONS:
                        if (auto const& animation = animating_to(coordinates{ x, y }, now))
                        {
                            auto const occupier = animation->first->movingPiece;
                            if (occupier != piece::None)
                            {
                                if (iColorizePieces)
                                {
                                    auto const pieceColor = piece_color(occupier) == piece::White ? iColorWhitePiece : iColorBlackPiece;
                                    aGc.draw_texture(ng::rect{ animation->second, pieceRect.extents() }, iPieceTextures.at(occupier), ng::gradient{ pieceColor.lighter(0x80), pieceColor }, ng::shader_effect::Colorize);
                                }
                                else
                                    aGc.draw_texture(ng::rect{ animation->second, pieceRect.extents() }, iPieceTextures.at(occupier));
                            }
                        }
                        break;
                    }
                }
    }

    void board::resized()
    {
        widget<>::resized();
        set_font(ng::service<ng::i_app>().current_style().font().with_size(11 * scale()).with_style(ng::font_style::Bold));
    }

    bool board::key_pressed(ng::scan_code_e aScanCode, ng::key_code_e aKeyCode, ng::key_modifiers_e aKeyModifiers)
    {
        if (aKeyCode == ng::key_code_e::KeyCode_i)
        {
            iSquareIdentification = static_cast<square_identification>((static_cast<std::uint32_t>(iSquareIdentification) + 1u) % static_cast<std::uint32_t>(square_identification::COUNT));
            iAnimations.clear();
            update();
            return true;
        }
        else if (aKeyCode == ng::key_code_e::KeyCode_f)
        {
            iFlipped = !iFlipped;
            iAnimations.clear();
            update();
            return true;
        }
        else if (aKeyCode == ng::key_code_e::KeyCode_SLASH)
        {
            display_query();
            return true;
        }
        else
            return widget<>::key_pressed(aScanCode, aKeyCode, aKeyModifiers);
    }

    ng::focus_policy board::focus_policy() const
    {
        return ng::focus_policy::StrongFocus;
    }

    void board::mouse_button_clicked(ng::mouse_button aButton, const ng::point& aPosition, ng::key_modifiers_e aKeyModifiers)
    {
        widget<>::mouse_button_clicked(aButton, aPosition, aKeyModifiers);
        if (!iAnimations.empty())
        {
            iAnimations.pop_front();
            update();
        }
        if (aButton == ng::mouse_button::Left && capturing())
        {
            auto const pos = at(aPosition);
            if (pos)
            {
                auto const pieceColor = piece_color(iPosition.rep[pos->y][pos->x]);
                bool const correctColor = (pieceColor == static_cast<piece>(iPosition.turn));
                if (pos == iSelection)
                {
                    iSelectionPosition = std::nullopt;
                    iSelection = std::nullopt;
                    iLastSelectionEventTime = std::nullopt;
                }
                else if (!iSelection || (correctColor && !iEditBoard))
                {
                    if ((correctColor && iMoveValidator.has_moves(iPosition.turn, iPosition, *pos)) || (iEditBoard && pieceColor != piece::None))
                    {
                        iSelectionPosition = aPosition;
                        iSelection = pos;
                        iLastSelectionEventTime = std::chrono::steady_clock::now();
                    }
                    else if (correctColor && iMoveValidator.in_check(iPosition.turn, iPosition))
                    {
                        iFlashCheck = std::make_pair(false, std::chrono::steady_clock::now());
                    }
                    else if (correctColor && iMoveValidator.check_if_moved(iPosition.turn, iPosition, *pos))
                    {
                        iFlashCheck = std::make_pair(true, std::chrono::steady_clock::now());
                    }
                }
                else
                {
                    if (iMoveValidator.can_move(iPosition.turn, iPosition, chess::move{ *iSelection, *pos }) || iEditBoard)
                    {
                        chess::move move{ *iSelection, *pos };
                        iSelectionPosition = std::nullopt;
                        iSelection = std::nullopt;
                        iLastSelectionEventTime = std::nullopt;
                        play(move);
                    }
                    else if (iMoveValidator.in_check(iPosition.turn, iPosition))
                    {
                        iFlashCheck = std::make_pair(false, std::chrono::steady_clock::now());
                    }
                }
            }
        }
        else if (aButton == ng::mouse_button::Right)
        {
            iSelectionPosition = std::nullopt;
            iSelection = std::nullopt;
            iLastSelectionEventTime = std::nullopt;

            auto square = at(mouse_position());

            ng::context_menu contextMenu{ *this, aPosition + non_client_rect().top_left() + root().window_position() };
            ng::action actionEditBoard{ "Edit Board"_t };
            ng::action actionErase{ "Erase"_t };
            actionEditBoard.set_checkable(true);
            actionEditBoard.set_checked(iEditBoard);
            contextMenu.menu().add_action(actionEditBoard);
            if (iEditBoard && square)
            {
                contextMenu.menu().add_separator();
                contextMenu.menu().add_action(actionErase);
            }
            actionEditBoard.Checked([&]() { iEditBoard = true; });
            actionEditBoard.Unchecked([&]() { iEditBoard = false; });
            actionErase.Triggered([&]()
            {
                iPosition.rep[square->y][square->x] = piece::None;
                current_player().setup(iPosition);
                next_player().setup(iPosition);
                display_eval();
            });
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
                if (pos && pos != *iSelection && (iMoveValidator.can_move(iPosition.turn, iPosition, chess::move{ *iSelection, *pos }) || iEditBoard))
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

    void board::new_game(i_player_factory& aPlayerFactory, player_type aWhitePlayer, player_type aBlackPlayer)
    {
        setup(chess::setup_position<mailbox_rep>());

        iWhitePlayer = std::move(aPlayerFactory.create_player(aWhitePlayer, player::White));
        iBlackPlayer = std::move(aPlayerFactory.create_player(aBlackPlayer, player::Black));

        white_player().greet(black_player());
        black_player().greet(white_player());

        white_player().moved([&](chess::move const& aMove)
        {
            moved(aMove);
        });
        black_player().moved([&](chess::move const& aMove)
        {
            moved(aMove);
        });

        iUndoneMoves.clear();
        Changed();

        play();
    }

    void board::setup(chess::mailbox_position const& aPosition)
    {
        iPosition = aPosition;
        Changed();
    }

    bool board::play(chess::move const& aMove)
    {
        if (!iEditBoard)
        {
            animate_move(aMove);

            auto const& source = iPosition.rep[aMove.from.y][aMove.from.x];
            auto const movingPiece = source;
            std::optional<piece> promotion = aMove.promoteTo;
            if (piece_type(movingPiece) == piece::Pawn)
            {
                // promotion
                animating_to(aMove.to)->first->hold = true;
                if ((piece_color(movingPiece) == piece::White && aMove.to.y == 7u) || (piece_color(movingPiece) == piece::Black && aMove.to.y == 0u) && !promotion)
                {
                    promotion = piece_color(movingPiece) | piece::Queen;
                    ng::point const mousePosition = mouse_position();
                    ng::context_menu contextMenu{ *this, mousePosition + non_client_rect().top_left() + root().window_position() };
                    ng::action actionQueen{ "", iPieceTextures.at(piece::Queen | piece_color(movingPiece)) };
                    ng::action actionRook{ "", iPieceTextures.at(piece::Rook | piece_color(movingPiece)) };
                    ng::action actionBishop{ "", iPieceTextures.at(piece::Bishop | piece_color(movingPiece)) };
                    ng::action actionKnight{ "", iPieceTextures.at(piece::Knight | piece_color(movingPiece)) };
                    contextMenu.menu().add_action(actionQueen);
                    contextMenu.menu().add_action(actionRook);
                    contextMenu.menu().add_action(actionBishop);
                    contextMenu.menu().add_action(actionKnight);
                    contextMenu.menu().opened([&]()
                    {
                        contextMenu.menu().item_at(0).as_widget().item_icon().set_fixed_size(ng::size{ 48.0_dip, 48.0_dip });
                        contextMenu.menu().item_at(1).as_widget().item_icon().set_fixed_size(ng::size{ 48.0_dip, 48.0_dip });
                        contextMenu.menu().item_at(2).as_widget().item_icon().set_fixed_size(ng::size{ 48.0_dip, 48.0_dip });
                        contextMenu.menu().item_at(3).as_widget().item_icon().set_fixed_size(ng::size{ 48.0_dip, 48.0_dip });
                        auto const pieceColor = piece_color(movingPiece) == piece::White ? iColorWhitePiece : iColorBlackPiece;
                        contextMenu.menu().item_at(0).as_widget().item_icon().set_image_color(ng::gradient{ pieceColor.lighter(0x80), pieceColor });
                        contextMenu.menu().item_at(1).as_widget().item_icon().set_image_color(ng::gradient{ pieceColor.lighter(0x80), pieceColor });
                        contextMenu.menu().item_at(2).as_widget().item_icon().set_image_color(ng::gradient{ pieceColor.lighter(0x80), pieceColor });
                        contextMenu.menu().item_at(3).as_widget().item_icon().set_image_color(ng::gradient{ pieceColor.lighter(0x80), pieceColor });
                    });
                    actionQueen.Triggered([&]() { promotion = piece_color(movingPiece) | piece::Queen; });
                    actionRook.Triggered([&]() { promotion = piece_color(movingPiece) | piece::Rook; });
                    actionBishop.Triggered([&]() { promotion = piece_color(movingPiece) | piece::Bishop; });
                    actionKnight.Triggered([&]() { promotion = piece_color(movingPiece) | piece::Knight; });
                    contextMenu.exec();
                }
                animating_to(aMove.to)->first->hold = false;
            }
            current_player().play(chess::move{ aMove.from, aMove.to, aMove.isCapture, promotion });
        }
        else
            edit(aMove);
        Changed();
        return true;
    }

    void board::edit(chess::move const& aMove)
    {
        auto& source = iPosition.rep[aMove.from.y][aMove.from.x];
        auto const movingPiece = source;
        auto& destination = iPosition.rep[aMove.to.y][aMove.to.x];
        auto const targetPiece = destination;
        destination = source;
        source = piece::None;
        if (piece_type(destination) == piece::King)
            iPosition.kings[as_color_cardinal<>(destination)] = aMove.to;
        iPosition.moveHistory.clear();
        current_player().setup(iPosition);
        next_player().setup(iPosition);
        display_eval();
        Changed();
        update();
    }

    bool board::can_undo() const
    {
        return !iPosition.moveHistory.empty();
    }

    void board::undo()
    {
        if (can_undo())
        {
            iUndoneMoves.push_back(*unmake(iPosition));
            current_player().undo();
            next_player().undo();
            Changed();
            update();
        }
    }

    bool board::can_redo() const
    {
        return !iUndoneMoves.empty();
    }

    void board::redo()
    {
        if (can_redo())
        {
            neolib::scoped_flag sf{ iInRedo };
            auto redoMove = iUndoneMoves.back();
            iUndoneMoves.pop_back();
            if (piece_color(piece_at(iPosition, redoMove.to)) == piece::White)
                white_player().play(redoMove);
            else
                black_player().play(redoMove);
            Changed();
            update();
        }
    }

    bool board::can_play() const
    {
        return current_player().type() == player_type::AI && !current_player().playing();
    }

    void board::play()
    {
        if (!current_player().playing())
        {
            current_player().play();
            Changed();
        }
    }

    bool board::can_stop() const
    {
        return current_player().playing();
    }

    void board::stop()
    {
        if (current_player().playing())
        {
            current_player().stop();
            Changed();
        }
    }

    i_player const& board::current_player() const
    {
        switch (iPosition.turn)
        {
        case player::White:
            return white_player();
        case player::Black:
            return black_player();
        default:
            throw no_player();
        }
    }

    i_player& board::current_player()
    {
        return const_cast<i_player&>(ng::to_const(*this).current_player());
    }

    i_player const& board::next_player() const
    {
        switch (iPosition.turn)
        {
        case player::White:
            return black_player();
        case player::Black:
            return white_player();
        default:
            throw no_player();
        }
    }

    i_player& board::next_player()
    {
        return const_cast<i_player&>(ng::to_const(*this).next_player());
    }

    i_player const& board::white_player() const
    {
        if (iWhitePlayer != nullptr)
            return *iWhitePlayer;
        throw no_player();
    }

    i_player& board::white_player()
    {
        if (iWhitePlayer != nullptr)
            return *iWhitePlayer;
        throw no_player();
    }

    i_player const& board::black_player() const
    {
        if (iBlackPlayer != nullptr)
            return *iBlackPlayer;
        throw no_player();
    }

    i_player& board::black_player()
    {
        if (iBlackPlayer != nullptr)
            return *iBlackPlayer;
        throw no_player();
    }
        
    void board::moved(chess::move const& aMove)
    {
        std::cerr << to_string(aMove) << std::endl; // todo: remove
        if (current_player().type() != player_type::Human)
            animate_move(aMove);
        make(iPosition, aMove);
        if (iMoveValidator.in_check(iPosition.turn, iPosition))
            iFlashCheck = std::make_pair(false, std::chrono::steady_clock::now());
        display_eval();
        if (!iInRedo)
            iUndoneMoves.clear();
        update();
    }

    template <typename CharT, typename CharTraitsT>
    inline std::basic_ostream<CharT, CharTraitsT>& operator<<(std::basic_ostream<CharT, CharTraitsT>& aStream, eval_info const& aEvalInfo)
    {
        aStream << "material: " << std::round(aEvalInfo.material) << std::endl;
        aStream << "mobility: " << std::round(aEvalInfo.mobility) << std::endl;
        aStream << "attack: " << std::round(aEvalInfo.attack) << std::endl;
        aStream << "defend: " << std::round(aEvalInfo.defend) << std::endl;
        aStream << "mobilityPlayer: " << aEvalInfo.mobilityPlayer << std::endl;
        aStream << "mobilityPlayerKing: " << aEvalInfo.mobilityPlayerKing << std::endl;
        aStream << "checkedPlayerKing: " << std::round(aEvalInfo.checkedPlayerKing) << std::endl;
        aStream << "mobilityOpponent: " << aEvalInfo.mobilityOpponent << std::endl;
        aStream << "mobilityOpponentKing: " << aEvalInfo.mobilityOpponentKing << std::endl;
        aStream << "checkedOpponentKing: " << std::round(aEvalInfo.checkedOpponentKing) << std::endl;
        aStream << "eval: " << std::round(aEvalInfo.eval) << std::endl;
        aStream << "eval time: " << aEvalInfo.time_usec.count() << " us" << std::endl;
        return aStream;
    }

    void board::display_eval() const
    {
        eval_info evalInfo;
        double eval = iMoveValidator.eval(current_player().player(), iPosition, evalInfo);
        std::cerr << std::setprecision(4);
        if (iEditBoard)
            std::cerr << "[EDIT BOARD]" << std::endl;
        std::cerr << evalInfo;
    }

    void board::display_query() const
    {
        std::cerr << std::endl << "[QUERY START]" << std::endl << std::endl;
        std::vector<std::pair<chess::move, eval_info>> results;
        auto queryBoard = iPosition;
        for (coordinate xFrom = 0u; xFrom <= 7u; ++xFrom)
            for (coordinate yFrom = 0u; yFrom <= 7u; ++yFrom)
                for (coordinate xTo = 0u; xTo<= 7u; ++xTo)
                    for (coordinate yTo = 0u; yTo <= 7u; ++yTo)
                    {
                        chess::move const candidateMove{ coordinates{ xFrom, yFrom }, coordinates{ xTo, yTo } };
                        if (iMoveValidator.can_move(queryBoard.turn, queryBoard, candidateMove))
                        {
                            make(queryBoard, candidateMove);
                            eval_info evalInfo;
                            double eval = iMoveValidator.eval(current_player().player(), queryBoard, evalInfo);
                            results.push_back(std::make_pair(candidateMove, evalInfo));
                            unmake(queryBoard);
                        }
                    }
        std::sort(results.begin(), results.end(), [](auto const& lhs, auto const& rhs) { return lhs.second.eval > rhs.second.eval; });
        for (auto const& m : results)
        {
            std::cerr << "*** " << to_string(m.first) << " ***" << std::endl;
            std::cerr << m.second;
        }
        std::cerr << std::endl << "[QUERY END]" << std::endl << std::endl;
    }

    void board::animate_move(chess::move const& aMove)
    {
        auto const movingPiece = iPosition.rep[aMove.from.y][aMove.from.x];
        auto const targetPiece = iPosition.rep[aMove.to.y][aMove.to.x];
        if (piece_type(movingPiece) == piece::King && aMove.from.x - aMove.to.x == 2u)
        {
            // queenside castling
            iAnimations.emplace_back(aMove, movingPiece);
            iAnimations.emplace_back(chess::move{ coordinates{ 0u, aMove.from.y }, coordinates{ 3u, aMove.to.y } }, iPosition.rep[aMove.from.y][0u]);
        }
        else if (piece_type(movingPiece) == piece::King && aMove.to.x - aMove.from.x == 2u)
        {
            // kingside castling
            iAnimations.emplace_back(aMove, movingPiece);
            iAnimations.emplace_back(chess::move{ coordinates{ 7u, aMove.from.y }, coordinates{ 5u, aMove.to.y } }, iPosition.rep[aMove.from.y][7u]);
        }
        else
            iAnimations.emplace_back(aMove, movingPiece, targetPiece);

    }

    std::optional<std::pair<animation const*, ng::point>> board::animating_to(coordinates const& aMovePos, std::chrono::steady_clock::time_point const& aTime) const
    {
        for (auto const& animation : iAnimations)
        {
            if (animation.move.to == aMovePos)
            {
                if (&animation == &iAnimations.front())
                {
                    if (!animation.startTime)
                        animation.startTime = std::chrono::steady_clock::now();
                    auto const start = piece_rect(animation.move.from).top_left();
                    auto const end = piece_rect(animation.move.to).top_left();
                    auto const maxDistance = (coordinates{ 0, 0 }.as<ng::scalar>() - coordinates{ 8, 8 }.as<ng::scalar>()).magnitude();
                    auto const distance = (animation.move.to.as<ng::scalar>() - animation.move.from.as<ng::scalar>()).magnitude();
                    auto const elapsed = std::min(std::chrono::duration_cast<std::chrono::duration<ng::scalar>>(aTime - *animation.startTime).count() * (maxDistance / distance), 1.0);
                    return std::make_pair(&animation, start + (end - start) * elapsed);
                }
                else
                    return std::make_pair(&animation, piece_rect(animation.move.from).top_left());
            }
        }
        return {};
    }

    bool board::animating_from(coordinates const& aMovePos) const
    {
        for (auto const& animation : iAnimations)
            if (animation.move.from == aMovePos)
                return true;
        return false;
    }
    
    void board::animate()
    {
        iAnimator.again();

        if (!root().has_native_surface()) // todo: shouldn't need this check
            return;

        double nodesPerSecond = static_cast<double>(current_player().nodes_per_second());
        std::ostringstream oss;
        oss << std::setprecision(2) << std::fixed;
        if (nodesPerSecond < 1000.0)
            oss << nodesPerSecond << " N/s";
        else if (nodesPerSecond < 1000000.0)
            oss << nodesPerSecond / 1000.0 << " kN/s";
        else
            oss << nodesPerSecond / 1000000.0 << " MN/s";
        root().status_bar().set_message(ng::string{ oss.str() });

        if (iLastSelectionEventTime || !iAnimations.empty() || iFlashCheck)
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
                    {
                        animation.capturedPiece = piece::None;
                        if (!animation.hold)
                            iAnimations.pop_front();
                    }
                }
                if (iAnimations.empty())
                    play();
            }
            if (iFlashCheck && std::chrono::duration_cast<std::chrono::duration<ng::scalar>>(std::chrono::steady_clock::now() - iFlashCheck->second).count() > 1)
                iFlashCheck = std::nullopt;
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
        if (iSquareIdentification == square_identification::Outer || iSquareIdentification == square_identification::Debug)
            result.deflate(dpi_scale(BORDER) * std::min(result.width() / 8.0 / 64.0_dip, 1.0));
        return result;
    }

    ng::rect board::square_rect(coordinates aCoordinates) const
    {
        if (iFlipped)
        {
            aCoordinates.x = 7 - aCoordinates.x;
            aCoordinates.y = 7 - aCoordinates.y;
        }
        auto const boardRect = board_rect();
        ng::size squareDimensions{ boardRect.extents() / 8.0 };
        return ng::rect{ boardRect.top_left() + ng::point{ squareDimensions * ng::size_u32{ aCoordinates.x, 7u - aCoordinates.y }.as<ng::scalar>() }, squareDimensions };
    }

    ng::scalar board::scale() const
    {
        return std::min(square_rect({ 0, 0 }).width() / 64.0_dip, 1.0);
    }

    ng::rect board::piece_rect(coordinates aCoordinates) const
    {
        if (iSquareIdentification == square_identification::Inner || iSquareIdentification == square_identification::InnerExtra)
            return square_rect(aCoordinates).deflated(8.0_dip * scale());
        else
            return square_rect(aCoordinates);
    }

    std::optional<coordinates> board::at(ng::point const& aPosition) const
    {
        for (coordinate y = 0u; y <= 7u; ++y)
            for (coordinate x = 0u; x <= 7u; ++x)
                if (square_rect(coordinates{ x, y }).contains(aPosition))
                    return coordinates{ x, y };
        return {};
    }
}