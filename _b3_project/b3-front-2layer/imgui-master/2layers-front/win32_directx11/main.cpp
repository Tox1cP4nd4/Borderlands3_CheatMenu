// Dear ImGui: standalone example application for DirectX 11

// Learn about Dear ImGui:
// - FAQ                  https://dearimgui.com/faq
// - Getting Started      https://dearimgui.com/getting-started
// - Documentation        https://dearimgui.com/docs (same as your local docs/ folder).
// - Introduction, links and more at the top of imgui.cpp

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include <d3d11.h>
#include <tchar.h>
#include <iostream>
#include "cheat_main.h"
#include "cheat_init.h"

// Data
static ID3D11Device*            g_pd3dDevice = nullptr;
static ID3D11DeviceContext*     g_pd3dDeviceContext = nullptr;
static IDXGISwapChain*          g_pSwapChain = nullptr;
static bool                     g_SwapChainOccluded = false;
static UINT                     g_ResizeWidth = 0, g_ResizeHeight = 0;
static ID3D11RenderTargetView*  g_mainRenderTargetView = nullptr;

// Forward declarations of helper functions
bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void CreateRenderTarget();
void CleanupRenderTarget();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

static void HelpMarker(const char* desc)
{
    ImGui::TextDisabled("(?)");
    if (ImGui::BeginItemTooltip())
    {
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

// Main code
int main(int, char**)
{
    // Define application window class
    WNDCLASSEXW wc = { sizeof(wc), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, L"ImGui Example", nullptr };
    ::RegisterClassExW(&wc);

    // Create the window with WS_EX_LAYERED and WS_EX_TRANSPARENT for click-through and transparency
    HWND hwnd = ::CreateWindowExW(WS_EX_LAYERED  | WS_EX_TOPMOST | WS_EX_NOACTIVATE, wc.lpszClassName, L"Dear ImGui DirectX11 Example",
        WS_POPUP, 0, 0, 1920, 1080, nullptr, nullptr, wc.hInstance, nullptr);

    // Set the window to be fully transparent (alpha = 0)
    SetLayeredWindowAttributes(hwnd, 0, 255, LWA_COLORKEY);

    // Initialize Direct3D and ImGui context as in your original code
    if (!CreateDeviceD3D(hwnd))
    {
        CleanupDeviceD3D();
        ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    // Show the window
    ::ShowWindow(hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.WantCaptureMouse = true;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

    // Initialize Cheat
    CheatHandler cHandler = initializeCheat();
    Cheat cheat(cHandler.moduleBase, cHandler.hProcess);
    std::cout << "ModuleBase: " << cHandler.moduleBase << std::endl;

    bool mod_menu = true;
    bool infinite_ammo = false;
    bool no_reload = false;
    bool fly_hack = false;
    bool god_mode = false;
    bool unlimited_armor = false;
    bool esp = false;
    float esp_color[4] = { 255.0, 0.0, 0.0, 255.0 };

    // Main loop
    bool done = false;
    while (!done)
    {
        // Poll and handle messages
        MSG msg;
        while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                done = true;
        }
        if (done)
            break;

        // Handle window being minimized or screen locked
        if (g_SwapChainOccluded && g_pSwapChain->Present(0, DXGI_PRESENT_TEST) == DXGI_STATUS_OCCLUDED)
        {
            ::Sleep(10);
            continue;
        }
        g_SwapChainOccluded = false;

        // Handle window resize (we don't resize directly in the WM_SIZE handler)
        if (g_ResizeWidth != 0 && g_ResizeHeight != 0)
        {
            CleanupRenderTarget();
            g_pSwapChain->ResizeBuffers(0, g_ResizeWidth, g_ResizeHeight, DXGI_FORMAT_UNKNOWN, 0);
            g_ResizeWidth = g_ResizeHeight = 0;
            CreateRenderTarget();
        }

        // Start ImGui frame
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();  // You still need to call NewFrame() for ImGui state, but no window will be created

        if (mod_menu) {
            //ImGui::ShowDemoWindow();
            ImVec2 initial_size(600, 400);  // Set your desired initial width and height
            ImGui::SetNextWindowSize(initial_size, ImGuiCond_FirstUseEver);  // Conditionally set it only on first use
            ImGui::Begin("ToxicPanda", nullptr, ImGuiWindowFlags_NoCollapse);

            //for (int i = 0; i < ImGuiCol_COUNT; i++)
            //{
            //    const char* name = ImGui::GetStyleColorName(i);
            //    if (!filter.PassFilter(name))
            //        continue;
            //    ImGui::PushID(i);
            //    #ifndef IMGUI_DISABLE_DEBUG_TOOLS
            //    if (ImGui::Button("?"))
            //        ImGui::DebugFlashStyleColor((ImGuiCol)i);
            //    ImGui::SetItemTooltip("Flash given color to identify places where it is used.");
            //    ImGui::SameLine();
            //    #endif
            //    ImGui::ColorEdit4("##color", (float*)&style.Colors[i], ImGuiColorEditFlags_AlphaBar | alpha_flags);
            //    if (memcmp(&style.Colors[i], &ref->Colors[i], sizeof(ImVec4)) != 0)
            //    {
            //        // Tips: in a real user application, you may want to merge and use an icon font into the main font,
            //        // so instead of "Save"/"Revert" you'd use icons!
            //        // Read the FAQ and docs/FONTS.md about using icon fonts. It's really easy and super convenient!
            //        ImGui::SameLine(0.0f, style.ItemInnerSpacing.x); if (ImGui::Button("Save")) { ref->Colors[i] = style.Colors[i]; }
            //        ImGui::SameLine(0.0f, style.ItemInnerSpacing.x); if (ImGui::Button("Revert")) { style.Colors[i] = ref->Colors[i]; }
            //    }
            //    ImGui::SameLine(0.0f, style.ItemInnerSpacing.x);
            //    ImGui::TextUnformatted(name);
            //    ImGui::PopID();
            //}
            
            //IMGUI_DEMO_MARKER("Widgets/Tabs/Basic");
            ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_NoCloseWithMiddleMouseButton;
            if (ImGui::BeginTabBar("MyTabBar", tab_bar_flags))
            {
                if (ImGui::BeginTabItem("Local Player"))
                {
                    ImGui::Checkbox("Infinite Ammo", &infinite_ammo);
                    ImGui::Checkbox("No Reload", &no_reload);
                    ImGui::Checkbox("Fly Hack", &fly_hack);
                    ImGui::SameLine(); HelpMarker("SPACE key: Go Up, SHIFT key: Go Down");
                    ImGui::Checkbox("Unlimited Armor", &unlimited_armor);
                    ImGui::Checkbox("God Mode", &god_mode);
                    ImGui::EndTabItem();
                }
                if (ImGui::BeginTabItem("Enemies"))
                {
                    ImGui::SeparatorText("Extra Sensory Perseption");
                    ImGui::Checkbox("ESP", &esp);
                    
                    ImGui::ColorEdit4("Rec Color", esp_color);
                    ImGui::SameLine(); HelpMarker(
                        "Click on the color square to open a color picker.\n"
                        "Click and hold to use drag and drop.\n"
                        "Right-click on the color square to show options.\n"
                        "CTRL+click on individual component to input value.\n");
                    ImGui::EndTabItem();
                }
                if (ImGui::BeginTabItem("Misc"))
                {
                    ImGui::Text("Miscellaneous Configs");
                    ImGui::EndTabItem();
                }
                if (ImGui::BeginTabItem("Settings"))
                {
                    ImGui::SeparatorText("Config");
                    if (ImGui::Button("Load")) {}
                    ImGui::SameLine();
                    if (ImGui::Button("Save")) {}
                    ImGui::SameLine();
                    if (ImGui::Button("Reset")) {}
                    ImGui::SeparatorText("Menu Settings");
                    ImGui::Text("Hotkey: ");
                    ImGui::Checkbox("INSERT", &esp);
                    ImGui::EndTabItem();
                }
                ImGui::EndTabBar();
            }

            ImGui::End();
            //ReleaseCapture();
        }

        if (GetAsyncKeyState(VK_INSERT) & 0x01) {  // 0x8000 means the key is pressed
            mod_menu = !mod_menu;
        }
        

        // Direct drawing using ImGui's background (no window involved)
        ImDrawList* draw_list = ImGui::GetForegroundDrawList(); // Use foreground to avoid window creation

        /* ---> CHEAT FUNCTIONS STARTS HERE <--- */
        // ESP
        if (esp) {
            Cheat::FVector* enemyCoords = cheat.ESP();
            while (enemyCoords != nullptr) {
                float distance = enemyCoords->z;
                float width_at_distance_1_p1x = 60000.0f;
                float height_at_distance_1_p1y = 40.0f;
                float width_at_distance_1 = 35000.0f;
                float height_at_distance_1 = 120000.0f;
                /*if (distance <= 0)
                    distance = 1;*/
                float p1x = (width_at_distance_1_p1x) / distance;
                float p1y = (height_at_distance_1_p1y) / distance;
                float p2x = (width_at_distance_1) / distance;
                float p2y = (height_at_distance_1) / distance;
                std::cout << "p1x: " << p1x << ", p1y: " << p1y << std::endl;
                std::cout << "p2x: " << p2x << ", p2y: " << p2y << std::endl;
                std::cout << "distance: " << distance << std::endl;
                ImVec2 p1(enemyCoords->x - p1x, enemyCoords->y);
                ImVec2 p2(enemyCoords->x + p2x, enemyCoords->y + p2y);

                // Draw rectangle
                draw_list->AddRect(p1, p2, IM_COL32(esp_color[0], esp_color[1], esp_color[2], esp_color[3]), 5.0f);  // Red outline, thickness 5
                enemyCoords = enemyCoords->next;
            }
           //Sleep(1000);
        }

        if (fly_hack) {
            cheat.flyHack();
        }

        if (unlimited_armor) {
            cheat.unlimitedArmor();
        }

        if (god_mode) {
            cheat.godMode();
        }
        
        // Render ImGui content without any window context
        ImGui::Render();
        const float clear_color_with_alpha[4] = { 0.0f, 0.0f, 0.0f, 0.0f };  // Fully transparent background
        g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);
        g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color_with_alpha);

        // Render the draw data (this will directly render your rectangle)
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        // Present the frame
        g_pSwapChain->Present(1, 0);
    }

    // Cleanup
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    ::DestroyWindow(hwnd);
    ::UnregisterClassW(wc.lpszClassName, wc.hInstance);

    return 0;
}

// Helper functions

bool CreateDeviceD3D(HWND hWnd)
{
    // Setup swap chain
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 2;
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT createDeviceFlags = 0;
    //createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
    HRESULT res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
    if (res == DXGI_ERROR_UNSUPPORTED) // Try high-performance WARP software driver if hardware is not available.
        res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
    if (res != S_OK)
        return false;

    CreateRenderTarget();
    return true;
}

void CleanupDeviceD3D()
{
    CleanupRenderTarget();
    if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = nullptr; }
    if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = nullptr; }
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = nullptr; }
}

void CreateRenderTarget()
{
    ID3D11Texture2D* pBackBuffer;
    g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_mainRenderTargetView);
    pBackBuffer->Release();
}

void CleanupRenderTarget()
{
    if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = nullptr; }
}

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Win32 message handler
// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
// Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_SIZE:
        if (wParam == SIZE_MINIMIZED)
            return 0;
        g_ResizeWidth = (UINT)LOWORD(lParam); // Queue resize
        g_ResizeHeight = (UINT)HIWORD(lParam);
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
            return 0;
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    }
    return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}
