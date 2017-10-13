// title_bar.cpp
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
#include <neogfx/gui/widget/title_bar.hpp>

namespace neogfx
{
	title_bar::title_bar(i_widget& aParent, const std::string& aTitle) :
		widget{ aParent },
		iLayout{ *this },
		iIcon{ iLayout, app::instance().default_window_icon() },
		iTitle{ iLayout, aTitle },
		iSpacer{ iLayout },
		iMinimizeButton{ iLayout, push_button_style::TitleBar },
		iMaximizeButton{ iLayout, push_button_style::TitleBar },
		iRestoreButton{ iLayout, push_button_style::TitleBar },
		iCloseButton{ iLayout, push_button_style::TitleBar }
	{
		init();
	}

	title_bar::title_bar(i_widget& aParent, const i_texture& aIcon, const std::string& aTitle) :
		widget{ aParent },
		iLayout{ *this },
		iIcon{ iLayout, aIcon },
		iTitle{ iLayout, aTitle },
		iSpacer{ iLayout },
		iMinimizeButton{ iLayout, push_button_style::TitleBar },
		iMaximizeButton{ iLayout, push_button_style::TitleBar },
		iRestoreButton{ iLayout, push_button_style::TitleBar },
		iCloseButton{ iLayout, push_button_style::TitleBar }
	{
		init();
	}

	title_bar::title_bar(i_widget& aParent, const i_image& aIcon, const std::string& aTitle) :
		widget{ aParent },
		iLayout{ *this },
		iIcon{ iLayout, aIcon },
		iTitle{ iLayout, aTitle },
		iSpacer{ iLayout },
		iMinimizeButton{ iLayout, push_button_style::TitleBar },
		iMaximizeButton{ iLayout, push_button_style::TitleBar },
		iRestoreButton{ iLayout, push_button_style::TitleBar },
		iCloseButton{ iLayout, push_button_style::TitleBar }
	{
		init();
	}

	title_bar::title_bar(i_layout& aLayout, const std::string& aTitle) :
		widget{ aLayout },
		iLayout{ *this },
		iIcon{ iLayout, app::instance().default_window_icon() },
		iTitle{ iLayout, aTitle },
		iSpacer{ iLayout },
		iMinimizeButton{ iLayout, push_button_style::TitleBar },
		iMaximizeButton{ iLayout, push_button_style::TitleBar },
		iRestoreButton{ iLayout, push_button_style::TitleBar },
		iCloseButton{ iLayout, push_button_style::TitleBar }
	{
		init();
	}

	title_bar::title_bar(i_layout& aLayout, const i_texture& aIcon, const std::string& aTitle) :
		widget{ aLayout },
		iLayout{ *this },
		iIcon{ iLayout, aIcon },
		iTitle{ iLayout, aTitle },
		iSpacer{ iLayout },
		iMinimizeButton{ iLayout, push_button_style::TitleBar },
		iMaximizeButton{ iLayout, push_button_style::TitleBar },
		iRestoreButton{ iLayout, push_button_style::TitleBar },
		iCloseButton{ iLayout, push_button_style::TitleBar }
	{
		init();
	}

	title_bar::title_bar(i_layout& aLayout, const i_image& aIcon, const std::string& aTitle) :
		widget{ aLayout },
		iLayout{ *this },
		iIcon{ iLayout, aIcon },
		iTitle{ iLayout, aTitle },
		iSpacer{ iLayout },
		iMinimizeButton{ iLayout, push_button_style::TitleBar },
		iMaximizeButton{ iLayout, push_button_style::TitleBar },
		iRestoreButton{ iLayout, push_button_style::TitleBar },
		iCloseButton{ iLayout, push_button_style::TitleBar }
	{
		init();
	}

	const image_widget& title_bar::icon() const
	{
		return iIcon;
	}

	image_widget& title_bar::icon()
	{
		return iIcon;
	}

	const text_widget& title_bar::title() const
	{
		return iTitle;
	}

	text_widget& title_bar::title()
	{
		return iTitle;
	}

	neogfx::size_policy title_bar::size_policy() const
	{
		return neogfx::size_policy{ neogfx::size_policy::Expanding, neogfx::size_policy::Minimum };
	}

	widget_part title_bar::hit_test(const point&) const
	{
		return widget_part::NonClientTitleBar;
	}

	void title_bar::init()
	{
		set_margins(neogfx::margins{});
		iLayout.set_margins(neogfx::margins{ 4.0, 4.0, 4.0, 4.0 });
		iLayout.set_spacing(8.0);
		icon().set_ignore_mouse_events(false);
		if (icon().image().is_empty())
			icon().set_fixed_size(size{ 24.0, 24.0 });
		else
			icon().set_fixed_size(size{ std::min(icon().image().extents().cx, 24.0), std::min(icon().image().extents().cy, 24.0) });
		iMinimizeButton.set_size_policy(neogfx::size_policy{ neogfx::size_policy::Minimum, neogfx::size_policy::Minimum });
		iMaximizeButton.set_size_policy(neogfx::size_policy{ neogfx::size_policy::Minimum, neogfx::size_policy::Minimum });
		iRestoreButton.set_size_policy(neogfx::size_policy{ neogfx::size_policy::Minimum, neogfx::size_policy::Minimum });
		iCloseButton.set_size_policy(neogfx::size_policy{ neogfx::size_policy::Minimum, neogfx::size_policy::Minimum });
		iSink += app::instance().current_style_changed([this](style_aspect aAspect) { if ((aAspect & style_aspect::Colour) == style_aspect::Colour) update_textures(); });
		auto update_widgets = [this]()
		{
			if (surface().surface_type() == surface_type::Window)
			{
				auto& w = static_cast<i_window&>(surface());
				bool isEnabled = w.window_enabled();
				bool isActive = w.is_active();
				bool isIconic = w.is_iconic();
				bool isMaximized = w.is_maximized();
				bool isRestored = w.is_restored();
				icon().enable(isActive);
				title().enable(isActive);
				iMinimizeButton.enable(!isIconic && isEnabled);
				iMaximizeButton.enable(!isMaximized && isEnabled);
				iRestoreButton.enable(!isRestored && isEnabled);
				iCloseButton.enable(isEnabled);
				bool layoutChanged = false;
				layoutChanged = iMinimizeButton.show(!isIconic && (w.style() & window_style::MinimizeBox) == window_style::MinimizeBox) || layoutChanged;
				layoutChanged = iMaximizeButton.show(!isMaximized && (w.style() & window_style::MaximizeBox) == window_style::MaximizeBox) || layoutChanged;
				layoutChanged = iRestoreButton.show(!isRestored && (w.style() & (window_style::MinimizeBox | window_style::MaximizeBox)) != window_style::Invalid) || layoutChanged;
				if (layoutChanged)
				{
					managing_layout().layout_items(true);
					update(true);
				}
			}
		};
		if (surface().surface_type() == surface_type::Window)
		{
			iSink += static_cast<i_window&>(surface()).window_event([this, update_widgets](neogfx::window_event& e)
			{
				switch (e.type())
				{
				case window_event_type::Enabled:
				case window_event_type::Disabled:
				case window_event_type::FocusGained:
				case window_event_type::FocusLost:
				case window_event_type::Iconized:
				case window_event_type::Maximized:
				case window_event_type::Restored:
					update_widgets();
					break;
				default:
					break;
				}
			});
		}
		update_textures();
		update_widgets();
	}

	void title_bar::update_textures()
	{
		auto ink = app::instance().current_style().palette().text_colour();
		auto paper = background_colour();
		const uint8_t sMinimizeTexturePattern[10][10]
		{
			{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
			{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
			{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
			{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
			{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
			{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
			{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
			{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
			{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
			{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 }
		};
		const uint8_t sMaximizeTexturePattern[10][10]
		{
			{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
			{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
			{ 1, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
			{ 1, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
			{ 1, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
			{ 1, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
			{ 1, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
			{ 1, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
			{ 1, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
			{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 }
		};
		const uint8_t sRestoreTexturePattern[10][10]
		{
			{ 0, 0, 0, 1, 1, 1, 1, 1, 1, 1 },
			{ 0, 0, 0, 1, 1, 1, 1, 1, 1, 1 },
			{ 0, 0, 0, 1, 0, 0, 0, 0, 0, 1 },
			{ 1, 1, 1, 1, 1, 1, 1, 0, 0, 1 },
			{ 1, 1, 1, 1, 1, 1, 1, 0, 0, 1 },
			{ 1, 0, 0, 0, 0, 0, 1, 0, 0, 1 },
			{ 1, 0, 0, 0, 0, 0, 1, 1, 1, 1 },
			{ 1, 0, 0, 0, 0, 0, 1, 0, 0, 0 },
			{ 1, 0, 0, 0, 0, 0, 1, 0, 0, 0 },
			{ 1, 1, 1, 1, 1, 1, 1, 0, 0, 0 }
		};
		const uint8_t sCloseTexturePattern[10][10]
		{
			{ 1, 2, 0, 0, 0, 0, 0, 0, 2, 1 },
			{ 2, 1, 2, 0, 0, 0, 0, 2, 1, 2 },
			{ 0, 2, 1, 2, 0, 0, 2, 1, 2, 0 },
			{ 0, 0, 2, 1, 2, 2, 1, 2, 0, 0 },
			{ 0, 0, 0, 2, 1, 1, 2, 0, 0, 0 },
			{ 0, 0, 0, 2, 1, 1, 2, 0, 0, 0 },
			{ 0, 0, 2, 1, 2, 2, 1, 2, 0, 0 },
			{ 0, 2, 1, 2, 0, 0, 2, 1, 2, 0 },
			{ 2, 1, 2, 0, 0, 0, 0, 2, 1, 2 },
			{ 1, 2, 0, 0, 0, 0, 0, 0, 2, 1 }
		};
		if (iTextures[TextureMinimize] == boost::none || iTextures[TextureMinimize]->first != ink)
		{
			iTextures[TextureMinimize] = std::make_pair(
				ink,
				neogfx::image{
				"neogfx::title_bar::iTextures[TextureMinimize]::" + ink.to_string(),
				sMinimizeTexturePattern,{ { 0, colour{} },{ 1, ink },{ 2, ink.with_alpha(0x80) } } });
		}
		if (iTextures[TextureMaximize] == boost::none || iTextures[TextureMaximize]->first != ink)
		{
			iTextures[TextureMaximize] = std::make_pair(
				ink,
				neogfx::image{
				"neogfx::title_bar::iTextures[TextureMaximize]::" + ink.to_string(),
				sMaximizeTexturePattern,{ { 0, colour{} },{ 1, ink },{ 2, ink.with_alpha(0x80) } } });
		}
		if (iTextures[TextureRestore] == boost::none || iTextures[TextureRestore]->first != ink)
		{
			iTextures[TextureRestore] = std::make_pair(
				ink,
				neogfx::image{
				"neogfx::title_bar::iTextures[TextureRestore]::" + ink.to_string(),
				sRestoreTexturePattern,{ { 0, colour{} },{ 1, ink },{ 2, ink.with_alpha(0x80) } } });
		}
		if (iTextures[TextureClose] == boost::none || iTextures[TextureClose]->first != ink)
		{
			iTextures[TextureClose] = std::make_pair(
				ink,
				neogfx::image{
				"neogfx::title_bar::iTextures[TextureClose]::" + ink.to_string(),
				sCloseTexturePattern, { { 0, colour{} },{ 1, ink },{ 2, ink.with_alpha(0x80) } } });
		}
		iMinimizeButton.image().set_image(iTextures[TextureMinimize]->second);
		iMaximizeButton.image().set_image(iTextures[TextureMaximize]->second);
		iRestoreButton.image().set_image(iTextures[TextureRestore]->second);
		iCloseButton.image().set_image(iTextures[TextureClose]->second);
	}
}