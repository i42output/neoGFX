// opengl_renderer.cpp
/*
  neogfx C++ GUI Library
  Copyright (c) 2015-present, Leigh Johnston.  All Rights Reserved.
  
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

#include <neogfx/core/numerical.hpp>
#include "opengl_renderer.hpp"
#include "../../gui/window/native/opengl_window.hpp"

#include "gradient.frag.hpp"

#include <neogfx/app/app.hpp>

namespace neogfx
{
	frame_counter::frame_counter(uint32_t aDuration) : iTimer{ app::instance(), [this](neolib::callback_timer& aTimer)
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

	opengl_renderer::shader_program::shader_program(GLuint aHandle, bool aHasProjectionMatrix) :
		iHandle(aHandle), iHasProjectionMatrix(aHasProjectionMatrix)
	{
	}

	void* opengl_renderer::shader_program::handle() const
	{
		return reinterpret_cast<void*>(iHandle);
	}

	bool opengl_renderer::shader_program::has_projection_matrix() const
	{
		return iHasProjectionMatrix;
	}

	void opengl_renderer::shader_program::set_projection_matrix(const i_native_graphics_context& aGraphicsContext)
	{
		auto const& logicalCoordinates = aGraphicsContext.logical_coordinates();
		if (iLogicalCoordinates != logicalCoordinates)
		{
			iLogicalCoordinates = logicalCoordinates;
			double left = logicalCoordinates.first.x;
			double right = logicalCoordinates.second.x;
			double bottom = logicalCoordinates.first.y;
			double top = logicalCoordinates.second.y;
			double zFar = 1.0;
			double zNear = -1.0;
			mat44 orthoMatrix = mat44{
				{ 2.0 / (right - left), 0.0, 0.0, -(right + left) / (right - left) },
				{ 0.0, 2.0 / (top - bottom), 0.0, -(top + bottom) / (top - bottom) },
				{ 0.0, 0.0, -2.0 / (zFar - zNear), -(zFar + zNear) / (zFar - zNear) },
				{ 0.0, 0.0, 0.0, 1.0 } }.transposed();
			set_uniform_matrix("uProjectionMatrix", basic_matrix<float, 4, 4>{ orthoMatrix });
		}
	}


	void* opengl_renderer::shader_program::variable(const std::string& aVariableName) const
	{
		auto v = iVariables.find(aVariableName);
		if (v == iVariables.end())
			throw variable_not_found();
		return reinterpret_cast<void*>(v->second);
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
		iRenderer{aRenderer},
		iFontManager{*this},
		iActiveProgram{iShaderPrograms.end()},
		iSubpixelRendering{true}
	{
#ifdef _WIN32
		SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);
#endif
	}

	opengl_renderer::~opengl_renderer()
	{
		if (iGradientTextures != boost::none)
		{
			glCheck(glDeleteTextures(1, &(*iGradientTextures)[0]));
			glCheck(glDeleteTextures(1, &(*iGradientTextures)[1]));
			glCheck(glDeleteTextures(1, &(*iGradientTextures)[2]));
		}
	}

	const i_device_metrics& opengl_renderer::default_screen_metrics() const
	{
		return app::instance().basic_services().display().metrics();
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
					"#version 130\n"
					"precision mediump float;\n"
					"uniform mat4 uProjectionMatrix;\n"
					"in vec3 VertexPosition;\n"
					"in vec4 VertexColor;\n"
					"in vec2 VertexTextureCoord;\n"
					"out vec4 Color;\n"
					"void main()\n"
					"{\n"
					"	Color = VertexColor / 255.0;\n"
					"   gl_Position = uProjectionMatrix * vec4(VertexPosition, 1.0);\n"
					"}\n"),
				GL_VERTEX_SHADER),
			std::make_pair(
				std::string(
					"#version 130\n"
					"precision mediump float;\n"
					"in vec4 Color;\n"
					"out vec4 FragColor;\n"
					"void main()\n"
					"{\n"
					"	FragColor = Color;\n"
					"}\n"),
				GL_FRAGMENT_SHADER) 
			}, { "VertexPosition", "VertexColor", "VertexTextureCoord" });

		iTextureProgram = create_shader_program(
			shaders
		{
			std::make_pair(
				std::string(
					"#version 130\n"
					"precision mediump float;\n"
					"uniform mat4 uProjectionMatrix;\n"
					"in vec3 VertexPosition;\n"
					"in vec4 VertexColor;\n"
					"in vec2 VertexTextureCoord;\n"
					"out vec4 Color;\n"
					"varying vec2 vTexCoord;\n"
					"void main()\n"
					"{\n"
					"	Color = VertexColor / 255.0;\n"
					"   gl_Position = uProjectionMatrix * vec4(VertexPosition, 1.0);\n"
					"	vTexCoord = VertexTextureCoord;\n"
					"}\n"),
				GL_VERTEX_SHADER),
			std::make_pair(
				std::string(
					"#version 130\n"
					"precision mediump float;\n"
					"uniform sampler2D tex;\n"
					"uniform int effect;\n"
					"in vec4 Color;\n"
					"out vec4 FragColor;\n"
					"varying vec2 vTexCoord;\n"
					"void main()\n"
					"{\n"
					"	vec4 texel = texture(tex, vTexCoord).rgba;\n"
					"	switch(effect)\n"
					"	{\n"
					"	case 0:\n" // effect: None
					"		FragColor = texel.rgba * Color;\n"
					"		break;\n"
					"	case 1:\n" // effect: Colourize, ColourizeAverage
					"		{\n"
					"			float avg = (texel.r + texel.g + texel.b) / 3.0;\n"
					"			FragColor = vec4(avg, avg, avg, texel.a) * Color;\n"
					"		}\n"
					"		break;\n"
					"	case 2:\n" // effect: ColourizeMaximum
					"		{\n"
					"			float maxChannel = max(texel.r, max(texel.g, texel.b));\n"
					"			FragColor = vec4(maxChannel, maxChannel, maxChannel, texel.a) * Color;\n"
					"		}\n"
					"		break;\n"
					"	case 3:\n" // effect: ColourizeSpot
					"		FragColor = vec4(1.0, 1.0, 1.0, texel.a) * Color;\n"
					"		break;\n"
					"	case 4:\n" // effect: Monochrome
					"		{\n"
					"			float gray = dot(Color.rgb * texel.rgb, vec3(0.299, 0.587, 0.114));\n"
					"			FragColor = vec4(gray, gray, gray, texel.a) * Color;\n"
					"		}\n"
					"		break;\n"
					"	}\n"
					"}\n"),
				GL_FRAGMENT_SHADER) 
			}, { "VertexPosition", "VertexColor", "VertexTextureCoord" });

		iGradientProgram = create_shader_program(
			shaders
			{
				std::make_pair(
					std::string(
						"#version 130\n"
						"precision mediump float;\n"
						"uniform mat4 uProjectionMatrix;\n"
						"in vec3 VertexPosition;\n"
						"in vec4 VertexColor;\n"
						"in vec2 VertexTextureCoord;\n"
						"out vec4 Color;\n"
						"void main()\n"
						"{\n"
						"	Color = VertexColor / 255.0;\n"
						"   gl_Position = uProjectionMatrix * vec4(VertexPosition, 1.0);\n"
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
						"#version 130\n"
						"precision mediump float;\n"
						"uniform mat4 uProjectionMatrix;\n"
						"in vec3 VertexPosition;\n"
						"in vec4 VertexColor;\n"
						"in vec2 VertexTextureCoord;\n"
						"out vec4 Color;\n"
						"varying vec2 vGlyphTexCoord;\n"
						"varying vec2 vOutputCoord;\n"
						"void main()\n"
						"{\n"
						"	Color = VertexColor / 255.0;\n"
						"   gl_Position = uProjectionMatrix * vec4(VertexPosition, 1.0);\n"
						"	vGlyphTexCoord = VertexTextureCoord;\n"
						"   vOutputCoord = VertexPosition.xy;\n"
						"}\n"),
					GL_VERTEX_SHADER),
				std::make_pair(
					std::string(
						"#version 130\n"
						"precision mediump float;\n"
						"uniform sampler2D glyphTexture;\n"
						"uniform vec2 glyphOrigin;\n"
						"uniform vec2 outputExtents;\n"
						"uniform bool guiCoordinates;\n"
						"uniform int effect;\n"
						"uniform int effectWidth;\n"
						"uniform vec4 effectRect;\n"
						"uniform vec4 glyphRect;\n"
						"in vec4 Color;\n"
						"out vec4 FragColor;\n"
						"varying vec2 vGlyphTexCoord;\n"
						"varying vec2 vOutputCoord;\n"
						"\n"
						"int adjust_y(float y)\n"
						"{\n"
						"	if (guiCoordinates)\n"
						"		return int(outputExtents.y) - 1 - int(y);\n"
						"	else\n"
						"		return int(y);\n"
						"}\n"
						"\n"
						"void main()\n"
						"{\n"
						"	float a = 0.0;\n"
						"	ivec2 dtpos = ivec2(vOutputCoord.x, adjust_y(vOutputCoord.y));\n"
						"   switch(effect)\n"
						"   {\n"
						"   case 0:\n"
						"	case 1:\n"
						"		{\n"
						"			vec4 rgbAlpha = texture(glyphTexture, vGlyphTexCoord);\n"
						"			if (rgbAlpha.a == 0.0)\n"
						"				discard;\n"						
						"			a = rgbAlpha.a;\n"
						"		}\n"
						"		break;\n"
						"   case 2:\n"/* todo */
						"		if (dtpos.x >= effectRect.x && dtpos.x < effectRect.z && dtpos.y >= adjust_y(effectRect.y) && dtpos.y < adjust_y(effectRect.w))\n"
						"		{\n"
						"			for (int y = -effectWidth; y <= effectWidth; ++y)\n"
						"			{\n"
						"				for (int x = -effectWidth; x <= effectWidth; ++x)\n"
						"				{\n"
						"					ivec2 pos = ivec2(dtpos.x + x, dtpos.y + y);\n"
						"					if (pos.x >= glyphRect.x && pos.x < glyphRect.z && pos.y >= adjust_y(glyphRect.y) && pos.y < adjust_y(glyphRect.w))\n"
						"					{\n"
						"						ivec2 tpos = ivec2(x, y) + ivec2(pos.x - glyphRect.x, pos.y - adjust_y(glyphRect.y));\n"
						"						float thisAlpha = texelFetch(glyphTexture, ivec2(glyphOrigin.x + tpos.x, glyphOrigin.y - (glyphRect.w - glyphRect.y) - tpos.y - 1), 0).a;\n"
						"						a = max(a, thisAlpha);\n"
						"					}\n"
						"				}\n"
						"			}\n"
						"		}\n"
						"		break;\n"
						"   }\n"
						"	FragColor = vec4(Color.xyz, Color.a * a);\n"
						"}\n"),
					GL_FRAGMENT_SHADER)
			},
			{ "VertexPosition", "VertexColor", "VertexTextureCoord" });

		switch (app::instance().basic_services().display(0).subpixel_format())
		{
		case subpixel_format::SubpixelFormatRGBHorizontal:
			iGlyphSubpixelProgram = create_shader_program(
				shaders
				{
					std::make_pair(
						std::string(
							"#version 130\n"
							"precision mediump float;\n"
							"uniform mat4 uProjectionMatrix;\n"
							"in vec3 VertexPosition;\n"
							"in vec4 VertexColor;\n"
							"in vec2 VertexTextureCoord;\n"
							"out vec4 Color;\n"
							"varying vec2 vGlyphTexCoord;\n"
							"varying vec2 vOutputCoord;\n"
							"void main()\n"
							"{\n"
							"	Color = VertexColor / 255.0;\n"
							"   gl_Position = uProjectionMatrix * vec4(VertexPosition, 1.0);\n"
							"	vGlyphTexCoord = VertexTextureCoord;\n"
							"   vOutputCoord = VertexPosition.xy;\n"
							"}\n"),
						GL_VERTEX_SHADER),
					std::make_pair(
						std::string(
							"#version 150\n"
							"precision mediump float;\n"
							"uniform sampler2D glyphTexture;\n"
							"uniform vec2 glyphOrigin;\n"
							"uniform sampler2DMS outputTexture;\n"
							"uniform vec2 outputExtents;\n"
							"uniform bool guiCoordinates;\n"
							"uniform int effect;\n"
							"uniform int effectWidth;\n"
							"uniform vec4 effectRect;\n"
							"uniform vec4 glyphRect;\n"
							"in vec4 Color;\n"
							"out vec4 FragColor;\n"
							"varying vec2 vGlyphTexCoord;\n"
							"varying vec2 vOutputCoord;\n"
							"\n"
							"int adjust_y(float y)\n"
							"{\n"
							"	if (guiCoordinates)\n"
							"		return int(outputExtents.y) - 1 - int(y);\n"
							"	else\n"
							"		return int(y);\n"
							"}\n"
							"void main()\n"
							"{\n"
							"	ivec2 dtpos = ivec2(vOutputCoord);\n"
							"	if (guiCoordinates)\n"
							"		dtpos.y = int(outputExtents.y) - 1 - dtpos.y;\n"
							"   switch(effect)\n"
							"   {\n"
							"   case 0:\n"
							"		{\n"
							"		 	vec4 rgbAlpha = texture(glyphTexture, vGlyphTexCoord);\n"
							"			if (rgbAlpha.rgb == vec3(1.0, 1.0, 1.0))\n"
							"				FragColor = Color;\n"
							"			else if (rgbAlpha.rgb == vec3(0.0, 0.0, 0.0))\n"
							"				discard;\n"
							"			else\n"
							"			{\n"
							"				vec4 rgbDestination = texelFetch(outputTexture, dtpos, 0);\n"
							"				FragColor = vec4(Color.rgb * rgbAlpha.rgb * Color.a + rgbDestination.rgb * (vec3(1.0, 1.0, 1.0) - rgbAlpha.rgb * Color.a), 1.0);\n"
							"			}\n"
							"		}\n"
							"		break;\n"
							"	case 1:\n"
							"		{\n"
							"		 	vec4 rgbAlpha = texture(glyphTexture, vGlyphTexCoord);\n"
							"			if (rgbAlpha.rgb != vec3(0.0, 0.0, 0.0))\n"
							"				FragColor = vec4(Color.rgb, ((rgbAlpha.r + rgbAlpha.g + rgbAlpha.b) / 3.0 * Color.a));\n"
							"			else\n"
							"				discard;\n"
							"		}\n"
							"		break;\n"
							"   case 2:\n"/* todo */
							"		if (dtpos.x >= effectRect.x && dtpos.x < effectRect.z && dtpos.y >= adjust_y(effectRect.y) && dtpos.y < adjust_y(effectRect.w))\n"
							"		{\n"
							"			for (int y = int(dtpos.y) - effectWidth; y <= int(dtpos.y) + effectWidth; ++y)\n"
							"				for (int x = int(dtpos.x) - effectWidth; x <= int(dtpos.x) + effectWidth; ++x)\n"
							"					if (x >= glyphRect.x && x < glyphRect.z && y >= adjust_y(glyphRect.y) && y < adjust_y(glyphRect.w))\n"
							"					{\n"
							"						if (texelFetch(glyphTexture, ivec2(glyphOrigin) + ivec2(x, adjust_y(glyphRect.w) - y) - ivec2(glyphRect.x, adjust_y(glyphRect.y)), 0).rgb != vec3(0.0, 0.0, 0.0))\n"
							"						{\n"
							"							FragColor = Color.rgba;\n"
							"							return;\n"
							"						}\n"
							"					}\n"
							"		}\n"
							"		FragColor = vec4(0.0, 0.0, 0.0, 0.0);\n"
							"		break;\n"
							"	}\n"
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
							"#version 130\n"
							"precision mediump float;\n"
							"uniform mat4 uProjectionMatrix;\n"
							"in vec3 VertexPosition;\n"
							"in vec4 VertexColor;\n"
							"in vec2 VertexTextureCoord;\n"
							"out vec4 Color;\n"
							"varying vec2 vGlyphTexCoord;\n"
							"varying vec2 vOutputCoord;\n"
							"void main()\n"
							"{\n"
							"	Color = VertexColor / 255.0;\n"
							"   gl_Position = uProjectionMatrix * vec4(VertexPosition, 1.0);\n"
							"	vGlyphTexCoord = VertexTextureCoord;\n"
							"   vOutputCoord = VertexPosition.xy;\n"
							"}\n"),
						GL_VERTEX_SHADER),
					std::make_pair(
						std::string(
							"#version 150\n"
							"precision mediump float;\n"
							"uniform sampler2D glyphTexture;\n"
							"uniform vec2 glyphOrigin;\n"
							"uniform sampler2DMS outputTexture;\n"
							"uniform vec2 outputExtents;\n"
							"uniform bool guiCoordinates;\n"
							"uniform int effect;\n"
							"uniform int effectWidth;\n"
							"uniform vec4 effectRect;\n"
							"uniform vec4 glyphRect;\n"
							"in vec4 Color;\n"
							"out vec4 FragColor;\n"
							"varying vec2 vGlyphTexCoord;\n"
							"varying vec2 vOutputCoord;\n"
							"\n"
							"int adjust_y(float y)\n"
							"{\n"
							"	if (guiCoordinates)\n"
							"		return int(outputExtents.y) - 1 - int(y);\n"
							"	else\n"
							"		return int(y);\n"
							"}\n"
							"void main()\n"
							"{\n"
							"	ivec2 dtpos = ivec2(vOutputCoord);\n"
							"	if (guiCoordinates)\n"
							"		dtpos.y = int(outputExtents.y) - 1 - dtpos.y;\n"
							"   switch(effect)\n"
							"   {\n"
							"   case 0:\n"
							"		{\n"
							"			vec4 rgbAlpha = texture(glyphTexture, vGlyphTexCoord);\n"
							"			if (rgbAlpha.rgb == vec3(1.0, 1.0, 1.0))\n"
							"				FragColor = Color;\n"
							"			else if (rgbAlpha.rgb == vec3(0.0, 0.0, 0.0))\n"
							"				discard;\n"
							"			else\n"
							"			{\n"
							"				vec4 rgbDestination = texelFetch(outputTexture, dtpos, 0);\n"
							"				FragColor = vec4(Color.rgb * rgbAlpha.bgr * Color.a + rgbDestination.rgb * (vec3(1.0, 1.0, 1.0) - rgbAlpha.bgr * Color.a), 1.0);\n"
							"			}\n"
							"		}\n"
							"		break;\n"
							"	case 1:\n"
							"		{\n"
							"		 	vec4 rgbAlpha = texture(glyphTexture, vGlyphTexCoord);\n"
							"			if (rgbAlpha.rgb != vec3(0.0, 0.0, 0.0))\n"
							"				FragColor = vec4(Color.rgb, ((rgbAlpha.r + rgbAlpha.g + rgbAlpha.b) / 3.0 * Color.a));\n"
							"			else\n"
							"				discard;\n"
							"		}\n"
							"		break;\n"
							"   case 2:\n"/* todo */
							"		if (dtpos.x >= effectRect.x && dtpos.x < effectRect.z && dtpos.y >= adjust_y(effectRect.y) && dtpos.y < adjust_y(effectRect.w))\n"
							"		{\n"
							"			for (int y = int(dtpos.y) - effectWidth; y <= int(dtpos.y) + effectWidth; ++y)\n"
							"				for (int x = int(dtpos.x) - effectWidth; x <= int(dtpos.x) + effectWidth; ++x)\n"
							"					if (x >= glyphRect.x && x < glyphRect.z && y >= adjust_y(glyphRect.y) && y < adjust_y(glyphRect.w))\n"
							"					{\n"
							"						if (texelFetch(glyphTexture, ivec2(glyphOrigin) + ivec2(x, adjust_y(glyphRect.w) - y) - ivec2(glyphRect.x, adjust_y(glyphRect.y)), 0).rgb != vec3(0.0, 0.0, 0.0))\n"
							"						{\n"
							"							FragColor = Color.rgba;\n"
							"							return;\n"
							"						}\n"
							"					}\n"
							"		}\n"
							"		FragColor = vec4(0.0, 0.0, 0.0, 0.0);\n"
							"		break;\n"
							"	}\n"
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
							"#version 130\n"/* todo */
							"precision mediump float;\n"
							"uniform mat4 uProjectionMatrix;\n"
							"in vec3 VertexPosition;\n"
							"in vec4 VertexColor;\n"
							"in vec2 VertexTextureCoord;\n"
							"out vec4 Color;\n"
							"varying vec2 vGlyphTexCoord;\n"
							"void main()\n"
							"{\n"
							"	Color = VertexColor / 255.0;\n"
							"   gl_Position = uProjectionMatrix * vec4(VertexPosition, 1.0);\n"
							"	vGlyphTexCoord = VertexTextureCoord;\n"
							"}\n"),
						GL_VERTEX_SHADER),
					std::make_pair(
						std::string(
							"#version 130\n"/* todo */
							"precision mediump float;\n"
							"uniform sampler2D glyphTexture;\n"
							"uniform vec2 glyphOrigin;\n"
							"uniform sampler2D glyphDestinationTexture;\n"
							"uniform vec2 glyphTextureOffset;\n"
							"uniform vec2 glyphTextureExtents;\n"
							"uniform vec2 glyphDestinationTextureExtents;\n"
							"in vec4 Color;\n"
							"out vec4 FragColor;\n"
							"varying vec2 vGlyphTexCoord;\n"
							"void main()\n"
							"{\n"
							"	vec4 rgbAlpha = texture(glyphTexture, vGlyphTexCoord);\n"
							"	FragColor = vec4(Color.rgb, (rgbAlpha.r + rgbAlpha.g + rgbAlpha.b) / 3.0);\n"
							"}\n"),
						GL_FRAGMENT_SHADER)
					},
					{ "VertexPosition", "VertexColor", "VertexTextureCoord" });
			break;
		}
	}

	i_font_manager& opengl_renderer::font_manager()
	{
		return iFontManager;
	}

	i_texture_manager& opengl_renderer::texture_manager()
	{
		return iTextureManager;
	}

	bool opengl_renderer::shader_program_active() const
	{
		return iActiveProgram != iShaderPrograms.end();
	}

	void opengl_renderer::activate_shader_program(i_native_graphics_context& aGraphicsContext, i_shader_program& aProgram)
	{
		for (auto i = iShaderPrograms.begin(); i != iShaderPrograms.end(); ++i)
			if (&*i == &aProgram)
			{
				if (iActiveProgram != i)
				{
					iActiveProgram = i;
					glCheck(glUseProgram(reinterpret_cast<GLuint>(iActiveProgram->handle())));
				}
				if (iActiveProgram->has_projection_matrix())
					iActiveProgram->set_projection_matrix(aGraphicsContext);
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

	const opengl_renderer::i_shader_program& opengl_renderer::texture_shader_program() const
	{
		return *iTextureProgram;
	}

	opengl_renderer::i_shader_program& opengl_renderer::texture_shader_program()
	{
		return *iTextureProgram;
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

	bool opengl_renderer::is_subpixel_rendering_on() const
	{
		return iSubpixelRendering;
	}
	
	void opengl_renderer::subpixel_rendering_on()
	{
		if (!iSubpixelRendering)
		{
			iSubpixelRendering = true;
			subpixel_rendering_changed.trigger();
		}
	}

	void opengl_renderer::subpixel_rendering_off()
	{
		if (iSubpixelRendering)
		{
			iSubpixelRendering = false;
			subpixel_rendering_changed.trigger();
		}
	}

	const std::array<GLuint, 3>& opengl_renderer::gradient_textures() const
	{
		// todo: use texture class
		glCheck(glEnable(GL_TEXTURE_RECTANGLE));
		if (iGradientTextures == boost::none)
		{
			iGradientTextures.emplace(std::array<GLuint, 3>{});
			glCheck(glGenTextures(1, &(*iGradientTextures)[0]));
			glCheck(glGenTextures(1, &(*iGradientTextures)[1]));
			glCheck(glGenTextures(1, &(*iGradientTextures)[2]));
			GLint previousTexture;
			glCheck(glGetIntegerv(GL_TEXTURE_BINDING_RECTANGLE, &previousTexture));
			glCheck(glBindTexture(GL_TEXTURE_RECTANGLE, (*iGradientTextures)[0]));
			glCheck(glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
			glCheck(glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
			static const std::array<float, gradient::MaxStops> sZeroStopPositions = {};
			glCheck(glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_R32F, static_cast<GLsizei>(gradient::MaxStops), 1, 0, GL_RED, GL_FLOAT, &sZeroStopPositions[0]));
			glCheck(glBindTexture(GL_TEXTURE_RECTANGLE, (*iGradientTextures)[1]));
			glCheck(glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
			glCheck(glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
			static const std::array<std::array<uint8_t, 4>, gradient::MaxStops> sZeroStopColours = {};
			glCheck(glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_RGBA, static_cast<GLsizei>(gradient::MaxStops), 1, 0, GL_RGBA, GL_FLOAT, &sZeroStopColours[0]));
			glCheck(glBindTexture(GL_TEXTURE_RECTANGLE, (*iGradientTextures)[2]));
			glCheck(glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
			glCheck(glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
			static const std::array<float, GRADIENT_FILTER_SIZE * GRADIENT_FILTER_SIZE> sFilter = {};
			glCheck(glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_R32F, GRADIENT_FILTER_SIZE, GRADIENT_FILTER_SIZE, 0, GL_RED, GL_FLOAT, &sFilter[0]));
			glCheck(glBindTexture(GL_TEXTURE_RECTANGLE, previousTexture));
		}
		return *iGradientTextures;
	}

	bool opengl_renderer::process_events()
	{
		bool didSome = false;
		auto lastRenderTime = neolib::thread::program_elapsed_ms();
		bool finished = false;
		while (!finished)
		{	
			finished = true;
			for (std::size_t s = 0; s < app::instance().surface_manager().surface_count(); ++s)
			{
				auto& surface = app::instance().surface_manager().surface(s);
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
		GLuint programHandle = glCheck(glCreateProgram());
		if (0 == programHandle)
			throw failed_to_create_shader_program("Failed to create shader program object");
		bool hasProjectionMatrix = false;
		for (auto& s : aShaders)
		{
			GLuint shader = glCheck(glCreateShader(s.second));
			if (0 == shader)
				throw failed_to_create_shader_program("Failed to create shader object");
			std::string source = s.first;
			if (source.find("uProjectionMatrix") != std::string::npos)
				hasProjectionMatrix = true;
			if (renderer() == neogfx::renderer::DirectX)
			{
				std::size_t v;
				const std::size_t VERSION_STRING_LENGTH = 12;
				if ((v = source.find("#version 130")) != std::string::npos)
					source.replace(v, VERSION_STRING_LENGTH, "#version 110");
				else if ((v = source.find("#version 150")) != std::string::npos)
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
				glCheck(glGetShaderInfoLog(shader, buf.size(), NULL, &buf[0]));
				std::string error(&buf[0]);
				throw failed_to_create_shader_program(error);
			}
			glCheck(glAttachShader(programHandle, shader));
		}
		shader_program program(programHandle, hasProjectionMatrix);
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
}