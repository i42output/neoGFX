// i_audio_track.hpp
/*
  neogfx C++ GUI Library
  Copyright (c) 2015-present, Leigh Johnston.
  
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
#include <neolib/variant.hpp>
#include <neogfx/audio/i_audio_sample.hpp>

namespace neogfx
{
	class i_audio_track
	{
	public:
		struct item_sample { std::shared_ptr<i_audio_sample> sample; };
		struct item_silence { double duration; };
		struct item_repeat_start { uint32_t repeatCount; };
		struct item_repeat_end {};
		typedef neolib::variant<item_sample, item_silence, item_repeat_start, item_repeat_end> value_type;
		typedef uint32_t item_index;
	public:
		struct bad_item_index : std::logic_error { bad_item_index() : std::logic_error("neogfx::i_audio_track::bad_item_index") {} };
	public:
		virtual const audio_spec& spec() const = 0;
	public:
		virtual void add_sample(i_audio_sample& aSample) = 0;
		virtual void add_sample(std::shared_ptr<i_audio_sample> aSample) = 0;
		virtual void add_silence(double aDuration) = 0;
		virtual void repeat_start(uint32_t aRepeatCount) = 0;
		virtual void repeat_end() = 0;
	public:
		virtual item_index item_count() const = 0;
		virtual const value_type& item(item_index aItemIndex) const = 0;
		virtual value_type& item(item_index aItemIndex) = 0;
	};
}