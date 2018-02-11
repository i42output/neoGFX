// i_text_document.hpp
/*
  neogfx C++ GUI Library
  Copyright(C) 2015-present Leigh Johnston
  
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
	class i_text_document : public i_document
	{
	public:
		virtual std::string plain_text() const = 0;
		virtual bool set_plain_text(const std::string& aPlainText) = 0;
		virtual std::string html() const = 0;
		virtual bool set_html(const std::string& aHtml) = 0;
	};
}