#include "application.h"
#include<iostream>

static LPDIRECT3D9              g_pD3D = NULL;
static LPDIRECT3DDEVICE9        g_pd3dDevice = NULL;
static D3DPRESENT_PARAMETERS    g_d3dpp = {};
Helheim::Application* Helheim::Application::pApp = NULL;

Helheim::Application* Helheim::Application::getInstance(const AppSpecification& appSpec)
{
    if (pApp == NULL) {
        pApp = new Application(appSpec);
    }
    return pApp;
}


Helheim::Application::Application(const AppSpecification& appSpec)
	: _appSpec(appSpec)
{
    std::cout << "Constructor" << std::endl;
	Init();
}

LRESULT __stdcall Helheim::Application::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    int x;
    int y;

    switch (msg)
    {
    case WM_DISPLAYCHANGE:

        x = GetSystemMetrics(SM_CXFULLSCREEN);
        y = GetSystemMetrics(SM_CYFULLSCREEN);
        ::SetWindowPos(hWnd, NULL, 0, 0, x, y, SWP_NOZORDER | SWP_NOACTIVATE);
        std::cout << "Resolution Changed: " << x << " x " << y << std::endl;
        if (pApp) {
            pApp->_appSpec.SetMetrics(x, y);
            std::cout << "Resolution Changed: " << pApp->_appSpec.width << " x " << pApp->_appSpec.height << std::endl;
        }
        return 0;

    case WM_SIZE:
        if (g_pd3dDevice != NULL && wParam != SIZE_MINIMIZED)
        {
            g_d3dpp.BackBufferWidth = LOWORD(lParam);
            g_d3dpp.BackBufferHeight = HIWORD(lParam);
            ResetDevice();
        }
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
            return 0;
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    case WM_DPICHANGED:
        if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_DpiEnableScaleViewports)
        {
            //const int dpi = HIWORD(wParam);
            //printf("WM_DPICHANGED to %d (%.0f%%)\n", dpi, (float)dpi / 96.0f * 100.0f);
            const RECT* suggested_rect = (RECT*)lParam;
            ::SetWindowPos(hWnd, NULL, suggested_rect->left, suggested_rect->top, suggested_rect->right - suggested_rect->left, suggested_rect->bottom - suggested_rect->top, SWP_NOZORDER | SWP_NOACTIVATE);
        }
        break;
    }
    return ::DefWindowProc(hWnd, msg, wParam, lParam);
}

bool Helheim::Application::CreateDeviceD3D(HWND hWnd)
{
    if ((g_pD3D = Direct3DCreate9(D3D_SDK_VERSION)) == NULL)
        return false;

    // Create the D3DDevice
    ZeroMemory(&g_d3dpp, sizeof(g_d3dpp));
    g_d3dpp.Windowed = TRUE;
    g_d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    g_d3dpp.BackBufferFormat = D3DFMT_UNKNOWN; // Need to use an explicit format with alpha if needing per-pixel alpha composition.
    g_d3dpp.EnableAutoDepthStencil = TRUE;
    g_d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
    //g_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;           // Present with vsync
    g_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;   // Present without vsync, maximum unthrottled framerate
    if (g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &g_d3dpp, &g_pd3dDevice) < 0)
        return false;

    return true;
}

void Helheim::Application::CleanupDeviceD3D()
{
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = NULL; }
    if (g_pD3D) { g_pD3D->Release(); g_pD3D = NULL; }
}

void Helheim::Application::ResetDevice()
{
    ImGui_ImplDX9_InvalidateDeviceObjects();
    HRESULT hr = g_pd3dDevice->Reset(&g_d3dpp);
    if (hr == D3DERR_INVALIDCALL)
        IM_ASSERT(0);
    ImGui_ImplDX9_CreateDeviceObjects();
}

bool Helheim::Application::Init()
{
	wc = { sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, LPCWSTR(_appSpec.className.c_str()), NULL };
	::RegisterClassEx(&wc);
	hwnd = ::CreateWindowEx(WS_EX_LAYERED, wc.lpszClassName, LPCWSTR(_appSpec.windowName.c_str()), WS_POPUP, 0, 0, _appSpec.width, _appSpec.height, NULL, NULL, wc.hInstance, NULL);

    // Initialize Direct3D
    if (!CreateDeviceD3D(hwnd))
    {
        CleanupDeviceD3D();
        ::UnregisterClass(wc.lpszClassName, wc.hInstance);
        return false;
    }

    // Show the window
    SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), 0, LWA_COLORKEY);
    ::ShowWindow(hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
    //io.DisplaySize = ImVec2(ImGui::GetMainViewport()->Size.x * 2.5, ImGui::GetMainViewport()->Size.y * 2.5);

    // Setup Fonts
    ImFontConfig fontConfig;
    //fontConfig.MergeMode = true;

    ImFont* roboto = io.Fonts->AddFontFromFileTTF("Roboto-Medium.ttf", 16, &fontConfig);

    io.Fonts->AddFontDefault(&fontConfig);

    // Setup Dear ImGui style
    //ImGui::StyleColorsDark();

    ImGuiStyle& Style = ImGui::GetStyle();
    auto Color = Style.Colors;
    Style.WindowBorderSize = 0;
    Style.ChildRounding = 0;
    Style.FrameRounding = 1;
    Style.ScrollbarRounding = 0;
    Style.GrabRounding = 4;
    Style.PopupRounding = 0;
    Style.WindowRounding = 0;
    Style.WindowTitleAlign = ImVec2(0.5, 0.5);


    Color[ImGuiCol_WindowBg] = ImColor(18, 18, 18, 255);
    Color[ImGuiCol_FrameBg] = ImColor(31, 31, 31, 255);
    Color[ImGuiCol_FrameBgActive] = ImColor(41, 41, 41, 255);
    Color[ImGuiCol_FrameBgHovered] = ImColor(41, 41, 41, 255);

    Color[ImGuiCol_TitleBgCollapsed] = ImColor(41, 74, 122, 255);

    Color[ImGuiCol_Button] = ImColor(29, 29, 29, 255);
    Color[ImGuiCol_ButtonActive] = ImColor(32, 32, 32, 255);
    Color[ImGuiCol_ButtonHovered] = ImColor(36, 36, 36, 255);

    Color[ImGuiCol_Border] = ImColor(0, 0, 0, 0);
    Color[ImGuiCol_Separator] = ImColor(36, 36, 36, 255);

    Color[ImGuiCol_ResizeGrip] = ImColor(40, 40, 40, 255);
    Color[ImGuiCol_ResizeGripActive] = ImColor(156, 156, 156, 255);
    Color[ImGuiCol_ResizeGripHovered] = ImColor(255, 255, 255, 255);

    Color[ImGuiCol_ChildBg] = ImColor(26, 26, 26, 255);

    Color[ImGuiCol_ScrollbarBg] = ImColor(24, 24, 24, 255);
    Color[ImGuiCol_ScrollbarGrab] = ImColor(24, 24, 24, 255);
    Color[ImGuiCol_ScrollbarGrabActive] = ImColor(24, 24, 24, 255);
    Color[ImGuiCol_ScrollbarGrabActive] = ImColor(24, 24, 24, 255);

    Color[ImGuiCol_Header] = ImColor(39, 39, 39, 255);
    Color[ImGuiCol_HeaderActive] = ImColor(39, 39, 39, 255);
    Color[ImGuiCol_HeaderHovered] = ImColor(39, 39, 39, 255);
    Color[ImGuiCol_CheckMark] = ImColor(66, 150, 250, 255);


    // Setup Platform/Renderer backends
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX9_Init(g_pd3dDevice);
}

void Helheim::Application::Destroy()
{
    ImGui_ImplDX9_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    ::DestroyWindow(hwnd);
    ::UnregisterClass(wc.lpszClassName, wc.hInstance);

    std::cout << "Destroed" << std::endl;
}

void Helheim::Application::RenderLoop(void (*OnRender)(bool& ))
{
    // Our state
    bool show_demo_window = true;
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.0f, 0.0f, 0.0f, 1.00f);
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    // Main loop
    bool exit = true;
    while (exit)
    {
        // Poll and handle messages (inputs, window resize, etc.)
        // See the WndProc() function below for our to dispatch events to the Win32 backend.
        MSG msg;
        while (::PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                exit = false;
        }
        if (!exit)
            break;

        // Start the Dear ImGui frame
        ImGui_ImplDX9_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        OnRender(exit);

        // Rendering
        ImGui::EndFrame();
        g_pd3dDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
        g_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
        g_pd3dDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
        D3DCOLOR clear_col_dx = D3DCOLOR_RGBA((int)(clear_color.x * clear_color.w * 255.0f), (int)(clear_color.y * clear_color.w * 255.0f), (int)(clear_color.z * clear_color.w * 255.0f), (int)(clear_color.w * 255.0f));
        g_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, clear_col_dx, 1.0f, 0);
        if (g_pd3dDevice->BeginScene() >= 0)
        {
            ImGui::Render();
            ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
            g_pd3dDevice->EndScene();
        }

        // Update and Render additional Platform Windows
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
        }

        HRESULT result = g_pd3dDevice->Present(NULL, NULL, NULL, NULL);

        // Handle loss of D3D9 device
        if (result == D3DERR_DEVICELOST && g_pd3dDevice->TestCooperativeLevel() == D3DERR_DEVICENOTRESET)
            ResetDevice();
    }
    Destroy();
}

#ifndef WM_DPICHANGED
#define WM_DPICHANGED 0x02E0 // From Windows SDK 8.1+ headers
#endif