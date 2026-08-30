// i_sequencer.hpp
/*
  neolib C++ App/Game Engine
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

#include <concepts>
#include <type_traits>
#include <utility>

#include <neolib/core/numerical.hpp>
#include <neolib/core/i_jar.hpp>
#include <neolib/core/reference_counted.hpp>

#include <neogfx/game/i_ecs.hpp>

namespace neogfx
{
    namespace game
    {
        static_assert(std::is_integral_v<neolib::cookie>,
            "sequencer id types require neolib::cookie to be an integral type");

        enum class sequencer_track_id : neolib::cookie {};
        using optional_sequencer_track_id = std::optional<sequencer_track_id>;

        enum class sequencer_clip_id : neolib::cookie {};
        using optional_sequencer_clip_id = std::optional<sequencer_clip_id>;

        using sequencer_position = i64;
        using sequencer_offset = i64;
        using sequencer_duration = i64;

        class i_sequencer_clip : public neolib::i_reference_counted
        {
        public:
            using abstract_type = i_sequencer_clip;
        public:
            virtual sequencer_clip_id id() const = 0;
            // called by the sequencer when the clip is added; not for general use
            virtual void set_id(sequencer_clip_id aId) = 0;
        public:
            virtual void advance(sequencer_offset aPosition) = 0;
        public:
            template <typename Payload>
            Payload const& payload() const
            {
                return *static_cast<Payload const*>(payload_ptr());
            }
            template <typename Payload>
            Payload& payload()
            {
                return *static_cast<Payload*>(payload_ptr());
            }
        private:
            virtual void const* payload_ptr() const = 0;
            virtual void* payload_ptr() = 0;
        };

        using sequencer_clip_ptr = neolib::ref_ptr<i_sequencer_clip>;

        template <typename Payload>
        concept SequencerClipPayload = requires (Payload& aPayload, sequencer_offset aPosition)
        {
            { aPayload.advance(aPosition) };
        };

        template <SequencerClipPayload Payload>
        class sequencer_clip : public neolib::reference_counted<i_sequencer_clip>
        {
        public:
            using payload_type = Payload;
        public:
            template <typename... Args>
                requires std::constructible_from<Payload, Args...>
            explicit sequencer_clip(Args&&... aArgs) :
                iPayload{ std::forward<Args>(aArgs)... }
            {}
            sequencer_clip(sequencer_clip const&) = delete;
            sequencer_clip& operator=(sequencer_clip const&) = delete;
        public:
            sequencer_clip_id id() const final
            {
                return iId;
            }
            void set_id(sequencer_clip_id aId) final
            {
                iId = aId;
            }
        public:
            void advance(sequencer_offset aPosition) final
            {
                iPayload.advance(aPosition);
            }
        private:
            void const* payload_ptr() const final
            {
                return &iPayload;
            }
            void* payload_ptr() final
            {
                return &iPayload;
            }
        private:
            sequencer_clip_id iId = {};
            Payload iPayload;
        };

        template <typename Payload, typename... Args>
        inline sequencer_clip_ptr make_sequencer_clip(Args&&... aArgs)
        {
            return make_ref<sequencer_clip<Payload>>(std::forward<Args>(aArgs)...);
        }

        struct sequencer_clip_info
        {
            sequencer_clip_id id;
            sequencer_duration elapsed;
        };

        using optional_sequencer_clip_info = std::optional<sequencer_clip_info>;

        class i_sequencer : public i_service
        {
        public:
            virtual bool is_multi_threaded() const= 0;
            virtual bool is_single_threaded() const = 0;
            virtual void set_multi_threaded() = 0;
            virtual void set_single_threaded() = 0;
        public:
            virtual sequencer_track_id create_track() = 0;
            virtual void delete_track(sequencer_track_id aTrack) = 0;
        public:
            virtual i_sequencer_clip const& clip(sequencer_clip_id aClipId) const = 0;
            virtual i_sequencer_clip& clip(sequencer_clip_id aClipId) = 0;
        public:
            virtual sequencer_clip_id add_clip(i_ref_ptr<i_sequencer_clip> const& aClip, sequencer_track_id aTrack, sequencer_position aStart, sequencer_duration aDuration) = 0;
            virtual void delete_clip(sequencer_clip_id aClipId) = 0;
        public:
            virtual bool is_playing() const = 0;
            virtual sequencer_position position() const = 0;
        public:
            virtual void play() = 0;
            virtual void pause() = 0;
            virtual void rewind() = 0;
            virtual void seek(sequencer_position aPosition) = 0;
            virtual void stop() = 0;
        public:
            virtual optional_sequencer_clip_info current_clip(sequencer_track_id aTrack) const = 0;
            virtual optional_sequencer_clip_info next_clip(sequencer_track_id aTrack) const = 0;
        public:
            template <typename Payload, typename... Args>
            sequencer_clip_id emplace_clip(sequencer_track_id aTrack, sequencer_position aStart, sequencer_duration aDuration, Args&&... aArgs)
            {
                return add_clip(make_sequencer_clip<Payload>(std::forward<Args>(aArgs)...), aTrack, aStart, aDuration);
            }
        public:
            static uuid const& iid() { static uuid const sIid{ 0xa2fe8afe, 0x4483, 0x4b66, 0x8ee4, { 0x70, 0xd2, 0xf, 0x4e, 0xfc, 0xd0 } }; return sIid; }
        };
    }
}
