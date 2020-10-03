// drag_drop.hpp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2020 Leigh Johnston.  All Rights Reserved.
  
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
#include <neolib/core/vector.hpp>
#include <neogfx/app/i_drag_drop.hpp>

namespace neogfx
{
    template <typename DragDropObjectInterface>
    class drag_drop_object : public DragDropObjectInterface
    {
        typedef drag_drop_object<DragDropObjectInterface> self_type;
    public:
        typedef DragDropObjectInterface object_interface;
    public:
        drag_drop_object(drag_drop_object_type_id aType = object_interface::otid()) :
            iType{ aType }
        {
        }
    public:
        drag_drop_object_type_id ddo_type() const override
        {
            return iType;
        }
    public:
        bool can_render() const override
        {
            return false;
        }
        size render_extents() const override
        {
            return {};
        }
        void render(i_graphics_context& aGc, point const& aPosition = {}) const override
        {
        }
    private:
        drag_drop_object_type_id iType;
    };

    class drag_drop_file_list : public drag_drop_object<i_drag_drop_file_list>
    {
    public:
        template <typename... Files>
        drag_drop_file_list(Files&&... aFiles) :
            iFilePaths{ std::forward<Files>(aFiles)... }
        {
        }
    public:
        neolib::vector<string> const& file_paths() const override
        {
            return iFilePaths;
        }
    private:
        neolib::vector<string> iFilePaths;
    };

    class drag_drop_entity : public drag_drop_object<i_drag_drop_entity>
    {
    public:
        drag_drop_entity(game::i_ecs const& aEcs, game::entity_id aEntity) :
            iEcs{ aEcs }, iEntity{ aEntity }
        {
        }
    public:
        game::i_ecs const& ecs() const override
        {
            return iEcs;
        }
        game::entity_id entity() const override
        {
            return iEntity;
        }
    private:
        game::i_ecs const& iEcs;
        game::entity_id iEntity;
    };

    class drag_drop_source : public i_drag_drop_source
    {
    public:
        define_declared_event(DraggingObject, dragging_object, i_drag_drop_object const&)
        define_declared_event(DraggingCancelled, dragging_cancelled, i_drag_drop_object const&)
        define_declared_event(ObjectDropped, object_dropped, i_drag_drop_object const&)
    public:
        drag_drop_source();
    public:
        bool drag_drop_active() const override;
        void start_drag_drop() override;
        void end_drag_drop() override;
    };

    class drag_drop_target_impl : public i_drag_drop_target
    {
    public:
        define_declared_event(ObjectAcceptable, object_acceptable, i_drag_drop_object const&, bool&)
        define_declared_event(ObjectDropped, object_dropped, i_drag_drop_object const&)
    public:
        drag_drop_target_impl();
    public:
        bool can_accept(i_drag_drop_object const& aObject) const override;
        bool accept(i_drag_drop_object const& aObject) override;
    public:
        bool is_widget() const override;
        i_widget const& as_widget() const override;
        i_widget& as_widget() override;
    };

    template <typename Base>
    class drag_drop_target : public Base, public drag_drop_target_impl
    {
        typedef Base base_type;
    public:
        using base_type::base_type;
    public:
        bool is_widget() const
        {
            return std::is_base_of_v<i_widget, base_type>;
        }
        i_widget const& as_widget() const override
        {
            if constexpr (std::is_base_of_v<i_widget, base_type>)
                return *this;
            else
                throw drag_drop_target_not_a_widget();
        }
        i_widget& as_widget() override
        {
            return const_cast<i_widget&>(to_const(*this).as_widget());
        }
    };

    class drag_drop : public i_drag_drop
    {
    public:
        define_declared_event(SourceRegistered, source_registered, i_drag_drop_source&)
        define_declared_event(SourceUnregistered, source_unregistered, i_drag_drop_source&)
        define_declared_event(TargetRegistered, target_registered, i_drag_drop_target&)
        define_declared_event(TargetUnregistered, target_unregistered, i_drag_drop_target&)
    private:
        typedef std::vector<i_drag_drop_source*> sources_t;
        typedef std::vector<i_drag_drop_target*> targets_t;
    public:
        drag_drop();
    public:
        void register_source(i_drag_drop_source& aSource) override;
        void unregister_source(i_drag_drop_source& aSource) override;
        void register_target(i_drag_drop_target& aTarget) override;
        void unregister_target(i_drag_drop_target& aTarget) override;
    public:
        bool is_target_for(i_drag_drop_object const& aObject) const override;
        bool is_target_at(i_drag_drop_object const& aObject, i_surface const& aSurface, point const& aPosition) const override;
        i_drag_drop_target& target_for(i_drag_drop_object const& aObject) const override;
        i_drag_drop_target& target_at(i_drag_drop_object const& aObject, i_surface const& aSurface, point const& aPosition) const override;
    private:
        i_drag_drop_target* find_target(i_drag_drop_object const& aObject) const;
        i_drag_drop_target* find_target(i_drag_drop_object const& aObject, i_surface const& aSurface, point const& aPosition) const;
    private:
        sources_t iSources;
        targets_t iTargets;
    };
}