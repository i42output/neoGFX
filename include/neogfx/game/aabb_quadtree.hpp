// aabb_quadree.hpp
/*
  neogfx C++ GUI Library
  Copyright(C) 2017 Leigh Johnston
  
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
#include <neolib/vecarray.hpp>
#include <boost/pool/pool_alloc.hpp>
#include <neogfx/core/numerical.hpp>
#include <neogfx/game/i_collidable.hpp>

namespace neogfx
{
	template <std::size_t BucketSize = 16, typename Allocator = boost::fast_pool_allocator<i_collidable>>
	class aabb_quadtree
	{
	public:
		typedef Allocator allocator_type;
		typedef typename allocator_type::pointer pointer;
		typedef typename allocator_type::const_pointer const_pointer;
		typedef typename allocator_type::reference reference;
		typedef typename allocator_type::const_reference const_reference;
	public:
		typedef const void* const_iterator; // todo
		typedef void* iterator; // todo
	private:
		class node
		{
		private:
			typedef neolib::vecarray<i_collidable*, BucketSize, -1> object_list;
			typedef std::array<std::array<aabb_2d, 2>, 2> quadrants;
			typedef std::array<std::array<node*, 2>, 2> children;
		private:
			struct no_parent : std::logic_error { no_parent() : std::logic_error{ "neogfx::aabb_quadtree::node::no_parent" } {} };
			struct no_children : std::logic_error { no_children() : std::logic_error{ "neogfx::aabb_quadtree::node::no_children" } {} };
		public:
			node(aabb_quadtree& aTree, const aabb_2d& aAabb) : iTree{ aTree }, iParent{ nullptr }, iAabb{ aAabb }, iChildren {}
			{
				populate_quadrants();
			}
			node(const node& aParent, const aabb_2d& aAabb) : iTree{ aParent.iTree }, iParent{ &aParent }, iAabb { aAabb }, iChildren{}
			{
				populate_quadrants();
			}
			~node()
			{
				iTree.destroy_node(iChildren[0][0]);
				iTree.destroy_node(iChildren[0][1]);
				iTree.destroy_node(iChildren[1][0]);
				iTree.destroy_node(iChildren[1][1]);
				if (has_parent())
				{
					if (parent().iChildren[0][0] == this)
						parent().iChildren[0][0] = nullptr;
					if (parent().iChildren[0][1] == this)
						parent().iChildren[0][1] = nullptr;
					if (parent().iChildren[1][0] == this)
						parent().iChildren[1][0] = nullptr;
					if (parent().iChildren[1][1] == this)
						parent().iChildren[1][1] = nullptr;
				}
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
				return const_cast<node&>(const_cast<const node*>(this)->parent());
			}
			const aabb_2d& aabb() const
			{
				return iAabb;
			}
			void add_object(i_collidable& aObject)
			{
				auto existing = std::find(iObjects.begin(), iObjects.end(), &aObject);
				if (existing == iObjects.end())
					iObjects.push_back(&aObject);
				if (iObjects.size() > BucketSize && (iAabb.max - iAabb.min).min() > iTree.minimum_quadrant_size())
					split();
			}
			void remove_object(i_collidable& aObject)
			{
				auto existing = std::find(iObjects.begin(), iObjects.end(), &aObject);
				if (existing != iObjects.end())
					iObjects.erase(existing);
				if (has_child<0, 0>())
					child<0, 0>().remove_object(aObject);
				if (has_child<0, 1>())
					child<0, 1>().remove_object(aObject);
				if (has_child<1, 0>())
					child<1, 0>().remove_object(aObject);
				if (has_child<1, 1>())
					child<1, 1>().remove_object(aObject);
				if (empty())
					iTree.destroy_node(this);
			}
			void remove_object(i_collidable& aObject, const aabb_2d& aAabb)
			{
				auto existing = std::find(iObjects.begin(), iObjects.end(), &aObject);
				if (existing != iObjects.end())
					iObjects.erase(existing);
				if (has_child<0, 0>() && aabb_intersects(iQuadrants[0][0], aAabb))
					child<0, 0>().remove_object(aObject);
				if (has_child<0, 1>() && aabb_intersects(iQuadrants[0][1], aAabb))
					child<0, 1>().remove_object(aObject);
				if (has_child<1, 0>() && aabb_intersects(iQuadrants[1][0], aAabb))
					child<1, 0>().remove_object(aObject);
				if (has_child<1, 1>() && aabb_intersects(iQuadrants[1][1], aAabb))
					child<1, 1>().remove_object(aObject);
				if (empty())
					iTree.destroy_node(this);
			}
			void update_object(i_collidable& aObject)
			{
				const auto& currentAabb = aabb_2d{ aObject.aabb() };
				const auto& savedAabb = aabb_2d{ aObject.saved_aabb() };
				if (currentAabb == savedAabb)
					return;
				if (aabb_intersects(savedAabb, iAabb) && !aabb_intersects(currentAabb, iAabb))
				{
					remove_object(aObject, savedAabb);
					if (has_child<0, 0>())
						child<0, 0>().update_object(aObject);
					if (has_child<0, 1>())
						child<0, 1>().update_object(aObject);
					if (has_child<1, 0>())
						child<1, 0>().update_object(aObject);
					if (has_child<1, 1>())
						child<1, 1>().update_object(aObject);
				}
				if (!aabb_intersects(savedAabb, iAabb) && aabb_intersects(currentAabb, iAabb))
					add_object(aObject);
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
			void visit(const i_collidable& aCandidate, const Visitor& aVisitor) const
			{
				visit(aCandidate.aabb(), aVisitor, &aCandidate);
			}
			template <typename Visitor>
			void visit(const vec2& aPoint, const Visitor& aVisitor) const
			{
				visit(aabb_2d{ aPoint, aPoint }, aVisitor);
			}
			template <typename Visitor>
			void visit(const aabb_2d& aAabb, const Visitor& aVisitor, const i_collidable* aCandidate = nullptr) const
			{
				if (aCandidate != nullptr && !aCandidate->collidable())
					return;
				for (auto o = objects().begin(); (aCandidate == nullptr || aCandidate->collidable()) && o != objects().end(); ++o)
					if (aabb_intersects(aAabb, aabb_2d{ (**o).aabb() }))
						aVisitor(*o);
				if (has_child<0, 0>() && aabb_intersects(iQuadrants[0][0], aAabb))
					child<0, 0>().visit(aAabb, aVisitor, aCandidate);
				if (has_child<0, 1>() && aabb_intersects(iQuadrants[0][1], aAabb))
					child<0, 1>().visit(aAabb, aVisitor, aCandidate);
				if (has_child<1, 0>() && aabb_intersects(iQuadrants[1][0], aAabb))
					child<1, 0>().visit(aAabb, aVisitor, aCandidate);
				if (has_child<1, 1>() && aabb_intersects(iQuadrants[1][1], aAabb))
					child<1, 1>().visit(aAabb, aVisitor, aCandidate);
			}
			template <typename Visitor>
			void visit_objects(const Visitor& aVisitor) const
			{
				if (!has_parent())
				{
					iTree.iCurrentDepth = 0;
					iTree.iTotalDepth = 0;
				}
				neolib::scoped_counter sc{ iTree.iCurrentDepth };
				iTree.iTotalDepth = std::max(iTree.iTotalDepth, iTree.iCurrentDepth);
				for (auto o : iObjects)
					aVisitor(o);
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
				const auto& min = iAabb.min;
				const auto& max = iAabb.max;
				const auto& centre = (min + max) / 2.0;
				iQuadrants[0][0] = aabb_2d{ min, centre };
				iQuadrants[0][1] = aabb_2d{ vec2{ min.x, centre.y }, vec2{ centre.x, max.y } };
				iQuadrants[1][0] = aabb_2d{ vec2{ centre.x, min.y }, vec2{ max.x, centre.y } };
				iQuadrants[1][1] = aabb_2d{ centre, max };
			}
			template <std::size_t X, std::size_t Y>
			node& child() const
			{
				if (iChildren[X][Y] == nullptr)
					iChildren[X][Y] = iTree.create_node(*this, iQuadrants[X][Y]);
				return *iChildren[X][Y];
			}
			template <std::size_t X, std::size_t Y>
			bool has_child() const
			{
				return iChildren[X][Y] != nullptr;
			}
			void split()
			{
				for (auto o : objects())
				{
					if (aabb_intersects(iQuadrants[0][0], o->aabb()))
						child<0, 0>().add_object(*o);
					if (aabb_intersects(iQuadrants[0][1], o->aabb()))
						child<0, 1>().add_object(*o);
					if (aabb_intersects(iQuadrants[1][0], o->aabb()))
						child<1, 0>().add_object(*o);
					if (aabb_intersects(iQuadrants[1][1], o->aabb()))
						child<1, 1>().add_object(*o);
				}
				iObjects.clear();
			}
		private:
			aabb_quadtree& iTree;
			const node* iParent;
			aabb_2d iAabb;
			quadrants iQuadrants;
			object_list iObjects;
			mutable children iChildren;
		};
		typedef typename allocator_type::template rebind<node>::other node_allocator;
	public:
		aabb_quadtree(const aabb_2d& aRootAabb = aabb_2d{ vec2{-4096.0, -4096.0}, vec2{4096.0, 4096.0} }, dimension aMinimumQuadrantSize = 16.0, const allocator_type& aAllocator = allocator_type{}) :
			iAllocator{ aAllocator },
			iRootAabb{ aRootAabb },
			iCount{ 0 },
			iTotalDepth{ 0 },
			iCurrentDepth{ 0 },
			iRootNode{ *this, aRootAabb },
			iMinimumQuadrantSize{ aMinimumQuadrantSize },
			iCollisionUpdateId{ 0 }
		{
		}
	public:
		dimension minimum_quadrant_size() const
		{
			return iMinimumQuadrantSize;
		}
		void full_update()
		{
			if (++iCollisionUpdateId == 0)
				iCollisionUpdateId = 1;
			std::vector<i_collidable*> objects;
			objects.reserve(count() * BucketSize);
			iRootNode.visit_objects([this, &objects](i_collidable* aObject)
			{
				if (aObject->collision_update_id() != iCollisionUpdateId)
				{
					aObject->set_collision_update_id(iCollisionUpdateId);
					objects.push_back(aObject);
				}
			});
			iRootNode.~node();
			new(&iRootNode) node{ *this, iRootAabb };
			for (auto o : objects)
				iRootNode.add_object(*o);
		}
		template <typename IterObject>
		void update_objects(IterObject aStart, IterObject aEnd)
		{
			for (IterObject o = aStart; o != aEnd; ++o)
				iRootNode.update_object((**o).as_collidable());
			for (IterObject o = aStart; o != aEnd; ++o)
				(**o).as_collidable().save_aabb();
		}
		template <typename IterObject, typename CollisionAction>
		void collisions(IterObject aStart, IterObject aEnd, CollisionAction aCollisionAction) const
		{
			for (IterObject o = aStart; o != aEnd; ++o)
			{
				auto& candidate = (**o).as_collidable();
				if (!candidate.collidable())
					continue;
				if (++iCollisionUpdateId == 0)
					iCollisionUpdateId = 1;
				iRootNode.visit(candidate, [this, &candidate, &aCollisionAction](i_collidable* aHit)
				{
					if (std::less<i_collidable*>{}(&candidate, aHit) && aHit->collidable())
					{
						if (aHit->collision_update_id() != iCollisionUpdateId)
						{
							aHit->set_collision_update_id(iCollisionUpdateId);
							if (candidate.has_collided(*aHit))
								aCollisionAction(candidate, *aHit);
						}
					}
				});
			}
		}
		template <typename ResultContainer>
		void pick(const vec2& aPoint, ResultContainer& aResult, std::function<bool(reference, const vec2& aPoint)> aColliderPredicate = [](reference, const vec2&) { return true; }) const
		{
			iRootNode.visit(aPoint, [&](i_collidable* aMatch)
			{
				if (aColliderPredicate(*aMatch, aPoint))
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
			return iTotalDepth;
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
		void destroy_node(node* aNode)
		{
			if (aNode != nullptr && aNode != &iRootNode)
			{
				--iCount;
				iAllocator.destroy(aNode);
				iAllocator.deallocate(aNode, 1);
			}
		}
	private:
		node_allocator iAllocator;
		aabb_2d iRootAabb;
		dimension iMinimumQuadrantSize;
		uint32_t iCount;
		mutable uint32_t iTotalDepth;
		mutable uint32_t iCurrentDepth;
		node iRootNode;
		mutable uint32_t iCollisionUpdateId;
	};
}