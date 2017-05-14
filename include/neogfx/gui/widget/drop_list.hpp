// drop_list.hpp
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
#include "push_button.hpp"

namespace neogfx
{
	class drop_list : public push_button
	{
	public:
		drop_list();
		drop_list(i_widget& aParent);
		drop_list(i_layout& aLayout);
		~drop_list();
	public:
		bool editable() const;
		void set_editable(bool aEditable);
	protected:
		size minimum_size(const optional_size& aAvailableSpace = optional_size()) const override;
	protected:
		void handle_clicked() override;
	private:
		void init();
		void update_arrow();
	private:
		sink iSink;
		bool iEditable;
		mutable boost::optional<std::pair<colour, texture>> iDownArrowTexture;
		image_widget iDownArrow;
	};
}