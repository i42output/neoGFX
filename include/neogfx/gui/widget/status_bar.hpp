// status_bar.hpp
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
#include <neogfx/gui/widget/widget.hpp>
#include <neogfx/gui/layout/horizontal_layout.hpp>
#include <neogfx/gui/layout/stack_layout.hpp>
#include <neogfx/gui/widget/label.hpp>
#include <neogfx/gui/layout/spacer.hpp>
#include <neogfx/gui/widget/image_widget.hpp>

namespace neogfx
{
	class status_bar : public widget
	{
	public:
		enum class style : uint32_t
		{
			DisplayNone				= 0x0000,
			DisplayMessage			= 0x0001,
			DisplayKeyboardLocks	= 0x0010,
			DisplaySizeGrip			= 0x8000
		};
		friend constexpr style operator|(style aLhs, style aRhs)
		{
			return static_cast<style>(static_cast<uint32_t>(aLhs) | static_cast<uint32_t>(aRhs));
		}
		friend constexpr style operator&(style aLhs, style aRhs)
		{
			return static_cast<style>(static_cast<uint32_t>(aLhs) & static_cast<uint32_t>(aRhs));
		}
		typedef uint32_t widget_index;
	private:
		class separator : public widget
		{
		public:
			separator();
		public:
			neogfx::size_policy size_policy() const override;
			size minimum_size(const optional_size& aAvailableSpace) const override;
		public:
			void paint(graphics_context& aGraphicsContext) const override;
		};
		class keyboard_lock_status : public widget
		{
		public:
			keyboard_lock_status(i_layout& aLayout);
		public:
			neogfx::size_policy size_policy() const override;
		private:
			horizontal_layout iLayout;
			std::unique_ptr<neolib::callback_timer> iUpdater;
		};
		class size_grip : public image_widget
		{
		public:
			size_grip(i_layout& aLayout);
		public:
			neogfx::size_policy size_policy() const override;
		public:
			widget_part hit_test(const point&) const override;
			bool ignore_non_client_mouse_events() const override;
		};
	public:
		struct style_conflict : std::runtime_error { style_conflict() : std::runtime_error("neogfx::status_bar::style_conflict") {} };
		struct no_message : std::runtime_error { no_message() : std::runtime_error("neogfx::status_bar::no_message") {} };
	public:
		status_bar(i_widget& aParent, style aStyle = style::DisplayMessage | style::DisplayKeyboardLocks | style::DisplaySizeGrip);
		status_bar(i_layout& aLayout, style aStyle = style::DisplayMessage | style::DisplayKeyboardLocks | style::DisplaySizeGrip);
	public:
		bool have_message() const;
		std::string message() const;
		void set_message(const std::string& aMessage);
		void clear_message();
		void add_normal_widget(i_widget& aWidget);
		void add_normal_widget_at(widget_index aPosition, i_widget& aWidget);
		void add_normal_widget(std::shared_ptr<i_widget> aWidget);
		void add_normal_widget_at(widget_index aPosition, std::shared_ptr<i_widget> aWidget);
		void add_permanent_widget(i_widget& aWidget);
		void add_permanent_widget_at(widget_index aPosition, i_widget& aWidget);
		void add_permanent_widget(std::shared_ptr<i_widget> aWidget);
		void add_permanent_widget_at(widget_index aPosition, std::shared_ptr<i_widget> aWidget);
	public:
		label& message_widget();
		label& idle_widget();
	protected:
		neogfx::size_policy size_policy() const override;
	protected:
		bool can_defer_layout() const override;
		bool is_managing_layout() const override;
	private:
		void init();
		void update_widgets();
	private:
		sink iSink;
		style iStyle;
		boost::optional<std::string> iMessage;
		horizontal_layout iLayout;
		stack_layout iNormalLayout;
		horizontal_layout iMessageLayout;
		label iMessageWidget;
		horizontal_layout iIdleLayout;
		label iIdleWidget;
		widget iNormalWidgetContainer;
		horizontal_layout iNormalWidgetLayout;
		horizontal_layout iPermanentWidgetLayout;
		keyboard_lock_status iKeyboardLockStatus;
		mutable boost::optional<std::pair<colour, texture>> iSizeGripTexture;
		size_grip iSizeGrip;
	};
}