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
        iAnimator{ ng::service<ng::i_async_task>(), [this](neolib::callback_timer&) { animate(); }, std::chrono::milliseconds{ 20 } },
        iSquareIdentification{ square_identification::None },
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

        set_focus();
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
            for (coordinate y = 0u; y <= 7u; ++y)
                for (coordinate x = 0u; x <= 7u; ++x)
                {
                    auto const squareRect = square_rect({ x, y });
                    auto const labelPadding = 2.0_dip * scale();
                    auto const labelRect = squareRect.deflated(ng::size{ labelPadding });
                    auto const pieceRect = piece_rect({ x, y });
                    auto squareColor = (x + y) % 2 == 0 ? ng::color::Gray25 : ng::color::Burlywood;
                    auto labelColor = (x + y) % 2 == 0 ? ng::color::Burlywood : ng::color::Gray25;
                    switch (pass)
                    {
                    case RENDER_BOARD:
                        {
                            bool const canMove = iSelection && iMoveValidator.can_move(iBoard.turn, iBoard, chess::move{ *iSelection, coordinates{x, y} });
                            auto const& labelFont = font();
                            auto yLabel = std::string{ static_cast<char>(y + '1') };
                            auto xLabel = std::string{ static_cast<char>(x + 'a') };
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
                                if (x == 0u)
                                    aGc.draw_text(ng::point{ squareRect.left() - (dpi_scale(BORDER) * scale() - yLabelExtents.cx) / 2.0 - yLabelExtents.cx, squareRect.center().y - yLabelExtents.cy / 2.0 }, yLabel, labelFont, palette_color(ng::color_role::Text));
                                else if (x == 7u)
                                    aGc.draw_text(ng::point{ squareRect.right() + (dpi_scale(BORDER) * scale() - yLabelExtents.cx) / 2.0, squareRect.center().y - yLabelExtents.cy / 2.0 }, yLabel, labelFont, palette_color(ng::color_role::Text));
                                if (y == 0u)
                                    aGc.draw_text(ng::point{ squareRect.center().x - xLabelExtents.cx / 2.0, squareRect.bottom() + (dpi_scale(BORDER) * scale() - xLabelExtents.cy) / 2.0 }, xLabel, labelFont, palette_color(ng::color_role::Text));
                                else if (y == 7u)
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
                                    if (x == 0)
                                        aGc.draw_text(labelRect.top_left(), yLabel, labelFont, labelColor);
                                    if (y == 0)
                                        aGc.draw_text(labelRect.bottom_right() - xLabelExtents, xLabel, labelFont, labelColor);
                                }
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
                            auto const occupier = iBoard.position[y][x];
                            if (occupier != piece::None)
                            {
                                auto pieceColor = piece_color(occupier) == piece::White ? ng::color::Goldenrod : ng::color::Silver;
                                bool useGradient = true;
                                if (iFlashCheck && (iMoveValidator.in_check(iBoard.turn, iBoard) || iFlashCheck->first) && piece_type(occupier) == piece::King && piece_color(occupier) == static_cast<piece>(iBoard.turn))
                                {
                                    useGradient = false;
                                    auto const since = std::chrono::steady_clock::now() - iFlashCheck->second;
                                    auto constexpr flashInterval_ms = 500;
                                    auto const normalizedFrameTime = ((std::chrono::duration_cast<std::chrono::milliseconds>(since).count() + flashInterval_ms / 2) % flashInterval_ms) / ((flashInterval_ms - 1) * 1.0);
                                    pieceColor = ng::mix(ng::color::Red, ng::color::White, ng::partitioned_ease(ng::easing::InvertedInOutQuint, ng::easing::InOutQuint, normalizedFrameTime));
                                }
                                ng::point const mousePosition = root().mouse_position() - origin();
                                auto adjust = (!selectedOccupier || !iSelectionPosition || (mousePosition - *iSelectionPosition).magnitude() < 8.0 ?
                                    ng::point{} : mousePosition - *iSelectionPosition);
                                aGc.draw_texture(pieceRect + adjust, iPieceTextures.at(piece_type(occupier)), useGradient ? ng::gradient{ pieceColor.lighter(0x80), pieceColor } : ng::color_or_gradient{ pieceColor }, ng::shader_effect::Colorize);
                            }
                        }
                        else if (animating_to(coordinates{ x, y }, now))
                        {
                            auto const& animation = animating_to(coordinates{ x, y }, now);
                            auto const occupier = animation->first->capturedPiece;
                            if (occupier != piece::None)
                            {
                                auto pieceColor = piece_color(occupier) == piece::White ? ng::color::Goldenrod : ng::color::Silver;
                                aGc.draw_texture(pieceRect, iPieceTextures.at(piece_type(occupier)), ng::gradient{ pieceColor.lighter(0x80), pieceColor }, ng::shader_effect::Colorize);
                            }
                        }
                        break;
                    case RENDER_ANIMATIONS:
                        if (auto const& animation = animating_to(coordinates{ x, y }, now))
                        {
                            auto const occupier = animation->first->movingPiece;
                            if (occupier != piece::None)
                            {
                                auto const pieceColor = piece_color(occupier) == piece::White ? ng::color::Goldenrod : ng::color::Silver;
                                aGc.draw_texture(ng::rect{ animation->second, pieceRect.extents() }, iPieceTextures.at(piece_type(occupier)), ng::gradient{ pieceColor.lighter(0x80), pieceColor }, ng::shader_effect::Colorize);
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
            iSquareIdentification = static_cast<square_identification>((static_cast<uint32_t>(iSquareIdentification) + 1u) % static_cast<uint32_t>(square_identification::COUNT));
            iAnimations.clear();
            update();
            return true;
        }
        else
            return widget<>::key_pressed(aScanCode, aKeyCode, aKeyModifiers);
    }

    ng::focus_policy board::focus_policy() const
    {
        return ng::focus_policy::StrongFocus;
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
            if (pos)
            {
                auto const pieceColor = piece_color(iBoard.position[pos->y][pos->x]);
                bool const correctColor = (pieceColor == static_cast<piece>(iBoard.turn));
                if (pos == iSelection)
                {
                    iSelectionPosition = std::nullopt;
                    iSelection = std::nullopt;
                    iLastSelectionEventTime = std::nullopt;
                }
                else if (!iSelection || (correctColor && !iEditBoard))
                {
                    if ((correctColor && iMoveValidator.has_moves(iBoard.turn, iBoard, *pos)) || (iEditBoard && pieceColor != piece::None))
                    {
                        iSelectionPosition = aPosition;
                        iSelection = pos;
                        iLastSelectionEventTime = std::chrono::steady_clock::now();
                    }
                    else if (correctColor && iMoveValidator.in_check(iBoard.turn, iBoard))
                    {
                        iFlashCheck = std::make_pair(false, std::chrono::steady_clock::now());
                    }
                    else if (correctColor && iMoveValidator.check_if_moved(iBoard.turn, iBoard, *pos))
                    {
                        iFlashCheck = std::make_pair(true, std::chrono::steady_clock::now());
                    }
                }
                else
                {
                    if (iMoveValidator.can_move(iBoard.turn, iBoard, chess::move{ *iSelection, *pos }) || iEditBoard)
                    {
                        chess::move move{ *iSelection, *pos };
                        iSelectionPosition = std::nullopt;
                        iSelection = std::nullopt;
                        iLastSelectionEventTime = std::nullopt;
                        play(move);
                    }
                    else if (iMoveValidator.in_check(iBoard.turn, iBoard))
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

            auto square = at(root().mouse_position() - origin());

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
                iBoard.position[square->y][square->x] = piece::None;
                current_player().setup(iBoard);
                next_player().setup(iBoard);
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
                if (pos && pos != *iSelection && (iMoveValidator.can_move(iBoard.turn, iBoard, chess::move{ *iSelection, *pos }) || iEditBoard))
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
        setup(chess::setup_position<matrix>());
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
        Changed.trigger();
    }

    void board::setup(chess::board const& aBoard)
    {
        iBoard = aBoard;
        Changed.trigger();
    }

    bool board::play(chess::move const& aMove)
    {
        if (!iEditBoard)
        {
            animate_move(aMove);

            auto const& source = iBoard.position[aMove.from.y][aMove.from.x];
            auto const movingPiece = source;
            std::optional<piece> promotion = aMove.promoteTo;
            if (piece_type(movingPiece) == piece::Pawn)
            {
                // promotion
                animating_to(aMove.to)->first->hold = true;
                if ((piece_color(movingPiece) == piece::White && aMove.to.y == 7u) || (piece_color(movingPiece) == piece::Black && aMove.to.y == 0u) && !promotion)
                {
                    promotion = piece_color(movingPiece) | piece::Queen;
                    ng::point const mousePosition = root().mouse_position() - origin();
                    ng::context_menu contextMenu{ *this, mousePosition + non_client_rect().top_left() + root().window_position() };
                    ng::action actionQueen{ "", iPieceTextures.at(piece::Queen) };
                    ng::action actionRook{ "", iPieceTextures.at(piece::Rook) };
                    ng::action actionBishop{ "", iPieceTextures.at(piece::Bishop) };
                    ng::action actionKnight{ "", iPieceTextures.at(piece::Knight) };
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
                        auto const pieceColor = piece_color(movingPiece) == piece::White ? ng::color::Goldenrod : ng::color::Silver;
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
            current_player().play(chess::move{ aMove.from, aMove.to, promotion });
        }
        else
            edit(aMove);
        Changed.trigger();
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
        iBoard.moveHistory.clear();
        current_player().setup(iBoard);
        next_player().setup(iBoard);
        display_eval();
        Changed.trigger();
        update();
    }

    bool board::can_undo() const
    {
        return !iBoard.moveHistory.empty();
    }

    void board::undo()
    {
        if (can_undo())
        {
            iUndoneMoves.push_back(*chess::undo(iBoard));
            current_player().undo();
            next_player().undo();
            Changed.trigger();
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
            if (piece_color(piece_at(iBoard, redoMove.to)) == piece::White)
                white_player().play(redoMove);
            else
                black_player().play(redoMove);
            Changed.trigger();
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
            Changed.trigger();
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
            Changed.trigger();
        }
    }

    i_player const& board::current_player() const
    {
        switch (iBoard.turn)
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
        switch (iBoard.turn)
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
        move_piece(iBoard, aMove);
        if (iMoveValidator.in_check(iBoard.turn, iBoard))
            iFlashCheck = std::make_pair(false, std::chrono::steady_clock::now());
        display_eval();
        if (!iInRedo)
            iUndoneMoves.clear();
        update();
    }

    void board::display_eval() const
    {
        eval_info evalInfo;
        double eval = iMoveValidator.eval(current_player().player(), iBoard, evalInfo);
        std::cerr << std::setprecision(4);
        if (iEditBoard)
            std::cerr << "[EDIT BOARD]" << std::endl;
        std::cerr << "material: " << std::round(evalInfo.material) << std::endl;
        std::cerr << "mobility: " << std::round(evalInfo.mobility) << std::endl;
        std::cerr << "attack: " << std::round(evalInfo.attack) << std::endl;
        std::cerr << "defend: " << std::round(evalInfo.defend) << std::endl;
        std::cerr << "mobilityPlayer: " << evalInfo.mobilityPlayer << std::endl;
        std::cerr << "mobilityPlayerKing: " << evalInfo.mobilityPlayerKing << std::endl;
        std::cerr << "checkedPlayerKing: " << std::round(evalInfo.checkedPlayerKing) << std::endl;
        std::cerr << "mobilityOpponent: " << evalInfo.mobilityOpponent << std::endl;
        std::cerr << "mobilityOpponentKing: " << evalInfo.mobilityOpponentKing << std::endl;
        std::cerr << "checkedOpponentKing: " << std::round(evalInfo.checkedOpponentKing) << std::endl;
        std::cerr << "eval: " << std::round(eval) << std::endl;
        std::cerr << "eval time: " << evalInfo.time_usec.count() << " us" << std::endl;
    }

    void board::animate_move(chess::move const& aMove)
    {
        auto const movingPiece = iBoard.position[aMove.from.y][aMove.from.x];
        auto const targetPiece = iBoard.position[aMove.to.y][aMove.to.x];
        if (piece_type(movingPiece) == piece::King && aMove.from.x - aMove.to.x == 2u)
        {
            // queenside castling
            iAnimations.emplace_back(aMove, movingPiece);
            iAnimations.emplace_back(chess::move{ coordinates{ 0u, aMove.from.y }, coordinates{ 3u, aMove.to.y } }, iBoard.position[aMove.from.y][0u]);
        }
        else if (piece_type(movingPiece) == piece::King && aMove.to.x - aMove.from.x == 2u)
        {
            // kingside castling
            iAnimations.emplace_back(aMove, movingPiece);
            iAnimations.emplace_back(chess::move{ coordinates{ 7u, aMove.from.y }, coordinates{ 5u, aMove.to.y } }, iBoard.position[aMove.from.y][7u]);
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
        if (iSquareIdentification == square_identification::Outer)
            result.deflate(dpi_scale(BORDER) * std::min(result.width() / 8.0 / 64.0_dip, 1.0));
        return result;
    }

    ng::rect board::square_rect(coordinates aCoordinates) const
    {
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