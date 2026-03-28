// graph_widget.hpp
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

#include <neogfx/gui/widget/widget.hpp>
#include <neogfx/gui/widget/widget.ipp>
#include <neogfx/gui/widget/i_graph_widget.hpp>

namespace neogfx
{
    struct no_graph_series_data : std::logic_error { no_graph_series_data() : std::logic_error{ "neogfx::no_graph_series_data" } {} };
    struct unknown_graph_datum_type : std::logic_error { unknown_graph_datum_type() : std::logic_error{ "neogfx::unknown_graph_datum_type" } {} };
    struct graph_axis_undefined : std::logic_error { graph_axis_undefined() : std::logic_error{ "neogfx::graph_axis_undefined" } {} };

    template <typename X = double, typename Y = double>
    class graph_datum : public i_graph_datum<abstract_t<X>, abstract_t<Y>>
    {
    public:
        using x_abstract_type = abstract_t<X>;
        using y_abstract_type = abstract_t<Y>;
        using abstract_type = i_graph_datum<x_abstract_type, y_abstract_type>;
    public:
        using x_type = X;
        using y_type = Y;
    public:
        graph_datum() :
            iX{}, iY{}
        {
        }
        graph_datum(graph_datum const& aOther) :
            iX{ aOther.iX }, iY{ aOther.iY }
        {
        }
        graph_datum(abstract_type const& aOther) :
            iX{ aOther.x() }, iY{ aOther.y() }
        {
        }
        graph_datum(x_type const& aX, y_type const& aY) :
            iX{ aX }, iY{ aY }
        {
        }
    public:
        [[nodiscard]] x_type const& x() const final
        {
            return iX;
        }
        [[nodiscard]] y_type const& y() const final
        {
            return iY;
        }
        void set_x(x_abstract_type const& aX) final
        {
            iX = aX;
        }
        void set_y(y_abstract_type const& aY) final
        {
            iY = aY;
        }
    private:
        x_type iX;
        y_type iY;
    };

    template <typename X = double, typename Y = double>
    class graph_series : public reference_counted<i_graph_series<abstract_t<X>, abstract_t<Y>>>
    {
    public:
        using x_abstract_type = abstract_t<X>;
        using y_abstract_type = abstract_t<Y>;
        using abstract_type = i_graph_series<x_abstract_type, y_abstract_type>;
    public:
        using x_type = X;
        using y_type = Y;
        using i_datum = i_graph_datum<x_abstract_type, y_abstract_type>;
        using datum = graph_datum<x_type, y_type>;
        using index_type = typename vector<datum>::size_type;
    public:
        define_declared_event(DataChanged, data_changed);
        define_declared_event(VisibilityChanged, visibility_changed);
        define_declared_event(AppearanceChanged, appearance_changed);
    public:
        graph_series(i_optional<i_string> const& aName = optional<string>{}) : 
            iName{ aName }
        {
        }
        graph_series(abstract_type const& aOther) : 
            iData{ aOther.data() },
            iName{ aOther.name() },
            iVisible{ aOther.visible() }
        {
        }
    public:
        [[nodiscard]] bool no_data() const final
        {
            return iData.empty();
        }
        [[nodiscard]] vector<datum> const& data() const final
        {
            return iData;
        }
        void set_data(i_vector<i_datum> const& aData) final
        {
            iData = aData;
            if (!iUpdating)
            {
                invalidate_cache();
                DataChanged();
            }
        }
        void push_back(i_datum const& aDatum) final
        {
            iData.as_std_vector().push_back(aDatum);
            if (!iUpdating)
            {
                invalidate_cache();
                DataChanged();
            }
        }
        void insert(index_type aIndex, i_datum const& aDatum) final
        {
            iData.as_std_vector().insert(std::next(iData.as_std_vector().begin(), aIndex), aDatum);
            if (!iUpdating)
            {
                invalidate_cache();
                DataChanged();
            }
        }
        void erase(index_type aIndex) final
        {
            iData.as_std_vector().erase(std::next(iData.as_std_vector().begin(), aIndex));
            if (!iUpdating)
            {
                invalidate_cache();
                DataChanged();
            }
        }
    public:
        [[nodiscard]] x_type const& x_min() const final
        {
            if (no_data())
                throw no_graph_series_data();
            if (iXMin != nullptr)
                return *iXMin;
            if constexpr (std::is_arithmetic_v<x_type>)
            {
                x_type const* min = &iData.front().x();
                for (auto const& d : iData)
                    if (d.x() < *min)
                        min = &d.x();
                iXMin = min;
            }
            else
                iXMin = &iData.front().x();
            return *iXMin;
        }
        [[nodiscard]] x_type const& x_max() const final
        {
            if (no_data())
                throw no_graph_series_data();
            if (iXMax != nullptr)
                return *iXMax;
            if constexpr (std::is_arithmetic_v<x_type>)
            {
                x_type const* max = &iData.front().x();
                for (auto const& d : iData)
                    if (d.x() >= *max)
                        max = &d.x();
                iXMax = max;
            }
            else
                iXMax = &iData.back().x();
            return *iXMax;
        }
        [[nodiscard]] y_type const& y_min() const final
        {

            if (no_data())
                throw no_graph_series_data();
            if (iYMin != nullptr)
                return *iYMin;
            if constexpr (std::is_arithmetic_v<y_type>)
            {
                y_type const* min = &iData.front().y();
                for (auto const& d : iData)
                    if (d.y() < *min)
                        min = &d.y();
                iYMin = min;
            }
            else
                iYMin = &iData.front().y();
            return *iYMin;
        }
        [[nodiscard]] y_type const& y_max() const final
        {
            if (no_data())
                throw no_graph_series_data();
            if (iYMax != nullptr)
                return *iYMax;
            if constexpr (std::is_arithmetic_v<y_type>)
            {
                y_type const* max = &iData.front().y();
                for (auto const& d : iData)
                    if (d.y() >= *max)
                        max = &d.y();
                iYMax = max;
            }
            else
                iYMax = &iData.back().y();
            return *iYMax;
        }
    public:
        [[nodiscard]] i_optional<i_string> const& name() const final
        {
            return iName;
        }
        void set_name(i_optional<i_string> const& aName) final
        {
            if (iName != aName)
            {
                iName = aName;
                AppearanceChanged();
            }
        }
    public:
        [[nodiscard]] bool visible() const final
        {
            return iVisible;
        }
        void set_visible(bool aVisible) final
        {
            if (iVisible != aVisible)
            {
                iVisible = aVisible;
                VisibilityChanged();
            }
        }
    public:
        void start_update() final
        {
            iUpdating = true;
            invalidate_cache();
        }
        void end_update() final
        {
            iUpdating = false;
            invalidate_cache();
            DataChanged();
        }
    private:
        void invalidate_cache()
        {
            iXMin = nullptr;
            iYMin = nullptr;
            iXMax = nullptr;
            iYMax = nullptr;
        }
    private:
        bool iUpdating = false;
        vector<datum> iData;
        optional<string> iName;
        bool iVisible = true;
        mutable x_type const* iXMin = nullptr;
        mutable x_type const* iXMax = nullptr;
        mutable y_type const* iYMin = nullptr;
        mutable y_type const* iYMax = nullptr;
    };

    template <typename T = double>
    class graph_axis : public reference_counted<i_graph_axis<abstract_t<T>>>
    {
    public:
        using abstract_datum_component_type = abstract_t<T>;
        using abstract_type = i_graph_axis<abstract_t<T>>;
    public:
        using datum_component_type = T;
    public:
        define_declared_event(Changed, changed);
    public:
        [[nodiscard]] string const& title() const final
        {
            return iTitle;
        }
        void set_title(i_string const& aTitle = string{}) final
        {
            if (iTitle != aTitle)
            {
                iTitle = aTitle;
                Changed();
            }
        }
    public:
        [[nodiscard]] bool has_minor_tick() const final
        {
            return iMinorTick.has_value();
        }
        [[nodiscard]] bool has_major_tick() const final
        {
            return iMajorTick.has_value();
        }
        [[nodiscard]] datum_component_type const& minor_tick() const final
        {
            return iMinorTick.value();
        }
        [[nodiscard]] datum_component_type const& major_tick() const final
        {
            return iMajorTick.value();
        }
        void set_minor_tick(abstract_datum_component_type const& aTick) final
        {
            if (iMinorTick != aTick)
            {
                iMinorTick = aTick;
                Changed();
            }
        }
        void set_major_tick(abstract_datum_component_type const& aTick) final
        {
            if (iMajorTick != aTick)
            {
                iMajorTick = aTick;
                Changed();
            }
        }
        void clear_minor_tick() final
        {
            if (iMinorTick != std::nullopt)
            {
                iMinorTick = std::nullopt;
                Changed();
            }
        }
        void clear_major_tick() final
        {
            if (iMajorTick != std::nullopt)
            {
                iMajorTick = std::nullopt;
                Changed();
            }
        }
    public:
        [[nodiscard]] neogfx::font const& font() const final
        {
            if (iFont)
                return iFont.value();
            return service<i_app>().current_style().font(font_role::Widget);
        }
        void set_font(optional_font const& aFont = {}) final
        {
            if (iFont != aFont)
            {
                iFont = aFont;
                Changed();
            }
        }
    private:
        string iTitle;
        std::optional<datum_component_type> iMinorTick;
        std::optional<datum_component_type> iMajorTick;
        optional_font iFont;
    };

    template <typename T>
    class default_graph_datum_traits
    {
    public:
        using type = T;
    private:
        template <typename T, typename = void>
        struct is_text_like : std::false_type {};
        template <typename T>
        struct is_text_like<T, std::void_t<decltype(to_string(std::declval<const T&>()))>> : std::true_type {};
        template <typename T>
        static constexpr bool is_text_like_v = is_text_like<T>::value;
    public:
        static string to_text(type const& v)
        {
            if constexpr (std::is_same_v<type, std::string> || std::is_same_v<type, string>)
                return v;
            else if constexpr (is_text_like_v<type>)
                return to_string(v);
            else
                throw unknown_graph_datum_type();
        }
    };

    template <typename X = double, typename Y = double, typename XTraits = default_graph_datum_traits<X>, typename YTraits = default_graph_datum_traits<Y>>
    class graph_renderer : public reference_counted<i_graph_renderer<abstract_t<X>, abstract_t<Y>>>
    {
    public:
        using x_abstract_type = abstract_t<X>;
        using y_abstract_type = abstract_t<Y>;
        using abstract_type = i_graph_renderer<x_abstract_type, y_abstract_type>;
    public:
        using x_type = X;
        using y_type = Y;
        using x_traits = XTraits;
        using y_traits = YTraits;
        using i_series = i_graph_series<x_abstract_type, y_abstract_type>;
    public:
        graph_renderer()
        {
        }
        // render
    public:
        void render(i_graph_widget<x_abstract_type, y_abstract_type> const& aWidget, i_graphics_context& aGc) const override
        {
            render_plot(aWidget, aGc);
            render_x_axis(aWidget, aGc);
            render_y_axis(aWidget, aGc);
        }
        void render_plot(i_graph_widget<x_abstract_type, y_abstract_type> const& aWidget, i_graphics_context& aGc) const override
        {
            for (auto const& s : aWidget.series())
                if (s->visible())
                    render_series(aWidget, aGc, *s);
        }
        void render_series(i_graph_widget<x_abstract_type, y_abstract_type> const& aWidget, i_graphics_context& aGc, i_series const& aSeries) const override
        {
            // todo
        }
        void render_x_axis(i_graph_widget<x_abstract_type, y_abstract_type> const& aWidget, i_graphics_context& aGc) const override
        {
            // todo
        }
        void render_y_axis(i_graph_widget<x_abstract_type, y_abstract_type> const& aWidget, i_graphics_context& aGc) const override
        {
            // todo
        }
        void render_x_label(x_abstract_type const& aX, i_graph_widget<x_abstract_type, y_abstract_type> const& aWidget, i_graphics_context& aGc, point const& aLabelOrigin, graph_rendering_element aElement) const override
        {
            aGc.draw_text(aLabelOrigin - x_label_extents(aX, aWidget, aGc, aElement) / 2.0, x_traits::to_text(aX), aWidget.x_axis().font(), aWidget.palette_color(color_role::Text));
        }
        void render_y_label(y_abstract_type const& aY, i_graph_widget<x_abstract_type, y_abstract_type> const& aWidget, i_graphics_context& aGc, point const& aLabelOrigin, graph_rendering_element aElement) const override
        {
            aGc.draw_text(aLabelOrigin - y_label_extents(aY, aWidget, aGc, aElement) / 2.0, y_traits::to_text(aY), aWidget.y_axis().font(), aWidget.palette_color(color_role::Text));
        }
        // metrics
    public:
        [[nodiscard]] rect plot_area(i_graph_widget<x_abstract_type, y_abstract_type> const& aWidget, i_graphics_context& aGc) const override
        {
            auto const& cr = aWidget.client_rect(false);
            point const topLeft{ y_axis_area(aWidget, aGc).right(), cr.top() };
            point const bottomRight{ cr.right(), x_axis_area(aWidget, aGc).top() };
            return rect{ topLeft, bottomRight - topLeft };
        }
        [[nodiscard]] rect x_axis_area(i_graph_widget<x_abstract_type, y_abstract_type> const& aWidget, i_graphics_context& aGc) const override
        {
            auto const& cr = aWidget.client_rect(false);
            size extents;
            if (aWidget.x_axis().has_major_tick())
            {
                if constexpr (std::is_arithmetic_v<x_type>)
                {
                    x_type xMin;
                    x_type xMax;
                    y_type ignore;
                    aWidget.get_view(xMin, xMax, ignore, ignore);
                    auto const& majorTick = aWidget.x_axis().major_tick();
                    using calc_type = std::conditional_t<std::is_floating_point_v<x_type>, x_type, double>;
                    auto tick = static_cast<x_type>(std::ceil(static_cast<calc_type>(xMin) / static_cast<calc_type>(majorTick)) * static_cast<calc_type>(majorTick));
                    while (tick >= xMin && tick <= xMax)
                    {
                        extents.cy = std::max(extents.cy, x_label_extents(tick, aWidget, aGc, graph_rendering_element::AxisLabel).cy);
                        tick += majorTick;
                    }
                }
                else
                {
                    for (auto const& s : aWidget.series())
                        for (auto const& d : s->data())
                            extents.cy = std::max(extents.cy, x_label_extents(d.x(), aWidget, aGc, graph_rendering_element::AxisLabel).cy);
                }
            }
            extents.cy += x_tick_length_px(aWidget);
            return cr.with_cy(extents.cy).with_y(cr.bottom() - extents.cy);
        }
        [[nodiscard]] rect y_axis_area(i_graph_widget<x_abstract_type, y_abstract_type> const& aWidget, i_graphics_context& aGc) const override
        {
            auto const& cr = aWidget.client_rect(false);
            size extents;
            if (aWidget.y_axis().has_major_tick())
            {
                if constexpr (std::is_arithmetic_v<y_type>)
                {
                    y_type yMin;
                    y_type yMax;
                    x_type ignore;
                    aWidget.get_view(ignore, ignore, yMin, yMax);
                    auto const& majorTick = aWidget.y_axis().major_tick();
                    using calc_type = std::conditional_t<std::is_floating_point_v<y_type>, y_type, double>;
                    auto tick = static_cast<y_type>(std::ceil(static_cast<calc_type>(yMin) / static_cast<calc_type>(majorTick)) * static_cast<calc_type>(majorTick));
                    while (tick >= yMin && tick <= yMax)
                    {
                        extents.cx = std::max(extents.cx, y_label_extents(tick, aWidget, aGc, graph_rendering_element::AxisLabel).cx);
                        tick += majorTick;
                    }
                }
                else
                {
                    for (auto const& s : aWidget.series())
                        for (auto const& d : s->data())
                            extents.cx = std::max(extents.cx, y_label_extents(d.y(), aWidget, aGc, graph_rendering_element::AxisLabel).cx);
                }
            }
            extents.cx += y_tick_length_px(aWidget);
            return cr.with_cx(extents.cx);
        }
        [[nodiscard]] size x_label_extents(x_abstract_type const& aX, i_graph_widget<x_abstract_type, y_abstract_type> const& aWidget, i_graphics_context& aGc, graph_rendering_element aElement) const override
        {
            return aGc.text_extent(x_traits::to_text(aX), aWidget.x_axis().font());
        }
        [[nodiscard]] size y_label_extents(y_abstract_type const& aY, i_graph_widget<x_abstract_type, y_abstract_type> const& aWidget, i_graphics_context& aGc, graph_rendering_element aElement) const override
        {
            return aGc.text_extent(y_traits::to_text(aY), aWidget.y_axis().font());
        }
        [[nodiscard]] scalar x_tick_length_px(i_graph_widget<x_abstract_type, y_abstract_type> const& aWidget) const override
        {
            if (iXTickLength_px.has_value())
                return iXTickLength_px.value();
            return aWidget.dpi_scale(size{ 4.0, 4.0 }).cx;
        }
        void set_x_tick_length_px(scalar aLength_px) override
        {
            iXTickLength_px = aLength_px;
        }
        void clear_x_tick_length_px() override
        {
            iXTickLength_px = std::nullopt;
        }
        [[nodiscard]] scalar y_tick_length_px(i_graph_widget<x_abstract_type, y_abstract_type> const& aWidget) const override
        {
            if (iYTickLength_px.has_value())
                return iYTickLength_px.value();
            return aWidget.dpi_scale(size{ 4.0, 4.0 }).cy;
        }
        void set_y_tick_length_px(scalar aLength_px) override
        {
            iYTickLength_px = aLength_px;
        }
        void clear_y_tick_length_px() override
        {
            iYTickLength_px = std::nullopt;
        }
    private:
        std::optional<scalar> iXTickLength_px;
        std::optional<scalar> iYTickLength_px;
    };

    template <typename X = double, typename Y = double, typename XTraits = default_graph_datum_traits<X>, typename YTraits = default_graph_datum_traits<Y>>
    class graph_widget : public widget<i_graph_widget<abstract_t<X>, abstract_t<Y>>>
    {
        using base_type = widget<i_graph_widget<abstract_t<X>, abstract_t<Y>>>;
    public:
        using x_abstract_type = abstract_t<X>;
        using y_abstract_type = abstract_t<Y>;
        using abstract_type = i_graph_widget<x_abstract_type, y_abstract_type>;
    public:
        using x_type = X;
        using y_type = Y;
        using x_traits = XTraits;
        using y_traits = YTraits;
        using i_datum = typename abstract_type::i_datum;
        using i_series = typename abstract_type::i_series;
        using datum_type = graph_datum<x_type, y_type>;
        using series_type = graph_series<x_type, y_type>;
        using series_container = vector<ref_ptr<series_type>>;
        using series_index = typename series_container::size_type;
        using i_renderer = typename abstract_type::i_renderer;
        using renderer_type = graph_renderer<x_type, y_type, x_traits, y_traits>;
    public:
        graph_widget(graph_widget_type aType = graph_widget_type::Line, graph_widget_flags aFlags = graph_widget_flags::None) :
            base_type{}, iType{ aType }, iFlags{ aFlags }
        {
            init();
        }
        graph_widget(i_widget& aParent, graph_widget_type aType = graph_widget_type::Line, graph_widget_flags aFlags = graph_widget_flags::None) :
            base_type{ aParent }, iType{ aType }, iFlags{ aFlags }
        {
            init();
        }
        graph_widget(i_layout& aLayout, graph_widget_type aType = graph_widget_type::Line, graph_widget_flags aFlags = graph_widget_flags::None) :
            base_type{ aLayout }, iType{ aType }, iFlags{ aFlags }
        {
            init();
        }
    public:
        void paint(i_graphics_context& aGc) const override
        {
            renderer().render(*this, aGc);
        }
    public:
        [[nodiscard]] graph_widget_type type() const final
        {
            return iType;
        }
        void set_type(graph_widget_type aType) final
        {
            if (iType != aType)
            {
                iType = aType;
                this->update();
            }
        }
        [[nodiscard]] graph_widget_flags flags() const final
        {
            return iFlags;
        }
        void set_flags(graph_widget_flags aFlags) final
        {
            if (iFlags != aFlags)
            {
                iFlags = aFlags;
                this->update();
            }
        }
    public:
        [[nodiscard]] series_index series_count() const final
        {
            return iSeries.size();
        }
        [[nodiscard]] series_container const& series() const final
        {
            return iSeries;
        }
        [[nodiscard]] series_type const& series(series_index aIndex) const final
        {
            return *iSeries.at(aIndex);
        }
        [[nodiscard]] series_type& series(series_index aIndex) final
        {
            return *iSeries.at(aIndex);
        }
        series_type& add_series(i_optional<i_string> const& aName = optional<string>{}, i_ref_ptr<i_graph_series_appearance> const& aAppearance = ref_ptr<graph_series_appearance>{}) final
        {
            iSeries.push_back(make_ref<series_type>(aName));
            auto& newSeries = *iSeries.back();
            iSink += newSeries.data_changed([&]()
                {
                    invalidate_cache();
                    this->update();
                });
            iSink += newSeries.visibility_changed([&]()
                {
                    invalidate_cache();
                    this->update();
                });
            iSink += newSeries.appearance_changed([&]()
                {
                    this->update();
                });
            if (aAppearance)
            {
                auto existingAppearance = std::find_if(iSeriesAppearanceMap.begin(), iSeriesAppearanceMap.end(), [&](auto const& e) { return e.second == aAppearance; });
                if (existingAppearance == iSeriesAppearanceMap.end())
                    iSink += aAppearance->changed([&]()
                        {
                            this->update();
                        });
                iSeriesAppearanceMap[newSeries] = aAppearance;
            }
            return newSeries;
        }
        void erase_series(series_index aIndex) final
        {
            auto existingSeries = std::next(iSeries.as_std_vector().begin(), aIndex);
            auto existingSeriesAppearance = iSeriesAppearanceMap.find(*existingSeries);
            iSeries.as_std_vector().erase(existingSeries);
            if (existingSeriesAppearance != iSeriesAppearanceMap.end())
                iSeriesAppearanceMap.erase(existingSeriesAppearance);
            invalidate_cache();
            this->update();
        }
    public:
        [[nodiscard]] x_type const& x_min(bool aIgnoreVisibility = false) const final
        {
            if (series_count() == 0u)
                throw no_graph_series_data();
            if (!iXMin.has_value())
            {
                for (auto const& s : iSeries)
                {
                    if (s->no_data())
                        continue;
                    if (!s->visible() && !aIgnoreVisibility)
                        continue;
                    if (!iXMin.has_value())
                        iXMin = s->x_min();
                    else
                    {
                        if constexpr (std::is_arithmetic_v<x_type>)
                            iXMin = std::min(iXMin.value(), s->x_min());
                    }
                }
            }
            if (!iXMin.has_value() && !aIgnoreVisibility)
                return x_min(true);
            if (!iXMin.has_value())
                throw no_graph_series_data();
            return iXMin.value();
        }
        [[nodiscard]] x_type const& x_max(bool aIgnoreVisibility = false) const final
        {
            if (series_count() == 0u)
                throw no_graph_series_data();
            if (!iXMax.has_value())
            {
                for (auto const& s : iSeries)
                {
                    if (s->no_data())
                        continue;
                    if (!s->visible() && !aIgnoreVisibility)
                        continue;
                    if (!iXMax.has_value())
                        iXMax = s->x_max();
                    else
                    {
                        if constexpr (std::is_arithmetic_v<x_type>)
                            iXMax = std::max(iXMax.value(), s->x_max());
                        else
                            iXMax = s->x_max();
                    }
                }
            }
            if (!iXMax.has_value() && !aIgnoreVisibility)
                return x_max(true);
            if (!iXMax.has_value())
                throw no_graph_series_data();
            return iXMax.value();
        }
        [[nodiscard]] y_type const& y_min(bool aIgnoreVisibility = false) const final
        {
            if (series_count() == 0u)
                throw no_graph_series_data();
            if (!iYMin.has_value())
            {
                for (auto const& s : iSeries)
                {
                    if (s->no_data())
                        continue;
                    if (!s->visible() && !aIgnoreVisibility)
                        continue;
                    if (!iYMin.has_value())
                        iYMin = s->y_min();
                    else
                    {
                        if constexpr (std::is_arithmetic_v<y_type>)
                            iYMin = std::min(iYMin.value(), s->y_min());
                    }
                }
            }
            if (!iYMin.has_value() && !aIgnoreVisibility)
                return y_min(true);
            if (!iYMin.has_value())
                throw no_graph_series_data();
            return iYMin.value();
        }
        [[nodiscard]] y_type const& y_max(bool aIgnoreVisibility = false) const final
        {
            if (series_count() == 0u)
                throw no_graph_series_data();
            if (!iYMax.has_value())
            {
                for (auto const& s : iSeries)
                {
                    if (s->no_data())
                        continue;
                    if (!s->visible() && !aIgnoreVisibility)
                        continue;
                    if (!iYMax.has_value())
                        iYMax = s->y_max();
                    else
                    {
                        if constexpr (std::is_arithmetic_v<y_type>)
                            iYMax = std::max(iYMax.value(), s->y_max());
                        else
                            iYMax = s->y_max();
                    }
                }
            }
            if (!iYMax.has_value() && !aIgnoreVisibility)
                return y_max(true);
            if (!iYMax.has_value())
                throw no_graph_series_data();
            return iYMax.value();
        }
        [[nodiscard]] x_type const& x_min(series_index aIndex) const final
        {
            return series(aIndex).x_min();
        }
        [[nodiscard]] x_type const& x_max(series_index aIndex) const final
        {
            return series(aIndex).x_max();
        }
        [[nodiscard]] y_type const& y_min(series_index aIndex) const final
        {
            return series(aIndex).y_min();
        }
        [[nodiscard]] y_type const& y_max(series_index aIndex) const final
        {
            return series(aIndex).y_max();
        }
    public:
        [[nodiscard]] i_graph_axis<x_abstract_type>& x_axis() const final
        {
            if (!iXAxis)
                throw graph_axis_undefined();
            return *iXAxis;
        }
        void set_x_axis(i_ref_ptr<i_graph_axis<x_abstract_type>> const& aAxis = ref_ptr<i_graph_axis<x_abstract_type>>{}) final
        {
            iXAxis = aAxis;
            if (iXAxis)
                iSink += iXAxis->changed([&]()
                    {
                        this->update();
                    });
        }
        [[nodiscard]] i_graph_axis<y_abstract_type>& y_axis() const final
        {
            if (!iYAxis)
                throw graph_axis_undefined();
            return *iYAxis;
        }
        void set_y_axis(i_ref_ptr<i_graph_axis<y_abstract_type>> const& aAxis = ref_ptr<i_graph_axis<y_abstract_type>>{}) final
        {
            iYAxis = aAxis;
            if (iYAxis)
                iSink += iYAxis->changed([&]()
                    {
                        this->update();
                    });
        }
    public:
        [[nodiscard]] i_renderer const& renderer() const final
        {
            return *iRenderer;
        }
        void set_renderer(i_renderer& aRenderer) final
        {
            iRenderer = aRenderer;
        }
        void set_default_renderer() final
        {
            iRenderer = make_ref<renderer_type>();
        }
    public:
        [[nodiscard]] bool has_view_transform_to_px() const final
        {
            return iViewTransformToPx.has_value();
        }
        [[nodiscard]] mat33 view_transform_to_px() const final
        {
            return iViewTransformToPx.value();
        }
        void set_view_transform_to_px(mat33 const& aTransform) final
        {
            iViewTransformToPx = aTransform;
            this->update();
        }
        void clear_view_transform_to_px() final
        {
            iViewTransformToPx = std::nullopt;
            this->update();
        }
        void get_view(x_abstract_type& xMin, x_abstract_type& xMax, y_abstract_type& yMin, y_abstract_type& yMax) const final
        {
            if (std::holds_alternative<std::monostate>(iView))
            {
                xMin = x_min();
                xMax = x_max();
                yMin = y_min();
                yMax = y_max();
            }
            else if (std::holds_alternative<view_min_max>(iView))
            {
                auto const& viewMinMax = std::get<view_min_max>(iView);
                xMin = viewMinMax.xMin;
                xMax = viewMinMax.xMax; 
                yMin = viewMinMax.yMin;
                yMax = viewMinMax.yMax;
            }
            else if (std::holds_alternative<ref_ptr<series_type>>(iView))
            {
                auto const& viewSeries = *(std::get<ref_ptr<series_type>>(iView));
                xMin = viewSeries.x_min();
                xMax = viewSeries.x_max();
                yMin = viewSeries.y_min();
                yMax = viewSeries.y_max();
            }
        }
        void set_default_view() final
        {
            iView = std::monostate{};
            this->update();
        }
        void set_view(series_index aIndex) final
        {
            iView = iSeries.at(aIndex);
            this->update();
        }
        void set_view(x_abstract_type const& xMin, x_abstract_type const& xMax, y_abstract_type const& yMin, y_abstract_type const& yMax) final
        {
            iView = view_min_max{ xMin, xMax, yMin, yMax };
            this->update();
        }
    public:
        [[nodiscard]] graph_series_appearance const& default_series_appearance() const final
        {
            if (iDefaultSeriesAppearance.has_value())
                return iDefaultSeriesAppearance.value();
            if (iStyleBasedSeriesAppearance.has_value())
                return iStyleBasedSeriesAppearance.value();
            iStyleBasedSeriesAppearance.emplace(pen{ this->palette_color(color_role::Text), 1.0_dip }, std::nullopt);
            return iStyleBasedSeriesAppearance.value();
        }
        void set_default_series_appearance(i_optional<i_graph_series_appearance> const& aSeriesAppearance = optional<graph_series_appearance>{}) final
        {
            iDefaultSeriesAppearance = aSeriesAppearance;
            this->update();
        }
    private:
        void init()
        {
            set_default_renderer();

            set_x_axis(make_ref<graph_axis<x_type>>());
            set_y_axis(make_ref<graph_axis<y_type>>());

            iSink += service<i_app>().current_style_changed([&](style_aspect aAspect)
                {
                    iStyleBasedSeriesAppearance = std::nullopt;
                    this->update();
                });
        }
        void invalidate_cache()
        {
            iXMin = std::nullopt;
            iYMin = std::nullopt;
            iXMax = std::nullopt;
            iYMax = std::nullopt;
        }
    private:
        graph_widget_type iType;
        graph_widget_flags iFlags;
        series_container iSeries;
        mutable std::optional<x_type> iXMin;
        mutable std::optional<x_type> iXMax;
        mutable std::optional<y_type> iYMin;
        mutable std::optional<y_type> iYMax;
        ref_ptr<i_graph_axis<x_abstract_type>> iXAxis;
        ref_ptr<i_graph_axis<y_abstract_type>> iYAxis;
        ref_ptr<i_renderer> iRenderer;
        std::optional<mat33> iViewTransformToPx;
        mutable optional<graph_series_appearance> iStyleBasedSeriesAppearance;
        mutable optional<graph_series_appearance> iDefaultSeriesAppearance;
        std::map<ref_ptr<series_type>, ref_ptr<graph_series_appearance>> iSeriesAppearanceMap;
        struct view_min_max
        {
            x_type xMin;
            x_type xMax;
            y_type yMin;
            y_type yMax;
        };
        std::variant<std::monostate, view_min_max, ref_ptr<series_type>> iView;
        sink iSink;
    };
}