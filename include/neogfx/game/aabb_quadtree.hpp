// aabb_quadree.hpp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2015, 2020 Leigh Johnston.  All Rights Reserved.
  
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

#include <boost/pool/pool_alloc.hpp>

#include <neolib/core/vecarray.hpp>
#include <neolib/core/lifetime.hpp>

#include <neogfx/core/numerical.hpp>
#include <neogfx/game/i_ecs.hpp>
#include <neogfx/game/entity_info.hpp>

namespace neogfx::game
{
    template <typename Collider, std::size_t BucketSize = 16, typename Allocator = boost::fast_pool_allocator<Collider>>
    class aabb_quadtree
    {
    public:
        typedef Collider collider_type;
        typedef Allocator allocator_type;
        typedef typename allocator_type::pointer pointer;
        typedef typename allocator_type::const_pointer const_pointer;
        typedef typename allocator_type::reference reference;
        typedef typename allocator_type::const_reference const_reference;
    public:
        typedef const void* const_iterator; // todo
        typedef void* iterator; // todo
    private:
        class node : public neolib::lifetime<>
        {
        private:
            typedef neolib::vecarray<entity_id, BucketSize, -1> entity_list;
            typedef std::array<std::array<aabb_2d, 2>, 2> quadrants;
            typedef std::array<std::array<node*, 2>, 2> children;
        private:
            struct no_parent : std::logic_error { no_parent() : std::logic_error{ "neogfx::aabb_quadtree::node::no_parent" } {} };
            struct no_children : std::logic_error { no_children() : std::logic_error{ "neogfx::aabb_quadtree::node::no_children" } {} };
        public:
            node(aabb_quadtree& aTree, const aabb_2d& aAabb) : iTree{ aTree }, iParent{ nullptr }, iDepth{ 1 }, iAabb { aAabb }, iChildren{}
            {
                iTree.iDepth = std::max(iTree.iDepth, iDepth);
                populate_quadrants();
            }
            node(const node& aParent, const aabb_2d& aAabb) : iTree{ aParent.iTree }, iParent{ &aParent }, iDepth{ aParent.iDepth + 1 }, iAabb { aAabb }, iChildren{}
            {
                iTree.iDepth = std::max(iTree.iDepth, iDepth);
                populate_quadrants();
            }
            ~node()
            {
                set_destroying();
                if (has_child<0, 0>())
                    remove_child<0, 0>();
                if (has_child<0, 1>())
                    remove_child<0, 1>();
                if (has_child<1, 0>())
                    remove_child<1, 0>();
                if (has_child<1, 1>())
                    remove_child<1, 1>();
                if (has_parent())
                    parent().unsplit(this);
            }
        public:
            bool has_parent() const
            {
                return iParent != nullptr;
            }
            const node& parent() const
            {
                if (has_parent())
                    return *iParent;
                throw no_parent();
            }
            node& parent()
            {
                return const_cast<node&>(to_const(*this).parent());
            }
            std::uint32_t depth() const
            {
                return iDepth;
            }
            const aabb_2d& aabb() const
            {
                return iAabb;
            }
            void add_entity(entity_id aEntity, const collider_type& aCollider)
            {
                iTree.iDepth = std::max(iTree.iDepth, iDepth);
                if (is_split())
                {
                    if (aabb_intersects(iQuadrants[0][0], aCollider.currentAabb))
                        child<0, 0>().add_entity(aEntity, aCollider);
                    if (aabb_intersects(iQuadrants[0][1], aCollider.currentAabb))
                        child<0, 1>().add_entity(aEntity, aCollider);
                    if (aabb_intersects(iQuadrants[1][0], aCollider.currentAabb))
                        child<1, 0>().add_entity(aEntity, aCollider);
                    if (aabb_intersects(iQuadrants[1][1], aCollider.currentAabb))
                        child<1, 1>().add_entity(aEntity, aCollider);
                }
                else
                {
                    iEntities.push_back(aEntity);
                    if (iEntities.size() > BucketSize && (iAabb.max - iAabb.min).min() > iTree.minimum_quadrant_size())
                        split();
                }
            }
            void remove_entity(entity_id aEntity, const collider_type& aCollider)
            {
                if (aCollider.previousAabb && aCollider.currentAabb)
                    remove_entity(aEntity, aCollider, aabb_union(*aCollider.previousAabb, *aCollider.currentAabb));
            }
            void remove_entity(entity_id aEntity, const collider_type& aCollider, const aabb_2d& aAabb)
            {
                if (!aabb_intersects(aCollider.currentAabb, iAabb))
                {
                    auto existing = std::find(iEntities.begin(), iEntities.end(), aEntity);
                    if (existing != iEntities.end())
                        iEntities.erase(existing);
                }
                if (has_child<0, 0>() && aabb_intersects(iQuadrants[0][0], aAabb))
                    child<0, 0>().remove_entity(aEntity, aCollider, aAabb);
                if (has_child<0, 1>() && aabb_intersects(iQuadrants[0][1], aAabb))
                    child<0, 1>().remove_entity(aEntity, aCollider, aAabb);
                if (has_child<1, 0>() && aabb_intersects(iQuadrants[1][0], aAabb))
                    child<1, 0>().remove_entity(aEntity, aCollider, aAabb);
                if (has_child<1, 1>() && aabb_intersects(iQuadrants[1][1], aAabb))
                    child<1, 1>().remove_entity(aEntity, aCollider, aAabb);
                if (empty())
                    iTree.destroy_node(*this);
            }
            void update_entity(entity_id aEntity, const collider_type& aCollider)
            {
                iTree.iDepth = std::max(iTree.iDepth, iDepth);
                auto const& currentAabb = aCollider.currentAabb;
                auto const& previousAabb = aCollider.previousAabb;
                if (currentAabb == previousAabb)
                    return;
                if (aabb_intersects(currentAabb, iAabb))
                    add_entity(aEntity, aCollider);
                if (aabb_intersects(previousAabb, iAabb))
                    remove_entity(aEntity, aCollider, previousAabb);
            }
            bool empty() const
            {
                bool result = iEntities.empty();
                if (has_child<0, 0>())
                    result = child<0, 0>().empty() && result;
                if (has_child<0, 1>())
                    result = child<0, 1>().empty() && result;
                if (has_child<1, 0>())
                    result = child<1, 0>().empty() && result;
                if (has_child<1, 1>())
                    result = child<1, 1>().empty() && result;
                return result;
            }
            const entity_list& entities() const
            {
                return iEntities;
            }
            template <typename Visitor>
            void visit(const collider_type& aCandidate, const Visitor& aVisitor) const
            {
                if (aCandidate.currentAabb)
                    visit(*aCandidate.currentAabb, aVisitor);
            }
            template <typename Visitor>
            void visit(const vec2& aPoint, const Visitor& aVisitor) const
            {
                visit(aabb_2d{ aPoint, aPoint }, aVisitor);
            }
            template <typename Visitor>
            void visit(const aabb_2d& aAabb, const Visitor& aVisitor) const
            {
                for (auto e : entities())
                    if (aabb_intersects(aAabb, iTree.iEcs.component<collider_type>().entity_record(e).currentAabb))
                        aVisitor(e);
                if (has_child<0, 0>() && aabb_intersects(iQuadrants[0][0], aAabb))
                    child<0, 0>().visit(aAabb, aVisitor);
                if (has_child<0, 1>() && aabb_intersects(iQuadrants[0][1], aAabb))
                    child<0, 1>().visit(aAabb, aVisitor);
                if (has_child<1, 0>() && aabb_intersects(iQuadrants[1][0], aAabb))
                    child<1, 0>().visit(aAabb, aVisitor);
                if (has_child<1, 1>() && aabb_intersects(iQuadrants[1][1], aAabb))
                    child<1, 1>().visit(aAabb, aVisitor);
            }
            template <typename Visitor>
            void visit_entities(const Visitor& aVisitor) const
            {
                for (auto e : entities())
                    aVisitor(e);
                if (has_child<0, 0>())
                    child<0, 0>().visit_entities(aVisitor);
                if (has_child<0, 1>())
                    child<0, 1>().visit_entities(aVisitor);
                if (has_child<1, 0>())
                    child<1, 0>().visit_entities(aVisitor);
                if (has_child<1, 1>())
                    child<1, 1>().visit_entities(aVisitor);
            }
            template <typename Visitor>
            void visit_aabbs(const Visitor& aVisitor) const
            {
                aVisitor(aabb());
                if (has_child<0, 0>())
                    child<0, 0>().visit_aabbs(aVisitor);
                if (has_child<0, 1>())
                    child<0, 1>().visit_aabbs(aVisitor);
                if (has_child<1, 0>())
                    child<1, 0>().visit_aabbs(aVisitor);
                if (has_child<1, 1>())
                    child<1, 1>().visit_aabbs(aVisitor);
            }
        private:
            void populate_quadrants()
            {
                auto const& min = iAabb.min;
                auto const& max = iAabb.max;
                auto const& center = (min + max) / 2.0;
                iQuadrants[0][0] = aabb_2d{ min, center };
                iQuadrants[0][1] = aabb_2d{ vec2{ min.x, center.y }, vec2{ center.x, max.y } };
                iQuadrants[1][0] = aabb_2d{ vec2{ center.x, min.y }, vec2{ max.x, center.y } };
                iQuadrants[1][1] = aabb_2d{ center, max };
            }
            template <std::size_t X, std::size_t Y>
            node& child() const
            {
                if (iChildren == std::nullopt)
                    iChildren.emplace();
                if ((*iChildren)[X][Y] == nullptr)
                    (*iChildren)[X][Y] = iTree.create_node(*this, iQuadrants[X][Y]);
                return *(*iChildren)[X][Y];
            }
            template <std::size_t X, std::size_t Y>
            bool has_child() const
            {
                if (iChildren == std::nullopt)
                    return false;
                return (*iChildren)[X][Y] != nullptr;
            }
            template <std::size_t X, std::size_t Y>
            bool remove_child(node* aDestroyedNode = nullptr) const
            {
                if (iChildren == std::nullopt)
                    return true;
                if ((*iChildren)[X][Y] == nullptr)
                    return true;
                if ((*iChildren)[X][Y] == aDestroyedNode || aDestroyedNode == nullptr)
                {
                    auto n = (*iChildren)[X][Y];
                    (*iChildren)[X][Y] = nullptr;
                    iTree.destroy_node(*n);
                    return true;
                }
                return false;
            }
            bool is_split() const
            {
                return iChildren != std::nullopt;
            }
            void split()
            {
                for (auto e : entities())
                {
                    auto const& collider = iTree.iEcs.component<collider_type>().entity_record(e);
                    if (aabb_intersects(iQuadrants[0][0], collider.currentAabb))
                        child<0, 0>().add_entity(e, collider);
                    if (aabb_intersects(iQuadrants[0][1], collider.currentAabb))
                        child<0, 1>().add_entity(e, collider);
                    if (aabb_intersects(iQuadrants[1][0], collider.currentAabb))
                        child<1, 0>().add_entity(e, collider);
                    if (aabb_intersects(iQuadrants[1][1], collider.currentAabb))
                        child<1, 1>().add_entity(e, collider);
                }
                iEntities.clear();
            }
            void unsplit(node* aDestroyedNode)
            {
                bool haveChildren = false;
                if (!remove_child<0, 0>(aDestroyedNode))
                    haveChildren = true;
                if (!remove_child<0, 1>(aDestroyedNode))
                    haveChildren = true;
                if (!remove_child<1, 0>(aDestroyedNode))
                    haveChildren = true;
                if (!remove_child<1, 1>(aDestroyedNode))
                    haveChildren = true;
                if (!haveChildren)
                    iChildren = std::nullopt;
                if (empty())
                    iTree.destroy_node(*this);
            }
        private:
            aabb_quadtree& iTree;
            const node* iParent;
            std::uint32_t iDepth;
            aabb_2d iAabb;
            quadrants iQuadrants;
            entity_list iEntities;
            mutable std::optional<children> iChildren;
        };
        typedef typename allocator_type::template rebind<node>::other node_allocator;
    public:
        aabb_quadtree(i_ecs& aEcs, const aabb_2d& aRootAabb = aabb_2d{ vec2{-4096.0, -4096.0}, vec2{4096.0, 4096.0} }, scalar aMinimumQuadrantSize = 16.0, const allocator_type& aAllocator = allocator_type{}) :
            iAllocator{ aAllocator },
            iEcs{ aEcs },
            iRootAabb{ aRootAabb },
            iCount{ 0 },
            iDepth{ 0 },
            iRootNode{ *this, aRootAabb },
            iMinimumQuadrantSize{ aMinimumQuadrantSize },
            iCollisionUpdateId{ 0 }
        {
        }
    public:
        scalar minimum_quadrant_size() const
        {
            return iMinimumQuadrantSize;
        }
        void full_update()
        {
            iDepth = 0;
            iRootNode.~node();
            new(&iRootNode) node{ *this, iRootAabb };
            for (auto entity : iEcs.component<collider_type>().entities())
            {
                auto& collider = iEcs.component<collider_type>().entity_record(entity);
                iRootNode.add_entity(entity, collider);
            }
        }
        void dynamic_update()
        {
            iDepth = 0;
            for (auto entity : iEcs.component<collider_type>().entities())
            {
                auto& collider = iEcs.component<collider_type>().entity_record(entity);
                iRootNode.update_entity(entity, collider);
            }
        }
        template <typename CollisionAction>
        void collisions(CollisionAction aCollisionAction) const
        {
            for (auto candidate : iEcs.component<collider_type>().entities())
            {
                auto const& candidateInfo = iEcs.component<entity_info>().entity_record(candidate);
                if (candidateInfo.destroyed)
                    continue;
                auto& candidateCollider = iEcs.component<collider_type>().entity_record(candidate);
                if (++iCollisionUpdateId == 0)
                    iCollisionUpdateId = 1;
                iRootNode.visit(candidateCollider, [&](entity_id aHit)
                {
                    if (candidateInfo.destroyed)
                        return;
                    if (candidate < aHit)
                    {
                        auto const& hitInfo = iEcs.component<entity_info>().entity_record(aHit);
                        if (hitInfo.destroyed)
                            return;
                        auto& hitCollider = iEcs.component<collider_type>().entity_record(aHit);
                        if ((candidateCollider.mask & hitCollider.mask) == 0 && hitCollider.collisionEventId != iCollisionUpdateId)
                        {
                            hitCollider.collisionEventId = iCollisionUpdateId;
                            aCollisionAction(candidate, aHit);
                        }
                    }
                });
            }
        }
        template <typename ResultContainer>
        void pick(const vec2& aPoint, ResultContainer& aResult, std::function<bool(entity_id aMatch, const vec2& aPoint)> aColliderPredicate = [](entity_id, const vec2&) { return true; }) const
        {
            iRootNode.visit(aPoint, [&](entity_id aMatch)
            {
                auto const& matchInfo = iEcs.component<entity_info>().entity_record(aMatch);
                if (!matchInfo.destroyed && aColliderPredicate(aMatch, aPoint))
                    aResult.insert(aResult.end(), aMatch);
            });
        }
        template <typename Visitor>
        void visit_aabbs(const Visitor& aVisitor) const
        {
            iRootNode.visit_aabbs(aVisitor);
        }
    public:
        void insert(reference aItem)
        {
            iRootNode.add_entity(aItem);
        }
        void remove(reference aItem)
        {
            iRootNode.remove_entity(aItem);
        }
    public:
        std::uint32_t count() const
        {
            return iCount;
        }
        std::uint32_t depth() const
        {
            return iDepth;
        }
    public:
        const node& root_node() const
        {
            return iRootNode;
        }
    private:
        node* create_node(const node& aParent, const aabb_2d& aAabb)
        {
            ++iCount;
            node* newNode = iAllocator.allocate(1);
            iAllocator.construct(newNode, aParent, aAabb);
            return newNode;
        }
        void destroy_node(node& aNode)
        {
            if (&aNode != &iRootNode && aNode.is_alive())
            {
                --iCount;
                iAllocator.destroy(&aNode);
                iAllocator.deallocate(&aNode, 1);
            }
        }
    private:
        node_allocator iAllocator;
        i_ecs& iEcs;
        aabb_2d iRootAabb;
        scalar iMinimumQuadrantSize;
        std::uint32_t iCount;
        mutable std::uint32_t iDepth;
        node iRootNode;
        mutable std::uint32_t iCollisionUpdateId;
    };
}