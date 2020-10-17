// tab_button.cpp
/*
neogfx C++ App/Game Engine
Copyright (c) 2015, 2020 Leigh Johnston.  All Rights Reserved.

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

#include <neogfx/app/i_app.hpp>
#include <neogfx/hid/i_surface_manager.hpp>
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
            push_button{ aParent.as_widget().layout() }, iParent{ aParent }, iTextureState{ Unknown }, iUpdater{ service<i_async_task>(), [this](neolib::callback_timer& aTimer) { aTimer.again(); update_appearance(); }, 20 }
        {
            set_padding(neogfx::padding{ 2.0 });
            iSink += service<i_app>().current_style_changed([this](style_aspect aAspect) { if ((aAspect & style_aspect::Color) == style_aspect::Color) update_textures(); });
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
        size minimum_size(optional_size const& aAvailableSpace) const
        {
            auto result = push_button::minimum_size(aAvailableSpace);
            if (has_minimum_size())
                return result;
            double const radius = std::sqrt(std::pow(image().extents().cx / 2.0, 2.0) * 2.0) + 2.0;
            result = result.max(size{ std::ceil(radius * 2.0) }) + padding().size();
            return result;
        }
        size maximum_size(optional_size const& aAvailableSpace) const
        {
            if (has_maximum_size())
                return push_button::maximum_size(aAvailableSpace);
            return minimum_size(aAvailableSpace);
        }
    protected:
        void paint(i_graphics_context& aGc) const
        {
            scoped_units su{ *this, units::Pixels };
            if (entered())
            {
                double const radius = std::sqrt(std::pow(image().extents().cx / 2.0, 2.0) * 2.0) + 2.0;
                aGc.fill_circle(
                    to_client_coordinates(image_widget().to_window_coordinates(image_widget().client_rect().center())), radius, service<i_app>().current_style().palette().color(color_role::Text));
            }
            if (has_focus())
            {
                rect focusRect = to_client_coordinates(image_widget().to_window_coordinates(rect{ image_widget().client_rect().center() - image().extents() / 2.0, image().extents() }));
                focusRect.inflate(2.0, 2.0);
                aGc.draw_focus_rect(focusRect);
            }
        }
    private:
        void update_textures()
        {
            auto ink = service<i_app>().current_style().palette().color(color_role::Text);
            auto paper = background_color();
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
            if (iTextures[TextureOff] == std::nullopt || iTextures[TextureOff]->first != ink)
            {
                iTextures[TextureOff].emplace(
                    ink,
                    !high_dpi() ?
                        neogfx::image{
                            "neogfx::tab_button::close_button::iTextures[TextureOff]::" + ink.to_string(),
                            sTexture, { { "paper", color{} },{ "ink1", ink.with_alpha(0.375) }, { "ink2", ink.with_alpha(0.1875) } } } :
                        neogfx::image{
                            "neogfx::tab_button::close_button::iHighDpiTextures[TextureOff]::" + ink.to_string(),
                            sHighDpiTexture, { { "paper", color{} },{ "ink1", ink.with_alpha(0.375) },{ "ink2", ink.with_alpha(0.1875) } }, 2.0 });
            }
            if (iTextures[TextureOn] == std::nullopt || iTextures[TextureOn]->first != ink)
            {
                iTextures[TextureOn].emplace(
                    ink,
                    !high_dpi() ?
                        neogfx::image{
                            "neogfx::tab_button::close_button::iTextures[TextureOn]::" + ink.to_string(),
                            sTexture, { { "paper", color{} }, { "ink1", ink }, { "ink2", ink.with_alpha(0.5) } } } :
                        neogfx::image{
                            "neogfx::tab_button::close_button::iHighDpiTextures[TextureOn]::" + ink.to_string(),
                            sHighDpiTexture, { { "paper", color{} }, { "ink1", ink }, { "ink2", ink.with_alpha(0.5) } }, 2.0 });
            }
            if (iTextures[TextureOnOver] == std::nullopt || iTextures[TextureOnOver]->first != ink)
            {
                iTextures[TextureOnOver].emplace(
                    ink,
                    !high_dpi() ?
                        neogfx::image{
                            "neogfx::tab_button::close_button::iTextures[TextureOnOver]::" + paper.to_string(),
                            sTexture, { { "paper", color{} }, { "ink1", paper }, { "ink2", paper.with_alpha(0.5) } } } :
                        neogfx::image{
                            "neogfx::tab_button::close_button::iHighDpiTextures[TextureOnOver]::" + paper.to_string(),
                            sHighDpiTexture, { { "paper", color{} }, { "ink1", paper }, { "ink2", paper.with_alpha(0.5) } }, 2.0 });
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
                set_image(iTextures[iTextureState]->second);
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
        mutable std::optional<std::pair<color, texture>> iTextures[3];
        texture_index_e iTextureState;
        neolib::callback_timer iUpdater;
    };

    tab_button::tab_button(i_tab_container& aContainer, std::string const& aText, bool aClosable, bool aStandardImageSize) :
        push_button{ aText, push_button_style::Tab }, iContainer{ aContainer }, iStandardImageSize{ aStandardImageSize }, iSelectedState{ false }
    {
        init();
        set_closable(aClosable);
    }

    tab_button::tab_button(i_widget& aParent, i_tab_container& aContainer, std::string const& aText, bool aClosable, bool aStandardImageSize) :
        push_button{ aParent, aText, push_button_style::Tab }, iContainer{ aContainer }, iStandardImageSize{ aStandardImageSize }, iSelectedState{ false }
    {
        init();
        set_closable(aClosable);
    }

    tab_button::tab_button(i_layout& aLayout, i_tab_container& aContainer, std::string const& aText, bool aClosable, bool aStandardImageSize) :
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

    std::string const& tab_button::text() const
    {
        return push_button::text();
    }

    void tab_button::set_text(std::string const& aText)
    {
        push_button::set_text(aText);
    }

    void tab_button::set_image(const i_texture& aTexture)
    {
        push_button::set_image(aTexture);
    }

    void tab_button::set_image(const i_image& aImage)
    {
        push_button::set_image(aImage);
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
        switch (container().style() & tab_container_style::TabAlignmentMask)
        {
        case tab_container_style::TabAlignmentTop:
            result.extents() += size{ 0.0, 5.0 };
            break;
        case tab_container_style::TabAlignmentBottom:
            result.y -= 5.0;
            result.extents() += size{ 0.0, 5.0 };
            break;
        case tab_container_style::TabAlignmentLeft:
            result.extents() += size{ 5.0, 0.0 };
            break;
        case tab_container_style::TabAlignmentRight:
            result.x -= 5.0;
            result.extents() += size{ 5.0, 0.0 };
            break;
        }
        return to_units(*this, su.saved_units(), result);
    }

    bool tab_button::spot_color() const
    {
        return is_selected();
    }

    bool tab_button::perform_hover_animation() const
    {
        return push_button::perform_hover_animation() && !is_selected();
    }

    padding tab_button::padding() const
    {
        neogfx::padding result = push_button::padding();
        if (has_padding())
            return result;
        switch (container().style() & tab_container_style::TabAlignmentMask)
        {
        case tab_container_style::TabAlignmentTop:
            result += ceil_rasterized(neogfx::padding{ 0.5_mm, 0.5_mm, 0.5_mm, is_selected() ? 0.5_mm : 0.0_mm });
            break;
        case tab_container_style::TabAlignmentBottom:
            result += ceil_rasterized(neogfx::padding{ 0.5_mm, is_selected() ? 0.5_mm : 0.0_mm, 0.5_mm, 0.5_mm });
            break;
        case tab_container_style::TabAlignmentLeft:
            result += ceil_rasterized(neogfx::padding{ 0.5_mm, 0.5_mm, is_selected() ? 0.5_mm : 0.0_mm, 0.5_mm });
            break;
        case tab_container_style::TabAlignmentRight:
            result += ceil_rasterized(neogfx::padding{ is_selected() ? 0.5_mm : 0.0_mm, 0.5_mm, 0.5_mm, 0.5_mm });
            break;
        }
        return result;
    }

    void tab_button::handle_clicked()
    {
        push_button::handle_clicked();
        select();
    }

    color tab_button::palette_color(color_role aColorRole) const
    {
        if (has_palette_color(aColorRole) || is_deselected() || !container().has_tab_page(container().index_of(*this)))
            return push_button::palette_color(aColorRole);
        if (aColorRole == color_role::Base)
        {
            auto& tabPage = container().tab_page(container().index_of(*this)).as_widget();
            return tabPage.palette_color(color_role::Background);
        }
        return push_button::palette_color(aColorRole);
    }

    bool tab_button::update(const rect& aUpdateRect)
    {
        if (!is_selected())
            return push_button::update(aUpdateRect);
        else
            return push_button::update(to_client_coordinates(non_client_rect().inflate(delta{ 2.0, 2.0 })));
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
            update_layout();
            update();
            if (is_selected())
                Selected.trigger();
            else if (is_deselected())
                Deselected.trigger();
        }
    }

    void tab_button::init()
    {
        layout().set_padding(neogfx::padding{ 2.0, 0.0, 0.0, 0.0 });
        set_size_policy(size_constraint::Minimum);
        auto update_image = [this]()
        {
            if (iStandardImageSize)
            {
                set_image_extents(dpi_select(size{ 16.0, 16.0 }, size{ 32.0, 32.0 }));
                image_widget().set_aspect_ratio(aspect_ratio::KeepExpanding);
            }
            else
            {
                image_widget().set_minimum_size({});
                image_widget().set_maximum_size({});
                image_widget().set_aspect_ratio(aspect_ratio::Keep);
            }
            label().layout().invalidate();
        };
        iSink += service<i_surface_manager>().dpi_changed([this, update_image](i_surface&) { update_image(); });
        update_image();
        iContainer.adding_tab(*this);
    }
}