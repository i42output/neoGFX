// message_box.cpp
/*
  neogfx C++ GUI Library
  Copyright(C) 2017 Leigh Johnston
  
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
#include <neogfx/gui/dialog/message_box.hpp>

namespace neogfx
{
	message_box::message_box(const std::string& aTitle, const image& aIcon, const std::string& aText, standard_button aButtons) :
		dialog{ aTitle, window_style::TitleBar | window_style::Close },
		iLayout1{ client_layout() },
		iIcon{ iLayout1, aIcon },
		iLayout2{ iLayout1 },
		iText{ iLayout2, aText },
		iDetailedText{ iLayout2, std::string{} }
	{
		button_box().add_buttons(aButtons);
		init();
	}

	message_box::message_box(i_widget& aParent, const std::string& aTitle, const image& aIcon, const std::string& aText, standard_button aButtons) :
		dialog{ aParent, aTitle, window_style::TitleBar | window_style::Close },
		iLayout1{ client_layout() },
		iIcon{ iLayout1, aIcon },
		iLayout2{ iLayout1 },
		iText{ iLayout2, aText },
		iDetailedText{ iLayout2, std::string{} }
	{
		button_box().add_buttons(aButtons);
		init();
	}

	standard_button message_box::information(const std::string& aTitle, const std::string& aText, standard_button aButtons)
	{
		message_box mb{ aTitle, image{ ":/neogfx/resources/icons.naa#warning.png" }, aText, aButtons };
		mb.exec();
		return mb.response();
	}

	standard_button message_box::information(i_widget& aParent, const std::string& aTitle, const std::string& aText, standard_button aButtons)
	{
		message_box mb{ aParent, aTitle, image{ ":/neogfx/resources/icons.naa#warning.png" }, aText, aButtons };
		mb.exec();
		return mb.response();
	}

	standard_button message_box::question(const std::string& aTitle, const std::string& aText, standard_button aButtons)
	{
		message_box mb{ aTitle, image{ ":/neogfx/resources/icons.naa#question.png" }, aText, aButtons };
		mb.exec();
		return mb.response();
	}

	standard_button message_box::question(i_widget& aParent, const std::string& aTitle, const std::string& aText, standard_button aButtons)
	{
		message_box mb{ aParent, aTitle, image{ ":/neogfx/resources/icons.naa#question.png" }, aText, aButtons };
		mb.exec();
		return mb.response();
	}

	standard_button message_box::warning(const std::string& aTitle, const std::string& aText, standard_button aButtons)
	{
		message_box mb{ aTitle, image{ ":/neogfx/resources/icons.naa#warning.png" }, aText, aButtons };
		mb.exec();
		return mb.response();
	}

	standard_button message_box::warning(i_widget& aParent, const std::string& aTitle, const std::string& aText, standard_button aButtons)
	{
		message_box mb{ aParent, aTitle, image{ ":/neogfx/resources/icons.naa#warning.png" }, aText, aButtons };
		mb.exec();
		return mb.response();
	}

	standard_button message_box::stop(const std::string& aTitle, const std::string& aText, standard_button aButtons)
	{
		message_box mb{ aTitle, image{ ":/neogfx/resources/icons.naa#stop.png" }, aText, aButtons };
		mb.exec();
		return mb.response();
	}

	standard_button message_box::stop(i_widget& aParent, const std::string& aTitle, const std::string& aText, standard_button aButtons)
	{
		message_box mb{ aParent, aTitle, image{ ":/neogfx/resources/icons.naa#stop.png" }, aText, aButtons };
		mb.exec();
		return mb.response();
	}

	standard_button message_box::error(const std::string& aTitle, const std::string& aText, standard_button aButtons)
	{
		message_box mb{ aTitle, image{ ":/neogfx/resources/icons.naa#error.png" }, aText, aButtons };
		mb.exec();
		return mb.response();
	}

	standard_button message_box::error(i_widget& aParent, const std::string& aTitle, const std::string& aText, standard_button aButtons)
	{
		message_box mb{ aParent, aTitle, image{ ":/neogfx/resources/icons.naa#error.png" }, aText, aButtons };
		mb.exec();
		return mb.response();
	}

	standard_button message_box::critical(const std::string& aTitle, const std::string& aText, standard_button aButtons)
	{
		message_box mb{ aTitle, image{ ":/neogfx/resources/icons.naa#critical.png" }, aText, aButtons };
		mb.exec();
		return mb.response();
	}

	standard_button message_box::critical(i_widget& aParent, const std::string& aTitle, const std::string& aText, standard_button aButtons)
	{
		message_box mb{ aParent, aTitle, image{ ":/neogfx/resources/icons.naa#critical.png" }, aText, aButtons };
		mb.exec();
		return mb.response();
	}

	const image_widget& message_box::icon() const
	{
		return iIcon;
	}

	image_widget& message_box::icon()
	{
		return iIcon;
	}

	const text_widget& message_box::text() const
	{
		return iText;
	}

	text_widget& message_box::text()
	{
		return iText;
	}

	const text_widget& message_box::detailed_text() const
	{
		return iDetailedText;
	}

	text_widget& message_box::detailed_text()
	{
		return iDetailedText;
	}

	bool message_box::has_response() const
	{
		return iResponse != boost::none;
	}

	standard_button message_box::response() const
	{
		if (has_response())
			return *iResponse;
		throw no_response();
	}

	void message_box::set_response(standard_button aResponse)
	{
		iResponse = aResponse;
	}

	dialog_result message_box::exec()
	{
		auto result = dialog::exec();
		if (!has_response())
			set_response(button_box().button_with_role(button_role::RejectRole));
		return result;
	}

	void message_box::init()
	{
		set_standard_layout(16.0);
		iLayout1.set_margins(neogfx::margins{});
		iLayout1.set_spacing(16.0);
		iLayout2.set_margins(neogfx::margins{});
		iLayout2.set_spacing(16.0);

		centre_on_parent();

		button_box().clicked([this](standard_button aButton)
		{
			set_response(aButton);
			set_result(dialog_result::Other);
		});
	}
}