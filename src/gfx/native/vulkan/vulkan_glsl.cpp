// vulkan_glsl.cpp
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

#include <algorithm>
#include <cctype>
#include <functional>
#include <regex>
#include <sstream>
#include <unordered_map>

#include "vulkan_glsl.hpp"

namespace neogfx
{
    namespace
    {
        bool identifier_char(char aChar)
        {
            return std::isalnum(static_cast<unsigned char>(aChar)) || aChar == '_';
        }

        // replace whole identifiers
        void replace_identifier(std::string& aSource, std::string const& aFrom, std::string const& aTo)
        {
            std::size_t position = 0u;
            while ((position = aSource.find(aFrom, position)) != std::string::npos)
            {
                bool const startOk = (position == 0u || !identifier_char(aSource[position - 1u]));
                bool const endOk = (position + aFrom.size() >= aSource.size() || !identifier_char(aSource[position + aFrom.size()]));
                if (startOk && endOk)
                {
                    aSource.replace(position, aFrom.size(), aTo);
                    position += aTo.size();
                }
                else
                    position += aFrom.size();
            }
        }

        // the position of the parenthesis closing the one at aOpen
        std::size_t closing_parenthesis(std::string const& aSource, std::size_t aOpen)
        {
            int depth = 0;
            for (std::size_t i = aOpen; i < aSource.size(); ++i)
            {
                if (aSource[i] == '(')
                    ++depth;
                else if (aSource[i] == ')' && --depth == 0)
                    return i;
            }
            return std::string::npos;
        }

        // the top level arguments of the call whose parentheses are at aOpen and aClose
        std::vector<std::string> arguments(std::string const& aSource, std::size_t aOpen, std::size_t aClose)
        {
            std::vector<std::string> result;
            int depth = 0;
            std::size_t start = aOpen + 1u;
            for (std::size_t i = aOpen + 1u; i < aClose; ++i)
            {
                if (aSource[i] == '(' || aSource[i] == '[')
                    ++depth;
                else if (aSource[i] == ')' || aSource[i] == ']')
                    --depth;
                else if (aSource[i] == ',' && depth == 0)
                {
                    result.push_back(aSource.substr(start, i - start));
                    start = i + 1u;
                }
            }
            result.push_back(aSource.substr(start, aClose - start));
            return result;
        }

        std::string trimmed(std::string const& aString)
        {
            auto const first = aString.find_first_not_of(" \t\r\n");
            if (first == std::string::npos)
                return {};
            auto const last = aString.find_last_not_of(" \t\r\n");
            return aString.substr(first, last - first + 1u);
        }

        // a rectangle sampler's calls adapted to a 2D sampler (n.b. a 2D sampler's texture coordinates are normalized
        // and its texelFetch and textureSize take a level of detail)
        void adapt_rectangle_sampler(std::string& aSource, std::string const& aSampler)
        {
            static std::string const sFunctions[] = { "texelFetch", "textureSize", "textureLod", "texture" };
            std::size_t position = 0u;
            while (position < aSource.size())
            {
                std::size_t found = std::string::npos;
                std::string const* function = nullptr;
                for (auto const& f : sFunctions)
                {
                    std::size_t p = position;
                    while ((p = aSource.find(f, p)) != std::string::npos)
                    {
                        bool const startOk = (p == 0u || !identifier_char(aSource[p - 1u]));
                        auto q = p + f.size();
                        while (q < aSource.size() && std::isspace(static_cast<unsigned char>(aSource[q])))
                            ++q;
                        if (startOk && q < aSource.size() && aSource[q] == '(')
                            break;
                        p += f.size();
                    }
                    if (p != std::string::npos && (found == std::string::npos || p < found))
                    {
                        found = p;
                        function = &f;
                    }
                }
                if (found == std::string::npos)
                    break;
                auto const open = aSource.find('(', found + function->size());
                auto const close = closing_parenthesis(aSource, open);
                if (close == std::string::npos)
                    break;
                auto args = arguments(aSource, open, close);
                if (args.empty() || trimmed(args[0]) != aSampler)
                {
                    position = open + 1u;
                    continue;
                }
                std::string replacement;
                if (*function == "texelFetch")
                {
                    if (args.size() == 2u)
                        replacement = "texelFetch(" + aSampler + ", " + trimmed(args[1]) + ", 0)";
                }
                else if (*function == "textureSize")
                {
                    if (args.size() == 1u)
                        replacement = "textureSize(" + aSampler + ", 0)";
                }
                else if (*function == "texture")
                {
                    if (args.size() >= 2u)
                        replacement = "textureLod(" + aSampler + ", (" + trimmed(args[1]) + ") / vec2(textureSize(" + aSampler + ", 0)), 0.0)";
                }
                else if (*function == "textureLod")
                {
                    if (args.size() == 3u)
                        replacement = "textureLod(" + aSampler + ", (" + trimmed(args[1]) + ") / vec2(textureSize(" + aSampler + ", 0)), " + trimmed(args[2]) + ")";
                }
                if (replacement.empty())
                {
                    position = open + 1u;
                    continue;
                }
                aSource.replace(found, close + 1u - found, replacement);
                // n.b. the replacement's own calls are not adapted again
                position = found + replacement.size();
            }
        }
    }

    std::string vulkan_glsl(std::string const& aSource, VkShaderStageFlagBits aStage, vulkan_glsl_samplers& aSamplers,
        std::uint32_t aStorageBindingBase, std::uint32_t aSamplerBindingBase)
    {
        std::string result;
        result.reserve(aSource.size() + 1024u);

        static std::regex const sUniformBlock{ R"(layout\s*\(\s*binding\s*=\s*(\d+)\s*\)\s*uniform\b)" };
        static std::regex const sStorageBlock{ R"(layout\s*\(\s*std430\s*,\s*binding\s*=\s*(\d+)\s*\)\s*buffer\b)" };
        static std::regex const sSampler{ R"(^(\s*)uniform\s+(sampler2DRect|sampler2DMS|sampler2D)\s+(\w+)\s*;(.*)$)" };

        std::vector<std::string> rectangleSamplers;
        std::istringstream lines{ aSource };
        std::string line;
        while (std::getline(lines, line))
        {
            std::smatch match;
            if (std::regex_search(line, match, sUniformBlock))
                line = match.prefix().str() + "layout (std140, set = 0, binding = " + match[1].str() + ") uniform" + match.suffix().str();
            else if (std::regex_search(line, match, sStorageBlock))
                line = match.prefix().str() + "layout (std430, set = 0, binding = " +
                    std::to_string(aStorageBindingBase + static_cast<std::uint32_t>(std::stoul(match[1].str()))) + ") buffer" + match.suffix().str();
            else if (std::regex_match(line, match, sSampler))
            {
                auto const type = match[2].str();
                auto const name = match[3].str();
                auto existing = aSamplers.find(name);
                if (existing == aSamplers.end())
                {
                    std::uint32_t binding = aSamplerBindingBase;
                    for (auto const& s : aSamplers)
                        binding = std::max(binding, s.second.binding + 1u);
                    existing = aSamplers.emplace(name, vulkan_glsl_sampler{ binding, type == "sampler2DMS", type == "sampler2DRect", 0u }).first;
                }
                existing->second.multisample = (type == "sampler2DMS");
                existing->second.rectangle = (type == "sampler2DRect");
                existing->second.stages |= aStage;
                if (existing->second.rectangle)
                    rectangleSamplers.push_back(name);
                line = match[1].str() + "layout (set = 0, binding = " + std::to_string(existing->second.binding) + ") uniform " +
                    (type == "sampler2DMS" ? "sampler2DMS " : "sampler2D ") + name + ";" + match[4].str();
            }
            result += line;
            result += '\n';
        }

        for (auto const& sampler : rectangleSamplers)
            adapt_rectangle_sampler(result, sampler);

        replace_identifier(result, "gl_VertexID", "gl_VertexIndex");
        replace_identifier(result, "gl_InstanceID", "gl_InstanceIndex");

        return result;
    }

    std::optional<spirv_block> spirv_uniform_block(std::vector<std::uint32_t> const& aSpirv, std::string const& aBlockName)
    {
        if (aSpirv.size() < 5u || aSpirv[0] != 0x07230203u)
            return {};

        enum : std::uint32_t
        {
            OpName = 5u,
            OpMemberName = 6u,
            OpTypeBool = 20u,
            OpTypeInt = 21u,
            OpTypeFloat = 22u,
            OpTypeVector = 23u,
            OpTypeMatrix = 24u,
            OpTypeArray = 28u,
            OpTypeStruct = 30u,
            OpConstant = 43u,
            OpDecorate = 71u,
            OpMemberDecorate = 72u,
            DecorationArrayStride = 6u,
            DecorationMatrixStride = 7u,
            DecorationOffset = 35u
        };

        auto const string_at = [&](std::size_t aWord, std::size_t aEnd)
            {
                std::string result;
                for (std::size_t w = aWord; w < aEnd; ++w)
                    for (std::uint32_t b = 0u; b < 4u; ++b)
                    {
                        char const c = static_cast<char>((aSpirv[w] >> (b * 8u)) & 0xFFu);
                        if (c == '\0')
                            return result;
                        result += c;
                    }
                return result;
            };

        struct type_info
        {
            std::uint32_t opcode = 0u;
            std::uint32_t width = 0u;          // scalar
            std::uint32_t componentType = 0u;  // vector, matrix (column), array (element)
            std::uint32_t count = 0u;          // vector, matrix (columns), array (length constant id)
            std::vector<std::uint32_t> members;
        };
        std::unordered_map<std::uint32_t, std::string> names;
        std::unordered_map<std::uint32_t, std::map<std::uint32_t, std::string>> memberNames;
        std::unordered_map<std::uint32_t, type_info> types;
        std::unordered_map<std::uint32_t, std::uint32_t> constants;
        std::unordered_map<std::uint32_t, std::uint32_t> arrayStrides;
        std::unordered_map<std::uint32_t, std::map<std::uint32_t, std::uint32_t>> memberOffsets;
        std::unordered_map<std::uint32_t, std::map<std::uint32_t, std::uint32_t>> memberMatrixStrides;

        for (std::size_t word = 5u; word < aSpirv.size();)
        {
            auto const wordCount = aSpirv[word] >> 16u;
            auto const opcode = aSpirv[word] & 0xFFFFu;
            if (wordCount == 0u || word + wordCount > aSpirv.size())
                return {};
            auto const operand = [&](std::size_t aIndex) { return aSpirv[word + 1u + aIndex]; };
            switch (opcode)
            {
            case OpName:
                names[operand(0)] = string_at(word + 2u, word + wordCount);
                break;
            case OpMemberName:
                memberNames[operand(0)][operand(1)] = string_at(word + 3u, word + wordCount);
                break;
            case OpTypeBool:
                types[operand(0)] = type_info{ opcode, 32u };
                break;
            case OpTypeInt:
            case OpTypeFloat:
                types[operand(0)] = type_info{ opcode, operand(1) };
                break;
            case OpTypeVector:
            case OpTypeMatrix:
                types[operand(0)] = type_info{ opcode, 0u, operand(1), operand(2) };
                break;
            case OpTypeArray:
                types[operand(0)] = type_info{ opcode, 0u, operand(1), operand(2) };
                break;
            case OpTypeStruct:
                {
                    type_info info{ opcode };
                    for (std::size_t m = 1u; m < wordCount - 1u; ++m)
                        info.members.push_back(operand(m));
                    types[operand(0)] = info;
                }
                break;
            case OpConstant:
                constants[operand(1)] = operand(2);
                break;
            case OpDecorate:
                if (operand(1) == DecorationArrayStride)
                    arrayStrides[operand(0)] = operand(2);
                break;
            case OpMemberDecorate:
                if (operand(2) == DecorationOffset)
                    memberOffsets[operand(0)][operand(1)] = operand(3);
                else if (operand(2) == DecorationMatrixStride)
                    memberMatrixStrides[operand(0)][operand(1)] = operand(3);
                break;
            default:
                break;
            }
            word += wordCount;
        }

        std::optional<std::uint32_t> blockType;
        for (auto const& n : names)
            if (n.second == aBlockName && types.count(n.first) != 0u && types[n.first].opcode == OpTypeStruct)
                blockType = n.first;
        if (!blockType)
            return {};

        std::function<std::uint32_t(std::uint32_t, std::uint32_t)> size_of = [&](std::uint32_t aType, std::uint32_t aMatrixStride) -> std::uint32_t
            {
                auto const& type = types[aType];
                switch (type.opcode)
                {
                case OpTypeBool:
                case OpTypeInt:
                case OpTypeFloat:
                    return type.width / 8u;
                case OpTypeVector:
                    return type.count * size_of(type.componentType, 0u);
                case OpTypeMatrix:
                    return type.count * (aMatrixStride != 0u ? aMatrixStride : size_of(type.componentType, 0u));
                case OpTypeArray:
                    return constants[type.count] * (arrayStrides.count(aType) != 0u ? arrayStrides[aType] : size_of(type.componentType, aMatrixStride));
                case OpTypeStruct:
                    {
                        std::uint32_t result = 0u;
                        for (std::uint32_t m = 0u; m < type.members.size(); ++m)
                            result = std::max(result, memberOffsets[aType][m] + size_of(type.members[m], memberMatrixStrides[aType][m]));
                        return result;
                    }
                default:
                    return 0u;
                }
            };

        spirv_block result;
        auto const& block = types[*blockType];
        for (std::uint32_t m = 0u; m < block.members.size(); ++m)
        {
            auto const memberType = block.members[m];
            auto const offset = memberOffsets[*blockType][m];
            auto const size = size_of(memberType, memberMatrixStrides[*blockType][m]);
            auto const arrayStride = (types[memberType].opcode == OpTypeArray && arrayStrides.count(memberType) != 0u ? arrayStrides[memberType] : 0u);
            result.members[memberNames[*blockType][m]] = spirv_block_member{ offset, size, arrayStride };
            result.size = std::max(result.size, offset + size);
        }
        result.size = (result.size + 15u) / 16u * 16u;
        return result;
    }
}
