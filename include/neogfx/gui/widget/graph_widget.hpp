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
        using i_datum = i_graph_datum<X, Y>;
        using datum = graph_datum<X, Y>;
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
            x_type const* min = &iData.front().x();
            for (auto const& d : iData)
                if (d.x() < *min)
                    min = &d.x();
            iXMin = min;
            return *iXMin;
        }
        [[nodiscard]] x_type const& x_max() const final
        {
            if (no_data())
                throw no_graph_series_data();
            if (iXMax != nullptr)
                return *iXMax;
            x_type const* max = &iData.front().x();
            for (auto const& d : iData)
                if (d.x() >= *max)
                    max = &d.x();
            iXMax = max;
            return *iXMax;
        }
        [[nodiscard]] y_type const& y_min() const final
        {
            if (no_data())
                throw no_graph_series_data();
            if (iYMin != nullptr)
                return *iYMin;
            y_type const* min = &iData.front().y();
            for (auto const& d : iData)
                if (d.y() < *min)
                    min = &d.y();
            iYMin = min;
            return *iYMin;
        }
        [[nodiscard]] y_type const& y_max() const final
        {
            if (no_data())
                throw no_graph_series_data();
            if (iYMax != nullptr)
                return *iYMax;
            y_type const* max = &iData.front().y();
            for (auto const& d : iData)
                if (d.y() >= *max)
                    max = &d.y();
            iYMax = max;
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

    template <typename X = double, typename Y = double>
    class graph_renderer : public reference_counted<i_graph_renderer<abstract_t<X>, abstract_t<Y>>>
    {
    public:
        using x_abstract_type = abstract_t<X>;
        using y_abstract_type = abstract_t<Y>;
        using abstract_type = i_graph_renderer<x_abstract_type, y_abstract_type>;
    public:
        using x_type = X;
        using y_type = Y;
        using i_series = i_graph_series<X, Y>;
    private:
        template <typename T, typename = void>
        struct is_text_like : std::false_type {};
        template <typename T>
        struct is_text_like<T, std::void_t<decltype(std::declval<to_string(std::declval<const T&>())>)>> : std::true_type {};
        template <typename T>
        static constexpr bool is_text_like_v = is_text_like<T>::value;
    public:
        graph_renderer()
        {
        }
        // series
    public:
        void render(i_graphics_context& aGc, i_graph_widget<X, Y> const& aWidget, i_series const& aSeries) const override
        {
            // todo
        }
        // axis labels, datum hover
    public:
        [[nodiscard]] size x_label_extents(x_abstract_type const& aX, i_graph_widget<X, Y> const& aWidget, i_graphics_context& aGc, graph_rendering_element aElement) const override
        {
            if constexpr (std::is_same_v<x_type, std::string> || std::is_same_v<x_type, string>)
                return aGc.text_extent(aX, aWidget.x_axis_font());
            else if constexpr (is_text_like_v<x_type>)
                return aGc.text_extent(to_string(aX), aWidget.x_axis().font());
            else
                throw unknown_graph_datum_type();
        }
        [[nodiscard]] size y_label_extents(y_abstract_type const& aY, i_graph_widget<X, Y> const& aWidget, i_graphics_context& aGc, graph_rendering_element aElement) const override
        {
            if constexpr (std::is_same_v<y_type, std::string> || std::is_same_v<y_type, string>)
                return aGc.text_extent(aY, aWidget.y_axis_font());
            else if constexpr (is_text_like_v<y_type>)
                return aGc.text_extent(to_string(aY), aWidget.y_axis().font());
            else
                throw unknown_graph_datum_type();
        }
        void render_x_label(x_abstract_type const& aX, i_graph_widget<X, Y> const& aWidget, i_graphics_context& aGc, point const& aLabelOrigin, graph_rendering_element aElement) const override
        {
            if constexpr (std::is_same_v<x_type, std::string> || std::is_same_v<x_type, string>)
                aGc.draw_text(aLabelOrigin - x_label_extents(aX, aWidget, aGc, aElement) / 2.0, aX, aWidget.x_axis().font(), aWidget.palette_color(color_role::Text));
            else if constexpr (is_text_like_v<x_type>)
                aGc.draw_text(aLabelOrigin - x_label_extents(aX, aWidget, aGc, aElement) / 2.0, to_string(aX), aWidget.x_axis().font(), aWidget.palette_color(color_role::Text));
            else
                throw unknown_graph_datum_type();
        }
        void render_y_label(y_abstract_type const& aY, i_graph_widget<X, Y> const& aWidget, i_graphics_context& aGc, point const& aLabelOrigin, graph_rendering_element aElement) const override
        {
            if constexpr (std::is_same_v<y_type, std::string> || std::is_same_v<y_type, string>)
                aGc.draw_text(aLabelOrigin - y_label_extents(aY, aWidget, aGc, aElement) / 2.0, aY, aWidget.y_axis().font(), aWidget.palette_color(color_role::Text));
            else if constexpr (is_text_like_v<y_type>)
                aGc.draw_text(aLabelOrigin - y_label_extents(aY, aWidget, aGc, aElement) / 2.0, to_string(aY), aWidget.y_axis().font(), aWidget.palette_color(color_role::Text));
            else
                throw unknown_graph_datum_type();
        }
    private:
    };

    template <typename X = double, typename Y = double>
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
        using i_datum = typename abstract_type::i_datum;
        using i_series = typename abstract_type::i_series;
        using datum_type = graph_datum<X, Y>;
        using series_type = graph_series<X, Y>;
        using series_container = vector<ref_ptr<series_type>>;
        using series_index = typename series_container::size_type;
        using i_renderer = typename abstract_type::i_renderer;
        using renderer_type = graph_renderer<X, Y>;
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
            if (aAppearance && iSeriesAppearances.find(aAppearance) == iSeriesAppearances.end())
            {
                iSeriesAppearances.insert(aAppearance);
                iSink += aAppearance->changed([&]()
                    {
                        this->update();
                    });
            }
            iSeriesAppearanceMap[newSeries] = aAppearance;
            return newSeries;
        }
        void erase_series(series_index aIndex) final
        {
            iSeries.as_std_vector().erase(std::next(iSeries.as_std_vector().begin(), aIndex));
            invalidate_cache();
            this->update();
        }
    public:
        [[nodiscard]] x_type const& x_min() const final
        {
            if (!iXMin.has_value())
            {
                for (auto const& s : iSeries)
                {
                    if (!s->visible())
                        continue;
                    if (!iXMin.has_value())
                        iXMin = s->x_min();
                    else
                        iXMin = std::min(iXMin.value(), s->x_min());
                }
            }
            return iXMin.value();
        }
        [[nodiscard]] x_type const& x_max() const final
        {
            if (!iXMax.has_value())
            {
                for (auto const& s : iSeries)
                {
                    if (!s->visible())
                        continue;
                    if (!iXMax.has_value())
                        iXMax = s->x_max();
                    else
                        iXMax = std::max(iXMax.value(), s->x_max());
                }
            }
            return iXMax.value();
        }
        [[nodiscard]] y_type const& y_min() const final
        {
            if (!iYMin.has_value())
            {
                for (auto const& s : iSeries)
                {
                    if (!s->visible())
                        continue;
                    if (!iYMin.has_value())
                        iYMin = s->y_min();
                    else
                        iYMin = std::min(iYMin.value(), s->y_min());
                }
            }
            return iYMin.value();
        }
        [[nodiscard]] y_type const& y_max() const final
        {
            if (!iYMax.has_value())
            {
                for (auto const& s : iSeries)
                {
                    if (!s->visible())
                        continue;
                    if (!iYMax.has_value())
                        iYMax = s->y_max();
                    else
                        iYMax = std::max(iYMax.value(), s->y_max());
                }
            }
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
        [[nodiscard]] i_graph_axis<x_type>& x_axis() const final
        {
            if (!iXAxis)
                throw graph_axis_undefined();
            return *iXAxis;
        }
        void set_x_axis(i_ref_ptr<i_graph_axis<x_type>> const& aAxis = ref_ptr<i_graph_axis<x_type>>{}) final
        {
            iXAxis = aAxis;
            iSink += iXAxis->changed([&]()
                {
                    this->update();
                });
        }
        [[nodiscard]] i_graph_axis<y_type>& y_axis() const final
        {
            if (!iYAxis)
                throw graph_axis_undefined();
            return *iYAxis;
        }
        void set_y_axis(i_ref_ptr<i_graph_axis<y_type>> const& aAxis = ref_ptr<i_graph_axis<y_type>>{}) final
        {
            iYAxis = aAxis;
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
        ref_ptr<i_graph_axis<x_type>> iXAxis;
        ref_ptr<i_graph_axis<y_type>> iYAxis;
        ref_ptr<i_renderer> iRenderer;
        std::optional<mat33> iViewTransformToPx;
        mutable optional<graph_series_appearance> iStyleBasedSeriesAppearance;
        mutable optional<graph_series_appearance> iDefaultSeriesAppearance;
        std::set<ref_ptr<graph_series_appearance>> iSeriesAppearances;
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