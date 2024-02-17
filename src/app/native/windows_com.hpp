// windows_com.hpp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2020 Leigh Johnston
  
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

#include <neogfx/neogfx.hpp>

#include <unknwn.h>

namespace neogfx
{
    namespace native::windows
    {
        template <typename Interface = ::IUnknown, bool DeallocateOnRelease = true>
        class com_component : public Interface
        {
        public:
            virtual ~com_component() = default;
        public:
            HRESULT QueryInterface(REFIID riid, void** ppvObject) override
            {
                if (riid == __uuidof(Interface))
                {
                    AddRef();
                    *ppvObject = static_cast<Interface*>(this);
                    return S_OK;
                }
                return E_NOINTERFACE;
            }
            ULONG AddRef() override
            {
                return ++iReferenceCount;
            }
            ULONG Release() override
            {
                if constexpr (DeallocateOnRelease)
                {
                    if (--iReferenceCount == 0)
                        delete this;
                }
                else
                    --iReferenceCount;
                return iReferenceCount;
            }
        private:
            ULONG iReferenceCount;
        };
    }
}