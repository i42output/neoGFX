// gradient_dialog.cpp
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

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/trim.hpp>

#include <neogfx/gui/dialog/gradient_dialog.hpp>
#include <neolib/core/string_utils.hpp>
#include <neolib/task/thread.hpp>
#include <neogfx/core/numerical.hpp>
#include <neogfx/gui/dialog/message_box.hpp>
#include <neogfx/app/file_dialog.hpp>
#include <neogfx/app/i_app.hpp>

namespace neogfx
{
    namespace
    {
        gradient convert_gpf_gradient(const gradient& aGradient, std::string const& aPath)
        {
            std::ifstream gpfGradient{ aPath };
            std::string line;
            gradient::color_stop_list colorStops;
            while (std::getline(gpfGradient, line))
            {
                if (line[0] == '#')
                    continue;
                neolib::vecarray<std::string, 4> bits;
                neolib::tokens(line, " "s, bits);
                if (bits.size() != 4)
                    continue;
                colorStops.emplace_back(
                    boost::lexical_cast<double>(bits[0]), 
                    vec3{ 
                        boost::lexical_cast<double>(bits[1]),
                        boost::lexical_cast<double>(bits[2]),
                        boost::lexical_cast<double>(bits[3]) });
            }
            return gradient{ aGradient, colorStops };
        }

        gradient convert_ggr_gradient(const gradient& aGradient, std::string const& aPath)
        {
            // todo: midpoint support
            // todo: some gradients don't work, e.g. Full_saturation_spectrum_CW
            std::ifstream ggrGradient{ aPath };
            std::string line;
            std::getline(ggrGradient, line);
            std::getline(ggrGradient, line);
            std::getline(ggrGradient, line);
            struct segment
            {
                double start;
                double mid;
                double end;
                vec4 startRgba;
                vec4 endRgba;
            };
            std::optional<segment> s;
            gradient::color_stop_list colorStops;
            gradient::alpha_stop_list alphaStops;
            while (std::getline(ggrGradient, line))
            {
                neolib::vecarray<std::string, 13> bits;
                neolib::tokens(line, " "s, bits);
                if (bits.size() != 13)
                    continue;
                s = segment
                {
                    boost::lexical_cast<double>(bits[0]),
                    boost::lexical_cast<double>(bits[1]),
                    boost::lexical_cast<double>(bits[2]),
                    vec4{
                        boost::lexical_cast<double>(bits[3]),
                        boost::lexical_cast<double>(bits[4]),
                        boost::lexical_cast<double>(bits[5]),
                        boost::lexical_cast<double>(bits[6]),
                    },
                    vec4{
                        boost::lexical_cast<double>(bits[7]),
                        boost::lexical_cast<double>(bits[8]),
                        boost::lexical_cast<double>(bits[9]),
                        boost::lexical_cast<double>(bits[10]),
                    }
                };
                colorStops.emplace_back(s->start, vec3{ s->startRgba.xyz });
                alphaStops.emplace_back(s->start, static_cast<color::component>(s->startRgba[3] * 0xFF));
                colorStops.emplace_back(s->mid, lerp(vec3{ s->startRgba.xyz }, vec3{ s->endRgba.xyz }, 0.5));
                alphaStops.emplace_back(s->mid, static_cast<color::component>(lerp(s->startRgba[3], s->endRgba[3], 0.5) * 0xFF));
            }
            colorStops.emplace_back(s->end, vec3{ s->endRgba.xyz });
            alphaStops.emplace_back(s->end, static_cast<color::component>(s->endRgba[3] * 0xFF));
            for (auto i = alphaStops.begin(); alphaStops.size() > 2 && i != std::prev(std::prev(alphaStops.end()));)
            {
                if (i->second() == std::next(i)->second() && i->second() == std::next(std::next(i))->second())
                    i = std::prev(alphaStops.erase(std::next(i)));
                else
                    ++i;
            }
            return gradient{ aGradient, colorStops, alphaStops };
        }

        struct gradient_definition
        {
            std::vector<std::pair<scalar, std::uint32_t>> colorStops;
            std::vector<std::pair<scalar, std::uint32_t>> alphaStops;
        };

        struct gradient_swatch
        {
            std::string name;
            std::vector<gradient_definition> gradients;
        };

        constexpr std::uint32_t kSwatchRows = 2u;
        constexpr std::uint32_t kSwatchColumns = 8u;

        color rgb_color(std::uint32_t aRgb)
        {
            return color{ static_cast<std::int32_t>((aRgb >> 16) & 0xFF), static_cast<std::int32_t>((aRgb >> 8) & 0xFF), static_cast<std::int32_t>(aRgb & 0xFF) };
        }

        // swatch gradients are stored (and rendered) diagonally at 45 degrees; only their stops are used when selected
        gradient to_swatch_gradient(gradient::abstract_color_stop_list const& aColorStops, gradient::abstract_alpha_stop_list const& aAlphaStops)
        {
            gradient const result = (aAlphaStops.empty() ?
                gradient{ aColorStops, gradient_direction::Diagonal } :
                gradient{ aColorStops, aAlphaStops, gradient_direction::Diagonal });
            return gradient{ *result.with_orientation(to_rad(45.0)) };
        }

        gradient to_gradient(gradient_definition const& aDefinition)
        {
            gradient::color_stop_list colorStops;
            for (auto const& stop : aDefinition.colorStops)
                colorStops.emplace_back(stop.first, rgb_color(stop.second));
            gradient::alpha_stop_list alphaStops;
            for (auto const& stop : aDefinition.alphaStops)
                alphaStops.emplace_back(stop.first, static_cast<color::component>(stop.second));
            return to_swatch_gradient(colorStops, alphaStops);
        }

        // evenly spaced color stops
        gradient_definition even(std::initializer_list<std::uint32_t> aColors)
        {
            gradient_definition result;
            std::size_t index = 0;
            for (auto const& c : aColors)
                result.colorStops.emplace_back(aColors.size() > 1 ? index++ / static_cast<scalar>(aColors.size() - 1) : 0.0, c);
            return result;
        }

        // shiny metal: light, highlight, (sharp edge) shadow, mid, light
        gradient_definition metal(std::uint32_t aLight, std::uint32_t aHighlight, std::uint32_t aShadow, std::uint32_t aMid, std::uint32_t aReflection)
        {
            return gradient_definition{ { { 0.0, aLight }, { 0.45, aHighlight }, { 0.5, aShadow }, { 0.85, aMid }, { 1.0, aReflection } } };
        }

        // single color fading to transparent
        gradient_definition fade(std::uint32_t aColor, std::initializer_list<std::pair<scalar, std::uint32_t>> aAlphaStops)
        {
            return gradient_definition{ { { 0.0, aColor }, { 1.0, aColor } }, aAlphaStops };
        }

        std::vector<gradient_swatch> const& gradient_swatches()
        {
            static std::vector<gradient_swatch> const sSwatches
            {
                { "Shiny Metals", {
                    metal(0xE6C35C, 0xFFF4C2, 0x9C7A16, 0xD4AF37, 0xF5D77A), // gold
                    metal(0xD8D8D8, 0xFFFFFF, 0x8C8C8C, 0xBDBDBD, 0xE6E6E6), // silver
                    metal(0xC0C6CC, 0xFFFFFF, 0x4A5058, 0x9AA3AC, 0xEEF2F5), // chrome
                    metal(0xE5E4E2, 0xFFFFFF, 0xA0A09C, 0xCFCFCB, 0xF2F2F0), // platinum
                    metal(0xE8B4A0, 0xFFE3D8, 0x9E6A5A, 0xD49A86, 0xF2C8B8), // rose gold
                    metal(0xC9A94B, 0xF7E59A, 0x7D6420, 0xB5913A, 0xE0C66A), // brass
                    metal(0xC08A4A, 0xF2C78D, 0x6B4421, 0xA8723A, 0xD9A066), // bronze
                    metal(0xD27D46, 0xFFC09A, 0x7A3B1A, 0xB8693A, 0xE8996A), // copper
                    metal(0xA9A9A3, 0xE6E6E0, 0x5F605C, 0x8E8F8A, 0xC3C4BE), // titanium
                    metal(0x6E7F99, 0xC9D6EA, 0x2E3A4D, 0x55657E, 0x93A4BF), // blued steel
                    metal(0x5A6068, 0xA7AFB8, 0x22262B, 0x474D55, 0x7A828C), // gunmetal
                    metal(0x3A3D42, 0x9A9FA6, 0x0E0F11, 0x2A2C30, 0x5E6268)  // black chrome
                } },
                { "Sky", {
                    even({ 0x2C3E7A, 0x8E6FB5, 0xF2A97F, 0xFFD9A8 }), // dawn
                    even({ 0x6FA8DC, 0xA9D0F5, 0xE3F1FF }), // morning
                    even({ 0x1E6FD9, 0x5AA9F0, 0xBFE3FF }), // midday
                    even({ 0x3B7DD8, 0x8DB8E8, 0xF3E2B8 }), // afternoon
                    even({ 0x2B1A4A, 0x8A2E5C, 0xE8553A, 0xFFB547 }), // sunset
                    even({ 0x1B1F3B, 0x53366B, 0xC06C84, 0xF8B195 }), // dusk
                    even({ 0x0B1026, 0x2B2F77, 0x6E5BA8 }), // twilight
                    even({ 0x02030A, 0x0B1A3A, 0x1E2F5C }), // night
                    even({ 0x8A939C, 0xB9C0C7, 0xDDE1E5 }), // overcast
                    even({ 0x1F262E, 0x3D4752, 0x6B7782 })  // storm
                } },
                { "Nature", {
                    even({ 0x0B3D1F, 0x1E6B34, 0x4C9A4A }), // forest
                    even({ 0x7CC242, 0x4E9A2A, 0x2E6B1A }), // grass
                    even({ 0x4A5D23, 0x6B7F3A, 0x9AAE5A }), // moss
                    even({ 0x7FDBFF, 0x00A6D6, 0x0077BE, 0x004A80 }), // ocean
                    even({ 0x0A4D68, 0x05294A, 0x020B1C }), // deep sea
                    even({ 0x9BE8D8, 0x3CC8C8, 0x1B8FA6 }), // lagoon
                    even({ 0xFF7F50, 0xFF6F61, 0xE2525C }), // coral
                    even({ 0xF5E1B0, 0xE0C080, 0xC19A5B }), // sand
                    even({ 0xEDC9AF, 0xD2996B, 0xA0522D }), // desert
                    even({ 0x8B5A2B, 0x5C3A1E, 0x3B2414 }), // earth
                    even({ 0xFFE08A, 0xE8862A, 0xB23A1E, 0x6B1E12 }), // autumn
                    even({ 0xFFF5C0, 0xFFB000, 0xFF4500, 0x8B0000, 0x2B0000 }), // lava
                    even({ 0xFFFFFF, 0xDDF3FF, 0xA9DCF5, 0x6FB7DE }), // ice
                    even({ 0xFFFFFF, 0xEEF4F8, 0xD7E3EC })  // snow
                } },
                { "Spectrum", {
                    even({ 0xFF0000, 0xFF7F00, 0xFFFF00, 0x00FF00, 0x0000FF, 0x4B0082, 0x8B00FF }), // rainbow
                    even({ 0xFF0000, 0xFFFF00, 0x00FF00, 0x00FFFF, 0x0000FF, 0xFF00FF, 0xFF0000 }), // hue wheel
                    even({ 0xFFB3BA, 0xFFDFBA, 0xFFFFBA, 0xBAFFC9, 0xBAE1FF, 0xD7BAFF }), // pastel
                    even({ 0xFF00FF, 0x00FFFF, 0x39FF14, 0xFFFF00 }), // neon
                    even({ 0xFFFF00, 0xFF8000, 0xFF0000 }), // warm
                    even({ 0x00FF80, 0x00FFFF, 0x0080FF, 0x0000FF }), // cool
                    even({ 0xFF0080, 0xFF8C00, 0x40E0D0 }), // tropical
                    even({ 0x8E2DE2, 0x4A00E0, 0x00C9FF })  // ultraviolet
                } },
                { "Scientific", {
                    even({ 0x440154, 0x3B528B, 0x21908C, 0x5DC963, 0xFDE725 }), // viridis
                    even({ 0x000004, 0x3B0F70, 0x8C2981, 0xDE4968, 0xFE9F6D, 0xFCFDBF }), // magma
                    even({ 0x000004, 0x420A68, 0x932667, 0xDD513A, 0xFCA50A, 0xFCFFA4 }), // inferno
                    even({ 0x0D0887, 0x6A00A8, 0xB12A90, 0xE16462, 0xFCA636, 0xF0F921 }), // plasma
                    even({ 0x00204D, 0x414D6B, 0x7C7B78, 0xBCAF6F, 0xFFEA46 }), // cividis
                    even({ 0x30123B, 0x4662D7, 0x36AAF9, 0x1AE4B6, 0x72FE5E, 0xC8EF34, 0xFABA39, 0xF66B19, 0xCA2A04, 0x7A0403 }), // turbo
                    even({ 0x00008F, 0x0000FF, 0x00FFFF, 0xFFFF00, 0xFF0000, 0x800000 }), // jet
                    even({ 0x000000, 0xFF0000, 0xFFFF00, 0xFFFFFF }), // heat
                    even({ 0x3B4CC0, 0xDDDDDD, 0xB40426 }), // cool-warm (diverging)
                    even({ 0x000000, 0xFFFFFF })  // greyscale
                } },
                { "Monochrome & Fades", {
                    even({ 0xFFFFFF, 0x000000 }), // white to black
                    even({ 0x000000, 0xFFFFFF }), // black to white
                    even({ 0x4A4A4A, 0x2B2B2B, 0x141414 }), // charcoal
                    even({ 0xF5F5F5, 0xD9D9D9, 0xB0B0B0 }), // fog
                    fade(0x000000, { { 0.0, 0xFF }, { 1.0, 0x00 } }), // black fade
                    fade(0xFFFFFF, { { 0.0, 0xFF }, { 1.0, 0x00 } }), // white fade
                    fade(0x000000, { { 0.0, 0x00 }, { 0.5, 0xFF }, { 1.0, 0x00 } }), // black band
                    fade(0xFFFFFF, { { 0.0, 0x00 }, { 0.5, 0xFF }, { 1.0, 0x00 } }), // white band
                    fade(0x000000, { { 0.0, 0x00 }, { 1.0, 0xC0 } }), // shadow (bottom)
                    fade(0xFFFFFF, { { 0.0, 0xC0 }, { 0.5, 0x00 }, { 1.0, 0x00 } })  // gloss (top)
                } }
            };
            return sSwatches;
        }

        std::string& previous_gradient_swatch()
        {
            static std::string sPreviousSwatch;
            return sPreviousSwatch;
        }
    }

    class gradient_dialog::preview_box : public framed_widget<>
    {
        typedef framed_widget<> base_type;
    private:
        static scalar constexpr CURSOR_RADIUS = 6.0;
        static scalar constexpr CURSOR_THICKNESS = 2.0;
    public:
        preview_box(gradient_dialog& aOwner) :
            base_type(aOwner.iPreviewGroupBox.item_layout()),
            iOwner(aOwner),
            iAnimationTimer{ *this, [this](widget_timer& aTimer)
            {
                aTimer.again();
                animate();
            }, std::chrono::milliseconds{ 10 }, true },
            iTracking{ false }
        {
            iSink += surface().closed([this]() { iAnimationTimer.cancel(); });
            set_padding(neogfx::padding{});
        }
    public:
        virtual void paint(i_graphics_context& aGc) const
        {
            base_type::paint(aGc);
            auto const& cr = client_rect();
            draw_alpha_background(aGc, cr, 16.0_dip);
            aGc.fill_rect(cr, iOwner.gradient());
            if (iOwner.gradient().direction() == gradient_direction::Radial && iOwner.gradient().center() != optional_point{})
            {
                point const center{ 
                    cr.center().x + cr.width() / 2.0 * iOwner.gradient().center()->x, 
                    cr.center().y + cr.height() / 2.0 * iOwner.gradient().center()->y };
                auto const radius = dip(CURSOR_RADIUS);
                auto const circumference = 2.0 * math::pi<double>() * radius;
                aGc.draw_circle(center, radius, pen{ color::White, dip(CURSOR_THICKNESS) });
                aGc.draw_circle(center, radius, 
                    pen{ 
                        color::Black, 
                        dip(CURSOR_THICKNESS), 
                        line_dash{ 0x5555u, circumference / 6.0, circumference * neolib::this_process::elapsed_ms() / 1000.0 } });
            }
        }
    public:
        virtual void mouse_button_clicked(mouse_button aButton, const point& aPosition, key_modifier aKeyModifier)
        {
            base_type::mouse_button_clicked(aButton, aPosition, aKeyModifier);
            if (aButton == mouse_button::Left)
            {
                select(aPosition - client_rect(false).top_left());
                iTracking = true;
            }
        }
        virtual void mouse_button_released(mouse_button aButton, const point& aPosition)
        {
            base_type::mouse_button_released(aButton, aPosition);
            if (!capturing())
                iTracking = false;
        }
        virtual void mouse_moved(const point& aPosition, key_modifier aKeyModifier)
        {
            if (iTracking)
                select(aPosition - client_rect(false).top_left());
        }
        void select(const point& aPosition)
        {
            auto cr = client_rect(false);
            point center{ aPosition.x / cr.width() * 2.0 - 1.0, aPosition.y / cr.height() * 2.0 - 1.0 };
            center = center.max(point{ -1.0, -1.0 }).min(point{ 1.0, 1.0 });
            iOwner.gradient_selector().set_gradient(*iOwner.gradient().with_center(center));
        }
    private:
        void animate()
        {
            if (iOwner.gradient().direction() == gradient_direction::Radial && iOwner.gradient().center() != optional_point{})
            {
                rect cr = client_rect();
                point center{ cr.center().x + cr.width() / 2.0 * iOwner.gradient().center()->x, cr.center().y + cr.height() / 2.0 * iOwner.gradient().center()->y };
                auto const cursorLength = dip(CURSOR_RADIUS) + dip(CURSOR_THICKNESS);
                update(rect{ center - point{ cursorLength, cursorLength }, size{ cursorLength * 2.0, cursorLength * 2.0 } });
            }
        }
    private:
        gradient_dialog& iOwner;
        neolib::sink iSink;
        widget_timer iAnimationTimer;
        bool iTracking;
    };

    class gradient_dialog::swatch_box : public framed_widget<>
    {
        typedef framed_widget<> base_type;
    public:
        swatch_box(gradient_dialog& aOwner, std::size_t aSlot) :
            base_type{ frame_style::SolidFrame }, iOwner{ aOwner }, iSlot{ aSlot }
        {
            set_padding(neogfx::padding{});
            set_fixed_size(size{ 48.0_dip, 32.0_dip });
        }
    public:
        void paint_non_client(i_graphics_context& aGc) const override
        {
            if (iOwner.swatch_gradient(iSlot) != nullptr)
                base_type::paint_non_client(aGc); // no frame for unused slots
        }
        void paint(i_graphics_context& aGc) const override
        {
            auto const swatchGradient = iOwner.swatch_gradient(iSlot);
            if (swatchGradient == nullptr)
                return;
            base_type::paint(aGc);
            auto const cr = client_rect(false);
            draw_alpha_background(aGc, cr, 4.0_dip);
            aGc.fill_rect(cr, *swatchGradient);
            if (iOwner.current_swatch_editable() && iOwner.iCurrentSwatchGradient == iSlot)
            {
                auto const radius = cr.height() * 0.25;
                aGc.fill_circle(cr.center(), radius, color::White);
                aGc.fill_circle(cr.center(), radius - 1.0_dip, color::Black);
            }
        }
        void mouse_button_clicked(mouse_button aButton, const point& aPosition, key_modifier aKeyModifier) override
        {
            base_type::mouse_button_clicked(aButton, aPosition, aKeyModifier);
            auto const swatchGradient = iOwner.swatch_gradient(iSlot);
            if (swatchGradient == nullptr)
                return;
            if (aButton == mouse_button::Left)
            {
                // take the swatch's color and alpha stops, keep the current gradient's other settings
                iOwner.set_gradient(neogfx::gradient{ iOwner.gradient(), swatchGradient->color_stops(), swatchGradient->alpha_stops() });
                iOwner.set_current_swatch_gradient(iSlot);
            }
            else if (aButton == mouse_button::Right)
                iOwner.set_current_swatch_gradient(iSlot);
        }
    private:
        gradient_dialog& iOwner;
        std::size_t iSlot;
    };

    gradient_dialog::gradient_dialog(i_widget& aParent, const neogfx::gradient& aCurrentGradient) :
        dialog(aParent, "Select Gradient"_t, window_style::Dialog | window_style::Modal | window_style::TitleBar | window_style::Close),
        iLayout{ client_layout() }, iLayout2{ iLayout }, iLayout3{ iLayout2 }, iLayout4{ iLayout2 },
        iSelectorGroupBox{ iLayout3 },
        iGradientSelector{ *this, iSelectorGroupBox.item_layout(), aCurrentGradient },
        iLayout3_1{ iSelectorGroupBox.item_layout() },
        iReverse{ iLayout3_1, image{ ":/neogfx/resources/icons/reverse.png" } },
        iReversePartial{ iLayout3_1, image{ ":/neogfx/resources/icons/reversepartial.png" } },
        iHueSlider{ iLayout3_1 },
        iImport{ iLayout3_1, image{ ":/neogfx/resources/icons/open.png" } },
        iDelete{ iLayout3_1, image{ ":/neogfx/resources/icons/delete.png" } },
        iLayout3_2{ iLayout3, alignment::Top },
        iDirectionGroupBox{ iLayout3_2, "Direction"_t },
        iDirectionHorizontalRadioButton{ iDirectionGroupBox.item_layout(), "Horizontal"_t },
        iDirectionVerticalRadioButton{ iDirectionGroupBox.item_layout(), "Vertical"_t },
        iDirectionDiagonalRadioButton{ iDirectionGroupBox.item_layout(), "Diagonal"_t },
        iDirectionRectangularRadioButton{ iDirectionGroupBox.item_layout(), "Rectangular"_t },
        iDirectionRadialRadioButton{ iDirectionGroupBox.item_layout(), "Radial"_t },
        iTile{ iLayout3_2, "Tile"_t },
        iTileWidthLabel{ iTile.with_item_layout<grid_layout>(3u, 2u).add_span(grid_layout::cell_coordinates{0u, 2u}, grid_layout::cell_coordinates{1u, 2u}), "Width:"_t },
        iTileWidth{ iTile.item_layout() },
        iTileHeightLabel{ iTile.item_layout(), "Height:"_t },
        iTileHeight{ iTile.item_layout() },
        iTileAligned{ iTile.item_layout(), "Aligned"_t },
        iSmoothnessGroupBox{ iLayout3_2, "Smooth (%)"_t },
        iSmoothnessSpinBox{ iSmoothnessGroupBox.item_layout() },
        iSmoothnessSlider{ iSmoothnessGroupBox.item_layout() },
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
        iLinkedExponents{ iExponentGroupBox.with_item_layout<grid_layout>(3u, 2u).add_span(grid_layout::cell_coordinates{0u, 0u}, grid_layout::cell_coordinates{1u, 0u}), "Linked"_t },
        iMExponent{ iExponentGroupBox.item_layout(), "m:"_t },
        iMExponentSpinBox{ iExponentGroupBox.item_layout() },
        iNExponent{ iExponentGroupBox.item_layout(), "n:"_t },
        iNExponentSpinBox{ iExponentGroupBox.item_layout() },
        iCenterGroupBox{ iLayout5, "Center"_t },
        iXCenter{ iCenterGroupBox.with_item_layout<grid_layout>(2u, 2u), "X:"_t },
        iXCenterSpinBox { iCenterGroupBox.item_layout() },
        iYCenter{ iCenterGroupBox.item_layout(), "Y:"_t },
        iYCenterSpinBox { iCenterGroupBox.item_layout() },
        iSpacer2{ iLayout5 },
        iSpacer3{ iLayout3 },
        iPreviewGroupBox{ iLayout4, "Preview"_t },
        iPreview{ new preview_box{*this} },
        iSwatchGroupBox{ iLayout4, "Swatches"_t },
        iSwatchToolbar{ iSwatchGroupBox.item_layout() },
        iSwatchSelector{ iSwatchToolbar },
        iNewSwatch{ iSwatchToolbar, "New..."_t },
        iImportSwatch{ iSwatchToolbar, "Import..."_t },
        iDeleteSwatch{ iSwatchToolbar, "Delete"_t },
        iSwatchGrid{ iSwatchGroupBox.item_layout() },
        iSwatchEditLayout{ iSwatchGroupBox.item_layout() },
        iAddToSwatch{ iSwatchEditLayout, "Add to Swatch"_t },
        iRemoveFromSwatch{ iSwatchEditLayout, "Remove from Swatch"_t },
        iSpacer4{ iLayout4 },
        iUpdatingWidgets{ false },
        iIgnoreHueSliderChange{ false }
    {
        init();
    }

    gradient_dialog::~gradient_dialog()
    {
    }

    gradient gradient_dialog::gradient() const
    {
        return gradient_selector().gradient();
    }

    void gradient_dialog::set_gradient(const i_gradient& aGradient)
    {
        gradient_selector().set_gradient(aGradient);
    }

    void gradient_dialog::set_gradient(const i_ref_ptr<i_gradient>& aGradient)
    {
        set_gradient(*aGradient);
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
        iLayout.set_padding(neogfx::padding{});
        iLayout2.set_padding(neogfx::padding{});
        iLayout3.set_padding(neogfx::padding{});
        iLayout5.set_alignment(alignment::Top);
        iReverse.set_size_policy(size_constraint::Fixed);
        iReverse.set_image_extents(size{ 16_dip, 16_dip });
        iReverse.image_widget().set_image_color(service<i_app>().current_style().palette().color(color_role::Text));
        iReversePartial.set_size_policy(size_constraint::Fixed);
        iReversePartial.set_image_extents(size{ 16_dip, 16_dip });
        iReversePartial.image_widget().set_image_color(service<i_app>().current_style().palette().color(color_role::Text));
        iHueSlider.set_minimum(0.0);
        iHueSlider.set_maximum(360.0);
        iHueSlider.set_step(1.0);
        iHueSlider.disable();
        neogfx::gradient::color_stop_list hues;
        for (std::uint32_t s = 0; s < neogfx::gradient::MaxStops; ++s)
        {
            double const pos = s / static_cast<double>((neogfx::gradient::MaxStops - 1));
            hues.emplace_back(pos, color::from_hsv(pos * 360.0, 1.0, 1.0));
        }
        iHueSlider.set_bar_color(neogfx::gradient{ hues, gradient_direction::Horizontal });
        iImport.set_size_policy(size_constraint::Fixed);
        iImport.set_image_extents(size{ 16_dip, 16_dip });
        iDelete.set_size_policy(size_constraint::Fixed);
        iDelete.set_image_extents(size{ 16_dip, 16_dip });
        iDelete.image_widget().set_image_color(service<i_app>().current_style().palette().color(color_role::Text));
        iTile.set_checkable(true);
        iTile.item_layout().set_alignment(alignment::Right);
        iTileWidth.set_minimum(2);
        iTileWidth.set_maximum(9999);
        iTileWidth.set_step(1);
        iTileHeight.set_minimum(2);
        iTileHeight.set_maximum(9999);
        iTileHeight.set_step(1);
        iTileAligned.set_size_policy(size_constraint::Expanding);
        iSmoothnessSpinBox.set_minimum(0.0);
        iSmoothnessSpinBox.set_maximum(100.0);
        iSmoothnessSpinBox.set_step(0.1);
        iSmoothnessSpinBox.set_format("{:.1f}");
        iSmoothnessSlider.set_minimum(0.0);
        iSmoothnessSlider.set_maximum(100.0);
        iSmoothnessSlider.set_step(0.1);
        iOrientationGroupBox.item_layout().set_alignment(alignment::Top);
        iAngleSpinBox.set_minimum(-360.0);
        iAngleSpinBox.set_maximum(360.0);
        iAngleSpinBox.set_step(0.1);
        iAngleSpinBox.set_format("{:.1f}");
        iAngleSlider.set_minimum(-360.0);
        iAngleSlider.set_maximum(360.0);
        iAngleSlider.set_step(0.1);
        iExponentGroupBox.set_checkable(true);
        iExponentGroupBox.item_layout().set_alignment(alignment::Right);
        iLinkedExponents.set_size_policy(size_constraint::Expanding);
        iLinkedExponents.set_checked(true);
        iMExponentSpinBox.set_minimum(0.0);
        iMExponentSpinBox.set_maximum(std::numeric_limits<double>::max());
        iMExponentSpinBox.set_step(0.1);
        iMExponentSpinBox.set_format("{:.2f}");
        iMExponentSpinBox.text_box().set_alignment(alignment::Right);
        iMExponentSpinBox.text_box().set_size_hint(size_hint{ "00.00" });
        iNExponentSpinBox.set_minimum(0.0);
        iNExponentSpinBox.set_maximum(std::numeric_limits<double>::max());
        iNExponentSpinBox.set_step(0.1);
        iNExponentSpinBox.set_format("{:.2f}");
        iNExponentSpinBox.text_box().set_alignment(alignment::Right);
        iNExponentSpinBox.text_box().set_size_hint(size_hint{ "00.00" });
        iCenterGroupBox.set_checkable(true);
        iXCenterSpinBox.set_minimum(-1.0);
        iXCenterSpinBox.set_maximum(1.0);
        iXCenterSpinBox.set_step(0.001);
        iXCenterSpinBox.set_format("{:.3f}");
        iXCenterSpinBox.text_box().set_alignment(alignment::Right);
        iXCenterSpinBox.text_box().set_size_hint(size_hint{ "-0.000" });
        iYCenterSpinBox.set_minimum(-1.0);
        iYCenterSpinBox.set_maximum(1.0);
        iYCenterSpinBox.set_step(0.001);
        iYCenterSpinBox.set_format("{:.3f}");
        iYCenterSpinBox.text_box().set_alignment(alignment::Right);
        iYCenterSpinBox.text_box().set_size_hint(size_hint{ "-0.000" });

        iGradientSelector.set_fixed_size(size{ 256.0_dip, iGradientSelector.minimum_size().cy });

        auto update_hue_selection = [this]()
        {
            neolib::scoped_flag scope{ iIgnoreHueSliderChange };
            iHueSlider.set_value(sRGB_color{ iGradientSelector.selected_color_stop()->second() }.to_hsv().hue());
            iHueSelection.clear();
            auto stopIter = iGradientSelector.gradient().color_stops().begin();
            for (std::size_t stopIndex = 0; stopIndex < iGradientSelector.gradient().color_stops().size(); ++stopIndex, ++stopIter)
            {
                auto const& colorStop = *stopIter;
                auto d = (sRGB_color{ colorStop.second() }.to_hsv().hue() - iHueSlider.value());
                auto const tolerance_deg = 3.0; // todo: make this configurable
                if (std::abs(d) < tolerance_deg)
                    iHueSelection.emplace_back(stopIndex, d);
            }
        };

        iGradientSelector.GradientChanged([this, update_hue_selection]()
        {
            if (iGradientSelector.selected_color_stop() != iGradientSelector.gradient().color_stops().end())
                update_hue_selection();
            update_widgets();
        });

        iReverse.Clicked([this]()
        {
            set_gradient(*gradient().reversed());
        });

        iReversePartial.Clicked([this]()
        {
            auto partiallyReversedGradient = gradient();
            for (auto colorStop = partiallyReversedGradient.color_stops().begin(); colorStop != partiallyReversedGradient.color_stops().end(); ++colorStop)
                colorStop->second() = std::prev(gradient().color_stops().end(), std::distance(partiallyReversedGradient.color_stops().begin(), colorStop) + 1)->second();
            set_gradient(partiallyReversedGradient);
        });

        iGradientSelector.ColorStopSelected([this, update_hue_selection]()
        {
            iHueSlider.enable();
            update_hue_selection();
        });

        iGradientSelector.ColorStopDeselected([this]()
        {
            iHueSlider.disable();
            iHueSelection.clear();
        });

        iHueSlider.ValueChanged([this]()
        {
            if (iIgnoreHueSliderChange)
                return;
            neolib::scoped_flag scope{ iIgnoreHueSliderChange };
            neogfx::gradient newGradient = iGradientSelector.gradient();
            for (auto const& hs : iHueSelection)
            {
                auto& colorStop = *std::next(newGradient.color_stops().begin(), hs.first);
                auto newColor = sRGB_color{ colorStop.second() }.to_hsv();
                newColor.set_hue(iHueSlider.value() + hs.second);
                colorStop.second() = newColor.to_rgb<color>();
            }
            thread_local decltype(iHueSelection) hueSelectionCopy;
            hueSelectionCopy = iHueSelection;
            auto stopIndex = iGradientSelector.selected_color_stop() - iGradientSelector.gradient().color_stops().begin();
            set_gradient(newGradient);
            iGradientSelector.select_color_stop(std::next(iGradientSelector.gradient().color_stops().begin(), stopIndex));
            iHueSelection = hueSelectionCopy;
        });

        iImport.Clicked([this]()
        {
            auto const imports = open_file_dialog(*this, file_dialog_spec{ "Import Gradients", {}, { "*.gpf", "*.ggr" }, "Gradient Files" }, true);
            if (imports)
            {
                // todo: populate swatch library
                try
                {
                    auto const& path = (*imports)[0];
                    if (path.rfind(".gpf") == path.size() - 4)
                        set_gradient(convert_gpf_gradient(gradient(), path));
                    else if (path.rfind(".ggr") == path.size() - 4)
                        set_gradient(convert_ggr_gradient(gradient(), path));
                }
                catch (...)
                {
                    message_box::error("Import Gradient", "Failed to import gradient(s)");
                }
            }
        });

        iDelete.Clicked([this]()
        {
            set_gradient(neogfx::gradient{});
        });

        iTile.check_box().checked([this]() 
        { 
            if (gradient().tile() == std::nullopt)
                set_gradient(gradient().with_tile(gradient_tile{ size{ 2, 2 } }));
        });
        iTile.check_box().unchecked([this]() 
        { 
            set_gradient(gradient().with_tile({})); 
        });
        iTileWidth.ValueChanged([this]()
        {
            auto existing = gradient().tile();
            if (existing != std::nullopt)
            {
                existing->extents.cx = iTileWidth.value();
                set_gradient(gradient().with_tile(existing));
            }
        });
        iTileHeight.ValueChanged([this]()
        {
            auto existing = gradient().tile();
            if (existing != std::nullopt)
            {
                existing->extents.cy = iTileHeight.value();
                set_gradient(gradient().with_tile(existing));
            }
        });
        iTileAligned.toggled([this]()
        {
            auto existing = gradient().tile();
            if (existing != std::nullopt)
            {
                existing->aligned = iTileAligned.is_checked();
                set_gradient(gradient().with_tile(existing));
            }
        });

        iSmoothnessSpinBox.ValueChanged([this]() { set_gradient(gradient().with_smoothness(iSmoothnessSpinBox.value() / 100.0)); });
        iSmoothnessSlider.ValueChanged([this]() { set_gradient(gradient().with_smoothness(iSmoothnessSlider.value() / 100.0)); });

        iDirectionHorizontalRadioButton.checked([this]() { set_gradient(gradient().with_direction(gradient_direction::Horizontal)); });
        iDirectionVerticalRadioButton.checked([this]() { set_gradient(gradient().with_direction(gradient_direction::Vertical)); });
        iDirectionDiagonalRadioButton.checked([this]() { set_gradient(gradient().with_direction(gradient_direction::Diagonal)); });
        iDirectionRectangularRadioButton.checked([this]() { set_gradient(gradient().with_direction(gradient_direction::Rectangular)); });
        iDirectionRadialRadioButton.checked([this]() { set_gradient(gradient().with_direction(gradient_direction::Radial)); });

        iTopLeftRadioButton.checked([this]() { set_gradient(gradient().with_orientation(corner::TopLeft)); });
        iTopRightRadioButton.checked([this]() { set_gradient(gradient().with_orientation(corner::TopRight)); });
        iBottomRightRadioButton.checked([this]() { set_gradient(gradient().with_orientation(corner::BottomRight)); });
        iBottomLeftRadioButton.checked([this]() { set_gradient(gradient().with_orientation(corner::BottomLeft)); });
        iAngleRadioButton.checked([this]() 
        { 
            if (!std::holds_alternative<double>(iGradientSelector.gradient().orientation())) 
                set_gradient(gradient().with_orientation(0.0)); 
        });

        iAngleSpinBox.ValueChanged([this]() { set_gradient(gradient().with_orientation(to_rad(iAngleSpinBox.value()))); });
        iAngleSlider.ValueChanged([this]() { set_gradient(gradient().with_orientation(to_rad(iAngleSlider.value()))); });

        iSizeClosestSideRadioButton.checked([this]() { set_gradient(gradient().with_size(gradient_size::ClosestSide)); });
        iSizeFarthestSideRadioButton.checked([this]() { set_gradient(gradient().with_size(gradient_size::FarthestSide)); });
        iSizeClosestCornerRadioButton.checked([this]() { set_gradient(gradient().with_size(gradient_size::ClosestCorner)); });
        iSizeFarthestCornerRadioButton.checked([this]() { set_gradient(gradient().with_size(gradient_size::FarthestCorner)); });

        iShapeEllipseRadioButton.checked([this]() { set_gradient(gradient().with_shape(gradient_shape::Ellipse)); });
        iShapeCircleRadioButton.checked([this]() { set_gradient(gradient().with_shape(gradient_shape::Circle)); });

        iExponentGroupBox.check_box().checked([this]() { set_gradient(gradient().with_exponents(vec2{2.0, 2.0})); });
        iExponentGroupBox.check_box().Unchecked([this]() { set_gradient(gradient().with_exponents(optional_vec2{})); });

        iLinkedExponents.checked([this]() { auto e = gradient().exponents(); if (e != std::nullopt) { set_gradient(gradient().with_exponents(vec2{ e->x, e->x })); } });

        iMExponentSpinBox.ValueChanged([this]()
        { 
            if (iUpdatingWidgets)
                return;
            auto e = gradient().exponents(); 
            if (e == std::nullopt) 
                e = vec2{}; 
            e->x = iMExponentSpinBox.value(); 
            if (iLinkedExponents.is_checked())
                e->y = e->x;
            set_gradient(gradient().with_exponents(e)); 
        });

        iNExponentSpinBox.ValueChanged([this]()
        {
            if (iUpdatingWidgets)
                return;
            auto e = gradient().exponents();
            if (e == std::nullopt)
                e = vec2{};
            e->y = iNExponentSpinBox.value();
            if (iLinkedExponents.is_checked())
                e->x = e->y;
            set_gradient(gradient().with_exponents(e));
        });

        iCenterGroupBox.check_box().Checked([this]() { set_gradient(gradient().center() ? gradient() : gradient().with_center(point{})); });
        iCenterGroupBox.check_box().Unchecked([this]() { set_gradient(gradient().with_center(optional_point{})); });

        iXCenterSpinBox.ValueChanged([this]() { auto c = gradient().center(); if (c == std::nullopt) c = point{}; c->x = iXCenterSpinBox.value(); set_gradient(gradient().with_center(c)); });
        iYCenterSpinBox.ValueChanged([this]() { auto c = gradient().center(); if (c == std::nullopt) c = point{}; c->y = iYCenterSpinBox.value(); set_gradient(gradient().with_center(c)); });

        iPreview->set_padding(neogfx::padding{});
        iPreview->set_fixed_size(size{ std::ceil(256.0_dip * 16.0 / 9.0), 256.0_dip });

        iSwatchGrid.set_dimensions(kSwatchRows, kSwatchColumns);
        for (std::size_t swatchSlot = 0; swatchSlot < kSwatchRows * kSwatchColumns; ++swatchSlot)
            iSwatchGrid.add(make_ref<swatch_box>(*this, swatchSlot));
        init_swatches();
        iSwatchSelector.SelectionChanged([this](const optional_item_model_index& aIndex)
        {
            if (aIndex != std::nullopt && aIndex->row() < iSwatches.size())
                set_current_swatch(aIndex->row());
        });
        iNewSwatch.clicked([this]()
        {
            auto const newPath = save_file_dialog(*this, file_dialog_spec{ "New Gradient Swatch", swatch_folder() + "/New Swatch.gsw", { "*.gsw" }, "Gradient Swatch Files" });
            if (newPath == std::nullopt)
                return;
            std::filesystem::path path{ *newPath };
            if (path.extension() != ".gsw")
                path += ".gsw";
            swatch newSwatch{ path.stem().string(), {}, false, path.string() };
            if (!save_swatch(newSwatch))
            {
                message_box::error(*this, "New Gradient Swatch", "Failed to save swatch");
                return;
            }
            add_swatch(std::move(newSwatch));
        });
        iImportSwatch.clicked([this]()
        {
            auto const imports = open_file_dialog(*this, file_dialog_spec{ "Import Gradient Swatches", swatch_folder() + "/", { "*.gsw" }, "Gradient Swatch Files" }, true);
            if (imports == std::nullopt)
                return;
            bool failed = false;
            for (auto const& path : *imports)
            {
                swatch importedSwatch;
                if (load_swatch(path, importedSwatch))
                    add_swatch(std::move(importedSwatch));
                else
                    failed = true;
            }
            if (failed)
                message_box::error(*this, "Import Gradient Swatches", "Failed to import swatch(es)");
        });
        iDeleteSwatch.clicked([this]()
        {
            if (!current_swatch_editable())
                return;
            auto const& existing = iSwatches[iCurrentSwatch];
            if (message_box::question(*this, "Delete Gradient Swatch", string{ "Delete swatch '" + existing.name + "'?" }) != standard_button::Yes)
                return;
            std::error_code ec;
            std::filesystem::remove(std::filesystem::path{ existing.path }, ec);
            iSwatches.erase(std::next(iSwatches.begin(), iCurrentSwatch));
            if (iCurrentSwatch > 0)
                --iCurrentSwatch;
            iCurrentSwatchGradient = std::nullopt;
            previous_gradient_swatch() = iSwatches[iCurrentSwatch].name;
            update_swatch_selector();
            update_swatch_buttons();
            update();
        });
        iAddToSwatch.clicked([this]()
        {
            if (!current_swatch_editable())
                return;
            auto& existing = iSwatches[iCurrentSwatch];
            if (existing.gradients.size() >= MaxSwatchGradients)
                return;
            existing.gradients.push_back(to_swatch_gradient(gradient().color_stops(), gradient().alpha_stops()));
            iCurrentSwatchGradient = existing.gradients.size() - 1;
            if (!save_swatch(existing))
                message_box::error(*this, "Add to Swatch", "Failed to save swatch");
            update_swatch_selector();
            update_swatch_buttons();
            update();
        });
        iRemoveFromSwatch.clicked([this]()
        {
            if (!current_swatch_editable() || iCurrentSwatchGradient == std::nullopt)
                return;
            auto& existing = iSwatches[iCurrentSwatch];
            if (*iCurrentSwatchGradient >= existing.gradients.size())
                return;
            existing.gradients.erase(std::next(existing.gradients.begin(), *iCurrentSwatchGradient));
            if (existing.gradients.empty())
                iCurrentSwatchGradient = std::nullopt;
            else if (*iCurrentSwatchGradient >= existing.gradients.size())
                iCurrentSwatchGradient = existing.gradients.size() - 1;
            if (!save_swatch(existing))
                message_box::error(*this, "Remove from Swatch", "Failed to save swatch");
            update_swatch_selector();
            update_swatch_buttons();
            update();
        });

        button_box().add_button(standard_button::Ok);
        button_box().add_button(standard_button::Cancel);
        
        update_widgets();

        update_layout();
        center_on_parent();
        set_ready_to_render(true);
    }

    std::string gradient_dialog::swatch_folder()
    {
        return service<i_app>().info().settings_folder().to_std_string();
    }

    // *.gsw format: one gradient per line: "pos:#RRGGBB ..." optionally followed by " | pos:alpha ..."
    bool gradient_dialog::load_swatch(std::string const& aPath, swatch& aSwatch)
    {
        std::ifstream input{ std::filesystem::path{ aPath } };
        if (!input)
            return false;
        aSwatch = swatch{ std::filesystem::path{ aPath }.stem().string(), {}, false, aPath };
        std::string line;
        while (std::getline(input, line))
        {
            boost::algorithm::trim(line);
            if (line.empty() || line[0] == ';')
                continue;
            if (line.rfind("name=", 0) == 0)
            {
                auto name = line.substr(5);
                boost::algorithm::trim(name);
                if (!name.empty())
                    aSwatch.name = name;
                continue;
            }
            if (aSwatch.gradients.size() >= MaxSwatchGradients)
                break;
            neogfx::gradient::color_stop_list colorStops;
            neogfx::gradient::alpha_stop_list alphaStops;
            try
            {
                std::istringstream tokens{ line };
                std::string token;
                bool alpha = false;
                while (tokens >> token)
                {
                    if (token == "|")
                    {
                        alpha = true;
                        continue;
                    }
                    auto const separator = token.find(':');
                    if (separator == std::string::npos)
                        continue;
                    auto const pos = std::clamp(std::stod(token.substr(0, separator)), 0.0, 1.0);
                    auto const value = token.substr(separator + 1);
                    if (!alpha)
                        colorStops.emplace_back(pos, color{ value });
                    else
                        alphaStops.emplace_back(pos, static_cast<color::component>(std::clamp(std::stoi(value), 0, 0xFF)));
                }
            }
            catch (...)
            {
                continue; // skip unrecognized entry
            }
            if (colorStops.empty())
                continue;
            if (colorStops.size() == 1)
                colorStops.emplace_back(1.0, colorStops[0].second());
            aSwatch.gradients.push_back(to_swatch_gradient(colorStops, alphaStops));
        }
        return true;
    }

    bool gradient_dialog::save_swatch(swatch const& aSwatch)
    {
        if (aSwatch.predefined || aSwatch.path.empty())
            return false;
        std::ofstream output{ std::filesystem::path{ aSwatch.path }, std::ios::trunc };
        if (!output)
            return false;
        output << "; neoGFX gradient swatch" << std::endl;
        output << "name=" << aSwatch.name << std::endl;
        for (auto const& swatchGradient : aSwatch.gradients)
        {
            bool first = true;
            for (auto const& stop : swatchGradient.color_stops())
            {
                output << (first ? "" : " ") << stop.first() << ":" << color{ stop.second() }.with_alpha(static_cast<color::component>(0xFF)).to_hex_string();
                first = false;
            }
            if (!swatchGradient.alpha_stops().empty())
            {
                output << " |";
                for (auto const& stop : swatchGradient.alpha_stops())
                    output << " " << stop.first() << ":" << static_cast<std::uint32_t>(stop.second());
            }
            output << std::endl;
        }
        return static_cast<bool>(output);
    }

    void gradient_dialog::init_swatches()
    {
        iSwatches.clear();
        for (auto const& definition : gradient_swatches())
        {
            swatch predefinedSwatch{ definition.name, {}, true };
            for (auto const& gradientDefinition : definition.gradients)
                predefinedSwatch.gradients.push_back(to_gradient(gradientDefinition));
            iSwatches.push_back(std::move(predefinedSwatch));
        }
        std::vector<swatch> userSwatches;
        try
        {
            std::filesystem::path const folder{ swatch_folder() };
            if (std::filesystem::is_directory(folder))
                for (auto const& file : std::filesystem::directory_iterator{ folder })
                    if (file.is_regular_file() && file.path().extension() == ".gsw")
                    {
                        swatch userSwatch;
                        if (load_swatch(file.path().string(), userSwatch))
                            userSwatches.push_back(std::move(userSwatch));
                    }
        }
        catch (...)
        {
            // swatch library unavailable; predefined swatches only
        }
        std::sort(userSwatches.begin(), userSwatches.end(), [](swatch const& lhs, swatch const& rhs) { return lhs.name < rhs.name; });
        for (auto& userSwatch : userSwatches)
            iSwatches.push_back(std::move(userSwatch));
        iCurrentSwatch = 0;
        for (std::size_t swatchIndex = 0; swatchIndex < iSwatches.size(); ++swatchIndex)
            if (iSwatches[swatchIndex].name == previous_gradient_swatch())
            {
                iCurrentSwatch = swatchIndex;
                break;
            }
        iCurrentSwatchGradient = std::nullopt;
        update_swatch_selector();
        update_swatch_buttons();
    }

    std::size_t gradient_dialog::add_swatch(swatch&& aSwatch)
    {
        auto existing = std::find_if(iSwatches.begin(), iSwatches.end(), [&](swatch const& s)
        {
            return !s.predefined && std::filesystem::path{ s.path }.lexically_normal() == std::filesystem::path{ aSwatch.path }.lexically_normal();
        });
        if (existing != iSwatches.end())
            *existing = std::move(aSwatch);
        else
            existing = iSwatches.insert(iSwatches.end(), std::move(aSwatch));
        iCurrentSwatch = static_cast<std::size_t>(std::distance(iSwatches.begin(), existing));
        iCurrentSwatchGradient = std::nullopt;
        previous_gradient_swatch() = iSwatches[iCurrentSwatch].name;
        update_swatch_selector();
        update_swatch_buttons();
        update();
        return iCurrentSwatch;
    }

    void gradient_dialog::update_swatch_selector()
    {
        iSwatchSelector.selection_model().clear_current_index();
        iSwatchSelector.model().clear();
        for (std::uint32_t swatchIndex = 0; swatchIndex < iSwatches.size(); ++swatchIndex)
            iSwatchSelector.model().insert_item(item_model_index{ swatchIndex }, string{ iSwatches[swatchIndex].name });
        if (iCurrentSwatch < iSwatches.size())
        {
            iSwatchSelector.selection_model().set_current_index(iSwatchSelector.presentation_model().from_item_model_index(item_model_index{ static_cast<std::uint32_t>(iCurrentSwatch) }));
            iSwatchSelector.accept_selection();
        }
    }

    void gradient_dialog::update_swatch_buttons()
    {
        bool const editable = current_swatch_editable();
        iDeleteSwatch.enable(editable);
        iAddToSwatch.enable(editable && iSwatches[iCurrentSwatch].gradients.size() < MaxSwatchGradients);
        iRemoveFromSwatch.enable(editable && iCurrentSwatchGradient != std::nullopt && *iCurrentSwatchGradient < iSwatches[iCurrentSwatch].gradients.size());
    }

    void gradient_dialog::set_current_swatch(std::size_t aSwatch)
    {
        if (iCurrentSwatch == aSwatch || aSwatch >= iSwatches.size())
            return;
        iCurrentSwatch = aSwatch;
        iCurrentSwatchGradient = std::nullopt;
        previous_gradient_swatch() = iSwatches[iCurrentSwatch].name;
        update_swatch_buttons();
        update();
    }

    bool gradient_dialog::current_swatch_editable() const
    {
        return iCurrentSwatch < iSwatches.size() && !iSwatches[iCurrentSwatch].predefined;
    }

    neogfx::gradient const* gradient_dialog::swatch_gradient(std::size_t aSlot) const
    {
        if (iCurrentSwatch < iSwatches.size() && aSlot < iSwatches[iCurrentSwatch].gradients.size())
            return &iSwatches[iCurrentSwatch].gradients[aSlot];
        return nullptr;
    }

    void gradient_dialog::set_current_swatch_gradient(std::optional<std::size_t> const& aSlot)
    {
        if (iCurrentSwatchGradient == aSlot)
            return;
        iCurrentSwatchGradient = aSlot;
        update_swatch_buttons();
        update();
    }

    void gradient_dialog::update_widgets()
    {
        if (iUpdatingWidgets)
            return;
        neolib::scoped_flag sf{ iUpdatingWidgets };
        static neogfx::gradient const sDefaultGradient{ color::Black };
        iDelete.enable(gradient() != sDefaultGradient);
        iTile.check_box().set_checked(gradient().tile() != std::nullopt);
        if (gradient().tile() != std::nullopt)
        {
            iTileWidth.set_value(static_cast<std::uint32_t>(gradient().tile()->extents.cx));
            iTileHeight.set_value(static_cast<std::uint32_t>(gradient().tile()->extents.cy));
        }
        else
        {
            iTileWidth.text_box().set_text(string{ "" });
            iTileHeight.text_box().set_text(string{ "" });
        }
        iTileAligned.set_checked(gradient().tile() != std::nullopt ? gradient().tile()->aligned : false);
        iTileWidthLabel.enable(gradient().tile() != std::nullopt);
        iTileWidth.enable(gradient().tile() != std::nullopt);
        iTileHeightLabel.enable(gradient().tile() != std::nullopt);
        iTileHeight.enable(gradient().tile() != std::nullopt);
        iTileAligned.enable(gradient().tile() != std::nullopt);
        iSmoothnessSpinBox.set_value(gradient().smoothness() * 100.0);
        iSmoothnessSlider.set_value(gradient().smoothness() * 100.0);
        iDirectionHorizontalRadioButton.set_checked(gradient().direction() == gradient_direction::Horizontal);
        iDirectionVerticalRadioButton.set_checked(gradient().direction() == gradient_direction::Vertical);
        iDirectionDiagonalRadioButton.set_checked(gradient().direction() == gradient_direction::Diagonal);
        iDirectionRectangularRadioButton.set_checked(gradient().direction() == gradient_direction::Rectangular);
        iDirectionRadialRadioButton.set_checked(gradient().direction() == gradient_direction::Radial);
        iTopLeftRadioButton.set_checked(gradient().orientation() == gradient_orientation(corner::TopLeft));
        iTopRightRadioButton.set_checked(gradient().orientation() == gradient_orientation(corner::TopRight));
        iBottomRightRadioButton.set_checked(gradient().orientation() == gradient_orientation(corner::BottomRight));
        iBottomLeftRadioButton.set_checked(gradient().orientation() == gradient_orientation(corner::BottomLeft));
        iAngleRadioButton.set_checked(std::holds_alternative<double>(gradient().orientation()));
        iAngleSpinBox.set_value(std::holds_alternative<double>(gradient().orientation()) ? to_deg(static_variant_cast<double>(gradient().orientation())) : 0.0);
        iAngleSlider.set_value(std::holds_alternative<double>(gradient().orientation()) ? to_deg(static_variant_cast<double>(gradient().orientation())) : 0.0);
        iSizeClosestSideRadioButton.set_checked(gradient().size() == gradient_size::ClosestSide);
        iSizeFarthestSideRadioButton.set_checked(gradient().size() == gradient_size::FarthestSide);
        iSizeClosestCornerRadioButton.set_checked(gradient().size() == gradient_size::ClosestCorner);
        iSizeFarthestCornerRadioButton.set_checked(gradient().size() == gradient_size::FarthestCorner);
        iShapeEllipseRadioButton.set_checked(gradient().shape() == gradient_shape::Ellipse);
        iShapeCircleRadioButton.set_checked(gradient().shape() == gradient_shape::Circle);
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
            iMExponentSpinBox.text_box().set_text(""_s);
            iNExponentSpinBox.text_box().set_text(""_s);
        }
        iLinkedExponents.enable(specifyExponents);
        iMExponent.enable(specifyExponents);
        iMExponentSpinBox.enable(specifyExponents);
        iNExponent.enable(specifyExponents);
        iNExponentSpinBox.enable(specifyExponents);        
        auto const center = gradient().center();
        bool const specifyCenter = (center != std::nullopt);
        if (specifyCenter)
        {
            iXCenterSpinBox.set_value(center->x, false);
            iYCenterSpinBox.set_value(center->y, false);
        }
        else
        {
            iXCenterSpinBox.text_box().set_text(""_s);
            iYCenterSpinBox.text_box().set_text(""_s);
        }
        iCenterGroupBox.check_box().set_checked(specifyCenter);
        iXCenter.enable(specifyCenter);
        iXCenterSpinBox.enable(specifyCenter);
        iYCenter.enable(specifyCenter);
        iYCenterSpinBox.enable(specifyCenter);
        switch (gradient().direction())
        {
        case gradient_direction::Vertical:
        case gradient_direction::Horizontal:
        case gradient_direction::Rectangular:
            iTileWidthLabel.show(gradient().direction() != gradient_direction::Vertical);
            iTileWidth.show(gradient().direction() != gradient_direction::Vertical);
            iTileHeightLabel.show(gradient().direction() != gradient_direction::Horizontal);
            iTileHeight.show(gradient().direction() != gradient_direction::Horizontal);
            iOrientationGroupBox.hide();
            iSizeGroupBox.hide();
            iShapeGroupBox.hide();
            iExponentGroupBox.hide();
            iCenterGroupBox.hide();
            break;
        case gradient_direction::Diagonal:
            iTileWidthLabel.show();
            iTileWidth.show();
            iTileHeightLabel.show();
            iTileHeight.show();
            iOrientationGroupBox.show();
            iAngleGroupBox.show(std::holds_alternative<double>(gradient().orientation()));
            iSizeGroupBox.hide();
            iShapeGroupBox.hide();
            iExponentGroupBox.hide();
            iCenterGroupBox.hide();
            break;
        case gradient_direction::Radial:
            iTileWidthLabel.show();
            iTileWidth.show();
            iTileHeightLabel.show();
            iTileHeight.show();
            iOrientationGroupBox.hide();
            iSizeGroupBox.show();
            iShapeGroupBox.show();
            iExponentGroupBox.show(gradient().shape() == gradient_shape::Ellipse);
            iCenterGroupBox.show();
            break;
        }
        iPreview->update();
    }
}