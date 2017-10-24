// nested_window.cpp
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

#include <neogfx/neogfx.hpp>
#include <neogfx/app/app.hpp>
#include <neogfx/gui/widget/i_nested_window_container.hpp>
#include <neogfx/gui/widget/nested_window.hpp>

namespace neogfx
{
	nested_window::nested_window(i_widget& aParentWidget, i_nested_window_container& aNestedRoot, const size& aDimensions, const std::string& aWindowTitle, window_style aStyle, scrollbar_style aScrollbarStyle, frame_style aFrameStyle) :
		scrollable_widget{ aParentWidget, aScrollbarStyle, aFrameStyle },
		iRoot{ aNestedRoot },
		iParent{ nullptr },
		iTitleText{ aWindowTitle },
		iStyle{ aStyle },
		iPlacement{ window_placement::Restored, rect{}, rect { point{}, aDimensions } },
		iActive{ false },
		iCountedEnable{ 0 },
		iNonClientLayout{ *this },
		iTitleBarLayout{ iNonClientLayout },
		iMenuLayout{ iNonClientLayout },
		iToolbarLayout{ iNonClientLayout },
		iClientLayout{ iNonClientLayout },
		iStatusBarLayout{ iNonClientLayout }
	{
		iRoot.add(*this);
	}

	nested_window::nested_window(i_widget& aParentWidget, i_nested_window& aParent, const size& aDimensions, const std::string& aWindowTitle, window_style aStyle, scrollbar_style aScrollbarStyle, frame_style aFrameStyle) :
		scrollable_widget{ aParentWidget, aScrollbarStyle, aFrameStyle },
		iRoot{ aParent.nested_root() },
		iParent{ &aParent },
		iTitleText{ aWindowTitle },
		iStyle{ aStyle },
		iPlacement{ window_placement::Restored, rect{}, rect{ point{}, aDimensions } },
		iActive{ false },
		iCountedEnable{ 0 },
		iNonClientLayout{ *this },
		iTitleBarLayout{ iNonClientLayout },
		iMenuLayout{ iNonClientLayout },
		iToolbarLayout{ iNonClientLayout },
		iClientLayout{ iNonClientLayout },
		iStatusBarLayout{ iNonClientLayout }
	{
		iRoot.add(*this);
	}

	nested_window::nested_window(i_nested_window_container& aNestedRoot, const size& aDimensions, window_style aStyle, scrollbar_style aScrollbarStyle, frame_style aFrameStyle) :
		nested_window{ aNestedRoot.as_widget(), aNestedRoot, aDimensions, std::string{}, aStyle, aScrollbarStyle, aFrameStyle }
	{
		init();
	}

	nested_window::nested_window(i_nested_window_container& aNestedRoot, const size& aDimensions, const std::string& aWindowTitle, window_style aStyle, scrollbar_style aScrollbarStyle, frame_style aFrameStyle) :
		nested_window{ aNestedRoot.as_widget(), aNestedRoot, aDimensions, aWindowTitle, aStyle, aScrollbarStyle, aFrameStyle }
	{
		init();
	}

	nested_window::nested_window(i_nested_window_container& aNestedRoot, const point& aPosition, const size& aDimensions, window_style aStyle, scrollbar_style aScrollbarStyle, frame_style aFrameStyle) :
		nested_window{ aNestedRoot.as_widget(), aNestedRoot, aDimensions, std::string{}, aStyle, aScrollbarStyle, aFrameStyle }
	{
		init(aPosition);
	}

	nested_window::nested_window(i_nested_window_container& aNestedRoot, const point& aPosition, const size& aDimensions, const std::string& aWindowTitle, window_style aStyle, scrollbar_style aScrollbarStyle, frame_style aFrameStyle) :
		nested_window{ aNestedRoot.as_widget(), aNestedRoot, aDimensions, aWindowTitle, aStyle, aScrollbarStyle, aFrameStyle }
	{
		init(aPosition);
	}

	nested_window::nested_window(i_nested_window& aParent, const size& aDimensions, window_style aStyle, scrollbar_style aScrollbarStyle, frame_style aFrameStyle) :
		nested_window{ aParent.nested_root().as_widget(), aParent, aDimensions, std::string{}, aStyle, aScrollbarStyle, aFrameStyle }
	{
		init();
	}

	nested_window::nested_window(i_nested_window& aParent, const size& aDimensions, const std::string& aWindowTitle, window_style aStyle, scrollbar_style aScrollbarStyle, frame_style aFrameStyle) :
		nested_window{ aParent.nested_root().as_widget(), aParent, aDimensions, aWindowTitle, aStyle, aScrollbarStyle, aFrameStyle }
	{
		init();
	}

	nested_window::nested_window(i_nested_window& aParent, const point& aPosition, const size& aDimensions, window_style aStyle, scrollbar_style aScrollbarStyle, frame_style aFrameStyle) :
		nested_window{ aParent.nested_root().as_widget(), aParent, aDimensions, std::string{}, aStyle, aScrollbarStyle, aFrameStyle }
	{
		init(aPosition);
	}

	nested_window::nested_window(i_nested_window& aParent, const point& aPosition, const size& aDimensions, const std::string& aWindowTitle, window_style aStyle, scrollbar_style aScrollbarStyle, frame_style aFrameStyle) :
		nested_window{ aParent.nested_root().as_widget(), aParent, aDimensions, aWindowTitle, aStyle, aScrollbarStyle, aFrameStyle }
	{
		init(aPosition);
	}

	nested_window::~nested_window()
	{
		iRoot.remove(*this);
	}

	window_style nested_window::style() const
	{
		return iStyle;
	}

	void nested_window::set_style(window_style aStyle)
	{
		if (iStyle != aStyle)
		{
			iStyle = aStyle;
			update(true);
		}
	}

	colour nested_window::frame_colour() const
	{
		if (!scrollable_widget::has_frame_colour() && is_active())
			return app::instance().current_style().palette().selection_colour();
		else
			return scrollable_widget::frame_colour();
	}

	bool nested_window::can_defer_layout() const
	{
		return true;
	}

	bool nested_window::is_managing_layout() const
	{
		return true;
	}

	widget_part nested_window::hit_test(const point& aPosition) const
	{
		auto result = scrollable_widget::hit_test(aPosition);
		if (result == widget_part::Client)
			result = widget_part::NonClientGrab;
		return result;
	}

	neogfx::size_policy nested_window::size_policy() const
	{
		if (widget::has_size_policy())
			return widget::size_policy();
		return neogfx::size_policy::Manual;
	}

	colour nested_window::background_colour() const
	{
		if (has_background_colour())
			return scrollable_widget::background_colour();
		else
			return container_background_colour();
	}

	scrolling_disposition nested_window::scrolling_disposition(const i_widget& aChildWidget) const
	{
		if (iTitleBar != boost::none && &aChildWidget == &*iTitleBar)
			return neogfx::scrolling_disposition::DontScrollChildWidget;
		return scrollable_widget::scrolling_disposition(aChildWidget);
	}

	const i_nested_window_container& nested_window::nested_root() const
	{
		return iRoot;
	}

	i_nested_window_container& nested_window::nested_root()
	{
		return iRoot;
	}

	bool nested_window::has_nested_parent() const
	{
		return iParent != nullptr;
	}

	const i_nested_window& nested_window::nested_parent() const
	{
		if (has_nested_parent())
			return *iParent;
		throw no_nested_parent();
	}

	i_nested_window& nested_window::nested_parent()
	{
		if (has_nested_parent())
			return *iParent;
		throw no_nested_parent();
	}

	const std::string& nested_window::title_text() const
	{
		return iTitleText;
	}

	void nested_window::set_title_text(const std::string& aTitleText)
	{
		if (iTitleText != aTitleText)
		{
			iTitleText = aTitleText;
			if (iTitleBar != boost::none)
				iTitleBar->title().set_text(iTitleText);
		}
	}

	bool nested_window::is_active() const
	{
		return iActive;
	}

	void nested_window::activate()
	{
		if (!iActive)
		{
			iActive = true;
			// todo
		}
	}

	bool nested_window::is_iconic() const
	{
		return iPlacement.state == window_placement::Iconized;
	}

	void nested_window::iconize()
	{
		if (iPlacement.state != window_placement::Iconized)
		{
			iPlacement.state = window_placement::Iconized;
			// todo
		}
	}

	bool nested_window::is_maximized() const
	{
		return iPlacement.state == window_placement::Maximized;
	}

	void nested_window::maximize()
	{
		if (iPlacement.state != window_placement::Maximized)
		{
			iPlacement.state = window_placement::Maximized;
			// todo
		}
	}

	bool nested_window::is_restored() const
	{
		return iPlacement.state == window_placement::Restored;
	}

	void nested_window::restore()
	{
		if (iPlacement.state != window_placement::Restored)
		{
			iPlacement.state = window_placement::Restored;
			// todo
		}
	}

	window_placement nested_window::placement() const
	{
		return iPlacement;
	}

	void nested_window::set_placement(const window_placement& aPlacement)
	{
		iPlacement = aPlacement;
		// todo
	}

	void nested_window::centre()
	{
		resize(minimum_size());
		rect rootRect = to_client_coordinates(
			nested_root().as_widget().to_window_coordinates(
				rect{ nested_root().as_widget().position(), nested_root().as_widget().extents() }));
		move((rootRect.extents() - extents()) / 2.0);
	}

	void nested_window::centre_on_parent()
	{
		if (has_nested_parent())
		{
			resize(minimum_size());
			rect rootRect = to_client_coordinates(
				nested_root().as_widget().to_window_coordinates(
					rect{ nested_root().as_widget().position(), nested_root().as_widget().extents() }));
			rect parentRect = to_client_coordinates(
				nested_parent().as_widget().to_window_coordinates(
					rect{ nested_parent().as_widget().position(), nested_parent().as_widget().extents() }));
			rect ourRect{ position(), extents() };
			point position = point{ (parentRect.extents() - ourRect.extents()) / 2.0 } +parentRect.top_left();
			if (position.x < 0.0)
				position.x = 0.0;
			if (position.y < 0.0)
				position.y = 0.0;
			if (position.x + ourRect.cx > rootRect.right())
				position.x = rootRect.right() - ourRect.cx;
			if (position.y + ourRect.cy > rootRect.bottom())
				position.y = rootRect.bottom() - ourRect.cy;
			move(position);
		}
		else
			centre();
	}

	bool nested_window::window_enabled() const
	{
		return enabled();
	}

	void nested_window::counted_window_enable(bool aEnable)
	{
		if (aEnable)
			++iCountedEnable;
		else
			--iCountedEnable;
		enable(iCountedEnable >= 0);
	}

	const i_layout& nested_window::non_client_layout() const
	{
		return iNonClientLayout;
	}

	i_layout& nested_window::non_client_layout()
	{
		return iNonClientLayout;
	}

	const i_layout& nested_window::title_bar_layout() const
	{
		return iTitleBarLayout;
	}

	i_layout& nested_window::title_bar_layout()
	{
		return iTitleBarLayout;
	}

	const i_layout& nested_window::menu_layout() const
	{
		return iMenuLayout;
	}

	i_layout& nested_window::menu_layout()
	{
		return iMenuLayout;
	}

	const i_layout& nested_window::toolbar_layout() const
	{
		return iToolbarLayout;
	}

	i_layout& nested_window::toolbar_layout()
	{
		return iToolbarLayout;
	}

	const i_layout& nested_window::client_layout() const
	{
		return iClientLayout;
	}

	i_layout& nested_window::client_layout()
	{
		return iClientLayout;
	}

	const i_layout& nested_window::status_bar_layout() const
	{
		return iStatusBarLayout;
	}

	i_layout& nested_window::status_bar_layout()
	{
		return iStatusBarLayout;
	}

	void nested_window::init(const optional_point& aPosition)
	{
		if (aPosition != boost::none)
			move(*aPosition);
		else
			centre_on_parent();
	}
}