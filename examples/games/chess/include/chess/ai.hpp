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

#include <memory>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <chrono>
#include <optional>
#include <string>

#include <neogfx/core/async_thread.hpp>
#include <neogfx/core/i_event.hpp>
#include <chess/i_player.hpp>

namespace stockparrot
{
    struct Engine;
}

namespace chess
{
    class ai : public i_player, public neogfx::async_thread
    {
    public:
        define_declared_event(Moved, moved, move)
    private:
        define_event(Decided, decided, move)
    private:
        struct engine_client;
    public:
        ai(chess::player aPlayer, std::chrono::milliseconds aMoveTime = std::chrono::milliseconds{ 3000 });
        ~ai();
    public:
        player_type type() const override;
        chess::player player() const override;
    public:
        void greet(i_player& aOpponent) override;
        void play() override;
        void stop() override;
        void finish() override;
        bool play(move const& aMove) override;
        bool playing() const override;
        void undo() override;
        void setup(mailbox_position const& aSetup) override;
    public:
        std::uint64_t nodes_per_second() const override;
    private:
        bool do_work(neolib::yield_type aYieldType = neolib::yield_type::NoYield) override;
    private:
        std::optional<move> execute();
    private:
        chess::player const iPlayer;
        std::chrono::milliseconds const iMoveTime;
        mutable std::recursive_mutex iMutex;
        mailbox_position iPosition;
        std::string iSetupFen;
        std::unique_ptr<engine_client> iEngineClient;
        std::unique_ptr<stockparrot::Engine> iEngine;
        std::mutex iSignalMutex;
        std::condition_variable iSignal;
        std::atomic<bool> iPlaying = false;
        std::atomic<bool> iFinished = false;
        std::atomic<std::uint64_t> iNodesPerSecond = 0;
        ng::sink iSink;
    };
}
