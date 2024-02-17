// i_text_document.hpp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2015, 2020 Leigh Johnston.  All Rights Reserved.
  
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

#include "i_document.hpp"

namespace neogfx
{
    enum class rich_text_format
    {
        Html
    };

    class i_text_document : public i_document
    {
    public:
        virtual i_string const& plain_text() const = 0;
        virtual bool set_plain_text(i_string const& aPlainText) = 0;
        virtual i_string const& rich_text(rich_text_format aFormat = rich_text_format::Html) const = 0;
        virtual bool set_rich_text(i_string const& aRichText, rich_text_format aFormat = rich_text_format::Html) = 0;
    public:
        virtual void paste_plain_text() = 0;
        virtual void paste_rich_text(rich_text_format aFormat = rich_text_format::Html) = 0;
    public:
        virtual void begin_update() = 0;
        virtual void end_update() = 0;
    };

    class scoped_document_update
    {
    public:
        scoped_document_update(i_text_document& aDocument)
            : iDocument{ aDocument }
        {
            iDocument.begin_update();
        }
        ~scoped_document_update()
        {
            iDocument.end_update();
        }
    private:
        i_text_document& iDocument;
    };
}