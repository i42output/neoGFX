// view.hpp
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

#pragma once

#include <neogfx/neogfx.hpp>
#include <neogfx/gui/widget/scrollable_widget.hpp>
#include "i_view.hpp"

namespace neogfx
{
	class i_controller;

	class view : public i_view, public scrollable_widget
	{
	public:
		view(i_controller& aController, i_model& aModel);
	public:
		virtual const i_widget& as_widget() const;
		virtual i_widget& as_widget();
		virtual const i_model& model() const;
		virtual i_model& model();
	public:
		virtual bool is_active() const;
		virtual void activate();
		virtual void deactivate();
	private:
		i_controller& iController;
		i_model& iModel;
		bool iActive;
	};
}