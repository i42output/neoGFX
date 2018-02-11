// opengl_window.cpp
/*
  neogfx C++ GUI Library
  Copyright(C) 2015-present Leigh Johnston
  
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
#include <numeric>
#include <neogfx/app/app.hpp>
#include <neogfx/hid/i_surface_window.hpp>
#include "opengl_window.hpp"
#ifdef _WIN32
#include <D2d1.h>
#endif

namespace neogfx
{
	opengl_window::opengl_window(i_rendering_engine& aRenderingEngine, i_surface_manager& aSurfaceManager, i_surface_window& aWindow) :
		native_window{ aRenderingEngine, aSurfaceManager },
		iSurfaceWindow{ aWindow },
		iLogicalCoordinateSystem{ neogfx::logical_coordinate_system::AutomaticGui },
		iFrameRate{ 60 },
		iFrameCounter{ 0 },
		iLastFrameTime{ 0 },
		iRendering{ false },
		iPaused{ 0 }
	{
#ifdef _WIN32
		ID2D1Factory* m_pDirect2dFactory;
		D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_pDirect2dFactory);
		FLOAT dpiX, dpiY;
		m_pDirect2dFactory->GetDesktopDpi(&dpiX, &dpiY);
		iPixelDensityDpi = size(static_cast<size::dimension_type>(dpiX), static_cast<size::dimension_type>(dpiY));
#endif
	}

	opengl_window::~opengl_window()
	{
		set_destroyed();
		if (rendering_engine().active_context_surface() == this)
			rendering_engine().deactivate_context();
	}

	neogfx::logical_coordinate_system opengl_window::logical_coordinate_system() const
	{
		return iLogicalCoordinateSystem;
	}

	void opengl_window::set_logical_coordinate_system(neogfx::logical_coordinate_system aSystem)
	{
		iLogicalCoordinateSystem = aSystem;
	}

	const std::pair<vec2, vec2>& opengl_window::logical_coordinates() const
	{
		switch (iLogicalCoordinateSystem)
		{
		case neogfx::logical_coordinate_system::Specified:
			break;
		case neogfx::logical_coordinate_system::AutomaticGui:
			iLogicalCoordinates.first = vec2{ 0.0, extents().cy };
			iLogicalCoordinates.second = vec2{ extents().cx, 0.0 };
			break;
		case neogfx::logical_coordinate_system::AutomaticGame:
			iLogicalCoordinates.first = vec2{ 0.0, 0.0 };
			iLogicalCoordinates.second = vec2{ extents().cx, extents().cy };
			break;
		}
		return iLogicalCoordinates;
	}

	void opengl_window::set_logical_coordinates(const std::pair<vec2, vec2>& aCoordinates)
	{
		iLogicalCoordinates = aCoordinates;
	}

	uint64_t opengl_window::frame_counter() const
	{
		return iFrameCounter;
	}

	void opengl_window::limit_frame_rate(uint32_t aFps)
	{
		iFrameRate = aFps;
	}

	double opengl_window::fps() const
	{
		return std::accumulate(iFpsData.begin(), iFpsData.end(), 0.0) / iFpsData.size();
	}

	void opengl_window::invalidate(const rect& aInvalidatedRect)
	{
		if (aInvalidatedRect.cx != 0.0 && aInvalidatedRect.cy != 0.0)
		{
			if (!has_invalidated_area())
				iInvalidatedArea = aInvalidatedRect.ceil();
			else
				iInvalidatedArea = invalidated_area().combine(aInvalidatedRect).ceil();
		}
	}

	bool opengl_window::has_invalidated_area() const
	{
		return iInvalidatedArea != boost::none;
	}

	const rect& opengl_window::invalidated_area() const
	{
		if (has_invalidated_area())
			return *iInvalidatedArea;
		throw no_invalidated_area();
	}

	rect opengl_window::validate()
	{
		if (has_invalidated_area())
		{
			rect validatedArea = invalidated_area();
			iInvalidatedArea = boost::none;
			return validatedArea;
		}
		throw no_invalidated_area();
	}

	bool opengl_window::can_render() const
	{
		return !iPaused;
	}

	void opengl_window::render(bool aOOBRequest)
	{
		if (iRendering || rendering_engine().creating_window() || !can_render())
			return;

		uint64_t now = app::instance().program_elapsed_ms();

		if (!aOOBRequest)
		{
			if (processing_event())
				return;

			if (iFrameRate != boost::none && now - iLastFrameTime < 1000 / (has_rendering_priority() ? *iFrameRate : *iFrameRate / 10.0))
				return;

			if (!surface_window().native_window_ready_to_render())
				return;
		}

		if (!has_invalidated_area())
			return;

		if (invalidated_area().cx <= 0.0 || invalidated_area().cy <= 0.0)
		{
			validate();
			return;
		}

		//std::cerr << "to render: " << rectToRender << std::endl;

		++iFrameCounter;

		iRendering = true;
		iLastFrameTime = now;

		surface_window().rendering.trigger();

		rendering_engine().activate_context(*this);

		glCheck(glViewport(0, 0, static_cast<GLsizei>(extents().cx), static_cast<GLsizei>(extents().cy)));
		glCheck(glEnableClientState(GL_VERTEX_ARRAY));
		glCheck(glEnableClientState(GL_COLOR_ARRAY));
		glCheck(glEnableClientState(GL_TEXTURE_COORD_ARRAY));
		glCheck(glEnable(GL_TEXTURE_2D));
		glCheck(glEnable(GL_MULTISAMPLE));
		glCheck(glEnable(GL_BLEND));
		glCheck(glEnable(GL_DEPTH_TEST));
		glCheck(glDepthFunc(GL_LEQUAL));
		if (iFrameBufferSize.cx < static_cast<double>(extents().cx) || iFrameBufferSize.cy < static_cast<double>(extents().cy))
		{
			if (iFrameBufferSize != size{})
			{
				glCheck(glDeleteRenderbuffers(1, &iDepthStencilBuffer));
				glCheck(glDeleteTextures(1, &iFrameBufferTexture));
				glCheck(glDeleteFramebuffers(1, &iFrameBuffer));
			}
			iFrameBufferSize = size(
				iFrameBufferSize.cx < extents().cx ? extents().cx * 1.5f : iFrameBufferSize.cx,
				iFrameBufferSize.cy < extents().cy ? extents().cy * 1.5f : iFrameBufferSize.cy);
			glCheck(glGenFramebuffers(1, &iFrameBuffer));
			glCheck(glBindFramebuffer(GL_FRAMEBUFFER, iFrameBuffer));
			glCheck(glGenTextures(1, &iFrameBufferTexture));
			glCheck(glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, iFrameBufferTexture));
			glCheck(glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGBA8, static_cast<GLsizei>(iFrameBufferSize.cx), static_cast<GLsizei>(iFrameBufferSize.cy), true));
			glCheck(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, iFrameBufferTexture, 0));
			glCheck(glGenRenderbuffers(1, &iDepthStencilBuffer));
			glCheck(glBindRenderbuffer(GL_RENDERBUFFER, iDepthStencilBuffer));
			glCheck(glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH24_STENCIL8, static_cast<GLsizei>(iFrameBufferSize.cx), static_cast<GLsizei>(iFrameBufferSize.cy)));
			glCheck(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, iDepthStencilBuffer));
			glCheck(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, iDepthStencilBuffer));
		}
		else
		{
			glCheck(glBindFramebuffer(GL_FRAMEBUFFER, iFrameBuffer));
			glCheck(glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, iFrameBufferTexture));
			glCheck(glBindRenderbuffer(GL_RENDERBUFFER, iDepthStencilBuffer));
		}
		glCheck(glClear(GL_DEPTH_BUFFER_BIT));
		GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if (status != GL_NO_ERROR && status != GL_FRAMEBUFFER_COMPLETE)
			throw failed_to_create_framebuffer(status);
		glCheck(glBindFramebuffer(GL_FRAMEBUFFER, iFrameBuffer));
		GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0 };
		glCheck(glDrawBuffers(sizeof(drawBuffers) / sizeof(drawBuffers[0]), drawBuffers));

		glCheck(surface_window().native_window_render(invalidated_area()));

		glCheck(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0));
		glCheck(glBindFramebuffer(GL_READ_FRAMEBUFFER, iFrameBuffer));
		glCheck(glBlitFramebuffer(0, 0, static_cast<GLint>(extents().cx), static_cast<GLint>(extents().cy), 0, 0, static_cast<GLint>(extents().cx), static_cast<GLint>(extents().cy), GL_COLOR_BUFFER_BIT, GL_NEAREST));

		display();

		iRendering = false;
		validate();

		surface_window().rendering_finished.trigger();

		iFpsData.push_back(1000.0 / (app::instance().program_elapsed_ms() - now));
		if (iFpsData.size() > 25)
			iFpsData.pop_front();		
	}

	void opengl_window::pause()
	{
		++iPaused;
	}

	void opengl_window::resume()
	{
		if (iPaused == 0)
			throw bad_pause_count();
		--iPaused;
	}

	bool opengl_window::is_rendering() const
	{
		return iRendering;
	}

	void* opengl_window::rendering_target_texture_handle() const
	{
		return reinterpret_cast<void*>(iFrameBufferTexture);
	}

	size opengl_window::rendering_target_texture_extents() const
	{
		return iFrameBufferSize;
	}

	bool opengl_window::metrics_available() const
	{
		return true;
	}

	size opengl_window::extents() const
	{
		return surface_size();
	}

	dimension opengl_window::horizontal_dpi() const
	{
		return iPixelDensityDpi.cx;
	}

	dimension opengl_window::vertical_dpi() const
	{
		return iPixelDensityDpi.cy;
	}

	dimension opengl_window::em_size() const
	{
		return 0;
	}

	i_surface_window& opengl_window::surface_window() const
	{
		return iSurfaceWindow;
	}

	void opengl_window::set_destroying()
	{
		destroyable::set_destroying();
		if (iFrameBufferSize != size{})
		{
			rendering_engine().activate_context(*this);
			glCheck(glDeleteRenderbuffers(1, &iDepthStencilBuffer));
			glCheck(glDeleteTextures(1, &iFrameBufferTexture));
			glCheck(glDeleteFramebuffers(1, &iFrameBuffer));
		}
	}

	void opengl_window::set_destroyed()
	{
		destroyable::set_destroyed();
	}

	neolib::i_destroyable& opengl_window::as_destroyable()
	{
		return *this;
	}
}