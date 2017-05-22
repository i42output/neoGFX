// tab_button.hpp
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

#include <neogfx/neogfx.hpp>
#include "push_button.hpp"
#include "i_tab.hpp"
#include "i_tab_container.hpp"

namespace neogfx
{
	class tab_button : public push_button, public i_tab
	{
		friend class tab_bar;
		class close_button;
	public:
		tab_button(i_tab_container& aContainer, const std::string& aText = std::string(), bool aClosable = false);
		tab_button(i_widget& aParent, i_tab_container& aContainer, const std::string& aText = std::string(), bool aClosable = false);
		tab_button(i_layout& aLayout, i_tab_container& aContainer, const std::string& aText = std::string(), bool aClosable = false);
		~tab_button();
	public:
		const i_tab_container& container() const override;
		i_tab_container& container() override;
		bool closable() const override;
		void set_closable(bool aClosable) override;
		bool is_selected() const override;
		bool is_deselected() const override;
		void select() override;
	public:
		const std::string& text() const override;
		void set_text(const std::string& aText) override;
		void set_image(const i_texture& aTexture) override;
		void set_image(const i_image& aImage) override;
	public:
		const i_widget& as_widget() const override;
		i_widget& as_widget() override;
	protected:
		rect path_bounding_rect() const override;
		bool spot_colour() const override;
		colour border_mid_colour() const override;
	protected:
		size minimum_size(const optional_size& aAvailableSpace = optional_size()) const override;
		void handle_clicked() override;
	protected:
		colour foreground_colour() const override;
	protected:
		void paint(graphics_context& aGraphicsContext) const override;
	protected:
		void mouse_entered() override;
		void mouse_left() override;
	protected:
		void set_selected_state(bool aSelectedState);
	private:
		i_tab_container& iContainer;
		std::unique_ptr<close_button> iCloseButton;
		bool iSelectedState;
	};
}