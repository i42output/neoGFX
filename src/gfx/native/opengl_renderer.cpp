// opengl_renderer.cpp
/*
  neogfx C++ GUI Library
  Copyright (c) 2015 Leigh Johnston.  All Rights Reserved.
  
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
#include <boost/filesystem.hpp>

#ifdef _WIN32
#include <ShellScalingApi.h>
#include <D2d1.h>
#pragma comment(lib, "Shcore.lib")
#endif

#include <neolib/thread.hpp>

#include <neogfx/core/numerical.hpp>

#include <neogfx/hid/i_display.hpp>
#include <neogfx/hid/i_surface_manager.hpp>
#include <neogfx/gfx/i_rendering_engine.hpp>
#include <neogfx/app/i_basic_services.hpp>
#include <neogfx/gui/widget/i_widget.hpp>
#include "opengl_renderer.hpp"
#include "i_native_texture.hpp"
#include "../../gui/window/native/opengl_window.hpp"
#include "gradient.frag.hpp"

namespace neogfx
{
    template <>
    i_font_manager& service<i_font_manager>()
    {
        return service<i_rendering_engine>().font_manager();
    }

    template <>
    i_texture_manager& service<i_texture_manager>()
    {
        return service<i_rendering_engine>().texture_manager();
    }
}

namespace neogfx
{
    frame_counter::frame_counter(uint32_t aDuration) : iTimer{ service<neolib::async_task>(), [this](neolib::callback_timer& aTimer)
        {
            aTimer.again();
            ++iCounter;
            for (auto w : iWidgets)
                w->update();
        }, aDuration }, iCounter{ 0 }
    {
    }

    uint32_t frame_counter::counter() const
    {
        return iCounter;
    }

    void frame_counter::add(i_widget& aWidget)
    {
        auto iterWidget = std::find(iWidgets.begin(), iWidgets.end(), &aWidget);
        if (iterWidget == iWidgets.end())
            iWidgets.push_back(&aWidget);
    }

    void frame_counter::remove(i_widget& aWidget)
    {
        auto iterWidget = std::find(iWidgets.begin(), iWidgets.end(), &aWidget);
        if (iterWidget != iWidgets.end())
            iWidgets.erase(iterWidget);
    }

    opengl_renderer::shader_program::shader_program(GLuint aHandle, bool aHasProjectionMatrix, bool aHasTransformationMatrix) :
        iHandle{ aHandle }, 
        iHasProjectionMatrix{ aHasProjectionMatrix }, 
        iHasTransformationMatrix{ aHasTransformationMatrix }
    {
    }

    void* opengl_renderer::shader_program::handle() const
    {
        return reinterpret_cast<void*>(static_cast<intptr_t>(iHandle));
    }

    bool opengl_renderer::shader_program::has_projection_matrix() const
    {
        return iHasProjectionMatrix;
    }

    const optional_mat44& opengl_renderer::shader_program::projection_matrix() const
    {
        return iProjectionMatrix;
    }

    void opengl_renderer::shader_program::set_projection_matrix(const i_rendering_context& aGraphicsContext, const optional_mat44& aProjectionMatrix)
    {
        if (aProjectionMatrix == std::nullopt)
        {
            iLogicalCoordinates = aGraphicsContext.logical_coordinates();
            double left = iLogicalCoordinates.bottomLeft.x;
            double right = iLogicalCoordinates.topRight.x;
            double bottom = iLogicalCoordinates.bottomLeft.y;
            double top = iLogicalCoordinates.topRight.y;
            double zFar = 1.0;
            double zNear = -1.0;
            mat44 orthoMatrix = mat44{
                { 2.0 / (right - left), 0.0, 0.0, -(right + left) / (right - left) },
                { 0.0, 2.0 / (top - bottom), 0.0, -(top + bottom) / (top - bottom) },
                { 0.0, 0.0, -2.0 / (zFar - zNear), -(zFar + zNear) / (zFar - zNear) },
                { 0.0, 0.0, 0.0, 1.0 } };
            set_projection_matrix(aGraphicsContext, orthoMatrix);
            return;
        }
        if (iProjectionMatrix != aProjectionMatrix)
        {
            iProjectionMatrix = aProjectionMatrix;
            set_uniform_matrix("uProjectionMatrix", basic_matrix<float, 4, 4>{ *iProjectionMatrix }.transposed());
        }
    }

    bool opengl_renderer::shader_program::has_transformation_matrix() const
    {
        return iHasTransformationMatrix;
    }

    const optional_mat44& opengl_renderer::shader_program::transformation_matrix() const
    {
        return iTransformationMatrix;
    }

    void opengl_renderer::shader_program::set_transformation_matrix(const i_rendering_context& aGraphicsContext, const optional_mat44& aTransformationMatrix)
    {
        if (aTransformationMatrix == std::nullopt)
        {
            set_transformation_matrix(aGraphicsContext, mat44::identity());
            return;
        }
        if (iTransformationMatrix != aTransformationMatrix)
        {
            iTransformationMatrix = aTransformationMatrix;
            auto transformationMatrix = basic_matrix<float, 4, 4>{ *iTransformationMatrix };
            set_uniform_matrix("uTransformationMatrix", transformationMatrix);
        }
    }

    void* opengl_renderer::shader_program::variable(const std::string& aVariableName) const
    {
        auto v = iVariables.find(aVariableName);
        if (v == iVariables.end())
            throw variable_not_found();
        return reinterpret_cast<void*>(static_cast<intptr_t>(v->second));
    }

    void opengl_renderer::shader_program::set_uniform_variable(const std::string& aName, float aValue)
    {
        glUniform1f(uniform_location(aName), aValue);
        GLenum errorCode = glGetError();
        if (errorCode != GL_NO_ERROR)
            throw shader_program_error(errorCode);
    }

    void opengl_renderer::shader_program::set_uniform_variable(const std::string& aName, double aValue)
    {
        glUniform1d(uniform_location(aName), aValue);
        GLenum errorCode = glGetError();
        if (errorCode != GL_NO_ERROR)
            throw shader_program_error(errorCode);
    }

    void opengl_renderer::shader_program::set_uniform_variable(const std::string& aName, int aValue)
    {
        glUniform1i(uniform_location(aName), aValue);
        GLenum errorCode = glGetError();
        if (errorCode != GL_NO_ERROR)
            throw shader_program_error(errorCode);
    }

    void opengl_renderer::shader_program::set_uniform_variable(const std::string& aName, double aValue1, double aValue2)
    {
        glUniform2d(uniform_location(aName), aValue1, aValue2);
        GLenum errorCode = glGetError();
        if (errorCode != GL_NO_ERROR)
            throw shader_program_error(errorCode);
    }

    void opengl_renderer::shader_program::set_uniform_variable(const std::string& aName, float aValue1, float aValue2)
    {
        glUniform2f(uniform_location(aName), aValue1, aValue2);
        GLenum errorCode = glGetError();
        if (errorCode != GL_NO_ERROR)
            throw shader_program_error(errorCode);
    }

    void opengl_renderer::shader_program::set_uniform_variable(const std::string& aName, const vec4f& aVector)
    {
        glUniform4f(uniform_location(aName), aVector[0], aVector[1], aVector[2], aVector[3]);
        GLenum errorCode = glGetError();
        if (errorCode != GL_NO_ERROR)
            throw shader_program_error(errorCode);
    }

    void opengl_renderer::shader_program::set_uniform_variable(const std::string& aName, const vec4& aVector)
    {
        glUniform4d(uniform_location(aName), aVector[0], aVector[1], aVector[2], aVector[3]);
        GLenum errorCode = glGetError();
        if (errorCode != GL_NO_ERROR)
            throw shader_program_error(errorCode);
    }

    void opengl_renderer::shader_program::set_uniform_array(const std::string& aName, uint32_t aSize, const float* aArray)
    {
        glUniform1fv(uniform_location(aName), aSize, aArray);
        GLenum errorCode = glGetError();
        if (errorCode != GL_NO_ERROR)
            throw shader_program_error(errorCode);
    }

    void opengl_renderer::shader_program::set_uniform_array(const std::string& aName, uint32_t aSize, const double* aArray)
    {
        glUniform1dv(uniform_location(aName), aSize, aArray);
        GLenum errorCode = glGetError();
        if (errorCode != GL_NO_ERROR)
            throw shader_program_error(errorCode);
    }

    void opengl_renderer::shader_program::set_uniform_matrix(const std::string& aName, const mat44::template rebind<float>::type& aMatrix)
    {
        glUniformMatrix4fv(uniform_location(aName), 1, false, aMatrix.data());
        GLenum errorCode = glGetError();
        if (errorCode != GL_NO_ERROR)
            throw shader_program_error(errorCode);
    }

    void opengl_renderer::shader_program::set_uniform_matrix(const std::string& aName, const mat44::template rebind<double>::type& aMatrix)
    {
        glUniformMatrix4dv(uniform_location(aName), 1, false, aMatrix.data());
        GLenum errorCode = glGetError();
        if (errorCode != GL_NO_ERROR)
            throw shader_program_error(errorCode);
    }

    GLuint opengl_renderer::shader_program::register_variable(const std::string& aVariableName)
    {
        GLuint index = static_cast<GLuint>(iVariables.size());
        iVariables.insert(std::make_pair(aVariableName, index));
        return index;
    }

    bool opengl_renderer::shader_program::operator<(const shader_program& aRhs) const
    {
        return iHandle < aRhs.iHandle;
    }

    GLint opengl_renderer::shader_program::uniform_location(const std::string& aName)
    {
        GLint var = glGetUniformLocation(iHandle, aName.c_str());
        GLenum errorCode = glGetError();
        if (errorCode != GL_NO_ERROR)
            throw shader_program_error(errorCode);
        return var;
    }

    opengl_renderer::opengl_renderer(neogfx::renderer aRenderer) :
        iRenderer{ aRenderer },
        iActiveProgram {iShaderPrograms.end( )},
        iLimitFrameRate{ true },
        iFrameRateLimit{ 60u },
        iSubpixelRendering{ true },
        iLastGameRenderTime{ 0ull }
    {
#ifdef _WIN32
        SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);
#endif
    }

    opengl_renderer::~opengl_renderer()
    {
    }

    const i_device_metrics& opengl_renderer::default_screen_metrics() const
    {
        return service<i_basic_services>().display().metrics();
    }

    renderer opengl_renderer::renderer() const
    {
        return iRenderer;
    }

    void opengl_renderer::initialize()
    {
        std::cout << "OpenGL vendor: " << reinterpret_cast<const char*>(glGetString(GL_VENDOR)) << std::endl;
        std::cout << "OpenGL renderer: " << reinterpret_cast<const char*>(glGetString(GL_RENDERER)) << std::endl;
        std::cout << "OpenGL version: " << reinterpret_cast<const char*>(glGetString(GL_VERSION)) << std::endl;
        std::cout << "OpenGL shading language version: " << reinterpret_cast<const char*>(glGetString(GL_SHADING_LANGUAGE_VERSION)) << std::endl;
            
        iDefaultProgram = create_shader_program(
            shaders
        {
            std::make_pair(
                std::string(
                    "#version 400\n"
                    "precision mediump float;\n"
                    "uniform mat4 uProjectionMatrix;\n"
                    "uniform mat4 uTransformationMatrix;\n"
                    "in mediump vec3 VertexPosition;\n"
                    "in mediump vec4 VertexColor;\n"
                    "in mediump vec2 VertexTextureCoord;\n"
                    "out vec4 Color;\n"
                    "void main()\n"
                    "{\n"
                    "    Color = VertexColor;\n"
                    "   gl_Position = uProjectionMatrix * (uTransformationMatrix * vec4(VertexPosition, 1.0));\n"
                    "}\n"),
                GL_VERTEX_SHADER),
            std::make_pair(
                std::string(
                    "#version 400\n"
                    "precision mediump float;\n"
                    "in vec4 Color;\n"
                    "out vec4 FragColor;\n"
                    "void main()\n"
                    "{\n"
                    "    FragColor = Color;\n"
                    "}\n"),
                GL_FRAGMENT_SHADER) 
            }, { "VertexPosition", "VertexColor", "VertexTextureCoord" });

        iMeshProgram = create_shader_program(
            shaders
        {
            std::make_pair(
                std::string(
                    "#version 400\n"
                    "precision mediump float;\n"
                    "uniform mat4 uProjectionMatrix;\n"
                    "uniform mat4 uTransformationMatrix;\n"
                    "in mediump vec3 VertexPosition;\n"
                    "in mediump vec4 VertexColor;\n"
                    "in mediump vec2 VertexTextureCoord;\n"
                    "out vec4 Color;\n"
                    "out vec2 vTexCoord;\n"
                    "void main()\n"
                    "{\n"
                    "    Color = VertexColor;\n"
                    "   gl_Position = uProjectionMatrix * (uTransformationMatrix * vec4(VertexPosition, 1.0));\n"
                    "    vTexCoord = VertexTextureCoord;\n"
                    "}\n"),
                GL_VERTEX_SHADER),
            std::make_pair(
                std::string(
                    "#version 400\n"
                    "precision mediump float;\n"
                    "uniform bool haveTexture;\n"
                    "uniform bool multisample;\n"
                    "uniform bool blitBlend;\n"
                    "uniform int texDataFormat;\n"
                    "uniform sampler2D tex;\n"
                    "uniform sampler2DMS texMS;\n"
                    "uniform int texSamples;\n"
                    "uniform vec2 texExtents;\n"
                    "uniform int effect;\n"
                    "in vec4 Color;\n"
                    "out vec4 FragColor;\n"
                    "in vec2 vTexCoord;\n"
                    "void main()\n"
                    "{\n"
                    "    if (!haveTexture)\n"
                    "    {\n"
                    "        FragColor = Color;\n"
                    "        return;\n"
                    "    }\n"
                    "    vec4 texel = vec4(0.0);\n"
                    "    if (!multisample)\n"
                    "    {\n"
                    "        texel = texture(tex, vTexCoord).rgba;\n"
                    "    }\n"
                    "    else\n"
                    "    {\n"
                    "        ivec2 texCoord = ivec2(vTexCoord * texExtents);\n"
                    "        texel = texelFetch(texMS, texCoord, gl_SampleID).rgba;\n"
                    "    }\n"
                    "    switch(texDataFormat)\n"
                    "    {\n"
                    "    case 1:\n" // RGBA
                    "    default:\n"
                    "        break;\n"
                    "    case 2:\n" // Red
                    "        texel = vec4(1.0, 1.0, 1.0, texel.r);\n"
                    "        break;\n"
                    "    case 3:\n" // SubPixel
                    "        texel = vec4(1.0, 1.0, 1.0, (texel.r + texel.g + texel.b) / 3.0);\n"
                    "        break;\n"
                    "    }\n"
                    "    switch(effect)\n"
                    "    {\n"
                    "    case 0:\n" // effect: None
                    "        FragColor = texel.rgba * Color;\n"
                    "        break;\n"
                    "    case 1:\n" // effect: Colourize, ColourizeAverage
                    "        {\n"
                    "            float avg = (texel.r + texel.g + texel.b) / 3.0;\n"
                    "            FragColor = vec4(avg, avg, avg, texel.a) * Color;\n"
                    "        }\n"
                    "        break;\n"
                    "    case 2:\n" // effect: ColourizeMaximum
                    "        {\n"
                    "            float maxChannel = max(texel.r, max(texel.g, texel.b));\n"
                    "            FragColor = vec4(maxChannel, maxChannel, maxChannel, texel.a) * Color;\n"
                    "        }\n"
                    "        break;\n"
                    "    case 3:\n" // effect: ColourizeSpot
                    "        FragColor = vec4(1.0, 1.0, 1.0, texel.a) * Color;\n"
                    "        break;\n"
                    "    case 4:\n" // effect: Monochrome
                    "        {\n"
                    "            float gray = dot(Color.rgb * texel.rgb, vec3(0.299, 0.587, 0.114));\n"
                    "            FragColor = vec4(gray, gray, gray, texel.a) * Color;\n"
                    "        }\n"
                    "        break;\n"
                    "    }\n"
                    "}\n"),
                GL_FRAGMENT_SHADER) 
            }, { "VertexPosition", "VertexColor", "VertexTextureCoord" });

        iGradientProgram = create_shader_program(
            shaders
            {
                std::make_pair(
                    std::string(
                        "#version 400\n"
                        "precision mediump float;\n"
                        "uniform mat4 uProjectionMatrix;\n"
                        "uniform mat4 uTransformationMatrix;\n"
                        "in mediump vec3 VertexPosition;\n"
                        "in mediump vec4 VertexColor;\n"
                        "in mediump vec2 VertexTextureCoord;\n"
                        "out vec4 Color;\n"
                        "void main()\n"
                        "{\n"
                        "    Color = VertexColor;\n"
                        "   gl_Position = uProjectionMatrix * (uTransformationMatrix * vec4(VertexPosition, 1.0));\n"
                        "}\n"),
                    GL_VERTEX_SHADER),
                std::make_pair(
                    glsl::NEOGFX_GRADIENT_FRAG,
                    GL_FRAGMENT_SHADER)
            }, { "VertexPosition", "VertexColor", "VertexTextureCoord" });

        iGlyphProgram = create_shader_program(
            shaders
            {
                std::make_pair(
                    std::string(
                        "#version 400\n"
                        "precision mediump float;\n"
                        "uniform mat4 uProjectionMatrix;\n"
                        "uniform mat4 uTransformationMatrix;\n"
                        "in mediump vec3 VertexPosition;\n"
                        "in mediump vec4 VertexColor;\n"
                        "in mediump vec2 VertexTextureCoord;\n"
                        "out vec4 Color;\n"
                        "out vec2 vGlyphTexCoord;\n"
                        "out vec2 vOutputCoord;\n"
                        "void main()\n"
                        "{\n"
                        "    Color = VertexColor;\n"
                        "   gl_Position = uProjectionMatrix * (uTransformationMatrix * vec4(VertexPosition, 1.0));\n"
                        "    vGlyphTexCoord = VertexTextureCoord;\n"
                        "   vOutputCoord = VertexPosition.xy;\n"
                        "}\n"),
                    GL_VERTEX_SHADER),
                std::make_pair(
                    std::string(
                        "#version 400\n"
                        "precision mediump float;\n"
                        "uniform sampler2D glyphTexture;\n"
                        "uniform vec2 glyphOrigin;\n"
                        "uniform vec2 outputExtents;\n"
                        "uniform bool guiCoordinates;\n"
                        "uniform int subpixel;\n"
                        "uniform vec4 glyphRect;\n"
                        "in vec4 Color;\n"
                        "out vec4 FragColor;\n"
                        "in vec2 vGlyphTexCoord;\n"
                        "in vec2 vOutputCoord;\n"
                        "\n"
                        "int adjust_y(float y)\n"
                        "{\n"
                        "    if (guiCoordinates)\n"
                        "        return int(outputExtents.y) - 1 - int(y);\n"
                        "    else\n"
                        "        return int(y);\n"
                        "}\n"
                        "\n"
                        "void main()\n"
                        "{\n"
                        "    float a = 0.0;\n"
                        "    ivec2 dtpos = ivec2(vOutputCoord.x, adjust_y(vOutputCoord.y));\n"
                        "    if (subpixel == 1)\n"
                        "    {\n"
                        "         vec4 aaaAlpha = texture(glyphTexture, vGlyphTexCoord);\n"
                        "        if (aaaAlpha.rgb == vec3(1.0, 1.0, 1.0))\n"
                        "            a = 1.0;\n"
                        "        else if (aaaAlpha.rgb == vec3(0.0, 0.0, 0.0))\n"
                        "            discard;\n"
                        "        else\n"
                        "            a = (aaaAlpha.r + aaaAlpha.g + aaaAlpha.b) / 3.0;\n"
                        "    }\n"
                        "    else\n"
                        "    {\n"
                        "        a = texture(glyphTexture, vGlyphTexCoord).r;\n"
                        "        if (a == 0)\n"
                        "            discard;\n"
                        "    }\n"
                        "    FragColor = vec4(Color.xyz, Color.a * a);\n"
                        "}\n"),
                    GL_FRAGMENT_SHADER)
            },
            { "VertexPosition", "VertexColor", "VertexTextureCoord" });

        switch (service<i_basic_services>().display(0).subpixel_format())
        {
        case subpixel_format::SubpixelFormatRGBHorizontal:
            iGlyphSubpixelProgram = create_shader_program(
                shaders
                {
                    std::make_pair(
                        std::string(
                            "#version 400\n"
                            "precision mediump float;\n"
                            "uniform mat4 uProjectionMatrix;\n"
                            "uniform mat4 uTransformationMatrix;\n"
                            "in mediump vec3 VertexPosition;\n"
                            "in mediump vec4 VertexColor;\n"
                            "in mediump vec2 VertexTextureCoord;\n"
                            "out vec4 Color;\n"
                            "out vec2 vGlyphTexCoord;\n"
                            "out vec2 vOutputCoord;\n"
                            "void main()\n"
                            "{\n"
                            "    Color = VertexColor;\n"
                            "   gl_Position = uProjectionMatrix * (uTransformationMatrix * vec4(VertexPosition, 1.0));\n"
                            "    vGlyphTexCoord = VertexTextureCoord;\n"
                            "   vOutputCoord = VertexPosition.xy;\n"
                            "}\n"),
                        GL_VERTEX_SHADER),
                    std::make_pair(
                        std::string(
                            "#version 400\n"
                            "precision mediump float;\n"
                            "uniform sampler2D glyphTexture;\n"
                            "uniform vec2 glyphOrigin;\n"
                            "uniform sampler2DMS outputTexture;\n"
                            "uniform vec2 outputExtents;\n"
                            "uniform bool guiCoordinates;\n"
                            "uniform int subpixel;\n"
                            "uniform vec4 glyphRect;\n"
                            "in vec4 Color;\n"
                            "out vec4 FragColor;\n"
                            "in vec2 vGlyphTexCoord;\n"
                            "in vec2 vOutputCoord;\n"
                            "\n"
                            "int adjust_y(float y)\n"
                            "{\n"
                            "    if (guiCoordinates)\n"
                            "        return int(outputExtents.y) - 1 - int(y);\n"
                            "    else\n"
                            "        return int(y);\n"
                            "}\n"
                            "void main()\n"
                            "{\n"
                            "    ivec2 dtpos = ivec2(vOutputCoord);\n"
                            "    if (guiCoordinates)\n"
                            "        dtpos.y = int(outputExtents.y) - 1 - dtpos.y;\n"
                            "    vec4 rgbAlpha = texture(glyphTexture, vGlyphTexCoord);\n"
                            "    if (rgbAlpha.rgb == vec3(1.0, 1.0, 1.0))\n"
                            "        FragColor = Color;\n"
                            "    else if (rgbAlpha.rgb == vec3(0.0, 0.0, 0.0))\n"
                            "        discard;\n"
                            "    else\n"
                            "    {\n"
                            "        vec4 rgbDestination = texelFetch(outputTexture, dtpos, 0);\n"
                            "        FragColor = vec4(Color.rgb * rgbAlpha.rgb * Color.a + rgbDestination.rgb * (vec3(1.0, 1.0, 1.0) - rgbAlpha.rgb * Color.a), 1.0);\n"
                            "    }\n"
                            "}\n"),
                        GL_FRAGMENT_SHADER)
                    },
                    { "VertexPosition", "VertexColor", "VertexTextureCoord" });
            break;
        case subpixel_format::SubpixelFormatBGRHorizontal:
            iGlyphSubpixelProgram = create_shader_program(
                shaders
                {
                    std::make_pair(
                        std::string(
                            "#version 400\n"
                            "precision mediump float;\n"
                            "uniform mat4 uProjectionMatrix;\n"
                            "uniform mat4 uTransformationMatrix;\n"
                            "in mediump vec3 VertexPosition;\n"
                            "in mediump vec4 VertexColor;\n"
                            "in mediump vec2 VertexTextureCoord;\n"
                            "out vec4 Color;\n"
                            "out vec2 vGlyphTexCoord;\n"
                            "out vec2 vOutputCoord;\n"
                            "void main()\n"
                            "{\n"
                            "    Color = VertexColor;\n"
                            "   gl_Position = uProjectionMatrix * (uTransformationMatrix * vec4(VertexPosition, 1.0));\n"
                            "    vGlyphTexCoord = VertexTextureCoord;\n"
                            "   vOutputCoord = VertexPosition.xy;\n"
                            "}\n"),
                        GL_VERTEX_SHADER),
                    std::make_pair(
                        std::string(
                            "#version 400\n"
                            "precision mediump float;\n"
                            "uniform sampler2D glyphTexture;\n"
                            "uniform vec2 glyphOrigin;\n"
                            "uniform sampler2DMS outputTexture;\n"
                            "uniform vec2 outputExtents;\n"
                            "uniform bool guiCoordinates;\n"
                            "uniform int subpixel;\n"
                            "uniform vec4 glyphRect;\n"
                            "in vec4 Color;\n"
                            "out vec4 FragColor;\n"
                            "in vec2 vGlyphTexCoord;\n"
                            "in vec2 vOutputCoord;\n"
                            "\n"
                            "int adjust_y(float y)\n"
                            "{\n"
                            "    if (guiCoordinates)\n"
                            "        return int(outputExtents.y) - 1 - int(y);\n"
                            "    else\n"
                            "        return int(y);\n"
                            "}\n"
                            "void main()\n"
                            "{\n"
                            "    ivec2 dtpos = ivec2(vOutputCoord);\n"
                            "    if (guiCoordinates)\n"
                            "        dtpos.y = int(outputExtents.y) - 1 - dtpos.y;\n"
                            "    vec4 rgbAlpha = texture(glyphTexture, vGlyphTexCoord);\n"
                            "    if (rgbAlpha.rgb == vec3(1.0, 1.0, 1.0))\n"
                            "        FragColor = Color;\n"
                            "    else if (rgbAlpha.rgb == vec3(0.0, 0.0, 0.0))\n"
                            "        discard;\n"
                            "    else\n"
                            "    {\n"
                            "        vec4 rgbDestination = texelFetch(outputTexture, dtpos, 0);\n"
                            "        FragColor = vec4(Color.rgb * rgbAlpha.bgr * Color.a + rgbDestination.rgb * (vec3(1.0, 1.0, 1.0) - rgbAlpha.bgr * Color.a), 1.0);\n"
                            "    }\n"
                            "}\n"),
                        GL_FRAGMENT_SHADER)
                    },
                    { "VertexPosition", "VertexColor", "VertexTextureCoord" });
            break;
        case subpixel_format::SubpixelFormatRGBVertical:/* todo */
        case subpixel_format::SubpixelFormatBGRVertical:/* todo */
        default:
            iGlyphSubpixelProgram = create_shader_program(
                shaders
                {
                    std::make_pair(
                        std::string(
                            "#version 400\n"
                            "precision mediump float;\n"
                            "uniform mat4 uProjectionMatrix;\n"
                            "uniform mat4 uTransformationMatrix;\n"
                            "in mediump vec3 VertexPosition;\n"
                            "in mediump vec4 VertexColor;\n"
                            "in mediump vec2 VertexTextureCoord;\n"
                            "out vec4 Color;\n"
                            "out vec2 vGlyphTexCoord;\n"
                            "void main()\n"
                            "{\n"
                            "    Color = VertexColor;\n"
                            "   gl_Position = uProjectionMatrix * (uTransformationMatrix * vec4(VertexPosition, 1.0));\n"
                            "    vGlyphTexCoord = VertexTextureCoord;\n"
                            "}\n"),
                        GL_VERTEX_SHADER),
                    std::make_pair(
                        std::string(
                            "#version 400\n"
                            "precision mediump float;\n"
                            "uniform sampler2D glyphTexture;\n"
                            "uniform vec2 glyphOrigin;\n"
                            "uniform sampler2D glyphDestinationTexture;\n"
                            "uniform vec2 glyphTextureOffset;\n"
                            "uniform vec2 glyphTextureExtents;\n"
                            "uniform vec2 glyphDestinationTextureExtents;\n"
                            "in vec4 Color;\n"
                            "out vec4 FragColor;\n"
                            "in vec2 vGlyphTexCoord;\n"
                            "void main()\n"
                            "{\n"
                            "    vec4 rgbAlpha = texture(glyphTexture, vGlyphTexCoord);\n"
                            "    FragColor = vec4(Color.rgb, (rgbAlpha.r + rgbAlpha.g + rgbAlpha.b) / 3.0);\n"
                            "}\n"),
                        GL_FRAGMENT_SHADER)
                    },
                    { "VertexPosition", "VertexColor", "VertexTextureCoord" });
            break;
        }
    }

    void opengl_renderer::cleanup()
    {
        // We explictly destroy these OpenGL objects here when context should still exist
        iVertexArrays = std::nullopt;
        iGradientDataCacheQueue.clear();
        iGradientDataCacheMap.clear();
        iGradientDataCache.clear();
        iUncachedGradient = std::nullopt;
        iFontManager = std::nullopt;
        iTextureManager = std::nullopt;
        opengl_buffer_cleanup();
    }
       
    i_font_manager& opengl_renderer::font_manager()
    {
        if (iFontManager == std::nullopt)
            iFontManager.emplace();
        return *iFontManager;
    }

    i_texture_manager& opengl_renderer::texture_manager()
    {
        if (iTextureManager == std::nullopt)
            iTextureManager.emplace();
        return *iTextureManager;
    }

    bool opengl_renderer::shader_program_active() const
    {
        return iActiveProgram != iShaderPrograms.end();
    }

    void opengl_renderer::activate_shader_program(i_rendering_context& aGraphicsContext, i_shader_program& aProgram, const optional_mat44& aProjectionMatrix, const optional_mat44& aTransformationMatrix)
    {
        for (auto i = iShaderPrograms.begin(); i != iShaderPrograms.end(); ++i)
            if (&*i == &aProgram)
            {
                if (iActiveProgram != i)
                {
                    iActiveProgram = i;
                    glCheck(glUseProgram(static_cast<GLuint>(reinterpret_cast<std::intptr_t>(iActiveProgram->handle()))));
                }
                if (iActiveProgram->has_projection_matrix())
                    iActiveProgram->set_projection_matrix(aGraphicsContext, aProjectionMatrix);
                if (iActiveProgram->has_transformation_matrix())
                    iActiveProgram->set_transformation_matrix(aGraphicsContext, aTransformationMatrix);
                return;
            }
        throw shader_program_not_found();
    }

    void opengl_renderer::deactivate_shader_program()
    {
        if (iActiveProgram == iShaderPrograms.end())
            throw no_shader_program_active();
        iActiveProgram = iShaderPrograms.end();
        glCheck(glUseProgram(0));
    }

    const opengl_renderer::i_shader_program& opengl_renderer::active_shader_program() const
    {
        if (iActiveProgram == iShaderPrograms.end())
            throw no_shader_program_active();
        return *iActiveProgram;
    }

    opengl_renderer::i_shader_program& opengl_renderer::active_shader_program()
    {
        if (iActiveProgram == iShaderPrograms.end())
            throw no_shader_program_active();
        return *iActiveProgram;
    }

    const opengl_renderer::i_shader_program& opengl_renderer::default_shader_program() const
    {
        return *iDefaultProgram;
    }

    opengl_renderer::i_shader_program& opengl_renderer::default_shader_program()
    {
        return *iDefaultProgram;
    }

    const opengl_renderer::i_shader_program& opengl_renderer::mesh_shader_program() const
    {
        return *iMeshProgram;
    }

    opengl_renderer::i_shader_program& opengl_renderer::mesh_shader_program()
    {
        return *iMeshProgram;
    }

    const opengl_renderer::i_shader_program& opengl_renderer::gradient_shader_program() const
    {
        return *iGradientProgram;
    }

    opengl_renderer::i_shader_program& opengl_renderer::gradient_shader_program()
    {
        return *iGradientProgram;
    }

    const opengl_renderer::i_shader_program& opengl_renderer::glyph_shader_program(bool aSubpixel) const
    {
        return aSubpixel ? *iGlyphSubpixelProgram : *iGlyphProgram;
    }

    opengl_renderer::i_shader_program& opengl_renderer::glyph_shader_program(bool aSubpixel)
    {
        return aSubpixel ? *iGlyphSubpixelProgram : *iGlyphProgram;
    }

    const opengl_standard_vertex_arrays& opengl_renderer::vertex_arrays() const
    {
        if (iVertexArrays == std::nullopt)
            iVertexArrays.emplace();
        return *iVertexArrays;
    }

    opengl_standard_vertex_arrays& opengl_renderer::vertex_arrays()
    {
        return const_cast<opengl_standard_vertex_arrays&>(const_cast<const opengl_renderer*>(this)->vertex_arrays());
    }

    i_texture& opengl_renderer::ping_pong_buffer1(const size& aExtents, texture_sampling aSampling)
    {
        auto& bufferTexture = create_ping_pong_buffer(iPingPongBuffer1s, aExtents, aSampling);
        return bufferTexture;
    }

    i_texture& opengl_renderer::ping_pong_buffer2(const size& aExtents, texture_sampling aSampling)
    {
        auto& bufferTexture = create_ping_pong_buffer(iPingPongBuffer2s, aExtents, aSampling);
        return bufferTexture;
    }

    bool opengl_renderer::is_subpixel_rendering_on() const
    {
        return iSubpixelRendering;
    }
    
    void opengl_renderer::subpixel_rendering_on()
    {
        if (!iSubpixelRendering)
        {
            iSubpixelRendering = true;
            evSubpixelRenderingChanged.trigger();
        }
    }

    void opengl_renderer::subpixel_rendering_off()
    {
        if (iSubpixelRendering)
        {
            iSubpixelRendering = false;
            evSubpixelRenderingChanged.trigger();
        }
    }

    bool opengl_renderer::frame_rate_limited() const
    {
        return iLimitFrameRate; 
    }

    void opengl_renderer::enable_frame_rate_limiter(bool aEnable)
    {
        iLimitFrameRate = aEnable;
    }

    uint32_t opengl_renderer::frame_rate_limit() const
    {
        return iFrameRateLimit;
    }

    void opengl_renderer::set_frame_rate_limit(uint32_t aFps)
    {
        iFrameRateLimit = aFps;
    }

    gradient_shader_data& opengl_renderer::gradient_shader_data(const gradient& aGradient)
    {
        auto instantiate_gradient = [this, &aGradient](neogfx::gradient_shader_data& aData)
        {
            auto combinedStops = aGradient.combined_stops();
            iGradientStopPositions.reserve(combinedStops.size());
            iGradientStopColours.reserve(combinedStops.size());
            iGradientStopPositions.clear();
            iGradientStopColours.clear();
            for (const auto& stop : combinedStops)
            {
                iGradientStopPositions.push_back(static_cast<float>(stop.first));
                iGradientStopColours.push_back(std::array<float, 4>{ {stop.second.red<float>(), stop.second.green<float>(), stop.second.blue<float>(), stop.second.alpha<float>()}});
            }
            aData.stopCount = static_cast<uint32_t>(combinedStops.size());
            aData.stops.data().set_pixels(rect{ point{}, size_u32{ static_cast<uint32_t>(iGradientStopPositions.size()), 1u } }, &iGradientStopPositions[0]);
            aData.stopColours.data().set_pixels(rect{ point{}, size_u32{ static_cast<uint32_t>(iGradientStopColours.size()), 1u } }, &iGradientStopColours[0]);
            auto filter = static_gaussian_filter<float, GRADIENT_FILTER_SIZE>(static_cast<float>(aGradient.smoothness() * 10.0));
            aData.filter.data().set_pixels(rect{ point(), size_u32{ GRADIENT_FILTER_SIZE, GRADIENT_FILTER_SIZE } }, &filter[0][0]);
        };
        if (aGradient.use_cache())
        {
            auto mapResult = iGradientDataCacheMap.try_emplace(aGradient, iGradientDataCache.end());
            auto mapEntry = mapResult.first;
            bool newGradient = mapResult.second;
            if (!newGradient)
            {
                auto queueEntry = std::find(iGradientDataCacheQueue.begin(), iGradientDataCacheQueue.end(), mapEntry);
                if (queueEntry != std::prev(iGradientDataCacheQueue.end()))
                {
                    iGradientDataCacheQueue.erase(queueEntry);
                    iGradientDataCacheQueue.push_back(mapEntry);
                }
            }
            else
            {
                if (iGradientDataCache.size() < GRADIENT_DATA_CACHE_QUEUE_SIZE)
                {
                    iGradientDataCache.emplace_back();
                    mapEntry->second = std::prev(iGradientDataCache.end());
                }
                else
                {
                    auto data = iGradientDataCacheQueue.front()->second;
                    iGradientDataCacheMap.erase(iGradientDataCacheQueue.front());
                    iGradientDataCacheQueue.pop_front();
                    mapEntry->second = data;
                }
                iGradientDataCacheQueue.push_back(mapEntry);
            }
            if (newGradient)
                instantiate_gradient(*mapEntry->second);
            return *mapEntry->second;
        }
        else
        {
            if (iUncachedGradient == std::nullopt)
                iUncachedGradient.emplace();
            instantiate_gradient(*iUncachedGradient);
            return *iUncachedGradient;
        }
    }

    bool opengl_renderer::process_events()
    {
        bool didSome = false;
        auto lastRenderTime = neolib::thread::program_elapsed_ms();
        bool finished = false;
        while (!finished)
        {    
            finished = true;
            for (std::size_t s = 0; s < service<i_surface_manager>().surface_count(); ++s)
            {
                auto& surface = service<i_surface_manager>().surface(s);
                if (surface.has_native_surface() && surface.native_surface().pump_event())
                {
                    didSome = true;
                    finished = false;
                }
            }
            if (neolib::thread::program_elapsed_ms() - lastRenderTime > 10)
            {
                lastRenderTime = neolib::thread::program_elapsed_ms();
                render_now();
            }
        }
        return didSome;
    }

    void opengl_renderer::want_game_mode()
    {
        iLastGameRenderTime = neolib::thread::program_elapsed_ms();
    }

    bool opengl_renderer::game_mode() const
    {
        return neolib::thread::program_elapsed_ms() - iLastGameRenderTime < 5000u;
    }

    void opengl_renderer::register_frame_counter(i_widget& aWidget, uint32_t aDuration)
    {
        auto iterFrameCounter = iFrameCounters.find(aDuration);
        if (iterFrameCounter == iFrameCounters.end())
            iterFrameCounter = iFrameCounters.emplace(aDuration, aDuration).first;
        iterFrameCounter->second.add(aWidget);
    }

    void opengl_renderer::unregister_frame_counter(i_widget& aWidget, uint32_t aDuration)
    {
        auto iterFrameCounter = iFrameCounters.find(aDuration);
        if (iterFrameCounter != iFrameCounters.end())
            iterFrameCounter->second.remove(aWidget);
    }

    uint32_t opengl_renderer::frame_counter(uint32_t aDuration) const
    {
        auto iterFrameCounter = iFrameCounters.find(aDuration);
        if (iterFrameCounter != iFrameCounters.end())
            return iterFrameCounter->second.counter();
        return 0;
    }    
    
    opengl_renderer::shader_programs::iterator opengl_renderer::create_shader_program(const shaders& aShaders, const std::vector<std::string>& aVariables)
    {
        GLuint programHandle;
        glCheck(programHandle = glCreateProgram());
        if (0 == programHandle)
            throw failed_to_create_shader_program("Failed to create shader program object");
        bool hasProjectionMatrix = false;
        bool hasTransformationMatrix = false;
        for (auto& s : aShaders)
        {
            GLuint shader;
            glCheck(shader = glCreateShader(s.second));
            if (0 == shader)
                throw failed_to_create_shader_program("Failed to create shader object");
            std::string source = s.first;
            if (source.find("uProjectionMatrix") != std::string::npos)
                hasProjectionMatrix = true;
            if (source.find("uTransformationMatrix") != std::string::npos)
                hasTransformationMatrix = true;
            if (renderer() == neogfx::renderer::DirectX)
            {
                std::size_t v;
                const std::size_t VERSION_STRING_LENGTH = 12;
                if ((v = source.find("#version 400")) != std::string::npos)
                    source.replace(v, VERSION_STRING_LENGTH, "#version 110");
                else if ((v = source.find("#version 400")) != std::string::npos)
                    source.replace(v, VERSION_STRING_LENGTH, "#version 110");
            }
            const char* codeArray[] = { source.c_str() };
            glCheck(glShaderSource(shader, 1, codeArray, NULL));
            glCheck(glCompileShader(shader));
            GLint result;
            glCheck(glGetShaderiv(shader, GL_COMPILE_STATUS, &result));
            if (GL_FALSE == result)
            {
                GLint buflen;
                glCheck(glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &buflen));
                std::vector<GLchar> buf(buflen);
                glCheck(glGetShaderInfoLog(shader, static_cast<GLsizei>(buf.size()), NULL, &buf[0]));
                std::string error(&buf[0]);
                throw failed_to_create_shader_program(error);
            }
            glCheck(glAttachShader(programHandle, shader));
        }
        shader_program program(programHandle, hasProjectionMatrix, hasTransformationMatrix);
        for (auto& v : aVariables)
            glCheck(glBindAttribLocation(programHandle, program.register_variable(v), v.c_str()));
        auto s = iShaderPrograms.insert(iShaderPrograms.end(), program);
        glCheck(glLinkProgram(programHandle));
        GLint result;
        glCheck(glGetProgramiv(programHandle, GL_LINK_STATUS, &result));
        if (GL_FALSE == result)
            throw failed_to_create_shader_program("Failed to link");
        return s;
    }

    i_texture& opengl_renderer::create_ping_pong_buffer(ping_pong_buffers_t& aBufferList, const size& aExtents, texture_sampling aSampling)
    {
        auto existing = aBufferList.lower_bound(std::make_pair(aSampling, aExtents));
        if (existing != aBufferList.end() && existing->first.first == aSampling && existing->first.second >= aExtents)
            return existing->second;
        auto const sizeMultiple = 1024;
        basic_size<int32_t> idealSize{ (((static_cast<int32_t>(aExtents.cx) - 1) / sizeMultiple) + 1) * sizeMultiple, (((static_cast<int32_t>(aExtents.cy) - 1) / sizeMultiple) + 1) * sizeMultiple };
        return aBufferList.emplace(std::make_pair(aSampling, idealSize), texture{ idealSize, 1.0, aSampling }).first->second;
    }
}