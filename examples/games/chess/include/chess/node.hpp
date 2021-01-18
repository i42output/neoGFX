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

#include <vector>
#include <chess/position.hpp>

namespace chess
{
    struct game_tree_node
    {
        std::optional<move> move;
        std::optional<double> eval;
        std::optional<std::vector<game_tree_node>> children;

        game_tree_node()
        {
        }
        game_tree_node(chess::move const& move) :
            move{ move }
        {
        }
        game_tree_node(game_tree_node&& other) :
            move{ other.move },
            eval{ other.eval },
            children{ std::move(other.children) }
        {
            if (children)
                other.children->clear();
        }
        game_tree_node(game_tree_node const&) = delete;

        game_tree_node& operator=(game_tree_node&& other)
        {
            move = other.move;
            eval = other.eval;
            children = std::move(other.children);
            if (children)
                other.children->clear();
            return *this;
        }
        game_tree_node& operator=(game_tree_node const&) = delete;
    };

    inline move const& as_move(game_tree_node const& aNode)
    {
        return *aNode.move;
    }

    inline std::vector<game_tree_node> const& as_valid_moves(game_tree_node const& aNode)
    {
        return *aNode.children;
    }

    inline std::vector<game_tree_node>& as_valid_moves(game_tree_node& aNode)
    {
        return *aNode.children;
    }
}
