// gradient_dialog.hpp
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
#include <neogfx/gui/dialog/dialog.hpp>
#include <neogfx/gui/widget/gradient_widget.hpp>
#include <neogfx/gui/widget/group_box.hpp>
#include <neogfx/gui/widget/radio_button.hpp>

namespace neogfx
{
	class gradient_dialog : public dialog
	{
	public:
		gradient_dialog(i_widget& aParent, const neogfx::gradient& aCurrentGradient);
	public:
		const neogfx::gradient& gradient() const;
		void set_gradient(const neogfx::gradient& aGradient);
		const gradient_widget& gradient_selector() const;
		gradient_widget& gradient_selector();
	private:
		void init();
	private:
		vertical_layout iLayout;
		horizontal_layout iLayout2;
		vertical_layout iLayout3;
		vertical_layout iLayout4;
		group_box iSelectorGroupBox;
		gradient_widget iGradientSelector;
		group_box iDirectionGroupBox;
		radio_button iDirectionHorizontalRadioButton;
		radio_button iDirectionVerticalRadioButton;
		radio_button iDirectionDiagonalRadioButton;
		radio_button iDirectionRadialRadioButton;
		group_box iSizeGroupBox;
		radio_button iSizeClosestSideRadioButton;
		radio_button iSizeFarthestSideRadioButton;
		radio_button iSizeClosestCornerRadioButton;
		radio_button iSizeFarthestCornerRadioButton;
		vertical_spacer iSpacer1;
		group_box iPreviewGroupBox;
		framed_widget iPreview;
		vertical_spacer iSpacer2;
	};
}