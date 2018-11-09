// native_font.hpp
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
#include <unordered_map>
#include <tuple>
#include <neolib/variant.hpp>
#include <ft2build.h>
#include FT_FREETYPE_H
#include "i_native_font.hpp"
#include "i_native_font_face.hpp"

namespace neogfx
{
	class native_font : public i_native_font
	{
	public:
		typedef std::string filename_type;
		typedef std::pair<const void*, std::size_t> memory_block_type;
	private:
		typedef neolib::variant<filename_type, memory_block_type> source_type;
		typedef std::multimap<font_style, std::pair<std::string, FT_Long>> style_map;
		typedef std::map<std::tuple<FT_Long, font::point_size, size>, std::shared_ptr<i_native_font_face>> face_map;
		typedef std::unordered_map<i_native_font_face*, uint32_t> usage_map;
	public:
		struct failed_to_load_font : std::runtime_error { failed_to_load_font() : std::runtime_error("neogfx::native_font::failed_to_load_font") {} };
		struct no_matching_style_found : std::runtime_error { no_matching_style_found() : std::runtime_error("neogfx::native_font::no_matching_style_found") {} };
	public:
		native_font(FT_Library aFontLib, const std::string aFileName);
		native_font(FT_Library aFontLib, const void* aData, std::size_t aSizeInBytes);
		~native_font();
	public:
		virtual const std::string& family_name() const;
		virtual bool has_style(font_style aStyle) const;
		virtual std::size_t style_count() const;
		virtual font_style style(std::size_t aStyleIndex) const;
		virtual const std::string& style_name(std::size_t aStyleIndex) const;
		virtual i_native_font_face& create_face(font_style aStyle, font::point_size aSize, const i_device_resolution& aDevice);
		virtual i_native_font_face& create_face(const std::string& aStyleName, font::point_size aSize, const i_device_resolution& aDevice);
	public:
		virtual void add_ref(i_native_font_face& aFace);
		virtual void release(i_native_font_face& aFace);
	private:
		void register_face(FT_Long aFaceIndex);
		FT_Face open_face(FT_Long aFaceIndex);
		void close_face(FT_Face aFace);
		i_native_font_face& create_face(FT_Long aFaceIndex, font_style aStyle, font::point_size aSize, const i_device_resolution& aDevice);
	private:
		FT_Library iFontLib;
		source_type iSource;
		std::vector<unsigned char> iCache;
		std::string iFamilyName;
		FT_Long iFaceCount;
		style_map iStyleMap;
		face_map iFaces;
		usage_map iFaceUsage;
	};
}