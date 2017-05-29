// tab_button.cpp
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
#include <neogfx/app/app.hpp>
#include <neogfx/gui/widget/tab_button.hpp>
#include <neogfx/gui/widget/i_tab_page_container.hpp>

namespace neogfx
{
	class tab_button::close_button : public push_button
	{
	private:
		enum texture_index_e
		{
			TextureOff,
			TextureOn,
			TextureOnOver
		};
	public:
		close_button(i_tab& aParent) :
			push_button{ aParent.as_widget().layout() }, iParent{ aParent }
		{
			set_margins(neogfx::margins{ 2.0 });
			iSink += app::instance().current_style_changed([this]() { update_textures(); });
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
			scoped_units su{ *this, aGraphicsContext, UnitsPixels };
			if (entered())
			{
				double radius = std::sqrt(std::pow(image().image().extents().cx / 2.0, 2.0) * 2.0) + 2.0;
				aGraphicsContext.fill_circle(
					to_client_coordinates(image().to_window_coordinates(image().client_rect().centre())), radius, app::instance().current_style().text_colour());
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
			auto ink = app::instance().current_style().text_colour();
			auto paper = background_colour();
			const uint8_t sTexture[8][8]
			{
				{ 1, 2, 0, 0, 0, 0, 2, 1 },
				{ 2, 1, 2, 0, 0, 2, 1, 2 },
				{ 0, 2, 1, 2, 2, 1, 2, 0 },
				{ 0, 0, 2, 1, 1, 2, 0, 0 },
				{ 0, 0, 2, 1, 1, 2, 0, 0 },
				{ 0, 2, 1, 2, 2, 1, 2, 0 },
				{ 2, 1, 2, 0, 0, 2, 1, 2 },
				{ 1, 2, 0, 0, 0, 0, 2, 1 }
			};
			if (iTextures[TextureOff] == boost::none || iTextures[TextureOff]->first != ink)
			{
				iTextures[TextureOff] = std::make_pair(
					ink,
					neogfx::image{
						"neogfx::tab_button::close_button::iTextures[TextureOff]::" + ink.to_string(),
						sTexture,{ { 0, colour{} },{ 1, ink.with_alpha(0x60) },{ 2, ink.with_alpha(0x30) } } });
			}
			if (iTextures[TextureOn] == boost::none || iTextures[TextureOn]->first != ink)
			{
				iTextures[TextureOn] = std::make_pair(
					ink,
					neogfx::image{
						"neogfx::tab_button::close_button::iTextures[TextureOn]::" + ink.to_string(),
						sTexture, { { 0, colour{} }, { 1, ink }, { 2, ink.with_alpha(0x80) } } });
			}
			if (iTextures[TextureOnOver] == boost::none || iTextures[TextureOnOver]->first != ink)
			{
				iTextures[TextureOnOver] = std::make_pair(
					ink,
					neogfx::image{
						"neogfx::tab_button::close_button::iTextures[TextureOnOver]::" + paper.to_string(),
						sTexture, { { 0, colour{} }, { 1, paper }, { 2, paper.with_alpha(0x80) } } });
			}
			update_appearance();
		}
		void update_appearance()
		{
			if (entered())
				image().set_image(iTextures[TextureOnOver]->second);
			else if (iParent.is_selected() || iParent.as_widget().entered() || (surface().has_entered_widget() && surface().entered_widget().is_descendent_of(iParent.as_widget())))
				image().set_image(iTextures[TextureOn]->second);
			else
				image().set_image(iTextures[TextureOff]->second);
		}
	protected:
		void mouse_entered()
		{
			push_button::mouse_entered();
			update_state();
		}
		void mouse_left()
		{
			push_button::mouse_left();
			update_state();
		}
	private:
		i_tab& iParent;
		sink iSink;
		mutable boost::optional<std::pair<colour, texture>> iTextures[3];
	};

	tab_button::tab_button(i_tab_container& aContainer, const std::string& aText, bool aClosable) :
		push_button{ aText, push_button_style::Tab }, iContainer{ aContainer }, iSelectedState{ false }
	{
		set_size_policy(neogfx::size_policy::Minimum);
		set_closable(aClosable);
		iContainer.adding_tab(*this);
	}

	tab_button::tab_button(i_widget& aParent, i_tab_container& aContainer, const std::string& aText, bool aClosable) :
		push_button{ aParent, aText, push_button_style::Tab }, iContainer{ aContainer }, iSelectedState{ false }
	{
		set_size_policy(neogfx::size_policy::Minimum);
		set_closable(aClosable);
		iContainer.adding_tab(*this);
	}

	tab_button::tab_button(i_layout& aLayout, i_tab_container& aContainer, const std::string& aText, bool aClosable) :
		push_button{ aLayout, aText, push_button_style::Tab }, iContainer{ aContainer }, iSelectedState{ false }
	{
		set_size_policy(neogfx::size_policy::Minimum);
		set_closable(aClosable);
		iContainer.adding_tab(*this);
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
		scoped_units su(*this, UnitsPixels);
		rect result = push_button::path_bounding_rect();
		if (is_deselected())
			result.deflate(as_units(*this, UnitsMillimetres, delta(0.0, 25.4/96.0)).ceil() * delta(0.0, 2.0));
		else
			result.extents() += size(0.0, 5.0);
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

	size tab_button::minimum_size(const optional_size& aAvailableSpace) const
	{
		if (has_minimum_size())
			return push_button::minimum_size(aAvailableSpace);
		scoped_units su(*this, UnitsPixels);
		return convert_units(*this, su.saved_units(), push_button::minimum_size(aAvailableSpace) + as_units(*this, UnitsMillimetres, size(25.4/96.0, 25.4/96.0)).ceil() * size(4.0, 4.0));
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
			push_button::update(to_client_coordinates(window_rect() + delta{ 0.0, 2.0 }));
	}

	void tab_button::mouse_entered()
	{
		push_button::mouse_entered();
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
}