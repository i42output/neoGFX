// gradient_manager.cpp
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

#include <neogfx/neogfx.hpp>
#include <neogfx/gfx/i_graphics_context.hpp>
#include <neogfx/gfx/gradient_manager.hpp>

std::unique_ptr<neogfx::i_gradient_manager> sGradientManager;

template <>
neogfx::i_gradient_manager& services::start_service<neogfx::i_gradient_manager>()
{
    static bool created = (sGradientManager = std::make_unique<neogfx::gradient_manager>(), true);
    return *sGradientManager;
}

template<> void services::teardown_service<neogfx::i_gradient_manager>()
{
    sGradientManager.reset();
}

namespace neogfx
{
    class gradient_object : public reference_counted<i_gradient>
    {
        // types
    public:
        typedef neolib::pair<scalar, sRGB_color> color_stop;
        typedef neolib::pair<scalar, sRGB_color::view_component> alpha_stop;
        typedef neolib::vector<color_stop> color_stop_list;
        typedef neolib::vector<alpha_stop> alpha_stop_list;
        typedef color_stop_list::abstract_type abstract_color_stop_list;
        typedef alpha_stop_list::abstract_type abstract_alpha_stop_list;
        // construction
    public:
        gradient_object(gradient_id aId) :
            iId{ aId }
        {
            fix();
        }
        gradient_object(gradient_id aId, const i_gradient& aOther) :
            iId{ aId },
            iColorStops{ aOther.color_stops() },
            iAlphaStops{ aOther.alpha_stops() },
            iDirection{ aOther.direction() },
            iOrientation{ aOther.orientation() },
            iShape{ aOther.shape() },
            iSize{ aOther.size() },
            iExponents{ aOther.exponents() },
            iCenter{ aOther.center() },
            iTile{ aOther.tile() },
            iSmoothness{ aOther.smoothness() },
            iBoundingBox{ aOther.bounding_box() }
        {
            fix();
        }
        gradient_object(gradient_id aId, const i_string& aCssDeclaration) :
            gradient_object{ aId, color{} }
        {
            // todo
            fix();
        }
        gradient_object(gradient_id aId, const sRGB_color& aColor) :
            gradient_object{ aId, aColor, gradient_direction::Vertical }
        {
            fix();
        }
        gradient_object(gradient_id aId, const sRGB_color& aColor, gradient_direction aDirection) :
            gradient_object{ aId, aColor, aColor, aDirection }
        {
            fix();
        }
        gradient_object(gradient_id aId, const sRGB_color& aColor1, const sRGB_color& aColor2, gradient_direction aDirection) :
            iId{ aId },
            iColorStops{ color_stop{0.0, aColor1.with_alpha(255_u8)}, color_stop{1.0, aColor2.with_alpha(255_u8)} },
            iAlphaStops{ alpha_stop{0.0, aColor1.alpha()}, alpha_stop{1.0, aColor2.alpha()} },
            iDirection{ aDirection }
        {
            fix();
        }
        gradient_object(gradient_id aId, const abstract_color_stop_list& aColorStops, gradient_direction aDirection) :
            iId{ aId },
            iColorStops{ aColorStops },
            iDirection{ aDirection }
        {
            fix();
        }
        gradient_object(gradient_id aId, const abstract_color_stop_list& aColorStops, const abstract_alpha_stop_list& aAlphaStops, gradient_direction aDirection) :
            iId{ aId },
            iColorStops{ aColorStops },
            iAlphaStops{ aAlphaStops },
            iDirection{ aDirection }
        {
            fix();
        }
        gradient_object(gradient_id aId, const i_gradient& aOther, const abstract_color_stop_list& aColorStops) :
            iId{ aId },
            iColorStops{ aColorStops },
            iDirection{ aOther.direction() },
            iOrientation{ aOther.orientation() },
            iShape{ aOther.shape() },
            iSize{ aOther.size() },
            iExponents{ aOther.exponents() },
            iCenter{ aOther.center() },
            iTile{ aOther.tile() },
            iSmoothness{ aOther.smoothness() },
            iBoundingBox{ aOther.bounding_box() }
        {
            fix();
        }
        gradient_object(gradient_id aId, const i_gradient& aOther, const abstract_color_stop_list& aColorStops, const abstract_alpha_stop_list& aAlphaStops) :
            iId{ aId },
            iColorStops{ aColorStops },
            iAlphaStops{ aAlphaStops },
            iDirection{ aOther.direction() },
            iOrientation{ aOther.orientation() },
            iShape{ aOther.shape() },
            iSize{ aOther.size() },
            iExponents{ aOther.exponents() },
            iCenter{ aOther.center() },
            iTile{ aOther.tile() },
            iSmoothness{ aOther.smoothness() },
            iBoundingBox{ aOther.bounding_box() }
        {
            fix();
        }
        gradient_object(gradient_id aId, const neolib::i_vector<sRGB_color::abstract_type>& aColors, gradient_direction aDirection) :
            iId{ aId },
            iDirection{ aDirection }
        {
            color_stop_list tempColors;
            alpha_stop_list tempAlphas;
            double pos = 0.0;
            for (auto const& c : aColors)
            {
                tempColors.push_back(color_stop{ pos, sRGB_color{c}.with_alpha(255_u8) });
                tempAlphas.push_back(alpha_stop{ pos, sRGB_color{c}.alpha() });
                if (aColors.size() > 1)
                    pos += (1.0 / (aColors.size() - 1));
            }
            color_stops() = tempColors;
            alpha_stops() = tempAlphas;
            fix();
        }
        // assignment
    public:
        gradient_object& operator=(const gradient_object& aOther) = default;
        gradient_object& operator=(const i_gradient& aOther) override
        {
            *this = gradient_object{ aOther.id() };
            return *this;
        }
        // operations
    public:
        void clone(neolib::i_ref_ptr<i_gradient>& aResult) const override
        {
            service<i_gradient_manager>().do_create_gradient(*this, aResult);
        }
        // meta
    public:
        gradient_id id() const override
        {
            return iId;
        }
        bool is_singular() const
        {
            return false;
        }
        // operations
    public:
        color_stop_list const& color_stops() const override
        {
            iFixer();
            return iColorStops;
        }
        color_stop_list& color_stops() override
        {
            if (iSampler)
            {
                iSampler->release(id());
                iSampler = nullptr;
            }
            if (!iInFixer)
            {
                iFixer();
                iColorStopsNeedFixing = true;
            }
            return iColorStops;
        }
        alpha_stop_list const& alpha_stops() const override
        {
            iFixer();
            return iAlphaStops;
        }
        alpha_stop_list& alpha_stops() override
        {
            if (iSampler)
            {
                iSampler->release(id());
                iSampler = nullptr;
            }
            if (!iInFixer)
            {
                iFixer();
                iAlphaStopsNeedFixing = true;
            }
            return iAlphaStops;
        }
        color_stop_list::const_iterator find_color_stop(scalar aPos, bool aToInsert = false) const override
        {
            auto colorStop = std::lower_bound(color_stops().begin(), color_stops().end(), color_stop{ aPos, sRGB_color{} },
                [](const color_stop& aLeft, const color_stop& aRight)
            {
                return aLeft.first() < aRight.first();
            });
            if (colorStop == color_stops().end() && !aToInsert)
                --colorStop;
            return colorStop;
        }
        color_stop_list::const_iterator find_color_stop(scalar aPos, scalar aStart, scalar aEnd, bool aToInsert = false) const override
        {
            return find_color_stop(normalized_position(aPos, aStart, aEnd), aToInsert);
        }
        alpha_stop_list::const_iterator find_alpha_stop(scalar aPos, bool aToInsert = false) const override
        {
            auto alphaStop = std::lower_bound(alpha_stops().begin(), alpha_stops().end(), alpha_stop{ aPos, 255_u8 },
                [](const alpha_stop& aLeft, const alpha_stop& aRight)
            {
                return aLeft.first() < aRight.first();
            });
            if (alphaStop == alpha_stops().end() && !aToInsert)
                --alphaStop;
            return alphaStop;
        }
        alpha_stop_list::const_iterator find_alpha_stop(scalar aPos, scalar aStart, scalar aEnd, bool aToInsert = false) const override
        {
            return find_alpha_stop(normalized_position(aPos, aStart, aEnd), aToInsert);
        }
        color_stop_list::iterator find_color_stop(scalar aPos, bool aToInsert = false) override
        {
            auto colorStop = std::lower_bound(color_stops().begin(), color_stops().end(), color_stop{ aPos, sRGB_color{} },
                [](const color_stop& aLeft, const color_stop& aRight)
            {
                return aLeft.first() < aRight.first();
            });
            if (colorStop == color_stops().end() && !aToInsert)
                --colorStop;
            return colorStop;
        }
        color_stop_list::iterator find_color_stop(scalar aPos, scalar aStart, scalar aEnd, bool aToInsert = false) override
        {
            return find_color_stop(normalized_position(aPos, aStart, aEnd), aToInsert);
        }
        alpha_stop_list::iterator find_alpha_stop(scalar aPos, bool aToInsert = false) override
        {
            auto alphaStop = std::lower_bound(alpha_stops().begin(), alpha_stops().end(), alpha_stop{ aPos, 255_u8 },
                [](const alpha_stop& aLeft, const alpha_stop& aRight)
            {
                return aLeft.first() < aRight.first();
            });
            if (alphaStop == alpha_stops().end() && !aToInsert)
                --alphaStop;
            return alphaStop;
        }
        alpha_stop_list::iterator find_alpha_stop(scalar aPos, scalar aStart, scalar aEnd, bool aToInsert = false) override
        {
            return find_alpha_stop(normalized_position(aPos, aStart, aEnd), aToInsert);
        }
        color_stop_list::iterator insert_color_stop(scalar aPos) override
        {
            return color_stops().insert(find_color_stop(aPos, true), color_stop(aPos, color_at(aPos)));
        }
        color_stop_list::iterator insert_color_stop(scalar aPos, scalar aStart, scalar aEnd) override
        {
            return insert_color_stop(normalized_position(aPos, aStart, aEnd));
        }
        alpha_stop_list::iterator insert_alpha_stop(scalar aPos) override
        {
            return alpha_stops().insert(find_alpha_stop(aPos, true), alpha_stop(aPos, alpha_at(aPos)));
        }
        alpha_stop_list::iterator insert_alpha_stop(scalar aPos, scalar aStart, scalar aEnd) override
        {
            return insert_alpha_stop(normalized_position(aPos, aStart, aEnd));
        }
        sRGB_color at(scalar aPos) const override
        {
            sRGB_color result = color_at(aPos);
            return result.with_combined_alpha(alpha_at(aPos));
        }
        sRGB_color at(scalar aPos, scalar aStart, scalar aEnd) const override
        {
            return at(normalized_position(aPos, aStart, aEnd));
        }
        sRGB_color color_at(scalar aPos) const override
        {
            if (aPos < 0.0 || aPos > 1.0)
                throw bad_position();
            auto colorStop = std::lower_bound(color_stops().begin(), color_stops().end(), color_stop{ aPos, sRGB_color{} },
                [](const color_stop& aLeft, const color_stop& aRight)
            {
                return aLeft.first() < aRight.first();
            });
            if (colorStop == color_stops().end())
                --colorStop;
            if (colorStop->first() >= aPos && colorStop != color_stops().begin())
                --colorStop;
            auto left = colorStop;
            auto right = colorStop + 1;
            if (right == color_stops().end())
                --right;
            aPos = std::min(std::max(left->first(), aPos), right->first());
            double nc = (left != right ? (aPos - left->first()) / (right->first() - left->first()) : 0.0);
            sRGB_color::base_component red{}, green{}, blue{}, alpha{};
            red = lerp(left->second()[0], right->second()[0], nc);
            green = lerp(left->second()[1], right->second()[1], nc);
            blue = lerp(left->second()[2], right->second()[2], nc);
            alpha = lerp(left->second()[3], right->second()[3], nc);
            return sRGB_color{ red, green, blue, alpha };
        }
        sRGB_color color_at(scalar aPos, scalar aStart, scalar aEnd) const override
        {
            return color_at(normalized_position(aPos, aStart, aEnd));
        }
        sRGB_color::view_component alpha_at(scalar aPos) const override
        {
            auto alphaStop = std::lower_bound(alpha_stops().begin(), alpha_stops().end(), alpha_stop{ aPos, 255_u8 },
                [](const alpha_stop& aLeft, const alpha_stop& aRight)
            {
                return aLeft.first() < aRight.first();
            });
            if (alphaStop == alpha_stops().end())
                --alphaStop;
            if (alphaStop->first() >= aPos && alphaStop != alpha_stops().begin())
                --alphaStop;
            auto left = alphaStop;
            auto right = alphaStop + 1;
            if (right == alpha_stops().end())
                --right;
            aPos = std::min(std::max(left->first(), aPos), right->first());
            double na = (left != right ? (aPos - left->first()) / (right->first() - left->first()) : 0.0);
            sRGB_color::view_component alpha = static_cast<sRGB_color::view_component>((lerp(left->second(), right->second(), na) / 255.0) * 255.0);
            return alpha;
        }
        sRGB_color::view_component alpha_at(scalar aPos, scalar aStart, scalar aEnd) const override
        {
            return alpha_at(normalized_position(aPos, aStart, aEnd));
        }
        i_gradient& reverse() override
        {
            auto reversedGradient = *this;
            std::reverse(reversedGradient.color_stops().begin(), reversedGradient.color_stops().end());
            std::reverse(reversedGradient.alpha_stops().begin(), reversedGradient.alpha_stops().end());
            for (auto& s : reversedGradient.color_stops())
                s.first() = 1.0 - s.first();
            for (auto& s : reversedGradient.alpha_stops())
                s.first() = 1.0 - s.first();
            *this = reversedGradient;
            return *this;
        }
        i_gradient& set_alpha(sRGB_color::view_component aAlpha) override
        {
            alpha_stops() = alpha_stop_list{ alpha_stop{0.0, aAlpha}, alpha_stop{1.0, aAlpha} };
            for (auto& stop : color_stops())
                stop.second()[3] = 1.0;
            return *this;
        }
        i_gradient& set_combined_alpha(sRGB_color::view_component aAlpha) override
        {
            for (auto& stop : alpha_stops())
                stop.second() = static_cast<sRGB_color::view_component>((stop.second() / 255.0 * aAlpha / 255.0) * 255.0);
            return *this;
        }
        gradient_direction direction() const override
        {
            return iDirection;
        }
        i_gradient& set_direction(gradient_direction aDirection) override
        {
            iDirection = aDirection;
            if (iDirection != gradient_direction::Diagonal)
                iOrientation = corner::TopLeft;
            if (iDirection != gradient_direction::Radial)
            {
                iExponents = optional_vec2{};
                iCenter = optional_point{};
            }
            return *this;
        }
        gradient_orientation orientation() const override
        {
            return iOrientation;
        }
        i_gradient& set_orientation(gradient_orientation aOrientation) override
        {
            iOrientation = aOrientation;
            return *this;
        }
        gradient_shape shape() const override
        {
            return iShape;
        }
        i_gradient& set_shape(gradient_shape aShape) override
        {
            iShape = aShape;
            return *this;
        }
        gradient_size size() const override
        {
            return iSize;
        }
        i_gradient& set_size(gradient_size aSize) override
        {
            iSize = aSize;
            return *this;
        }
        const optional_vec2& exponents() const override
        {
            return iExponents;
        }
        i_gradient& set_exponents(const optional_vec2& aExponents) override
        {
            iExponents = aExponents;
            if (iExponents != optional_vec2{})
                iExponents = iExponents->max(vec2{ 0.0, 0.0 });
            return *this;
        }
        const optional_point& center() const override
        {
            return iCenter;
        }
        i_gradient& set_center(const optional_point& aCenter) override
        {
            iCenter = aCenter;
            if (iCenter != optional_point{})
                iCenter = iCenter->min(point{ 1.0, 1.0 }).max(point{ -1.0, -1.0 });
            return *this;
        }
        const std::optional<gradient_tile>& tile() const override
        {
            return iTile;
        }
        i_gradient& set_tile(const std::optional<gradient_tile>& aTile) override
        {
            iTile = aTile;
            return *this;
        }
        scalar smoothness() const override
        {
            return iSmoothness;
        }
        i_gradient& set_smoothness(scalar aSmoothness) override
        {
            iSmoothness = aSmoothness;
            return *this;
        }
        const optional_rect& bounding_box() const override
        {
            return iBoundingBox;
        }
        i_gradient& set_bounding_box(const optional_rect& aBoundingBox) override
        {
            iBoundingBox = aBoundingBox;
            return *this;
        }
        i_gradient& set_bounding_box_if_none(const optional_rect& aBoundingBox) override
        {
            if (iBoundingBox != std::nullopt)
                iBoundingBox = aBoundingBox;
            return *this;
        }
        // shader
    public:
        const i_gradient_sampler& colors() const override
        {
            iFixer();
            if (iSampler && iSampler->used_by(id()))
                return *iSampler;
            return *(iSampler = &service<i_gradient_manager>().sampler(*this));
        }
        const i_gradient_filter& filter() const override
        {
            return service<i_gradient_manager>().filter(*this);
        }
        // object
    public:
        void share_object(i_ref_ptr<i_gradient>& aRef) const override
        {
            service<i_gradient_manager>().do_find_gradient(id(), aRef);
        }
        // implementation
    private:
        void fix() 
        {
            if (iInFixer)
                return;
            neolib::scoped_flag sf{ iInFixer };
            if (iColorStopsNeedFixing)
            {
                iColorStopsNeedFixing = false;
                if (color_stops().empty())
                    color_stops() = color_stop_list{ color_stop{ 0.0, sRGB_color::Black }, color_stop{ 1.0, sRGB_color::Black } };
                else if (color_stops().size() == 1)
                {
                    auto stop = color_stops().back();
                    color_stops().push_back(stop);
                    if (color_stops()[0].first() < 1.0)
                        color_stops()[1].first() = 1.0;
                    else
                        color_stops()[0].first() = 0.0;
                }
            }
            if (iAlphaStopsNeedFixing)
            {
                iAlphaStopsNeedFixing = false;
                if (alpha_stops().empty())
                    alpha_stops() = alpha_stop_list{ { 0.0, 255_u8 }, { 1.0, 255_u8 } };
                else if (alpha_stops().size() == 1)
                {
                    auto stop = alpha_stops().back();
                    alpha_stops().push_back(stop);
                    if (alpha_stops()[0].first() < 1.0)
                        alpha_stops()[1].first() = 1.0;
                    else
                        alpha_stops()[0].first() = 0.0;
                }
            }
        }
        // state
    private:
        gradient_id iId;
        color_stop_list iColorStops;
        alpha_stop_list iAlphaStops;
        gradient_direction iDirection = gradient_direction::Vertical;
        gradient_orientation iOrientation = corner::TopLeft;
        gradient_shape iShape = gradient_shape::Ellipse;
        gradient_size iSize = gradient_size::FarthestCorner;
        optional_vec2 iExponents;
        optional_point iCenter;
        std::optional<gradient_tile> iTile;
        scalar iSmoothness = 0.0;
        optional_rect iBoundingBox;
        mutable const i_gradient_sampler* iSampler = nullptr;
        mutable bool iColorStopsNeedFixing = true;
        mutable bool iAlphaStopsNeedFixing = true;
        bool iInFixer = false;
        std::function<void()> iFixer = [&]() { fix(); };
    };

    neolib::cookie item_cookie(gradient_manager::gradient_list_entry const& aEntry)
    {
        return aEntry.first()->id();
    }

    gradient_manager::gradient_manager()
    {
    }

    void gradient_manager::clear_gradients()
    {
        gradients().clear();
    }

    i_gradient_sampler const& gradient_manager::sampler(i_gradient const& aGradient)
    {
        thread_local std::pair<gradient::color_stop_list, gradient::alpha_stop_list> key;
        key.first = aGradient.color_stops();
        key.second = aGradient.alpha_stops();
        auto allocated = iAllocatedSamplers.find(key);
        if (allocated == iAllocatedSamplers.end())
        {
            if (!free_samplers().empty())
            {
                allocated = iAllocatedSamplers.insert(std::make_pair(key, free_samplers().back())).first;
                free_samplers().pop_back();
            }
            else
            {
                allocated = iAllocatedSamplers.insert(std::make_pair(key, iSamplerQueue.front()->second)).first;
                iAllocatedSamplers.erase(iSamplerQueue.front());
                iSamplerQueue.pop_front();
            }
            allocated->second.release_all();
            avec4u8 colorValues[i_gradient::MaxStops];
            auto const cx = static_cast<uint32_t>(samplers().data().extents().cx);
            for (uint32_t x = 0u; x < cx; ++x)
            {
                auto const color = aGradient.at(x, 0u, cx - 1u);
                colorValues[x] = avec4u8{ color.red(), color.green(), color.blue(), color.alpha() };
            }
            samplers().data().set_pixels(rect{ basic_point<uint32_t>{ 0u, allocated->second.sampler_row() }, size_u32{ i_gradient::MaxStops, 1u } }, &colorValues[0]);
        }
        auto existingQueueEntry = std::find(iSamplerQueue.begin(), iSamplerQueue.end(), allocated);
        if (existingQueueEntry != iSamplerQueue.end())
            iSamplerQueue.erase(existingQueueEntry);
        iSamplerQueue.push_back(allocated);
        allocated->second.add_ref(aGradient.id());
        return allocated->second;
    }

    i_gradient_filter const& gradient_manager::filter(i_gradient const& aGradient)
    {
        scalar const key{ aGradient.smoothness()};
        auto allocated = iAllocatedFilters.find(key);
        if (allocated == iAllocatedFilters.end())
        {
            if (!free_filters().empty())
            {
                allocated = iAllocatedFilters.insert(std::make_pair(key, free_filters().back())).first;
                free_filters().pop_back();
            }
            else
            {
                allocated = iAllocatedFilters.insert(std::make_pair(key, iFilterQueue.front()->second)).first;
                iAllocatedFilters.erase(iFilterQueue.front());
                iFilterQueue.pop_front();
            }
            auto const filterValues = static_gaussian_filter<float, GRADIENT_FILTER_SIZE>(static_cast<float>(aGradient.smoothness() * 10.0));
            allocated->second.sampler().data().set_pixels(rect{ point{}, size_u32{ GRADIENT_FILTER_SIZE, GRADIENT_FILTER_SIZE } }, &filterValues[0][0]);
        }
        auto existingQueueEntry = std::find(iFilterQueue.begin(), iFilterQueue.end(), allocated);
        if (existingQueueEntry != iFilterQueue.end())
            iFilterQueue.erase(existingQueueEntry);
        iFilterQueue.push_back(allocated);
        return allocated->second;
    }

    void gradient_manager::add_ref(gradient_id aId)
    {
        ++gradients()[aId].second();
    }

    void gradient_manager::release(gradient_id aId)
    {
        if (gradients()[aId].second() == 0u)
            throw invalid_release();
        if (--gradients()[aId].second() == 0u)
        {
            if (gradients()[aId].first().use_count() <= 1)
                gradients().remove(aId);
        }
    }

    long gradient_manager::use_count(gradient_id aId) const
    {
        return gradients()[aId].second();
    }

    gradient_id gradient_manager::allocate_gradient_id()
    {
        return gradients().next_cookie();
    }

    const gradient_manager::gradient_list& gradient_manager::gradients() const
    {
        return iGradients;
    }

    gradient_manager::gradient_list& gradient_manager::gradients()
    {
        return iGradients;
    }

    neolib::ref_ptr<i_gradient> gradient_manager::add_gradient(neolib::ref_ptr<i_gradient> aGradient)
    {
        // cleanup opportunity
        cleanup();
        return gradients().add(aGradient->id(), gradient_list_entry{ aGradient, 0u })->first();
    }

    void gradient_manager::do_find_gradient(gradient_id aId, neolib::i_ref_ptr<i_gradient>& aResult) const
    {
        aResult = gradients()[aId].first();
    }

    void gradient_manager::do_create_gradient(neolib::i_ref_ptr<i_gradient>& aResult)
    {
        aResult = add_gradient(make_ref<gradient_object>(allocate_gradient_id()));
    }

    void gradient_manager::do_create_gradient(i_gradient const& aOther, neolib::i_ref_ptr<i_gradient>& aResult)
    {
        aResult = add_gradient(make_ref<gradient_object>(allocate_gradient_id(), aOther));
    }

    void gradient_manager::do_create_gradient(i_string const& aCssDeclaration, neolib::i_ref_ptr<i_gradient>& aResult)
    {
        aResult = add_gradient(make_ref<gradient_object>(allocate_gradient_id(), aCssDeclaration));
    }

    void gradient_manager::do_create_gradient(sRGB_color const& aColor, neolib::i_ref_ptr<i_gradient>& aResult)
    {
        aResult = add_gradient(make_ref<gradient_object>(allocate_gradient_id(), aColor));
    }

    void gradient_manager::do_create_gradient(sRGB_color const& aColor, gradient_direction aDirection, neolib::i_ref_ptr<i_gradient>& aResult)
    {
        aResult = add_gradient(make_ref<gradient_object>(allocate_gradient_id(), aColor, aDirection));
    }

    void gradient_manager::do_create_gradient(sRGB_color const& aColor1, sRGB_color const& aColor2, gradient_direction aDirection, neolib::i_ref_ptr<i_gradient>& aResult)
    {
        aResult = add_gradient(make_ref<gradient_object>(allocate_gradient_id(), aColor1, aColor2, aDirection));
    }

    void gradient_manager::do_create_gradient(i_gradient::color_stop_list const& aColorStops, gradient_direction aDirection, neolib::i_ref_ptr<i_gradient>& aResult)
    {
        aResult = add_gradient(make_ref<gradient_object>(allocate_gradient_id(), aColorStops, aDirection));
    }

    void gradient_manager::do_create_gradient(i_gradient::color_stop_list const& aColorStops, i_gradient::alpha_stop_list const& aAlphaStops, gradient_direction aDirection, neolib::i_ref_ptr<i_gradient>& aResult)
    {
        aResult = add_gradient(make_ref<gradient_object>(allocate_gradient_id(), aColorStops, aAlphaStops, aDirection));
    }

    void gradient_manager::do_create_gradient(i_gradient const& aOther, i_gradient::color_stop_list const& aColorStops, neolib::i_ref_ptr<i_gradient>& aResult)
    {
        aResult = add_gradient(make_ref<gradient_object>(allocate_gradient_id(), aOther, aColorStops));
    }

    void gradient_manager::do_create_gradient(i_gradient const& aOther, i_gradient::color_stop_list const& aColorStops, i_gradient::alpha_stop_list const& aAlphaStops, neolib::i_ref_ptr<i_gradient>& aResult)
    {
        aResult = add_gradient(make_ref<gradient_object>(allocate_gradient_id(), aOther, aColorStops, aAlphaStops));
    }

    void gradient_manager::do_create_gradient(neolib::i_vector<sRGB_color::abstract_type> const& aColors, gradient_direction aDirection, neolib::i_ref_ptr<i_gradient>& aResult)
    {
        aResult = add_gradient(make_ref<gradient_object>(allocate_gradient_id(), aColors, aDirection));
    }

    shader_array<avec4u8>& gradient_manager::samplers()
    {
        if (iSamplers == std::nullopt)
            iSamplers.emplace(size_u32{ gradient::MaxStops, MaxSamplers });
        return *iSamplers;
    }

    std::vector<gradient_sampler>& gradient_manager::free_samplers()
    {
        if (iFreeSamplers == std::nullopt)
        {
            iFreeSamplers.emplace();
            for (uint32_t row = 0; row < MaxSamplers; ++row)
                free_samplers().emplace_back(samplers(), row);
        }
        return *iFreeSamplers;
    }

    std::vector<gradient_filter>& gradient_manager::free_filters()
    {
        if (iFreeFilters == std::nullopt)
        {
            iFreeFilters.emplace();
            for (uint32_t filter = 0; filter < MaxFilters; ++filter)
                free_filters().emplace_back();
        }
        return *iFreeFilters;
    }

    void gradient_manager::cleanup()
    {
        for (auto i = gradients().begin(); i != gradients().end();)
        {
            auto& gradient = *i;
            if (gradient.first().use_count() <= 1 && gradient.second() == 0u)
                i = gradients().erase(i);
            else
                ++i;
        }
    }
}