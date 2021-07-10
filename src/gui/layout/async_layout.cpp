// async_layout.cpp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2021 Leigh Johnston.  All Rights Reserved.
  
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
#include <neogfx/gui/layout/async_layout.hpp>

template <> neogfx::i_async_layout& services::start_service<neogfx::i_async_layout>()
{
    static neogfx::async_layout sAsyncLayout;
    return sAsyncLayout;
}

namespace neogfx
{
    async_layout::async_layout() :
        iTimer{ service<i_async_task>(), [this](neolib::callback_timer& aTimer)
        {
            aTimer.again();
            process();
        }, std::chrono::milliseconds{ 20 } }
    {
    }

    bool async_layout::exists(i_widget& aWidget) const noexcept
    {
        return pending(aWidget) || processing(aWidget);
    }

    bool async_layout::defer_layout(i_widget& aWidget)
    {
        if (aWidget.has_root())
        {
            if (!exists(aWidget))
                iPending.emplace_back(aWidget, &aWidget);
            else
                invalidate(aWidget);
            return true;
        }
        return false;
    }

    void async_layout::validate(i_widget& aWidget)
    {
        if (auto existing = pending(aWidget))
            (**existing).validated = true;
        else if (existing = processing(aWidget))
            (**existing).validated = true;
    }

    void async_layout::invalidate(i_widget& aWidget)
    {
        if (auto existing = pending(aWidget))
            (**existing).validated = false;
        else if (existing = processing(aWidget))
            (**existing).validated = false;
    }

    std::optional<async_layout::entry_queue::const_iterator> async_layout::pending(i_widget& aWidget) const noexcept
    {
        auto existing = std::find_if(iPending.begin(), iPending.end(), [&](auto const& e) { return !e.destroyed && e.widget == &aWidget; });
        if (existing != iPending.end())
            return existing;
        else
            return std::nullopt;
    }

    std::optional<async_layout::entry_queue::iterator> async_layout::pending(i_widget& aWidget) noexcept
    {
        auto existing = std::find_if(iPending.begin(), iPending.end(), [&](auto const& e) { return !e.destroyed && e.widget == &aWidget; });
        if (existing != iPending.end())
            return existing;
        else
            return std::nullopt;
    }

    std::optional<async_layout::entry_queue::const_iterator> async_layout::processing(i_widget& aWidget) const noexcept
    {
        auto existing = std::find_if(iProcessing.begin(), iProcessing.end(), [&](auto const& e) { return !e.destroyed && e.widget == &aWidget; });
        if (existing != iProcessing.end())
            return existing;
        else
            return std::nullopt;
    }

    std::optional<async_layout::entry_queue::iterator> async_layout::processing(i_widget& aWidget) noexcept
    {
        auto existing = std::find_if(iProcessing.begin(), iProcessing.end(), [&](auto const& e) { return !e.destroyed && e.widget == &aWidget; });
        if (existing != iProcessing.end())
            return existing;
        else
            return std::nullopt;
    }

    void async_layout::process()
    {
        std::swap(iPending, iProcessing);

        for (auto& e : iProcessing)
        {
            if (e.validated || e.destroyed)
                continue;
            auto& next = *e.widget;
#ifdef NEOGFX_DEBUG
            if (debug::layoutItem == &next)
                service<debug::logger>() << typeid(next).name() << ": async_layout::process()" << endl;
#endif
            if (next.root().has_native_window())
            {
                next.layout_items();
                next.update();
            }
        }

        iProcessing.clear();
    }
}