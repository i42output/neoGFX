// gradient_manager.hpp
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
#include <unordered_set>
#include <neogfx/gfx/shader_array.hpp>
#include <neogfx/gfx/gradient.hpp>
#include <neogfx/gfx/i_gradient_manager.hpp>

namespace neogfx
{
    class gradient_sampler : public i_gradient_sampler
    {
        friend class gradient_manager;
    public:
        gradient_sampler(i_shader_array<avec4u8> const& aSampler, uint32_t aRow) : 
            iSampler{ &aSampler },
            iRow{ aRow }
        {
        }
    public:
        i_shader_array<avec4u8> const& sampler() const override
        {
            return *iSampler;
        }
        uint32_t sampler_row() const override
        {
            return iRow;
        }
    public:
        bool used_by(gradient_id aGradient) const override
        {
            return iReferences.find(aGradient) != iReferences.end();
        }
        void add_ref(gradient_id aGradient) const override
        {
            iReferences.insert(aGradient);
        }
        void release(gradient_id aGradient) const override
        {
            auto existing = iReferences.find(aGradient);
            if (existing != iReferences.end())
                iReferences.erase(existing);
        }
        void release_all() const override
        {
            iReferences.clear();
        }
    private:
        i_shader_array<avec4u8> const* iSampler;
        uint32_t iRow;
        mutable std::unordered_set<gradient_id> iReferences;
    };

    constexpr uint32_t GRADIENT_FILTER_SIZE = 15;

    class gradient_filter : public i_gradient_filter
    {
        friend class gradient_manager;
    public:
        gradient_filter() :
            iSampler{ std::make_shared<shader_array<float>>(size_u32{ GRADIENT_FILTER_SIZE, GRADIENT_FILTER_SIZE }) }
        {
        }
    public:
        i_shader_array<float> const& sampler() const override
        {
            return *iSampler;
        }
    private:
        i_shader_array<float>& sampler() override
        {
            return *iSampler;
        }
    private:
        std::shared_ptr<shader_array<float>> iSampler;
    };

    class gradient_manager : public i_gradient_manager
    {
        friend class gradient_object;
        // types
    protected:
        typedef ref_ptr<i_gradient> gradient_pointer;
        typedef neolib::pair<gradient_pointer, uint32_t> gradient_list_entry;
        typedef neolib::jar<gradient_list_entry> gradient_list;
        typedef std::pair<gradient::color_stop_list, gradient::alpha_stop_list> sampler_key_t;
        typedef std::map<sampler_key_t, gradient_sampler> sampler_map_t;
        typedef std::map<scalar, gradient_filter> filter_map_t;
        // constants
    public:
        static constexpr uint32_t MaxSamplers = 1024;
        static constexpr uint32_t MaxFilters = 16;
        // construction
    public:
        gradient_manager();
        // operations
    public:
        void clear_gradients() override;
        i_gradient_sampler const& sampler(i_gradient const& aGradient) override;
        i_gradient_filter const& filter(i_gradient const& aGradient) override;
        // implementation
    protected:
        friend neolib::cookie item_cookie(gradient_list_entry const&);
        void add_ref(gradient_id aId) override;
        void release(gradient_id aId) override;
        long use_count(gradient_id aId) const override;
    protected:
        gradient_id allocate_gradient_id();
        const gradient_list& gradients() const;
        gradient_list& gradients();
        neolib::ref_ptr<i_gradient> add_gradient(neolib::ref_ptr<i_gradient> aGradient);
    private:
        void do_find_gradient(gradient_id aId, neolib::i_ref_ptr<i_gradient>& aResult) const override;
        void do_create_gradient(neolib::i_ref_ptr<i_gradient>& aResult) override;
        void do_create_gradient(i_gradient const& aOther, neolib::i_ref_ptr<i_gradient>& aResult) override;
        void do_create_gradient(i_string const& aCssDeclaration, neolib::i_ref_ptr<i_gradient>& aResult) override;
        void do_create_gradient(sRGB_color const& aColor, neolib::i_ref_ptr<i_gradient>& aResult) override;
        void do_create_gradient(sRGB_color const& aColor, gradient_direction aDirection, neolib::i_ref_ptr<i_gradient>& aResult) override;
        void do_create_gradient(sRGB_color const& aColor1, sRGB_color const& aColor2, gradient_direction aDirection, neolib::i_ref_ptr<i_gradient>& aResult) override;
        void do_create_gradient(i_gradient::color_stop_list const& aColorStops, gradient_direction aDirection, neolib::i_ref_ptr<i_gradient>& aResult) override;
        void do_create_gradient(i_gradient::color_stop_list const& aColorStops, i_gradient::alpha_stop_list const& aAlphaStops, gradient_direction aDirection, neolib::i_ref_ptr<i_gradient>& aResult) override;
        void do_create_gradient(i_gradient const& aOther, i_gradient::color_stop_list const& aColorStops, neolib::i_ref_ptr<i_gradient>& aResult) override;
        void do_create_gradient(i_gradient const& aOther, i_gradient::color_stop_list const& aColorStops, i_gradient::alpha_stop_list const& aAlphaStops, neolib::i_ref_ptr<i_gradient>& aResult) override;
        void do_create_gradient(neolib::i_vector<sRGB_color::abstract_type> const& aColors, gradient_direction aDirection, neolib::i_ref_ptr<i_gradient>& aResult) override;
    private:
        shader_array<avec4u8>& samplers();
        std::vector<gradient_sampler>& free_samplers();
        std::vector<gradient_filter>& free_filters();
        void cleanup();
    private:
        gradient_list iGradients;
        std::optional<shader_array<avec4u8>> iSamplers;
        sampler_map_t iAllocatedSamplers;
        std::optional<std::vector<gradient_sampler>> iFreeSamplers;
        std::deque<sampler_map_t::const_iterator> iSamplerQueue;
        filter_map_t iAllocatedFilters;
        std::optional<std::vector<gradient_filter>> iFreeFilters;
        std::deque<filter_map_t::const_iterator> iFilterQueue;
    };
}