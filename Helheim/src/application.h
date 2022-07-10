#pragma once
#include "../imgui/imgui.h"
#include "../imgui/imgui_impl_dx9.h"
#include "../imgui/imgui_impl_win32.h"

#include <d3d9.h>
#include <tchar.h>
#include <string>

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace Helheim
{
	struct AppSpecification {
		std::string windowName = "Helheim DX9";
		std::string className = "HelheimClass";

		int width = GetSystemMetrics(SM_CXFULLSCREEN);
		int height = GetSystemMetrics(SM_CYFULLSCREEN);

		void SetMetrics(int x, int y) {
			width = x;
			height = y;
		}
	};



	class Application
	{
	private:
		AppSpecification _appSpec;
		WNDCLASSEX wc;
		HWND hwnd;
		static Application* pApp;

	public:
		static Application* getInstance(const AppSpecification& appSpec = AppSpecification());

	private:
		Application(const AppSpecification& appSpec = AppSpecification());
		Application(Application& const) = delete;
		Application& operator=(Application const&) = delete;

	private:
		static LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
		static bool CreateDeviceD3D(HWND hWnd);
		static void CleanupDeviceD3D();
		static void ResetDevice();

		bool Init();
		void Destroy();
	public:
		void RenderLoop(void (*OnRender)(bool&));


	};
}