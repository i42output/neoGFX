// opengl_window.cpp
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
#include <numeric>
#include <neogfx/app/app.hpp>
#include "opengl_window.hpp"
#ifdef _WIN32
#include <D2d1.h>
#endif

namespace neogfx
{
	opengl_window::opengl_window(i_rendering_engine& aRenderingEngine, i_surface_manager& aSurfaceManager, i_window& aWindow) :
		native_window(aRenderingEngine, aSurfaceManager),
		iWindow(aWindow),
		iLogicalCoordinateSystem(neogfx::logical_coordinate_system::AutomaticGui),
		iFrameRate(60),
		iFrameCounter(0),
		iLastFrameTime(0),
		iRendering(false),
		iDestroying(false),
		iPaused(0)
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
			return iLogicalCoordinates;
		case neogfx::logical_coordinate_system::AutomaticGui:
			return iLogicalCoordinates = std::make_pair<vec2, vec2>({ 0.0, extents().cy }, { extents().cx, 0.0 });
		case neogfx::logical_coordinate_system::AutomaticGame:
			return iLogicalCoordinates = std::make_pair<vec2, vec2>({ 0.0, 0.0 }, { extents().cx, extents().cy });
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
		if (aInvalidatedRect.cx != 0.0 && aInvalidatedRect.cy != 0.0 && 
			iInvalidatedRects.find(aInvalidatedRect) == iInvalidatedRects.end())
			iInvalidatedRects.insert(aInvalidatedRect);
	}

	void opengl_window::render(bool aOOBRequest)
	{
		if (iRendering || rendering_engine().creating_window() || iPaused)
			return;

		uint64_t now = app::instance().program_elapsed_ms();

		if (!aOOBRequest)
		{
			if (processing_event())
				return;

			if (iFrameRate != boost::none && now - iLastFrameTime < 1000 / (has_rendering_priority() ? *iFrameRate : *iFrameRate / 10.0))
				return;

			if (!iWindow.native_window_ready_to_render())
				return;
		}

		rendering_check.trigger();

		if (iInvalidatedRects.empty())
			return;

		++iFrameCounter;

		iRendering = true;
		iLastFrameTime = now;

		rendering.trigger();

		rect invalidatedRect = *iInvalidatedRects.begin();
		for (const auto& ir : iInvalidatedRects)
		{
			invalidatedRect = invalidatedRect.combine(ir);
		}
		iInvalidatedRects.clear();
		invalidatedRect.cx = std::min(invalidatedRect.cx, surface_size().cx - invalidatedRect.x);
		invalidatedRect.cy = std::min(invalidatedRect.cy, surface_size().cy - invalidatedRect.y);

		rendering_engine().activate_context(*this);

		glCheck(glViewport(0, 0, static_cast<GLsizei>(extents().cx), static_cast<GLsizei>(extents().cy)));
		glCheck(glEnableClientState(GL_VERTEX_ARRAY));
		glCheck(glEnableClientState(GL_COLOR_ARRAY));
		glCheck(glEnableClientState(GL_TEXTURE_COORD_ARRAY));
		glCheck(glEnable(GL_TEXTURE_2D));
		glCheck(glEnable(GL_MULTISAMPLE));
		glCheck(glEnable(GL_BLEND));
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
		GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if (status != GL_NO_ERROR && status != GL_FRAMEBUFFER_COMPLETE)
			throw failed_to_create_framebuffer(status);
		glCheck(glBindFramebuffer(GL_FRAMEBUFFER, iFrameBuffer));
		GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0 };
		glCheck(glDrawBuffers(sizeof(drawBuffers) / sizeof(drawBuffers[0]), drawBuffers));

		glCheck(iWindow.native_window_render(invalidatedRect));

		glCheck(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0));
		glCheck(glBindFramebuffer(GL_READ_FRAMEBUFFER, iFrameBuffer));
		glCheck(glBlitFramebuffer(0, 0, static_cast<GLint>(extents().cx), static_cast<GLint>(extents().cy), 0, 0, static_cast<GLint>(extents().cx), static_cast<GLint>(extents().cy), GL_COLOR_BUFFER_BIT, GL_NEAREST));

		display();

		iRendering = false;

		rendering_finished.trigger();

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

	i_window& opengl_window::window() const
	{
		return iWindow;
	}

	void opengl_window::destroying()
	{
		if (iDestroying)
			return;
		iDestroying = true;
		if (iFrameBufferSize != size{})
		{
			rendering_engine().activate_context(*this);
			glCheck(glDeleteRenderbuffers(1, &iDepthStencilBuffer));
			glCheck(glDeleteTextures(1, &iFrameBufferTexture));
			glCheck(glDeleteFramebuffers(1, &iFrameBuffer));
		}
	}

	void opengl_window::destroyed()
	{
	}
}