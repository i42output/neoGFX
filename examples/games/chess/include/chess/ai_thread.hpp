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

#include <deque>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>

#include <chess/primitives.hpp>
#include <chess/i_player.hpp>

namespace chess
{
    struct game_state
    {
        std::atomic<bool> stopped = false;
        std::atomic<bool> finished = false;
    };

    template <typename Representation, player Player>
    class ai_thread
    {
    public:
        typedef Representation representation_type;
        typedef basic_position<representation_type> position_type;
        struct work_item
        {
            position_type position;
            move move;
            game_tree_node node;
            std::promise<game_tree_node> result;
        };
    public:
        ai_thread(i_player const& aPlayer, int32_t aPly);
        ~ai_thread();
    public:
        std::promise<game_tree_node>& eval(position_type const& aPosition, game_tree_node&& aNode);
        void start();
        void stop();
        void finish();
    private:
        void process();
    private:
        i_player const& iPlayer;
        int32_t iPly;
        move_tables<representation_type> const iMoveTables;
        std::deque<work_item> iQueue;
        std::mutex iMutex;
        std::condition_variable iSignal;
        std::thread iThread;
        std::atomic<game_state*> iGameState = nullptr;
    };
}