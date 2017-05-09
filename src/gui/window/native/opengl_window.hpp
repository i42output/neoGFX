// opengl_window.hpp
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

#pragma once

#include <neogfx/neogfx.hpp>
#include <unordered_set>
#include <boost/lexical_cast.hpp>
#include <neolib/string_utils.hpp>
#include <neolib/timer.hpp>
#include <neogfx/neogfx.hpp>
#include "../../../gfx/native/opengl.hpp"
#include "../../../gfx/native/opengl.hpp"
#include "../../../gfx/native/i_native_graphics_context.hpp"
#include "../../../gfx/native/opengl_texture.hpp"
#include "native_window.hpp"

namespace neogfx
{
	class opengl_window : public native_window
	{
	public:
		struct failed_to_create_framebuffer : std::runtime_error { 
			failed_to_create_framebuffer(GLenum aErrorCode) : 
				std::runtime_error("neogfx::opengl_window::failed_to_create_framebuffer: Failed to create frame buffer, reason: " + glErrorString(aErrorCode)) {} };
		struct busy_rendering : std::logic_error { busy_rendering() : std::logic_error("neogfx::opengl_window::busy_rendering") {} };
	public:
		opengl_window(i_rendering_engine& aRenderingEngine, i_surface_manager& aSurfaceManager, i_window& aWindow);
		~opengl_window();
	public:
		virtual neogfx::logical_coordinate_system logical_coordinate_system() const;
		virtual void set_logical_coordinate_system(neogfx::logical_coordinate_system aSystem);
		virtual const std::pair<vec2, vec2>& logical_coordinates() const;
		virtual void set_logical_coordinates(const std::pair<vec2, vec2>& aCoordinates);
	public:
		virtual uint64_t frame_counter() const;
		virtual bool using_frame_buffer() const;
		virtual void limit_frame_rate(uint32_t aFps);
	public:
		virtual void invalidate(const rect& aInvalidatedRect);
		virtual void render(bool aOOBRequest = false);
		virtual bool is_rendering() const;
		virtual void* rendering_target_texture_handle() const;
		virtual size rendering_target_texture_extents() const;
	public:
		virtual bool metrics_available() const;
		virtual size extents() const;
		virtual dimension horizontal_dpi() const;
		virtual dimension vertical_dpi() const;
		virtual dimension em_size() const;
	protected:
		virtual i_window& window() const;
		virtual void destroying();
		virtual void destroyed();
	private:
		virtual void display() = 0;
	private:
		i_window& iWindow;
		size iPixelDensityDpi;
		neogfx::logical_coordinate_system iLogicalCoordinateSystem;
		mutable std::pair<vec2, vec2> iLogicalCoordinates;
		GLuint iFrameBuffer;
		GLuint iFrameBufferTexture;
		GLuint iDepthStencilBuffer;
		size iFrameBufferSize;
		std::unordered_set<rect> iInvalidatedRects;
		uint64_t iFrameCounter;
		boost::optional<uint32_t> iFrameRate;
		uint64_t iLastFrameTime;
		bool iRendering;
		bool iDestroying;
	};
}