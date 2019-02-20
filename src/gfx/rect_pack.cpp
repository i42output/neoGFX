// rect_pack.cpp
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
#include <neogfx/gfx/rect_pack.hpp>

namespace neogfx
{
    rect_pack::node* rect_pack::node::insert(const size& aElementSize)
    {
        if (!is_leaf())
        {
            auto result = iChildren[0]->insert(aElementSize);
            if (result != nullptr)
                return result;
            return iChildren[1]->insert(aElementSize);
        }
        if (iInUse)
            return nullptr;
        if (iRect.extents().cx < aElementSize.cx || iRect.extents().cy < aElementSize.cy)
            return nullptr;
        else if (iRect.extents() == aElementSize)
        {
            iInUse = true;
            return this;
        }
        auto dw = iRect.width() - aElementSize.cx;
        auto dh = iRect.height() - aElementSize.cy;
        neogfx::rect rcChild0 = dw > dh ?
            neogfx::rect{ iRect.left(), iRect.top(), iRect.left() + aElementSize.cx, iRect.bottom() } :
            neogfx::rect{ iRect.left(), iRect.top(), iRect.right(), iRect.top() + aElementSize.cy };
        neogfx::rect rcChild1 = dw > dh ?
            neogfx::rect{ iRect.left() + aElementSize.cx, iRect.top(), iRect.right(), iRect.bottom() } :
            neogfx::rect{ iRect.left(), iRect.top() + aElementSize.cy, iRect.right(), iRect.bottom() };
        iChildren[0] = iAllocator.allocate(1);
        iChildren[1] = iAllocator.allocate(1);
        iAllocator.construct(iChildren[0], rcChild0, iAllocator);
        iAllocator.construct(iChildren[1], rcChild1, iAllocator);
        return iChildren[0]->insert(aElementSize);
    }

    rect_pack::rect_pack(const size& aDimensions) :
        iRoot{ rect{ point{}, aDimensions }, iAllocator }
    {
    }

    bool rect_pack::insert(const size& aElementSize, rect& aResult)
    {
        auto result = iRoot.insert(aElementSize);
        if (result != nullptr)
        {
            aResult = result->rect();
            return true;
        }
        return false;
    }
}