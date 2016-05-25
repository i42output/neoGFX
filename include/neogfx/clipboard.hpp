// clipboard.hpp
/*
  neogfx C++ GUI Library
  Copyright(C) 2016 Leigh Johnston
  
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

#include "neogfx.hpp"
#include "i_clipboard.hpp"
#include "i_native_clipboard.hpp"

namespace neogfx
{
	class clipboard : public i_clipboard
	{
	public:
		clipboard(i_native_clipboard& aNativeClipboard);
	public:
		virtual bool sink_active() const;
		virtual i_clipboard_sink& active_sink();
		virtual void activate(i_clipboard_sink& aSink);
		virtual void deactivate(i_clipboard_sink& aSink);
	public:
		virtual bool has_text() const;
		virtual std::string text() const;
		virtual void set_text(const std::string& aText);
	public:
		virtual void cut();
		virtual void copy();
		virtual void paste();
	private:
		i_native_clipboard& iNativeClipboard;
		i_clipboard_sink* iActiveSink;
	};
}