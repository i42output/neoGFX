// nested_window.hpp
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
#include <string>
#include <neogfx/gui/widget/scrollable_widget.hpp>
#include <neogfx/gui/layout/vertical_layout.hpp>
#include <neogfx/gui/widget/title_bar.hpp>
#include <neogfx/gui/widget/i_nested_window.hpp>

namespace neogfx
{
	class nested_window : public i_nested_window, public scrollable_widget
	{
	private:
		typedef boost::optional<title_bar> optional_title_bar;
	public:
		nested_window(i_nested_window_container& aNestedRoot, const size& aDimensions, window_style aStyle = window_style::Default, scrollbar_style aScrollbarStyle = scrollbar_style::Normal, frame_style aFrameStyle = frame_style::WindowFrame);
		nested_window(i_nested_window_container& aNestedRoot, const size& aDimensions, const std::string& aWindowTitle, window_style aStyle = window_style::Default, scrollbar_style aScrollbarStyle = scrollbar_style::Normal, frame_style aFrameStyle = frame_style::WindowFrame);
		nested_window(i_nested_window_container& aNestedRoot, const point& aPosition, const size& aDimensions, window_style aStyle = window_style::Default, scrollbar_style aScrollbarStyle = scrollbar_style::Normal, frame_style aFrameStyle = frame_style::WindowFrame);
		nested_window(i_nested_window_container& aNestedRoot, const point& aPosition, const size& aDimensions, const std::string& aWindowTitle, window_style aStyle = window_style::Default, scrollbar_style aScrollbarStyle = scrollbar_style::Normal, frame_style aFrameStyle = frame_style::WindowFrame);
		nested_window(i_nested_window& aParent, const size& aDimensions, window_style aStyle = window_style::Default, scrollbar_style aScrollbarStyle = scrollbar_style::Normal, frame_style aFrameStyle = frame_style::WindowFrame);
		nested_window(i_nested_window& aParent, const size& aDimensions, const std::string& aWindowTitle, window_style aStyle = window_style::Default, scrollbar_style aScrollbarStyle = scrollbar_style::Normal, frame_style aFrameStyle = frame_style::WindowFrame);
		nested_window(i_nested_window& aParent, const point& aPosition, const size& aDimensions, window_style aStyle = window_style::Default, scrollbar_style aScrollbarStyle = scrollbar_style::Normal, frame_style aFrameStyle = frame_style::WindowFrame);
		nested_window(i_nested_window& aParent, const point& aPosition, const size& aDimensions, const std::string& aWindowTitle, window_style aStyle = window_style::Default, scrollbar_style aScrollbarStyle = scrollbar_style::Normal, frame_style aFrameStyle = frame_style::WindowFrame);
		~nested_window();
	private:
		nested_window(i_widget& aParentWidget, i_nested_window_container& aNestedRoot, const size& aDimensions, const std::string& aWindowTitle, window_style aStyle, scrollbar_style aScrollbarStyle, frame_style aFrameStyle);
		nested_window(i_widget& aParentWidget, i_nested_window& aParent, const size& aDimensions, const std::string& aWindowTitle, window_style aStyle, scrollbar_style aScrollbarStyle, frame_style aFrameStyle);
	public:
		window_style style() const;
		void set_style(window_style aStyle);
	public:
		colour frame_colour() const override;
	public:
		bool can_defer_layout() const override;
		bool is_managing_layout() const override;
	public:
		widget_part hit_test(const point& aPosition) const override;
	public:
		neogfx::size_policy size_policy() const override;
	public:
		colour background_colour() const override;
	public:
		neogfx::scrolling_disposition scrolling_disposition(const i_widget& aChildWidget) const override;
	public:
		const i_nested_window_container& nested_root() const override;
		i_nested_window_container& nested_root() override;
		bool has_nested_parent() const override;
		const i_nested_window& nested_parent() const override;
		i_nested_window& nested_parent() override;
	public:
		const std::string& title_text() const override;
		void set_title_text(const std::string& aTitleText) override;
	public:
		bool is_active() const override;
		void activate() override;
		bool is_iconic() const override;
		void iconize() override;
		bool is_maximized() const override;
		void maximize() override;
		bool is_restored() const override;
		void restore() override;
		window_placement placement() const override;
		void set_placement(const window_placement& aPlacement) override;
		void centre() override;
		void centre_on_parent() override;
		bool window_enabled() const override;
		void counted_window_enable(bool aEnable) override;
	public:
		const i_layout& non_client_layout() const override;
		i_layout& non_client_layout() override;
		const i_layout& title_bar_layout() const override;
		i_layout& title_bar_layout() override;
		const i_layout& menu_layout() const override;
		i_layout& menu_layout() override;
		const i_layout& toolbar_layout() const override;
		i_layout& toolbar_layout() override;
		const i_layout& client_layout() const override;
		i_layout& client_layout() override;
		const i_layout& status_bar_layout() const override;
		i_layout& status_bar_layout() override;
	public:
		const i_widget& as_widget() const override;
		i_widget& as_widget() override;
	private:
		void init(const optional_point& aPosition = optional_point{});
	private:
		i_nested_window_container& iRoot;
		i_nested_window* iParent;
		std::string iTitleText;
		window_style iStyle;
		window_placement iPlacement;
		bool iActive;
		int32_t iCountedEnable;
		vertical_layout iNonClientLayout;
		vertical_layout iTitleBarLayout;
		vertical_layout iMenuLayout;
		vertical_layout iToolbarLayout;
		vertical_layout iClientLayout;
		vertical_layout iStatusBarLayout;
		optional_title_bar iTitleBar;
		sink iSink;
	};
}