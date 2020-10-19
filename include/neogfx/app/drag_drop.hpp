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

    template <typename Base>
    class drag_drop_source : public Base, public i_drag_drop_source
    {
        typedef Base base_type;
    public:
        define_declared_event(DraggingObject, dragging_object, i_drag_drop_object const&)
        define_declared_event(DraggingCancelled, dragging_cancelled, i_drag_drop_object const&)
        define_declared_event(ObjectDropped, object_dropped, i_drag_drop_object const&)
    public:
        template <typename... Args>
        drag_drop_source(Args&&... aArgs) :
            base_type{ std::forward<Args>(aArgs)... }, iEnabled{ false }, iObject{ nullptr }, iMonitor{ nullptr }
        {
        }
        ~drag_drop_source()
        {
            enable_drag_drop(false);
        }
    public:
        bool drag_drop_enabled() const override
        {
            return iEnabled;
        }
        void enable_drag_drop(bool aEnable) override
        {
            if (iEnabled != aEnable)
            {
                iEnabled = aEnable;
                if (drag_drop_enabled())
                {
                    if constexpr (std::is_base_of_v<i_widget, base_type>)
                        monitor_drag_drop_events(*this);
                    service<i_drag_drop>().register_source(*this);
                }
                else
                {
                    service<i_drag_drop>().unregister_source(*this);
                    stop_monitoring_drag_drop_events();
                }
            }
        }
        bool drag_drop_active() const override
        {
            return iObject != nullptr;
        }
        i_drag_drop_object const& object_being_dragged() const override
        {
            if (!drag_drop_active())
                throw drag_drop_not_active();
            return *iObject;
        }
        void start_drag_drop(i_drag_drop_object const& aObject) override
        {
            if (!drag_drop_enabled())
                enable_drag_drop(true);
            if (drag_drop_active())
                throw drag_drop_already_active();
            iObject = &aObject;
            DraggingObject.trigger(*iObject);
        }
        void cancel_drag_drop() override
        {
            if (!drag_drop_active())
                throw drag_drop_not_active();
            auto object = iObject;
            iObject = nullptr;
            DraggingCancelled.trigger(*object);
        }
        void end_drag_drop() override
        {
            if (!drag_drop_active())
                throw drag_drop_not_active();
            auto object = iObject;
            iObject = nullptr;
            ObjectDropped.trigger(*object);
        }
    public:
        i_widget& drag_drop_event_monitor() const override
        {
            if (iMonitor != nullptr)
                return *iMonitor;
            throw no_drag_drop_event_monitor();
        }
        void monitor_drag_drop_events(i_widget& aWidget) override
        {
            iMonitor = &aWidget;
            iSink = aWidget.mouse_event([this, &aWidget](const neogfx::mouse_event& aEvent)
            {
                handle_drag_drop_event(aWidget, aEvent);
            });
        }
        void stop_monitoring_drag_drop_events() override
        {
            iMonitor = nullptr;
            iSink.clear();
        }
    protected:
        scalar drag_drop_trigger_distance() const
        {
            return iTriggerDistance;
        }
        void set_drag_drop_trigger_distance(scalar aDistance)
        {
            iTriggerDistance = aDistance;
        }
    protected:
        virtual bool is_drag_drop_object(point const& aPosition) const
        {
            return false;
        }
        virtual i_drag_drop_object const* drag_drop_object(point const& aPosition)
        {
            return nullptr;
        }
    private:
        void handle_drag_drop_event(i_widget& aWidget, const neogfx::mouse_event& aEvent)
        {
            switch (aEvent.type())
            {
            case mouse_event_type::ButtonClicked:
                if (aEvent.is_left_button() && is_drag_drop_object(aEvent.position()))
                {
                    aWidget.set_capture();
                    iTrackStart = aEvent.position();
                }
                break;
            case mouse_event_type::Moved:
                if (iTrackStart)
                {
                    if ((*iTrackStart - aEvent.position()).to_vec2().magnitude() >= drag_drop_trigger_distance())
                    {
                        if (!drag_drop_active())
                            start_drag_drop(*(iObject = drag_drop_object(*iTrackStart)));
                    }
                    else
                    {
                        if (drag_drop_active())
                            cancel_drag_drop();
                    }
                    if (drag_drop_active() && !service<i_drag_drop>().is_target_for(object_being_dragged()))
                        cancel_drag_drop();
                    if (drag_drop_active() && object_being_dragged().can_render())
                    {
                        // todo
                    }
                }
                break;
            case mouse_event_type::ButtonReleased:
                if (iTrackStart)
                {
                    if (aEvent.is_left_button())
                    {
                        iTrackStart = std::nullopt;
                        if (drag_drop_active() && service<i_drag_drop>().is_target_at(
                            object_being_dragged(), aWidget.to_window_coordinates(aEvent.position())))
                        {
                            auto& target = service<i_drag_drop>().target_at(object_being_dragged(), aWidget.to_window_coordinates(aEvent.position()));
                            target.accept(object_being_dragged());
                            end_drag_drop();
                        }
                        else
                            cancel_drag_drop();
                    }
                }
                break;
            }
        }
    private:
        bool iEnabled;
        i_drag_drop_object const* iObject;
        i_widget* iMonitor;
        sink iSink;
        std::optional<point> iTrackStart;
        scalar iTriggerDistance = 8.0;
    };

    template <typename Base>
    class drag_drop_target : public Base, public i_drag_drop_target
    {
        typedef Base base_type;
    public:
        define_declared_event(ObjectAcceptable, object_acceptable, i_drag_drop_object const&, drop_operation&)
        define_declared_event(ObjectDropped, object_dropped, i_drag_drop_object const&)
    public:
        template <typename... Args>
        drag_drop_target(Args&&... aArgs) : 
            base_type{ std::forward<Args>(aArgs)... }
        {
            service<i_drag_drop>().register_target(*this);
        }
        ~drag_drop_target()
        {
            service<i_drag_drop>().unregister_target(*this);
        }
    public:
        bool can_accept(i_drag_drop_object const& aObject) const override
        {
            return accepted_as(aObject) != drop_operation::None;
        }
        drop_operation accepted_as(i_drag_drop_object const& aObject) const override
        {
            drop_operation acceptableAs = drop_operation::None;
            ObjectAcceptable.trigger(aObject, acceptableAs);
            return acceptableAs;
        }
        bool accept(i_drag_drop_object const& aObject) override
        {
            if (can_accept(aObject))
            {
                ObjectDropped.trigger(aObject);
                return true;
            }
            return false;
        }
    public:
        bool is_widget() const override
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
        typedef std::vector<std::shared_ptr<i_drag_drop_source>> sources_t;
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
        bool is_target_at(i_drag_drop_object const& aObject, point const& aPosition) const override;
        i_drag_drop_target& target_for(i_drag_drop_object const& aObject) const override;
        i_drag_drop_target& target_at(i_drag_drop_object const& aObject, point const& aPosition) const override;
    protected:
        void register_source(std::shared_ptr<i_drag_drop_source> const& aSource);
        i_drag_drop_target* find_target(i_drag_drop_object const& aObject) const;
        i_drag_drop_target* find_target(i_drag_drop_object const& aObject, point const& aPosition) const;
    private:
        sources_t iSources;
        targets_t iTargets;
    };
}