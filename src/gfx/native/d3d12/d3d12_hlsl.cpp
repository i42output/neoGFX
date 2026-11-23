// d3d12_hlsl.cpp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2026 Leigh Johnston.  All Rights Reserved.

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

#include <mutex>
#include <regex>
#include <sstream>

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <spirv_cross/spirv_cross_c.h>

#include <neogfx/gfx/i_shader_program.hpp>
#include "d3d12_hlsl.hpp"

namespace neogfx
{
    std::string d3d12_glsl(std::string const& aVulkanGlsl)
    {
        // (see vulkan_glsl: "layout (std430, set = 0, binding = N) buffer")
        static std::regex const sStorageBlock{ R"((layout\s*\(\s*std430\s*,[^)]*\)\s*)buffer\b)" };
        return std::regex_replace(aVulkanGlsl, sStorageBlock, "$1readonly buffer");
    }

    std::string d3d12_glsl_scaled_inputs(std::string const& aSource, std::uint32_t aScaledLocations)
    {
        if (aScaledLocations == 0u)
            return aSource;
        // (see shader<>::generate_code: "layout (location = L) [flat] in T Name;")
        static std::regex const sInput{ R"(^(\s*layout\s*\(\s*location\s*=\s*(\d+)\s*\)\s*(?:flat\s+)?in\s+)(float|vec2|vec3|vec4)(\s+)(\w+)(\s*;.*)$)" };
        std::string result;
        result.reserve(aSource.size() + 256u);
        std::string conversions;
        std::istringstream lines{ aSource };
        std::string line;
        while (std::getline(lines, line))
        {
            std::smatch match;
            if (std::regex_match(line, match, sInput))
            {
                auto const location = static_cast<std::uint32_t>(std::stoul(match[2].str()));
                if (location < 32u && (aScaledLocations & (1u << location)) != 0u)
                {
                    auto const type = match[3].str();
                    auto const name = match[5].str();
                    auto const unsignedType = (type == "float" ? std::string{ "uint" } : "u" + type);
                    line = match[1].str() + unsignedType + match[4].str() + "neogfx_scaled_" + name + match[6].str() + "\n" +
                        type + " " + name + ";";
                    conversions += "    " + name + " = " + type + "(neogfx_scaled_" + name + ");\n";
                }
            }
            result += line;
            result += '\n';
        }
        if (!conversions.empty())
        {
            static std::regex const sMain{ R"(\bvoid\s+main\s*\(\s*\)\s*\{)" };
            std::smatch match;
            if (std::regex_search(result, match, sMain))
                result.insert(static_cast<std::size_t>(match.position(0) + match.length(0)), "\n" + conversions);
        }
        return result;
    }

    namespace
    {
        // the SPIRV-Cross C API functions used (n.b. the DLL is loaded rather than linked, so that a program without it still
        // runs, with Vulkan or OpenGL)
        struct spirv_cross_api
        {
            HMODULE module = nullptr;
            decltype(&spvc_context_create) context_create = nullptr;
            decltype(&spvc_context_destroy) context_destroy = nullptr;
            decltype(&spvc_context_parse_spirv) context_parse_spirv = nullptr;
            decltype(&spvc_context_create_compiler) context_create_compiler = nullptr;
            decltype(&spvc_context_get_last_error_string) context_get_last_error_string = nullptr;
            decltype(&spvc_compiler_create_compiler_options) compiler_create_compiler_options = nullptr;
            decltype(&spvc_compiler_options_set_uint) compiler_options_set_uint = nullptr;
            decltype(&spvc_compiler_options_set_bool) compiler_options_set_bool = nullptr;
            decltype(&spvc_compiler_install_compiler_options) compiler_install_compiler_options = nullptr;
            decltype(&spvc_compiler_compile) compiler_compile = nullptr;
        };

        spirv_cross_api& spirv_cross()
        {
            static spirv_cross_api sApi;
            static std::once_flag sLoaded;
            std::call_once(sLoaded, []()
            {
                auto const module = ::LoadLibraryA("spirv-cross-c-shared.dll");
                if (module == nullptr)
                    return;
                auto const get = [&](auto& aFunction, char const* aName)
                {
                    aFunction = reinterpret_cast<std::remove_reference_t<decltype(aFunction)>>(reinterpret_cast<void*>(::GetProcAddress(module, aName)));
                    return aFunction != nullptr;
                };
                bool const complete =
                    get(sApi.context_create, "spvc_context_create") &&
                    get(sApi.context_destroy, "spvc_context_destroy") &&
                    get(sApi.context_parse_spirv, "spvc_context_parse_spirv") &&
                    get(sApi.context_create_compiler, "spvc_context_create_compiler") &&
                    get(sApi.context_get_last_error_string, "spvc_context_get_last_error_string") &&
                    get(sApi.compiler_create_compiler_options, "spvc_compiler_create_compiler_options") &&
                    get(sApi.compiler_options_set_uint, "spvc_compiler_options_set_uint") &&
                    get(sApi.compiler_options_set_bool, "spvc_compiler_options_set_bool") &&
                    get(sApi.compiler_install_compiler_options, "spvc_compiler_install_compiler_options") &&
                    get(sApi.compiler_compile, "spvc_compiler_compile");
                if (complete)
                    sApi.module = module;
            });
            return sApi;
        }

        // FXC can fail to compile a function that returns a value and discards with "X3507: Not all control paths return a
        // value" (as standard_shader_program's shape_color did); the workaround (Tint's) is to wrap the function's body in
        // "if (true) { ... }" followed by an unused return. n.b. SPIRV-Cross puts a function's opening and closing braces alone
        // at the start of their lines (as it does a struct's and cbuffer's, but those close with "};")
        std::string fxc_discard_workaround(std::string const& aHlsl)
        {
            if (aHlsl.find("discard;") == std::string::npos)
                return aHlsl;
            static std::regex const sHeader{ R"(^(\w+)\s+\w+\s*\(.*\)\s*$)" };
            std::vector<std::string> lines;
            std::istringstream input{ aHlsl };
            for (std::string line; std::getline(input, line);)
                lines.push_back(line);
            std::string result;
            result.reserve(aHlsl.size() + 1024u);
            for (std::size_t i = 0u; i < lines.size(); ++i)
            {
                result += lines[i] + "\n";
                std::smatch header;
                if (lines[i] != "{" || i == 0u || !std::regex_match(lines[i - 1u], header, sHeader) || header[1].str() == "void" ||
                    header[1].str() == "struct" || header[1].str() == "cbuffer")
                    continue;
                auto end = i + 1u;
                bool discards = false;
                for (; end < lines.size() && lines[end] != "}" && lines[end] != "};"; ++end)
                    discards = discards || lines[end].find("discard;") != std::string::npos;
                if (!discards || end == lines.size() || lines[end] != "}")
                    continue;
                result += "    if (true)\n    {\n";
                for (auto j = i + 1u; j < end; ++j)
                    result += (lines[j].empty() ? "" : "    ") + lines[j] + "\n";
                result += "    }\n    return (" + header[1].str() + ")0;\n";
                i = end - 1u;
            }
            return result;
        }
    }

    void load_spirv_cross()
    {
        if (spirv_cross().module == nullptr)
            throw std::runtime_error("spirv-cross-c-shared.dll not found (or not the SPIRV-Cross C API)");
    }

    std::string spirv_to_hlsl(std::vector<std::uint32_t> const& aSpirv)
    {
        load_spirv_cross();
        auto const& api = spirv_cross();
        spvc_context context = nullptr;
        if (api.context_create(&context) != SPVC_SUCCESS)
            throw failed_to_create_shader_program("SPIRV-Cross: spvc_context_create");
        struct scoped_context
        {
            spirv_cross_api const& api;
            spvc_context context;
            ~scoped_context() { api.context_destroy(context); }
        } scopedContext{ api, context };
        auto const fail = [&](char const* aWhat)
        {
            throw failed_to_create_shader_program(std::string{ "SPIRV-Cross: " } + aWhat + ": " + api.context_get_last_error_string(context));
        };
        spvc_parsed_ir ir = nullptr;
        if (api.context_parse_spirv(context, reinterpret_cast<SpvId const*>(aSpirv.data()), aSpirv.size(), &ir) != SPVC_SUCCESS)
            fail("spvc_context_parse_spirv");
        spvc_compiler compiler = nullptr;
        if (api.context_create_compiler(context, SPVC_BACKEND_HLSL, ir, SPVC_CAPTURE_MODE_TAKE_OWNERSHIP, &compiler) != SPVC_SUCCESS)
            fail("spvc_context_create_compiler");
        spvc_compiler_options options = nullptr;
        if (api.compiler_create_compiler_options(compiler, &options) != SPVC_SUCCESS)
            fail("spvc_compiler_create_compiler_options");
        // n.b. registers in space 0 (shader model 5.1); clip space (depth and y) is already as Direct3D has it (see
        // basic_d3d12_shader_program::compile), so neither is fixed up
        api.compiler_options_set_uint(options, SPVC_COMPILER_OPTION_HLSL_SHADER_MODEL, 51u);
        api.compiler_options_set_bool(options, SPVC_COMPILER_OPTION_HLSL_POINT_SIZE_COMPAT, SPVC_TRUE);
        api.compiler_options_set_bool(options, SPVC_COMPILER_OPTION_HLSL_POINT_COORD_COMPAT, SPVC_TRUE);
        api.compiler_options_set_bool(options, SPVC_COMPILER_OPTION_FLIP_VERTEX_Y, SPVC_FALSE);
        api.compiler_options_set_bool(options, SPVC_COMPILER_OPTION_FIXUP_DEPTH_CONVENTION, SPVC_FALSE);
        if (api.compiler_install_compiler_options(compiler, options) != SPVC_SUCCESS)
            fail("spvc_compiler_install_compiler_options");
        char const* hlsl = nullptr;
        if (api.compiler_compile(compiler, &hlsl) != SPVC_SUCCESS || hlsl == nullptr)
            fail("spvc_compiler_compile");
        return fxc_discard_workaround(hlsl);
    }
}
