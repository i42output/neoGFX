// opengl_graphics_backend.cpp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2015-2026 Leigh Johnston.  All Rights Reserved.

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

#include <neogfx/gfx/i_texture.hpp>
#include <neogfx/gfx/i_fragment_shader.hpp>
#include "../i_native_texture.hpp"
#include "opengl_texture_manager.hpp"
#include "opengl_shader_program.hpp"
#include "../native_vertex.hpp"
#include "opengl_graphics_backend.hpp"

namespace neogfx
{
    namespace
    {
        inline GLenum to_gl_enum(gpu_primitive aPrimitive)
        {
            switch (aPrimitive)
            {
            case gpu_primitive::Triangles:
            default:
                return GL_TRIANGLES;
            case gpu_primitive::TriangleStrip:
                return GL_TRIANGLE_STRIP;
            }
        }

        inline GLenum to_gl_enum(gpu_attribute_type aType)
        {
            switch (aType)
            {
            case gpu_attribute_type::Float:
            default:
                return GL_FLOAT;
            case gpu_attribute_type::Double:
                return GL_DOUBLE;
            case gpu_attribute_type::UnsignedByte:
                return GL_UNSIGNED_BYTE;
            case gpu_attribute_type::UnsignedShort:
                return GL_UNSIGNED_SHORT;
            }
        }
    }

    namespace scene_shadows
    {
        constexpr GLsizei AtlasSize = scene_shadow_atlas::AtlasSize;

        // the depth only program, FBO and atlas (created when first needed; n.b. one GL context)
        struct resources
        {
            bool failed = false;
            GLuint program = 0;
            GLint viewProjection = -1;
            GLint modelTableBase = -1;
            GLint baseTexture = -1;
            GLint alphaTest = -1;
            GLint alphaCutoff = -1;
            GLint textureTransform = -1;
            GLint textureWrap = -1;
            GLuint framebuffer = 0;
            GLuint atlas = 0;
        };

        inline GLuint compile(GLenum aType, std::string const& aSource)
        {
            GLuint const shader = glCreateShader(aType);
            char const* source = aSource.c_str();
            glShaderSource(shader, 1, &source, nullptr);
            glCompileShader(shader);
            GLint ok = GL_FALSE;
            glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
            if (ok != GL_TRUE)
            {
                GLchar log[1024] = {};
                glGetShaderInfoLog(shader, sizeof(log), nullptr, log);
                service<debug::logger>() << neolib::logger::severity::Debug << "neogfx: shadow map shader: " << log << std::endl;
                glDeleteShader(shader);
                return 0;
            }
            return shader;
        }

        inline resources& get(i_standard_shader_program& aProgram)
        {
            static resources sResources;
            if (sResources.program != 0 || sResources.failed)
                return sResources;
            sResources.failed = true;
            // n.b. the same model transformation (and skinning) as standard.vert
            std::string vertexSource =
                "#version 460 core\n"
                "layout (location = 0) in vec3 VertexPosition;\n"
                "layout (location = 1) in vec4 VertexColor;\n"
                "layout (location = 2) in vec2 VertexTextureCoord;\n"
                "layout (location = 11) in float VertexModel;\n"
                "layout (location = 12) in vec4 VertexJoints;\n"
                "layout (location = 13) in vec4 VertexWeights;\n"
                "layout(std430, binding = %MATRICES%) buffer SSBO_bModelMatrices { mat4 bModelMatrices[]; };\n"
                "layout(std430, binding = %TABLE%) buffer SSBO_bModelTable { uint bModelTable[]; };\n"
                "uniform mat4 uViewProjection;\n"
                "uniform uint uModelTableBase;\n"
                "out vec2 TexCoord;\n"
                "out float Alpha;\n"
                "void main()\n"
                "{\n"
                "    TexCoord = VertexTextureCoord;\n"
                "    Alpha = VertexColor.a;\n"
                "    vec4 position = vec4(VertexPosition, 1.0);\n"
                "    if (VertexModel > 0.0)\n"
                "    {\n"
                "        uint first = bModelTable[uModelTableBase + uint(VertexModel + 0.5)];\n"
                "        if (any(greaterThan(VertexWeights, vec4(0.0))))\n"
                "        {\n"
                "            vec4 skinned = vec4(0.0);\n"
                "            for (int i = 0; i < 4; ++i)\n"
                "                if (VertexWeights[i] > 0.0)\n"
                "                    skinned += VertexWeights[i] * (bModelMatrices[first + 1u + uint(VertexJoints[i] + 0.5)] * position);\n"
                "            position = skinned;\n"
                "        }\n"
                "        position = bModelMatrices[first] * position;\n"
                "    }\n"
                "    gl_Position = uViewProjection * vec4(position.xyz / position.w, 1.0);\n"
                "}\n";
            auto const replace = [&](std::string const& aWhat, std::string const& aWith)
                {
                    vertexSource.replace(vertexSource.find(aWhat), aWhat.size(), aWith);
                };
            replace("%MATRICES%", std::to_string(static_cast<std::uint32_t>(aProgram.model_matrices().id())));
            replace("%TABLE%", std::to_string(static_cast<std::uint32_t>(aProgram.model_table().id())));
            // alpha tested (glTF alpha mode MASK) meshes' base colour alpha (wrapped as in standard-pbr.frag)
            std::string const fragmentSource =
                "#version 460 core\n"
                "in vec2 TexCoord;\n"
                "in float Alpha;\n"
                "uniform sampler2D uBaseTexture;\n"
                "uniform int uAlphaTest;\n"
                "uniform float uAlphaCutoff;\n"
                "uniform vec4 uTextureTransform;\n"
                "uniform ivec2 uTextureWrap;\n"
                "float wrap(float c, int mode)\n"
                "{\n"
                "    if (mode == 1)\n"
                "        return fract(c);\n"
                "    if (mode == 2)\n"
                "    {\n"
                "        float m = mod(c, 2.0);\n"
                "        return m > 1.0 ? 2.0 - m : m;\n"
                "    }\n"
                "    return clamp(c, 0.0, 1.0);\n"
                "}\n"
                "void main()\n"
                "{\n"
                "    if (uAlphaTest != 0)\n"
                "    {\n"
                "        vec2 coord = vec2(wrap(TexCoord.x, uTextureWrap.x), wrap(TexCoord.y, uTextureWrap.y)) * uTextureTransform.xy + uTextureTransform.zw;\n"
                "        vec2 unwrapped = TexCoord * uTextureTransform.xy;\n"
                "        if (Alpha * textureGrad(uBaseTexture, coord, dFdx(unwrapped), dFdy(unwrapped)).a < uAlphaCutoff)\n"
                "            discard;\n"
                "    }\n"
                "}\n";
            GLuint const vertexShader = compile(GL_VERTEX_SHADER, vertexSource);
            GLuint const fragmentShader = compile(GL_FRAGMENT_SHADER, fragmentSource);
            if (vertexShader == 0 || fragmentShader == 0)
                return sResources;
            GLuint const program = glCreateProgram();
            glAttachShader(program, vertexShader);
            glAttachShader(program, fragmentShader);
            glLinkProgram(program);
            glDeleteShader(vertexShader);
            glDeleteShader(fragmentShader);
            GLint ok = GL_FALSE;
            glGetProgramiv(program, GL_LINK_STATUS, &ok);
            if (ok != GL_TRUE)
            {
                glDeleteProgram(program);
                return sResources;
            }
            sResources.program = program;
            sResources.viewProjection = glGetUniformLocation(program, "uViewProjection");
            sResources.modelTableBase = glGetUniformLocation(program, "uModelTableBase");
            sResources.baseTexture = glGetUniformLocation(program, "uBaseTexture");
            sResources.alphaTest = glGetUniformLocation(program, "uAlphaTest");
            sResources.alphaCutoff = glGetUniformLocation(program, "uAlphaCutoff");
            sResources.textureTransform = glGetUniformLocation(program, "uTextureTransform");
            sResources.textureWrap = glGetUniformLocation(program, "uTextureWrap");
            glCheck(glCreateTextures(GL_TEXTURE_2D, 1, &sResources.atlas));
            glCheck(glTextureStorage2D(sResources.atlas, 1, GL_DEPTH_COMPONENT32F, AtlasSize, AtlasSize));
            glCheck(glTextureParameteri(sResources.atlas, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
            glCheck(glTextureParameteri(sResources.atlas, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
            glCheck(glTextureParameteri(sResources.atlas, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
            glCheck(glTextureParameteri(sResources.atlas, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
            glCheck(glTextureParameteri(sResources.atlas, GL_TEXTURE_COMPARE_MODE, GL_NONE));
            glCheck(glCreateFramebuffers(1, &sResources.framebuffer));
            glCheck(glNamedFramebufferTexture(sResources.framebuffer, GL_DEPTH_ATTACHMENT, sResources.atlas, 0));
            glCheck(glNamedFramebufferDrawBuffer(sResources.framebuffer, GL_NONE));
            glCheck(glNamedFramebufferReadBuffer(sResources.framebuffer, GL_NONE));
            if (glCheckNamedFramebufferStatus(sResources.framebuffer, GL_DRAW_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
                return sResources;
            sResources.failed = false;
            return sResources;
        }
    }

    namespace scene_background
    {
        // the background program: a quad covering the camera's view (its NDC square), each fragment the environment in the
        // direction it views (equirectangular: u = atan2(z, x) / 2pi + 0.5, v = 0.5 - asin(y) / pi), tone mapped and sRGB
        // encoded as standard-pbr.frag does; from the prefiltered environment texture (see pbr_environment in
        // fragment_shader.cpp: bands 0 to 5 prefiltered for roughness 0 to 1) or the background texture (RGBE texels: the
        // panorama at the top, versions of it halved repeatedly side by side below it; n.b. all filtering is done here (texel
        // fetches) so that it wraps horizontally)
        struct resources
        {
            bool failed = false;
            GLuint program = 0;
            GLuint vertexArray = 0;
            GLint ndcToClip = -1;
            GLint clipToWorld = -1;
            GLint source = -1;
            GLint blur = -1;
            GLint intensity = -1;
            GLint backgroundExtents = -1;
        };

        inline resources& get()
        {
            static resources sResources;
            if (sResources.program != 0 || sResources.failed)
                return sResources;
            sResources.failed = true;
            std::string const vertexSource =
                "#version 460 core\n"
                "uniform mat4 uNdcToClip;\n"
                "out vec2 Ndc;\n"
                "void main()\n"
                "{\n"
                "    Ndc = vec2(float((gl_VertexID & 1) * 2 - 1), float((gl_VertexID >> 1) * 2 - 1));\n"
                "    vec4 position = uNdcToClip * vec4(Ndc, 0.0, 1.0);\n"
                "    gl_Position = vec4(position.xy, 0.0, position.w);\n"
                "}\n";
            std::string const fragmentSource =
                "#version 460 core\n"
                "in vec2 Ndc;\n"
                "layout (location = 0) out vec4 FragColor;\n"
                "uniform mat4 uClipToWorld;\n"
                "uniform sampler2DRect uEnvironment;\n"
                "uniform sampler2DRect uBackground;\n"
                "uniform int uSource;\n"
                "uniform float uBlur;\n"
                "uniform float uIntensity;\n"
                "uniform ivec2 uBackgroundExtents;\n"
                "const float PI = 3.14159265358979;\n"
                "vec3 environment_texel(int band, ivec2 t)\n"
                "{\n"
                "    return texelFetch(uEnvironment, ivec2(((t.x % 256) + 256) % 256, band * 128 + clamp(t.y, 0, 127))).rgb;\n"
                "}\n"
                "vec3 environment_band(int band, vec2 uv)\n"
                "{\n"
                "    vec2 p = uv * vec2(256.0, 128.0) - 0.5;\n"
                "    ivec2 i = ivec2(floor(p));\n"
                "    vec2 f = p - vec2(i);\n"
                "    return mix(mix(environment_texel(band, i), environment_texel(band, i + ivec2(1, 0)), f.x),\n"
                "        mix(environment_texel(band, i + ivec2(0, 1)), environment_texel(band, i + ivec2(1, 1)), f.x), f.y);\n"
                "}\n"
                "int background_top_level()\n"
                "{\n"
                "    int level = 0;\n"
                "    for (ivec2 e = uBackgroundExtents; e.x > 8 && e.y > 4; e /= 2)\n"
                "        ++level;\n"
                "    return level;\n"
                "}\n"
                "ivec4 background_level(int level)\n"
                "{\n"
                "    if (level == 0)\n"
                "        return ivec4(0, 0, uBackgroundExtents);\n"
                "    int x = 0;\n"
                "    for (int i = 1; i < level; ++i)\n"
                "        x += uBackgroundExtents.x >> i;\n"
                "    return ivec4(x, uBackgroundExtents.y, uBackgroundExtents.x >> level, uBackgroundExtents.y >> level);\n"
                "}\n"
                "vec3 background_texel(ivec4 level, ivec2 t)\n"
                "{\n"
                "    vec4 rgbe = floor(texelFetch(uBackground, level.xy + ivec2(((t.x % level.z) + level.z) % level.z, clamp(t.y, 0, level.w - 1))) * 255.0 + 0.5);\n"
                "    return rgbe.a > 0.0 ? (rgbe.rgb + 0.5) * exp2(rgbe.a - 136.0) : vec3(0.0);\n"
                "}\n"
                "vec3 background_bilinear(int level, vec2 uv)\n"
                "{\n"
                "    ivec4 l = background_level(level);\n"
                "    vec2 p = uv * vec2(l.zw) - 0.5;\n"
                "    ivec2 i = ivec2(floor(p));\n"
                "    vec2 f = p - vec2(i);\n"
                "    return mix(mix(background_texel(l, i), background_texel(l, i + ivec2(1, 0)), f.x),\n"
                "        mix(background_texel(l, i + ivec2(0, 1)), background_texel(l, i + ivec2(1, 1)), f.x), f.y);\n"
                "}\n"
                "vec4 bspline(float t)\n"
                "{\n"
                "    float s = 1.0 - t;\n"
                "    return vec4(s * s * s, 4.0 - 6.0 * t * t + 3.0 * t * t * t, 4.0 - 6.0 * s * s + 3.0 * s * s * s, t * t * t) / 6.0;\n"
                "}\n"
                "// cubic B-spline: smooth where the halved versions are magnified\n"
                "vec3 background_bicubic(int level, vec2 uv)\n"
                "{\n"
                "    ivec4 l = background_level(level);\n"
                "    vec2 p = uv * vec2(l.zw) - 0.5;\n"
                "    ivec2 i = ivec2(floor(p));\n"
                "    vec2 f = p - vec2(i);\n"
                "    vec4 wx = bspline(f.x);\n"
                "    vec4 wy = bspline(f.y);\n"
                "    vec3 result = vec3(0.0);\n"
                "    for (int y = 0; y < 4; ++y)\n"
                "    {\n"
                "        vec3 row = vec3(0.0);\n"
                "        for (int x = 0; x < 4; ++x)\n"
                "            row += wx[x] * background_texel(l, i + ivec2(x - 1, y - 1));\n"
                "        result += wy[y] * row;\n"
                "    }\n"
                "    return result;\n"
                "}\n"
                "vec3 from_linear(vec3 c)\n"
                "{\n"
                "    c = clamp(c, 0.0, 1.0);\n"
                "    return mix(c * 12.92, 1.055 * pow(c, vec3(1.0 / 2.4)) - 0.055, step(vec3(0.0031308), c));\n"
                "}\n"
                "vec3 tone_map(vec3 color)\n"
                "{\n"
                "    const float startCompression = 0.8 - 0.04;\n"
                "    const float desaturation = 0.15;\n"
                "    float x = min(color.r, min(color.g, color.b));\n"
                "    float offset = x < 0.08 ? x - 6.25 * x * x : 0.04;\n"
                "    color -= offset;\n"
                "    float peak = max(color.r, max(color.g, color.b));\n"
                "    if (peak < startCompression)\n"
                "        return color;\n"
                "    const float d = 1.0 - startCompression;\n"
                "    float newPeak = 1.0 - d * d / (peak + d - startCompression);\n"
                "    color *= newPeak / peak;\n"
                "    float g = 1.0 - 1.0 / (desaturation * (peak - newPeak) + 1.0);\n"
                "    return mix(color, vec3(newPeak), g);\n"
                "}\n"
                "void main()\n"
                "{\n"
                "    vec4 nearPoint = uClipToWorld * vec4(Ndc, -1.0, 1.0);\n"
                "    vec4 farPoint = uClipToWorld * vec4(Ndc, 1.0, 1.0);\n"
                "    vec3 direction = normalize(farPoint.xyz * nearPoint.w - nearPoint.xyz * farPoint.w);\n"
                "    vec2 uv = vec2(atan(direction.z, direction.x) / (2.0 * PI) + 0.5, 0.5 - asin(clamp(direction.y, -1.0, 1.0)) / PI);\n"
                "    // the angle (radians) a fragment covers\n"
                "    float footprint = max(length(dFdx(direction)), length(dFdy(direction)));\n"
                "    float blur = clamp(uBlur, 0.0, 1.0);\n"
                "    vec3 radiance;\n"
                "    if (uSource == 1)\n"
                "    {\n"
                "        // the level whose texels match the blur (about the width of a GGX lobe of roughness blur) or the footprint\n"
                "        float texelsPerRadian = float(uBackgroundExtents.x) / (2.0 * PI);\n"
                "        float level = log2(max(max(footprint, 2.0 * blur * blur) * texelsPerRadian, 1.0));\n"
                "        level = min(level, float(background_top_level()));\n"
                "        int level0 = int(floor(level));\n"
                "        float t = level - float(level0);\n"
                "        radiance = level0 == 0 ? background_bilinear(0, uv) : background_bicubic(level0, uv);\n"
                "        if (t > 0.0)\n"
                "            radiance = mix(radiance, background_bicubic(level0 + 1, uv), t);\n"
                "    }\n"
                "    else\n"
                "    {\n"
                "        float band = blur * 5.0;\n"
                "        int band0 = min(int(floor(band)), 4);\n"
                "        radiance = mix(environment_band(band0, uv), environment_band(band0 + 1, uv), band - float(band0));\n"
                "    }\n"
                "    FragColor = vec4(from_linear(tone_map(radiance * uIntensity)), 1.0);\n"
                "}\n";
            GLuint const vertexShader = scene_shadows::compile(GL_VERTEX_SHADER, vertexSource);
            GLuint const fragmentShader = scene_shadows::compile(GL_FRAGMENT_SHADER, fragmentSource);
            if (vertexShader == 0 || fragmentShader == 0)
                return sResources;
            GLuint const program = glCreateProgram();
            glAttachShader(program, vertexShader);
            glAttachShader(program, fragmentShader);
            glLinkProgram(program);
            glDeleteShader(vertexShader);
            glDeleteShader(fragmentShader);
            GLint ok = GL_FALSE;
            glGetProgramiv(program, GL_LINK_STATUS, &ok);
            if (ok != GL_TRUE)
            {
                glDeleteProgram(program);
                return sResources;
            }
            sResources.program = program;
            sResources.ndcToClip = glGetUniformLocation(program, "uNdcToClip");
            sResources.clipToWorld = glGetUniformLocation(program, "uClipToWorld");
            sResources.source = glGetUniformLocation(program, "uSource");
            sResources.blur = glGetUniformLocation(program, "uBlur");
            sResources.intensity = glGetUniformLocation(program, "uIntensity");
            sResources.backgroundExtents = glGetUniformLocation(program, "uBackgroundExtents");
            glCheck(glProgramUniform1i(program, glGetUniformLocation(program, "uEnvironment"), static_cast<GLint>(reserved_texture_unit::PbrEnvironment)));
            glCheck(glProgramUniform1i(program, glGetUniformLocation(program, "uBackground"), static_cast<GLint>(reserved_texture_unit::Pbr0)));
            glCheck(glCreateVertexArrays(1, &sResources.vertexArray));
            sResources.failed = false;
            return sResources;
        }
    }

    opengl_graphics_backend::opengl_graphics_backend(neogfx::renderer aRenderer) :
        iRenderer{ aRenderer }
    {
    }

    opengl_graphics_backend::~opengl_graphics_backend()
    {
    }

    neogfx::renderer opengl_graphics_backend::renderer() const
    {
        return iRenderer;
    }

    void opengl_graphics_backend::initialize()
    {
        service<debug::logger>() << neolib::logger::severity::Debug << "OpenGL vendor: " << reinterpret_cast<const char*>(glGetString(GL_VENDOR)) << std::endl;
        service<debug::logger>() << neolib::logger::severity::Debug << "OpenGL renderer: " << reinterpret_cast<const char*>(glGetString(GL_RENDERER)) << std::endl;
        service<debug::logger>() << neolib::logger::severity::Debug << "OpenGL version: " << reinterpret_cast<const char*>(glGetString(GL_VERSION)) << std::endl;
        service<debug::logger>() << neolib::logger::severity::Debug << "OpenGL shading language version: " << reinterpret_cast<const char*>(glGetString(GL_SHADING_LANGUAGE_VERSION)) << std::endl;
    }

    void opengl_graphics_backend::cleanup()
    {
    }

    void opengl_graphics_backend::finish()
    {
        glCheck(glFinish());
    }

    void opengl_graphics_backend::execute()
    {
        // nothing to do
    }

    std::unique_ptr<texture_manager> opengl_graphics_backend::create_texture_manager()
    {
        return std::make_unique<opengl_texture_manager>();
    }

    ref_ptr<i_shader_program> opengl_graphics_backend::create_standard_shader_program()
    {
        return neolib::make_ref<opengl_standard_shader_program>().as<i_shader_program>();
    }

    void* opengl_graphics_backend::create_shader_program_object()
    {
        GLuint programHandle = 0;;
        glCheck(programHandle = glCreateProgram());
        if (0 == programHandle)
             throw i_rendering_engine::failed_to_create_shader_program("Failed to create shader program object");
        return to_opaque_handle(programHandle);
    }

    void opengl_graphics_backend::destroy_shader_program_object(void* aShaderProgramObject)
    {
        glCheck(glDeleteProgram(to_gl_handle<GLuint>(aShaderProgramObject)));
    }

    void* opengl_graphics_backend::create_shader_object(shader_type aShaderType)
    {
        GLenum shaderType;
        switch (aShaderType)
        {
        case shader_type::Compute:
            shaderType = GL_COMPUTE_SHADER;
            break;
        case shader_type::Vertex:
            shaderType = GL_VERTEX_SHADER;
            break;
        case shader_type::TessellationControl:
            shaderType = GL_TESS_CONTROL_SHADER;
            break;
        case shader_type::TessellationEvaluation:
            shaderType = GL_TESS_EVALUATION_SHADER;
            break;
        case shader_type::Geometry:
            shaderType = GL_GEOMETRY_SHADER;
            break;
        case shader_type::Fragment:
            shaderType = GL_FRAGMENT_SHADER;
            break;
        default:
            throw std::logic_error("neogfx: invalid shader type");
        }
        GLuint shaderHandle = 0;
        glCheck(shaderHandle = glCreateShader(shaderType));
        if (0 == shaderHandle)
            throw failed_to_create_shader();
        return to_opaque_handle(shaderHandle);
    }

    void opengl_graphics_backend::destroy_shader_object(void* aShaderObject)
    {
        glCheck(glDeleteShader(to_gl_handle<GLuint>(aShaderObject)));
    }

    viewport opengl_graphics_backend::viewport() const
    {
        GLint viewport[4];
        glGetIntegerv(GL_VIEWPORT, viewport);
        return rect{ point_i32{ viewport[0], viewport[1] }.as<scalar>(), size_i32{ viewport[2], viewport[3] }.as<scalar>() };
    }

    std::optional<rect> opengl_graphics_backend::scissor() const
    {
        if (!glIsEnabled(GL_SCISSOR_TEST))
            return std::nullopt;
        GLint scissor[4];
        glGetIntegerv(GL_SCISSOR_BOX, scissor);
        return rect{ point_i32{ scissor[0], scissor[1] }.as<scalar>(), size_i32{ scissor[2], scissor[3] }.as<scalar>() };
    }

    gpu_buffer opengl_graphics_backend::create_buffer(std::size_t aSize, bool aDeviceLocal)
    {
        GLuint bufferName = 0;
        glCheck(glCreateBuffers(1, &bufferName));
        glCheck(glNamedBufferStorage(bufferName, aSize, nullptr,
            !aDeviceLocal ? GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT : GL_DYNAMIC_STORAGE_BIT));
        return static_cast<gpu_buffer>(bufferName);
    }

    void opengl_graphics_backend::destroy_buffer(gpu_buffer aBuffer)
    {
        GLuint const bufferName = static_cast<GLuint>(aBuffer);
        glCheck(glDeleteBuffers(1, &bufferName));
    }

    void* opengl_graphics_backend::map_buffer(gpu_buffer aBuffer, std::size_t aSize)
    {
        void* result = nullptr;
        glCheck(result = glMapNamedBufferRange(static_cast<GLuint>(aBuffer), 0, aSize,
            GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_FLUSH_EXPLICIT_BIT));
        return result;
    }

    void opengl_graphics_backend::flush_buffer(gpu_buffer aBuffer, std::size_t aOffset, std::size_t aSize)
    {
        glCheck(glFlushMappedNamedBufferRange(static_cast<GLuint>(aBuffer), aOffset, aSize));
    }

    void opengl_graphics_backend::unmap_buffer(gpu_buffer aBuffer)
    {
        glCheck(glUnmapNamedBuffer(static_cast<GLuint>(aBuffer)));
    }

    bool opengl_graphics_backend::discard_buffer(gpu_buffer)
    {
        // n.b. unchanged: a persistently mapped buffer is not orphaned
        return false;
    }

    void opengl_graphics_backend::write_buffer(gpu_buffer aBuffer, std::size_t aOffset, void const* aData, std::size_t aSize)
    {
        glCheck(glNamedBufferSubData(static_cast<GLuint>(aBuffer), aOffset, aSize, aData));
    }

    void opengl_graphics_backend::copy_buffer(gpu_buffer aSource, gpu_buffer aDestination, std::size_t aSize)
    {
        glCheck(glCopyNamedBufferSubData(static_cast<GLuint>(aSource), static_cast<GLuint>(aDestination), 0, 0, aSize));
    }

    gpu_vertex_array opengl_graphics_backend::create_vertex_array()
    {
        GLuint handle = 0;
        glCheck(glGenVertexArrays(1, &handle));
        return static_cast<gpu_vertex_array>(handle);
    }

    void opengl_graphics_backend::destroy_vertex_array(gpu_vertex_array aVertexArray)
    {
        GLuint const handle = static_cast<GLuint>(aVertexArray);
        glCheck(glDeleteVertexArrays(1, &handle));
    }

    gpu_vertex_array opengl_graphics_backend::bound_vertex_array() const
    {
        GLint handle = 0;
        glCheck(glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &handle));
        return static_cast<gpu_vertex_array>(handle);
    }

    void opengl_graphics_backend::bind_vertex_array(gpu_vertex_array aVertexArray)
    {
        glCheck(glBindVertexArray(static_cast<GLuint>(aVertexArray)));
    }

    void opengl_graphics_backend::set_vertex_attribute(i_shader_program const& aProgram, std::string const& aName, gpu_buffer aBuffer,
        std::uint32_t aArity, gpu_attribute_type aType, bool aNormalized, std::size_t aStride, std::size_t aOffset)
    {
        GLint previousBindingHandle;
        glCheck(glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &previousBindingHandle));
        glCheck(glBindBuffer(GL_ARRAY_BUFFER, static_cast<GLuint>(aBuffer)));
        GLuint index;
        glCheck(index = glGetAttribLocation(to_gl_handle<GLuint>(aProgram.handle()), aName.c_str()));
        if (index != -1)
        {
            glCheck(glVertexAttribPointer(
                index,
                static_cast<GLint>(aArity),
                to_gl_enum(aType),
                aNormalized ? GL_TRUE : GL_FALSE,
                static_cast<GLsizei>(aStride),
                reinterpret_cast<const GLvoid*>(aOffset)));
            glCheck(glEnableVertexAttribArray(index));
        }
        if (previousBindingHandle != static_cast<GLint>(aBuffer))
            glCheck(glBindBuffer(GL_ARRAY_BUFFER, previousBindingHandle));
    }

    void opengl_graphics_backend::set_vertex_attribute(std::uint32_t aLocation, gpu_buffer aBuffer,
        std::uint32_t aArity, gpu_attribute_type aType, bool aNormalized, std::size_t aStride, std::size_t aOffset)
    {
        glCheck(glBindBuffer(GL_ARRAY_BUFFER, static_cast<GLuint>(aBuffer)));
        glCheck(glEnableVertexAttribArray(aLocation));
        glCheck(glVertexAttribPointer(aLocation, static_cast<GLint>(aArity), to_gl_enum(aType), aNormalized ? GL_TRUE : GL_FALSE,
            static_cast<GLsizei>(aStride), reinterpret_cast<const void*>(aOffset)));
    }

    void opengl_graphics_backend::set_index_buffer(gpu_buffer aBuffer)
    {
        glCheck(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLuint>(aBuffer)));
    }

    void opengl_graphics_backend::draw_arrays(gpu_primitive aPrimitive, std::size_t aFirst, std::size_t aCount)
    {
        glCheck(glDrawArrays(to_gl_enum(aPrimitive), static_cast<GLint>(aFirst), static_cast<GLsizei>(aCount)));
    }

    void opengl_graphics_backend::draw_elements(gpu_primitive aPrimitive, std::size_t aFirstIndex, std::size_t aCount)
    {
        glCheck(glDrawElements(to_gl_enum(aPrimitive), static_cast<GLsizei>(aCount), GL_UNSIGNED_INT,
            reinterpret_cast<const void*>(aFirstIndex * sizeof(std::uint32_t))));
    }

    void opengl_graphics_backend::texture_barrier()
    {
        glCheck(glTextureBarrier());
    }

    void opengl_graphics_backend::enable_scissor(bool aEnable)
    {
        glCheck((aEnable ? glEnable(GL_SCISSOR_TEST) : glDisable(GL_SCISSOR_TEST)));
    }

    void opengl_graphics_backend::set_scissor(std::int32_t aX, std::int32_t aY, std::int32_t aWidth, std::int32_t aHeight)
    {
        glCheck(glScissor(static_cast<GLint>(aX), static_cast<GLint>(aY), static_cast<GLsizei>(aWidth), static_cast<GLsizei>(aHeight)));
    }

    void opengl_graphics_backend::enable_multisample(bool aEnable)
    {
        if (aEnable)
        {
            glCheck(glEnable(GL_MULTISAMPLE));
        }
        else
        {
            glCheck(glDisable(GL_MULTISAMPLE));
        }
    }

    void opengl_graphics_backend::set_sample_shading(std::optional<double> const& aSampleShadingRate)
    {
        if (aSampleShadingRate)
        {
            glCheck(glEnable(GL_SAMPLE_SHADING));
            glCheck(glMinSampleShading(static_cast<float>(aSampleShadingRate.value())));
        }
        else
        {
            glCheck(glDisable(GL_SAMPLE_SHADING));
        }
    }

    void opengl_graphics_backend::set_front_face(neogfx::front_face aFrontFace)
    {
        switch (aFrontFace)
        {
        case neogfx::front_face::CounterClockwise:
            glCheck(glFrontFace(GL_CCW));
            break;
        case neogfx::front_face::Clockwise:
            glCheck(glFrontFace(GL_CW));
            break;
        }
    }

    void opengl_graphics_backend::set_face_culling(neogfx::face_culling aCulling, bool aFlipped)
    {
        switch (aCulling)
        {
        case neogfx::face_culling::None:
            glCheck(glDisable(GL_CULL_FACE));
            break;
        case neogfx::face_culling::Front:
            glCheck(glCullFace(!aFlipped ? GL_FRONT : GL_BACK));
            glCheck(glEnable(GL_CULL_FACE));
            break;
        case neogfx::face_culling::Back:
            glCheck(glCullFace(!aFlipped ? GL_BACK : GL_FRONT));
            glCheck(glEnable(GL_CULL_FACE));
            break;
        case neogfx::face_culling::FrontAndBack:
            glCheck(glCullFace(GL_FRONT_AND_BACK));
            glCheck(glEnable(GL_CULL_FACE));
            break;
        }
    }

    void opengl_graphics_backend::set_blending_mode(neogfx::blending_mode aBlendingMode)
    {
        switch (aBlendingMode)
        {
        case neogfx::blending_mode::None:
            glCheck(glDisable(GL_BLEND));
            break;
        case neogfx::blending_mode::Default:
            glCheck(glEnable(GL_BLEND));
            glCheck(glBlendEquation(GL_FUNC_ADD));
            glCheck(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
            break;
        case neogfx::blending_mode::Sprite:
            glCheck(glEnable(GL_BLEND));
            glCheck(glBlendEquation(GL_FUNC_ADD));
            glCheck(glBlendFuncSeparate(GL_ONE, GL_ONE_MINUS_SRC_ALPHA, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
            break;
        case neogfx::blending_mode::Blit:
            glCheck(glEnable(GL_BLEND));
            glCheck(glBlendEquation(GL_FUNC_ADD));
            glCheck(glBlendFunc(GL_ONE, GL_ZERO));
            break;
        case neogfx::blending_mode::Lighten:
            glCheck(glEnable(GL_BLEND));
            glCheck(glBlendEquationSeparate(GL_MAX, GL_FUNC_ADD));
            glCheck(glBlendFuncSeparate(GL_ONE, GL_ONE, GL_ONE, GL_ONE_MINUS_SRC_ALPHA));
            break;
        case neogfx::blending_mode::Filter:
            glCheck(glEnable(GL_BLEND));
            glCheck(glBlendEquation(GL_FUNC_ADD));
            glCheck(glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA));
            break;
        case neogfx::blending_mode::FilterFinish:
            glCheck(glEnable(GL_BLEND));
            glCheck(glBlendEquation(GL_FUNC_ADD));
            glCheck(glBlendFuncSeparate(GL_ONE, GL_ONE_MINUS_SRC_ALPHA, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
            break;
        case neogfx::blending_mode::Premultiply:
            glCheck(glEnable(GL_BLEND));
            glCheck(glBlendEquation(GL_FUNC_ADD));
            glCheck(glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA));
            break;
        }
    }

    void opengl_graphics_backend::set_xor_blending()
    {
        glCheck(glEnable(GL_BLEND));
        glCheck(glBlendEquation(GL_FUNC_ADD));
        glCheck(glBlendFunc(GL_ONE_MINUS_DST_COLOR, GL_ONE_MINUS_SRC_COLOR));
    }

    void opengl_graphics_backend::enable_smoothing(bool aEnable)
    {
        if (aEnable)
        {
            glCheck(glEnable(GL_LINE_SMOOTH));
            glCheck(glEnable(GL_POLYGON_SMOOTH));
        }
        else
        {
            glCheck(glDisable(GL_LINE_SMOOTH));
            glCheck(glDisable(GL_POLYGON_SMOOTH));
        }
    }

    void opengl_graphics_backend::clear(color const& aColor)
    {
        glCheck(glClearColor(aColor.red<GLclampf>(), aColor.green<GLclampf>(), aColor.blue<GLclampf>(), aColor.alpha<GLclampf>()));
        glCheck(glClear(GL_COLOR_BUFFER_BIT));
    }

    void opengl_graphics_backend::clear_depth_buffer()
    {
        GLboolean depthMask;
        glCheck(glGetBooleanv(GL_DEPTH_WRITEMASK, &depthMask));
        if (!depthMask)
            glCheck(glDepthMask(GL_TRUE));
        glCheck(glClearDepth(1.0));
        glCheck(glClear(GL_DEPTH_BUFFER_BIT));
        if (!depthMask)
            glCheck(glDepthMask(GL_FALSE));
    }

    void opengl_graphics_backend::clear_stencil_buffer(std::int32_t aValue)
    {
        glCheck(glStencilMask(0xFF));
        glCheck(glClearStencil(static_cast<GLint>(aValue)));
        glCheck(glClear(GL_STENCIL_BUFFER_BIT));
    }

    void opengl_graphics_backend::apply_stencil(bool aEnabled, bool aUpdating, std::int32_t aRef)
    {
        if (aEnabled)
        {
            glCheck(glEnable(GL_STENCIL_TEST));
            if (aUpdating)
            {
                glCheck(glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE));
                glCheck(glDepthMask(GL_FALSE));
                glCheck(glStencilFunc(GL_ALWAYS, aRef, 0xFF));
                glCheck(glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE));
                glCheck(glStencilMask(0xFF));
            }
            else
            {
                glCheck(glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE));
                glCheck(glDepthMask(GL_TRUE));
                glCheck(glStencilFunc(GL_EQUAL, aRef, 0xFF));
                glCheck(glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP));
                glCheck(glStencilMask(0x00));
            }
        }
        else
        {
            glCheck(glDisable(GL_STENCIL_TEST));
        }
    }

    bool opengl_graphics_backend::depth_test_enabled() const
    {
        return glIsEnabled(GL_DEPTH_TEST);
    }

    void opengl_graphics_backend::enable_depth_test(bool aEnable)
    {
        if (aEnable)
            glCheck(glEnable(GL_DEPTH_TEST))
        else
            glCheck(glDisable(GL_DEPTH_TEST))
    }

    void opengl_graphics_backend::set_texture_filter(i_texture const&, texture_sampling aSampling)
    {
        // n.b. the texture just bound is the one bound to the active texture unit
        glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, aSampling != texture_sampling::Nearest && aSampling != texture_sampling::Data ?
            GL_LINEAR :
            GL_NEAREST));
        glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, aSampling == texture_sampling::NormalMipmap ?
            GL_LINEAR_MIPMAP_LINEAR :
            aSampling != texture_sampling::Nearest && aSampling != texture_sampling::Data ?
            GL_LINEAR :
            GL_NEAREST));
    }

    void opengl_graphics_backend::set_texture_linear_clamp(i_texture const& aTexture)
    {
        auto const handle = static_cast<GLuint>(aTexture.native_handle());
        glCheck(glTextureParameteri(handle, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
        glCheck(glTextureParameteri(handle, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
        glCheck(glTextureParameteri(handle, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
        glCheck(glTextureParameteri(handle, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
    }

    void opengl_graphics_backend::set_active_texture_unit(std::uint32_t aTextureUnit)
    {
        glCheck(glActiveTexture(GL_TEXTURE0 + static_cast<GLenum>(aTextureUnit)));
    }

    void opengl_graphics_backend::unbind_texture_unit(std::uint32_t aTextureUnit)
    {
        glCheck(glBindTextureUnit(static_cast<GLuint>(aTextureUnit), 0));
    }

    bool opengl_graphics_backend::scene_shadows_available(i_standard_shader_program& aProgram)
    {
        return !scene_shadows::get(aProgram).failed;
    }

    void opengl_graphics_backend::draw_scene_shadow_maps(i_standard_shader_program& aProgram, native_scene_buffer& aSceneBuffer,
        std::vector<mat44> const& aViews, std::int32_t aDirectionalView,
        std::vector<std::optional<gpu_mesh_range>> const& aMeshes, std::vector<std::optional<scene_shadow_alpha_test>> const& aAlphaTests,
        std::uint32_t aModelTableBase)
    {
        auto& resources = scene_shadows::get(aProgram);

        // the shadow maps: depth only, drawn with their own program into the atlas (n.b. GL state saved and restored)
        GLint previousDrawFramebuffer = 0;
        GLint previousReadFramebuffer = 0;
        GLint previousViewport[4] = {};
        GLint previousScissor[4] = {};
        GLint previousDepthFunc = GL_LESS;
        GLboolean previousDepthMask = GL_TRUE;
        glCheck(glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &previousDrawFramebuffer));
        glCheck(glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &previousReadFramebuffer));
        glCheck(glGetIntegerv(GL_VIEWPORT, previousViewport));
        glCheck(glGetIntegerv(GL_SCISSOR_BOX, previousScissor));
        glCheck(glGetIntegerv(GL_DEPTH_FUNC, &previousDepthFunc));
        glCheck(glGetBooleanv(GL_DEPTH_WRITEMASK, &previousDepthMask));
        bool const previousDepthTest = glIsEnabled(GL_DEPTH_TEST);
        bool const previousScissorTest = glIsEnabled(GL_SCISSOR_TEST);
        bool const previousCullFace = glIsEnabled(GL_CULL_FACE);
        bool const previousBlend = glIsEnabled(GL_BLEND);
        bool const previousPolygonOffset = glIsEnabled(GL_POLYGON_OFFSET_FILL);

        glCheck(glBindFramebuffer(GL_FRAMEBUFFER, resources.framebuffer));
        glCheck(glUseProgram(resources.program));
        glCheck(glUniform1ui(resources.modelTableBase, aModelTableBase));
        // alpha tested meshes' base colour textures are bound to reserved_texture_unit::Pbr0 (unbound by the time it is used)
        glCheck(glUniform1i(resources.baseTexture, static_cast<GLint>(reserved_texture_unit::Pbr0)));
        glCheck(glEnable(GL_DEPTH_TEST));
        glCheck(glDepthFunc(GL_LESS));
        glCheck(glDepthMask(GL_TRUE));
        glCheck(glDisable(GL_CULL_FACE));
        glCheck(glDisable(GL_BLEND));
        glCheck(glEnable(GL_SCISSOR_TEST));
        // slope scaled depth bias against shadow acne
        glCheck(glEnable(GL_POLYGON_OFFSET_FILL));
        glCheck(glPolygonOffset(1.5f, 2.0f));
        for (std::uint32_t view = 0u; view < aViews.size(); ++view)
        {
            if (view == 0u && aDirectionalView != 0)
                continue;
            auto const tile = scene_shadow_atlas::tile(view);
            glCheck(glViewport(tile[0], tile[1], tile[2], tile[3]));
            glCheck(glScissor(tile[0], tile[1], tile[2], tile[3]));
            glCheck(glClear(GL_DEPTH_BUFFER_BIT));
            auto const viewProjection = aViews[view].as<float>();
            glCheck(glUniformMatrix4fv(resources.viewProjection, 1, GL_FALSE, viewProjection.data()));
            for (std::size_t meshIndex = 0u; meshIndex < aMeshes.size(); ++meshIndex)
            {
                auto const& mesh = aMeshes[meshIndex];
                if (!mesh)
                    continue;
                auto const& alphaTest = aAlphaTests[meshIndex];
                glCheck(glUniform1i(resources.alphaTest, alphaTest ? 1 : 0));
                if (alphaTest)
                {
                    auto const& [texture, transform, cutoff, wrapS, wrapT] = *alphaTest;
                    glCheck(glBindTextureUnit(static_cast<GLuint>(reserved_texture_unit::Pbr0), static_cast<GLuint>(texture->native_handle())));
                    glCheck(glUniform4f(resources.textureTransform, transform.x, transform.y, transform.z, transform.w));
                    glCheck(glUniform1f(resources.alphaCutoff, cutoff));
                    glCheck(glUniform2i(resources.textureWrap, static_cast<GLint>(wrapS), static_cast<GLint>(wrapT)));
                }
                aSceneBuffer.draw_depth(mesh->indexStart, mesh->indexEnd - mesh->indexStart);
            }
        }

        glCheck(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, static_cast<GLuint>(previousDrawFramebuffer)));
        glCheck(glBindFramebuffer(GL_READ_FRAMEBUFFER, static_cast<GLuint>(previousReadFramebuffer)));
        glCheck(glViewport(previousViewport[0], previousViewport[1], previousViewport[2], previousViewport[3]));
        glCheck(glScissor(previousScissor[0], previousScissor[1], previousScissor[2], previousScissor[3]));
        glCheck(glDepthFunc(static_cast<GLenum>(previousDepthFunc)));
        glCheck(glDepthMask(previousDepthMask));
        if (!previousDepthTest)
            glCheck(glDisable(GL_DEPTH_TEST));
        if (!previousScissorTest)
            glCheck(glDisable(GL_SCISSOR_TEST));
        if (previousCullFace)
            glCheck(glEnable(GL_CULL_FACE));
        if (previousBlend)
            glCheck(glEnable(GL_BLEND));
        if (!previousPolygonOffset)
            glCheck(glDisable(GL_POLYGON_OFFSET_FILL));
        glCheck(glBindTextureUnit(static_cast<GLuint>(reserved_texture_unit::Pbr0), 0));
        // n.b. the standard program still considers itself active
        glCheck(glUseProgram(static_cast<GLuint>(reinterpret_cast<std::intptr_t>(aProgram.handle()))));

        glCheck(glBindTextureUnit(static_cast<GLuint>(reserved_texture_unit::PbrShadow), resources.atlas));
    }

    void opengl_graphics_backend::draw_scene_background(i_standard_shader_program& aProgram, mat44 const& aNdcToClip, mat44 const& aClipToWorld,
        i_texture const* aBackgroundTexture, i_texture const* aEnvironment, double aBlur, double aIntensity)
    {
        auto& resources = scene_background::get();
        if (resources.failed)
            return;

        GLint previousVertexArray = 0;
        glCheck(glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &previousVertexArray));
        GLboolean previousDepthMask = GL_TRUE;
        glCheck(glGetBooleanv(GL_DEPTH_WRITEMASK, &previousDepthMask));
        bool const previousDepthTest = glIsEnabled(GL_DEPTH_TEST);
        bool const previousCullFace = glIsEnabled(GL_CULL_FACE);
        bool const previousBlend = glIsEnabled(GL_BLEND);

        glCheck(glUseProgram(resources.program));
        auto const ndcToClipf = aNdcToClip.as<float>();
        auto const clipToWorldf = aClipToWorld.as<float>();
        glCheck(glUniformMatrix4fv(resources.ndcToClip, 1, GL_FALSE, ndcToClipf.data()));
        glCheck(glUniformMatrix4fv(resources.clipToWorld, 1, GL_FALSE, clipToWorldf.data()));
        glCheck(glUniform1i(resources.source, aBackgroundTexture != nullptr ? 1 : 0));
        glCheck(glUniform1f(resources.blur, static_cast<float>(aBlur)));
        glCheck(glUniform1f(resources.intensity, static_cast<float>(aIntensity)));
        if (aBackgroundTexture != nullptr)
        {
            // the panorama's extents: the texture is half as tall again (see i_pbr_shader::set_background_texture)
            auto const extents = aBackgroundTexture->storage_extents();
            glCheck(glUniform2i(resources.backgroundExtents, static_cast<GLint>(extents.cx), static_cast<GLint>(extents.cy * 2.0 / 3.0 + 0.5)));
            glCheck(glBindTextureUnit(static_cast<GLuint>(reserved_texture_unit::Pbr0), static_cast<GLuint>(aBackgroundTexture->native_handle())));
        }
        else
            glCheck(glBindTextureUnit(static_cast<GLuint>(reserved_texture_unit::PbrEnvironment), static_cast<GLuint>(aEnvironment->native_handle())));
        glCheck(glDisable(GL_DEPTH_TEST));
        glCheck(glDepthMask(GL_FALSE));
        glCheck(glDisable(GL_CULL_FACE));
        glCheck(glDisable(GL_BLEND));
        glCheck(glBindVertexArray(resources.vertexArray));
        glCheck(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));

        glCheck(glBindVertexArray(static_cast<GLuint>(previousVertexArray)));
        glCheck(glBindTextureUnit(static_cast<GLuint>(aBackgroundTexture != nullptr ? reserved_texture_unit::Pbr0 : reserved_texture_unit::PbrEnvironment), 0));
        glCheck(glDepthMask(previousDepthMask));
        if (previousDepthTest)
            glCheck(glEnable(GL_DEPTH_TEST));
        if (previousCullFace)
            glCheck(glEnable(GL_CULL_FACE));
        if (previousBlend)
            glCheck(glEnable(GL_BLEND));
        // n.b. the standard program still considers itself active
        glCheck(glUseProgram(static_cast<GLuint>(reinterpret_cast<std::intptr_t>(aProgram.handle()))));
    }
}
