// button.cpp
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

#include <neogfx/neogfx.hpp>
#include <neogfx/gui/widget/button.hpp>
#include <neogfx/gui/widget/label.hpp>
#include <neogfx/app/app.hpp>

namespace neogfx
{
	button::button(const std::string& aText, alignment aAlignment) :
		widget(), iCheckable(NotCheckable), iCheckedState(false), iLayout(*this), iLabel(iLayout, aText, true, aAlignment)
	{
		init();
	}
	
	button::button(i_widget& aParent, const std::string& aText, alignment aAlignment) :
		widget(aParent), iCheckable(NotCheckable), iCheckedState(false), iLayout(*this), iLabel(iLayout, aText, true, aAlignment)
	{
		init();
	}


	button::button(i_layout& aLayout, const std::string& aText, alignment aAlignment) :
		widget(aLayout), iCheckable(NotCheckable), iCheckedState(false), iLayout(*this), iLabel(iLayout, aText, true, aAlignment)
	{
		init();
	}

	button::~button()
	{
		app::instance().remove_mnemonic(*this);
	}

	neogfx::size_policy button::size_policy() const
	{
		if (widget::has_size_policy())
			return widget::size_policy();
		return neogfx::size_policy{neogfx::size_policy::Expanding, neogfx::size_policy::Minimum};
	}

	margins button::margins() const
	{
		neogfx::margins result = widget::margins();
		if (!has_margins())
		{
			result.left *= 2.0;
			result.right *= 2.0;
		}
		return result;
	}

	button::checkable_e button::checkable() const
	{
		return iCheckable;
	}

	void button::set_checkable(checkable_e aCheckable)
	{
		iCheckable = aCheckable;
	}

	bool button::is_checked() const
	{
		return iCheckedState != boost::none && *iCheckedState == true;
	}

	bool button::is_unchecked() const
	{
		return iCheckedState != boost::none && *iCheckedState == false;
	}

	bool button::is_indeterminate() const
	{
		return iCheckedState == boost::none;
	}

	void button::check()
	{
		set_checked_state(true);
	}

	void button::uncheck()
	{
		set_checked_state(false);
	}

	void button::set_indeterminate()
	{
		set_checked_state(boost::none);
	}

	void button::set_checked(bool aChecked)
	{
		set_checked_state(aChecked);
	}

	void button::toggle()
	{
		if (is_checked() || is_indeterminate())
			set_checked(false);
		else
			set_checked(true);
	}

	const neogfx::label& button::label() const
	{
		return iLabel;
	}

	neogfx::label& button::label()
	{
		return iLabel;
	}

	const image_widget& button::image() const
	{
		return label().image();
	}

	image_widget& button::image()
	{
		return label().image();
	}

	const text_widget& button::text() const
	{
		return label().text();
	}

	text_widget& button::text()
	{
		return label().text();
	}

	void button::mouse_button_pressed(mouse_button aButton, const point& aPosition, key_modifiers_e aKeyModifiers)
	{
		widget::mouse_button_pressed(aButton, aPosition, aKeyModifiers);
		if (aButton == mouse_button::Left)
		{
			update();
			pressed.trigger();
		}
	}

	void button::mouse_button_double_clicked(mouse_button aButton, const point& aPosition, key_modifiers_e aKeyModifiers)
	{
		widget::mouse_button_double_clicked(aButton, aPosition, aKeyModifiers);
		if (aButton == mouse_button::Left)
		{
			update();
			double_clicked.trigger();
		}
	}

	void button::mouse_button_released(mouse_button aButton, const point& aPosition)
	{
		bool wasCapturing = capturing();
		widget::mouse_button_released(aButton, aPosition);
		if (aButton == mouse_button::Left)
		{
			update();
			if (wasCapturing)
			{
				destroyed_flag destroyed(*this);
				if (client_rect().contains(aPosition))
					handle_clicked();
				if (!destroyed)
					released.trigger();
			}
		}
	}

	bool button::key_pressed(scan_code_e aScanCode, key_code_e, key_modifiers_e)
	{
		if (aScanCode == ScanCode_SPACE)
		{
			handle_clicked();
			return true;
		}
		return false;
	}

	void button::handle_clicked()
	{
		destroyed_flag destroyed(*this);
		clicked.trigger();
		if (!destroyed && iCheckable != NotCheckable)
			toggle();
	}

	const boost::optional<bool>& button::checked_state() const
	{
		return iCheckedState;
	}

	bool button::set_checked_state(const boost::optional<bool>& aCheckedState)
	{
		if (iCheckedState == aCheckedState)
			return false;
		if (aCheckedState == boost::none && iCheckable != TriState)
			throw not_tri_state_checkable();
		iCheckedState = aCheckedState;
		update();
		if (is_checked())
			checked.trigger();
		else if (is_unchecked())
			unchecked.trigger();
		else if (is_indeterminate())
			indeterminate.trigger();
		return true;
	}

	std::string button::mnemonic() const
	{
		return mnemonic_from_text(label().text().text());
	}

	void button::mnemonic_execute()
	{
		handle_clicked();
	}

	i_widget& button::mnemonic_widget()
	{
		return label().text();
	}

	void button::init()
	{
		clicked.set_trigger_type(event_trigger_type::Asynchronous);
		double_clicked.set_trigger_type(event_trigger_type::Asynchronous);

		layout().set_margins(neogfx::margins(0.0));
		iLabel.set_size_policy(neogfx::size_policy::Expanding);

		set_focus_policy(focus_policy::TabFocus);

		auto label_text_updated = [this]()
		{
			auto m = mnemonic_from_text(label().text().text());
			if (!m.empty())
				app::instance().add_mnemonic(*this);
			else
				app::instance().remove_mnemonic(*this);
		};
		iSink = label().text().text_changed(label_text_updated);
		label_text_updated();
	}
}


