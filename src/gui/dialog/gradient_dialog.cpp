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
		iLayout5{ iLayout3 },
		iSizeGroupBox{ iLayout5, "Size" },
		iSizeClosestSideRadioButton{ iSizeGroupBox.item_layout(), "Closest side" },
		iSizeFarthestSideRadioButton{ iSizeGroupBox.item_layout(), "Farthest side" },
		iSizeClosestCornerRadioButton{ iSizeGroupBox.item_layout(), "Closest corner" },
		iSizeFarthestCornerRadioButton{ iSizeGroupBox.item_layout(), "Farthest corner" },
		iShapeGroupBox{ iLayout5, "Shape" },
		iShapeEllipseRadioButton{ iShapeGroupBox.item_layout(), "Ellipse" },
		iShapeCircleRadioButton{ iShapeGroupBox.item_layout(), "Circle" },
		iCentreGroupBox{ iLayout5, "Centre" },
		iCentreLayout{ iCentreGroupBox.layout() },
		iXCentre { iCentreLayout, "X:" },
		iXCentreSpinBox { iCentreLayout },
		iYCentre { iCentreLayout, "Y:" },
		iYCentreSpinBox { iCentreLayout },
		iSpacer1{ iLayout3 },
		iPreviewGroupBox{ iLayout4, "Preview" },
		iPreview{ iPreviewGroupBox.item_layout() },
		iSpacer2{ iLayout4 },
		iAnimationTimer{ app::instance(), [this](neolib::callback_timer& aTimer)
		{
			aTimer.again();
			animate();
		}, 10, true}
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
		iLayout5.set_alignment(alignment::Top);

		iCentreGroupBox.set_checkable(true);
		iCentreGroupBox.set_item_layout(iCentreLayout);
		iXCentreSpinBox.set_minimum(-1.0);
		iXCentreSpinBox.set_maximum(1.0);
		iXCentreSpinBox.set_step(0.01);
		iYCentreSpinBox.set_minimum(-1.0);
		iYCentreSpinBox.set_maximum(1.0);
		iYCentreSpinBox.set_step(0.01);

		iGradientSelector.set_fixed_size(size{ 256.0, iGradientSelector.minimum_size().cy });

		auto update_widgets = [this]()
		{
			iDirectionDiagonalRadioButton.enable(false); // todo
			iDirectionHorizontalRadioButton.set_checked(gradient().direction() == gradient::Horizontal);
			iDirectionVerticalRadioButton.set_checked(gradient().direction() == gradient::Vertical);
			iDirectionDiagonalRadioButton.set_checked(gradient().direction() == gradient::Diagonal);
			iDirectionRadialRadioButton.set_checked(gradient().direction() == gradient::Radial);
			iSizeClosestSideRadioButton.set_checked(gradient().size() == gradient::ClosestSide);
			iSizeFarthestSideRadioButton.set_checked(gradient().size() == gradient::FarthestSide);
			iSizeClosestCornerRadioButton.set_checked(gradient().size() == gradient::ClosestCorner);
			iSizeFarthestCornerRadioButton.set_checked(gradient().size() == gradient::FarthestCorner);
			iShapeEllipseRadioButton.set_checked(gradient().shape() == gradient::Ellipse);
			iShapeCircleRadioButton.set_checked(gradient().shape() == gradient::Circle);
			iCentreGroupBox.check_box().set_checked(gradient().centre() != optional_point{});
			iXCentreSpinBox.set_value(gradient().centre() != optional_point{} ? gradient().centre()->x : 0.0);
			iYCentreSpinBox.set_value(gradient().centre() != optional_point{} ? gradient().centre()->y : 0.0);
			iXCentre.enable(gradient().centre() != optional_point{});
			iXCentreSpinBox.enable(gradient().centre() != optional_point{});
			iXCentreSpinBox.set_format(gradient().centre() != optional_point{} ? "%.2f" : "");
			iYCentre.enable(gradient().centre() != optional_point{});
			iYCentreSpinBox.enable(gradient().centre() != optional_point{});
			iYCentreSpinBox.set_format(gradient().centre() != optional_point{} ? "%.2f" : "");
			switch (gradient().direction())
			{
			case gradient::Vertical:
			case gradient::Horizontal:
			case gradient::Diagonal:
				iSizeGroupBox.hide();
				iShapeGroupBox.hide();
				iCentreGroupBox.hide();
				break;
			case gradient::Radial:
				iSizeGroupBox.show();
				iShapeGroupBox.show();
				iCentreGroupBox.show();
				break;
			}
			iPreview.update();
		};

		iGradientSelector.gradient_changed([this, update_widgets]() { update_widgets(); });

		iDirectionHorizontalRadioButton.checked([this, update_widgets]() { iGradientSelector.set_gradient(gradient().with_direction(gradient::Horizontal)); update_widgets(); });
		iDirectionVerticalRadioButton.checked([this, update_widgets]() { iGradientSelector.set_gradient(gradient().with_direction(gradient::Vertical)); update_widgets(); });
		iDirectionDiagonalRadioButton.checked([this, update_widgets]() { iGradientSelector.set_gradient(gradient().with_direction(gradient::Diagonal)); update_widgets(); });
		iDirectionRadialRadioButton.checked([this, update_widgets]() { iGradientSelector.set_gradient(gradient().with_direction(gradient::Radial)); update_widgets(); });

		iSizeClosestSideRadioButton.checked([this, update_widgets]() { iGradientSelector.set_gradient(gradient().with_size(gradient::ClosestSide)); update_widgets(); });
		iSizeFarthestSideRadioButton.checked([this, update_widgets]() { iGradientSelector.set_gradient(gradient().with_size(gradient::FarthestSide)); update_widgets(); });
		iSizeClosestCornerRadioButton.checked([this, update_widgets]() { iGradientSelector.set_gradient(gradient().with_size(gradient::ClosestCorner)); update_widgets(); });
		iSizeFarthestCornerRadioButton.checked([this, update_widgets]() { iGradientSelector.set_gradient(gradient().with_size(gradient::FarthestCorner)); update_widgets(); });

		iShapeEllipseRadioButton.checked([this, update_widgets]() { iGradientSelector.set_gradient(gradient().with_shape(gradient::Ellipse)); update_widgets(); });
		iShapeCircleRadioButton.checked([this, update_widgets]() { iGradientSelector.set_gradient(gradient().with_shape(gradient::Circle)); update_widgets(); });

		iCentreGroupBox.check_box().checked([this, update_widgets]() { iGradientSelector.set_gradient(gradient().with_centre(point{})); update_widgets(); });
		iCentreGroupBox.check_box().unchecked([this, update_widgets]() { iGradientSelector.set_gradient(gradient().with_centre(optional_point{})); update_widgets(); });

		iPreview.set_margins(neogfx::margins{});
		iPreview.set_fixed_size(size{ std::ceil(256.0 * 16.0 / 9.0), 256.0 });
		iPreview.painting([this](graphics_context& aGc)
		{
			rect cr = iPreview.client_rect(false);
			draw_alpha_background(aGc, cr);
			aGc.fill_rect(cr, gradient());
			if (gradient().centre() != optional_point{})
			{
				point centre{ cr.centre().x + cr.width() / 2.0 * gradient().centre()->x, cr.centre().y + cr.height() / 2.0 * gradient().centre()->y };
				aGc.draw_circle(centre, 4.0, pen{ colour::White, 2.0 });
				aGc.line_stipple_on(3, 0xAAAA);
				aGc.draw_circle(centre, 4.0, pen{ colour::Black, 2.0 }, fill{}, to_rad(neolib::thread::program_elapsed_ms() / 10 % 100 * 3.6));
				aGc.line_stipple_off();
			}
		});

		button_box().add_button(dialog_button_box::Ok);
		button_box().add_button(dialog_button_box::Cancel);
		
		update_widgets();

		centre_on_parent();
	}

	void gradient_dialog::animate()
	{
		if (gradient().centre() != optional_point{})
		{
			rect cr = iPreview.client_rect(false);
			point centre{ cr.centre().x + cr.width() / 2.0 * gradient().centre()->x, cr.centre().y + cr.height() / 2.0 * gradient().centre()->y };
			iPreview.update(rect{ centre - point{10, 10}, size{20, 20} });
		}
	}
}