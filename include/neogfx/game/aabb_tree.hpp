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
#include <neogfx/core/numerical.hpp>
#include <neogfx/game/i_collidable.hpp>

namespace neogfx
{
	template <typename Allocator = boost::fast_pool_allocator<i_collidable>>
	class aabb_tree
	{
	public:
		typedef Allocator allocator_type;
		typedef typename allocator_type::reference reference;
		typedef typename allocator_type::const_reference const_reference;
	public:
		typedef const void* const_iterator; // todo
		typedef void* iterator; // todo
	private:
		class node
		{
		public:
			node(aabb_tree& aTree) : iTree{ aTree }, iParent { nullptr }, iLeft{ nullptr }, iRight{ nullptr }, iAabb{}, iData{ nullptr }, iChildrenCrossed{ false }
			{
			}
		public:
			bool is_leaf() const
			{
				return iLeft == nullptr;
			}
			void set_branch(node* aLeft, node* aRight)
			{
				iLeft = aLeft;
				iRight = aRight;
				iLeft->iParent = this;
				iRight->iParent = this;
				iData = nullptr;
			}
			void set_leaf(i_collidable& aData)
			{
				iLeft = nullptr;
				iRight = nullptr;
				iData = &aData;
				iData->set_collision_tree_link(this);
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
					iAabb = aabb_union(iLeft->aabb(), iRight->aabb());
			}
			node* parent() const
			{
				return iParent;
			}
			node*& parent()
			{
				return iParent;
			}
			node* left() const
			{
				return iLeft;
			}
			node*& left()
			{
				return iLeft;
			}
			node* right() const
			{
				return iRight;
			}
			node*& right()
			{
				return iRight;
			}
			node* sibling() const
			{
				return this == iParent->iLeft ? iParent->iRight : iParent->iLeft;
			}
			const neogfx::aabb& aabb() const
			{
				return iAabb;
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
			node* iParent;
			node* iLeft;
			node* iRight;
			neogfx::aabb iAabb;
			i_collidable* iData;
			mutable bool iChildrenCrossed;
		};
		typedef typename allocator_type::template rebind<node>::other node_allocator;
	public:
		aabb_tree(scalar aFatMarginMultiplier = 0.2, scalar aFatMinimumMargin = 32.0, const allocator_type& aAllocator = allocator_type{}) :
			iAllocator{ aAllocator },
			iFatMarginMultiplier{ aFatMarginMultiplier },
			iFatMinimumMargin {	aFatMinimumMargin, aFatMinimumMargin, aFatMinimumMargin	},
			iRoot{ nullptr }
		{
		}
	public:
		void update()
		{
			do_update();
		}
		void collisions(std::function<bool(reference, reference)> aColliderPredicate, std::function<void(reference, reference)> aColliderAction) const
		{
			if (iRoot && !iRoot->is_leaf())
			{
				clear_children_cross_flag_helper(iRoot);
				collisions_helper(aColliderPredicate, aColliderAction, iRoot->left(), iRoot->right());
			}
		}
		template <typename ResultContainer>
		const_reference pick(const vec3& aPoint, std::function<bool(reference, const vec3& aPoint)> aColliderPredicate, ResultContainer& aResult) const
		{
			std::queue<node*> q;
			if (iRoot)
				q.push(m_root);

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
					q.push(n.left());
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
			if (iRoot)
			{
				// not first node, insert node to tree
				node* newNode = create_node();
				newNode->set_leaf(aItem);
				newNode->update();
				insert_node(newNode, iRoot);
				return newNode;
			}
			else
			{
				// first node, make root
				iRoot = create_node();
				iRoot->set_leaf(aItem);
				iRoot->update();
				return iRoot;
			}
		}
		void erase(iterator aItem)
		{
			static_cast<node*>(aItem)->data()->set_collision_tree_link(nullptr);
			static_cast<node*>(aItem)->set_data(nullptr);
			remove_node(static_cast<node*>(aItem));
		}
	private:
		void clear_children_cross_flag_helper(node* aNode) const
		{
			aNode->set_children_crossed(false);
			if (!aNode->is_leaf())
			{
				clear_children_cross_flag_helper(aNode->left());
				clear_children_cross_flag_helper(aNode->right());
			}
		}
		void cross_children(const std::function<bool(reference, reference)>& aColliderPredicate, const std::function<void(reference, reference)>& aColliderAction, node* aNode) const
		{
			if (!aNode->children_crossed())
			{
				collisions_helper(aColliderPredicate, aColliderAction, aNode->left(), aNode->right());
				aNode->set_children_crossed(true);
			}
		}
		void collisions_helper(const std::function<bool(reference, reference)>& aColliderPredicate, const std::function<void(reference, reference)>& aColliderAction, node* aNode0, node* aNode1) const
		{
			if (aNode0->is_leaf())
			{
				// 2 leaves, check proxies instead of fat AABBs
				if (aNode1->is_leaf())
				{
					if (aColliderPredicate(*aNode0->data(), *aNode1->data()))
						aColliderAction(*aNode0->data(), *aNode1->data());
				}
				// 1 branch / 1 leaf
				else
				{
					cross_children(aColliderPredicate, aColliderAction, aNode1);
					collisions_helper(aColliderPredicate, aColliderAction, aNode0, aNode1->left());
					collisions_helper(aColliderPredicate, aColliderAction, aNode0, aNode1->right());
				}
			}
			else if (aNode1->is_leaf())
			{
				cross_children(aColliderPredicate, aColliderAction, aNode0);
				collisions_helper(aColliderPredicate, aColliderAction, aNode0->left(), aNode1);
				collisions_helper(aColliderPredicate, aColliderAction, aNode0->right(), aNode1);
			}
			// 2 branches
			else
			{
				cross_children(aColliderPredicate, aColliderAction, aNode0);
				cross_children(aColliderPredicate, aColliderAction, aNode1);
				collisions_helper(aColliderPredicate, aColliderAction, aNode0->left(), aNode1->left());
				collisions_helper(aColliderPredicate, aColliderAction, aNode0->left(), aNode1->right());
				collisions_helper(aColliderPredicate, aColliderAction, aNode0->right(), aNode1->left());
				collisions_helper(aColliderPredicate, aColliderAction, aNode0->right(), aNode1->right());
			}
		}
		vec3 fat_margin(const aabb& aAabb) const
		{
			return ((aAabb.max - aAabb.min) * iFatMarginMultiplier).max(iFatMinimumMargin);
		}
		void insert_node(node* aNode, node*& aParent)
		{
			node *p = aParent;
			if (p->is_leaf())
			{
				// parent is leaf, simply split
				node *newParent = create_node();
				newParent->parent() = p->parent();
				newParent->set_branch(aNode, p);
				aParent = newParent;
			}
			else
			{
				// parent is branch, compute volume differences 
				// between pre-insert and post-insert
				const aabb& aabb0 = p->left()->aabb();
				const aabb& aabb1 = p->right()->aabb();
				const scalar volumeDiff0 = aabb_volume(aabb_union(aabb0, aNode->aabb())) - aabb_volume(aabb0);
				const scalar volumeDiff1 = aabb_volume(aabb_union(aabb1, aNode->aabb())) - aabb_volume(aabb1);

				// insert to the child that gives less volume increase
				if (volumeDiff0 < volumeDiff1)
					insert_node(aNode, p->left());
				else
					insert_node(aNode, p->right());
			}

			// update parent AABB
			// (propagates back up the recursion stack)
			aParent->update();
		}
		void remove_node(node* aNode)
		{
			node* parent = aNode->parent();
			if (parent) // node is not root
			{
				if (parent->parent()) // if there's a grandparent
				{
					// update links
					node* sibling = aNode->sibling();
					sibling->parent() = parent->parent();
					(parent == parent->parent()->left() ? parent->parent()->left() : parent->parent()->right()) = sibling;
				}
				else // no grandparent
				{
					// make sibling root
					node* sibling = aNode->sibling();
					iRoot = sibling;
					sibling->parent() = nullptr;
				}
				destroy_node(aNode);
				destroy_node(parent);
			}
			else // node is root
			{
				iRoot = nullptr;
				destroy_node(aNode);
			}
		}
		void do_update()
		{
			if (iRoot)
			{
				if (iRoot->is_leaf())
					iRoot->update();
				else
				{
					// grab all invalid nodes
					iInvalidNodes.clear();
					do_update_helper(iRoot);

					// re-insert all invalid nodes
					for (node* n : iInvalidNodes)
					{
						// grab parent link
						node* parent = n->parent();
						node* sibling = n->sibling();
						node*& parentLink = parent->parent() ? (parent == parent->parent()->left() ? parent->parent()->left() : parent->parent()->right()) : iRoot;

						// replace parent with sibling
						sibling->parent() =	parent->parent() ? parent->parent() : nullptr; // root has null parent
						parentLink = sibling;
						destroy_node(parent);

						// re-insert node
						n->update();
						insert_node(n, iRoot);
					}
					iInvalidNodes.clear();
				}
			}
		}
		void do_update_helper(node* aNode)
		{
			if (aNode->is_leaf())
			{
				// check if fat AABB doesn't 
				// contain the collider's AABB anymore
				if (!aabb_contains(aNode->aabb(), aNode->data()->aabb()))
					iInvalidNodes.push_back(aNode);
			}
			else
			{
				do_update_helper(aNode->left());
				do_update_helper(aNode->right());
			}
		}
		node* create_node()
		{
			node* newNode = iAllocator.allocate(1);
			iAllocator.construct(newNode, *this);
			return newNode;
		}
		void destroy_node(node* aNode)
		{
			iAllocator.destroy(aNode);
			iAllocator.deallocate(aNode, 1);
		}
	private:
		node_allocator iAllocator;
		scalar iFatMarginMultiplier;
		vec3 iFatMinimumMargin;
		node* iRoot;
		std::vector<node*> iInvalidNodes;
	};
}