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
#include <neogfx/gui/widget/i_graph_widget.hpp>

namespace neogfx
{
    template <typename X = double, typename Y = double>
    class graph_datum : public i_graph_datum<abstract_t<X>, abstract_t<Y>>
    {
    public:
        using abstract_type = i_graph_datum<abstract_t<X>, abstract_t<Y>>;
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
        void set_x(x_type const& aX) final
        {
            iX = aX;
        }
        void set_y(y_type const& aY) final
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
        using abstract_type = i_graph_series<abstract_t<X>, abstract_t<Y>>;
    public:
        using x_type = X;
        using y_type = Y;
        using i_datum = i_graph_datum<X, Y>;
        using datum = graph_datum<X, Y>;
        using index_type = typename vector<datum>::size_type;
    public:
        define_declared_event(DataChanged, data_changed);
        define_declared_event(AppearanceChanged, appearance_changed);
    public:
        graph_series(i_optional<i_string> const& aName = optional<string>{}) : 
            iName{ aName }
        {
        }
        graph_series(abstract_type const& aOther) : 
            iData{ aOther.data() },
            iName{ aOther.name() },
            iVisible{ aOther.visible() },
            iPen{ aOther.pen() },
            iFill{ aOther.fill() }
        {
        }
    public:
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
            if (iData.empty())
                return {};
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
            if (iData.empty())
                return {};
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
            if (iData.empty())
                return {};
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
            if (iData.empty())
                return {};
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
                AppearanceChanged();
            }
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
                AppearanceChanged();
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
                AppearanceChanged();
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
        optional_pen iPen;
        optional_color_or_gradient iFill;
        mutable x_type const* iXMin = nullptr;
        mutable x_type const* iXMax = nullptr;
        mutable y_type const* iYMin = nullptr;
        mutable y_type const* iYMax = nullptr;
    };

    template <typename X = double, typename Y = double>
    class graph_widget : public widget<i_graph_widget<abstract_t<X>, abstract_t<Y>>>
    {
        using base_type = widget<i_graph_widget<abstract_t<X>, abstract_t<Y>>>;
    public:
        using abstract_type = i_graph_widget<abstract_t<X>, abstract_t<Y>>;
    public:
        using x_type = X;
        using y_type = Y;
        using i_datum = typename abstract_type::i_datum;
        using i_series = typename abstract_type::i_series;
        using datum_type = graph_datum<X, Y>;
        using series_type = graph_series<X, Y>;
        using series_container = vector<ref_ptr<series_type>>;
        using series_index = typename series_container::size_type;
    public:
        graph_widget(graph_widget_type aType = graph_widget_type::Line, graph_widget_flags aFlags = graph_widget_flags::None) :
            base_type{}, iType{ aType }, iFlags{ aFlags }
        {
        }
        graph_widget(i_widget& aParent, graph_widget_type aType = graph_widget_type::Line, graph_widget_flags aFlags = graph_widget_flags::None) :
            base_type{ aParent }, iType{ aType }, iFlags{ aFlags }
        {
        }
        graph_widget(i_layout& aLayout, graph_widget_type aType = graph_widget_type::Line, graph_widget_flags aFlags = graph_widget_flags::None) :
            base_type{ aLayout }, iType{ aType }, iFlags{ aFlags }
        {
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
        [[nodiscard]] bool has_minor_x_tick() const final
        {
            return iMinorXTick.has_value();
        }
        [[nodiscard]] bool has_major_x_tick() const final
        {
            return iMajorXTick.has_value();
        }
        [[nodiscard]] x_type const& minor_x_tick() const final
        {
            return iMinorXTick.value();
        }
        [[nodiscard]] x_type const& major_x_tick() const final
        {
            return iMajorXTick.value();
        }
        void set_minor_x_tick(x_type const& aTick) final
        {
            if (iMinorXTick != aTick)
            {
                iMinorXTick = aTick;
                this->update();
            }
        }
        void set_major_x_tick(x_type const& aTick) final
        {
            if (iMajorXTick != aTick)
            {
                iMajorXTick = aTick;
                this->update();
            }
        }
        void clear_minor_x_tick() final
        {
            if (iMinorXTick != std::nullopt)
            {
                iMinorXTick = std::nullopt;
                this->update();
            }
        }
        void clear_major_x_tick() final
        {
            if (iMajorXTick != std::nullopt)
            {
                iMajorXTick = std::nullopt;
                this->update();
            }
        }
        [[nodiscard]] bool has_minor_y_tick() const final
        {
            return iMinorYTick.has_value();
        }
        [[nodiscard]] bool has_major_y_tick() const final
        {
            return iMajorYTick.has_value();
        }
        [[nodiscard]] y_type const& minor_y_tick() const final
        {
            return iMinorYTick.value();
        }
        [[nodiscard]] y_type const& major_y_tick() const final
        {
            return iMajorYTick.value();
        }
        void set_minor_y_tick(y_type const& aTick) final
        {
            if (iMinorYTick != aTick)
            {
                iMinorYTick = aTick;
                this->update();
            }
        }
        void set_major_y_tick(y_type const& aTick) final
        {
            if (iMajorYTick != aTick)
            {
                iMajorYTick = aTick;
                this->update();
            }
        }
        void clear_minor_y_tick() final
        {
            if (iMinorYTick != std::nullopt)
            {
                iMinorYTick = std::nullopt;
                this->update();
            }
        }
        void clear_major_y_tick() final
        {
            if (iMajorYTick != std::nullopt)
            {
                iMajorYTick = std::nullopt;
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
        series_type& add_series(i_optional<i_string> const& aName = optional<string>{}) final
        {
            iSeries.push_back(make_ref<series_type>(aName));
            auto& newSeries = *iSeries.back();
            newSeries.data_changed([&]()
                {
                    invalidate_cache();
                    this->update();
                });
            newSeries.appearance_changed([&]()
                {
                    this->update();
                });
            return newSeries;
        }
        void erase_series(series_index aIndex) final
        {
            iSeries.as_std_vector().erase(std::next(iSeries.as_std_vector().begin(), aIndex));
            invalidate_cache();
            this->update();
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
        }
        void clear_view_transform_to_px() final
        {
            iViewTransformToPx = std::nullopt;
        }
        void get_view(x_type& xMin, x_type& xMax, y_type& yMin, y_type& yMax) const final
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
        void set_view(x_type const& xMin, x_type const& xMax, y_type const& yMin, y_type const& yMax) final
        {
            iView = view_min_max{ xMin, xMax, yMin, yMax };
            this->update();
        }
    public:
        [[nodiscard]] x_type const& x_min() const final
        {
            if (!iXMin.has_value())
            {
                for (auto const& s : iSeries)
                {
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
    private:
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
        std::optional<x_type> iMinorXTick;
        std::optional<x_type> iMajorXTick;
        std::optional<y_type> iMinorYTick;
        std::optional<y_type> iMajorYTick;
        std::optional<mat33> iViewTransformToPx;
        series_container iSeries;
        mutable std::optional<x_type> iXMin;
        mutable std::optional<x_type> iXMax;
        mutable std::optional<y_type> iYMin;
        mutable std::optional<y_type> iYMax;
        struct view_min_max
        {
            x_type xMin;
            x_type xMax;
            y_type yMin;
            y_type yMax;
        };
        std::variant<std::monostate, view_min_max, ref_ptr<series_type>> iView;
    };
}