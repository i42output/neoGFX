// spacer.hpp
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

#pragma once

#include <neogfx/neogfx.hpp>
#include "i_spacer.hpp"

namespace neogfx
{
	class i_layout;

	class spacer : public i_spacer
	{
	public:
		struct no_parent : std::logic_error { no_parent() : std::logic_error("neogfx::spacer::no_parent") {} };
		struct margins_unsupported : std::logic_error { margins_unsupported() : std::logic_error("neogfx::spacer::margins_unsupported") {} };
	public:
		spacer(expansion_policy_e aExpansionPolicy);
		spacer(i_layout& aParent, expansion_policy_e aExpansionPolicy);
	public:
		bool has_parent() const override;
		const i_layout& parent() const override;
		i_layout& parent() override;
		void set_parent(i_layout& aParent) override;
		expansion_policy_e expansion_policy() const override;
		void set_expansion_policy(expansion_policy_e aExpansionPolicy) override;
	public:
		point position() const override;
		void set_position(const point& aPosition) override;
		size extents() const override;
		void set_extents(const size& aExtents) override;
		bool has_size_policy() const override;
		neogfx::size_policy size_policy() const override;
		using i_widget_geometry::set_size_policy;
		void set_size_policy(const optional_size_policy& aSizePolicy, bool aUpdateLayout = true) override;
		bool has_weight() const override;
		size weight() const override;
		void set_weight(const optional_size& aWeight, bool aUpdateLayout = true) override;
		bool has_minimum_size() const override;
		size minimum_size(const optional_size& aAvailableSpace = optional_size()) const override;
		void set_minimum_size(const optional_size& aMinimumSize, bool aUpdateLayout = true) override;
		bool has_maximum_size() const override;
		size maximum_size(const optional_size& aAvailableSpace = optional_size()) const override;
		void set_maximum_size(const optional_size& aMaximumSize, bool aUpdateLayout = true) override;
	public:
		neogfx::margins margins() const override;
		bool has_margins() const override;
		void set_margins(const optional_margins& aMargins, bool aUpdateLayout = true) override;
	public:
		bool device_metrics_available() const override;
		const i_device_metrics& device_metrics() const override;
		neogfx::units units() const override;
		neogfx::units set_units(neogfx::units aUnits) const override;
	private:
		i_layout* iParent;
		units_context iUnitsContext;
		point iPosition;
		size iExtents;
		optional_size_policy iSizePolicy;
		optional_size iMinimumSize;
		optional_size iMaximumSize;
		optional_margins iMargins;
		expansion_policy_e iExpansionPolicy;
		optional_size iWeight;
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