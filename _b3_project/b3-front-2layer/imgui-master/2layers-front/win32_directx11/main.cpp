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
#include <string> // toString
#include <unordered_map>
#include <atlstr.h> // CW2A

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

std::string getConfigFilePath() {
    OPENFILENAME ofn;       // common dialog box structure
    wchar_t szFile[260];    // buffer for file name (max path length)

    // Initialize OPENFILENAME
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = NULL;  // No owner window, typically you'd set a parent window here
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile) / sizeof(wchar_t);  // Size in wide characters
    ofn.lpstrFilter = L"Config File\0*.CFG\0All files\0*.*\0";  // File filter
    ofn.nFilterIndex = 1;
    ofn.lpstrFile[0] = L'\0';  // Initialize file name to an empty string
    ofn.lpstrInitialDir = NULL;  // No initial directory
    ofn.lpstrTitle = L"Select a file";  // Dialog box title
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST; // Flags: File must exist -> OFN_FILEMUSTEXIST

    // Display the file open dialog box
    if (GetOpenFileName(&ofn) == TRUE) {
        std::wcout << L"Selected file: " << ofn.lpstrFile << std::endl;

        // Convert the wide-character file path (LPWSTR) to std::string
        int bufferSize = WideCharToMultiByte(CP_UTF8, 0, ofn.lpstrFile, -1, NULL, 0, NULL, NULL);
        std::string filePath(bufferSize, 0);  // Create a string with the required size
        WideCharToMultiByte(CP_UTF8, 0, ofn.lpstrFile, -1, &filePath[0], bufferSize, NULL, NULL);

        return filePath;  // Return the converted std::string
    }
    else {
        std::wcout << L"No file selected or dialog was cancelled." << std::endl;
        return "";  // Return an empty string in case of failure
    }
}

std::string getNewConfigFilePath() {
    OPENFILENAME ofn;       // common dialog box structure
    wchar_t szFile[260];    // buffer for file name (max path length)

    // Initialize OPENFILENAME
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = NULL;  // No owner window, typically you'd set a parent window here
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile) / sizeof(wchar_t);  // Size in wide characters
    ofn.lpstrFilter = L"Config File\0*.CFG\0All files\0*.*\0";  // File filter
    ofn.nFilterIndex = 1;
    ofn.lpstrFile[0] = L'\0';  // Initialize file name to an empty string
    ofn.lpstrInitialDir = NULL;  // No initial directory
    ofn.lpstrTitle = L"Save Configuration As";  // Dialog box title
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;  // Ensure path exists, prompt if file exists

    // Display the file save dialog box
    if (GetSaveFileName(&ofn) == TRUE) {
        std::wcout << L"Selected file: " << ofn.lpstrFile << std::endl;

        // Convert the wide-character file path (LPWSTR) to std::string
        int bufferSize = WideCharToMultiByte(CP_UTF8, 0, ofn.lpstrFile, -1, NULL, 0, NULL, NULL);
        std::string filePath(bufferSize, 0);  // Create a string with the required size
        WideCharToMultiByte(CP_UTF8, 0, ofn.lpstrFile, -1, &filePath[0], bufferSize, NULL, NULL);

        return filePath;  // Return the converted std::string
    }
    else {
        std::wcout << L"No file selected or dialog was cancelled." << std::endl;
        return "";  // Return an empty string in case of failure
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

        if (cheat.cfg_mod_menu) {
            //ImGui::ShowDemoWindow();
            ImVec2 initial_size(400, 400);  // Set your desired initial width and height
            ImGui::SetNextWindowSize(initial_size, ImGuiCond_FirstUseEver);  // Conditionally set it only on first use
            ImGui::Begin("Tox1c P4nd4", nullptr, ImGuiWindowFlags_NoCollapse);

            /*# Converter RGB(0 - 255) to ImVec4(0.0 - 1.0)
                def rgb_to_imvec4(r, g, b, a = 255) :
                return (r / 255.0, g / 255.0, b / 255.0, a / 255.0)

                # Example
                rgb = (52, 235, 70)
                imvec4_color = rgb_to_imvec4(*rgb)
                print(imvec4_color)*/


            // RGBA
            ImVec4 greenColor = ImVec4(0.0f, 0.2f, 0.0f, 1.0f);
            ImVec4 whiteColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
            ImVec4 blackColor = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
            ImVec4 lightGreenColor = ImVec4(0.20f, 0.92f, 0.27f, 1.0);

            ImVec4 tabColor = ImVec4(0.09, 0.34, 0.01, 1.0);
            ImVec4 tabActiveColor = ImVec4(0.03, 0.56, 0.13, 1.0);
            ImVec4 frameBgHoveredColor = ImVec4(0.23, 0.98, 0.00, 1.0);
            ImVec4 checkMarkColor = ImVec4(0.0, 1.0, 0.43, 1.0);
            ImVec4 sliderGrabColor = ImVec4(0.36, 0.75, 0.0, 1.0);

            ImGui::GetStyle().Colors[ImGuiCol_Text] = whiteColor;            // Text color
            ImGui::GetStyle().Colors[ImGuiCol_Button] = greenColor;          // Button color
            ImGui::GetStyle().Colors[ImGuiCol_ButtonHovered] = ImVec4(0.0f, 0.8f, 0.0f, 1.0f); // Slightly darker green when hovered
            ImGui::GetStyle().Colors[ImGuiCol_ButtonActive] = ImVec4(0.0f, 0.6f, 0.0f, 1.0f); // Even darker green when clicked
            ImGui::GetStyle().Colors[ImGuiCol_Border] = greenColor;         // Border color
            ImGui::GetStyle().Colors[ImGuiCol_SliderGrab] = greenColor;     // Slider grab color
            ImGui::GetStyle().Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.0f, 0.8f, 0.0f, 1.0f); // Slider grab active color
            ImGui::GetStyle().Colors[ImGuiCol_TitleBgActive] = greenColor;
            ImGui::GetStyle().Colors[ImGuiCol_Tab] = tabColor;
            ImGui::GetStyle().Colors[ImGuiCol_TabActive] = tabActiveColor;
            ImGui::GetStyle().Colors[ImGuiCol_TabHovered] = greenColor;
            ImGui::GetStyle().Colors[ImGuiCol_FrameBg] = greenColor;
            ImGui::GetStyle().Colors[ImGuiCol_TabHovered] = lightGreenColor;
            ImGui::GetStyle().Colors[ImGuiCol_FrameBgHovered] = frameBgHoveredColor;
            ImGui::GetStyle().Colors[ImGuiCol_CheckMark] = checkMarkColor;
            ImGui::GetStyle().Colors[ImGuiCol_SliderGrab] = sliderGrabColor;
            
            //IMGUI_DEMO_MARKER("Widgets/Tabs/Basic");
            ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_NoCloseWithMiddleMouseButton;
            if (ImGui::BeginTabBar("MyTabBar", tab_bar_flags))
            {
                if (ImGui::BeginTabItem("Local Player"))
                {
                    ImGui::SeparatorText("Health/Armor");
                    ImGui::Checkbox("Unlimited Armor", &cheat.cfg_unlimited_armor);
                    ImGui::Checkbox("God Mode", &cheat.cfg_god_mode);
                    ImGui::SeparatorText("Gun/Ammo");
                    ImGui::Checkbox("Infinite Ammo", &cheat.cfg_infinite_ammo);
                    ImGui::Checkbox("No Reload", &cheat.cfg_no_reload);
                    ImGui::EndTabItem();
                }
                if (ImGui::BeginTabItem("ESP"))
                {
                    ImGui::SeparatorText("Extra Sensory Perseption");
                    ImGui::Checkbox("ESP", &cheat.cfg_esp);
                    
                    ImGui::ColorEdit4("Rec Color", cheat.cfg_esp_color);
                    ImGui::SameLine(); HelpMarker(
                        "Click on the color square to open a color picker.\n"
                        "Click and hold to use drag and drop.\n"
                        "Right-click on the color square to show options.\n"
                        "CTRL+click on individual component to input value.\n");
                    ImGui::EndTabItem();
                }
                if (ImGui::BeginTabItem("Aimbot"))
                {
                    ImGui::SeparatorText("Aimbot");
                    ImGui::Checkbox("Aimbot", &cheat.cfg_aimbot);
                    ImGui::SliderInt("Max Distance", &cheat.aimbot_max_distance, 0, 500);
                    ImGui::SameLine(); HelpMarker("CTRL+click to input value.");
                    ImGui::Text("Hotkey:"); ImGui::SameLine(); if (ImGui::Button("LALT")) {}
                    ImGui::EndTabItem();
                }
                if (ImGui::BeginTabItem("Misc"))
                {
                    ImGui::SeparatorText("Miscellaneous Configs");
                    ImGui::Checkbox("Fly Hack", &cheat.cfg_fly_hack);
                    ImGui::SameLine(); HelpMarker("SPACE key: Go Up, SHIFT key: Go Down");
                    ImGui::SeparatorText("Money");
                    ImGui::SliderInt("##a", &cheat.cfg_add_money, 1, 10000000);
                    ImGui::SameLine(); if (ImGui::Button("Add##m")) { cheat.addMoney(cheat.cfg_add_money); }
                    ImGui::SeparatorText("Keys");
                    ImGui::Text("Normal Key");
                    ImGui::SliderInt("##b", &cheat.cfg_add_keys, 1, 10);
                    ImGui::SameLine(); if (ImGui::Button("Add##nk")) {}
                    ImGui::Text("Golden Key");
                    ImGui::SliderInt("##c", &cheat.cfg_add_golden_keys, 1, 10);
                    ImGui::SameLine(); if (ImGui::Button("Add##gk")) {}
                    ImGui::SeparatorText("Skill Points");
                    ImGui::SliderInt("##d", &cheat.cfg_add_skillPoints, 1, 30);
                    ImGui::SameLine(); if (ImGui::Button("Add##sp")) { cheat.addSkillPoints(cheat.cfg_add_skillPoints); }
                    ImGui::EndTabItem();
                }
                if (ImGui::BeginTabItem("Settings"))
                {
                    ImGui::SeparatorText("Config");
                    if (ImGui::Button("Load")) {
                       std::string cfgPath = getConfigFilePath();
                       std::unordered_map<std::string, std::string> config;

                       // Read configuration from file
                       cheat.readConfigFile(cfgPath, config);

                       //// Print out the values
                       for (const auto& pair : config) {
                           std::cout << pair.first << " = " << pair.second << std::endl;
                       }

                       // BOOL
                       cheat.cfg_infinite_ammo = (config["cfg_infinite_ammo"] == "true");
                       cheat.cfg_no_reload = (config["cfg_no_reload"] == "true");
                       cheat.cfg_aimbot = (config["cfg_aimbot"] == "true");
                       cheat.cfg_fly_hack = (config["cfg_fly_hack"] == "true");
                       cheat.cfg_god_mode = (config["cfg_god_mode"] == "true");
                       cheat.cfg_unlimited_armor = (config["cfg_unlimited_armor"] == "true");
                       cheat.cfg_esp = (config["cfg_esp"] == "true");

                       // INT
                       cheat.cfg_add_money = (std::stoi(config["cfg_add_money"]));
                       cheat.cfg_add_golden_keys = (std::stoi(config["cfg_add_golden_keys"]));
                       cheat.cfg_add_keys = (std::stoi(config["cfg_add_keys"]));
                       cheat.cfg_add_skillPoints = (std::stoi(config["cfg_add_skillPoints"]));
                       cheat.aimbot_max_distance = (std::stoi(config["aimbot_max_distance"]));

                       //// FLOAT
                       cheat.cfg_esp_color[0] = (std::stof(config["cfg_esp_color_R"]));
                       cheat.cfg_esp_color[1] = (std::stof(config["cfg_esp_color_G"]));
                       cheat.cfg_esp_color[2] = (std::stof(config["cfg_esp_color_B"]));
                       cheat.cfg_esp_color[3] = (std::stof(config["cfg_esp_color_A"]));

                    }
                    ImGui::SameLine();
                    if (ImGui::Button("Save")) {
                        std::string cfgPath = getNewConfigFilePath();
                        cheat.writeConfigFile(cfgPath);
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("Reset")) {
                        cheat.resetConfig();
                    }
                    ImGui::SeparatorText("Menu Settings");
                    ImGui::Text("Hotkey: ");
                    ImGui::Checkbox("INSERT", &cheat.cfg_esp);
                    ImGui::EndTabItem();
                }
                ImGui::EndTabBar();
            }

            ImGui::End();
            //ReleaseCapture();
        }

        if (GetAsyncKeyState(VK_INSERT) & 0x01) {  // 0x8000 means the key is pressed
            cheat.cfg_mod_menu = !cheat.cfg_mod_menu;
        }
        

        // Direct drawing using ImGui's background (no window involved)
        ImDrawList* draw_list = ImGui::GetForegroundDrawList(); // Use foreground to avoid window creation

        /* ---> CHEAT FUNCTIONS STARTS HERE <--- */
        if (cheat.cfg_esp) { // ESP or Aimbot = loop through Entity List
            Cheat::Entity* entityArr = cheat.ESP();
            while (entityArr != nullptr) {
                float distance = entityArr->z/91; // Adjusting distance so that when we`re very close its set to 1

                // Manually adjusting ESP at different distances:
                float p1x = 600.0f / distance;
                float p1y = 10.0f / distance;
                float p2x = 350.0f / distance;
                float p2y = 1600.0f / distance;
                /*std::cout << "p1x: " << p1x << ", p1y: " << p1y << std::endl;
                std::cout << "p2x: " << p2x << ", p2y: " << p2y << std::endl;
                std::cout << "distance: " << distance << std::endl;*/
                ImVec2 p1_esp(entityArr->x - p1x, entityArr->y);
                ImVec2 p2_esp(entityArr->x + p2x, entityArr->y + p2y);

                // 3D ESP
                float Const_x = 85;
                float Const_y = 40;
                float CYb = 2;
                ImVec2 p3_esp(p2_esp.x + Const_x, p2_esp.y - Const_y);
                ImVec2 p4_esp(p2_esp.x - Const_x, p3_esp.y - CYb);
                ImVec2 p5_esp(p4_esp.x - Const_x, p4_esp.y + (Const_y + CYb));
                ImVec2 p6_esp(p3_esp.x, p3_esp.y - (p2_esp.y - p1_esp.y));
                ImVec2 p7_esp(p4_esp.x, p6_esp.y - CYb);
                ImVec2 p8_esp(p2_esp.x, p1_esp.y + CYb);

                int enemyHp = (int)entityArr->health;

                float hpBarIncreaseVar = 16.0f / distance;
                ImVec2 p1_health(entityArr->x - p1x - 7, entityArr->y + p2y - (enemyHp * hpBarIncreaseVar)); // Quanto maior o HP, maior a barra de vida do ESP
                ImVec2 p2_health(entityArr->x - p1x - 3, entityArr->y + p2y);

                // If alive, draw rectangle
                ImU32 healthColor = 0xFF00FF00; // Green
                if (enemyHp > 0) {
                    if (enemyHp < 30) { healthColor = 0xFF0000FF; } // Red
                    else if (enemyHp < 60) { healthColor = 0xFF00A5FF; } // Orange

                    float esp_thickness = 2.0f;

                    // ESP BOX
                    draw_list->AddRect(p1_esp, p2_esp, IM_COL32(cheat.cfg_esp_color[0], cheat.cfg_esp_color[1], cheat.cfg_esp_color[2], cheat.cfg_esp_color[3]), esp_thickness);  // Red outline, thickness 5

                    // 3D ESP
                    /*draw_list->AddLine(p2_esp, p3_esp, IM_COL32(cheat.cfg_esp_color[0], cheat.cfg_esp_color[1], cheat.cfg_esp_color[2], cheat.cfg_esp_color[3]), esp_thickness);
                    draw_list->AddLine(p3_esp, p4_esp, IM_COL32(cheat.cfg_esp_color[0], cheat.cfg_esp_color[1], cheat.cfg_esp_color[2], cheat.cfg_esp_color[3]), esp_thickness);
                    draw_list->AddLine(p4_esp, p5_esp, IM_COL32(cheat.cfg_esp_color[0], cheat.cfg_esp_color[1], cheat.cfg_esp_color[2], cheat.cfg_esp_color[3]), esp_thickness);
                    draw_list->AddLine(p6_esp, p3_esp, IM_COL32(cheat.cfg_esp_color[0], cheat.cfg_esp_color[1], cheat.cfg_esp_color[2], cheat.cfg_esp_color[3]), esp_thickness);
                    draw_list->AddLine(p6_esp, p7_esp, IM_COL32(cheat.cfg_esp_color[0], cheat.cfg_esp_color[1], cheat.cfg_esp_color[2], cheat.cfg_esp_color[3]), esp_thickness);
                    draw_list->AddLine(p7_esp, p1_esp, IM_COL32(cheat.cfg_esp_color[0], cheat.cfg_esp_color[1], cheat.cfg_esp_color[2], cheat.cfg_esp_color[3]), esp_thickness);
                    draw_list->AddLine(p7_esp, p4_esp, IM_COL32(cheat.cfg_esp_color[0], cheat.cfg_esp_color[1], cheat.cfg_esp_color[2], cheat.cfg_esp_color[3]), esp_thickness);
                    draw_list->AddLine(p8_esp, p6_esp, IM_COL32(cheat.cfg_esp_color[0], cheat.cfg_esp_color[1], cheat.cfg_esp_color[2], cheat.cfg_esp_color[3]), esp_thickness);*/


                    draw_list->AddText(ImVec2((float)p1_esp.x, (float)p1_esp.y - 20), 0xFFFFFFFF, "Health: ");
                    draw_list->AddText(ImVec2((float)p1_esp.x + 55, (float)p1_esp.y - 20), healthColor, (std::to_string(enemyHp) + "%").c_str());
                    draw_list->AddText(ImVec2((float)p1_esp.x, (float)p1_esp.y - 33), 0xFFFFFFFF, "Distance: ");
                    draw_list->AddText(ImVec2((float)p1_esp.x + 65, (float)p1_esp.y - 33), 0xFFFFFFFF, (std::to_string((int)distance)).c_str());
                    draw_list->AddText(ImVec2((float)p1_esp.x, (float)p1_esp.y - 46), 0xFFFFFFFF, "Level: ");
                    //draw_list->AddRect(p1_health, p2_health, healthColor, 5.0f);
                    draw_list->AddRectFilled(p1_health, p2_health, healthColor, 5.0f);
                }
                entityArr = entityArr->next;
            }
             //Sleep(1000);
        }

        if (cheat.cfg_aimbot && (GetAsyncKeyState(VK_LMENU) & 0x8000)) {
            cheat.Aimbot();
        }

        if (cheat.cfg_fly_hack) {
            cheat.flyHack();
        }

        if (cheat.cfg_infinite_ammo) {
            cheat.infAMmo();
        }

        if (cheat.cfg_unlimited_armor) {
            //cheat.unlimitedArmor();
        }

        if (cheat.cfg_god_mode) {
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
