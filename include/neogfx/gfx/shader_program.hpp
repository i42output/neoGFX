// shader_program.hpp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2019, 2020 Leigh Johnston.  All Rights Reserved.
  
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
#include <neolib/core/map.hpp>
#include <neolib/core/reference_counted.hpp>

#include <neogfx/gfx/i_shader_program.hpp>

namespace neogfx
{
    template <typename T>
    class ssbo : public reference_counted<i_ssbo>
    {
    public:
        using value_type = T;
    public:
        ssbo(ssbo_id aId, i_shader_uniform& aSizeUniform) :
            iId{ aId }, iSizeUniform{ aSizeUniform }
        {
            iSizeUniform.set_value<std::uint32_t>(0u);
        }
    public:
        ssbo_id id() const final
        {
            return iId;
        }
        shader_data_type data_type() const final
        {
            return shader_data_type_v<T>;
        }
    public:
        void reserve(std::size_t aCapacity) override
        {
            iCapacity = std::max(aCapacity, iCapacity);
        }
        std::size_t capacity() const final
        {
            return iCapacity;
        }
        bool empty() const final
        {
            return iSize == 0u;
        }
        std::size_t size() const final
        {
            return iSize;
        }
    public:
        void const* back(shader_data_type aDataType) const final
        {
            if (aDataType != this->data_type())
                throw std::logic_error("neogfx::ssbo::back: invalid data type");
            if (empty())
                throw std::logic_error("neogfx::ssbo::back: empty");
            return static_cast<value_type const*>(cdata()) + (size() - 1);
        }
        void* back(shader_data_type aDataType) final
        {
            if (aDataType != this->data_type())
                throw std::logic_error("neogfx::ssbo::back: invalid data type");
            if (empty())
                throw std::logic_error("neogfx::ssbo::back: empty");
            return static_cast<value_type*>(data()) + (size() - 1);
        }
        void const* at(shader_data_type aDataType, std::size_t aIndex) const final
        {
            if (aDataType != this->data_type())
                throw std::logic_error("neogfx::ssbo::at: invalid data type");
            if (aIndex >= size())
                throw std::logic_error("neogfx::ssbo::at: out of range");
            return static_cast<value_type const*>(cdata()) + aIndex;
        }
        void* at(shader_data_type aDataType, std::size_t aIndex) final
        {
            if (aDataType != this->data_type())
                throw std::logic_error("neogfx::ssbo::at: invalid data type");
            if (aIndex >= size())
                throw std::logic_error("neogfx::ssbo::at: out of range");
            return static_cast<value_type*>(data()) + aIndex;
        }
    public:
        void clear() final
        {
            resize(0);
        }
        void resize(std::size_t aSize)
        {
            if (aSize > capacity())
                need(aSize - capacity());
            iSizeUniform.set_value(static_cast<std::uint32_t>(aSize));
            iSize = aSize;
        }
        void push_back(shader_data_type aDataType, void const* aValue) final
        {
            if (aDataType != this->data_type())
                throw std::logic_error("neogfx::ssbo::push_back: invalid data type");
            resize(size() + 1);
            scoped_ssbo_map ssm{ *this };
            std::copy(
                static_cast<value_type const*>(aValue),
                static_cast<value_type const*>(aValue) + 1,
                static_cast<value_type*>(data()) + size() - 1);
        }
        void* insert(shader_data_type aDataType, std::size_t aPos, void const* aFirst, void const* aLast) final
        {
            if (aDataType != this->data_type())
                throw std::logic_error("neogfx::ssbo::insert: invalid data type");
            auto const first = static_cast<value_type const*>(aFirst);
            auto const last = static_cast<value_type const*>(aLast);
            auto const count = (last - first);
            if (count != 0)
            {
                resize(size() + count);
                {
                    scoped_ssbo_map ssm{ *this };
                    auto const where = static_cast<value_type*>(data()) + aPos;
                    std::copy_backward(
                        where,
                        static_cast<value_type*>(data()) + size() - count,
                        static_cast<value_type*>(data()) + size());
                    std::copy(
                        static_cast<value_type const*>(first),
                        static_cast<value_type const*>(last),
                        where);
                }
            }
            return mapped() ? static_cast<value_type*>(data()) + aPos : nullptr;
        }
        void* erase(void const* aFirst, void const* aLast) final
        {
            auto const where = static_cast<value_type*>(data()) +
                (static_cast<value_type const*>(aFirst) - static_cast<value_type const*>(cdata()));
            auto const count = (static_cast<T const*>(aLast) - static_cast<T const*>(aFirst));
            if (count != 0)
            {
                std::copy(
                    static_cast<value_type const*>(aLast),
                    static_cast<value_type const*>(cdata()) + size(),
                    where);
                resize(size() - count);
            }
            return where;
        }
    private:
        void need(std::size_t aExtra)
        {
            if (size() + aExtra > capacity())
                reserve(static_cast<std::size_t>(capacity() * 1.5 + aExtra));
        }
    private:
        ssbo_id iId;
        i_shader_uniform& iSizeUniform;
        std::size_t iCapacity = 0u;
        std::size_t iSize = 0u;
    };

    template <typename Base = i_shader_stage>
    class shader_stage : public reference_counted<Base>
    {
    public:
        using i_shader_stage::i_shader_t;
        using i_shader_stage::i_shaders_t;
        typedef neolib::ref_ptr<i_shader> shader_t;
        typedef neolib::vector<shader_t> shaders_t;
    public:
        shader_stage(shader_type aType);
    public:
        shader_type type() const override;
        shaders_t const& shaders() const override;
        shaders_t& shaders() override;
    private:
        shader_type iType;
        shaders_t iShaders;
    };
        
    template <typename Base = i_shader_program>
    class shader_program : public reference_counted<Base>
    {
    public:
        using i_shader_program::i_stage_t;
        using i_shader_program::i_stages_t;
        typedef neolib::ref_ptr<i_shader_stage> stage_t;
        typedef neolib::vector<stage_t> stages_t;
        typedef neolib::ref_ptr<i_shader> shader_t;
    public:
        using i_shader_program::have_stage;
        using i_shader_program::compile;
        using i_shader_program::link;
        using i_shader_program::use;
        using i_shader_program::update_uniform_storage;
        using i_shader_program::update_uniform_locations;
        using i_shader_program::update_uniforms;
    private:
        typedef std::map<neolib::string, shader_t> shader_index;
    public:
        shader_program(const std::string aName);
        ~shader_program();
    public:
        const i_string& name() const override;
        bool supports(vertex_buffer_type aBufferType) const override;
        bool created() const override;
        void* handle() const override;
        const stages_t& stages() const override;
        stages_t& stages() override;
        const stage_t& stage(shader_type aStage) const override;
        stage_t& stage(shader_type aStage) override;
        const i_shader& shader(const neolib::i_string& aName) const override;
        i_shader& shader(const neolib::i_string& aName) override;
        const i_vertex_shader& vertex_shader() const override;
        i_vertex_shader& vertex_shader() override;
        const i_fragment_shader& fragment_shader() const override;
        i_fragment_shader& fragment_shader() override;
        bool is_first_in_stage(const i_shader& aShader) const override;
        bool is_last_in_stage(const i_shader& aShader) const override;
        const i_shader& first_in_stage(shader_type aStage) const override;
        const i_shader& next_in_stage(const i_shader& aPreviousShader) const override;
        const i_shader& last_in_stage(shader_type aStage) const override;
        i_shader& add_shader(const neolib::i_ref_ptr<i_shader>& aShader) override;
        bool dirty() const override;
        void set_clean() override;
        void prepare_uniforms(const i_rendering_context& aContext) override;
        void make() override;
        bool uniforms_changed() const override;
        bool active() const override;
        void activate(const i_rendering_context& aContext) override;
        void instantiate(const i_rendering_context& aContext) override;
    protected:
        bool need_full_uniform_update() const;
        void set_active();
        void set_inactive();
    private:
        string iName;
        mutable std::optional<void*> iHandle;
        stages_t iStages;
        shader_index iShaderIndex;
        mutable bool iNeedFullUniformUpdate;
        bool iActive;
    };
}

#include "shader_program.inl"