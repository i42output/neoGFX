// gradient_dialog.cpp
/*
  neogfx C++ GUI Library
  Copyright (c) 2015 Leigh Johnston.  All Rights Reserved.
  
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

	class gradient_dialog::preview_box : public framed_widget
	{
	public:
		preview_box(gradient_dialog& aOwner) :
			framed_widget(aOwner.iPreviewGroupBox.item_layout()),
			iOwner(aOwner),
			iAnimationTimer{ app::instance(), [this](neolib::callback_timer& aTimer)
			{
				aTimer.again();
				animate();
			}, 10, true },
			iTracking{ false }
		{
			set_margins(neogfx::margins{});
		}
	public:
		virtual void paint(graphics_context& aGc) const
		{
			framed_widget::paint(aGc);
			rect cr = client_rect();
			draw_alpha_background(aGc, cr, dpi_scale(16.0));
			aGc.fill_rect(cr, iOwner.gradient());
			if (iOwner.gradient().direction() == gradient::Radial && iOwner.gradient().centre() != optional_point{})
			{
				point centre{ cr.centre().x + cr.width() / 2.0 * iOwner.gradient().centre()->x, cr.centre().y + cr.height() / 2.0 * iOwner.gradient().centre()->y };
				aGc.draw_circle(centre, dpi_scale(4.0), pen{ colour::White, 2.0 });
				aGc.line_stipple_on(3, 0xAAAA);
				aGc.draw_circle(centre, dpi_scale(4.0), pen{ colour::Black, 2.0 }, brush{}, to_rad(neolib::thread::program_elapsed_ms() / 10 % 100 * 3.6));
				aGc.line_stipple_off();
			}
		}
	public:
		virtual void mouse_button_pressed(mouse_button aButton, const point& aPosition, key_modifiers_e aKeyModifiers)
		{
			framed_widget::mouse_button_pressed(aButton, aPosition, aKeyModifiers);
			if (aButton == mouse_button::Left)
			{
				select(aPosition - client_rect(false).top_left());
				iTracking = true;
			}
		}
		virtual void mouse_button_released(mouse_button aButton, const point& aPosition)
		{
			framed_widget::mouse_button_released(aButton, aPosition);
			if (!capturing())
				iTracking = false;
		}
		virtual void mouse_moved(const point& aPosition)
		{
			if (iTracking)
				select(aPosition - client_rect(false).top_left());
		}
		void select(const point& aPosition)
		{
			auto cr = client_rect(false);
			point centre{ aPosition.x / cr.width() * 2.0 - 1.0, aPosition.y / cr.height() * 2.0 - 1.0 };
			centre = centre.max(point{ -1.0, -1.0 }).min(point{ 1.0, 1.0 });
			iOwner.gradient_selector().set_gradient(iOwner.gradient().with_centre(centre));
		}
	private:
		void animate()
		{
			if (iOwner.gradient().direction() == gradient::Radial && iOwner.gradient().centre() != optional_point{})
			{
				rect cr = client_rect();
				point centre{ cr.centre().x + cr.width() / 2.0 * iOwner.gradient().centre()->x, cr.centre().y + cr.height() / 2.0 * iOwner.gradient().centre()->y };
				update(rect{ centre - point{ 10, 10 }, size{ 20, 20 } });
			}
		}
	private:
		gradient_dialog& iOwner;
		neolib::callback_timer iAnimationTimer;
		bool iTracking;
	};

	gradient_dialog::gradient_dialog(i_widget& aParent, const neogfx::gradient& aCurrentGradient) :
		dialog(aParent, "Select Gradient"_t, window_style::Modal | window_style::TitleBar | window_style::Close),
		iLayout{ client_layout() }, iLayout2{ iLayout }, iLayout3{ iLayout2 }, iLayout4{ iLayout2 },
		iSelectorGroupBox{ iLayout3 },
		iGradientSelector{ *this, iSelectorGroupBox.item_layout(), aCurrentGradient },
		iSmoothnessGroupBox{ iSelectorGroupBox.item_layout(), "Smoothness (%)"_t },
		iSmoothnessSpinBox{ iSmoothnessGroupBox.item_layout() },
		iSmoothnessSlider{ iSmoothnessGroupBox.item_layout() },
		iDirectionGroupBox{ iLayout3, "Direction"_t },
		iDirectionHorizontalRadioButton{ iDirectionGroupBox.item_layout(), "Horizontal"_t },
		iDirectionVerticalRadioButton{ iDirectionGroupBox.item_layout(), "Vertical"_t },
		iDirectionDiagonalRadioButton{ iDirectionGroupBox.item_layout(), "Diagonal"_t },
		iDirectionRectangularRadioButton{ iDirectionGroupBox.item_layout(), "Rectangular"_t },
		iDirectionRadialRadioButton{ iDirectionGroupBox.item_layout(), "Radial"_t },
		iLayout5{ iLayout3 },
		iOrientationGroupBox{ iLayout5, "Orientation"_t },
		iStartingFromGroupBox{ iOrientationGroupBox.with_item_layout<horizontal_layout>(), "Starting From"_t },
		iTopLeftRadioButton{ iStartingFromGroupBox.item_layout(), "Top left corner"_t },
		iTopRightRadioButton{ iStartingFromGroupBox.item_layout(), "Top right corner"_t },
		iBottomRightRadioButton{ iStartingFromGroupBox.item_layout(), "Bottom right corner"_t },
		iBottomLeftRadioButton{ iStartingFromGroupBox.item_layout(), "Bottom left corner"_t },
		iAngleRadioButton{ iStartingFromGroupBox.item_layout(), "At a specific angle"_t },
		iLayout6 { iOrientationGroupBox.item_layout() },
		iAngleGroupBox{ iLayout6 },
		iAngle{ iAngleGroupBox.with_item_layout<grid_layout>(), "Angle:"_t },
		iAngleSpinBox{ iAngleGroupBox.item_layout() },
		iAngleSlider{ iAngleGroupBox.item_layout() },
		iSizeGroupBox{ iLayout5, "Size"_t },
		iSizeClosestSideRadioButton{ iSizeGroupBox.item_layout(), "Closest side"_t },
		iSizeFarthestSideRadioButton{ iSizeGroupBox.item_layout(), "Farthest side"_t },
		iSizeClosestCornerRadioButton{ iSizeGroupBox.item_layout(), "Closest corner"_t },
		iSizeFarthestCornerRadioButton{ iSizeGroupBox.item_layout(), "Farthest corner"_t },
		iShapeGroupBox{ iLayout5, "Shape"_t },
		iShapeEllipseRadioButton{ iShapeGroupBox.item_layout(), "Ellipse"_t },
		iShapeCircleRadioButton{ iShapeGroupBox.item_layout(), "Circle"_t },
		iExponentGroupBox{ iLayout5, "Exponents"_t },
		iLinkedExponents{ iExponentGroupBox.with_item_layout<grid_layout>(3, 2).add_span(grid_layout::cell_coordinates{0, 0}, grid_layout::cell_coordinates{1, 0}), "Linked"_t },
		iMExponent{ iExponentGroupBox.item_layout(), "m:"_t },
		iMExponentSpinBox{ iExponentGroupBox.item_layout() },
		iNExponent{ iExponentGroupBox.item_layout(), "n:"_t },
		iNExponentSpinBox{ iExponentGroupBox.item_layout() },
		iCentreGroupBox{ iLayout5, "Centre"_t },
		iXCentre{ iCentreGroupBox.with_item_layout<grid_layout>(2, 2), "X:"_t },
		iXCentreSpinBox { iCentreGroupBox.item_layout() },
		iYCentre{ iCentreGroupBox.item_layout(), "Y:"_t },
		iYCentreSpinBox { iCentreGroupBox.item_layout() },
		iSpacer2{ iLayout5 },
		iSpacer3{ iLayout3 },
		iPreviewGroupBox{ iLayout4, "Preview"_t },
		iPreview{ new preview_box{*this} },
		iSpacer4{ iLayout4 },
		iUpdatingWidgets(false)
	{
		init();
		layout::debug = &iExponentGroupBox.item_layout();
	}

	gradient_dialog::~gradient_dialog()
	{
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
		auto standardSpacing = set_standard_layout(16.0);

		iLayout.set_margins(neogfx::margins{});
		iLayout.set_spacing(standardSpacing);
		iLayout2.set_margins(neogfx::margins{});
		iLayout2.set_spacing(standardSpacing);
		iLayout3.set_margins(neogfx::margins{});
		iLayout3.set_spacing(standardSpacing);
		iLayout5.set_alignment(alignment::Top);
		iSmoothnessSpinBox.set_minimum(0.0);
		iSmoothnessSpinBox.set_maximum(100.0);
		iSmoothnessSpinBox.set_step(0.1);
		iSmoothnessSpinBox.set_format("%.1f");
		iSmoothnessSlider.set_minimum(0.0);
		iSmoothnessSlider.set_maximum(100.0);
		iSmoothnessSlider.set_step(0.1);
		iOrientationGroupBox.item_layout().set_alignment(alignment::Top);
		iAngleSpinBox.set_minimum(-360.0);
		iAngleSpinBox.set_maximum(360.0);
		iAngleSpinBox.set_step(0.1);
		iAngleSpinBox.set_format("%.1f");
		iAngleSlider.set_minimum(-360.0);
		iAngleSlider.set_maximum(360.0);
		iAngleSlider.set_step(0.1);
		iExponentGroupBox.set_checkable(true);
		iExponentGroupBox.item_layout().set_alignment(alignment::Right);
		iLinkedExponents.set_size_policy(size_policy::Expanding);
		iLinkedExponents.set_checked(true);
		iMExponentSpinBox.set_minimum(0.0);
		iMExponentSpinBox.set_maximum(std::numeric_limits<double>::max());
		iMExponentSpinBox.set_step(0.1);
		iMExponentSpinBox.set_format("%.2f");
		iMExponentSpinBox.text_box().set_alignment(alignment::Right);
		iMExponentSpinBox.text_box().set_hint("00.00");
		iNExponentSpinBox.set_minimum(0.0);
		iNExponentSpinBox.set_maximum(std::numeric_limits<double>::max());
		iNExponentSpinBox.set_step(0.1);
		iNExponentSpinBox.set_format("%.2f");
		iNExponentSpinBox.text_box().set_alignment(alignment::Right);
		iNExponentSpinBox.text_box().set_hint("00.00");
		iCentreGroupBox.set_checkable(true);
		iXCentreSpinBox.set_minimum(-1.0);
		iXCentreSpinBox.set_maximum(1.0);
		iXCentreSpinBox.set_step(0.001);
		iXCentreSpinBox.set_format("%.3f");
		iXCentreSpinBox.text_box().set_alignment(alignment::Right);
		iXCentreSpinBox.text_box().set_hint("-0.000");
		iYCentreSpinBox.set_minimum(-1.0);
		iYCentreSpinBox.set_maximum(1.0);
		iYCentreSpinBox.set_step(0.001);
		iYCentreSpinBox.set_format("%.3f");
		iYCentreSpinBox.text_box().set_alignment(alignment::Right);
		iYCentreSpinBox.text_box().set_hint("-0.000");

		iGradientSelector.set_fixed_size(size{ dpi_scale(256.0), iGradientSelector.minimum_size().cy });

		iGradientSelector.gradient_changed([this]() { update_widgets(); });

		iSmoothnessSpinBox.value_changed([this]() { iGradientSelector.set_gradient(gradient().with_smoothness(iSmoothnessSpinBox.value() / 100.0)); });
		iSmoothnessSlider.value_changed([this]() { iGradientSelector.set_gradient(gradient().with_smoothness(iSmoothnessSlider.value() / 100.0)); });

		iDirectionHorizontalRadioButton.checked([this]() { iGradientSelector.set_gradient(gradient().with_direction(gradient::Horizontal)); });
		iDirectionVerticalRadioButton.checked([this]() { iGradientSelector.set_gradient(gradient().with_direction(gradient::Vertical)); });
		iDirectionDiagonalRadioButton.checked([this]() { iGradientSelector.set_gradient(gradient().with_direction(gradient::Diagonal)); });
		iDirectionRectangularRadioButton.checked([this]() { iGradientSelector.set_gradient(gradient().with_direction(gradient::Rectangular)); });
		iDirectionRadialRadioButton.checked([this]() { iGradientSelector.set_gradient(gradient().with_direction(gradient::Radial)); });

		iTopLeftRadioButton.checked([this]() { iGradientSelector.set_gradient(gradient().with_orientation(gradient::TopLeft)); });
		iTopRightRadioButton.checked([this]() { iGradientSelector.set_gradient(gradient().with_orientation(gradient::TopRight)); });
		iBottomRightRadioButton.checked([this]() { iGradientSelector.set_gradient(gradient().with_orientation(gradient::BottomRight)); });
		iBottomLeftRadioButton.checked([this]() { iGradientSelector.set_gradient(gradient().with_orientation(gradient::BottomLeft)); });
		iAngleRadioButton.checked([this]() { iGradientSelector.set_gradient(gradient().with_orientation(0.0)); });

		iAngleSpinBox.value_changed([this]() { iGradientSelector.set_gradient(gradient().with_orientation(to_rad(iAngleSpinBox.value()))); });
		iAngleSlider.value_changed([this]() { iGradientSelector.set_gradient(gradient().with_orientation(to_rad(iAngleSlider.value()))); });

		iSizeClosestSideRadioButton.checked([this]() { iGradientSelector.set_gradient(gradient().with_size(gradient::ClosestSide)); });
		iSizeFarthestSideRadioButton.checked([this]() { iGradientSelector.set_gradient(gradient().with_size(gradient::FarthestSide)); });
		iSizeClosestCornerRadioButton.checked([this]() { iGradientSelector.set_gradient(gradient().with_size(gradient::ClosestCorner)); });
		iSizeFarthestCornerRadioButton.checked([this]() { iGradientSelector.set_gradient(gradient().with_size(gradient::FarthestCorner)); });

		iShapeEllipseRadioButton.checked([this]() { iGradientSelector.set_gradient(gradient().with_shape(gradient::Ellipse)); });
		iShapeCircleRadioButton.checked([this]() { iGradientSelector.set_gradient(gradient().with_shape(gradient::Circle)); });

		iExponentGroupBox.check_box().checked([this]() { iGradientSelector.set_gradient(gradient().with_exponents(vec2{1.0, 1.0})); });
		iExponentGroupBox.check_box().unchecked([this]() { iGradientSelector.set_gradient(gradient().with_exponents(optional_vec2{})); });

		iLinkedExponents.checked([this]() { auto e = gradient().exponents(); if (e != std::nullopt) { iGradientSelector.set_gradient(gradient().with_exponents(vec2{ e->x, e->x })); } });

		iMExponentSpinBox.value_changed([this]() 
		{ 
			if (iUpdatingWidgets)
				return;
			auto e = gradient().exponents(); 
			if (e == std::nullopt) 
				e = vec2{}; 
			~e->x = iMExponentSpinBox.value(); 
			if (iLinkedExponents.is_checked())
				~e->y = ~e->x;
			iGradientSelector.set_gradient(gradient().with_exponents(e)); 
		});

		iNExponentSpinBox.value_changed([this]()
		{
			if (iUpdatingWidgets)
				return;
			auto e = gradient().exponents();
			if (e == std::nullopt)
				e = vec2{};
			~e->y = iNExponentSpinBox.value();
			if (iLinkedExponents.is_checked())
				~e->x = ~e->y;
			iGradientSelector.set_gradient(gradient().with_exponents(e));
		});

		iCentreGroupBox.check_box().checked([this]() { iGradientSelector.set_gradient(gradient().with_centre(point{})); });
		iCentreGroupBox.check_box().unchecked([this]() { iGradientSelector.set_gradient(gradient().with_centre(optional_point{})); });

		iXCentreSpinBox.value_changed([this]() { auto c = gradient().centre(); if (c == std::nullopt) c = point{}; c->x = iXCentreSpinBox.value(); iGradientSelector.set_gradient(gradient().with_centre(c)); });
		iYCentreSpinBox.value_changed([this]() { auto c = gradient().centre(); if (c == std::nullopt) c = point{}; c->y = iYCentreSpinBox.value(); iGradientSelector.set_gradient(gradient().with_centre(c)); });

		iPreview->set_margins(neogfx::margins{});
		iPreview->set_fixed_size(dpi_scale(size{ std::ceil(256.0 * 16.0 / 9.0), 256.0 }));

		button_box().add_button(standard_button::Ok);
		button_box().add_button(standard_button::Cancel);
		
		update_widgets();

		layout().invalidate();
		centre_on_parent();
	}

	void gradient_dialog::update_widgets()
	{
		if (iUpdatingWidgets)
			return;
		neolib::scoped_flag sf{ iUpdatingWidgets };
		iSmoothnessSpinBox.set_value(gradient().smoothness() * 100.0);
		iSmoothnessSlider.set_value(gradient().smoothness() * 100.0);
		iDirectionHorizontalRadioButton.set_checked(gradient().direction() == gradient::Horizontal);
		iDirectionVerticalRadioButton.set_checked(gradient().direction() == gradient::Vertical);
		iDirectionDiagonalRadioButton.set_checked(gradient().direction() == gradient::Diagonal);
		iDirectionRectangularRadioButton.set_checked(gradient().direction() == gradient::Rectangular);
		iDirectionRadialRadioButton.set_checked(gradient().direction() == gradient::Radial);
		iTopLeftRadioButton.set_checked(gradient().orientation() == gradient::orientation_type(gradient::TopLeft));
		iTopRightRadioButton.set_checked(gradient().orientation() == gradient::orientation_type(gradient::TopRight));
		iBottomRightRadioButton.set_checked(gradient().orientation() == gradient::orientation_type(gradient::BottomRight));
		iBottomLeftRadioButton.set_checked(gradient().orientation() == gradient::orientation_type(gradient::BottomLeft));
		iAngleRadioButton.set_checked(std::holds_alternative<double>(gradient().orientation()));
		iAngleSpinBox.set_value(std::holds_alternative<double>(gradient().orientation()) ? to_deg(static_variant_cast<double>(gradient().orientation())) : 0.0);
		iAngleSlider.set_value(std::holds_alternative<double>(gradient().orientation()) ? to_deg(static_variant_cast<double>(gradient().orientation())) : 0.0);
		iSizeClosestSideRadioButton.set_checked(gradient().size() == gradient::ClosestSide);
		iSizeFarthestSideRadioButton.set_checked(gradient().size() == gradient::FarthestSide);
		iSizeClosestCornerRadioButton.set_checked(gradient().size() == gradient::ClosestCorner);
		iSizeFarthestCornerRadioButton.set_checked(gradient().size() == gradient::FarthestCorner);
		iShapeEllipseRadioButton.set_checked(gradient().shape() == gradient::Ellipse);
		iShapeCircleRadioButton.set_checked(gradient().shape() == gradient::Circle);
		auto exponents = gradient().exponents();
		bool specifyExponents = (exponents != std::nullopt);
		iExponentGroupBox.check_box().set_checked(specifyExponents);
		if (specifyExponents)
		{
			iMExponentSpinBox.set_value(exponents->x, false);
			iNExponentSpinBox.set_value(exponents->y, false);
		}
		else
		{
			iMExponentSpinBox.text_box().set_text("");
			iNExponentSpinBox.text_box().set_text("");
		}
		iLinkedExponents.enable(specifyExponents);
		iMExponent.enable(specifyExponents);
		iMExponentSpinBox.enable(specifyExponents);
		iNExponent.enable(specifyExponents);
		iNExponentSpinBox.enable(specifyExponents);		
		auto centre = gradient().centre();
		bool specifyCentre = (centre != std::nullopt);
		iCentreGroupBox.check_box().set_checked(specifyCentre);
		if (specifyCentre)
		{
			iXCentreSpinBox.set_value(centre->x, false);
			iYCentreSpinBox.set_value(centre->y, false);
		}
		else
		{
			iXCentreSpinBox.text_box().set_text("");
			iYCentreSpinBox.text_box().set_text("");
		}
		iXCentre.enable(specifyCentre);
		iXCentreSpinBox.enable(specifyCentre);
		iYCentre.enable(specifyCentre);
		iYCentreSpinBox.enable(specifyCentre);
		switch (gradient().direction())
		{
		case gradient::Vertical:
		case gradient::Horizontal:
		case gradient::Rectangular:
			iOrientationGroupBox.hide();
			iSizeGroupBox.hide();
			iShapeGroupBox.hide();
			iExponentGroupBox.hide();
			iCentreGroupBox.hide();
			break;
		case gradient::Diagonal:
			iOrientationGroupBox.show();
			iAngleGroupBox.show(std::holds_alternative<double>(gradient().orientation()));
			iSizeGroupBox.hide();
			iShapeGroupBox.hide();
			iExponentGroupBox.hide();
			iCentreGroupBox.hide();
			break;
		case gradient::Radial:
			iOrientationGroupBox.hide();
			iSizeGroupBox.show();
			iShapeGroupBox.show();
			iExponentGroupBox.show(gradient().shape() == gradient::Ellipse);
			iCentreGroupBox.show();
			break;
		}
		iPreview->update();
	}
}