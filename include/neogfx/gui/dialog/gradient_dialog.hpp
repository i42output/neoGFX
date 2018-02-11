// gradient_dialog.hpp
/*
  neogfx C++ GUI Library
  Copyright (c) 2015-present, Leigh Johnston.  All Rights Reserved.
  
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
#include <neogfx/gui/widget/spin_box.hpp>
#include <neogfx/gui/widget/slider.hpp>

namespace neogfx
{
	class gradient_dialog : public dialog
	{
		class preview_box;
	public:
		gradient_dialog(i_widget& aParent, const neogfx::gradient& aCurrentGradient);
		~gradient_dialog();
	public:
		const neogfx::gradient& gradient() const;
		void set_gradient(const neogfx::gradient& aGradient);
		const gradient_widget& gradient_selector() const;
		gradient_widget& gradient_selector();
	private:
		void init();
		void update_widgets();
	private:
		vertical_layout iLayout;
		horizontal_layout iLayout2;
		vertical_layout iLayout3;
		vertical_layout iLayout4;
		group_box iSelectorGroupBox;
		gradient_widget iGradientSelector;
		group_box iSmoothnessGroupBox;
		double_spin_box iSmoothnessSpinBox;
		double_slider iSmoothnessSlider;
		group_box iDirectionGroupBox;
		radio_button iDirectionHorizontalRadioButton;
		radio_button iDirectionVerticalRadioButton;
		radio_button iDirectionDiagonalRadioButton;
		radio_button iDirectionRadialRadioButton;
		horizontal_layout iLayout5;
		group_box iOrientationGroupBox;
		group_box iStartingFromGroupBox;
		radio_button iTopLeftRadioButton;
		radio_button iTopRightRadioButton;
		radio_button iBottomRightRadioButton;
		radio_button iBottomLeftRadioButton;
		radio_button iAngleRadioButton;
		vertical_layout iLayout6;
		group_box iAngleGroupBox;
		label iAngle;
		double_spin_box iAngleSpinBox;
		double_slider iAngleSlider;
		group_box iSizeGroupBox;
		radio_button iSizeClosestSideRadioButton;
		radio_button iSizeFarthestSideRadioButton;
		radio_button iSizeClosestCornerRadioButton;
		radio_button iSizeFarthestCornerRadioButton;
		group_box iShapeGroupBox;
		radio_button iShapeEllipseRadioButton;
		radio_button iShapeCircleRadioButton;
		group_box iCentreGroupBox;
		label iXCentre;
		double_spin_box iXCentreSpinBox;
		label iYCentre;
		double_spin_box iYCentreSpinBox;
		horizontal_spacer iSpacer2;
		vertical_spacer iSpacer3;
		group_box iPreviewGroupBox;
		std::shared_ptr<i_widget> iPreview;
		vertical_spacer iSpacer4;
		bool iUpdatingWidgets;
	};
}