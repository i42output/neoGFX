// rect_pack.hpp
/*
 *  Based on public domain code @ http://blackpawn.com/texts/lightmaps/default.html.
 *
 *  This implementation written by Leigh Johnston.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are
 *  met:
 *
 *     * Redistributions of source code must retain the above authorship
 *       notice, this list of conditions and the following disclaimer.
 *
 *     * Redistributions in binary form must reproduce the above authorship
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *
 *     * Neither the name of Leigh Johnston nor the names of any
 *       other contributors to this software may be used to endorse or
 *       promote products derived from this software without specific prior
 *       written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
 *  IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 *  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 *  PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 *  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 *  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 *  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 *  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 *  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 *  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 *  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <neogfx/neogfx.hpp>
#include <boost/pool/pool_alloc.hpp>
#include <neogfx/core/geometrical.hpp>

#pragma once

namespace neogfx
{
    class rect_pack
    {
    private:
        class node
        {
        public:
            typedef boost::fast_pool_allocator<node, boost::default_user_allocator_new_delete, boost::details::pool::null_mutex> allocator_type;
        public:
            node(const rect& aRect, allocator_type& aAllocator) :
                iAllocator{ aAllocator }, iInUse { false }, iChildren{}, iRect{ aRect }
            {
            }
            ~node()
            {
                if (iChildren[0] != nullptr)
                {
                    iAllocator.destroy(iChildren[0]);
                    iAllocator.deallocate(iChildren[0]);
                }
                if (iChildren[1] != nullptr)
                {
                    iAllocator.destroy(iChildren[1]);
                    iAllocator.deallocate(iChildren[1]);
                }
            }
        public:
            bool is_leaf() const 
            {
                return iChildren[0] == nullptr;
            }
            const neogfx::rect& rect() const
            {
                return iRect;
            }
            node* insert(const size& aElementSize);
        private:
            allocator_type& iAllocator;
            bool iInUse;
            std::array<node*, 2> iChildren;
            neogfx::rect iRect;
        };
    public:
        rect_pack(const size& aDimensions);
    public:
        bool insert(const size& aElementSize, rect& aResult);
    private:
        node::allocator_type iAllocator;
        node iRoot;
    };
}