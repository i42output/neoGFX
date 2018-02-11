// context_menu.cpp
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

#include <neogfx/neogfx.hpp>
#include <neogfx/app/app.hpp>
#include <neogfx/gui/widget/menu.hpp>
#include <neogfx/gui/window/context_menu.hpp>

namespace neogfx
{
	std::unique_ptr<popup_menu> context_menu::sWidget;

	context_menu::context_menu(const point& aPosition, window_style aStyle)
		: iMenu{ new neogfx::menu{} }, iParent{ nullptr }, iPosition{ aPosition }, iStyle{ aStyle }
	{
	}

	context_menu::context_menu(i_widget& aParent, const point& aPosition, window_style aStyle)
		: iMenu{ new neogfx::menu{} }, iParent{ &aParent }, iPosition{ aPosition }, iStyle{ aStyle }
	{
	}

	context_menu::~context_menu()
	{
	}

	i_menu& context_menu::menu()
	{
		return *iMenu;
	}

	popup_menu& context_menu::root_widget()
	{
		if (sWidget == nullptr)
			throw widget_not_created_yet();
		return *sWidget;
	}

	void context_menu::exec()
	{
		bool finished = false;
		menu().set_modal(true);
		menu().closed([&finished]()
		{
			finished = true;
		});
		if (sWidget != nullptr)
			sWidget = nullptr;
		sWidget = (iParent != nullptr ?
			std::make_unique<popup_menu>(*iParent, iPosition, menu(), iStyle) :
			std::make_unique<popup_menu>(iPosition, menu(), iStyle));
		app::event_processing_context epc(app::instance(), "neogfx::context_menu");
		while (!finished)
		{
			app::instance().process_events(epc);
		}
		sWidget = nullptr;
	}
}