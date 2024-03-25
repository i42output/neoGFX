/*
  ui_element_library_plugin.hpp

  Copyright (c) 2019, 2020 Leigh Johnston.  All Rights Reserved.

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

#include <boost/dll.hpp>

#include <neolib/core/string.hpp>
#include <neolib/core/reference_counted.hpp>
#include <neolib/app/version.hpp>
#include <neolib/app/i_application.hpp>

#include <neogfx/app/i_resource_manager.hpp>
#include <neogfx/tools/nrc/i_ui_element_library.hpp>

namespace neogfx::nrc
{
    template <typename ElementLibrary>
    class ui_element_library_plugin : public neolib::reference_counted<neolib::i_plugin>
    {
    public:
        typedef ElementLibrary library_type;
    public:
        ui_element_library_plugin(
            neolib::i_application& aApplication,
            const neolib::uuid& aId = library_type::library_id(),
            std::string const& aName = library_type::library_name(),
            std::string const& aDescription = library_type::library_description(),
            const neolib::version& aVersion = library_type::library_version(),
            std::string const& aCopyright = library_type::library_copyright()) :
            iApplication{ aApplication },
            iId{ aId },
            iName{ aName },
            iDescription{ aDescription },
            iVersion{ aVersion },
            iCopyright{ aCopyright },
            iLoaded{ false }
        {
        }
    public:
        bool discover(const neolib::uuid& aId, void*& aObject) override
        {
            if (aId == i_ui_element_library::iid())
            {
                aObject = new library_type{ iApplication, "file:///" + boost::dll::this_line_location().string() };
                return true;
            }
            return false;
        }
    public:
        const neolib::uuid& id() const override
        {
            return iId;
        }
        const neolib::i_string& name() const override
        {
            return iName;
        }
        const neolib::i_string& description() const override
        {
            return iDescription;
        }
        const neolib::i_version& version() const override
        {
            return iVersion;
        }
        const neolib::i_string& copyright() const override
        {
            return iCopyright;
        }
        bool load() override
        {
            iLoaded = true;
            return true;
        }
        bool unload() override
        {
            service<i_resource_manager>().clean();
            iLoaded = false;
            return true;
        }
        bool loaded() const override
        {
            return iLoaded;
        }
        bool open_uri(const neolib::i_string& aUri) override
        {
            return false;
        }
    private:
        neolib::i_application& iApplication;
        neolib::uuid iId;
        neolib::string iName;
        neolib::string iDescription;
        neolib::version iVersion;
        neolib::string iCopyright;
        bool iLoaded;
    };
}