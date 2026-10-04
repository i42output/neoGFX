// fragment_shader.cpp
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

#include <neogfx/neogfx.hpp>

#include <cmath>
#include <thread>
#include <atomic>

#include <neogfx/gfx/fragment_shader.hpp>
#include "standard-gradient.frag.hpp"
#include "standard-texture.frag.hpp"
#include "standard-pbr.frag.hpp"
#include "standard-filter.frag.hpp"
#include "standard-glyph.frag.hpp"
#include "standard-stipple.frag.hpp"
#include "standard-shape.frag.hpp"

namespace neogfx
{ 
    standard_gradient_shader::standard_gradient_shader(std::string const& aName) :
        standard_fragment_shader{ aName }
    {
        disable();
        // the shader refers to the filter gradient whether one is set or not, and a cached uniform
        // only comes into being, with a type, once it has been given a value
        uFilterGradientEnabled = false;
        uFilterGradientGuiCoordinates = false;
        uFilterGradientDirection = gradient_direction::Horizontal;
        uFilterGradientAngle = 0.0f;
        uFilterGradientStartFrom = -1;
        uFilterGradientSize = gradient_size::ClosestSide;
        uFilterGradientShape = gradient_shape::Ellipse;
        uFilterGradientExponents = vec2f{ 2.0f, 2.0f };
        uFilterGradientCenter = vec2f{};
        uFilterGradientTile = false;
        uFilterGradientTileParams = vec3i32{};
        uFilterGradientColorCount = 0;
        uFilterGradientColorRow = 0;
        uFilterGradientBoundingBox = vec4f{};
    }

    void standard_gradient_shader::generate_code(i_shader_program const& aProgram, shader_language aLanguage, i_string& aOutput) const
    {
        standard_fragment_shader<i_gradient_shader>::generate_code(aProgram, aLanguage, aOutput);
        if (aLanguage == shader_language::Glsl)
            aOutput += string{ glsl::StandardGradientFragmentShader };
        else
            throw unsupported_shader_language();
    }

    void standard_gradient_shader::clear_gradient()
    {
        uGradientEnabled = false;
    }

    void standard_gradient_shader::clear_filter_gradient()
    {
        uFilterGradientEnabled = false;
    }

    void standard_gradient_shader::set_filter_gradient(i_rendering_context& aContext, gradient const& aGradient)
    {
        // a filter has to bring its own bounding box: the one the gradient above is evaluated
        // against arrives with the vertices of whatever is being drawn
        if (aGradient.bounding_box() == std::nullopt)
        {
            clear_filter_gradient();
            return;
        }
        enable();
        uFilterGradientGuiCoordinates = aContext.logical_coordinates().is_gui_orientation();
        uFilterGradientDirection = aGradient.direction();
        uFilterGradientAngle = std::holds_alternative<double>(aGradient.orientation()) ? static_cast<float>(static_variant_cast<double>(aGradient.orientation())) : 0.0f;
        uFilterGradientStartFrom = std::holds_alternative<corner>(aGradient.orientation()) ? static_cast<int>(static_variant_cast<corner>(aGradient.orientation())) : -1;
        uFilterGradientSize = aGradient.size();
        uFilterGradientShape = aGradient.shape();
        uFilterGradientExponents = (aGradient.exponents() != std::nullopt ? aGradient.exponents()->as<float>() : vec2f{ 2.0f, 2.0f });
        basic_point<float> const gradientCenter = (aGradient.center() != std::nullopt ? *aGradient.center() : point{});
        uFilterGradientCenter = vec2f{ gradientCenter.x, gradientCenter.y };
        uFilterGradientTile = (aGradient.tile() != std::nullopt);
        if (aGradient.tile() != std::nullopt)
            uFilterGradientTileParams = vec3{ aGradient.tile()->extents.cx, aGradient.tile()->extents.cy, aGradient.tile()->aligned ? 1.0 : 0.0 }.as<std::int32_t>();
        else
            uFilterGradientTileParams = vec3i32{};
        auto const& boundingBox = *aGradient.bounding_box();
        uFilterGradientBoundingBox = vec4f{
            static_cast<float>(boundingBox.left()), static_cast<float>(boundingBox.top()),
            static_cast<float>(boundingBox.right()), static_cast<float>(boundingBox.bottom()) };
        auto const& colorsSampler = aGradient.colors().sampler();
        uFilterGradientColorCount = static_cast<int>(colorsSampler.data().extents().cx);
        uFilterGradientColorRow = static_cast<int>(aGradient.colors().sampler_row());
        colorsSampler.data().bind(static_cast<std::uint32_t>(reserved_texture_unit::ColorSampler));
        uGradientColors = sampler2DRect{ static_cast<std::uint32_t>(reserved_texture_unit::ColorSampler) };
        uFilterGradientEnabled = true;
    }

    void standard_gradient_shader::set_gradient(i_rendering_context& aContext, gradient const& aGradient)
    {
        enable();
        uGradientGuiCoordinates = aContext.logical_coordinates().is_gui_orientation();
        uGradientDirection = aGradient.direction();
        uGradientAngle = std::holds_alternative<double>(aGradient.orientation()) ? static_cast<float>(static_variant_cast<double>(aGradient.orientation())) : 0.0f;
        uGradientStartFrom = std::holds_alternative<corner>(aGradient.orientation()) ? static_cast<int>(static_variant_cast<corner>(aGradient.orientation())) : -1;
        uGradientSize = aGradient.size();
        uGradientShape = aGradient.shape();
        uGradientExponents = (aGradient.exponents() != std::nullopt ? aGradient.exponents()->as<float>() : vec2f{2.0f, 2.0f});
        basic_point<float> const gradientCenter = (aGradient.center() != std::nullopt ? *aGradient.center() : point{});
        uGradientCenter = vec2f{ gradientCenter.x, gradientCenter.y };
        uGradientTile = (aGradient.tile() != std::nullopt);
        if (aGradient.tile() != std::nullopt)
            uGradientTileParams = vec3{ aGradient.tile()->extents.cx, aGradient.tile()->extents.cy, aGradient.tile()->aligned ? 1.0 : 0.0 }.as<std::int32_t>();
        else
            uGradientTileParams = vec3i32{};
        auto const& colorsSampler = aGradient.colors().sampler();
        auto const& filterSampler = aGradient.filter().sampler();
        uGradientColorCount = static_cast<int>(colorsSampler.data().extents().cx);
        uGradientColorRow = static_cast<int>(aGradient.colors().sampler_row());
        uGradientFilterSize = (aGradient.smoothness() == 0.0 ? 1 : static_cast<int>(filterSampler.data().extents().cx));
        colorsSampler.data().bind(static_cast<std::uint32_t>(reserved_texture_unit::ColorSampler));
        filterSampler.data().bind(static_cast<std::uint32_t>(reserved_texture_unit::FilterSampler));
        uGradientColors = sampler2DRect{ static_cast<std::uint32_t>(reserved_texture_unit::ColorSampler) };
        uGradientFilter = sampler2DRect{ static_cast<std::uint32_t>(reserved_texture_unit::FilterSampler) };
        uGradientEnabled = true;
    }

    void standard_gradient_shader::set_gradient(i_rendering_context& aContext,  game::gradient const& aGradient)
    {
        gradient g = service<i_gradient_manager>().find_gradient(aGradient.id.cookie());
        set_gradient(aContext, g);
    }

    standard_texture_shader::standard_texture_shader(std::string const& aName) :
        standard_fragment_shader<i_texture_shader>{ aName },
        iDummyTexture{ size{ 1.0, 1.0 }, 1.0, texture_sampling::Normal },
        iDummyTextureMS{ size{ 1.0, 1.0 }, 1.0, texture_sampling::Multisample }
    {
        disable();
        set_uniform("tex"_s, sampler2D{ static_cast<std::uint32_t>(reserved_texture_unit::Tex) });
        set_uniform("texMS"_s, sampler2DMS{ static_cast<std::uint32_t>(reserved_texture_unit::TexMS) });
        uTextureEffect = shader_effect::None;
        uTexturePassThrough = false;
        uEffectGain = gain{}.as<float>();
        uTextureWrap = vec4i32{ 0, 0, 0, 0 };
        uTextureWrapTransform = vec4f{ 1.0f, 1.0f, 0.0f, 0.0f };
    }

    bool standard_texture_shader::supports(vertex_buffer_type aBufferType) const
    {
        return enabled() && (aBufferType & vertex_buffer_type::UV) != vertex_buffer_type::Invalid;
    }

    void standard_texture_shader::generate_code(i_shader_program const& aProgram, shader_language aLanguage, i_string& aOutput) const
    {
        standard_fragment_shader<i_texture_shader>::generate_code(aProgram, aLanguage, aOutput);
        if (aLanguage == shader_language::Glsl)
            aOutput += string{ glsl::StandardTextureFragmentShader };
        else
            throw unsupported_shader_language();
    }

    void standard_texture_shader::clear_texture()
    {
        enable();
        iDummyTexture.bind(static_cast<std::uint32_t>(reserved_texture_unit::Tex));
        iDummyTextureMS.bind(static_cast<std::uint32_t>(reserved_texture_unit::TexMS));
        uTextureEnabled = false;
        uTextureEffect = shader_effect::None;
        uTextureDataFormat = texture_data_format::RGBA;
        uTextureMultisample = texture_sampling::Normal;
        uTextureExtents = vec2f{};
        uEffectGain = gain{}.as<float>();
        uTextureWrap = vec4i32{ 0, 0, 0, 0 };
    }

    void standard_texture_shader::set_texture(i_texture const& aTexture)
    {
        enable();
        uTextureEnabled = true;
        uTextureDataFormat = aTexture.data_format();
        uTextureMultisample = aTexture.sampling();
        uTextureExtents = aTexture.storage_extents().to_vec2().as<float>();
        uTextureWrap = vec4i32{ 0, 0, 0, 0 };
    }

    void standard_texture_shader::set_effect(shader_effect aEffect)
    {
        uTextureEffect = aEffect;
    }

    void standard_texture_shader::set_effect_gain(vec4 const& aGain)
    {
        uEffectGain = aGain.as<float>();
    }

    void standard_texture_shader::set_pass_through(bool aPassThrough)
    {
        uTexturePassThrough = aPassThrough;
    }

    void standard_texture_shader::set_wrap(vec4 const& aTransform, texture_wrap aWrapS, texture_wrap aWrapT)
    {
        // x: wrap s, y: wrap t, z: 1 if wrapped
        uTextureWrap = vec4i32{ static_cast<std::int32_t>(aWrapS), static_cast<std::int32_t>(aWrapT), 1, 0 };
        uTextureWrapTransform = aTransform.as<float>();
    }

    namespace pbr_environment
    {
        // the environment texture (see standard-pbr.frag): equirectangular bands one above the other, each Width x BandHeight;
        // bands 0 to 5 are the environment prefiltered (GGX) for roughness 0, 0.2, ... 1.0; band 6 the diffuse irradiance
        // (divided by pi); band 7 the split sum environment BRDF (scale, bias) for n.v (u) and roughness (v)
        constexpr std::uint32_t Width = 256u;
        constexpr std::uint32_t BandHeight = 128u;
        constexpr std::uint32_t SpecularBands = 6u;
        constexpr std::uint32_t IrradianceBand = 6u;
        constexpr std::uint32_t BrdfBand = 7u;
        constexpr std::uint32_t Bands = 8u;
        // importance samples per texel (n.b. namespace scope: MSVC warns (C4189) of constexpr locals used only in lambdas)
        constexpr std::uint32_t SpecularSamples = 64u;
        constexpr std::uint32_t BrdfSamples = 128u;
        constexpr float Pi = 3.14159265358979f;

        struct rgb { float r = 0.0f; float g = 0.0f; float b = 0.0f; };
        struct dir { float x; float y; float z; };

        // a linear RGB equirectangular image, row 0 at the top (+y); u = atan2(z, x) / 2pi + 0.5
        struct panorama
        {
            std::uint32_t width = 0u;
            std::uint32_t height = 0u;
            std::vector<rgb> pixels;

            rgb const& at(std::int32_t x, std::int32_t y) const
            {
                x = ((x % static_cast<std::int32_t>(width)) + static_cast<std::int32_t>(width)) % static_cast<std::int32_t>(width);
                y = std::clamp(y, 0, static_cast<std::int32_t>(height) - 1);
                return pixels[static_cast<std::size_t>(y) * width + static_cast<std::size_t>(x)];
            }
            rgb sample(dir const& d) const
            {
                float const u = std::atan2(d.z, d.x) / (2.0f * Pi) + 0.5f;
                float const v = 0.5f - std::asin(std::clamp(d.y, -1.0f, 1.0f)) / Pi;
                float const fx = u * static_cast<float>(width) - 0.5f;
                float const fy = v * static_cast<float>(height) - 0.5f;
                auto const x0 = static_cast<std::int32_t>(std::floor(fx));
                auto const y0 = static_cast<std::int32_t>(std::floor(fy));
                float const tx = fx - static_cast<float>(x0);
                float const ty = fy - static_cast<float>(y0);
                auto const& p00 = at(x0, y0);
                auto const& p10 = at(x0 + 1, y0);
                auto const& p01 = at(x0, y0 + 1);
                auto const& p11 = at(x0 + 1, y0 + 1);
                auto const lerp = [](float c00, float c10, float c01, float c11, float s, float t)
                    { return (c00 * (1.0f - s) + c10 * s) * (1.0f - t) + (c01 * (1.0f - s) + c11 * s) * t; };
                return rgb{ lerp(p00.r, p10.r, p01.r, p11.r, tx, ty), lerp(p00.g, p10.g, p01.g, p11.g, tx, ty), lerp(p00.b, p10.b, p01.b, p11.b, tx, ty) };
            }
            panorama half() const
            {
                panorama result;
                result.width = std::max(width / 2u, 1u);
                result.height = std::max(height / 2u, 1u);
                result.pixels.resize(static_cast<std::size_t>(result.width) * result.height);
                for (std::uint32_t y = 0u; y < result.height; ++y)
                    for (std::uint32_t x = 0u; x < result.width; ++x)
                    {
                        rgb sum;
                        for (std::uint32_t sy = 0u; sy < 2u; ++sy)
                            for (std::uint32_t sx = 0u; sx < 2u; ++sx)
                            {
                                auto const& p = at(static_cast<std::int32_t>(x * 2u + sx), static_cast<std::int32_t>(y * 2u + sy));
                                sum.r += p.r; sum.g += p.g; sum.b += p.b;
                            }
                        result.pixels[static_cast<std::size_t>(y) * result.width + x] = rgb{ sum.r / 4.0f, sum.g / 4.0f, sum.b / 4.0f };
                    }
                return result;
            }
        };

        inline dir direction(float u, float v)
        {
            float const phi = (u - 0.5f) * 2.0f * Pi;
            float const elevation = (0.5f - v) * Pi;
            return dir{ std::cos(phi) * std::cos(elevation), std::sin(elevation), std::sin(phi) * std::cos(elevation) };
        }

        inline float dot(dir const& a, dir const& b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
        inline dir cross(dir const& a, dir const& b) { return dir{ a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x }; }
        inline dir normalized(dir const& a) { float const m = std::sqrt(dot(a, a)); return m > 0.0f ? dir{ a.x / m, a.y / m, a.z / m } : dir{ 0.0f, 1.0f, 0.0f }; }

        inline std::pair<float, float> hammersley(std::uint32_t i, std::uint32_t n)
        {
            std::uint32_t bits = i;
            bits = (bits << 16u) | (bits >> 16u);
            bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
            bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
            bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
            bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
            return { static_cast<float>(i) / static_cast<float>(n), static_cast<float>(bits) * 2.3283064365386963e-10f };
        }

        // a GGX distributed half vector about n (alpha = roughness squared)
        inline dir importance_ggx(std::pair<float, float> const& xi, dir const& n, float alpha)
        {
            float const phi = 2.0f * Pi * xi.first;
            float const cosTheta = std::sqrt((1.0f - xi.second) / (1.0f + (alpha * alpha - 1.0f) * xi.second));
            float const sinTheta = std::sqrt(std::max(1.0f - cosTheta * cosTheta, 0.0f));
            dir const up = std::abs(n.y) < 0.999f ? dir{ 0.0f, 1.0f, 0.0f } : dir{ 1.0f, 0.0f, 0.0f };
            dir const tx = normalized(cross(up, n));
            dir const ty = cross(n, tx);
            float const hx = sinTheta * std::cos(phi);
            float const hy = sinTheta * std::sin(phi);
            return normalized(dir{ tx.x * hx + ty.x * hy + n.x * cosTheta, tx.y * hx + ty.y * hy + n.y * cosTheta, tx.z * hx + ty.z * hy + n.z * cosTheta });
        }

        template <typename Function>
        inline void parallel_rows(std::uint32_t aRows, Function aFunction)
        {
            std::uint32_t const threadCount = std::clamp(std::thread::hardware_concurrency(), 1u, 16u);
            std::atomic<std::uint32_t> next = 0u;
            auto worker = [&]() { for (auto row = next++; row < aRows; row = next++) aFunction(row); };
            std::vector<std::thread> threads;
            for (std::uint32_t t = 1u; t < threadCount; ++t)
                threads.emplace_back(worker);
            worker();
            for (auto& t : threads)
                t.join();
        }

        // the default environment: a sky (zenith to horizon) above a ground (horizon to nadir); n.b. no sun: that is the
        // directional light
        inline panorama default_panorama()
        {
            panorama result;
            result.width = 512u;
            result.height = 256u;
            result.pixels.resize(static_cast<std::size_t>(result.width) * result.height);
            rgb const zenith{ 0.16f, 0.24f, 0.42f };
            rgb const skyHorizon{ 0.62f, 0.64f, 0.68f };
            rgb const groundHorizon{ 0.24f, 0.22f, 0.20f };
            rgb const nadir{ 0.07f, 0.065f, 0.06f };
            auto const mix = [](rgb const& a, rgb const& b, float t) { return rgb{ a.r + (b.r - a.r) * t, a.g + (b.g - a.g) * t, a.b + (b.b - a.b) * t }; };
            for (std::uint32_t y = 0u; y < result.height; ++y)
            {
                float const elevation = std::sin((0.5f - (static_cast<float>(y) + 0.5f) / static_cast<float>(result.height)) * Pi);
                rgb const colour = elevation >= 0.0f ?
                    mix(skyHorizon, zenith, std::pow(elevation, 0.45f)) :
                    mix(groundHorizon, nadir, std::pow(-elevation, 0.35f));
                for (std::uint32_t x = 0u; x < result.width; ++x)
                    result.pixels[static_cast<std::size_t>(y) * result.width + x] = colour;
            }
            return result;
        }

        // the split sum environment BRDF: scale (r) and bias (g) applied to f0, for n.v (x) and roughness (y)
        inline std::vector<rgb> const& brdf_lut()
        {
            static std::vector<rgb> const sLut = []()
            {
                std::vector<rgb> result(static_cast<std::size_t>(Width) * BandHeight);
                parallel_rows(BandHeight, [&](std::uint32_t y)
                    {
                        float const roughness = (static_cast<float>(y) + 0.5f) / static_cast<float>(BandHeight);
                        float const alpha = roughness * roughness;
                        float const k = alpha / 2.0f;
                        dir const n{ 0.0f, 0.0f, 1.0f };
                        for (std::uint32_t x = 0u; x < Width; ++x)
                        {
                            float const nDotV = (static_cast<float>(x) + 0.5f) / static_cast<float>(Width);
                            dir const v{ std::sqrt(1.0f - nDotV * nDotV), 0.0f, nDotV };
                            float a = 0.0f;
                            float b = 0.0f;
                            for (std::uint32_t i = 0u; i < BrdfSamples; ++i)
                            {
                                dir const h = importance_ggx(hammersley(i, BrdfSamples), n, alpha);
                                float const vDotH = dot(v, h);
                                dir const l{ 2.0f * vDotH * h.x - v.x, 2.0f * vDotH * h.y - v.y, 2.0f * vDotH * h.z - v.z };
                                float const nDotL = l.z;
                                float const nDotH = std::max(h.z, 0.0f);
                                if (nDotL <= 0.0f || vDotH <= 0.0f)
                                    continue;
                                float const g = (nDotL / (nDotL * (1.0f - k) + k)) * (nDotV / (nDotV * (1.0f - k) + k));
                                float const gVis = g * vDotH / (nDotH * nDotV);
                                float const fc = std::pow(1.0f - vDotH, 5.0f);
                                a += (1.0f - fc) * gVis;
                                b += fc * gVis;
                            }
                            result[static_cast<std::size_t>(y) * Width + x] = rgb{ a / static_cast<float>(BrdfSamples), b / static_cast<float>(BrdfSamples), 0.0f };
                        }
                    });
                return result;
            }();
            return sLut;
        }

        // builds the environment texture (RGBA, Width x BandHeight * Bands) from a panorama
        inline std::vector<float> build(panorama const& aSource)
        {
            std::vector<panorama> levels;
            levels.push_back(aSource);
            while (levels.back().width > 8u && levels.back().height > 4u)
                levels.push_back(levels.back().half());
            auto const sample_level = [&](float aLevel, dir const& d) -> rgb
                {
                    aLevel = std::clamp(aLevel, 0.0f, static_cast<float>(levels.size() - 1u));
                    auto const l0 = static_cast<std::size_t>(std::floor(aLevel));
                    auto const l1 = std::min(l0 + 1u, levels.size() - 1u);
                    float const t = aLevel - static_cast<float>(l0);
                    auto const a = levels[l0].sample(d);
                    if (t <= 0.0f || l1 == l0)
                        return a;
                    auto const b = levels[l1].sample(d);
                    return rgb{ a.r + (b.r - a.r) * t, a.g + (b.g - a.g) * t, a.b + (b.b - a.b) * t };
                };
            // the source level whose texels match the bands' (for roughness 0)
            float const baseLevel = std::max(std::log2(static_cast<float>(aSource.width) / static_cast<float>(Width)), 0.0f);
            float const sourceTexelSolidAngle = 4.0f * Pi / (static_cast<float>(aSource.width) * static_cast<float>(aSource.height));

            std::vector<float> result(static_cast<std::size_t>(Width) * BandHeight * Bands * 4u, 0.0f);
            auto const put = [&](std::uint32_t aBand, std::uint32_t x, std::uint32_t y, rgb const& c, float a = 1.0f)
                {
                    auto* p = &result[((static_cast<std::size_t>(aBand) * BandHeight + y) * Width + x) * 4u];
                    p[0] = c.r; p[1] = c.g; p[2] = c.b; p[3] = a;
                };

            // specular: GGX prefiltered (n = v = r), filtered importance sampling
            parallel_rows(BandHeight * SpecularBands, [&](std::uint32_t aRow)
                {
                    std::uint32_t const band = aRow / BandHeight;
                    std::uint32_t const y = aRow % BandHeight;
                    float const roughness = static_cast<float>(band) / static_cast<float>(SpecularBands - 1u);
                    float const alpha = roughness * roughness;
                    for (std::uint32_t x = 0u; x < Width; ++x)
                    {
                        dir const n = direction((static_cast<float>(x) + 0.5f) / static_cast<float>(Width), (static_cast<float>(y) + 0.5f) / static_cast<float>(BandHeight));
                        if (band == 0u)
                        {
                            put(band, x, y, sample_level(baseLevel, n));
                            continue;
                        }
                        rgb sum;
                        float weight = 0.0f;
                        for (std::uint32_t i = 0u; i < SpecularSamples; ++i)
                        {
                            dir const h = importance_ggx(hammersley(i, SpecularSamples), n, alpha);
                            float const nDotH = dot(n, h);
                            dir const l{ 2.0f * nDotH * h.x - n.x, 2.0f * nDotH * h.y - n.y, 2.0f * nDotH * h.z - n.z };
                            float const nDotL = dot(n, l);
                            if (nDotL <= 0.0f)
                                continue;
                            float const alpha2 = alpha * alpha;
                            float const d = nDotH * nDotH * (alpha2 - 1.0f) + 1.0f;
                            float const distribution = alpha2 / (Pi * d * d);
                            float const pdf = distribution / 4.0f; // n = v: D * n.h / (4 v.h)
                            float const sampleSolidAngle = 1.0f / (static_cast<float>(SpecularSamples) * pdf + 1e-6f);
                            float const level = 0.5f * std::log2(sampleSolidAngle / sourceTexelSolidAngle) + 1.0f;
                            auto const c = sample_level(level, normalized(l));
                            sum.r += c.r * nDotL; sum.g += c.g * nDotL; sum.b += c.b * nDotL;
                            weight += nDotL;
                        }
                        put(band, x, y, weight > 0.0f ? rgb{ sum.r / weight, sum.g / weight, sum.b / weight } : rgb{});
                    }
                });

            // diffuse: irradiance (divided by pi) from the third order spherical harmonics of the environment
            {
                auto const& level = levels[std::min<std::size_t>(levels.size() - 1u,
                    static_cast<std::size_t>(std::max(std::log2(static_cast<float>(aSource.width) / 64.0f), 0.0f)))];
                double sh[9][3] = {};
                for (std::uint32_t y = 0u; y < level.height; ++y)
                {
                    float const v = (static_cast<float>(y) + 0.5f) / static_cast<float>(level.height);
                    float const solidAngle = (2.0f * Pi / static_cast<float>(level.width)) * (Pi / static_cast<float>(level.height)) * std::cos((0.5f - v) * Pi);
                    for (std::uint32_t x = 0u; x < level.width; ++x)
                    {
                        dir const d = direction((static_cast<float>(x) + 0.5f) / static_cast<float>(level.width), v);
                        float const basis[9] = { 0.282095f, 0.488603f * d.y, 0.488603f * d.z, 0.488603f * d.x,
                            1.092548f * d.x * d.y, 1.092548f * d.y * d.z, 0.315392f * (3.0f * d.z * d.z - 1.0f), 1.092548f * d.x * d.z, 0.546274f * (d.x * d.x - d.y * d.y) };
                        auto const& c = level.pixels[static_cast<std::size_t>(y) * level.width + x];
                        for (std::uint32_t i = 0u; i < 9u; ++i)
                        {
                            sh[i][0] += c.r * basis[i] * solidAngle;
                            sh[i][1] += c.g * basis[i] * solidAngle;
                            sh[i][2] += c.b * basis[i] * solidAngle;
                        }
                    }
                }
                float const band[9] = { Pi, 2.0f * Pi / 3.0f, 2.0f * Pi / 3.0f, 2.0f * Pi / 3.0f, Pi / 4.0f, Pi / 4.0f, Pi / 4.0f, Pi / 4.0f, Pi / 4.0f };
                for (std::uint32_t y = 0u; y < BandHeight; ++y)
                    for (std::uint32_t x = 0u; x < Width; ++x)
                    {
                        dir const d = direction((static_cast<float>(x) + 0.5f) / static_cast<float>(Width), (static_cast<float>(y) + 0.5f) / static_cast<float>(BandHeight));
                        float const basis[9] = { 0.282095f, 0.488603f * d.y, 0.488603f * d.z, 0.488603f * d.x,
                            1.092548f * d.x * d.y, 1.092548f * d.y * d.z, 0.315392f * (3.0f * d.z * d.z - 1.0f), 1.092548f * d.x * d.z, 0.546274f * (d.x * d.x - d.y * d.y) };
                        rgb e;
                        for (std::uint32_t i = 0u; i < 9u; ++i)
                        {
                            e.r += static_cast<float>(sh[i][0]) * band[i] * basis[i];
                            e.g += static_cast<float>(sh[i][1]) * band[i] * basis[i];
                            e.b += static_cast<float>(sh[i][2]) * band[i] * basis[i];
                        }
                        put(IrradianceBand, x, y, rgb{ std::max(e.r, 0.0f) / Pi, std::max(e.g, 0.0f) / Pi, std::max(e.b, 0.0f) / Pi });
                    }
            }

            // the split sum environment BRDF (independent of the environment)
            auto const& brdf = brdf_lut();
            for (std::uint32_t y = 0u; y < BandHeight; ++y)
                for (std::uint32_t x = 0u; x < Width; ++x)
                    put(BrdfBand, x, y, brdf[static_cast<std::size_t>(y) * Width + x]);
            return result;
        }

        inline void upload(std::optional<texture>& aTexture, panorama const& aSource)
        {
            auto const data = build(aSource);
            size const extents{ static_cast<scalar>(Width), static_cast<scalar>(BandHeight * Bands) };
            if (!aTexture)
                aTexture.emplace(extents, 1.0, texture_sampling::Data, texture_data_format::RGBA, texture_data_type::Float);
            aTexture->set_pixels(rect{ point{}, extents }, data.data());
        }

        // the background texture (see i_pbr_shader::set_background_texture and native_rendering_context::draw_scene_background):
        // RGBE texels; the panorama (at most MaxBackgroundWidth wide, at least 16 x 8 and an even height) at the top, then below
        // it, side by side, versions of it halved while wider than 8 and taller than 4 (so the texture is half as tall again)
        constexpr std::uint32_t MaxBackgroundWidth = 4096u;

        inline void upload_background(std::optional<texture>& aTexture, panorama aSource)
        {
            while (aSource.width > MaxBackgroundWidth)
                aSource = aSource.half();
            while (aSource.width < 16u || aSource.height < 8u || aSource.height % 2u != 0u)
            {
                // too small (doubled) or an odd height (last row repeated)
                bool const twice = aSource.width < 16u || aSource.height < 8u;
                panorama larger;
                larger.width = twice ? aSource.width * 2u : aSource.width;
                larger.height = twice ? aSource.height * 2u : aSource.height + 1u;
                larger.pixels.resize(static_cast<std::size_t>(larger.width) * larger.height);
                for (std::uint32_t y = 0u; y < larger.height; ++y)
                    for (std::uint32_t x = 0u; x < larger.width; ++x)
                        larger.pixels[static_cast<std::size_t>(y) * larger.width + x] = twice ?
                            aSource.at(static_cast<std::int32_t>(x / 2u), static_cast<std::int32_t>(y / 2u)) :
                            aSource.at(static_cast<std::int32_t>(x), static_cast<std::int32_t>(y));
                aSource = std::move(larger);
            }
            std::vector<panorama> levels;
            for (panorama const* level = &aSource; level->width > 8u && level->height > 4u; level = &levels.back())
                levels.push_back(level->half());
            std::uint32_t const width = aSource.width;
            std::uint32_t const height = aSource.height + aSource.height / 2u;
            std::vector<std::uint8_t> data(static_cast<std::size_t>(width) * height * 4u, 0u);
            auto const put = [&](panorama const& aLevel, std::uint32_t aX, std::uint32_t aY)
                {
                    for (std::uint32_t y = 0u; y < aLevel.height; ++y)
                        for (std::uint32_t x = 0u; x < aLevel.width; ++x)
                        {
                            auto const& c = aLevel.pixels[static_cast<std::size_t>(y) * aLevel.width + x];
                            auto* p = &data[((static_cast<std::size_t>(aY) + y) * width + aX + x) * 4u];
                            float const m = std::max({ c.r, c.g, c.b });
                            if (!(m > 1e-32f))
                                continue;
                            int exponent = 0;
                            float const scale = std::frexp(m, &exponent) * 256.0f / m;
                            p[0] = static_cast<std::uint8_t>(std::min(c.r * scale, 255.0f));
                            p[1] = static_cast<std::uint8_t>(std::min(c.g * scale, 255.0f));
                            p[2] = static_cast<std::uint8_t>(std::min(c.b * scale, 255.0f));
                            p[3] = static_cast<std::uint8_t>(std::clamp(exponent + 128, 0, 255));
                        }
                };
            put(aSource, 0u, 0u);
            std::uint32_t x = 0u;
            for (auto const& level : levels)
            {
                put(level, x, aSource.height);
                x += level.width;
            }
            size const extents{ static_cast<scalar>(width), static_cast<scalar>(height) };
            aTexture = std::nullopt;
            aTexture.emplace(extents, 1.0, texture_sampling::Data, texture_data_format::RGBA, texture_data_type::UnsignedByte);
            aTexture->set_pixels(rect{ point{}, extents }, data.data());
        }

        inline panorama to_panorama(size_u32 const& aExtents, float const* aRgbaPixels)
        {
            auto const finite = [](float c) { return std::isfinite(c) ? std::max(c, 0.0f) : 0.0f; };
            panorama result;
            result.width = aExtents.cx;
            result.height = aExtents.cy;
            result.pixels.resize(static_cast<std::size_t>(result.width) * result.height);
            for (std::size_t i = 0u; i < result.pixels.size(); ++i)
                result.pixels[i] = rgb{ finite(aRgbaPixels[i * 4u]), finite(aRgbaPixels[i * 4u + 1u]), finite(aRgbaPixels[i * 4u + 2u]) };
            return result;
        }
    }

    standard_pbr_shader::standard_pbr_shader(std::string const& aName) :
        standard_fragment_shader<i_pbr_shader>{ aName }
    {
        disable();
        add_in_variable<vec3f>("WorldNormal"_s, 10u);
        set_uniform("uPbrTexture0"_s, sampler2D{ static_cast<std::uint32_t>(reserved_texture_unit::Pbr0) });
        set_uniform("uPbrTexture1"_s, sampler2D{ static_cast<std::uint32_t>(reserved_texture_unit::Pbr1) });
        set_uniform("uPbrTexture2"_s, sampler2D{ static_cast<std::uint32_t>(reserved_texture_unit::Pbr2) });
        set_uniform("uPbrTexture3"_s, sampler2D{ static_cast<std::uint32_t>(reserved_texture_unit::Pbr3) });
        set_uniform("uPbrBaseTexture"_s, sampler2D{ static_cast<std::uint32_t>(reserved_texture_unit::Tex) });
        // n.b. a texture_sampling::Data texture: a rectangle texture
        set_uniform("uPbrEnvironment"_s, sampler2DRect{ static_cast<std::uint32_t>(reserved_texture_unit::PbrEnvironment) });
        set_uniform("uPbrShadowAtlas"_s, sampler2D{ static_cast<std::uint32_t>(reserved_texture_unit::PbrShadow) });
        uPbrEnabled = false;
        uPbrLightDirection = vec3f{ 0.0f, 1.0f, 0.0f };
        uPbrViewPosition = vec3f{};
        uPbrLightBase = 0u;
        uPbrLightCount = 0u;
        uPbrShadowMatrixBase = 0u;
        uPbrDirectionalShadow = -1;
        uPbrDirectionalShadowTexel = 0.0f;
        uPbrFactors = vec4f{ 1.0f, 1.0f, 1.0f, 1.0f };
        uPbrEmissive = vec3f{};
        uPbrAlphaCutoff = -1.0f;
        uPbrDoubleSided = false;
        uPbrBaseColorTextured = false;
        uPbrBaseTextureTransform = vec4f{ 1.0f, 1.0f, 0.0f, 0.0f };
        uPbrTextureWrap = vec4i32{ 1, 1, 0, 0 };
        uPbrEnvironmentIntensity = 1.0f;
        uPbrTextureSources = vec4i32{ pbr_shader_material::NoTexture, pbr_shader_material::NoTexture, pbr_shader_material::NoTexture, pbr_shader_material::NoTexture };
        uPbrTextureTransform0 = vec4f{ 1.0f, 1.0f, 0.0f, 0.0f };
        uPbrTextureTransform1 = vec4f{ 1.0f, 1.0f, 0.0f, 0.0f };
        uPbrTextureTransform2 = vec4f{ 1.0f, 1.0f, 0.0f, 0.0f };
        uPbrTextureTransform3 = vec4f{ 1.0f, 1.0f, 0.0f, 0.0f };
    }

    void standard_pbr_shader::generate_code(i_shader_program const& aProgram, shader_language aLanguage, i_string& aOutput) const
    {
        standard_fragment_shader<i_pbr_shader>::generate_code(aProgram, aLanguage, aOutput);
        if (aLanguage == shader_language::Glsl)
            aOutput += string{ glsl::StandardPbrFragmentShader };
        else
            throw unsupported_shader_language();
    }

    std::optional<vec3> const& standard_pbr_shader::pbr_light() const
    {
        return iLight;
    }

    vec3 const& standard_pbr_shader::pbr_camera() const
    {
        return iCamera;
    }

    void standard_pbr_shader::set_pbr_light(std::optional<vec3> const& aDirection, vec3 const& aCameraPosition)
    {
        iLight = (aDirection && aDirection->magnitude() > 0.0) ? std::optional<vec3>{ aDirection->normalized() } : std::nullopt;
        iCamera = aCameraPosition;
    }

    void standard_pbr_shader::set_pbr_light_buffer(std::uint32_t aBase, std::uint32_t aCount)
    {
        uPbrLightBase = aBase;
        uPbrLightCount = aCount;
    }

    std::optional<std::pair<vec3, scalar>> const& standard_pbr_shader::pbr_shadows() const
    {
        return iShadows;
    }

    void standard_pbr_shader::set_pbr_shadows(std::optional<std::pair<vec3, scalar>> const& aSceneBounds)
    {
        iShadows = aSceneBounds;
    }

    void standard_pbr_shader::set_pbr_shadow_buffer(std::uint32_t aMatrixBase, std::int32_t aDirectionalView, scalar aDirectionalTexelSize)
    {
        uPbrShadowMatrixBase = aMatrixBase;
        uPbrDirectionalShadow = aDirectionalView;
        uPbrDirectionalShadowTexel = static_cast<float>(aDirectionalTexelSize);
    }

    void standard_pbr_shader::clear_pbr()
    {
        // n.b. stays disabled (not compiled) until first used
        if (enabled())
            uPbrEnabled = false;
    }

    void standard_pbr_shader::set_pbr(pbr_shader_material const& aMaterial)
    {
        enable();
        uPbrEnabled = true;
        uPbrLightDirection = (iLight ? *iLight : vec3{ 0.0, 1.0, 0.0 }).as<float>();
        uPbrViewPosition = aMaterial.viewPosition.as<float>();
        uPbrFactors = vec4{ aMaterial.metallic, aMaterial.roughness, aMaterial.normalScale, aMaterial.occlusionStrength }.as<float>();
        uPbrEmissive = aMaterial.emissive.as<float>();
        uPbrAlphaCutoff = aMaterial.alphaCutoff ? static_cast<float>(*aMaterial.alphaCutoff) : -1.0f;
        uPbrDoubleSided = aMaterial.doubleSided;
        uPbrBaseColorTextured = aMaterial.baseColorTextured;
        uPbrBaseTextureTransform = aMaterial.baseColorTransform.as<float>();
        uPbrTextureWrap = vec4i32{ static_cast<std::int32_t>(aMaterial.wrapS), static_cast<std::int32_t>(aMaterial.wrapT), 0, 0 };
        uPbrEnvironmentIntensity = static_cast<float>(iEnvironmentIntensity);
        uPbrTextureSources = aMaterial.textureSources;
        uPbrTextureTransform0 = aMaterial.textureTransforms[0].as<float>();
        uPbrTextureTransform1 = aMaterial.textureTransforms[1].as<float>();
        uPbrTextureTransform2 = aMaterial.textureTransforms[2].as<float>();
        uPbrTextureTransform3 = aMaterial.textureTransforms[3].as<float>();
    }

    void standard_pbr_shader::set_environment(size_u32 const& aExtents, float const* aRgbaPixels, scalar aIntensity)
    {
        if (aExtents.cx == 0u || aExtents.cy == 0u || aRgbaPixels == nullptr)
        {
            clear_environment();
            return;
        }
        pbr_environment::upload(iEnvironment, pbr_environment::to_panorama(aExtents, aRgbaPixels));
        iEnvironmentIntensity = aIntensity;
    }

    void standard_pbr_shader::clear_environment()
    {
        pbr_environment::upload(iEnvironment, pbr_environment::default_panorama());
        iEnvironmentIntensity = 1.0;
    }

    i_texture const& standard_pbr_shader::environment() const
    {
        // the default is made when first needed
        if (!iEnvironment)
            pbr_environment::upload(iEnvironment, pbr_environment::default_panorama());
        return *iEnvironment;
    }

    scalar standard_pbr_shader::environment_intensity() const
    {
        return iEnvironmentIntensity;
    }

    std::optional<pbr_background> const& standard_pbr_shader::background() const
    {
        return iBackground;
    }

    void standard_pbr_shader::set_background(std::optional<pbr_background> const& aBackground)
    {
        iBackground = aBackground;
    }

    void standard_pbr_shader::set_background_texture(size_u32 const& aExtents, float const* aRgbaPixels)
    {
        if (aExtents.cx == 0u || aExtents.cy == 0u || aRgbaPixels == nullptr)
        {
            clear_background_texture();
            return;
        }
        pbr_environment::upload_background(iBackgroundTexture, pbr_environment::to_panorama(aExtents, aRgbaPixels));
    }

    void standard_pbr_shader::clear_background_texture()
    {
        iBackgroundTexture = std::nullopt;
    }

    i_texture const* standard_pbr_shader::background_texture() const
    {
        return iBackgroundTexture ? &*iBackgroundTexture : nullptr;
    }

    standard_filter_shader::standard_filter_shader(std::string const& aName) :
        standard_fragment_shader<i_filter_shader>{ aName }
    {
        disable();
    }

    bool standard_filter_shader::supports(vertex_buffer_type aBufferType) const
    {
        return enabled() && (aBufferType & vertex_buffer_type::UV) != vertex_buffer_type::Invalid;
    }

    void standard_filter_shader::generate_code(i_shader_program const& aProgram, shader_language aLanguage, i_string& aOutput) const
    {
        standard_fragment_shader<i_filter_shader>::generate_code(aProgram, aLanguage, aOutput);
        if (aLanguage == shader_language::Glsl)
            aOutput += string{ glsl::StandardFilterFragmentShader };
        else
            throw unsupported_shader_language();
    }

    void standard_filter_shader::clear_filter()
    {
        uFilterEnabled = false;
    }

    void standard_filter_shader::set_filter(shader_filter aFilter, std::int32_t aPass, scalar aArgument1, scalar aArgument2, scalar aArgument3, scalar aArgument4)
    {
        enable();
        uFilterEnabled = true;
        uFilterType = aFilter;
        uFilterPass = aPass;
        if (aFilter == shader_filter::GaussianBlur || aFilter == shader_filter::GaussianBlur2D)
            aArgument1 = (static_cast<std::uint32_t>(aArgument1) | 1u);
        auto const arguments = vec4{ aArgument1, aArgument2, aArgument3, aArgument4 };
        uFilterArguments = arguments.as<float>();
        if (aFilter == shader_filter::DilateOctagon || aFilter == shader_filter::DilateDisk)
        {
            // Morphological: no kernel texture, extent is carried in the arguments.
            uFilterKernelSize = 0;
            return;
        }
        auto kernel = iFilterKernels.find(std::make_pair(aFilter, arguments));
        if (kernel == iFilterKernels.end())
        {
            switch (aFilter)
            {
            case shader_filter::GaussianBlur:
                {
                    kernel = iFilterKernels.emplace(std::make_pair(aFilter, arguments), std::optional<shader_array<float>>{}).first;
                    auto const kernelValues = dynamic_gaussian_filter<float>(static_cast<std::uint32_t>(aArgument1), static_cast<float>(aArgument2));
                    auto const kernelSize = size_u32{ static_cast<std::uint32_t>(aArgument1), 1u };
                    kernel->second.emplace(kernelSize);
                    kernel->second->data().set_pixels(rect{ point{}, kernelSize }, &kernelValues[0]);
                }
                break;
            case shader_filter::GaussianBlur2D:
                {
                    kernel = iFilterKernels.emplace(std::make_pair(aFilter, arguments), std::optional<shader_array<float>>{}).first;
                    auto const kernelValues = dynamic_gaussian_filter_2d<float>(static_cast<std::uint32_t>(aArgument1), static_cast<float>(aArgument2));
                    auto const kernelSize = size_u32{ static_cast<std::uint32_t>(aArgument1), static_cast<std::uint32_t>(aArgument1) };
                    kernel->second.emplace(kernelSize);
                    kernel->second->data().set_pixels(rect{ point{}, kernelSize }, &kernelValues[0][0]);
                }
                break;
            }
        }
        auto& kernelData = kernel->second->data();
        uFilterKernelSize = static_cast<i32>(kernelData.extents().cx);
        if (iActiveKernel && iActiveKernel != &kernelData)
            iActiveKernel->unbind();
        iActiveKernel = &kernelData;
        kernelData.bind(static_cast<std::uint32_t>(reserved_texture_unit::FilterKernel));
        uFilterKernel = sampler2DRect{ static_cast<std::uint32_t>(reserved_texture_unit::FilterKernel) };
    }

    standard_glyph_shader::standard_glyph_shader(std::string const& aName) :
        standard_fragment_shader<i_glyph_shader>{ aName }
    {
        disable();
    }

    void standard_glyph_shader::generate_code(i_shader_program const& aProgram, shader_language aLanguage, i_string& aOutput) const
    {
        standard_fragment_shader<i_glyph_shader>::generate_code(aProgram, aLanguage, aOutput);
        if (aLanguage == shader_language::Glsl)
            aOutput += string{ glsl::StandardGlyphFragmentShader };
        else
            throw unsupported_shader_language();
    }

    void standard_glyph_shader::clear_glyph()
    {
        uGlyphEnabled = false;
    }

    void standard_glyph_shader::set_first_glyph(i_rendering_context const& aContext, glyph_text const& aText, glyph_char const& aGlyphChar)
    {
        enable();
        bool subpixelRender = subpixel(aGlyphChar) && aText.glyph(aGlyphChar).subpixel();
        if (subpixelRender)
            aContext.render_target().target_texture().bind(static_cast<std::uint32_t>(reserved_texture_unit::RenderTarget));
        uGlyphRenderOutput = sampler2DMS{ static_cast<std::uint32_t>(reserved_texture_unit::RenderTarget) };
        uGlyphSubpixel = aText.glyph(aGlyphChar).subpixel();
        uGlyphSubpixelFormat = subpixelRender ? aContext.subpixel_format() : subpixel_format::None;
        uGlyphEnabled = true;
    }

    standard_stipple_shader::standard_stipple_shader(std::string const& aName) :
        standard_fragment_shader<i_stipple_shader>{ aName }, iPosition{ 0.0 }
    {
        disable();
    }

    void standard_stipple_shader::generate_code(i_shader_program const& aProgram, shader_language aLanguage, i_string& aOutput) const
    {
        standard_fragment_shader<i_stipple_shader>::generate_code(aProgram, aLanguage, aOutput);
        if (aLanguage == shader_language::Glsl)
            aOutput += string{ glsl::StandardStippleFragmentShader };
        else
            throw unsupported_shader_language();
    }

    bool standard_stipple_shader::stipple_active() const
    {
        return !uStippleEnabled.uniform().value().empty() && 
            uStippleEnabled.uniform().value().get<bool>();
    }

    void standard_stipple_shader::clear_stipple()
    {
        iPosition = 0.0;
        uStippleEnabled = false;
    }

    void standard_stipple_shader::set_stipple(stipple const& aStipple)
    {
        enable();
        iPosition = aStipple.position;
        thread_local shader_float_array pattern;
        pattern.resize(16);
        std::transform(aStipple.pattern.begin(), aStipple.pattern.end(), pattern.begin(),
            [](scalar value) { return static_cast<float>(value); });
        uStipplePattern = pattern;
        uStipplePatternSize = aStipple.pattern.size();
        uStipplePatternLength = static_cast<float>(std::accumulate(aStipple.pattern.begin(), aStipple.pattern.end(), 0.0));
        uStipplePosition = static_cast<float>(iPosition);
        uStippleVertex = vec3f{};
        uStippleEnabled = true;
    }

    void standard_stipple_shader::start(i_rendering_context const& aContext, vec3 const& aFrom)
    {
        next(aContext, aFrom, 0.0);
    }
    
    void standard_stipple_shader::next(i_rendering_context const& aContext, vec3 const& aFrom, scalar aPositionOffset)
    {
        uStipplePosition = static_cast<float>(iPosition + aPositionOffset);
        uStippleVertex = aFrom.as<float>();
    }

    standard_shape_shader::standard_shape_shader(i_shader_program& aShaderProgram, std::string const& aName) :
        standard_fragment_shader<i_shape_shader>{ aName }
    {
        uShapeEnabled = false;
        uShape = shader_shape::None;
        iShapeVertices = aShaderProgram.create_ssbo<vec4f>("bShapeVertices"_s);
        // Always ensure SSBO is mapped to prevent Intel integrated graphics driver crash for shapes that don't need it...
        iShapeVertices->alloc(1);
        disable();
    }

    void standard_shape_shader::generate_code(i_shader_program const& aProgram, shader_language aLanguage, i_string& aOutput) const
    {
        standard_fragment_shader<i_shape_shader>::generate_code(aProgram, aLanguage, aOutput);
        if (aLanguage == shader_language::Glsl)
            aOutput += string{ glsl::StandardShapeFragmentShader };
        else
            throw unsupported_shader_language();
    }

    bool standard_shape_shader::shape_active() const
    {
        return !uShapeEnabled.uniform().value().empty() &&
            uShapeEnabled.uniform().value().get<bool>();
    }

    void standard_shape_shader::clear_shape()
    {
        uShapeEnabled = false;
    }

    void standard_shape_shader::set_shape(shader_shape aShape)
    {
        enable();
        uShape = aShape;
        uShapeEnabled = true;
    }

    i_ssbo& standard_shape_shader::shape_vertices()
    {
        return *iShapeVertices;
    }
}