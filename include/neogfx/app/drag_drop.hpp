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
#include <neogfx/gui/window/i_window.hpp>

namespace neogfx
{
    template <typename DragDropObjectInterface>
    class drag_drop_object : public DragDropObjectInterface
    {
        typedef drag_drop_object<DragDropObjectInterface> self_type;
    public:
        typedef DragDropObjectInterface object_interface;
    public:
        drag_drop_object(i_drag_drop_source& aSource, drag_drop_object_type_id aType = object_interface::otid()) :
            iSource{ aSource }, 
            iType{ aType }
        {
        }
    public:
        i_drag_drop_source& source() const final
        {
            return iSource;
        }
        drag_drop_object_type_id ddo_type() const final
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
        i_drag_drop_source& iSource;
        drag_drop_object_type_id iType;
    };

    class drag_drop_file_list : public drag_drop_object<i_drag_drop_file_list>
    {
        typedef drag_drop_object<i_drag_drop_file_list> base_type;
    public:
        template <typename... Files>
        drag_drop_file_list(i_drag_drop_source& aSource, Files&&... aFiles) :
            base_type{ aSource },
            iFilePaths{ std::forward<Files>(aFiles)... }
        {
        }
    public:
        neolib::vector<string> const& file_paths() const final
        {
            return iFilePaths;
        }
    private:
        neolib::vector<string> iFilePaths;
    };

    class drag_drop_item : public drag_drop_object<i_drag_drop_item>
    {
        typedef drag_drop_object<i_drag_drop_item> base_type;
    public:
        drag_drop_item(i_drag_drop_source& aSource, i_item_presentation_model const& aPresentationModel, item_presentation_model_index const& aItem) :
            base_type{ aSource },
            iPresentationModel{ aPresentationModel }, 
            iItem{ aItem }
        {
        }
    public:
        i_item_presentation_model const& presentation_model() const final
        {
            return iPresentationModel;
        }
        item_presentation_model_index const& index() const final
        {
            return iItem;
        }
    public:
        bool can_render() const final
        {
            bool canRender = false;
            size renderExtents;
            presentation_model().dragging_item_render_info().trigger(*this, canRender, renderExtents);
            return canRender;
        }
        size render_extents() const final
        {
            bool canRender = false;
            size renderExtents;
            presentation_model().dragging_item_render_info().trigger(*this, canRender, renderExtents);
            return renderExtents;
        }
        void render(i_graphics_context& aGc, point const& aPosition = {}) const final
        {
            presentation_model().dragging_item_render().trigger(*this, aGc, aPosition);
        }
    private:
        i_item_presentation_model const& iPresentationModel;
        item_presentation_model_index iItem;
    };

    class drag_drop_entity : public drag_drop_object<i_drag_drop_entity>
    {
        typedef drag_drop_object<i_drag_drop_entity> base_type;
    public:
        drag_drop_entity(i_drag_drop_source& aSource, game::i_ecs const& aEcs, game::entity_id aEntity) :
            base_type{ aSource },
            iEcs{ aEcs }, 
            iEntity{ aEntity }
        {
        }
    public:
        game::i_ecs const& ecs() const final
        {
            return iEcs;
        }
        game::entity_id entity() const final
        {
            return iEntity;
        }
    private:
        game::i_ecs const& iEcs;
        game::entity_id iEntity;
    };

    struct drag_drop_source_empty_base {};

    template <typename Base = drag_drop_source_empty_base>
    class drag_drop_source : public Base, public i_drag_drop_source
    {
        typedef Base base_type;
    public:
        define_declared_event(DraggingObject, dragging_object, i_drag_drop_object const&)
        define_declared_event(DraggingCancelled, dragging_cancelled, i_drag_drop_object const&)
        define_declared_event(ObjectDroppedOnTarget, object_dropped_on_target, i_drag_drop_object const&, i_drag_drop_target&)
    public:
        template <typename... Args>
        drag_drop_source(Args&&... aArgs) :
            base_type{ std::forward<Args>(aArgs)... }, iEnabled{ false }, iObject{ nullptr }, iMonitor{ nullptr }
        {
        }
        ~drag_drop_source()
        {
            enable_drag_drop_source(false);
        }
    public:
        bool drag_drop_source_enabled() const final
        {
            return iEnabled;
        }
        void enable_drag_drop_source(bool aEnable = true) override
        {
            if (iEnabled != aEnable)
            {
                iEnabled = aEnable;
                if (drag_drop_source_enabled())
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
        bool drag_drop_active() const final
        {
            return iObject != nullptr;
        }
        i_drag_drop_object const& object_being_dragged() const final
        {
            if (!drag_drop_active())
                throw drag_drop_not_active();
            return *iObject;
        }
        void start_drag_drop(i_drag_drop_object const& aObject) final
        {
            if (!drag_drop_source_enabled())
                enable_drag_drop_source(true);
            if (drag_drop_active())
                throw drag_drop_already_active();
            iObject = &aObject;
            DraggingObject.trigger(*iObject);
        }
        void cancel_drag_drop() final
        {
            if (!drag_drop_active())
                throw drag_drop_not_active();
            auto object = iObject;
            iObject = nullptr;
            DraggingCancelled.trigger(*object);
            iWidget = nullptr;
            iWidgetOffset = std::nullopt;
        }
        void end_drag_drop(i_drag_drop_target& aTarget) final
        {
            if (!drag_drop_active())
                throw drag_drop_not_active();
            auto object = iObject;
            iObject = nullptr;
            ObjectDroppedOnTarget.trigger(*object, aTarget);
            iWidget = nullptr;
            iWidgetOffset = std::nullopt;
        }
    public:
        point const& drag_drop_tracking_position() const final
        {
            if (drag_drop_active())
                return *iTrackCurrent;
            throw drag_drop_not_active();
        }
        i_ref_ptr<i_widget> const& drag_drop_widget() const final
        {
            return iWidget;
        }
        void set_drag_drop_widget(i_ref_ptr<i_widget> const& aWidget) final
        {
            iWidget = aWidget;
            if (iWidget)
                iWidgetOffset = iWidget->position() - drag_drop_tracking_position();
            else
                iWidgetOffset = std::nullopt;
        }
    public:
        i_widget& drag_drop_event_monitor() const final
        {
            if (iMonitor != nullptr)
                return *iMonitor;
            throw no_drag_drop_event_monitor();
        }
        void monitor_drag_drop_events(i_widget& aWidget) final
        {
            iMonitor = &aWidget;
            iSink = aWidget.mouse_event([this, &aWidget](const neogfx::mouse_event& aEvent)
            {
                handle_drag_drop_event(aWidget, aEvent);
            });
            iSink += aWidget.root().paint_overlay([&](i_graphics_context& aGc)
            {
                if (drag_drop_active() && object_being_dragged().can_render() && iTrackCurrent)
                    object_being_dragged().render(aGc, aWidget.to_window_coordinates(*iTrackCurrent));
            });
        }
        void stop_monitoring_drag_drop_events() final
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
        virtual bool is_drag_drop_object(point const& aPosition) const = 0;
        virtual i_drag_drop_object const* drag_drop_object(point const& aPosition) = 0;
    private:
        void handle_drag_drop_event(i_widget& aWidget, const neogfx::mouse_event& aEvent)
        {
            auto const eventPos = aEvent.position() - aWidget.origin();
            switch (aEvent.type())
            {
            case mouse_event_type::ButtonClicked:
                if (aEvent.is_left_button() && is_drag_drop_object(eventPos))
                {
                    aWidget.set_capture();
                    iTrackStart = eventPos;
                    iTrackCurrent = eventPos;
                }
                break;
            case mouse_event_type::Moved:
                if (iTrackStart)
                {
                    iTrackCurrent = eventPos;
                    bool needUpdate = false;
                    if ((*iTrackStart - *iTrackCurrent).to_vec2().magnitude() >= drag_drop_trigger_distance())
                    {
                        if (!drag_drop_active())
                            start_drag_drop(*drag_drop_object(*iTrackStart));
                        needUpdate = object_being_dragged().can_render();
                        if (iWidget)
                        {
                            iWidget->move(*iTrackCurrent + *iWidgetOffset);
                            auto const windowPosition = aWidget.to_window_coordinates(eventPos) + aWidget.root().window_position();
                            iWidget->set_opacity(service<i_drag_drop>().is_target_at(object_being_dragged(), windowPosition) ? 1.0 : 0.25);
                        }
                    }
                    else
                    {
                        if (drag_drop_active())
                        {
                            needUpdate = object_being_dragged().can_render();
                            cancel_drag_drop();
                        }
                    }
                    if (needUpdate)
                        aWidget.root().as_widget().update();
                }
                break;
            case mouse_event_type::ButtonReleased:
                if (iTrackStart)
                {
                    if (aEvent.is_left_button())
                    {
                        iTrackStart = std::nullopt;
                        iTrackCurrent = std::nullopt;
                        if (drag_drop_active())
                        {
                            if (object_being_dragged().can_render())
                                aWidget.root().as_widget().update();
                            auto const windowPosition = aWidget.to_window_coordinates(eventPos) + aWidget.root().window_position();
                            if (service<i_drag_drop>().is_target_at(object_being_dragged(), windowPosition))
                            {
                                if (iWidget)
                                    iWidget->set_opacity(1.0);
                                auto& target = service<i_drag_drop>().target_at(object_being_dragged(), windowPosition);
                                target.accept(object_being_dragged(), target.as_widget().to_client_coordinates(windowPosition - aWidget.root().window_position()));
                                end_drag_drop(target);
                            }
                            else
                                cancel_drag_drop();
                        }
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
        optional_point iTrackStart;
        optional_point iTrackCurrent;
        scalar iTriggerDistance = 8.0;
        ref_ptr<i_widget> iWidget;
        optional_point iWidgetOffset;
    };

    template <typename Base>
    class drag_drop_target : public Base, public i_drag_drop_target
    {
        typedef Base base_type;
    public:
        define_declared_event(ObjectAcceptable, object_acceptable, i_drag_drop_object const&, optional_point const&, drop_operation&)
        define_declared_event(ObjectDropped, object_dropped, i_drag_drop_object const&, optional_point const&)
    public:
        template <typename... Args>
        drag_drop_target(Args&&... aArgs) : 
            base_type{ std::forward<Args>(aArgs)... }, iEnabled{ false }
        {
        }
        ~drag_drop_target()
        {
            enable_drag_drop_target(false);
        }
    public:
        bool drag_drop_target_enabled() const final
        {
            return iEnabled;
        }
        void enable_drag_drop_target(bool aEnable = true) final
        {
            if (iEnabled != aEnable)
            {
                iEnabled = aEnable;
                if (drag_drop_target_enabled())
                    service<i_drag_drop>().register_target(*this);
                else
                    service<i_drag_drop>().unregister_target(*this);
            }
        }
    public:
        bool can_accept(i_drag_drop_object const& aObject, optional_point const& aDropPosition = {}) const final
        {
            return accepted_as(aObject, aDropPosition) != drop_operation::None;
        }
        drop_operation accepted_as(i_drag_drop_object const& aObject, optional_point const& aDropPosition = {}) const final
        {
            drop_operation acceptableAs = drop_operation::None;
            ObjectAcceptable.trigger(aObject, aDropPosition, acceptableAs);
            return acceptableAs;
        }
        bool accept(i_drag_drop_object const& aObject, optional_point const& aDropPosition = {}) final
        {
            if (can_accept(aObject, aDropPosition))
            {
                ObjectDropped.trigger(aObject, aDropPosition);
                return true;
            }
            return false;
        }
    public:
        bool is_widget() const final
        {
            return std::is_base_of_v<i_widget, base_type>;
        }
        i_widget const& as_widget() const final
        {
            if constexpr (std::is_base_of_v<i_widget, base_type>)
                return *this;
            else
                throw drag_drop_target_not_a_widget();
        }
        i_widget& as_widget() final
        {
            return const_cast<i_widget&>(to_const(*this).as_widget());
        }
    private:
        bool iEnabled;
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
        bool is_target_for(i_drag_drop_object const& aObject) const final;
        bool is_target_at(i_drag_drop_object const& aObject, point const& aPosition) const final;
        i_drag_drop_target& target_for(i_drag_drop_object const& aObject) const final;
        i_drag_drop_target& target_at(i_drag_drop_object const& aObject, point const& aPosition) const final;
    protected:
        void register_source(std::shared_ptr<i_drag_drop_source> const& aSource);
        i_drag_drop_target* find_target(i_drag_drop_object const& aObject) const;
        i_drag_drop_target* find_target(i_drag_drop_object const& aObject, point const& aPosition) const;
    private:
        sources_t iSources;
        targets_t iTargets;
    };
}