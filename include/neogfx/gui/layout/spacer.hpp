// spacer.hpp
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

#pragma once

#include <neogfx/neogfx.hpp>
#include <neogfx/core/object.hpp>
#include <neogfx/gui/layout/layout_item.hpp>
#include <neogfx/gui/layout/i_spacer.hpp>

namespace neogfx
{
    class i_layout;

    class spacer : public layout_item<object<i_spacer>>
    {
        typedef layout_item<object<i_spacer>> base_type;
    public:
        struct no_parent : std::logic_error { no_parent() : std::logic_error("neogfx::spacer::no_parent") {} };
        struct padding_unsupported : std::logic_error { padding_unsupported() : std::logic_error("neogfx::spacer::padding_unsupported") {} };
    public:
        typedef i_spacer abstract_type;
    public:
        spacer(neogfx::expansion_policy aExpansionPolicy);
        spacer(i_layout& aParent, neogfx::expansion_policy aExpansionPolicy);
        virtual ~spacer();
    public:
        bool has_parent_layout() const final;
        const i_layout& parent_layout() const final;
        i_layout& parent_layout() final;
        void set_parent_layout(i_layout* aParentLayout) final;
        bool has_parent_widget() const final;
        const i_widget& parent_widget() const final;
        i_widget& parent_widget() final;
        void set_parent_widget(i_widget* aParentWidget) final;
    public:
        neogfx::expansion_policy expansion_policy() const override;
        void set_expansion_policy(neogfx::expansion_policy aExpansionPolicy) override;
    public:
        bool high_dpi() const override;
        dimension dpi_scale_factor() const override;
    public:
        neogfx::size_policy size_policy() const override;
    public:
        bool has_margin() const noexcept override;
        neogfx::margin margin() const override;
        bool has_border() const noexcept override;
        neogfx::border border() const override;
        bool has_padding() const noexcept override;
		neogfx::padding padding() const override;
    public:
        bool device_metrics_available() const override;
        const i_device_metrics& device_metrics() const override;
    public:
        void layout_as(const point& aPosition, const size& aSize) override;
    public:
        bool visible() const override;
    private:
        i_layout* iParentLayout;
        neogfx::expansion_policy iExpansionPolicy;
    };

    class horizontal_spacer : public spacer
    {
    public:
        horizontal_spacer();
        horizontal_spacer(i_layout& aParent);
    };

    class vertical_spacer : public spacer
    {
    public:
        vertical_spacer();
        vertical_spacer(i_layout& aParent);
    };
}