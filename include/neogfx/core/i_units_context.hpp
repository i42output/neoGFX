// i_units_context.hpp
/*
  neogfx C++ GUI Library
  Copyright (c) 2015 Leigh Johnston.  All Rights Reserved.
  
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
#include <neogfx/core/geometrical.hpp>
#include <neogfx/core/device_metrics.hpp>

namespace neogfx
{ 
	enum class units
	{
		Pixels,
		Points,
		Picas,
		Ems,
		Millimetres,
		Millimeters = units::Millimetres,
		Centimetres,
		Centimeters = units::Centimetres,
		Inches,
		Percentage
	};

	class i_units_context
	{
	public:
		struct no_device_metrics : std::logic_error { no_device_metrics() : std::logic_error("neogfx::i_units_context::no_device_metrics") {} };
	public:
		virtual bool high_dpi() const = 0;
		virtual dimension dpi_scale_factor() const = 0;
	public:
		virtual bool device_metrics_available() const = 0;
		virtual const i_device_metrics& device_metrics() const = 0;
		virtual neogfx::units units() const = 0;
		virtual neogfx::units set_units(neogfx::units aUnits) const = 0;
		// helpers
	public:
		dimension dpi_scale(dimension aValue) const
		{
			return aValue * dpi_scale_factor();
		}
		size dpi_scale(const size& aSize) const
		{
			auto result = aSize;
			if (result.cx != size::max_dimension())
				result.cx *= dpi_scale_factor();
			if (result.cy != size::max_dimension())
				result.cy *= dpi_scale_factor();
			return result;
		}
		point dpi_scale(const point& aPoint) const
		{
			return aPoint * dpi_scale_factor();
		}
		margins dpi_scale(const margins& aMargins) const
		{
			return aMargins * dpi_scale_factor();
		}
		template <typename T>
		T&& dpi_select(T&& aLowDpiValue, T&& aHighDpiValue) const
		{
			return std::forward<T>(high_dpi() ? aHighDpiValue : aLowDpiValue);
		}
	};

	enum class alignment : uint32_t
	{
		None = 0x0000,
		Left = 0x0001,
		Right = 0x0002,
		Centre = 0x0004,
		Center = alignment::Centre,
		Justify = 0x0008,
		Top = 0x0010,
		VCentre = 0x0020,
		VCenter = alignment::VCentre,
		Bottom = 0x0040,
		Horizontal = Left | Centre | Right | Justify,
		Vertical = Top | VCentre | Bottom
	};

	inline constexpr alignment operator|(alignment aLhs, alignment aRhs)
	{
		return static_cast<alignment>(static_cast<uint32_t>(aLhs) | static_cast<uint32_t>(aRhs));
	}

	inline constexpr alignment operator&(alignment aLhs, alignment aRhs)
	{
		return static_cast<alignment>(static_cast<uint32_t>(aLhs) & static_cast<uint32_t>(aRhs));
	}
}
