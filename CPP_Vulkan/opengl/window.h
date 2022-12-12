#pragma once

#include <utils/win32/window/window.h>
#include <utils/memory.h>
#include <utils/containers/handled_container.h>

namespace utils::graphics::opengl::window
	{
	class window : public virtual utils::win32::window::base
		{
		public:
			inline window() : context{ create_context() }
				{
				::ShowWindow(get_handle(), SW_SHOWDEFAULT);
				}
			inline ~window() { ::wglDeleteContext(context); }

			inline HGLRC get_context() const noexcept { return context; }
				
			inline void swap_buffers()
				{
				// swap buffers
				HDC hDC = ::GetDC(get_handle());
				::SwapBuffers( hDC );
				::ReleaseDC(get_handle(), hDC );

				// create message WM_ERASEBKGND
				::InvalidateRect(get_handle(), NULL, TRUE);

				glFinish();
				}

			std::optional<LRESULT> procedure(UINT msg, WPARAM w, LPARAM l)
				{
				using namespace std::string_literals;
				switch (msg)
					{
					case WM_ERASEBKGND:
						{
						return 1;
						}
					}
				return std::nullopt;
				}

		private:
			inline HGLRC create_context()
				{
				HDC hDC = ::GetDC(get_handle());

				DWORD pixelFormatFlags = PFD_SUPPORT_OPENGL | PFD_SUPPORT_COMPOSITION | PFD_GENERIC_ACCELERATED | PFD_DRAW_TO_WINDOW | PFD_DOUBLEBUFFER;
				PIXELFORMATDESCRIPTOR pfd =
					{
					  sizeof(PIXELFORMATDESCRIPTOR),
					  1,
					  pixelFormatFlags,         //Flags
					  PFD_TYPE_RGBA,            //The kind of framebuffer. RGBA or palette.
					  32,                       //Colordepth of the framebuffer.
					  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
					  24,                       //Number of bits for the depthbuffer
					  8,                        //Number of bits for the stencilbuffer
					  0,                        //Number of Aux buffers in the framebuffer.
					  PFD_MAIN_PLANE,
					  0, 0, 0, 0
					};
				int pixelFormat{ ChoosePixelFormat(hDC, &pfd) };
				::SetPixelFormat(hDC, pixelFormat, &pfd);
				auto context{ ::wglCreateContext(hDC) };

				// make OpenGL context the current context
				::wglMakeCurrent(hDC, context);

				// release device context
				::ReleaseDC(get_handle(), hDC);

				return context;
				}

			HGLRC context;
		};
	}