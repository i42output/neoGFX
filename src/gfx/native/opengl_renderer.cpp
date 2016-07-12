// opengl_renderer.cpp
/*
  neogfx C++ GUI Library
  Copyright(C) 2016 Leigh Johnston
  
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
#include <D2d1.h>
#endif
#include "opengl_renderer.hpp"
#include "../../gui/window/native/opengl_window.hpp"

#include "gradient.vert.hpp"
#include "gradient.frag.hpp"

#include <neogfx/app/app.hpp>

namespace neogfx
{
	detail::screen_metrics::screen_metrics() :
		iSubpixelFormat(SubpixelFormatNone)
	{
#ifdef _WIN32
		ID2D1Factory* pDirect2dFactory;
		D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &pDirect2dFactory);
		FLOAT dpiX, dpiY;
		pDirect2dFactory->GetDesktopDpi(&dpiX, &dpiY);
		pDirect2dFactory->Release();
		iPixelDensityDpi = size(static_cast<size::dimension_type>(dpiX), static_cast<size::dimension_type>(dpiY));
		HKEY hkeySubpixelFormat;
		if (::RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Avalon.Graphics\\DISPLAY1", 0, KEY_READ, &hkeySubpixelFormat) == ERROR_SUCCESS)
		{
			DWORD subpixelFormat = 0;
			DWORD cbValue = sizeof(subpixelFormat);
			if (RegQueryValueEx(hkeySubpixelFormat, L"PixelStructure", NULL, NULL, (LPBYTE)&subpixelFormat, &cbValue) == ERROR_SUCCESS)
			{
				switch (subpixelFormat)
				{
				case 1:
					iSubpixelFormat = SubpixelFormatRGBHorizontal;
					break;
				case 2:
					iSubpixelFormat = SubpixelFormatBGRHorizontal;
					break;
				}
			}
			::RegCloseKey(hkeySubpixelFormat);
		}
		else
			iSubpixelFormat = SubpixelFormatRGBHorizontal;
#endif
		/* Sub-pixel rendering currently disabled. */
		iSubpixelFormat = SubpixelFormatNone; 
	}

	dimension detail::screen_metrics::horizontal_dpi() const
	{
		return iPixelDensityDpi.cx;
	}

	dimension detail::screen_metrics::vertical_dpi() const
	{
		return iPixelDensityDpi.cy;
	}

	size detail::screen_metrics::extents() const
	{
		throw unsupported_function();
	}

	dimension detail::screen_metrics::em_size() const
	{
		throw unsupported_function();
	}

	i_screen_metrics::subpixel_format_e detail::screen_metrics::subpixel_format() const
	{
		return iSubpixelFormat;
	}

	opengl_renderer::shader_program::shader_program(GLuint aHandle) :
		iHandle(aHandle)
	{
	}

	void* opengl_renderer::shader_program::handle() const
	{
		return reinterpret_cast<void*>(iHandle);
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

	opengl_renderer::opengl_renderer() :
		iFontManager(*this, iScreenMetrics),
		iActiveProgram(iShaderPrograms.end())
	{
	}

	void opengl_renderer::initialize()
	{
		glCheck(glewInit());
		iMonochromeProgram = create_shader_program(
			shaders
			{
				std::make_pair(
					std::string(
						"#version 130\n"
						"uniform sampler2D tex;"
						"void main()\n"
						"{\n"
						"	float gray = dot(gl_Color.rgb * texture2D(tex, gl_TexCoord[1].xy).rgb, vec3(0.299, 0.587, 0.114));\n"
						"	gl_FragColor = vec4(gray, gray, gray, gl_Color.a * texture2D(tex, gl_TexCoord[1].xy).a);\n"
						"}\n"),
					GL_FRAGMENT_SHADER) 
			}, {});
		iGradientProgram = create_shader_program(
			shaders
			{
				std::make_pair(
					glsl::NEOGFX_GRADIENT_VERT,
					GL_VERTEX_SHADER),
				std::make_pair(
					glsl::NEOGFX_GRADIENT_FRAG,
					GL_FRAGMENT_SHADER)
			}, {});
		switch (screen_metrics().subpixel_format())
		{
		case i_screen_metrics::SubpixelFormatRGBHorizontal:
			iGlyphProgram = create_shader_program(
				shaders
				{
					std::make_pair(
						std::string(
							"#version 130\n"
							"in vec3 VertexPosition;\n"
							"in vec4 VertexColor;\n"
							"in vec2 VertexTextureCoord;\n"
							"out vec4 Color;\n"
							"varying vec2 vGlyphTexCoord;\n"
							"void main()\n"
							"{\n"
							"	Color = VertexColor;\n"
							"   gl_Position = gl_ModelViewProjectionMatrix * vec4(VertexPosition, 1.0);\n"
							"	vGlyphTexCoord = VertexTextureCoord;\n"
							"}\n"),
						GL_VERTEX_SHADER),
					std::make_pair(
						std::string(
							"#version 130\n"
							"uniform sampler2D glyphTexture;\n"
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
							"   vec4 rgbDestination = texture(glyphDestinationTexture, (vGlyphTexCoord - glyphTextureOffset) / glyphTextureExtents * glyphDestinationTextureExtents);\n"
							"	if (rgbAlpha.rgb == vec3(1.0, 1.0, 1.0))\n"
							"		FragColor = Color;\n"
							"	else\n"
							"		FragColor = vec4(Color.rgb * rgbAlpha.rgb + rgbDestination.rgb * (vec3(1.0, 1.0, 1.0) - rgbAlpha.rgb), 1.0);\n"
							"}\n"),
						GL_FRAGMENT_SHADER)
					},
					{ "VertexPosition", "VertexColor", "VertexTextureCoord" });
			break;
		case i_screen_metrics::SubpixelFormatBGRHorizontal:
			iGlyphProgram = create_shader_program(
				shaders
				{
					std::make_pair(
						std::string(
							"#version 130\n"
							"in vec3 VertexPosition;\n"
							"in vec4 VertexColor;\n"
							"in vec2 VertexTextureCoord;\n"
							"out vec4 Color;\n"
							"varying vec2 vGlyphTexCoord;\n"
							"void main()\n"
							"{\n"
							"	Color = VertexColor;\n"
							"   gl_Position = gl_ModelViewProjectionMatrix * vec4(VertexPosition, 1.0);\n"
							"	vGlyphTexCoord = VertexTextureCoord;\n"
							"}\n"),
						GL_VERTEX_SHADER),
					std::make_pair(
						std::string(
							"#version 130\n"
							"uniform sampler2D glyphTexture;\n"
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
							"   vec4 rgbDestination = texture(glyphDestinationTexture, (vGlyphTexCoord - glyphTextureOffset) / glyphTextureExtents * glyphDestinationTextureExtents);\n"
							"	if (rgbAlpha.rgb == vec3(1.0, 1.0, 1.0))\n"
							"		FragColor = Color;\n"
							"	else\n"
							"		FragColor = vec4(Color.rgb * rgbAlpha.bgr + rgbDestination.rgb * (vec3(1.0, 1.0, 1.0) - rgbAlpha.bgr), 1.0);\n"
							"}\n"),
						GL_FRAGMENT_SHADER)
					},
					{ "VertexPosition", "VertexColor", "VertexTextureCoord" });
			break;
		case i_screen_metrics::SubpixelFormatRGBVertical:
		case i_screen_metrics::SubpixelFormatBGRVertical:
		default:
			iGlyphProgram = create_shader_program(
				shaders
				{
					std::make_pair(
						std::string(
							"#version 130\n"
							"in vec3 VertexPosition;\n"
							"in vec4 VertexColor;\n"
							"in vec2 VertexTextureCoord;\n"
							"out vec4 Color;\n"
							"varying vec2 vGlyphTexCoord;\n"
							"void main()\n"
							"{\n"
							"	Color = VertexColor;\n"
							"   gl_Position = gl_ModelViewProjectionMatrix * vec4(VertexPosition, 1.0);\n"
							"	vGlyphTexCoord = VertexTextureCoord;\n"
							"}\n"),
						GL_VERTEX_SHADER),
					std::make_pair(
						std::string(
							"#version 130\n"
							"uniform sampler2D glyphTexture;\n"
							"in vec4 Color;\n"
							"out vec4 FragColor;\n"
							"varying vec2 vGlyphTexCoord;\n"
							"void main()\n"
							"{\n"
							"	FragColor = vec4(Color.xyz, Color.a * texture(glyphTexture, vGlyphTexCoord).a);\n"
							"}\n"),
						GL_FRAGMENT_SHADER)
				},
				{ "VertexPosition", "VertexColor", "VertexTextureCoord" });
			break;
		}
	}

	const i_screen_metrics& opengl_renderer::screen_metrics() const
	{
		return iScreenMetrics;
	}

	i_font_manager& opengl_renderer::font_manager()
	{
		return iFontManager;
	}

	i_texture_manager& opengl_renderer::texture_manager()
	{
		return iTextureManager;
	}

	void opengl_renderer::activate_shader_program(i_shader_program& aProgram)
	{
		for (auto i = iShaderPrograms.begin(); i != iShaderPrograms.end(); ++i)
			if (&*i == &aProgram)
			{
				iActiveProgram = i;
				glCheck(glUseProgram(reinterpret_cast<GLuint>(iActiveProgram->handle())));
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

	const opengl_renderer::i_shader_program& opengl_renderer::monochrome_shader_program() const
	{
		return *iMonochromeProgram;
	}

	opengl_renderer::i_shader_program& opengl_renderer::monochrome_shader_program()
	{
		return *iMonochromeProgram;
	}

	const opengl_renderer::i_shader_program& opengl_renderer::gradient_shader_program() const
	{
		return *iGradientProgram;
	}

	opengl_renderer::i_shader_program& opengl_renderer::gradient_shader_program()
	{
		return *iGradientProgram;
	}

	const opengl_renderer::i_shader_program& opengl_renderer::glyph_shader_program() const
	{
		return *iGlyphProgram;
	}

	opengl_renderer::i_shader_program& opengl_renderer::glyph_shader_program()
	{
		return *iGlyphProgram;
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
				if (!surface.destroyed() && surface.native_surface().pump_event())
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

	opengl_renderer::shader_programs::iterator opengl_renderer::create_shader_program(const shaders& aShaders, const std::vector<std::string>& aVariables)
	{
		GLuint programHandle = glCheck(glCreateProgram());
		if (0 == programHandle)
			throw failed_to_create_shader_program("Failed to create shader program object");
		for (auto& s : aShaders)
		{
			GLuint shader = glCheck(glCreateShader(s.second));
			if (0 == shader)
				throw failed_to_create_shader_program("Failed to create shader object");
			const char* codeArray[] = { s.first.c_str() };
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
		shader_program program(programHandle);
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