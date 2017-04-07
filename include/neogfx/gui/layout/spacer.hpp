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
	private:
		class device_metrics_forwarder : public i_device_metrics
		{
		public:
			device_metrics_forwarder(i_spacer& aOwner);
		public:
			virtual bool metrics_available() const;
			virtual size extents() const;
			virtual dimension horizontal_dpi() const;
			virtual dimension vertical_dpi() const;
			virtual dimension em_size() const;
		private:
			i_spacer& iOwner;
		};
	public:
		spacer(expansion_policy_e aExpansionPolicy);
		spacer(i_layout& aParent, expansion_policy_e aExpansionPolicy);
	public:
		virtual const i_layout& parent() const;
		virtual i_layout& parent();
		virtual void set_parent(i_layout& aParent);
		virtual expansion_policy_e expansion_policy() const;
		virtual void set_expansion_policy(expansion_policy_e aExpansionPolicy);
	public:
		virtual point position() const;
		virtual void set_position(const point& aPosition);
		virtual size extents() const;
		virtual void set_extents(const size& aExtents);
		virtual bool has_size_policy() const;
		virtual neogfx::size_policy size_policy() const;
		using i_widget_geometry::set_size_policy;
		virtual void set_size_policy(const optional_size_policy& aSizePolicy, bool aUpdateLayout = true);
		virtual bool has_weight() const;
		virtual size weight() const;
		virtual void set_weight(const optional_size& aWeight, bool aUpdateLayout = true);
		virtual bool has_minimum_size() const;
		virtual size minimum_size(const optional_size& aAvailableSpace = optional_size()) const;
		virtual void set_minimum_size(const optional_size& aMinimumSize, bool aUpdateLayout = true);
		virtual bool has_maximum_size() const;
		virtual size maximum_size(const optional_size& aAvailableSpace = optional_size()) const;
		virtual void set_maximum_size(const optional_size& aMaximumSize, bool aUpdateLayout = true);
	public:
		virtual neogfx::margins margins() const;
		virtual bool has_margins() const;
		virtual void set_margins(const optional_margins& aMargins, bool aUpdateLayout = true);
	public:
		virtual const i_device_metrics& device_metrics() const;
		virtual units_e units() const;
		virtual units_e set_units(units_e aUnits) const;
	private:
		i_layout* iParent;
		device_metrics_forwarder iDeviceMetricsForwarder;
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