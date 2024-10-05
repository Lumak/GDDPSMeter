#include <iostream>
#include <windows.h>
#include <dwmapi.h>
#include <d3d9.h>
#include <cctype>
#include "imgui/imgui.h"
#include "imgui/imgui_impl_dx9.h"
#include "imgui/imgui_impl_win32.h"
#include "proc.h"

#include "MainImgui.h"
#include "IPCMessage.h"
#include "CombatLog.h"
#include "MainUIController.h"
#include "Texture.h"
#include "Logger.h"

#pragma comment(lib,"d3d9.lib")

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
static bool menuShow = false;
HWND _HWND = NULL;
DWORD threadId = 0;

int ScreenHeight = NULL;
int ScreenWidth = NULL;
int ScreenLeft = NULL;
int ScreenRight = NULL;
int ScreenTop = NULL;
int ScreenBottom = NULL;
bool running = true;
ImFont* font = NULL;

//#define USE_NOTEPAD
#ifdef USE_NOTEPAD
#define WIN_WNAME L"Notepad"
#define EXE_WNAME L"Notepad.exe"
#else
#define WIN_WNAME L"Grim Dawn"
#define EXE_WNAME L"Grim Dawn.exe"
#endif

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
static MainUIController optionsWin;

int IsCtrlKey(WPARAM key);
static ImGuiKey ImGui_ImplWin32_VirtualKeyToImGuiKey(WPARAM wParam);

namespace OverlayWindow
{
	WNDCLASSEX WindowClass;
	HWND Hwnd;
	LPCSTR Name;
}

namespace DirectX9Interface
{
    IDirect3D9Ex* Direct3D9 = NULL;
    IDirect3DDevice9Ex* pDevice = NULL;
    D3DPRESENT_PARAMETERS pParams = { NULL };
    MARGINS Margin = { -1 };
    MSG Message = { NULL };
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void InputHandler()
{
    for (int i = 0; i < 5; i++) ImGui::GetIO().MouseDown[i] = false;
    int button = -1;
    if (GetAsyncKeyState(VK_LBUTTON)) button = 0;
    if (button != -1) ImGui::GetIO().MouseDown[button] = true;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void Render()
{
    ImGui_ImplDX9_NewFrame();
    ImGui_ImplWin32_NewFrame();

    ImGui::NewFrame();
    if (menuShow)
    {
        InputHandler();

        optionsWin.ShowWin(DirectX9Interface::pDevice);

        // clear keys (prevents repeated inputs)
        ImGuiIO& io = ImGui::GetIO();
        for (int i = 0; i < IM_ARRAYSIZE(io.KeysData); i++)
        {
            ImGuiKeyData* key_data = &io.KeysData[i];
            key_data->Down = 0;
        }
    }

	 ImGui::EndFrame();

	 DirectX9Interface::pDevice->SetRenderState(D3DRS_ZENABLE, false);
	 DirectX9Interface::pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, false);
	 DirectX9Interface::pDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, false);

	 DirectX9Interface::pDevice->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_ARGB(0, 0, 0, 0), 1.0f, 0);
	 
    if (DirectX9Interface::pDevice->BeginScene() >= 0) 
    {
		  ImGui::Render();
		  ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
		  DirectX9Interface::pDevice->EndScene();
    }

    HRESULT result = DirectX9Interface::pDevice->Present(NULL, NULL, NULL, NULL);
	if (result == D3DERR_DEVICELOST && DirectX9Interface::pDevice->TestCooperativeLevel() == D3DERR_DEVICENOTRESET) 
    {
		  ImGui_ImplDX9_InvalidateDeviceObjects();
		  DirectX9Interface::pDevice->Reset(&DirectX9Interface::pParams);
		  ImGui_ImplDX9_CreateDeviceObjects();
  	}
}


//============================================================================
//============================================================================
void MainLoop()
{
    static RECT OldRect;
    ZeroMemory(&DirectX9Interface::Message, sizeof(MSG));
    LOGF("---main loop start");

    while (running && DirectX9Interface::Message.message != WM_QUIT) 
    {
		Sleep(10);

		// process queued msgs
		optionsWin.QueryMessages(menuShow);

		if (GetProcId(EXE_WNAME) == 0)
		{
		    running = false;
		    break;
		}

		if (PeekMessage(&DirectX9Interface::Message, OverlayWindow::Hwnd, 0, 0, PM_REMOVE))
		{
		    TranslateMessage(&DirectX9Interface::Message);
		    DispatchMessage(&DirectX9Interface::Message);
		}

		// refresh win pos
		HWND ForegroundWindow = GetForegroundWindow();
		if (ForegroundWindow == _HWND) 
		{
			HWND TempProcessHwnd = GetWindow(ForegroundWindow, GW_HWNDPREV);
			SetWindowPos(OverlayWindow::Hwnd, TempProcessHwnd, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		}

		RECT TempRect;
		POINT TempPoint;
		ZeroMemory(&TempRect, sizeof(RECT));
		ZeroMemory(&TempPoint, sizeof(POINT));

		GetClientRect(_HWND, &TempRect);
		ClientToScreen(_HWND, &TempPoint);

		TempRect.left = TempPoint.x;
		TempRect.top = TempPoint.y;
		ImGuiIO& io = ImGui::GetIO();
		io.ImeWindowHandle = _HWND;

		POINT TempPoint2;
		GetCursorPos(&TempPoint2);
		io.MousePos.x = (float)(TempPoint2.x - TempPoint.x);
		io.MousePos.y = (float)(TempPoint2.y - TempPoint.y);

		if (GetAsyncKeyState(0x1)) 
		{
			io.MouseDown[0] = true;
			io.MouseClicked[0] = true;
			//io.MouseClickedPos[0].x = io.MousePos.x;
			//io.MouseClickedPos[0].x = io.MousePos.y;
		}
		else 
		{
			io.MouseDown[0] = false;
		}

		if (TempRect.left != OldRect.left || TempRect.right != OldRect.right || TempRect.top != OldRect.top || TempRect.bottom != OldRect.bottom) 
		{
            OldRect = TempRect;
		    ScreenWidth = TempRect.right;
		    ScreenHeight = TempRect.bottom;
		    DirectX9Interface::pParams.BackBufferWidth = ScreenWidth;
		    DirectX9Interface::pParams.BackBufferHeight = ScreenHeight;
		    SetWindowPos(OverlayWindow::Hwnd, (HWND)0, TempPoint.x, TempPoint.y, ScreenWidth, ScreenHeight, SWP_NOREDRAW);
		    DirectX9Interface::pDevice->Reset(&DirectX9Interface::pParams);
		}

        Render();
	}

    //clean up
    ImGui_ImplDX9_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
  
	if (DirectX9Interface::pDevice != NULL) 
	{
	    DirectX9Interface::pDevice->EndScene();
	    DirectX9Interface::pDevice->Release();
	}
  
	if (DirectX9Interface::Direct3D9 != NULL) 
	{
	    DirectX9Interface::Direct3D9->Release();
	}

	DestroyWindow(OverlayWindow::Hwnd);
	UnregisterClass(OverlayWindow::WindowClass.lpszClassName, OverlayWindow::WindowClass.hInstance);
}

//============================================================================
 //============================================================================
bool DirectXInit() 
{
    if (FAILED(Direct3DCreate9Ex(D3D_SDK_VERSION, &DirectX9Interface::Direct3D9))) 
    {
        LOGF("DirectXInit() FAILED");
        return false;
    }

    D3DPRESENT_PARAMETERS Params = { 0 };
    Params.Windowed = TRUE;
    Params.SwapEffect = D3DSWAPEFFECT_DISCARD;
    Params.hDeviceWindow = OverlayWindow::Hwnd;
    Params.MultiSampleQuality = D3DMULTISAMPLE_NONE;
    Params.BackBufferFormat = D3DFMT_A8R8G8B8;
    Params.BackBufferWidth = ScreenWidth;
    Params.BackBufferHeight =ScreenHeight;
    Params.PresentationInterval = D3DPRESENT_INTERVAL_ONE;
    Params.EnableAutoDepthStencil = TRUE;
    Params.AutoDepthStencilFormat = D3DFMT_D16;
    Params.PresentationInterval = D3DPRESENT_INTERVAL_ONE;
    Params.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;

    HRESULT result = DirectX9Interface::Direct3D9->CreateDeviceEx(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, OverlayWindow::Hwnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &Params, 0, &DirectX9Interface::pDevice);
    if (FAILED(result))
    {
        DirectX9Interface::Direct3D9->Release();
        return false;
    }

    //setup imgui
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.WantCaptureMouse = true; 
    io.WantTextInput = false;
    io.WantCaptureKeyboard = true;
    io.MouseDrawCursor = false;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui_ImplWin32_Init(OverlayWindow::Hwnd);
    ImGui_ImplDX9_Init(DirectX9Interface::pDevice);
    DirectX9Interface::Direct3D9->Release();

    return true;
}

//============================================================================
//============================================================================
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
#if 1
LRESULT WndKeyProcHandlerMod(HWND hwnd, WPARAM wParam, LPARAM lParam, bool mouse)
{
  //essentially, we want imgui to process the keys
  //IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
  //input to this fn:
  // wParam -> msg : ImGui_ImplWin32_WndProcHandler
  // lParam -> wParam : ''
  
  // keys should reach 
  //   InputEventsQueue
#if 0
  KBDLLHOOKSTRUCT* keyboardInfo = reinterpret_cast<KBDLLHOOKSTRUCT*>(lParam);

  return ImGui_ImplWin32_WndProcHandler(hwnd, (UINT)wParam, (WPARAM)keyboardInfo->vkCode, keyboardInfo->scanCode);
#else
  if (mouse)
  {
    ImGui_ImplWin32_WndProcHandler(hwnd, (UINT)wParam, (WPARAM)lParam, 0);
    return 0;
  }
  if (ImGui::GetCurrentContext() == NULL)
  {
    LOGF("current context null\n");
    return 0;
  }

  ImGuiIO& io = ImGui::GetIO();

  if (!mouse)
  {
    KBDLLHOOKSTRUCT* keyboardInfo = reinterpret_cast<KBDLLHOOKSTRUCT*>(lParam);
    unsigned int vkey = keyboardInfo->vkCode;

    switch (wParam) {
    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
      //if (keyboardInfo->vkCode < 512)
        //io.KeysDown[keyboardInfo->vkCode] = true;
      {
      //LOGF("key down: %d\n", vkey);

        if (IsCtrlKey(vkey))
        {
          ImGuiKey imKey = ImGui_ImplWin32_VirtualKeyToImGuiKey(vkey);
          if (imKey == ImGuiKey_Escape)
          {
            LOGF("escape key hit\n");

          }
          io.AddKeyAnalogEvent(imKey, true, 0.0f);
          if (vkey == VK_LSHIFT || vkey == VK_RSHIFT)
          {
            io.KeyShift = true;
          }
        }
        else
        {
          GetKeyState(keyboardInfo->vkCode);
          BYTE keys[256];
          if (GetKeyboardState(keys))
          {
            WCHAR keyPressed[10];
            HKL keyboardLayout = GetKeyboardLayout(NULL);

            if (ToUnicodeEx(keyboardInfo->vkCode, keyboardInfo->scanCode, (BYTE*)keys, keyPressed, 10, 2, keyboardLayout))
            {
              for (int i = 0; i < 10; i++)
              {
                if (keyPressed[i] > 0 && keyPressed[i] < 0x10000)
                  io.AddInputCharacterUTF16((unsigned short)keyPressed[i]);
              }

            }
          }
        }

      }
      break;
    case WM_KEYUP:
    case WM_SYSKEYUP:
      //if (keyboardInfo->vkCode < 512)
      //  io.KeysDown[keyboardInfo->vkCode] = 0;
    {
      if (vkey == VK_LSHIFT || vkey == VK_RSHIFT)
      {
        io.KeyShift = false;
      }

    }
      break;
    }
    return 0;
  }
  else
  {
    switch (wParam)
    {
    case WM_LBUTTONDOWN: case WM_LBUTTONDBLCLK:
    case WM_RBUTTONDOWN: case WM_RBUTTONDBLCLK:
    case WM_MBUTTONDOWN: case WM_MBUTTONDBLCLK:
    case WM_XBUTTONDOWN: case WM_XBUTTONDBLCLK:
    {
      int button = 0;
      if (wParam == WM_LBUTTONDOWN || wParam == WM_LBUTTONDBLCLK) { button = 0; }
      if (wParam == WM_RBUTTONDOWN || wParam == WM_RBUTTONDBLCLK) { button = 1; }
      if (wParam == WM_MBUTTONDOWN || wParam == WM_MBUTTONDBLCLK) { button = 2; }
      if (wParam == WM_XBUTTONDOWN || wParam == WM_XBUTTONDBLCLK) { button = (GET_XBUTTON_WPARAM(reinterpret_cast<MSLLHOOKSTRUCT*>(lParam)->mouseData) == XBUTTON1) ? 3 : 4; }
      if (!ImGui::IsAnyMouseDown() && ::GetCapture() == NULL)
        ::SetCapture(hwnd);
      io.MouseDown[button] = true;
      return 0;
    }
    case WM_LBUTTONUP:
    case WM_RBUTTONUP:
    case WM_MBUTTONUP:
    case WM_XBUTTONUP:
    {
      int button = 0;
      if (wParam == WM_LBUTTONUP) { button = 0; }
      if (wParam == WM_RBUTTONUP) { button = 1; }
      if (wParam == WM_MBUTTONUP) { button = 2; }
      if (wParam == WM_XBUTTONUP) { button = (GET_XBUTTON_WPARAM(lParam) == XBUTTON1) ? 3 : 4; }
      io.MouseDown[button] = false;
      if (!ImGui::IsAnyMouseDown() && ::GetCapture() == hwnd)
        ::ReleaseCapture();
      return 0;
    }
    case WM_MOUSEWHEEL:
    {
      MSLLHOOKSTRUCT* info = reinterpret_cast<MSLLHOOKSTRUCT*>(lParam);
      io.MouseWheel += (float)GET_WHEEL_DELTA_WPARAM(info->mouseData) / (float)WHEEL_DELTA;
      return 0;
    }
    case WM_MOUSEHWHEEL:
    {
      MSLLHOOKSTRUCT* info = reinterpret_cast<MSLLHOOKSTRUCT*>(lParam);
      io.MouseWheelH += (float)GET_WHEEL_DELTA_WPARAM(info->mouseData) / (float)WHEEL_DELTA;
      return 0;
    }
    }
  }
#endif
  return 0;
}

//============================================================================
 //============================================================================
LRESULT CALLBACK MouseHookProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode != HC_ACTION)  // Nothing to do
        return CallNextHookEx(NULL, nCode, wParam, lParam);

    //printf("%X, %X, %X\n", nCode, wParam, lParam);
    WndKeyProcHandlerMod(OverlayWindow::Hwnd, wParam, lParam, true);

    return DefWindowProc(NULL, nCode, wParam, lParam);
}

LRESULT CALLBACK KeyboardHookProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    //LOGF("keyproc code=%d, w=%u, l=%u\n", nCode, wParam, lParam);
    if (nCode != HC_ACTION)  // Nothing to do
        return CallNextHookEx(NULL, nCode, wParam, lParam);

    WndKeyProcHandlerMod(OverlayWindow::Hwnd, wParam, lParam, false);

    return DefWindowProc(NULL, nCode, wParam, lParam);
}

//#define OVERRIDE_MOUSE

HHOOK mhook, khook;
//============================================================================
 //============================================================================
void HookMouse()
{
#ifdef OVERRIDE_MOUSE
  mhook = SetWindowsHookExA(WH_MOUSE_LL, MouseHookProc, nullptr, 0); //hook mouse
#endif

  //HMODULE hmod = GetModuleHandleA("DPSMeter.dll");
  khook = SetWindowsHookExA(WH_KEYBOARD_LL, KeyboardHookProc, NULL, 0); //hook keyboard
  if (!khook)
  {
    //LOGF("  err=0x%X\n", GetLastError());
  }
  else
  {
    //LOGF("  set\n");
  }
}
#endif

//============================================================================
 //============================================================================
LRESULT CALLBACK WinProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
#ifndef OVERRIDE_MOUSE
    if (ImGui_ImplWin32_WndProcHandler(hWnd, Message, wParam, lParam))
        return 0;
#endif

    switch (Message) 
    {
    case WM_DESTROY:
#if 0
   if (DirectX9Interface::pDevice != NULL) {
        DirectX9Interface::pDevice->EndScene();
        DirectX9Interface::pDevice->Release();
    }
    if (DirectX9Interface::Direct3D9 != NULL) {
        DirectX9Interface::Direct3D9->Release();
    }
    PostQuitMessage(0);
#endif
    //exit(4);
        if (hWnd == _HWND)
        {
            running = false;
        }
        break;
 
    case WM_QUIT:
#if 0
        if (DirectX9Interface::pDevice != NULL) {
        DirectX9Interface::pDevice->EndScene();
        DirectX9Interface::pDevice->Release();
        }
        if (DirectX9Interface::Direct3D9 != NULL) {
        DirectX9Interface::Direct3D9->Release();
        }
        PostQuitMessage(0);
        //exit(4);
#endif
        if (hWnd == _HWND)
        {
            running = false;
        }
        break;

    case WM_SIZE:
        if (DirectX9Interface::pDevice != NULL && wParam != SIZE_MINIMIZED) {
            ImGui_ImplDX9_InvalidateDeviceObjects();
            DirectX9Interface::pParams.BackBufferWidth = LOWORD(lParam);
            DirectX9Interface::pParams.BackBufferHeight = HIWORD(lParam);
            HRESULT hr = DirectX9Interface::pDevice->Reset(&DirectX9Interface::pParams);
            if (hr == D3DERR_INVALIDCALL)
            {
				//IM_ASSERT(0);
                break;
            }
            ImGui_ImplDX9_CreateDeviceObjects();
        }
        break;
    }
    return DefWindowProc(hWnd, Message, wParam, lParam);
}

//============================================================================
//============================================================================
void SetupWindow()
{
	OverlayWindow::WindowClass = 
	{
		sizeof(WNDCLASSEX), 
		0,
		WinProc, 
		0, 
		0, 
		NULL,
		LoadIcon(nullptr, IDI_APPLICATION), 
		LoadCursor(nullptr, IDC_ARROW), 
		nullptr, 
		nullptr, 
		OverlayWindow::Name, 
		LoadIcon(nullptr, IDI_APPLICATION)
	};

	if (!RegisterClassEx(&OverlayWindow::WindowClass))
	{
		DWORD dwError = GetLastError();
		printf("error=%X\n", dwError);
	}

	if (_HWND)
	{
		static RECT TempRect = { NULL };
		static POINT TempPoint;

        GetClientRect(_HWND, &TempRect);
        ClientToScreen(_HWND, &TempPoint);

        ScreenWidth = TempRect.right - TempRect.left;
        ScreenHeight = TempRect.bottom - TempRect.top;

        ScreenLeft = TempPoint.x;
        ScreenRight = TempPoint.y;
        ScreenTop = TempRect.top;
        ScreenBottom = TempRect.bottom;
    }
    LOGF("SetupWindow: l,t,w,h=%d, %d, %d, %d", ScreenLeft, ScreenTop, ScreenWidth, ScreenHeight);

	OverlayWindow::Hwnd = CreateWindowEx(NULL, OverlayWindow::Name, OverlayWindow::Name, WS_POPUP | WS_VISIBLE, ScreenLeft, ScreenTop, ScreenWidth, ScreenHeight, NULL, NULL, 0, NULL);
	DwmExtendFrameIntoClientArea(OverlayWindow::Hwnd, &DirectX9Interface::Margin);
	SetWindowLong(OverlayWindow::Hwnd, GWL_EXSTYLE, WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOOLWINDOW);
	ShowWindow(OverlayWindow::Hwnd, SW_SHOW);
	UpdateWindow(OverlayWindow::Hwnd);
	ImGui::CreateContext();
	optionsWin.SetFonts();
}

static HWND hwndGame = NULL;
void ImGuiMain::SetHwnWindow(void *hwnd)
{
    hwndGame = (HWND)hwnd;
}

//============================================================================
//============================================================================
void ImGuiMain::ImGuiStartup()
{
    DWORD procId = GetProcId(EXE_WNAME);

    LOGF("-- ImGuiStartup --");

    if (procId == 0)
    {
        LOGF("  procId not found, exit");
        return;
    }


#ifndef USE_INGAME_HWND
    DWORD ForegroundWindowProcessID;
    bool WindowFocus = false;
    RECT rect;

    while (WindowFocus == false)
    {
        GetWindowThreadProcessId(GetForegroundWindow(), &ForegroundWindowProcessID);

        if (ForegroundWindowProcessID == procId)
        {
            _HWND = GetForegroundWindow();
            threadId = GetWindowThreadProcessId(_HWND, NULL);
            //LOGF("grim thread id=%d(%X)\n", threadId, threadId);
            GetWindowRect(_HWND, &rect);
            ScreenWidth = rect.right - rect.left;
            ScreenHeight = rect.bottom - rect.top;
            ScreenLeft = rect.left;
            ScreenRight = rect.right;
            ScreenTop = rect.top;
            ScreenBottom = rect.bottom;

            RECT clrect;
            GetClientRect(_HWND, &clrect);
            if (clrect.right > 0 && clrect.bottom > 0)
            {
                WindowFocus = true;
            }
        }
    }
    LOGF("  hwnd=0x%X, l,t,r,b(%d,%d,%d,%d)", _HWND, rect.left, rect.top, rect.right, rect.bottom);

#else
    RECT rect;
    while (!hwndGame)
    {
        // process queued msgs
		optionsWin.QueryMessages(menuShow);

        Sleep(10);
    }

    //GetWindowRect(hwnGame, &rect);
    _HWND = hwndGame;
    GetWindowRect(_HWND, &rect);
    ScreenWidth = rect.right - rect.left;
    ScreenHeight = rect.bottom - rect.top;
    ScreenLeft = rect.left;
    ScreenRight = rect.right;
    ScreenTop = rect.top;
    ScreenBottom = rect.bottom;
    LOGF("  hwnd=0x%X, l,t,r,b(%d,%d,%d,%d)", hwndGame, rect.left, rect.top, rect.right, rect.bottom);

#endif

    HookMouse();
    OverlayWindow::Name = "overlay";
    SetupWindow();
    DirectXInit();

    MainLoop();
}

// Map VK_xxx to ImGuiKey_xxx.
int IsCtrlKey(WPARAM key)
{
  switch (key)
  {
  case VK_TAB: return ImGuiKey_Tab;
  case VK_LEFT: return ImGuiKey_LeftArrow;
  case VK_RIGHT: return ImGuiKey_RightArrow;
  case VK_UP: return ImGuiKey_UpArrow;
  case VK_DOWN: return ImGuiKey_DownArrow;
  case VK_PRIOR: return ImGuiKey_PageUp;
  case VK_NEXT: return ImGuiKey_PageDown;
  case VK_HOME: return ImGuiKey_Home;
  case VK_END: return ImGuiKey_End;
  case VK_INSERT: return ImGuiKey_Insert;
  case VK_DELETE: return ImGuiKey_Delete;
  case VK_BACK: return ImGuiKey_Backspace;
  case VK_LSHIFT: return ImGuiKey_LeftShift;
  case VK_LCONTROL: return ImGuiKey_LeftCtrl;
  case VK_LMENU: return ImGuiKey_LeftAlt;
  case VK_LWIN: return ImGuiKey_LeftSuper;
  case VK_RSHIFT: return ImGuiKey_RightShift;
  case VK_RCONTROL: return ImGuiKey_RightCtrl;
  case VK_F1: return ImGuiKey_F1;
  case VK_F2: return ImGuiKey_F2;
  case VK_F3: return ImGuiKey_F3;
  case VK_F4: return ImGuiKey_F4;
  case VK_F5: return ImGuiKey_F5;
  case VK_F6: return ImGuiKey_F6;
  case VK_F7: return ImGuiKey_F7;
  case VK_F8: return ImGuiKey_F8;
  case VK_F9: return ImGuiKey_F9;
  case VK_F10: return ImGuiKey_F10;
  case VK_F11: return ImGuiKey_F11;
  case VK_F12: return ImGuiKey_F12;
  case VK_ESCAPE: return ImGuiKey_Escape;

  }
  return 0;
}

static ImGuiKey ImGui_ImplWin32_VirtualKeyToImGuiKey(WPARAM wParam)
{
  switch (wParam)
  {
  case VK_TAB: return ImGuiKey_Tab;
  case VK_LEFT: return ImGuiKey_LeftArrow;
  case VK_RIGHT: return ImGuiKey_RightArrow;
  case VK_UP: return ImGuiKey_UpArrow;
  case VK_DOWN: return ImGuiKey_DownArrow;
  case VK_PRIOR: return ImGuiKey_PageUp;
  case VK_NEXT: return ImGuiKey_PageDown;
  case VK_HOME: return ImGuiKey_Home;
  case VK_END: return ImGuiKey_End;
  case VK_INSERT: return ImGuiKey_Insert;
  case VK_DELETE: return ImGuiKey_Delete;
  case VK_BACK: return ImGuiKey_Backspace;
  case VK_SPACE: return ImGuiKey_Space;
  case VK_RETURN: return ImGuiKey_Enter;
  case VK_ESCAPE: return ImGuiKey_Escape;
  case VK_OEM_7: return ImGuiKey_Apostrophe;
  case VK_OEM_COMMA: return ImGuiKey_Comma;
  case VK_OEM_MINUS: return ImGuiKey_Minus;
  case VK_OEM_PERIOD: return ImGuiKey_Period;
  case VK_OEM_2: return ImGuiKey_Slash;
  case VK_OEM_1: return ImGuiKey_Semicolon;
  case VK_OEM_PLUS: return ImGuiKey_Equal;
  case VK_OEM_4: return ImGuiKey_LeftBracket;
  case VK_OEM_5: return ImGuiKey_Backslash;
  case VK_OEM_6: return ImGuiKey_RightBracket;
  case VK_OEM_3: return ImGuiKey_GraveAccent;
  case VK_CAPITAL: return ImGuiKey_CapsLock;
  case VK_SCROLL: return ImGuiKey_ScrollLock;
  case VK_NUMLOCK: return ImGuiKey_NumLock;
  case VK_SNAPSHOT: return ImGuiKey_PrintScreen;
  case VK_PAUSE: return ImGuiKey_Pause;
  case VK_NUMPAD0: return ImGuiKey_Keypad0;
  case VK_NUMPAD1: return ImGuiKey_Keypad1;
  case VK_NUMPAD2: return ImGuiKey_Keypad2;
  case VK_NUMPAD3: return ImGuiKey_Keypad3;
  case VK_NUMPAD4: return ImGuiKey_Keypad4;
  case VK_NUMPAD5: return ImGuiKey_Keypad5;
  case VK_NUMPAD6: return ImGuiKey_Keypad6;
  case VK_NUMPAD7: return ImGuiKey_Keypad7;
  case VK_NUMPAD8: return ImGuiKey_Keypad8;
  case VK_NUMPAD9: return ImGuiKey_Keypad9;
  case VK_DECIMAL: return ImGuiKey_KeypadDecimal;
  case VK_DIVIDE: return ImGuiKey_KeypadDivide;
  case VK_MULTIPLY: return ImGuiKey_KeypadMultiply;
  case VK_SUBTRACT: return ImGuiKey_KeypadSubtract;
  case VK_ADD: return ImGuiKey_KeypadAdd;
  //case IM_VK_KEYPAD_ENTER: return ImGuiKey_KeypadEnter;
  case VK_LSHIFT: return ImGuiKey_LeftShift;
  case VK_LCONTROL: return ImGuiKey_LeftCtrl;
  case VK_LMENU: return ImGuiKey_LeftAlt;
  case VK_LWIN: return ImGuiKey_LeftSuper;
  case VK_RSHIFT: return ImGuiKey_RightShift;
  case VK_RCONTROL: return ImGuiKey_RightCtrl;
  case VK_RMENU: return ImGuiKey_RightAlt;
  case VK_RWIN: return ImGuiKey_RightSuper;
  case VK_APPS: return ImGuiKey_Menu;
  case '0': return ImGuiKey_0;
  case '1': return ImGuiKey_1;
  case '2': return ImGuiKey_2;
  case '3': return ImGuiKey_3;
  case '4': return ImGuiKey_4;
  case '5': return ImGuiKey_5;
  case '6': return ImGuiKey_6;
  case '7': return ImGuiKey_7;
  case '8': return ImGuiKey_8;
  case '9': return ImGuiKey_9;
  case 'A': return ImGuiKey_A;
  case 'B': return ImGuiKey_B;
  case 'C': return ImGuiKey_C;
  case 'D': return ImGuiKey_D;
  case 'E': return ImGuiKey_E;
  case 'F': return ImGuiKey_F;
  case 'G': return ImGuiKey_G;
  case 'H': return ImGuiKey_H;
  case 'I': return ImGuiKey_I;
  case 'J': return ImGuiKey_J;
  case 'K': return ImGuiKey_K;
  case 'L': return ImGuiKey_L;
  case 'M': return ImGuiKey_M;
  case 'N': return ImGuiKey_N;
  case 'O': return ImGuiKey_O;
  case 'P': return ImGuiKey_P;
  case 'Q': return ImGuiKey_Q;
  case 'R': return ImGuiKey_R;
  case 'S': return ImGuiKey_S;
  case 'T': return ImGuiKey_T;
  case 'U': return ImGuiKey_U;
  case 'V': return ImGuiKey_V;
  case 'W': return ImGuiKey_W;
  case 'X': return ImGuiKey_X;
  case 'Y': return ImGuiKey_Y;
  case 'Z': return ImGuiKey_Z;
  case VK_F1: return ImGuiKey_F1;
  case VK_F2: return ImGuiKey_F2;
  case VK_F3: return ImGuiKey_F3;
  case VK_F4: return ImGuiKey_F4;
  case VK_F5: return ImGuiKey_F5;
  case VK_F6: return ImGuiKey_F6;
  case VK_F7: return ImGuiKey_F7;
  case VK_F8: return ImGuiKey_F8;
  case VK_F9: return ImGuiKey_F9;
  case VK_F10: return ImGuiKey_F10;
  case VK_F11: return ImGuiKey_F11;
  case VK_F12: return ImGuiKey_F12;
  default: return ImGuiKey_None;
  }
}
