// tab_button.cpp
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

#include <neogfx/neogfx.hpp>
#include <neogfx/app/app.hpp>
#include <neogfx/gui/widget/tab_button.hpp>

namespace neogfx
{
	class tab_button::close_button : public push_button
	{
	private:
		enum texture_index_e
		{
			Unknown = -1,
			TextureOff = 0,
			TextureOn,
			TextureOnOver
		};
	public:
		close_button(i_tab& aParent) :
			push_button{ aParent.as_widget().layout() }, iParent{ aParent }, iTextureState{ Unknown }, iUpdater{ app::instance(), [this](neolib::callback_timer& aTimer) { aTimer.again(); update_appearance(); }, 20 }
		{
			set_margins(neogfx::margins{ 2.0 });
			iSink += app::instance().current_style_changed([this](style_aspect aAspect) { if ((aAspect & style_aspect::Colour) == style_aspect::Colour) update_textures(); });
			iSink += aParent.selected([this]() { update_state(); });
			iSink += aParent.deselected([this]() { update_state(); });
			update_textures();
			update_state();
		}
	public:
		void update_state()
		{
			update_appearance();
		}
	protected:
		void paint(graphics_context& aGraphicsContext) const
		{
			scoped_units su{ *this, aGraphicsContext, units::Pixels };
			if (entered())
			{
				double radius = std::sqrt(std::pow(image().image().extents().cx / 2.0, 2.0) * 2.0) + 2.0;
				aGraphicsContext.fill_circle(
					to_client_coordinates(image().to_window_coordinates(image().client_rect().centre())), radius, app::instance().current_style().palette().text_colour());
			}
			if (has_focus())
			{
				rect focusRect = to_client_coordinates(image().to_window_coordinates(rect{ image().client_rect().centre() - image().image().extents() / 2.0, image().image().extents() }));
				focusRect.inflate(2.0, 2.0);
				aGraphicsContext.draw_focus_rect(focusRect);
			}
		}
	private:
		void update_textures()
		{
			auto ink = app::instance().current_style().palette().text_colour();
			auto paper = background_colour();
			const char* sTexture
			{
				"[8,8]"
				"{0,paper}"
				"{1,ink1}"
				"{2,ink2}"

				"12000021"
				"21200212"
				"02122120"
				"00211200"
				"00211200"
				"02122120"
				"21200212"
				"12000021"
			};
			const char* sHighDpiTexture
			{
				"[16,16]"
				"{0,paper}"
				"{1,ink1}"
				"{2,ink2}"

				"1120000000000211"
				"1112000000002111"
				"2111200000021112"
				"0211120000211120"
				"0021112002111200"
				"0002111221112000"
				"0000211111120000"
				"0000021111200000"
				"0000021111200000"
				"0000211111120000"
				"0002111221112000"
				"0021112002111200"
				"0211120000211120"
				"2111200000021112"
				"1112000000002111"
				"1120000000000211"
			};
			if (iTextures[TextureOff] == boost::none || iTextures[TextureOff]->first != ink)
			{
				iTextures[TextureOff] = std::make_pair(
					ink,
					!high_dpi() ?
						neogfx::image{
							"neogfx::tab_button::close_button::iTextures[TextureOff]::" + ink.to_string(),
							sTexture, { { "paper", colour{} },{ "ink1", ink.with_alpha(0x60) }, { "ink2", ink.with_alpha(0x30) } } } :
						neogfx::image{
							"neogfx::tab_button::close_button::iHighDpiTextures[TextureOff]::" + ink.to_string(),
							sHighDpiTexture, { { "paper", colour{} },{ "ink1", ink.with_alpha(0x60) },{ "ink2", ink.with_alpha(0x30) } }, 2.0 });
			}
			if (iTextures[TextureOn] == boost::none || iTextures[TextureOn]->first != ink)
			{
				iTextures[TextureOn] = std::make_pair(
					ink,
					!high_dpi() ?
						neogfx::image{
							"neogfx::tab_button::close_button::iTextures[TextureOn]::" + ink.to_string(),
							sTexture, { { "paper", colour{} }, { "ink1", ink }, { "ink2", ink.with_alpha(0x80) } } } :
						neogfx::image{
							"neogfx::tab_button::close_button::iHighDpiTextures[TextureOn]::" + ink.to_string(),
							sHighDpiTexture, { { "paper", colour{} }, { "ink1", ink }, { "ink2", ink.with_alpha(0x80) } }, 2.0 });
			}
			if (iTextures[TextureOnOver] == boost::none || iTextures[TextureOnOver]->first != ink)
			{
				iTextures[TextureOnOver] = std::make_pair(
					ink,
					!high_dpi() ?
						neogfx::image{
							"neogfx::tab_button::close_button::iTextures[TextureOnOver]::" + paper.to_string(),
							sTexture, { { "paper", colour{} }, { "ink1", paper }, { "ink2", paper.with_alpha(0x80) } } } :
						neogfx::image{
							"neogfx::tab_button::close_button::iHighDpiTextures[TextureOnOver]::" + paper.to_string(),
							sHighDpiTexture, { { "paper", colour{} }, { "ink1", paper }, { "ink2", paper.with_alpha(0x80) } }, 2.0 });
			}
			iTextureState = Unknown;
			update_appearance();
		}
		void update_appearance()
		{
			auto oldState = iTextureState;
			if (entered())
				iTextureState = TextureOnOver;
			else if (iParent.is_selected() || iParent.as_widget().entered() || (has_root() && root().has_entered_widget() && root().entered_widget().is_descendent_of(iParent.as_widget())))
				iTextureState = TextureOn;
			else
				iTextureState = TextureOff;
			if (iTextureState != oldState)
				image().set_image(iTextures[iTextureState]->second);
		}
	protected:
		void mouse_entered(const point& aPosition) override
		{
			push_button::mouse_entered(aPosition);
			update_state();
		}
		void mouse_left() override
		{
			push_button::mouse_left();
			update_state();
		}
	private:
		i_tab & iParent;
		sink iSink;
		mutable boost::optional<std::pair<colour, texture>> iTextures[3];
		texture_index_e iTextureState;
		neolib::callback_timer iUpdater;
	};

	tab_button::tab_button(i_tab_container& aContainer, const std::string& aText, bool aClosable, bool aStandardImageSize) :
		push_button{ aText, push_button_style::Tab }, iContainer{ aContainer }, iStandardImageSize{ aStandardImageSize }, iSelectedState{ false }
	{
		init();
		set_closable(aClosable);
	}

	tab_button::tab_button(i_widget& aParent, i_tab_container& aContainer, const std::string& aText, bool aClosable, bool aStandardImageSize) :
		push_button{ aParent, aText, push_button_style::Tab }, iContainer{ aContainer }, iStandardImageSize{ aStandardImageSize }, iSelectedState{ false }
	{
		init();
		set_closable(aClosable);
	}

	tab_button::tab_button(i_layout& aLayout, i_tab_container& aContainer, const std::string& aText, bool aClosable, bool aStandardImageSize) :
		push_button{ aLayout, aText, push_button_style::Tab }, iContainer{ aContainer }, iStandardImageSize{ aStandardImageSize }, iSelectedState{ false }
	{
		init();
		set_closable(aClosable);
	}

	tab_button::~tab_button()
	{
		iContainer.removing_tab(*this);
	}

	const i_tab_container& tab_button::container() const
	{
		return iContainer;
	}

	i_tab_container& tab_button::container()
	{
		return iContainer;
	}

	bool tab_button::closable() const
	{
		return iCloseButton.get() != nullptr;
	}

	void tab_button::set_closable(bool aClosable)
	{
		if (aClosable != closable())
		{
			if (aClosable)
			{
				iCloseButton = std::make_unique<close_button>(*this);
				iCloseButton->clicked([this]()
				{
					iContainer.remove_tab(iContainer.index_of(*this));
				});
			}
			else
				iCloseButton.reset();
		}
	}

	bool tab_button::is_selected() const
	{
		return iSelectedState == true;
	}

	bool tab_button::is_deselected() const
	{
		return iSelectedState == false;
	}

	void tab_button::select()
	{
		set_selected_state(true);
	}

	const std::string& tab_button::text() const
	{
		return push_button::text().text();
	}

	void tab_button::set_text(const std::string& aText)
	{
		push_button::text().set_text(aText);
	}

	void tab_button::set_image(const i_texture& aTexture)
	{
		push_button::image().set_image(aTexture);
	}

	void tab_button::set_image(const i_image& aImage)
	{
		push_button::image().set_image(aImage);
	}

	const i_widget& tab_button::as_widget() const
	{
		return *this;
	}

	i_widget& tab_button::as_widget()
	{
		return *this;
	}

	rect tab_button::path_bounding_rect() const
	{
		scoped_units su{ *this, units::Pixels };
		rect result = push_button::path_bounding_rect();
		switch (container().style())
		{
		case tab_container_style::TabAlignmentTop:
		case tab_container_style::TabAlignmentLeft: // todo
		case tab_container_style::TabAlignmentRight: // todo
			result.extents() += size{ 0.0, 5.0 };
			break;
		case tab_container_style::TabAlignmentBottom:
			result.y -= 5.0;
			result.extents() += size{ 0.0, 5.0 };
			break;
		}
		return convert_units(*this, su.saved_units(), result);
	}

	bool tab_button::spot_colour() const
	{
		return is_selected();
	}

	colour tab_button::border_mid_colour() const
	{
		if (is_deselected() || !container().has_tab_page(container().index_of(*this)))
			return push_button::border_mid_colour();
		auto& tabPage = container().tab_page(container().index_of(*this)).as_widget();
		return tabPage.background_colour();
	}

	bool tab_button::perform_hover_animation() const
	{
		return push_button::perform_hover_animation() && !is_selected();
	}

	size tab_button::minimum_size(const optional_size& aAvailableSpace) const
	{
		auto result = push_button::minimum_size(aAvailableSpace);
		if (has_minimum_size())
			return result;
		result = convert_units(*this, units::Millimetres, result) + size{ 2.0, is_selected() ? 1.0 : 0.0 };
		scoped_units su{ *this, units::Millimetres };
		result = convert_units(*this, units::Pixels, result).ceil();
		scoped_units su2{ *this, units::Pixels };
		return convert_units(*this, su.saved_units(), result);
	}

	void tab_button::handle_clicked()
	{
		push_button::handle_clicked();
		select();
	}

	colour tab_button::foreground_colour() const
	{
		if (has_foreground_colour() || is_deselected() || !container().has_tab_page(container().index_of(*this)))
			return push_button::foreground_colour();
		auto& tabPage = container().tab_page(container().index_of(*this)).as_widget();
		return tabPage.background_colour();
	}

	void tab_button::update(const rect& aUpdateRect)
	{
		if (!is_selected())
			push_button::update(aUpdateRect);
		else
			push_button::update(to_client_coordinates(window_rect().inflate(delta{ 0.0, 2.0 })));
	}

	void tab_button::mouse_entered(const point& aPosition)
	{
		push_button::mouse_entered(aPosition);
		if (closable())
			iCloseButton->update_state();
	}

	void tab_button::mouse_left()
	{
		push_button::mouse_left();
		if (closable())
			iCloseButton->update_state();
	}

	void tab_button::set_selected_state(bool aSelectedState)
	{
		if (iSelectedState != aSelectedState)
		{
			if (aSelectedState)
				iContainer.selecting_tab(*this);
			iSelectedState = aSelectedState;
			update();
			if (is_selected())
				selected.trigger();
			else if (is_deselected())
				deselected.trigger();
		}
	}

	void tab_button::init()
	{
		set_size_policy(neogfx::size_policy::Minimum);
		auto update_image = [this]()
		{
			if (iStandardImageSize)
			{
				push_button::image().set_fixed_size(dpi_select(size{ 16.0, 16.0 }, size{ 32.0, 32.0 }));
				push_button::image().set_aspect_ratio(aspect_ratio::KeepExpanding);
			}
			else
			{
				push_button::image().set_minimum_size(optional_size{});
				push_button::image().set_maximum_size(optional_size{});
				push_button::image().set_aspect_ratio(aspect_ratio::Keep);
			}
			label().layout().invalidate();
		};
		iSink += root().surface().dpi_changed([this, update_image]() { update_image(); });
		update_image();
		iContainer.adding_tab(*this);
	}
}