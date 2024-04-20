// skin_manager.cpp
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

#include <neogfx/gui/widget/skin_manager.hpp>

template<> neogfx::i_skin_manager& services::start_service<neogfx::i_skin_manager>() { return neogfx::skin_manager::instance(); }

namespace neogfx
{
    skin_manager::skin_manager() :
        iActiveSkin{ nullptr }
    {
        register_skin(iDefaultSkin);
        activate_skin(iDefaultSkin);
    }

    skin_manager& skin_manager::instance()
    {
        static skin_manager sInstance;
        return sInstance;
    }

    std::uint32_t skin_manager::skin_count() const
    {
        return static_cast<std::uint32_t>(iSkins.size());
    }

    const i_skin& skin_manager::skin(std::uint32_t aIndex) const
    {
        if (aIndex < iSkins.size())
            return **std::next(iSkins.begin(), aIndex);
        throw skin_not_found();
    }

    i_skin& skin_manager::skin(std::uint32_t aIndex)
    {
        return const_cast<i_skin&>(to_const(*this).skin(aIndex));
    }

    void skin_manager::register_skin(i_skin& aSkin)
    {
        iSkins.insert(&aSkin);
        SkinRegistered.trigger(aSkin);
    }

    void skin_manager::unregister_skin(i_skin& aSkin)
    {
        auto existing = iSkins.find(&aSkin);
        if (existing != iSkins.end())
        {
            iSkins.erase(existing);
            if (&aSkin == iActiveSkin)
                iActiveSkin = nullptr;
            SkinUnregistered.trigger(aSkin);
            return;
        }
        throw skin_not_found();
    }

    bool skin_manager::skin_active() const
    {
        return iActiveSkin != nullptr;
    }

    const i_skin& skin_manager::active_skin() const
    {
        if (skin_active())
            return *iActiveSkin;
        throw no_skin_active();
    }

    i_skin& skin_manager::active_skin()
    {
        return const_cast<i_skin&>(to_const(*this).active_skin());
    }

    void skin_manager::activate_skin(i_skin& aSkin)
    {
        if (skin_active())
            active_skin().deactivate();
        iActiveSkin = &aSkin;
        active_skin().activate();
    }
}