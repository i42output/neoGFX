// i_graph_widget.hpp
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
#pragma once

#include <neogfx/neogfx.hpp>

#include <neogfx/gui/widget/i_widget.hpp>

namespace neogfx
{
    enum class graph_widget_type
    {
        Line
    };

    enum class graph_widget_flags : std::uint32_t
    {
        None = 0x00,
    };

    inline constexpr graph_widget_flags operator|(graph_widget_flags aLhs, graph_widget_flags aRhs)
    {
        return static_cast<graph_widget_flags>(static_cast<std::uint32_t>(aLhs) | static_cast<std::uint32_t>(aRhs));
    }

    inline constexpr graph_widget_flags operator&(graph_widget_flags aLhs, graph_widget_flags aRhs)
    {
        return static_cast<graph_widget_flags>(static_cast<std::uint32_t>(aLhs) & static_cast<std::uint32_t>(aRhs));
    }

    inline constexpr graph_widget_flags operator~(graph_widget_flags aLhs)
    {
        return static_cast<graph_widget_flags>(~static_cast<std::uint32_t>(aLhs));
    }

    template <typename X = double, typename Y = double>
    class i_graph_datum
    {
    public:
        using abstract_type = i_graph_datum;
    public:
        using x_type = X;
        using y_type = Y;
    public:
        [[nodiscard]] virtual x_type const& x() const = 0;
        [[nodiscard]] virtual y_type const& y() const = 0;
        virtual void set_x(x_type const& aX) = 0;
        virtual void set_y(y_type const& aY) = 0;
    public:
        virtual ~i_graph_datum() = default;
    };

    template <typename X = double, typename Y = double>
    class i_graph_series : public i_reference_counted
    {
    public:
        using abstract_type = i_graph_series;
    public:
        using x_type = X;
        using y_type = Y;
        using i_datum = i_graph_datum<X, Y>;
        using index_type = typename i_vector<i_datum>::size_type;
    public:
        declare_event(data_changed);
        declare_event(visibility_changed);
        declare_event(appearance_changed);
    public:
        [[nodiscard]] virtual bool no_data() const = 0;
        [[nodiscard]] virtual i_vector<i_datum> const& data() const = 0;
        virtual void set_data(i_vector<i_datum> const& aData) = 0;
        virtual void push_back(i_datum const& aDatum) = 0;
        virtual void insert(index_type aIndex, i_datum const& aDatum) = 0;
        virtual void erase(index_type aIndex) = 0;
    public:
        [[nodiscard]] virtual x_type const& x_min() const = 0;
        [[nodiscard]] virtual x_type const& x_max() const = 0;
        [[nodiscard]] virtual y_type const& y_min() const = 0;
        [[nodiscard]] virtual y_type const& y_max() const = 0;
    public:
        [[nodiscard]] virtual i_optional<i_string> const& name() const = 0;
        virtual void set_name(i_optional<i_string> const& aName) = 0;
    public:
        [[nodiscard]] virtual bool visible() const = 0;
        virtual void set_visible(bool aVisible) = 0;
    public:
        virtual void start_update() = 0;
        virtual void end_update() = 0;
    public:
        virtual ~i_graph_series() = default;
    };

    template <typename X, typename Y>
    class scoped_graph_series_update
    {
    public:
        scoped_graph_series_update(i_graph_series<X, Y>& aSeries) :
            iSeries{ aSeries }
        {
            iSeries.start_update();
        }
        ~scoped_graph_series_update()
        {
            iSeries.end_update();
        }
    private:
        i_graph_series<X, Y>& iSeries;
    };

    class i_graph_series_appearance : public i_reference_counted
    {
    public:
        using abstract_type = i_graph_series_appearance;
    public:
        declare_event(changed);
    public:
        [[nodiscard]] virtual optional_pen const& pen() const = 0;
        virtual void set_pen(optional_pen const& aPen) = 0;
        [[nodiscard]] virtual optional_color_or_gradient const& fill() const = 0;
        virtual void set_fill(optional_color_or_gradient const& aFill) = 0;
    };

    class graph_series_appearance : public reference_counted<i_graph_series_appearance>
    {
    public:
        using abstract_type = i_graph_series_appearance;
    public:
        define_declared_event(Changed, changed);
    public:
        graph_series_appearance(i_graph_series_appearance const& aOther) :
            iPen{ aOther.pen() }, iFill{ aOther.fill() }
        {
        }
        graph_series_appearance(optional_pen const& aPen = {}, optional_color_or_gradient const& aFill = {}) :
            iPen{ aPen }, iFill{ aFill }
        {
        }
    public:
        [[nodiscard]] optional_pen const& pen() const final
        {
            return iPen;
        }
        void set_pen(optional_pen const& aPen) final
        {
            if (iPen != aPen)
            {
                iPen = aPen;
                Changed();
            }
        }
        [[nodiscard]] optional_color_or_gradient const& fill() const final
        {
            return iFill;
        }
        void set_fill(optional_color_or_gradient const& aFill) final
        {
            if (iFill != aFill)
            {
                iFill = aFill;
                Changed();
            }
        }
    private:
        optional_pen iPen;
        optional_color_or_gradient iFill;
    };

    template <typename T = double>
    class i_graph_axis : public i_reference_counted
    {
    public:
        using abstract_type = i_graph_axis;
    public:
        using datum_component_type = T;
    public:
        declare_event(changed);
    public:
        [[nodiscard]] virtual i_string const& title() const = 0;
        virtual void set_title(i_string const& aTitle = string{}) = 0;
    public:
        [[nodiscard]] virtual bool has_minor_tick() const = 0;
        [[nodiscard]] virtual bool has_major_tick() const = 0;
        [[nodiscard]] virtual datum_component_type const& minor_tick() const = 0;
        [[nodiscard]] virtual datum_component_type const& major_tick() const = 0;
        virtual void set_minor_tick(datum_component_type const& aTick) = 0;
        virtual void set_major_tick(datum_component_type const& aTick) = 0;
        virtual void clear_minor_tick() = 0;
        virtual void clear_major_tick() = 0;
    public:
        [[nodiscard]] virtual neogfx::font const& font() const = 0;
        virtual void set_font(optional_font const& aFont = {}) = 0;
    };

    template <typename X = double, typename Y = double>
    class i_graph_widget;
        
    enum class graph_rendering_element
    {
        AxisLabel,
        DatumLabel,
        DatumHoverLabel
    };

    template <typename X = double, typename Y = double>
    class i_graph_renderer : public i_reference_counted
    {
    public:
        using abstract_type = i_graph_renderer;
    public:
        using x_type = X;
        using y_type = Y;
        using i_series = i_graph_series<x_type, y_type>;
        // render
    public:
        virtual void render(i_graph_widget<x_type, y_type> const& aWidget, i_graphics_context& aGc) const = 0;
        virtual void render_plot(i_graph_widget<x_type, y_type> const& aWidget, i_graphics_context& aGc) const = 0;
        virtual void render_series(i_graph_widget<x_type, y_type> const& aWidget, i_graphics_context& aGc, i_series const& aSeries) const = 0;
        virtual void render_x_axis(i_graph_widget<x_type, y_type> const& aWidget, i_graphics_context& aGc) const = 0;
        virtual void render_y_axis(i_graph_widget<x_type, y_type> const& aWidget, i_graphics_context& aGc) const = 0;
        virtual void render_x_label(x_type const& aX, i_graph_widget<x_type, y_type> const& aWidget, i_graphics_context& aGc, point const& aLabelOrigin, graph_rendering_element) const = 0;
        virtual void render_y_label(y_type const& aY, i_graph_widget<x_type, y_type> const& aWidget, i_graphics_context& aGc, point const& aLabelOrigin, graph_rendering_element) const = 0;
        // text
    public:
        virtual void x_to_text(x_type const& aX, i_string& aText) const = 0;
        virtual void y_to_text(y_type const& aY, i_string& aText) const = 0;
        // metrics
    public:
        [[nodiscard]] virtual rect plot_area(i_graph_widget<x_type, y_type> const& aWidget, i_graphics_context& aGc) const = 0;
        [[nodiscard]] virtual rect x_axis_area(i_graph_widget<x_type, y_type> const& aWidget, i_graphics_context& aGc) const = 0;
        [[nodiscard]] virtual rect y_axis_area(i_graph_widget<x_type, y_type> const& aWidget, i_graphics_context& aGc) const = 0;
        [[nodiscard]] virtual size x_label_extents(x_type const& aX, i_graph_widget<x_type, y_type> const& aWidget, i_graphics_context& aGc, graph_rendering_element) const = 0;
        [[nodiscard]] virtual size y_label_extents(y_type const& aY, i_graph_widget<x_type, y_type> const& aWidget, i_graphics_context& aGc, graph_rendering_element) const = 0;
        [[nodiscard]] virtual scalar x_tick_length_px(i_graph_widget<x_type, y_type> const& aWidget) const = 0;
        virtual void set_x_tick_length_px(scalar aLength_px) = 0;
        virtual void clear_x_tick_length_px() = 0;
        [[nodiscard]] virtual scalar y_tick_length_px(i_graph_widget<x_type, y_type> const& aWidget) const = 0;
        virtual void set_y_tick_length_px(scalar aLength_px) = 0;
        virtual void clear_y_tick_length_px() = 0;
        // helpers
    public:
        string x_to_text(x_type const& aX) const
        {
            string result;
            x_to_text(aX, result);
        }
        string y_to_text(y_type const& aY) const
        {
            string result;
            y_to_text(aY, result);
        }
    };

    template <typename X, typename Y>
    class i_graph_widget : public i_widget
    {
    public:
        using abstract_type = i_graph_widget;
    public:
        using x_type = X;
        using y_type = Y;
        using i_datum = i_graph_datum<x_type, y_type>;
        using i_series = i_graph_series<x_type, y_type>;
        using series_container = i_vector<i_ref_ptr<i_series>>;
        using series_index = typename series_container::size_type;
        using i_renderer = i_graph_renderer<x_type, y_type>;
    public:
        [[nodiscard]] virtual graph_widget_type type() const = 0;
        virtual void set_type(graph_widget_type aType) = 0;
        [[nodiscard]] virtual graph_widget_flags flags() const = 0;
        virtual void set_flags(graph_widget_flags aFlags) = 0;
    public:
        [[nodiscard]] virtual series_index series_count() const = 0;
        [[nodiscard]] virtual series_container const& series() const = 0;
        [[nodiscard]] virtual i_series const& series(series_index aIndex) const = 0;
        [[nodiscard]] virtual i_series& series(series_index aIndex) = 0;
        virtual i_series& add_series(i_optional<i_string> const& aName = optional<string>{}, i_ref_ptr<i_graph_series_appearance> const& aAppearance = ref_ptr<graph_series_appearance>{}) = 0;
        virtual void erase_series(series_index aIndex) = 0;
    public:
        [[nodiscard]] virtual x_type const& x_min(bool aIgnoreVisibility = false) const = 0;
        [[nodiscard]] virtual x_type const& x_max(bool aIgnoreVisibility = false) const = 0;
        [[nodiscard]] virtual y_type const& y_min(bool aIgnoreVisibility = false) const = 0;
        [[nodiscard]] virtual y_type const& y_max(bool aIgnoreVisibility = false) const = 0;
        [[nodiscard]] virtual x_type const& x_min(series_index aIndex) const = 0;
        [[nodiscard]] virtual x_type const& x_max(series_index aIndex) const = 0;
        [[nodiscard]] virtual y_type const& y_min(series_index aIndex) const = 0;
        [[nodiscard]] virtual y_type const& y_max(series_index aIndex) const = 0;
    public:
        [[nodiscard]] virtual i_graph_axis<x_type>& x_axis() const = 0;
        virtual void set_x_axis(i_ref_ptr<i_graph_axis<x_type>> const& aAxis = ref_ptr<i_graph_axis<x_type>>{}) = 0;
        [[nodiscard]] virtual i_graph_axis<y_type>& y_axis() const = 0;
        virtual void set_y_axis(i_ref_ptr<i_graph_axis<y_type>> const& aAxis = ref_ptr<i_graph_axis<y_type>>{}) = 0;
    public:
        [[nodiscard]] virtual i_renderer const& renderer() const = 0;
        virtual void set_renderer(i_renderer& aRenderer) = 0;
        virtual void set_default_renderer() = 0;
    public:
        [[nodiscard]] virtual bool has_view_transform_to_px() const = 0;
        [[nodiscard]] virtual mat33 view_transform_to_px() const = 0;
        virtual void set_view_transform_to_px(mat33 const& aTransform) = 0;
        virtual void clear_view_transform_to_px() = 0;
        virtual void get_view(x_type& xMin, x_type& xMax, y_type& yMin, y_type& yMax) const = 0;
        virtual void set_default_view() = 0;
        virtual void set_view(series_index aIndex) = 0;
        virtual void set_view(x_type const& xMin, x_type const& xMax, y_type const& yMin, y_type const& yMax) = 0;
    public:
        [[nodiscard]] virtual i_graph_series_appearance const& default_series_appearance() const = 0;
        virtual void set_default_series_appearance(i_optional<i_graph_series_appearance> const& aSeriesAppearance = optional<graph_series_appearance>{}) = 0;
        // helpers
    public:
        [[nodiscard]] i_series& add_series(std::string const& aName)
        {
            return add_series(optional<string>{ aName });
        }
    public:
        void set_scale_to_px(scalar aScaleX, scalar aScaleY)
        {
            mat33 transform = mat33::identity();
            transform[0][0] = aScaleX;
            transform[1][1] = aScaleY;
            set_view_transform_to_px(transform);
        }
    };
}