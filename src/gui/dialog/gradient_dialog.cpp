// gradient_dialog.cpp
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
#include <neogfx/gui/dialog/gradient_dialog.hpp>

namespace neogfx
{
	void draw_alpha_background(graphics_context& aGraphicsContext, const rect& aRect, dimension aAlphaPatternSize = 4.0);

	gradient_dialog::gradient_dialog(i_widget& aParent, const neogfx::gradient& aCurrentGradient) :
		dialog(aParent, "Select Gradient", Modal | Titlebar | Close),
		iLayout{ *this }, iLayout2{ iLayout }, iLayout3{ iLayout2 }, iLayout4{ iLayout2 },
		iSelectorGroupBox{ iLayout3 },
		iGradientSelector{ *this, iSelectorGroupBox.item_layout(), aCurrentGradient },
		iDirectionGroupBox{ iLayout3, "Direction" },
		iDirectionHorizontalRadioButton{ iDirectionGroupBox.item_layout(), "Horizontal" },
		iDirectionVerticalRadioButton{ iDirectionGroupBox.item_layout(), "Vertical" },
		iDirectionDiagonalRadioButton{ iDirectionGroupBox.item_layout(), "Diagonal" },
		iDirectionRadialRadioButton{ iDirectionGroupBox.item_layout(), "Radial" },
		iSizeGroupBox{ iLayout3, "Size" },
		iSizeClosestSideRadioButton{ iSizeGroupBox.item_layout(), "Closest side" },
		iSizeFarthestSideRadioButton{ iSizeGroupBox.item_layout(), "Farthest side" },
		iSizeClosestCornerRadioButton{ iSizeGroupBox.item_layout(), "Closest corner" },
		iSizeFarthestCornerRadioButton{ iSizeGroupBox.item_layout(), "Farthest corner" },
		iSpacer1{ iLayout3 },
		iPreviewGroupBox{ iLayout4, "Preview" },
		iPreview{ iPreviewGroupBox.item_layout() },
		iSpacer2{ iLayout4 }
	{
		init();
	}

	const gradient& gradient_dialog::gradient() const
	{
		return gradient_selector().gradient();
	}

	void gradient_dialog::set_gradient(const neogfx::gradient& aGradient)
	{
		gradient_selector().set_gradient(aGradient);
	}
	
	const gradient_widget& gradient_dialog::gradient_selector() const
	{
		return iGradientSelector;
	}

	gradient_widget& gradient_dialog::gradient_selector()
	{
		return iGradientSelector;
	}

	void gradient_dialog::init()
	{
		set_margins(neogfx::margins{ 16.0 });
		iLayout.set_margins(neogfx::margins{});
		iLayout.set_spacing(16.0);
		iLayout2.set_margins(neogfx::margins{});
		iLayout2.set_spacing(16.0);
		iLayout3.set_margins(neogfx::margins{});
		iLayout3.set_spacing(16.0);

		iGradientSelector.set_fixed_size(size{ 256.0, iGradientSelector.minimum_size().cy });

		auto update_widgets = [this]()
		{
			iSizeClosestSideRadioButton.set_checked(false); // todo
			iSizeFarthestSideRadioButton.set_checked(true);
			iSizeClosestCornerRadioButton.set_checked(false); // todo
			iSizeFarthestCornerRadioButton.set_checked(false); // todo
			iSizeClosestSideRadioButton.enable(false); // todo
			iSizeFarthestSideRadioButton.enable(true);
			iSizeClosestCornerRadioButton.enable(false); // todo
			iSizeFarthestCornerRadioButton.enable(false); // todo
			iDirectionDiagonalRadioButton.enable(false); // todo
			iDirectionHorizontalRadioButton.set_checked(gradient().direction() == gradient::Horizontal);
			iDirectionVerticalRadioButton.set_checked(gradient().direction() == gradient::Vertical);
			iDirectionDiagonalRadioButton.set_checked(gradient().direction() == gradient::Diagonal);
			iDirectionRadialRadioButton.set_checked(gradient().direction() == gradient::Radial);
			switch (gradient().direction())
			{
			case gradient::Vertical:
			case gradient::Horizontal:
			case gradient::Diagonal:
				iSizeGroupBox.hide();
				break;
			case gradient::Radial:
				iSizeGroupBox.show();
				break;
			}
			iPreview.update();
		};

		iGradientSelector.gradient_changed([this, update_widgets]() { update_widgets(); });

		iDirectionHorizontalRadioButton.checked([this, update_widgets]() { iGradientSelector.set_gradient(gradient().with_direction(gradient::Horizontal)); update_widgets(); });
		iDirectionVerticalRadioButton.checked([this, update_widgets]() { iGradientSelector.set_gradient(gradient().with_direction(gradient::Vertical)); update_widgets(); });
		iDirectionDiagonalRadioButton.checked([this, update_widgets]() { iGradientSelector.set_gradient(gradient().with_direction(gradient::Diagonal)); update_widgets(); });
		iDirectionRadialRadioButton.checked([this, update_widgets]() { iGradientSelector.set_gradient(gradient().with_direction(gradient::Radial)); update_widgets(); });

		iPreview.set_fixed_size(size{ 256.0, 256.0 });
		iPreview.painting([this](graphics_context& aGc)
		{
			rect r{ point{}, aGc.extents() };
			r.deflate(size{ 1.0 });
			draw_alpha_background(aGc, r);
			aGc.fill_rect(r, gradient());
		});

		button_box().add_button(dialog_button_box::Ok);
		button_box().add_button(dialog_button_box::Cancel);
		
		update_widgets();

		centre_on_parent();
	}
}