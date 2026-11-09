// linux_skin.cpp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2026 Leigh Johnston.  All Rights Reserved.

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

#include <dlfcn.h>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <functional>
#include <map>
#include <optional>
#include <string>
#include <tuple>
#include <vector>

#include <neogfx/gfx/i_texture_manager.hpp>
#include <neogfx/gfx/i_graphics_context.hpp>
#include <neogfx/gui/widget/i_widget.hpp>
#include <neogfx/gui/widget/default_skin.hpp>
#include "native_skin.hpp"

namespace neogfx
{
    namespace native::linux
    {
        namespace
        {
            // GTK 3 (with the GLib, GIO and cairo it uses), loaded at runtime so that it is neither a build nor a package
            // dependency: the few declarations used
            namespace gtk
            {
                typedef unsigned long GType;
                struct GtkStyleContext;
                struct GtkWidgetPath;
                struct GtkCssProvider;
                struct GtkSettings;
                struct cairo_t;
                struct cairo_surface_t;
                struct GDBusConnection;
                struct GVariant;
                struct GError { std::uint32_t domain; int code; char* message; };
                struct GtkBorder { std::int16_t left, right, top, bottom; };
                struct GdkRGBA { double red, green, blue, alpha; };

                constexpr GType TypeNone = 4u; // G_TYPE_NONE
                constexpr unsigned StateNormal = 0u;
                constexpr unsigned StateActive = 1u << 0;
                constexpr unsigned StatePrelight = 1u << 1;
                constexpr unsigned StateInsensitive = 1u << 3;
                constexpr unsigned StateInconsistent = 1u << 4;
                constexpr unsigned StateFocused = 1u << 5;
                constexpr unsigned StateChecked = 1u << 11;
                constexpr unsigned ProviderPriorityApplication = 600u; // GTK_STYLE_PROVIDER_PRIORITY_APPLICATION
                constexpr int OrientationHorizontal = 0;
                constexpr int OrientationVertical = 1;
                constexpr int CairoFormatArgb32 = 0;
                constexpr int BusTypeSession = 2; // G_BUS_TYPE_SESSION
                constexpr double Pi = 3.14159265358979323846;

                struct api
                {
                    void* library = nullptr;

                    void (*gtk_disable_setlocale)() = nullptr;
                    int (*gtk_init_check)(int*, char***) = nullptr;
                    GtkSettings* (*gtk_settings_get_default)() = nullptr;
                    GtkCssProvider* (*gtk_css_provider_get_named)(char const*, char const*) = nullptr;
                    GtkCssProvider* (*gtk_css_provider_new)() = nullptr;
                    int (*gtk_css_provider_load_from_data)(GtkCssProvider*, char const*, long, GError**) = nullptr;
                    GtkStyleContext* (*gtk_style_context_new)() = nullptr;
                    void (*gtk_style_context_set_path)(GtkStyleContext*, GtkWidgetPath*) = nullptr;
                    void (*gtk_style_context_set_parent)(GtkStyleContext*, GtkStyleContext*) = nullptr;
                    void (*gtk_style_context_set_state)(GtkStyleContext*, unsigned) = nullptr;
                    void (*gtk_style_context_set_scale)(GtkStyleContext*, int) = nullptr;
                    void (*gtk_style_context_add_provider)(GtkStyleContext*, GtkCssProvider*, unsigned) = nullptr;
                    void (*gtk_style_context_get)(GtkStyleContext*, unsigned, char const*, ...) = nullptr;
                    void (*gtk_style_context_get_color)(GtkStyleContext*, unsigned, GdkRGBA*) = nullptr;
                    void (*gtk_style_context_get_margin)(GtkStyleContext*, unsigned, GtkBorder*) = nullptr;
                    void (*gtk_style_context_get_padding)(GtkStyleContext*, unsigned, GtkBorder*) = nullptr;
                    void (*gtk_style_context_get_border)(GtkStyleContext*, unsigned, GtkBorder*) = nullptr;
                    GtkWidgetPath* (*gtk_widget_path_new)() = nullptr;
                    int (*gtk_widget_path_append_type)(GtkWidgetPath*, GType) = nullptr;
                    void (*gtk_widget_path_iter_set_object_name)(GtkWidgetPath*, int, char const*) = nullptr;
                    void (*gtk_widget_path_iter_add_class)(GtkWidgetPath*, int, char const*) = nullptr;
                    void (*gtk_widget_path_unref)(GtkWidgetPath*) = nullptr;
                    void (*gtk_render_background)(GtkStyleContext*, cairo_t*, double, double, double, double) = nullptr;
                    void (*gtk_render_frame)(GtkStyleContext*, cairo_t*, double, double, double, double) = nullptr;
                    void (*gtk_render_focus)(GtkStyleContext*, cairo_t*, double, double, double, double) = nullptr;
                    void (*gtk_render_check)(GtkStyleContext*, cairo_t*, double, double, double, double) = nullptr;
                    void (*gtk_render_option)(GtkStyleContext*, cairo_t*, double, double, double, double) = nullptr;
                    void (*gtk_render_expander)(GtkStyleContext*, cairo_t*, double, double, double, double) = nullptr;
                    void (*gtk_render_slider)(GtkStyleContext*, cairo_t*, double, double, double, double, int) = nullptr;
                    void (*gtk_render_arrow)(GtkStyleContext*, cairo_t*, double, double, double, double) = nullptr;

                    void (*g_object_get)(void*, char const*, ...) = nullptr;
                    void (*g_object_unref)(void*) = nullptr;
                    void (*g_free)(void*) = nullptr;
                    void (*g_error_free)(GError*) = nullptr;
                    GVariant* (*g_variant_new)(char const*, ...) = nullptr;
                    void (*g_variant_get)(GVariant*, char const*, ...) = nullptr;
                    int (*g_variant_is_of_type)(GVariant*, char const*) = nullptr;
                    std::uint32_t (*g_variant_get_uint32)(GVariant*) = nullptr;
                    void (*g_variant_unref)(GVariant*) = nullptr;
                    GDBusConnection* (*g_bus_get_sync)(int, void*, GError**) = nullptr;
                    GVariant* (*g_dbus_connection_call_sync)(GDBusConnection*, char const*, char const*, char const*, char const*, GVariant*, char const*, int, int, void*, GError**) = nullptr;

                    cairo_surface_t* (*cairo_image_surface_create)(int, int, int) = nullptr;
                    void (*cairo_surface_set_device_scale)(cairo_surface_t*, double, double) = nullptr;
                    cairo_t* (*cairo_create)(cairo_surface_t*) = nullptr;
                    void (*cairo_destroy)(cairo_t*) = nullptr;
                    void (*cairo_surface_flush)(cairo_surface_t*) = nullptr;
                    unsigned char* (*cairo_image_surface_get_data)(cairo_surface_t*) = nullptr;
                    int (*cairo_image_surface_get_stride)(cairo_surface_t*) = nullptr;
                    void (*cairo_surface_destroy)(cairo_surface_t*) = nullptr;

                    // true if GTK is installed and can use the display
                    bool load()
                    {
                        library = ::dlopen("libgtk-3.so.0", RTLD_NOW | RTLD_LOCAL);
                        if (library == nullptr)
                            return false;
                        bool all = true;
                        auto const resolve = [&](auto& aFunction, char const* aName)
                        {
                            // (n.b. dlsym on the library's handle also searches the libraries it uses: GLib, GIO, cairo)
                            aFunction = reinterpret_cast<std::remove_reference_t<decltype(aFunction)>>(::dlsym(library, aName));
                            all = all && aFunction != nullptr;
                        };
                        resolve(gtk_disable_setlocale, "gtk_disable_setlocale");
                        resolve(gtk_init_check, "gtk_init_check");
                        resolve(gtk_settings_get_default, "gtk_settings_get_default");
                        resolve(gtk_css_provider_get_named, "gtk_css_provider_get_named");
                        resolve(gtk_css_provider_new, "gtk_css_provider_new");
                        resolve(gtk_css_provider_load_from_data, "gtk_css_provider_load_from_data");
                        resolve(gtk_style_context_new, "gtk_style_context_new");
                        resolve(gtk_style_context_set_path, "gtk_style_context_set_path");
                        resolve(gtk_style_context_set_parent, "gtk_style_context_set_parent");
                        resolve(gtk_style_context_set_state, "gtk_style_context_set_state");
                        resolve(gtk_style_context_set_scale, "gtk_style_context_set_scale");
                        resolve(gtk_style_context_add_provider, "gtk_style_context_add_provider");
                        resolve(gtk_style_context_get, "gtk_style_context_get");
                        resolve(gtk_style_context_get_color, "gtk_style_context_get_color");
                        resolve(gtk_style_context_get_margin, "gtk_style_context_get_margin");
                        resolve(gtk_style_context_get_padding, "gtk_style_context_get_padding");
                        resolve(gtk_style_context_get_border, "gtk_style_context_get_border");
                        resolve(gtk_widget_path_new, "gtk_widget_path_new");
                        resolve(gtk_widget_path_append_type, "gtk_widget_path_append_type");
                        resolve(gtk_widget_path_iter_set_object_name, "gtk_widget_path_iter_set_object_name");
                        resolve(gtk_widget_path_iter_add_class, "gtk_widget_path_iter_add_class");
                        resolve(gtk_widget_path_unref, "gtk_widget_path_unref");
                        resolve(gtk_render_background, "gtk_render_background");
                        resolve(gtk_render_frame, "gtk_render_frame");
                        resolve(gtk_render_focus, "gtk_render_focus");
                        resolve(gtk_render_check, "gtk_render_check");
                        resolve(gtk_render_option, "gtk_render_option");
                        resolve(gtk_render_expander, "gtk_render_expander");
                        resolve(gtk_render_slider, "gtk_render_slider");
                        resolve(gtk_render_arrow, "gtk_render_arrow");
                        resolve(g_object_get, "g_object_get");
                        resolve(g_object_unref, "g_object_unref");
                        resolve(g_free, "g_free");
                        resolve(g_error_free, "g_error_free");
                        resolve(g_variant_new, "g_variant_new");
                        resolve(g_variant_get, "g_variant_get");
                        resolve(g_variant_is_of_type, "g_variant_is_of_type");
                        resolve(g_variant_get_uint32, "g_variant_get_uint32");
                        resolve(g_variant_unref, "g_variant_unref");
                        resolve(g_bus_get_sync, "g_bus_get_sync");
                        resolve(g_dbus_connection_call_sync, "g_dbus_connection_call_sync");
                        resolve(cairo_image_surface_create, "cairo_image_surface_create");
                        resolve(cairo_surface_set_device_scale, "cairo_surface_set_device_scale");
                        resolve(cairo_create, "cairo_create");
                        resolve(cairo_destroy, "cairo_destroy");
                        resolve(cairo_surface_flush, "cairo_surface_flush");
                        resolve(cairo_image_surface_get_data, "cairo_image_surface_get_data");
                        resolve(cairo_image_surface_get_stride, "cairo_image_surface_get_stride");
                        resolve(cairo_surface_destroy, "cairo_surface_destroy");
                        if (!all)
                            return false;

                        // n.b. GTK must not change the process's locale; and on X11 GDK replaces the Xlib error handlers (with ones
                        // that abort on an error it hasn't trapped) so whatever was installed (e.g. by neoGFX's X11 display server) is
                        // put back
                        typedef void* (*set_handler_t)(void*);
                        auto const setErrorHandler = reinterpret_cast<set_handler_t>(::dlsym(RTLD_DEFAULT, "XSetErrorHandler"));
                        auto const setIoErrorHandler = reinterpret_cast<set_handler_t>(::dlsym(RTLD_DEFAULT, "XSetIOErrorHandler"));
                        void* errorHandler = nullptr;
                        void* ioErrorHandler = nullptr;
                        if (setErrorHandler != nullptr)
                            setErrorHandler(errorHandler = setErrorHandler(nullptr));
                        if (setIoErrorHandler != nullptr)
                            setIoErrorHandler(ioErrorHandler = setIoErrorHandler(nullptr));
                        gtk_disable_setlocale();
                        bool const initialized = gtk_init_check(nullptr, nullptr) != 0;
                        if (setErrorHandler != nullptr)
                            setErrorHandler(errorHandler);
                        if (setIoErrorHandler != nullptr)
                            setIoErrorHandler(ioErrorHandler);
                        return initialized;
                    }
                };

                api* instance()
                {
                    // (loaded once, when first needed, and never unloaded)
                    static api* const sApi = []() -> api*
                    {
                        static api sInstance;
                        return sInstance.load() ? &sInstance : nullptr;
                    }();
                    return sApi;
                }
            }

            constexpr std::size_t kMaxCachedParts = 512u;

            // the desktop's appearance settings, from the XDG desktop portal (org.freedesktop.portal.Settings)
            struct desktop_settings
            {
                std::optional<color> accentColor;
                bool highContrast = false;
            };

            gtk::GVariant* read_portal_setting(gtk::api& aGtk, gtk::GDBusConnection* aBus, char const* aNamespace, char const* aKey)
            {
                for (char const* method : { "ReadOne", "Read" })
                {
                    gtk::GError* error = nullptr;
                    auto const reply = aGtk.g_dbus_connection_call_sync(aBus, "org.freedesktop.portal.Desktop", "/org/freedesktop/portal/desktop",
                        "org.freedesktop.portal.Settings", method, aGtk.g_variant_new("(ss)", aNamespace, aKey), "(v)", 0, 500, nullptr, &error);
                    if (reply == nullptr)
                    {
                        if (error != nullptr)
                            aGtk.g_error_free(error);
                        continue;
                    }
                    gtk::GVariant* value = nullptr;
                    aGtk.g_variant_get(reply, "(v)", &value);
                    aGtk.g_variant_unref(reply);
                    // (Read, the older method, wraps the value in a further variant)
                    while (value != nullptr && aGtk.g_variant_is_of_type(value, "v"))
                    {
                        gtk::GVariant* inner = nullptr;
                        aGtk.g_variant_get(value, "v", &inner);
                        aGtk.g_variant_unref(value);
                        value = inner;
                    }
                    return value;
                }
                return nullptr;
            }

            desktop_settings read_desktop_settings(gtk::api& aGtk)
            {
                desktop_settings result;
                gtk::GError* error = nullptr;
                auto const bus = aGtk.g_bus_get_sync(gtk::BusTypeSession, nullptr, &error);
                if (bus == nullptr)
                {
                    if (error != nullptr)
                        aGtk.g_error_free(error);
                    return result;
                }
                if (auto const accent = read_portal_setting(aGtk, bus, "org.freedesktop.appearance", "accent-color"))
                {
                    if (aGtk.g_variant_is_of_type(accent, "(ddd)"))
                    {
                        double r = -1.0, g = -1.0, b = -1.0;
                        aGtk.g_variant_get(accent, "(ddd)", &r, &g, &b);
                        // (out of range: no accent colour set)
                        if (r >= 0.0 && r <= 1.0 && g >= 0.0 && g <= 1.0 && b >= 0.0 && b <= 1.0)
                            result.accentColor = color{ r, g, b, 1.0 };
                    }
                    aGtk.g_variant_unref(accent);
                }
                if (auto const contrast = read_portal_setting(aGtk, bus, "org.freedesktop.appearance", "contrast"))
                {
                    if (aGtk.g_variant_is_of_type(contrast, "u"))
                        result.highContrast = (aGtk.g_variant_get_uint32(contrast) == 1u);
                    aGtk.g_variant_unref(contrast);
                }
                aGtk.g_object_unref(bus);
                return result;
            }

            double item_scale(i_skinnable_item const& aItem)
            {
                if (aItem.is_widget() && aItem.as_widget().device_metrics_available())
                {
                    auto const dpi = aItem.as_widget().device_metrics().horizontal_dpi();
                    if (dpi > 0.0)
                        return dpi / 96.0;
                }
                return 1.0;
            }

            // The dark variant of the theme is used for a widget on a dark background (as the Windows skin does).
            bool dark_background(i_skinnable_item const& aItem)
            {
                return aItem.is_widget() && aItem.as_widget().container_background_color().dark();
            }

            unsigned state_flags(bool aEnabled, bool aHot, bool aPressed)
            {
                if (!aEnabled)
                    return gtk::StateInsensitive;
                return (aHot ? gtk::StatePrelight : 0u) | (aPressed ? gtk::StateActive : 0u);
            }

            // a CSS node path, e.g. { "scale.horizontal", "contents", "trough", "slider" } (each node: name.class.class...)
            typedef std::vector<std::string> node_path;

            // Renders widgets using the user's GTK 3 theme (gtk_render_*), the GTK equivalent of the Windows visual styles that
            // the Windows skin uses: parts are drawn with cairo into an image and uploaded as textures; anything GTK has no part
            // for, or everything if GTK isn't available, is drawn by the default skin (the fallback).  The desktop's accent
            // colour and contrast setting (from the XDG desktop portal) are applied.
            class skin : public i_skin
            {
            private:
                typedef std::tuple<std::string, std::int32_t, std::int32_t, std::int32_t> part_key; // part (path, state, variant), cx, cy, scale (%)
            public:
                skin() = default;
                ~skin()
                {
                    reset();
                }
            public:
                std::string const& name() const override
                {
                    static std::string const sName = "Native";
                    return sName;
                }
                bool has_fallback() const override
                {
                    return true;
                }
                i_skin& fallback() override
                {
                    return iFallback;
                }
            public:
                void activate() override
                {
                    // pick up any change to the theme or the desktop's settings made while we were inactive
                    reset();
                }
                void deactivate() override
                {
                    reset();
                }
            public:
                size preferred_size(skin_element aElement, optional_size const& aDesiredSize = {}) const override
                {
                    if (aElement != skin_element::CheckBox && aElement != skin_element::RadioButton)
                        return iFallback.preferred_size(aElement, aDesiredSize);
                    auto const ctx = context(aElement == skin_element::CheckBox ? node_path{ "checkbutton", "check" } : node_path{ "radiobutton", "radio" }, false);
                    if (ctx == nullptr)
                        return iFallback.preferred_size(aElement, aDesiredSize);
                    auto const extents = css_size(ctx, gtk::StateNormal);
                    // (CSS pixels are device independent pixels)
                    return size{ dip(extents.cx), dip(extents.cy) }.ceil();
                }
                dimension push_button_border_width(const i_skinnable_item& aItem, bool aDefaultButton, dimension aBorderWidth) const override
                {
                    return iFallback.push_button_border_width(aItem, aDefaultButton, aBorderWidth);
                }
                dimension window_frame_width(const i_skinnable_item& aItem, dimension aFrameWidth) const override
                {
                    // (window frames and title bars are drawn by neoGFX itself, so keep their appearance consistent with it)
                    return iFallback.window_frame_width(aItem, aFrameWidth);
                }
                void adjust_title_bar_appearance(const i_skinnable_item& aItem, title_bar_appearance& aAppearance) const override
                {
                    iFallback.adjust_title_bar_appearance(aItem, aAppearance);
                }
            public:
                void draw_push_button(i_graphics_context& aGc, const i_skinnable_item& aItem, const i_push_button& aButton, const push_button_appearance& aAppearance) const override
                {
                    auto const ctx = push_button_context(aItem, aButton, aAppearance);
                    if (ctx == nullptr)
                    {
                        iFallback.draw_push_button(aGc, aItem, aButton, aAppearance);
                        return;
                    }
                    auto const& widget = aItem.as_widget();
                    scoped_units su{ widget, units::Pixels };
                    auto const state = push_button_state(aItem, aButton);
                    rect buttonRect = aAppearance.boundingRect;
                    buttonRect.inflate(aAppearance.borderWidth / 2.0);
                    auto& api = *gtk::instance();
                    draw_part(aGc, ctx, state, part_name(aItem, aAppearance.defaultButton ? "button.default" : "button", state), buttonRect, item_scale(aItem),
                        [&](gtk::cairo_t* aCr, double aWidth, double aHeight)
                        {
                            api.gtk_render_background(ctx, aCr, 0.0, 0.0, aWidth, aHeight);
                            api.gtk_render_frame(ctx, aCr, 0.0, 0.0, aWidth, aHeight);
                            if ((state & gtk::StateFocused) != 0u)
                                api.gtk_render_focus(ctx, aCr, 0.0, 0.0, aWidth, aHeight);
                        });
                }
                void push_button_text_color(const i_skinnable_item& aItem, const i_push_button& aButton, const push_button_appearance& aAppearance, i_optional<color>& aTextColor) const override
                {
                    auto const ctx = push_button_context(aItem, aButton, aAppearance);
                    if (ctx == nullptr)
                    {
                        iFallback.push_button_text_color(aItem, aButton, aAppearance, aTextColor);
                        return;
                    }
                    gtk::GdkRGBA textColor{};
                    gtk::instance()->gtk_style_context_get_color(ctx, push_button_state(aItem, aButton), &textColor);
                    aTextColor = color{ textColor.red, textColor.green, textColor.blue, textColor.alpha };
                }
                using i_skin::push_button_text_color;
                void draw_scrollbar(i_graphics_context& aGc, const i_skinnable_item& aItem, const i_scrollbar& aScrollbar) const override
                {
                    if (!aScrollbar.visible() || aScrollbar.auto_hidden())
                        return;
                    bool const vertical = aScrollbar.orientation() == scrollbar_orientation::Vertical;
                    bool const dark = dark_background(aItem);
                    std::string const scrollbar = vertical ? "scrollbar.vertical" : "scrollbar.horizontal";
                    auto const scrollbarCtx = aScrollbar.type() == scrollbar_style::Normal ? context({ scrollbar }, dark) : nullptr;
                    auto const troughCtx = scrollbarCtx != nullptr ? context({ scrollbar, "contents", "trough" }, dark) : nullptr;
                    auto const sliderCtx = troughCtx != nullptr ? context({ scrollbar, "contents", "trough", "slider" }, dark) : nullptr;
                    auto const buttonCtx = sliderCtx != nullptr ? context({ scrollbar, "contents", "button" }, dark) : nullptr;
                    if (buttonCtx == nullptr)
                    {
                        iFallback.draw_scrollbar(aGc, aItem, aScrollbar);
                        return;
                    }

                    scoped_units su{ aItem.as_widget(), units::Pixels };
                    auto& api = *gtk::instance();
                    double const scale = item_scale(aItem);
                    bool const enabled = aItem.as_widget().effectively_enabled();
                    bool const noThumb = (aScrollbar.maximum() - aScrollbar.minimum() <= aScrollbar.page());
                    auto const hovering = aScrollbar.hovering_element();
                    auto const clicked = aScrollbar.clicked_element();

                    // the scrollbar and its trough
                    unsigned const barState = state_flags(enabled && !noThumb, false, false);
                    draw_part(aGc, scrollbarCtx, barState, part_name(aItem, scrollbar + "/trough", barState), aItem.element_rect(skin_element::Scrollbar), scale,
                        [&](gtk::cairo_t* aCr, double aWidth, double aHeight)
                        {
                            api.gtk_render_background(scrollbarCtx, aCr, 0.0, 0.0, aWidth, aHeight);
                            api.gtk_render_frame(scrollbarCtx, aCr, 0.0, 0.0, aWidth, aHeight);
                            api.gtk_style_context_set_state(troughCtx, barState);
                            api.gtk_render_background(troughCtx, aCr, 0.0, 0.0, aWidth, aHeight);
                            api.gtk_render_frame(troughCtx, aCr, 0.0, 0.0, aWidth, aHeight);
                        });

                    // arrow buttons
                    bool const upEnabled = enabled && !noThumb && aScrollbar.position() != aScrollbar.minimum();
                    bool const downEnabled = enabled && !noThumb && aScrollbar.position() != aScrollbar.maximum() - aScrollbar.page();
                    auto const drawArrowButton = [&](skin_element aElement, bool aEnabled, scrollbar_element aButton, double aAngle)
                    {
                        unsigned const state = state_flags(aEnabled, hovering == aButton, clicked == aButton);
                        draw_part(aGc, buttonCtx, state, part_name(aItem, scrollbar + "/button" + std::to_string(static_cast<int>(aAngle * 100.0)), state), aItem.element_rect(aElement), scale,
                            [&](gtk::cairo_t* aCr, double aWidth, double aHeight)
                            {
                                api.gtk_render_background(buttonCtx, aCr, 0.0, 0.0, aWidth, aHeight);
                                api.gtk_render_frame(buttonCtx, aCr, 0.0, 0.0, aWidth, aHeight);
                                double const arrowSize = std::floor(std::min(aWidth, aHeight) / 2.0);
                                api.gtk_render_arrow(buttonCtx, aCr, aAngle, std::floor((aWidth - arrowSize) / 2.0), std::floor((aHeight - arrowSize) / 2.0), arrowSize);
                            });
                    };
                    drawArrowButton(skin_element::ScrollbarUpArrow, upEnabled, scrollbar_element::UpButton, vertical ? 0.0 : gtk::Pi * 1.5);
                    drawArrowButton(skin_element::ScrollbarDownArrow, downEnabled, scrollbar_element::DownButton, vertical ? gtk::Pi : gtk::Pi * 0.5);

                    // the thumb (the slider), inside its margin
                    if (!noThumb)
                    {
                        unsigned const state = state_flags(enabled, hovering == scrollbar_element::Thumb, clicked == scrollbar_element::Thumb);
                        api.gtk_style_context_set_state(sliderCtx, state);
                        gtk::GtkBorder margin{};
                        api.gtk_style_context_get_margin(sliderCtx, state, &margin);
                        rect thumb = aItem.element_rect(skin_element::ScrollbarThumb);
                        thumb.deflate(margin.left * scale, margin.top * scale, margin.right * scale, margin.bottom * scale);
                        draw_part(aGc, sliderCtx, state, part_name(aItem, scrollbar + "/slider", state), thumb, scale,
                            [&](gtk::cairo_t* aCr, double aWidth, double aHeight)
                            {
                                api.gtk_render_slider(sliderCtx, aCr, 0.0, 0.0, aWidth, aHeight, vertical ? gtk::OrientationVertical : gtk::OrientationHorizontal);
                            });
                    }
                }
                void draw_check_box(i_graphics_context& aGc, const i_skinnable_item& aItem, const button_checked_state& aCheckedState) const override
                {
                    auto const ctx = context({ "checkbutton", "check" }, dark_background(aItem));
                    if (ctx == nullptr)
                    {
                        iFallback.draw_check_box(aGc, aItem, aCheckedState);
                        return;
                    }
                    auto const& widget = aItem.as_widget();
                    scoped_units su{ widget, units::Pixels };
                    bool const enabled = widget.effectively_enabled();
                    bool const hot = enabled && !widget.ignore_mouse_events() &&
                        aItem.element_rect(skin_element::ClickableArea).contains(widget.capture_position() ? *widget.capture_position() : widget.mouse_position());
                    unsigned const state = state_flags(enabled, hot, hot && widget.capturing()) |
                        (aCheckedState == std::nullopt ? gtk::StateInconsistent : *aCheckedState ? gtk::StateChecked : 0u);
                    auto& api = *gtk::instance();
                    draw_part_true_size(aGc, ctx, state, part_name(aItem, "check", state), aItem.element_rect(skin_element::CheckBox), item_scale(aItem),
                        [&](gtk::cairo_t* aCr, double aWidth, double aHeight)
                        {
                            api.gtk_render_background(ctx, aCr, 0.0, 0.0, aWidth, aHeight);
                            api.gtk_render_frame(ctx, aCr, 0.0, 0.0, aWidth, aHeight);
                            api.gtk_render_check(ctx, aCr, 0.0, 0.0, aWidth, aHeight);
                        });
                }
                void draw_radio_button(i_graphics_context& aGc, const i_skinnable_item& aItem, const button_checked_state& aCheckedState) const override
                {
                    auto const ctx = context({ "radiobutton", "radio" }, dark_background(aItem));
                    if (ctx == nullptr)
                    {
                        iFallback.draw_radio_button(aGc, aItem, aCheckedState);
                        return;
                    }
                    auto const& widget = aItem.as_widget();
                    scoped_units su{ widget, units::Pixels };
                    bool const enabled = widget.effectively_enabled();
                    bool const hot = enabled && !widget.ignore_mouse_events() &&
                        aItem.element_rect(skin_element::ClickableArea).contains(widget.mouse_position());
                    unsigned const state = state_flags(enabled, hot, hot && widget.capturing()) |
                        (aCheckedState != std::nullopt && *aCheckedState ? gtk::StateChecked : 0u);
                    auto& api = *gtk::instance();
                    draw_part_true_size(aGc, ctx, state, part_name(aItem, "radio", state), aItem.element_rect(skin_element::RadioButton), item_scale(aItem),
                        [&](gtk::cairo_t* aCr, double aWidth, double aHeight)
                        {
                            api.gtk_render_background(ctx, aCr, 0.0, 0.0, aWidth, aHeight);
                            api.gtk_render_frame(ctx, aCr, 0.0, 0.0, aWidth, aHeight);
                            api.gtk_render_option(ctx, aCr, 0.0, 0.0, aWidth, aHeight);
                        });
                }
                void draw_tree_expander(i_graphics_context& aGc, const i_skinnable_item& aItem, bool aExpandedState) const override
                {
                    auto const ctx = context({ "treeview.view", "expander" }, dark_background(aItem));
                    if (ctx == nullptr)
                    {
                        iFallback.draw_tree_expander(aGc, aItem, aExpandedState);
                        return;
                    }
                    unsigned const state = aExpandedState ? gtk::StateChecked : gtk::StateNormal;
                    auto& api = *gtk::instance();
                    draw_part_true_size(aGc, ctx, state, part_name(aItem, "expander", state), aItem.element_rect(skin_element::TreeExpander), item_scale(aItem),
                        [&](gtk::cairo_t* aCr, double aWidth, double aHeight)
                        {
                            api.gtk_render_expander(ctx, aCr, 0.0, 0.0, aWidth, aHeight);
                        });
                }
                void draw_slider(i_graphics_context& aGc, const i_skinnable_item& aItem, slider_orientation aOrientation, double aNormalizedValue, bool aDragging, i_optional<color_or_gradient::abstract_type> const& aBarColor) const override
                {
                    bool const vertical = aOrientation == slider_orientation::Vertical;
                    bool const dark = dark_background(aItem);
                    std::string const scale = vertical ? "scale.vertical" : "scale.horizontal";
                    auto const troughCtx = context({ scale, "contents", "trough" }, dark);
                    auto const highlightCtx = troughCtx != nullptr ? context({ scale, "contents", "trough", "highlight" }, dark) : nullptr;
                    auto const sliderCtx = highlightCtx != nullptr ? context({ scale, "contents", "trough", "slider" }, dark) : nullptr;
                    if (sliderCtx == nullptr)
                    {
                        iFallback.draw_slider(aGc, aItem, aOrientation, aNormalizedValue, aDragging, aBarColor);
                        return;
                    }
                    auto const& widget = aItem.as_widget();
                    scoped_units su{ widget, units::Pixels };
                    auto& api = *gtk::instance();
                    double const scaleFactor = item_scale(aItem);
                    bool const enabled = widget.effectively_enabled();
                    rect const track = aItem.element_rect(skin_element::SliderTrack);
                    rect const thumb = aItem.element_rect(skin_element::SliderThumb);

                    // the trough at its themed thickness, centred across the slider's track
                    unsigned const troughState = state_flags(enabled, false, false);
                    auto const troughExtents = css_size(troughCtx, troughState) * scaleFactor;
                    rect trough = track;
                    if (!vertical)
                    {
                        trough.cy = std::max<dimension>(1.0, std::ceil(troughExtents.cy));
                        trough.y = std::floor(track.center().y - trough.cy / 2.0);
                    }
                    else
                    {
                        trough.cx = std::max<dimension>(1.0, std::ceil(troughExtents.cx));
                        trough.x = std::floor(track.center().x - trough.cx / 2.0);
                    }
                    draw_part(aGc, troughCtx, troughState, part_name(aItem, scale + "/trough", troughState), trough, scaleFactor,
                        [&](gtk::cairo_t* aCr, double aWidth, double aHeight)
                        {
                            api.gtk_render_background(troughCtx, aCr, 0.0, 0.0, aWidth, aHeight);
                            api.gtk_render_frame(troughCtx, aCr, 0.0, 0.0, aWidth, aHeight);
                        });

                    // the highlight (the trough up to the thumb), unless the slider's bar has a colour of its own
                    if (aBarColor == std::nullopt)
                    {
                        rect highlight = trough;
                        if (!vertical)
                            highlight.cx = std::max<dimension>(0.0, std::floor(thumb.center().x - trough.x));
                        else
                        {
                            highlight.cy = std::max<dimension>(0.0, std::floor(trough.bottom() - thumb.center().y));
                            highlight.y = trough.bottom() - highlight.cy;
                        }
                        if (highlight.cx >= 1.0 && highlight.cy >= 1.0)
                            draw_part(aGc, highlightCtx, troughState, part_name(aItem, scale + "/highlight", troughState), highlight, scaleFactor,
                                [&](gtk::cairo_t* aCr, double aWidth, double aHeight)
                                {
                                    api.gtk_render_background(highlightCtx, aCr, 0.0, 0.0, aWidth, aHeight);
                                    api.gtk_render_frame(highlightCtx, aCr, 0.0, 0.0, aWidth, aHeight);
                                });
                    }

                    // the thumb (the slider)
                    bool const hot = enabled && widget.entered() && thumb.contains(widget.mouse_position());
                    unsigned const thumbState = state_flags(enabled, hot, aDragging) | (enabled && widget.has_focus() ? gtk::StateFocused : 0u);
                    draw_part_true_size(aGc, sliderCtx, thumbState, part_name(aItem, scale + "/slider", thumbState), thumb, scaleFactor,
                        [&](gtk::cairo_t* aCr, double aWidth, double aHeight)
                        {
                            api.gtk_render_slider(sliderCtx, aCr, 0.0, 0.0, aWidth, aHeight, vertical ? gtk::OrientationVertical : gtk::OrientationHorizontal);
                        });
                }
                void draw_progress_bar(i_graphics_context& aGc, const i_skinnable_item& aItem, const i_progress_bar& aProgressBar) const override
                {
                    bool const dark = dark_background(aItem);
                    auto const troughCtx = context({ "progressbar.horizontal", "trough" }, dark);
                    auto const progressCtx = troughCtx != nullptr ? context({ "progressbar.horizontal", "trough", "progress" }, dark) : nullptr;
                    if (progressCtx == nullptr)
                    {
                        iFallback.draw_progress_bar(aGc, aItem, aProgressBar);
                        return;
                    }
                    scoped_units su{ aItem.as_widget(), units::Pixels };
                    auto& api = *gtk::instance();
                    double const scale = item_scale(aItem);
                    rect const barRect = aProgressBar.bar_rect();
                    draw_part(aGc, troughCtx, gtk::StateNormal, part_name(aItem, "progressbar/trough", gtk::StateNormal), barRect, scale,
                        [&](gtk::cairo_t* aCr, double aWidth, double aHeight)
                        {
                            api.gtk_render_background(troughCtx, aCr, 0.0, 0.0, aWidth, aHeight);
                            api.gtk_render_frame(troughCtx, aCr, 0.0, 0.0, aWidth, aHeight);
                        });
                    // the progress, inside the trough's border and padding
                    api.gtk_style_context_set_state(troughCtx, gtk::StateNormal);
                    gtk::GtkBorder border{};
                    gtk::GtkBorder padding{};
                    api.gtk_style_context_get_border(troughCtx, gtk::StateNormal, &border);
                    api.gtk_style_context_get_padding(troughCtx, gtk::StateNormal, &padding);
                    rect content = barRect;
                    content.deflate((border.left + padding.left) * scale, (border.top + padding.top) * scale, (border.right + padding.right) * scale, (border.bottom + padding.bottom) * scale);
                    auto const range = aProgressBar.maximum() - aProgressBar.minimum();
                    auto const fraction = range > 0.0 ? std::clamp((aProgressBar.value() - aProgressBar.minimum()) / range, 0.0, 1.0) : 0.0;
                    rect const fill{ content.top_left(), size{ std::floor(content.cx * fraction), content.cy } };
                    if (fill.cx >= 1.0 && fill.cy >= 1.0)
                        draw_part(aGc, progressCtx, gtk::StateNormal, part_name(aItem, "progressbar/progress", gtk::StateNormal), fill, scale,
                            [&](gtk::cairo_t* aCr, double aWidth, double aHeight)
                            {
                                api.gtk_render_background(progressCtx, aCr, 0.0, 0.0, aWidth, aHeight);
                                api.gtk_render_frame(progressCtx, aCr, 0.0, 0.0, aWidth, aHeight);
                            });
                }
                void draw_separators(i_graphics_context& aGc, const i_skinnable_item& aItem, const i_layout& aLayout) const override
                {
                    iFallback.draw_separators(aGc, aItem, aLayout);
                }
                void draw_window_frame(i_graphics_context& aGc, const i_skinnable_item& aItem, const rect& aFrameRect, dimension aFrameWidth, const color& aFrameColor) const override
                {
                    iFallback.draw_window_frame(aGc, aItem, aFrameRect, aFrameWidth, aFrameColor);
                }
                void draw_title_bar(i_graphics_context& aGc, const i_skinnable_item& aItem, const title_bar_appearance& aAppearance) const override
                {
                    iFallback.draw_title_bar(aGc, aItem, aAppearance);
                }
                void draw_title_bar_button(i_graphics_context& aGc, const i_skinnable_item& aItem, widget_part_e aButton, const title_bar_appearance& aAppearance) const override
                {
                    iFallback.draw_title_bar_button(aGc, aItem, aButton, aAppearance);
                }
                void draw_container_frame(i_graphics_context& aGc, const i_skinnable_item& aItem, const rect& aFrameRect, dimension aFrameWidth, const color& aFrameColor, const color& aInnerFrameColor) const override
                {
                    iFallback.draw_container_frame(aGc, aItem, aFrameRect, aFrameWidth, aFrameColor, aInnerFrameColor);
                }
                void draw_tab_page_join(i_graphics_context& aGc, const i_skinnable_item& aItem, const i_tab_page& aPage, dimension aFrameWidth, const color& aInnerFrameColor) const override
                {
                    iFallback.draw_tab_page_join(aGc, aItem, aPage, aFrameWidth, aInnerFrameColor);
                }
            private:
                // the style context of the last node of a CSS node path in the theme's normal or dark variant (each node's parent
                // the context of the path before it); null if GTK isn't available
                gtk::GtkStyleContext* context(node_path const& aPath, bool aDark) const
                {
                    auto const gtk = gtk::instance();
                    if (gtk == nullptr)
                        return nullptr;
                    load_theme();
                    gtk::GtkStyleContext* parent = nullptr;
                    std::string key = aDark ? "dark" : "light";
                    for (std::size_t n = 0u; n < aPath.size(); ++n)
                    {
                        key += "/" + aPath[n];
                        auto existing = iContexts.find(key);
                        if (existing == iContexts.end())
                        {
                            auto const path = gtk->gtk_widget_path_new();
                            for (std::size_t i = 0u; i <= n; ++i)
                            {
                                // (name.class.class...)
                                std::vector<std::string> parts;
                                std::string::size_type start = 0u;
                                for (std::string::size_type dot; (dot = aPath[i].find('.', start)) != std::string::npos; start = dot + 1u)
                                    parts.push_back(aPath[i].substr(start, dot - start));
                                parts.push_back(aPath[i].substr(start));
                                auto const position = gtk->gtk_widget_path_append_type(path, gtk::TypeNone);
                                gtk->gtk_widget_path_iter_set_object_name(path, position, parts[0].c_str());
                                for (std::size_t c = 1u; c < parts.size(); ++c)
                                    gtk->gtk_widget_path_iter_add_class(path, position, parts[c].c_str());
                            }
                            auto const newContext = gtk->gtk_style_context_new();
                            gtk->gtk_style_context_set_path(newContext, path);
                            gtk->gtk_widget_path_unref(path);
                            if (parent != nullptr)
                                gtk->gtk_style_context_set_parent(newContext, parent);
                            if (auto const theme = aDark ? iDarkTheme : iLightTheme)
                                gtk->gtk_style_context_add_provider(newContext, theme, gtk::ProviderPriorityApplication);
                            if (iAccent != nullptr)
                                gtk->gtk_style_context_add_provider(newContext, iAccent, gtk::ProviderPriorityApplication + 1u);
                            existing = iContexts.emplace(key, newContext).first;
                        }
                        parent = existing->second;
                    }
                    return parent;
                }
                // the user's theme in its normal and dark variants (or GTK's high contrast themes if the desktop asks for high
                // contrast) and a style sheet giving Adwaita (whose GTK 3 version has no accent colour of its own) the desktop's
                // accent colour
                void load_theme() const
                {
                    if (iThemeLoaded)
                        return;
                    iThemeLoaded = true;
                    auto& api = *gtk::instance();
                    auto const settings = read_desktop_settings(api);
                    std::string themeName = "Adwaita";
                    if (auto const gtkSettings = api.gtk_settings_get_default())
                    {
                        char* name = nullptr;
                        api.g_object_get(gtkSettings, "gtk-theme-name", &name, nullptr);
                        if (name != nullptr)
                        {
                            themeName = name;
                            api.g_free(name);
                        }
                    }
                    // (a "-dark" theme name is its dark variant: the normal variant is wanted for light backgrounds)
                    if (themeName.size() > 5u && themeName.compare(themeName.size() - 5u, 5u, "-dark") == 0)
                        themeName.erase(themeName.size() - 5u);
                    if (settings.highContrast)
                    {
                        iLightTheme = api.gtk_css_provider_get_named("HighContrast", nullptr);
                        iDarkTheme = api.gtk_css_provider_get_named("HighContrastInverse", nullptr);
                    }
                    else
                    {
                        iLightTheme = api.gtk_css_provider_get_named(themeName.c_str(), nullptr);
                        iDarkTheme = api.gtk_css_provider_get_named(themeName.c_str(), "dark");
                        // (a theme without a dark variant, or not installed, gets GTK's fallback, which is light: Adwaita's dark
                        // variant instead)
                        if (iDarkTheme == nullptr || !dark_theme(iDarkTheme))
                            iDarkTheme = api.gtk_css_provider_get_named("Adwaita", "dark");
                    }
                    if (settings.accentColor && !settings.highContrast && themeName.compare(0u, 7u, "Adwaita") == 0)
                    {
                        auto const css_color = [](color const& aColor)
                        {
                            char result[32];
                            std::snprintf(result, sizeof(result), "rgb(%d,%d,%d)", static_cast<int>(aColor.red()), static_cast<int>(aColor.green()), static_cast<int>(aColor.blue()));
                            return std::string{ result };
                        };
                        auto const accent = css_color(*settings.accentColor);
                        auto const border = css_color(settings.accentColor->darker(0x20));
                        std::string const css =
                            "check:checked:not(:disabled), check:indeterminate:not(:disabled), radio:checked:not(:disabled), radio:indeterminate:not(:disabled) "
                            "{ background-image: image(" + accent + "); border-color: " + border + "; color: #ffffff; }\n"
                            "scale highlight:not(:disabled), progressbar progress "
                            "{ background-image: image(" + accent + "); background-color: " + accent + "; border-color: " + border + "; }\n";
                        iAccent = api.gtk_css_provider_new();
                        gtk::GError* error = nullptr;
                        if (api.gtk_css_provider_load_from_data(iAccent, css.c_str(), -1, &error) == 0)
                        {
                            if (error != nullptr)
                                api.g_error_free(error);
                            api.g_object_unref(iAccent);
                            iAccent = nullptr;
                        }
                    }
                }
                // true if a theme is dark (a button's text is light)
                static bool dark_theme(gtk::GtkCssProvider* aTheme)
                {
                    auto& api = *gtk::instance();
                    auto const path = api.gtk_widget_path_new();
                    api.gtk_widget_path_iter_set_object_name(path, api.gtk_widget_path_append_type(path, gtk::TypeNone), "button");
                    auto const probe = api.gtk_style_context_new();
                    api.gtk_style_context_set_path(probe, path);
                    api.gtk_widget_path_unref(path);
                    api.gtk_style_context_add_provider(probe, aTheme, gtk::ProviderPriorityApplication);
                    gtk::GdkRGBA text{};
                    api.gtk_style_context_get_color(probe, gtk::StateNormal, &text);
                    api.g_object_unref(probe);
                    return color{ text.red, text.green, text.blue }.light();
                }
                gtk::GtkStyleContext* push_button_context(const i_skinnable_item& aItem, const i_push_button& aButton, const push_button_appearance& aAppearance) const
                {
                    // Only buttons that look like push buttons are drawn natively; a button whose appearance has been customized
                    // (e.g. a colour swatch) keeps its neoGFX look.
                    bool const pushButtonStyle = aButton.style() == push_button_style::Normal || aButton.style() == push_button_style::ButtonBox;
                    if (!pushButtonStyle || aAppearance.customized)
                        return nullptr;
                    return context({ aAppearance.defaultButton ? "button.default" : "button" }, dark_background(aItem));
                }
                static unsigned push_button_state(const i_skinnable_item& aItem, const i_push_button& aButton)
                {
                    auto const& widget = aItem.as_widget();
                    bool const enabled = widget.effectively_enabled();
                    bool const hot = enabled && widget.entered();
                    bool const pressed = enabled && ((hot && widget.capturing()) || aButton.is_checked());
                    return state_flags(enabled, hot, pressed) |
                        (enabled && aButton.is_checked() ? gtk::StateChecked : 0u) |
                        (enabled && widget.has_focus() ? gtk::StateFocused : 0u);
                }
                static std::string part_name(const i_skinnable_item& aItem, std::string const& aPart, unsigned aState)
                {
                    return (dark_background(aItem) ? "dark/" : "light/") + aPart + "#" + std::to_string(aState);
                }
                // a node's size: its CSS minimum size, border and padding (CSS pixels)
                size css_size(gtk::GtkStyleContext* aContext, unsigned aState) const
                {
                    auto& api = *gtk::instance();
                    api.gtk_style_context_set_state(aContext, aState);
                    int minWidth = 0;
                    int minHeight = 0;
                    api.gtk_style_context_get(aContext, aState, "min-width", &minWidth, "min-height", &minHeight, nullptr);
                    gtk::GtkBorder border{};
                    gtk::GtkBorder padding{};
                    api.gtk_style_context_get_border(aContext, aState, &border);
                    api.gtk_style_context_get_padding(aContext, aState, &padding);
                    return size{
                        static_cast<dimension>(minWidth + border.left + border.right + padding.left + padding.right),
                        static_cast<dimension>(minHeight + border.top + border.bottom + padding.top + padding.bottom) };
                }
                void draw_part(i_graphics_context& aGc, gtk::GtkStyleContext* aContext, unsigned aState, std::string const& aPart, rect const& aRect, double aScale,
                    std::function<void(gtk::cairo_t*, double, double)> const& aRender) const
                {
                    auto const cx = static_cast<std::int32_t>(std::ceil(aRect.cx));
                    auto const cy = static_cast<std::int32_t>(std::ceil(aRect.cy));
                    if (cx <= 0 || cy <= 0)
                        return;
                    part_key const key{ aPart, cx, cy, static_cast<std::int32_t>(std::round(aScale * 100.0)) };
                    auto existing = iParts.find(key);
                    if (existing == iParts.end())
                    {
                        if (iParts.size() >= kMaxCachedParts)
                            iParts.clear();
                        auto& api = *gtk::instance();
                        // drawn in CSS pixels (cairo's device scale) with the theme's images at the next whole scale
                        auto const surface = api.cairo_image_surface_create(gtk::CairoFormatArgb32, cx, cy);
                        api.cairo_surface_set_device_scale(surface, aScale, aScale);
                        auto const cr = api.cairo_create(surface);
                        api.gtk_style_context_set_scale(aContext, static_cast<int>(std::ceil(aScale)));
                        api.gtk_style_context_set_state(aContext, aState);
                        aRender(cr, cx / aScale, cy / aScale);
                        api.cairo_destroy(cr);
                        api.cairo_surface_flush(surface);
                        // cairo's ARGB32 is premultiplied (B, G, R, A in memory): textures want straight alpha, rows bottom up
                        // (as texture set_pixels expects (cf. set_pixels(const i_image&)))
                        auto const data = api.cairo_image_surface_get_data(surface);
                        auto const stride = static_cast<std::size_t>(api.cairo_image_surface_get_stride(surface));
                        std::vector<std::uint32_t> bgra(static_cast<std::size_t>(cx) * static_cast<std::size_t>(cy));
                        for (std::int32_t y = 0; y < cy; ++y)
                        {
                            auto const row = reinterpret_cast<std::uint32_t const*>(data + stride * static_cast<std::size_t>(y));
                            auto const destination = &bgra[static_cast<std::size_t>(cy - 1 - y) * static_cast<std::size_t>(cx)];
                            for (std::int32_t x = 0; x < cx; ++x)
                            {
                                auto const pixel = row[x];
                                auto const alpha = pixel >> 24u;
                                std::uint32_t result = 0u;
                                if (alpha != 0u)
                                {
                                    result = alpha << 24u;
                                    for (std::uint32_t shift = 0u; shift <= 16u; shift += 8u)
                                        result |= std::min<std::uint32_t>(255u, ((pixel >> shift) & 0xFFu) * 255u / alpha) << shift;
                                }
                                destination[x] = result;
                            }
                        }
                        api.cairo_surface_destroy(surface);
                        auto texture = service<i_texture_manager>().create_texture(
                            size{ static_cast<dimension>(cx), static_cast<dimension>(cy) }, 1.0, texture_sampling::Nearest, texture_data_format::BGRA);
                        texture->set_pixels(rect{ point{}, size{ static_cast<dimension>(cx), static_cast<dimension>(cy) } }, bgra.data(), static_cast<std::uint32_t>(cx));
                        existing = iParts.emplace(key, texture).first;
                    }
                    aGc.draw_texture(rect{ point{ std::floor(aRect.x), std::floor(aRect.y) }, size{ static_cast<dimension>(cx), static_cast<dimension>(cy) } }, *existing->second);
                }
                // drawn at the node's (CSS) size, centred in the rectangle
                void draw_part_true_size(i_graphics_context& aGc, gtk::GtkStyleContext* aContext, unsigned aState, std::string const& aPart, rect const& aRect, double aScale,
                    std::function<void(gtk::cairo_t*, double, double)> const& aRender) const
                {
                    auto const extents = (css_size(aContext, aState) * aScale).ceil();
                    if (extents.cx <= 0.0 || extents.cy <= 0.0)
                    {
                        draw_part(aGc, aContext, aState, aPart, aRect, aScale, aRender);
                        return;
                    }
                    point const position{
                        std::floor(aRect.x + (aRect.cx - extents.cx) / 2.0),
                        std::floor(aRect.y + (aRect.cy - extents.cy) / 2.0) };
                    draw_part(aGc, aContext, aState, aPart, rect{ position, extents }, aScale, aRender);
                }
                void reset() const
                {
                    iParts.clear();
                    if (auto const gtk = gtk::instance())
                    {
                        for (auto const& c : iContexts)
                            gtk->g_object_unref(c.second);
                        if (iAccent != nullptr)
                            gtk->g_object_unref(iAccent);
                    }
                    iContexts.clear();
                    iAccent = nullptr;
                    // (named theme providers are GTK's)
                    iLightTheme = nullptr;
                    iDarkTheme = nullptr;
                    iThemeLoaded = false;
                }
            private:
                default_skin iFallback;
                mutable bool iThemeLoaded = false;
                mutable gtk::GtkCssProvider* iLightTheme = nullptr;
                mutable gtk::GtkCssProvider* iDarkTheme = nullptr;
                mutable gtk::GtkCssProvider* iAccent = nullptr;
                mutable std::map<std::string, gtk::GtkStyleContext*> iContexts;
                mutable std::map<part_key, ref_ptr<i_texture>> iParts;
            };
        }
    }

    namespace native
    {
        i_skin& native_skin()
        {
            // Intentionally never destroyed: the skin caches textures, which must not be released during static destruction
            // (after the rendering engine has gone).
            static native::linux::skin* const sNativeSkin = new native::linux::skin{};
            return *sNativeSkin;
        }
    }
}
