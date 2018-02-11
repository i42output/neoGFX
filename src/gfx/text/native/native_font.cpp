// native_font.hpp
/*
  neogfx C++ GUI Library
  Copyright(C) 2015-present Leigh Johnston
  
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

#include <neogfx/neogfx.hpp>
#include <boost/filesystem.hpp>
#include "native_font.hpp"
#include "native_font_face.hpp"

namespace neogfx
{
	native_font::native_font(i_rendering_engine& aRenderingEngine, FT_Library aFontLib, const std::string aFileName) :
		iRenderingEngine(aRenderingEngine), iFontLib(aFontLib), iSource(filename_type(aFileName)), iCache{}, iFaceCount(0)
	{
		register_face(0);
		for (FT_Long f = 1; f < iFaceCount; ++f)
			register_face(f);
		iCache.clear();
		iCache.shrink_to_fit();
	}

	native_font::native_font(i_rendering_engine& aRenderingEngine, FT_Library aFontLib, const void* aData, std::size_t aSizeInBytes) :
		iRenderingEngine(aRenderingEngine), iFontLib(aFontLib), iSource(memory_block_type(aData, aSizeInBytes)), iCache{}, iFaceCount(0)
	{
		register_face(0);
		for (FT_Long f = 1; f < iFaceCount; ++f)
			register_face(f);
		iCache.clear();
		iCache.shrink_to_fit();
	}

	native_font::~native_font()
	{
	}

	const std::string& native_font::family_name() const
	{
		return iFamilyName;
	}

	bool native_font::has_style(font::style_e aStyle) const
	{
		return iStyleMap.find(aStyle) != iStyleMap.end();
	}

	std::size_t native_font::style_count() const
	{
		return iStyleMap.size();
	}

	font::style_e native_font::style(std::size_t aStyleIndex) const
	{
		return std::next(iStyleMap.begin(), aStyleIndex)->first;
	}

	const std::string& native_font::style_name(std::size_t aStyleIndex) const
	{
		return std::next(iStyleMap.begin(), aStyleIndex)->second.first;
	}

	namespace
	{
		uint32_t matching_bits(uint32_t lhs, uint32_t rhs)
		{
			if (lhs == rhs)
				return 32;
			uint32_t matches = 0;
			uint32_t test = 1;
			while (test != 0)
			{
				if ((lhs & rhs) & test)
					++matches;
				test <<= 1;
			}
			return matches;
		}
	}

	i_native_font_face& native_font::create_face(font::style_e aStyle, font::point_size aSize, const i_device_resolution& aDevice)
	{
		std::multimap<uint32_t, style_map::value_type*> matches;
		for (auto& s : iStyleMap)
			matches.insert(std::make_pair(matching_bits(s.first, aStyle), &s));
		if (matches.empty())
			throw no_matching_style_found();
		FT_Long faceIndex = matches.rbegin()->second->second.second;
		font::style_e faceStyle = matches.rbegin()->second->first;
		return create_face(faceIndex, faceStyle, aSize, aDevice);
	}

	i_native_font_face& native_font::create_face(const std::string& aStyleName, font::point_size aSize, const i_device_resolution& aDevice)
	{
		style_map::value_type* foundStyle = 0;
		for (auto& s : iStyleMap)
			if (neolib::make_ci_string(s.second.first) == neolib::make_ci_string(aStyleName))
			{
				foundStyle = &s;
				break;
			}
		if (foundStyle == 0)
			return create_face(font::Normal, aSize, aDevice);
		FT_Long faceIndex = foundStyle->second.second;
		font::style_e faceStyle = foundStyle->first;
		return create_face(faceIndex, faceStyle, aSize, aDevice);
	}

	void native_font::add_ref(i_native_font_face& aFace)
	{
		++iFaceUsage[&aFace];
		if (iFaceUsage[&aFace] == 1 && aFace.handle() == nullptr)
		{
			for (auto& face : iFaces)
			{
				if (&*face.second == &aFace)
				{
					face.second->update_handle(open_face(std::get<0>(face.first)));
					break;
				}
			}
		}
	}

	void native_font::release(i_native_font_face& aFace)
	{
		--iFaceUsage[&aFace];
		if (iFaceUsage[&aFace] == 0)
		{
			close_face(static_cast<FT_Face>(aFace.handle()));
			iFaceUsage.erase(iFaceUsage.find(&aFace));
			aFace.update_handle(nullptr);
		}
		if (iFaceUsage.empty())
		{
			iCache.clear();
			iCache.shrink_to_fit();
		}
	}

	void native_font::register_face(FT_Long aFaceIndex)
	{
		FT_Face face = open_face(aFaceIndex);
		try
		{
			if (aFaceIndex == 0)
			{
				iFaceCount = face->num_faces;
				iFamilyName = face->family_name;
			}
			font::style_e style = font::Invalid;
			if (face->style_flags & FT_STYLE_FLAG_ITALIC)
				style = static_cast<font::style_e>(style | font::Italic);
			if (face->style_flags & FT_STYLE_FLAG_BOLD)
				style = static_cast<font::style_e>(style | font::Bold);
			if (style == font::Invalid)
				style = font::Normal;
			iStyleMap.emplace(style, std::make_pair(face->style_name, aFaceIndex));
		}
		catch (...)
		{
			close_face(face);
			throw;
		}
		close_face(face);
	}

	FT_Face native_font::open_face(FT_Long aFaceIndex)
	{
		FT_Face face;
		if (iSource.is<filename_type>())
		{
			if (iCache.empty())
			{
				std::size_t fileSize = static_cast<std::size_t>(boost::filesystem::file_size(static_variant_cast<const filename_type&>(iSource)));
				iCache.resize(fileSize);
				std::ifstream file{ static_variant_cast<const filename_type&>(iSource).c_str(), std::ios::in | std::ios::binary };
				file.read(reinterpret_cast<char*>(&iCache[0]), fileSize);
			}
			FT_Error error = FT_New_Memory_Face(
				iFontLib,
				static_cast<const FT_Byte*>(&iCache[0]),
				iCache.size(),
				aFaceIndex,
				&face);
			if (error)
				throw failed_to_load_font();
		}
		else
		{
			FT_Error error = FT_New_Memory_Face(
				iFontLib,
				static_cast<const FT_Byte*>(static_variant_cast<const memory_block_type&>(iSource).first),
				static_variant_cast<const memory_block_type&>(iSource).second,
				aFaceIndex,
				&face);
			if (error)
				throw failed_to_load_font();
		}
		return face;
	}

	void native_font::close_face(FT_Face aFace)
	{
		FT_Done_Face(aFace);
	}

	i_native_font_face& native_font::create_face(FT_Long aFaceIndex, font::style_e aStyle, font::point_size aSize, const i_device_resolution& aDevice)
	{
		auto existingFace = iFaces.find(std::make_tuple(aFaceIndex, aSize, size(aDevice.horizontal_dpi(), aDevice.vertical_dpi())));
		if (existingFace != iFaces.end())
			return *existingFace->second;
		FT_Face newFace = open_face(aFaceIndex);
		try
		{
			std::unique_ptr<i_native_font_face> newFaceObject(new native_font_face(iRenderingEngine, *this, aStyle, aSize, size(aDevice.horizontal_dpi(), aDevice.vertical_dpi()), newFace));
			newFace = 0;
			auto iterNewFace = iFaces.insert(std::make_pair(std::make_tuple(aFaceIndex, aSize, size(aDevice.horizontal_dpi(), aDevice.vertical_dpi())), std::move(newFaceObject))).first;
			return *iterNewFace->second;
		}
		catch (...)
		{
			if (newFace != 0)
				close_face(newFace);
			throw;
		}
	}
}