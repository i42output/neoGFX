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
    class graph_series : public i_graph_series<abstract_t<X>, abstract_t<Y>>
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
        define_event(DataChanged, data_changed);
        define_event(AppearanceChanged, appearance_changed);
    public:
        graph_series()
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
                data_changed();
        }
        void push_back(i_datum const& aDatum) final
        {
            iData.as_std_vector().push_back(aDatum);
            if (!iUpdating)
                data_changed();
        }
        void insert(index_type aIndex, i_datum const& aDatum) final
        {
            iData.as_std_vector().insert(std::next(iData.as_std_vector().begin(), aIndex), aDatum);
            if (!iUpdating)
                data_changed();
        }
        void erase(index_type aIndex) final
        {
            iData.as_std_vector().erase(std::next(iData.as_std_vector().begin(), aIndex));
            if (!iUpdating)
                data_changed();
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
                appearance_changed();
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
                appearance_changed();
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
                appearance_changed();
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
                appearance_changed();
            }
        }
    public:
        void start_update() final
        {
            iUpdating = true;
        }
        void end_update() final
        {
            iUpdating = false;
            data_changed();
        }
    private:
        bool iUpdating = false;
        vector<datum> iData;
        optional<string> iName;
        bool iVisible = true;
        optional_pen iPen;
        optional_color_or_gradient iFill;
    };

    template <typename X = double, typename Y = double>
    class graph_widget : public widget<i_graph_widget<abstract_t<X>, abstract_t<Y>>>
    {
    public:
        using abstract_type = i_graph_widget<abstract_t<X>, abstract_t<Y>>;
    public:
        using x_type = X;
        using y_type = Y;
        using i_datum = typename abstract_type::i_datum;
        using i_series = typename abstract_type::i_series;
        using datum_type = graph_datum<X, Y>;
        using series_type = graph_series<X, Y>;
        using series_container = vector<series_type>;
        using series_index = typename series_container::size_type;
    public:
        [[nodiscard]] graph_widget_type type() const final
        {
            throw std::logic_error("neogfx::graph_widget: not yet implemented");
        }
        void set_type(graph_widget_type aType) final
        {
            throw std::logic_error("neogfx::graph_widget: not yet implemented");
        }
        [[nodiscard]] graph_widget_flags flags() const final
        {
            throw std::logic_error("neogfx::graph_widget: not yet implemented");
        }
        void set_flags(graph_widget_flags aFlags) final
        {
            throw std::logic_error("neogfx::graph_widget: not yet implemented");
        }
    public:
        [[nodiscard]] bool has_minor_x_tick() const final
        {
            throw std::logic_error("neogfx::graph_widget: not yet implemented");
        }
        [[nodiscard]] bool has_major_x_tick() const final
        {
            throw std::logic_error("neogfx::graph_widget: not yet implemented");
        }
        [[nodiscard]] x_type const& minor_x_tick() const final
        {
            throw std::logic_error("neogfx::graph_widget: not yet implemented");
        }
        [[nodiscard]] x_type const& major_x_tick() const final
        {
            throw std::logic_error("neogfx::graph_widget: not yet implemented");
        }
        void set_minor_x_tick(x_type const& aTick) final
        {
            throw std::logic_error("neogfx::graph_widget: not yet implemented");
        }
        void set_major_x_tick(x_type const& aTick) final
        {
            throw std::logic_error("neogfx::graph_widget: not yet implemented");
        }
        void clear_minor_x_tick() final
        {
            throw std::logic_error("neogfx::graph_widget: not yet implemented");
        }
        void clear_major_x_tick() final
        {
            throw std::logic_error("neogfx::graph_widget: not yet implemented");
        }
        [[nodiscard]] bool has_minor_y_tick() const final
        {
            throw std::logic_error("neogfx::graph_widget: not yet implemented");
        }
        [[nodiscard]] bool has_major_y_tick() const final
        {
            throw std::logic_error("neogfx::graph_widget: not yet implemented");
        }
        [[nodiscard]] y_type const& minor_y_tick() const final
        {
            throw std::logic_error("neogfx::graph_widget: not yet implemented");
        }
        [[nodiscard]] y_type const& major_y_tick() const final
        {
            throw std::logic_error("neogfx::graph_widget: not yet implemented");
        }
        void set_minor_y_tick(y_type const& aTick) final
        {
            throw std::logic_error("neogfx::graph_widget: not yet implemented");
        }
        void set_major_y_tick(y_type const& aTick) final
        {
            throw std::logic_error("neogfx::graph_widget: not yet implemented");
        }
        void clear_minor_y_tick() final
        {
            throw std::logic_error("neogfx::graph_widget: not yet implemented");
        }
        void clear_major_y_tick() final
        {
            throw std::logic_error("neogfx::graph_widget: not yet implemented");
        }
    public:
        [[nodiscard]] series_index series_count() const final
        {
            throw std::logic_error("neogfx::graph_widget: not yet implemented");
        }
        [[nodiscard]] series_type const& series(series_index aIndex) const final
        {
            throw std::logic_error("neogfx::graph_widget: not yet implemented");
        }
        [[nodiscard]] series_type& series(series_index aIndex) final
        {
            throw std::logic_error("neogfx::graph_widget: not yet implemented");
        }
        [[nodiscard]] series_index add_series(i_optional<i_string> const& aName) final
        {
            throw std::logic_error("neogfx::graph_widget: not yet implemented");
        }
        void erase_series(series_index aIndex) final
        {
            throw std::logic_error("neogfx::graph_widget: not yet implemented");
        }
    public:
        [[nodiscard]] bool has_view_transform_to_px() const final
        {
            throw std::logic_error("neogfx::graph_widget: not yet implemented");
        }
        [[nodiscard]] mat33 view_transform_to_px() const final
        {
            throw std::logic_error("neogfx::graph_widget: not yet implemented");
        }
        void set_view_transform_to_px(mat33 const& aTransform) final
        {
            throw std::logic_error("neogfx::graph_widget: not yet implemented");
        }
        void clear_view_transform_to_px() final
        {
            throw std::logic_error("neogfx::graph_widget: not yet implemented");
        }
        void get_view(x_type& xMin, x_type& xMax, y_type& yMin, y_type& yMax) const final
        {
            throw std::logic_error("neogfx::graph_widget: not yet implemented");
        }
        void set_default_view() final
        {
            throw std::logic_error("neogfx::graph_widget: not yet implemented");
        }
        void set_view(series_index aIndex) final
        {
            throw std::logic_error("neogfx::graph_widget: not yet implemented");
        }
        void set_view(x_type const& xMin, x_type const& xMax, y_type const& yMin, y_type const& yMax) final
        {
            throw std::logic_error("neogfx::graph_widget: not yet implemented");
        }
    public:
        [[nodiscard]] x_type const& min_x() const final
        {
            throw std::logic_error("neogfx::graph_widget: not yet implemented");
        }
        [[nodiscard]] x_type const& max_x() const final
        {
            throw std::logic_error("neogfx::graph_widget: not yet implemented");
        }
        [[nodiscard]] y_type const& min_y() const final
        {
            throw std::logic_error("neogfx::graph_widget: not yet implemented");
        }
        [[nodiscard]] y_type const& max_y() const final
        {
            throw std::logic_error("neogfx::graph_widget: not yet implemented");
        }
        [[nodiscard]] x_type const& min_x(series_index aIndex) const final
        {
            throw std::logic_error("neogfx::graph_widget: not yet implemented");
        }
        [[nodiscard]] x_type const& max_x(series_index aIndex) const final
        {
            throw std::logic_error("neogfx::graph_widget: not yet implemented");
        }
        [[nodiscard]] y_type const& min_y(series_index aIndex) const final
        {
            throw std::logic_error("neogfx::graph_widget: not yet implemented");
        }
        [[nodiscard]] y_type const& max_y(series_index aIndex) const final
        {
            throw std::logic_error("neogfx::graph_widget: not yet implemented");
        }
    };
}