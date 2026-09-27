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

#include <chrono>
#include <optional>
#include <string>
#include <vector>

#include <chess/chess.hpp>
#include <chess/primitives.hpp>

namespace chess
{
    enum class player_type : std::uint32_t
    {
        Human,
        NetworkedHuman,
        AI
    };

    // latest completed search iteration; scores are from the searching player's perspective
    struct evaluation
    {
        std::int32_t depth;
        std::chrono::milliseconds time;
        std::int64_t nodes;
        std::int32_t score; // centipawns
        std::optional<std::int32_t> mateIn; // moves; negative if the searching player is being mated
        std::string bestMove; // UCI notation
        std::vector<std::string> continuation; // UCI notation; expected moves after bestMove, alternating sides
    };

    class i_player 
    {
    public:
        declare_event(moved, move)
    public:
        virtual ~i_player() = default;
    public:
        virtual player_type type() const = 0;
        virtual chess::player player() const = 0;
    public:
        virtual void greet(i_player& aOpponent) = 0;
        virtual void play() = 0;
        virtual void stop() = 0;
        virtual void finish() = 0;
        virtual bool play(move const& aMove) = 0;
        virtual bool playing() const = 0;
        virtual void undo() = 0;
        virtual void setup(mailbox_position const& aSetup) = 0;
    public:
        virtual std::uint64_t nodes_per_second() const = 0;
        virtual std::optional<chess::evaluation> current_evaluation() const = 0;
    };

    class i_player_factory
    {
    public:
        virtual ~i_player_factory() = default;
    public:
        virtual std::unique_ptr<i_player> create_player(player_type aType, chess::player aPlayer) = 0;
    };
}