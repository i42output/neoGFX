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
        class node : public neolib::lifetime
        {
        private:
            struct object
            {
                entity_id id;
                const collider_type* collider;
            };
            typedef neolib::vecarray<object, BucketSize, -1> object_list;
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
            uint32_t depth() const
            {
                return iDepth;
            }
            const aabb_2d& aabb() const
            {
                return iAabb;
            }
            void add_object(entity_id aObjectId, const collider_type& aCollider)
            {
                add_object(object{ aObjectId, &aCollider });
            }
            void add_object(const object& aObject)
            {
                iTree.iDepth = std::max(iTree.iDepth, iDepth);
                if (is_split())
                {
                    if (aabb_intersects(iQuadrants[0][0], *aObject.collider->currentAabb))
                        child<0, 0>().add_object(aObject);
                    if (aabb_intersects(iQuadrants[0][1], *aObject.collider->currentAabb))
                        child<0, 1>().add_object(aObject);
                    if (aabb_intersects(iQuadrants[1][0], *aObject.collider->currentAabb))
                        child<1, 0>().add_object(aObject);
                    if (aabb_intersects(iQuadrants[1][1], *aObject.collider->currentAabb))
                        child<1, 1>().add_object(aObject);
                }
                else
                {
                    iObjects.push_back(aObject);
                    if (iObjects.size() > BucketSize && (iAabb.max - iAabb.min).min() > iTree.minimum_quadrant_size())
                        split();
                }
            }
            void remove_object(entity_id aObjectId, const collider_type& aCollider)
            {
                remove_object(aObjectId, aCollider, aabb_union(*aCollider.previousAabb, *aCollider.currentAabb));
            }
            void remove_object(entity_id aObjectId, const collider_type& aCollider, const aabb_2d& aAabb)
            {
                if (!aabb_intersects(*aCollider.currentAabb, iAabb))
                {
                    auto existing = std::find_if(iObjects.begin(), iObjects.end(), [aObjectId](const object& aObject) { return aObject.id == aObjectId; });
                    if (existing != iObjects.end())
                        iObjects.erase(existing);
                }
                if (has_child<0, 0>() && aabb_intersects(iQuadrants[0][0], aAabb))
                    child<0, 0>().remove_object(aObjectId, aCollider, aAabb);
                if (has_child<0, 1>() && aabb_intersects(iQuadrants[0][1], aAabb))
                    child<0, 1>().remove_object(aObjectId, aCollider, aAabb);
                if (has_child<1, 0>() && aabb_intersects(iQuadrants[1][0], aAabb))
                    child<1, 0>().remove_object(aObjectId, aCollider, aAabb);
                if (has_child<1, 1>() && aabb_intersects(iQuadrants[1][1], aAabb))
                    child<1, 1>().remove_object(aObjectId, aCollider, aAabb);
                if (empty())
                    iTree.destroy_node(*this);
            }
            void update_object(entity_id aObjectId, const collider_type& aCollider)
            {
                iTree.iDepth = std::max(iTree.iDepth, iDepth);
                auto const& currentAabb = *aCollider.currentAabb;
                auto const& previousAabb = *aCollider.previousAabb;
                if (currentAabb == previousAabb)
                    return;
                if (aabb_intersects(currentAabb, iAabb))
                    add_object(aObjectId, aCollider);
                if (aabb_intersects(previousAabb, iAabb))
                    remove_object(aObjectId, aCollider, previousAabb);
            }
            bool empty() const
            {
                bool result = iObjects.empty();
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
            const object_list& objects() const
            {
                return iObjects;
            }
            template <typename Visitor>
            void visit(const collider_type& aCandidate, const Visitor& aVisitor) const
            {
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
                for (auto const& o : objects())
                    if (aabb_intersects(aAabb, *o.collider->currentAabb))
                        aVisitor(o.id);
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
            void visit_objects(const Visitor& aVisitor) const
            {
                for (auto const& o : iObjects)
                    aVisitor(o.id);
                if (has_child<0, 0>())
                    child<0, 0>().visit_objects(aVisitor);
                if (has_child<0, 1>())
                    child<0, 1>().visit_objects(aVisitor);
                if (has_child<1, 0>())
                    child<1, 0>().visit_objects(aVisitor);
                if (has_child<1, 1>())
                    child<1, 1>().visit_objects(aVisitor);
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
                auto const& centre = (min + max) / 2.0;
                iQuadrants[0][0] = aabb_2d{ min, centre };
                iQuadrants[0][1] = aabb_2d{ vec2{ min.x, centre.y }, vec2{ centre.x, max.y } };
                iQuadrants[1][0] = aabb_2d{ vec2{ centre.x, min.y }, vec2{ max.x, centre.y } };
                iQuadrants[1][1] = aabb_2d{ centre, max };
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
                for (auto const& o : objects())
                {
                    if (aabb_intersects(iQuadrants[0][0], *o.collider->currentAabb))
                        child<0, 0>().add_object(o);
                    if (aabb_intersects(iQuadrants[0][1], *o.collider->currentAabb))
                        child<0, 1>().add_object(o);
                    if (aabb_intersects(iQuadrants[1][0], *o.collider->currentAabb))
                        child<1, 0>().add_object(o);
                    if (aabb_intersects(iQuadrants[1][1], *o.collider->currentAabb))
                        child<1, 1>().add_object(o);
                }
                iObjects.clear();
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
            uint32_t iDepth;
            aabb_2d iAabb;
            quadrants iQuadrants;
            object_list iObjects;
            mutable std::optional<children> iChildren;
        };
        typedef typename allocator_type::template rebind<node>::other node_allocator;
    public:
        aabb_quadtree(const aabb_2d& aRootAabb = aabb_2d{ vec2{-4096.0, -4096.0}, vec2{4096.0, 4096.0} }, scalar aMinimumQuadrantSize = 16.0, const allocator_type& aAllocator = allocator_type{}) :
            iAllocator{ aAllocator },
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
        void full_update(const i_ecs& aEcs)
        {
            iDepth = 0;
            iRootNode.~node();
            new(&iRootNode) node{ *this, iRootAabb };
            for (auto entity : aEcs.component<collider_type>().entities())
            {
                auto& collider = aEcs.component<collider_type>().entity_record(entity);
                iRootNode.add_object(entity, collider);
            }
        }
        void dynamic_update(const i_ecs& aEcs)
        {
            iDepth = 0;
            for (auto entity : aEcs.component<collider_type>().entities())
            {
                auto& collider = aEcs.component<collider_type>().entity_record(entity);
                iRootNode.update_object(entity, collider);
            }
        }
        template <typename CollisionAction>
        void collisions(i_ecs& aEcs, CollisionAction aCollisionAction) const
        {
            for (auto candidate : aEcs.component<collider_type>().entities())
            {
                auto& candidateCollider = aEcs.component<collider_type>().entity_record(candidate);
                if (++iCollisionUpdateId == 0)
                    iCollisionUpdateId = 1;
                iRootNode.visit(candidateCollider, [&](entity_id aHit)
                {
                    if (candidate < aHit)
                    {
                        auto& hitCollider = aEcs.component<collider_type>().entity_record(aHit);
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
        void pick(const vec2& aPoint, ResultContainer& aResult, std::function<bool(reference, const vec2& aPoint)> aColliderPredicate = [](reference, const vec2&) { return true; }) const
        {
            iRootNode.visit(aPoint, [&](entity_id aMatch)
            {
                if (aColliderPredicate(aMatch, aPoint))
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
            iRootNode.add_object(aItem);
        }
        void remove(reference aItem)
        {
            iRootNode.remove_object(aItem);
        }
    public:
        uint32_t count() const
        {
            return iCount;
        }
        uint32_t depth() const
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
        aabb_2d iRootAabb;
        scalar iMinimumQuadrantSize;
        uint32_t iCount;
        mutable uint32_t iDepth;
        node iRootNode;
        mutable uint32_t iCollisionUpdateId;
    };
}