// gradient_dialog.hpp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2015, 2020 Leigh Johnston.  All Rights Reserved.
  
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

#include <string>
#include <vector>
#include <optional>

#include <neogfx/gui/dialog/dialog.hpp>
#include <neogfx/gui/widget/gradient_widget.hpp>
#include <neogfx/gui/widget/group_box.hpp>
#include <neogfx/gui/widget/radio_button.hpp>
#include <neogfx/gui/widget/spin_box.hpp>
#include <neogfx/gui/widget/slider.hpp>
#include <neogfx/gui/widget/drop_list.hpp>

namespace neogfx
{
    class gradient_dialog : public dialog
    {
        meta_object(dialog)
        class preview_box;
        class swatch_box;
    public:
        struct swatch
        {
            std::string name;
            std::vector<neogfx::gradient> gradients;
            bool predefined = false;
            std::string path; ///< *.gsw file (user swatches only)
        };
        static constexpr std::size_t MaxSwatchGradients = 16;
    public:
        gradient_dialog(i_widget& aParent, const neogfx::gradient& aCurrentGradient);
        ~gradient_dialog();
    public:
        neogfx::gradient gradient() const;
        void set_gradient(const i_gradient& aGradient);
        void set_gradient(const i_ref_ptr<i_gradient>& aGradient);
        const gradient_widget& gradient_selector() const;
        gradient_widget& gradient_selector();
    private:
        void init();
        void update_widgets();
        static std::string swatch_folder();
        static bool load_swatch(std::string const& aPath, swatch& aSwatch);
        static bool save_swatch(swatch const& aSwatch);
        void init_swatches();
        std::size_t add_swatch(swatch&& aSwatch);
        void update_swatch_selector();
        void update_swatch_buttons();
        void set_current_swatch(std::size_t aSwatch);
        bool current_swatch_editable() const;
        neogfx::gradient const* swatch_gradient(std::size_t aSlot) const;
        void set_current_swatch_gradient(std::optional<std::size_t> const& aSlot);
    private:
        vertical_layout iLayout;
        horizontal_layout iLayout2;
        vertical_layout iLayout3;
        vertical_layout iLayout4;
        group_box iSelectorGroupBox;
        gradient_widget iGradientSelector;
        horizontal_layout iLayout3_1;
        push_button iReverse;
        push_button iReversePartial;
        double_slider iHueSlider;
        push_button iImport;
        push_button iDelete;
        horizontal_layout iLayout3_2;
        group_box iDirectionGroupBox;
        radio_button iDirectionHorizontalRadioButton;
        radio_button iDirectionVerticalRadioButton;
        radio_button iDirectionDiagonalRadioButton;
        radio_button iDirectionRectangularRadioButton;
        radio_button iDirectionRadialRadioButton;
        group_box iTile;
        label iTileWidthLabel;
        uint32_spin_box iTileWidth;
        label iTileHeightLabel;
        uint32_spin_box iTileHeight;
        check_box iTileAligned;
        group_box iSmoothnessGroupBox;
        double_spin_box iSmoothnessSpinBox;
        double_slider iSmoothnessSlider;
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
        group_box iExponentGroupBox;
        check_box iLinkedExponents;
        label iMExponent;
        double_spin_box iMExponentSpinBox;
        label iNExponent;
        double_spin_box iNExponentSpinBox;
        group_box iCenterGroupBox;
        label iXCenter;
        double_spin_box iXCenterSpinBox;
        label iYCenter;
        double_spin_box iYCenterSpinBox;
        horizontal_spacer iSpacer2;
        vertical_spacer iSpacer3;
        group_box iPreviewGroupBox;
        std::shared_ptr<i_widget> iPreview;
        group_box iSwatchGroupBox;
        horizontal_layout iSwatchToolbar;
        drop_list iSwatchSelector;
        push_button iNewSwatch;
        push_button iImportSwatch;
        push_button iDeleteSwatch;
        grid_layout iSwatchGrid;
        horizontal_layout iSwatchEditLayout;
        push_button iAddToSwatch;
        push_button iRemoveFromSwatch;
        vertical_spacer iSpacer4;
        bool iUpdatingWidgets;
        bool iIgnoreHueSliderChange;
        std::vector<std::pair<std::size_t, double>> iHueSelection;
        std::vector<swatch> iSwatches;
        std::size_t iCurrentSwatch = 0;
        std::optional<std::size_t> iCurrentSwatchGradient;
    };
}