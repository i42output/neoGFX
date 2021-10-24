// scrollable_widget.cpp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2020 Leigh Johnston.  All Rights Reserved.
  
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

#include <neogfx/neogfx.hpp>
#include <neogfx/gui/widget/scrollable_widget.ipp>
#include <neogfx/gui/widget/framed_widget.hpp>
#include <neogfx/gui/view/view_container.hpp>
#include <neogfx/gui/window/i_window.hpp>

namespace neogfx
{
    class scrollbar_container_updater : public i_scrollbar_container_updater
    {
    public:
        scrollbar_container_updater() :
            iProcessing{ false }
        {
        }
    public:
        void queue(i_scrollbar_container& aContainer) final
        {
            iQueue.push_back(&aContainer);
        }
        bool processing() const final
        {
            return iProcessing;
        }
        void process() final
        {
            neolib::scoped_flag sf{ iProcessing };
            std::reverse(iQueue.begin(), iQueue.end());
            for (auto sc1 = iQueue.begin(); sc1 != iQueue.end(); ++sc1)
                for (auto sc2 = std::next(sc1); sc2 != iQueue.end(); ++sc2)
                    if (*sc1 == *sc2)
                        *sc2 = nullptr;
            while (!iQueue.empty())
            {
                if (iQueue.back() != nullptr)
                    iQueue.back()->update_scrollbar_visibility();
                iQueue.pop_back();
            }
        }
        i_scrollbar_container& current() const final
        {
            if (!iQueue.empty() && processing())
                return *iQueue.back();
            throw std::logic_error{ "neogfx::scrollbar_container_updater: not processing" };
        }
    private:
        std::vector<i_scrollbar_container*> iQueue;
        bool iProcessing;
    };

    template class scrollable_widget<>;
    template class scrollable_widget<framed_widget<widget<>>>;
    template class scrollable_widget<framed_widget<widget<i_window>>>;
}

template<> neogfx::i_scrollbar_container_updater& services::start_service<neogfx::i_scrollbar_container_updater>()
{
    thread_local neogfx::scrollbar_container_updater tUpdater;
    return tUpdater;
}

