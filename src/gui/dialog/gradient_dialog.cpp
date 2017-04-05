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
		}
	public:
		virtual void paint(graphics_context& aGc) const
		{
			framed_widget::paint(aGc);
			rect cr = client_rect(false);
			draw_alpha_background(aGc, cr);
			aGc.fill_rect(cr, iOwner.gradient());
			if (iOwner.gradient().centre() != optional_point{})
			{
				point centre{ cr.centre().x + cr.width() / 2.0 * iOwner.gradient().centre()->x, cr.centre().y + cr.height() / 2.0 * iOwner.gradient().centre()->y };
				aGc.draw_circle(centre, 4.0, pen{ colour::White, 2.0 });
				aGc.line_stipple_on(3, 0xAAAA);
				aGc.draw_circle(centre, 4.0, pen{ colour::Black, 2.0 }, fill{}, to_rad(neolib::thread::program_elapsed_ms() / 10 % 100 * 3.6));
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
			centre.x = std::max(-1.0, std::min(1.0, centre.x));
			centre.y = std::max(-1.0, std::min(1.0, centre.y));
			iOwner.gradient_selector().set_gradient(iOwner.gradient().with_centre(centre));
		}
	private:
		void animate()
		{
			if (iOwner.gradient().centre() != optional_point{})
			{
				rect cr = client_rect(false);
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
		iPreview{ new preview_box{*this} },
		iSpacer2{ iLayout4 },
		iUpdatingWidgets(false)
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
		iXCentreSpinBox.set_step(0.001);
		iYCentreSpinBox.set_minimum(-1.0);
		iYCentreSpinBox.set_maximum(1.0);
		iYCentreSpinBox.set_step(0.001);

		iGradientSelector.set_fixed_size(size{ 256.0, iGradientSelector.minimum_size().cy });

		iGradientSelector.gradient_changed([this]() { update_widgets(); });

		iDirectionHorizontalRadioButton.checked([this]() { iGradientSelector.set_gradient(gradient().with_direction(gradient::Horizontal)); update_widgets(); });
		iDirectionVerticalRadioButton.checked([this]() { iGradientSelector.set_gradient(gradient().with_direction(gradient::Vertical)); update_widgets(); });
		iDirectionDiagonalRadioButton.checked([this]() { iGradientSelector.set_gradient(gradient().with_direction(gradient::Diagonal)); update_widgets(); });
		iDirectionRadialRadioButton.checked([this]() { iGradientSelector.set_gradient(gradient().with_direction(gradient::Radial)); update_widgets(); });

		iSizeClosestSideRadioButton.checked([this]() { iGradientSelector.set_gradient(gradient().with_size(gradient::ClosestSide)); update_widgets(); });
		iSizeFarthestSideRadioButton.checked([this]() { iGradientSelector.set_gradient(gradient().with_size(gradient::FarthestSide)); update_widgets(); });
		iSizeClosestCornerRadioButton.checked([this]() { iGradientSelector.set_gradient(gradient().with_size(gradient::ClosestCorner)); update_widgets(); });
		iSizeFarthestCornerRadioButton.checked([this]() { iGradientSelector.set_gradient(gradient().with_size(gradient::FarthestCorner)); update_widgets(); });

		iShapeEllipseRadioButton.checked([this]() { iGradientSelector.set_gradient(gradient().with_shape(gradient::Ellipse)); update_widgets(); });
		iShapeCircleRadioButton.checked([this]() { iGradientSelector.set_gradient(gradient().with_shape(gradient::Circle)); update_widgets(); });

		iCentreGroupBox.check_box().checked([this]() { if (gradient().centre() == boost::none) iGradientSelector.set_gradient(gradient().with_centre(point{})); update_widgets(); });
		iCentreGroupBox.check_box().unchecked([this]() { iGradientSelector.set_gradient(gradient().with_centre(optional_point{})); update_widgets(); });

		iXCentreSpinBox.value_changed([this]() { auto c = gradient().centre(); c->x = iXCentreSpinBox.value(); iGradientSelector.set_gradient(gradient().with_centre(c)); update_widgets(); });
		iYCentreSpinBox.value_changed([this]() { auto c = gradient().centre(); c->y = iYCentreSpinBox.value(); iGradientSelector.set_gradient(gradient().with_centre(c)); update_widgets(); });

		iPreview->set_margins(neogfx::margins{});
		iPreview->set_fixed_size(size{ std::ceil(256.0 * 16.0 / 9.0), 256.0 });

		button_box().add_button(dialog_button_box::Ok);
		button_box().add_button(dialog_button_box::Cancel);
		
		update_widgets();

		centre_on_parent();
	}

	void gradient_dialog::update_widgets()
	{
		if (iUpdatingWidgets)
			return;
		iUpdatingWidgets = true;
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
		iXCentreSpinBox.set_format(gradient().centre() != optional_point{} ? "%.3f" : "");
		iYCentre.enable(gradient().centre() != optional_point{});
		iYCentreSpinBox.enable(gradient().centre() != optional_point{});
		iYCentreSpinBox.set_format(gradient().centre() != optional_point{} ? "%.3f" : "");
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
		iPreview->update();
		iUpdatingWidgets = false;
	}
}