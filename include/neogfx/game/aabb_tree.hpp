// aabb_tree.hpp
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
#include <vector>
#include <unordered_set>
#include <queue>
#include <boost/pool/pool_alloc.hpp>
#include <neolib/red_black_tree.hpp>
#include <neogfx/core/numerical.hpp>
#include <neogfx/game/i_collidable.hpp>

namespace neogfx
{
	template <typename Allocator = boost::fast_pool_allocator<i_collidable>>
	class aabb_tree : private neolib::red_black_tree
	{
	public:
		typedef Allocator allocator_type;
		typedef typename allocator_type::reference reference;
		typedef typename allocator_type::const_reference const_reference;
	public:
		typedef const void* const_iterator; // todo
		typedef void* iterator; // todo
	private:
		typedef red_black_tree::node red_black_node;
		class node : public red_black_node
		{
		public:
			node(aabb_tree& aTree) : iTree{ aTree }, iAabb{}, iData{ nullptr }, iChildrenCrossed{ false }
			{
			}
			~node()
			{
			}
		public:
			bool is_leaf() const
			{
				return !has_left() || !has_right();
			}
			void set_leaf(i_collidable& aData)
			{
				iData = &aData;
				data()->set_collision_tree_link(this);
			}
			void update()
			{
				if (is_leaf())
				{
					// make fat AABB
					const vec3 marginVec = iTree.fat_margin(aabb());
					iAabb = neogfx::aabb{ data()->aabb().min - marginVec, data()->aabb().max + marginVec };
				}
				else // make union of child AABBs of child nodes
					iAabb = aabb_union(left()->aabb(), right()->aabb());
			}
			node* parent() const
			{
				return static_cast<node*>(red_black_node::parent());
			}
			node* left() const
			{
				return static_cast<node*>(red_black_node::left());
			}
			node* right() const
			{
				return static_cast<node*>(red_black_node::right());
			}
			node* sibling() const
			{
				return static_cast<node*>(red_black_node::sibling());
			}
			const neogfx::aabb& aabb() const
			{
				if (iAabb != boost::none)
					return *iAabb;
				return data()->aabb();
			}
			void set_aabb(const neogfx::aabb& aAabb)
			{
				iAabb = aAabb;
			}
			i_collidable* data() const
			{
				return iData;
			}
			void set_data(i_collidable* aData)
			{
				iData = aData;
			}
			bool children_crossed() const
			{
				return iChildrenCrossed;
			}
			void set_children_crossed(bool aChildrenCrossed) const
			{
				iChildrenCrossed = aChildrenCrossed;
			}
		private:
			aabb_tree& iTree;
			optional_aabb iAabb;
			i_collidable* iData;
			mutable bool iChildrenCrossed;
		};
		typedef typename allocator_type::template rebind<node>::other node_allocator;
	public:
		aabb_tree(scalar aFatMarginMultiplier = 0.0, scalar aFatMinimumMargin = 32.0, const allocator_type& aAllocator = allocator_type{}) :
			iAllocator{ aAllocator },
			iFatMarginMultiplier{ aFatMarginMultiplier },
			iFatMinimumMargin {	aFatMinimumMargin, aFatMinimumMargin, aFatMinimumMargin	},
			iCount{ 0 },
			iTotalDepth{ 0 },
			iCurrentDepth{ 0 }
		{
		}
	public:
		void update()
		{
			do_update();
		}
		template <typename CollisionAction>
		void collisions(CollisionAction aCollisionAction) const
		{
			if (root_node() != nil_node() && !root_node()->is_leaf())
			{
				clear_children_cross_flag_helper(root_node());
				iTotalDepth = 0;
				iCurrentDepth = 0;
				collisions_helper(aCollisionAction, root_node()->left(), root_node()->right());
			}
		}
		template <typename ResultContainer>
		const_reference pick(const vec3& aPoint, std::function<bool(reference, const vec3& aPoint)> aColliderPredicate, ResultContainer& aResult) const
		{
			std::queue<node*> q;
			if (root_node() != nil_node())
				q.push(root_node());

			while (!q.empty())
			{
				node& n = *q.front();
				q.pop();
				if (n.is_leaf())
				{
					if (aColliderPredicate(*n->data()->second.first, aPoint))
						aResult.insert(aResult.end(), n->data()->second.first);
				}
				else
				{
					red_black_node* left = n.left();
					red_black_node* right = n.right();
					if (!left->is_nil())
						q.push(n.left());
					if (!right->is_nil())
						q.push(n.right());
				}
			}
		}
		template <typename ResultContainer>
		reference pick(const vec3& aPoint, std::function<bool(reference, const vec3& aPoint)> aColliderPredicate, ResultContainer& aResult)
		{
			return const_cast<reference>(const_cast<const aabb_tree*>(this)->pick(aPoint, aColliderPredicate, aResult));
		}
	public:
		const_iterator begin() const
		{
			// todo
			throw std::logic_error("neogfx::aabb_tree::not_yet_implemented");
		}
		const_iterator cbegin() const
		{
			// todo
			throw std::logic_error("neogfx::aabb_tree::not_yet_implemented");
		}
		iterator begin()
		{
			// todo
			throw std::logic_error("neogfx::aabb_tree::not_yet_implemented");
		}
		const_iterator end() const
		{
			// todo
			throw std::logic_error("neogfx::aabb_tree::not_yet_implemented");
		}
		const_iterator cend() const
		{
			// todo
			throw std::logic_error("neogfx::aabb_tree::not_yet_implemented");
		}
		iterator end()
		{
			// todo
			throw std::logic_error("neogfx::aabb_tree::not_yet_implemented");
		}
	public:
		const_iterator find(const_reference aItem) const
		{
			return aItem.collision_tree_link();
		}
		const_iterator find(const vec3& aPoint) const
		{
			// todo
			throw std::logic_error("neogfx::aabb_tree::not_yet_implemented");
		}
		const_iterator find(const aabb& aAabb) const
		{
			// todo
			throw std::logic_error("neogfx::aabb_tree::not_yet_implemented");
		}
		iterator find(reference aItem)
		{
			return aItem.collision_tree_link();
		}
		iterator find(const vec3& aPoint)
		{
			// todo
			throw std::logic_error("neogfx::aabb_tree::not_yet_implemented");
		}
		iterator find(const aabb& aAabb)
		{
			// todo
			throw std::logic_error("neogfx::aabb_tree::not_yet_implemented");
		}
	public:
		iterator insert(reference aItem)
		{
			node* newNode = create_node();
			newNode->set_leaf(aItem);
			insert_node(newNode);
			return newNode;
		}
		void erase(iterator aItem)
		{
			static_cast<node*>(aItem)->data()->set_collision_tree_link(nullptr);
			static_cast<node*>(aItem)->set_data(nullptr);
			remove_node(static_cast<node*>(aItem));
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
	protected:
		node* root_node() const
		{
			return static_cast<node*>(red_black_tree::root_node());
		}
	private:
		void clear_children_cross_flag_helper(red_black_node* aNode) const
		{
			if (aNode->is_nil())
				return;
			node* n = static_cast<node*>(aNode);
			n->set_children_crossed(false);
			if (!n->is_leaf())
			{
				clear_children_cross_flag_helper(n->left());
				clear_children_cross_flag_helper(n->right());
			}
		}
		template <typename CollisionAction>
		void cross_children(const CollisionAction& aCollisionAction, red_black_node* aNode) const
		{
			if (aNode->is_nil())
				return;
			node* n = static_cast<node*>(aNode);
			if (!n->children_crossed())
			{
				collisions_helper(aCollisionAction, n->left(), n->right());
				n->set_children_crossed(true);
			}
		}
		template <typename CollisionAction>
		void collisions_helper(const CollisionAction& aCollisionAction, red_black_node* aNode0, red_black_node* aNode1) const
		{
			if (aNode0->is_nil() || aNode1->is_nil())
				return;
			node* n0 = static_cast<node*>(aNode0);
			node* n1 = static_cast<node*>(aNode1);
			neolib::scoped_counter sc{ iCurrentDepth };
			iTotalDepth = std::max(iTotalDepth, iCurrentDepth);
			if (n0->is_leaf())
			{
				// 2 leaves, check proxies instead of fat AABBs
				if (n1->is_leaf())
				{
					if (n0->data()->has_collided(*n1->data()))
						aCollisionAction(*n0->data(), *n1->data());
				}
				// 1 branch / 1 leaf
				else
				{
					cross_children(aCollisionAction, n1);
					collisions_helper(aCollisionAction, n0, n1->left());
					collisions_helper(aCollisionAction, n0, n1->right());
				}
			}
			else if (n1->is_leaf())
			{
				cross_children(aCollisionAction, n0);
				collisions_helper(aCollisionAction, n0->left(), n1);
				collisions_helper(aCollisionAction, n0->right(), n1);
			}
			// 2 branches
			else
			{
				cross_children(aCollisionAction, n0);
				cross_children(aCollisionAction, n1);
				collisions_helper(aCollisionAction, n0->left(), n1->left());
				collisions_helper(aCollisionAction, n0->left(), n1->right());
				collisions_helper(aCollisionAction, n0->right(), n1->left());
				collisions_helper(aCollisionAction, n0->right(), n1->right());
			}
		}
		vec3 fat_margin(const aabb& aAabb) const
		{
			return ((aAabb.max - aAabb.min) * iFatMarginMultiplier).max(iFatMinimumMargin);
		}
		void insert_node(node* aNode, node* aParent = nullptr)
		{
			if (aParent == nullptr && root_node() != nil_node())
				aParent = root_node();

			if (aParent == nullptr)
				red_black_tree::insert_node(aNode, [](red_black_node*, red_black_node*) { return true; });
			else if (aParent->is_leaf())
				red_black_tree::insert_node(aNode, [](red_black_node*, red_black_node* x) { return !x->has_left(); }, aParent);
			else
			{
				// parent is branch, compute volume differences 
				// between pre-insert and post-insert
				const aabb& aabb0 = aParent->left()->aabb();
				const aabb& aabb1 = aParent->right()->aabb();
				const scalar volumeDiff0 = aabb_volume(aabb_union(aabb0, aNode->aabb())) - aabb_volume(aabb0);
				const scalar volumeDiff1 = aabb_volume(aabb_union(aabb1, aNode->aabb())) - aabb_volume(aabb1);

				// insert to the child that gives less volume increase
				if (volumeDiff0 < volumeDiff1)
					insert_node(aNode, aParent->left());
				else
					insert_node(aNode, aParent->right());
				return;
			}
		}
		void remove_node(node* aNode)
		{
			red_black_tree::delete_node(aNode);
			destroy_node(aNode);
		}
		void do_update()
		{
			if (red_black_tree::root_node() != nil_node())
			{
				if (root_node()->is_leaf())
					root_node()->update();
				else
				{
					// grab all nodes
					iNodeCache.clear();
					do_update_helper(root_node());

					// re-insert all nodes
					clear();
					for (node* n : iNodeCache)
						insert_node(n);

					iNodeCache.clear();
					do_update_helper(root_node());
				}
			}
		}
		void do_update_helper(red_black_node* aNode)
		{
			node* n = static_cast<node*>(aNode);
			if (n->has_left())
				do_update_helper(n->left());
			if (n->has_right())
				do_update_helper(n->right());
			n->update();
			iNodeCache.push_back(n);
		}
		node* create_node()
		{
			++iCount;
			node* newNode = iAllocator.allocate(1);
			iAllocator.construct(newNode, *this);
			return newNode;
		}
		void destroy_node(node* aNode)
		{
			--iCount;
			iAllocator.destroy(aNode);
			iAllocator.deallocate(aNode, 1);
		}
	private:
		node_allocator iAllocator;
		scalar iFatMarginMultiplier;
		vec3 iFatMinimumMargin;
		std::vector<node*> iNodeCache;
		uint32_t iCount;
		mutable uint32_t iTotalDepth;
		mutable uint32_t iCurrentDepth;
	};
}