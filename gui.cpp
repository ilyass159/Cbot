#include "gui.h"

#include "../Simple Colorbot/imgui/imgui.h"
#include "../Simple Colorbot/imgui/imgui_impl_dx9.h"
#include "../Simple Colorbot/imgui/imgui_impl_win32.h"
#include "styles.h"


extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(
	HWND window,
	UINT message,
	WPARAM wideParameter,
	LPARAM longParameter
);




LRESULT __stdcall WindowProcess(
	HWND window,
	UINT message,
	WPARAM wideParameter,
	LPARAM longParameter)
{
	if (ImGui_ImplWin32_WndProcHandler(window, message, wideParameter, longParameter))
		return true;

	switch (message)
	{
	case WM_SIZE: {
		if (gui::device && wideParameter != SIZE_MINIMIZED)
		{
			gui::presentParameters.BackBufferWidth = LOWORD(longParameter);
			gui::presentParameters.BackBufferHeight = HIWORD(longParameter);
			gui::ResetDevice();
		}
	}return 0;

	case WM_SYSCOMMAND: {
		if ((wideParameter & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
			return 0;
	}break;

	case WM_DESTROY: {
		PostQuitMessage(0);
	}return 0;

	case WM_LBUTTONDOWN: {
		gui::position = MAKEPOINTS(longParameter); // set click points
	}return 0;

	case WM_MOUSEMOVE: {
		if (wideParameter == MK_LBUTTON)
		{
			const auto points = MAKEPOINTS(longParameter);
			auto rect = ::RECT{ };

			GetWindowRect(gui::window, &rect);

			rect.left += points.x - gui::position.x;
			rect.top += points.y - gui::position.y;

			if (gui::position.x >= 0 &&
				gui::position.x <= gui::WIDTH &&
				gui::position.y >= 0 && gui::position.y <= 19)
				SetWindowPos(
					gui::window,
					HWND_TOPMOST,
					rect.left,
					rect.top,
					0, 0,
					SWP_SHOWWINDOW | SWP_NOSIZE | SWP_NOZORDER
				);
		}

	}return 0;

	}

    return DefWindowProc(window, message, wideParameter, longParameter);
}

void gui::CreateHWindow(const char* windowName) noexcept
{
	windowClass.cbSize = sizeof(WNDCLASSEX);
	windowClass.style = CS_CLASSDC;
	windowClass.lpfnWndProc = WindowProcess;
	windowClass.cbClsExtra = 0;
	windowClass.cbWndExtra = 0;
	windowClass.hInstance = GetModuleHandleA(0);
	windowClass.hIcon = 0;
	windowClass.hCursor = 0;
	windowClass.hbrBackground = 0;
	windowClass.lpszMenuName = 0;
	windowClass.lpszClassName = "class001";
	windowClass.hIconSm = 0;

	RegisterClassEx(&windowClass);

	window = CreateWindowEx(
		0,
		"class001",
		windowName,
		WS_POPUP,
		100,
		100,
		WIDTH,
		HEIGHT,
		0,
		0,
		windowClass.hInstance,
		0
	);

	ShowWindow(window, SW_SHOWDEFAULT);
	UpdateWindow(window);
}

void gui::DestroyHWindow() noexcept
{
	DestroyWindow(window);
	UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);
}

bool gui::CreateDevice() noexcept
{
	d3d = Direct3DCreate9(D3D_SDK_VERSION);

	if (!d3d)
		return false;

	ZeroMemory(&presentParameters, sizeof(presentParameters));

	presentParameters.Windowed = TRUE;
	presentParameters.SwapEffect = D3DSWAPEFFECT_DISCARD;
	presentParameters.BackBufferFormat = D3DFMT_UNKNOWN;
	presentParameters.EnableAutoDepthStencil = TRUE;
	presentParameters.AutoDepthStencilFormat = D3DFMT_D16;
	presentParameters.PresentationInterval = D3DPRESENT_INTERVAL_ONE;

	if (d3d->CreateDevice(
		D3DADAPTER_DEFAULT,
		D3DDEVTYPE_HAL,
		window,
		D3DCREATE_HARDWARE_VERTEXPROCESSING,
		&presentParameters,
		&device) < 0)
		return false;

	return true;
}

void gui::ResetDevice() noexcept
{
	ImGui_ImplDX9_InvalidateDeviceObjects();

	const auto result = device->Reset(&presentParameters);

	if (result == D3DERR_INVALIDCALL)
		IM_ASSERT(0);

	ImGui_ImplDX9_CreateDeviceObjects();
}

void gui::DestroyDevice() noexcept
{
	if (device)
	{
		device->Release();
		device = nullptr;
	}

	if (d3d)
	{
		d3d->Release();
		d3d = nullptr;
	}
}

void gui::CreateImGui() noexcept
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ::ImGui::GetIO();

	io.IniFilename = NULL;

	ImGui::StyleColorsDark();

	ImGui_ImplWin32_Init(window);
	ImGui_ImplDX9_Init(device);
}

void gui::DestroyImGui() noexcept
{
	ImGui_ImplDX9_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

void gui::BeginRender() noexcept
{
	MSG message;
	while (PeekMessage(&message, 0, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&message);
		DispatchMessage(&message);

		if (message.message == WM_QUIT)
		{
			isRunning = !isRunning;
			return;
		}
	}

	// Start the Dear ImGui frame
	ImGui_ImplDX9_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
}

void gui::EndRender() noexcept
{
	ImGui::EndFrame();

	device->SetRenderState(D3DRS_ZENABLE, FALSE);
	device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	device->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);

	device->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_RGBA(0, 0, 0, 255), 1.0f, 0);

	if (device->BeginScene() >= 0)
	{
		ImGui::Render();
		ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
		device->EndScene();
	}

	const auto result = device->Present(0, 0, 0, 0);

	// Handle loss of D3D9 device
	if (result == D3DERR_DEVICELOST && device->TestCooperativeLevel() == D3DERR_DEVICENOTRESET)
		ResetDevice();
}




using namespace globals;




void gui::Render() noexcept
{
	ImGui::SetNextWindowPos({ 0, 0 });
	ImGui::SetNextWindowSize({ WIDTH, HEIGHT });
	ImGui::Begin(
		"Elmopios",
		&isRunning,
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoSavedSettings |
		ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoMove
	);








	ImGui::Checkbox("ColorBot", &colorbotfonction);
	// ImGui sliders for scanning size and smoothing

	ImGui::SliderFloat("Scanning Size", &globals::ScanningSize, 1.0f, 500.0f);
	ImGui::SliderFloat("Smoothing", &globals::Smoothing, 0.0f, 1.0f);
	// ImGui::SliderFloat("Tolerance", &globals::colorTolerance, 0.0f, 300.0f);


	ImGui::SliderFloat("displacementX", &maxDisplacementX, -70.0f, 70.0f);
	ImGui::SliderFloat("displacementY", &maxDisplacementY, -70.0f, 70.0f);

	 ImGui::InputFloat("Speed2", &globals::speedofF, 0.001f, 50.0f);

	// Inside your ImGui rendering function (gui::Render or a similar function)
	ImGui::Text("Current Trigger Key: %s", keyToString(triggerKey).c_str());

	if (ImGui::Button("Change Trigger Key")) {
		ImGui::OpenPopup("ChangeKeyPopup");
	}

	if (ImGui::BeginPopup("ChangeKeyPopup")) {
		ImGui::Text("Press a key to set as the new trigger key:");

		for (int i = ImGuiKey_Tab; i < ImGuiKey_COUNT; ++i) {
			if (ImGui::IsKeyPressed((ImGuiKey)i, true)) {
				triggerKey = i;
				ImGui::CloseCurrentPopup();
				break;
			}
		}
		

		ImGui::EndPopup();
	}

	ImGui::Text("Choose a style:");

	// Buttons in the same line
	ImGui::SameLine();
	if (ImGui::Button("Light Style"))
	{
		ImGui::StyleColorsLight();
	}

	ImGui::SameLine();
	if (ImGui::Button("Dark Style"))
	{
		ImGui::StyleColorsDark();
	}

	ImGui::SameLine();
	if (ImGui::Button("Default"))
	{
		ApplyCustomStyles();
	}



	ImGui::End();


}

// Function to convert virtual key code to string
std::string keyToString(int virtualKeyCode) {
	switch (virtualKeyCode) {
	case VK_LBUTTON: return "Left Mouse Button";
	case VK_RBUTTON: return "Right Mouse Button";
	case VK_CANCEL: return "Control-break processing";
	case VK_MBUTTON: return "Middle Mouse Button";
	case VK_XBUTTON1: return "X1 Mouse Button";
	case VK_XBUTTON2: return "X2 Mouse Button";
	case VK_BACK: return "Backspace";
	case VK_TAB: return "Tab";
	case VK_CLEAR: return "Clear";
	case VK_RETURN: return "Enter";
	case VK_SHIFT: return "Shift";
	case VK_CONTROL: return "Control";
	case VK_MENU: return "Alt";
	case VK_PAUSE: return "Pause";
	case VK_CAPITAL: return "Caps Lock";
	case VK_ESCAPE: return "Escape";
	case VK_SPACE: return "Space";
	case VK_PRIOR: return "Page Up";
	case VK_NEXT: return "Page Down";
	case VK_END: return "End";
	case VK_HOME: return "Home";
	case VK_LEFT: return "Left Arrow";
	case VK_UP: return "Up Arrow";
	case VK_RIGHT: return "Right Arrow";
	case VK_DOWN: return "Down Arrow";
	case VK_SELECT: return "Select";
	case VK_PRINT: return "Print";
	case VK_EXECUTE: return "Execute";
	case VK_SNAPSHOT: return "Print Screen";
	case VK_INSERT: return "Insert";
	case VK_DELETE: return "Delete";
	case VK_HELP: return "Help";
	case VK_LWIN: return "Left Windows key (Natural keyboard)";
	case VK_RWIN: return "Right Windows key (Natural keyboard)";
	case VK_APPS: return "Applications key (Natural keyboard)";
	case VK_SLEEP: return "Computer Sleep key";
	case VK_NUMPAD0: return "Numpad 0";
	case VK_NUMPAD1: return "Numpad 1";
	case VK_NUMPAD2: return "Numpad 2";
	case VK_NUMPAD3: return "Numpad 3";
	case VK_NUMPAD4: return "Numpad 4";
	case VK_NUMPAD5: return "Numpad 5";
	case VK_NUMPAD6: return "Numpad 6";
	case VK_NUMPAD7: return "Numpad 7";
	case VK_NUMPAD8: return "Numpad 8";
	case VK_NUMPAD9: return "Numpad 9";
	case VK_MULTIPLY: return "Multiply";
	case VK_ADD: return "Add";
	case VK_SEPARATOR: return "Separator";
	case VK_SUBTRACT: return "Subtract";
	case VK_DECIMAL: return "Decimal";
	case VK_DIVIDE: return "Divide";
	case VK_F1: return "F1";
	case VK_F2: return "F2";
	case VK_F3: return "F3";
	case VK_F4: return "F4";
	case VK_F5: return "F5";
	case VK_F6: return "F6";
	case VK_F7: return "F7";
	case VK_F8: return "F8";
	case VK_F9: return "F9";
	case VK_F10: return "F10";
	case VK_F11: return "F11";
	case VK_F12: return "F12";
	case VK_F13: return "F13";
	case VK_F14: return "F14";
	case VK_F15: return "F15";
	case VK_F16: return "F16";
	case VK_F17: return "F17";
	case VK_F18: return "F18";
	case VK_F19: return "F19";
	case VK_F20: return "F20";
	case VK_F21: return "F21";
	case VK_F22: return "F22";
	case VK_F23: return "F23";
	case VK_F24: return "F24";
	case VK_NUMLOCK: return "Num Lock";
	case VK_SCROLL: return "Scroll Lock";
	case VK_OEM_NEC_EQUAL: return "OEM NEC Equal";
	case VK_OEM_FJ_MASSHOU: return "OEM FJ Masshou";
	case VK_OEM_FJ_TOUROKU: return "OEM FJ Touroku";
	case VK_OEM_FJ_LOYA: return "OEM FJ Loya";
	case VK_OEM_FJ_ROYA: return "OEM FJ Roya";
	case VK_LSHIFT: return "Left Shift";
	case VK_RSHIFT: return "Right Shift";
	case VK_LCONTROL: return "Left Control";
	case VK_RCONTROL: return "Right Control";
	case VK_LMENU: return "Left Alt";
	case VK_RMENU: return "Right Alt";
	case VK_BROWSER_BACK: return "Browser Back";
	case VK_BROWSER_FORWARD: return "Browser Forward";
	case VK_BROWSER_REFRESH: return "Browser Refresh";
	case VK_BROWSER_STOP: return "Browser Stop";
	case VK_BROWSER_SEARCH: return "Browser Search";
	case VK_BROWSER_FAVORITES: return "Browser Favorites";
	case VK_BROWSER_HOME: return "Browser Start and Home";
	case VK_VOLUME_MUTE: return "Volume Mute";
	case VK_VOLUME_DOWN: return "Volume Down";
	case VK_VOLUME_UP: return "Volume Up";
	case VK_MEDIA_NEXT_TRACK: return "Next Track";
	case VK_MEDIA_PREV_TRACK: return "Previous Track";
	case VK_MEDIA_STOP: return "Stop Media";
	case VK_MEDIA_PLAY_PAUSE: return "Play/Pause Media";
	case VK_LAUNCH_MAIL: return "Start Mail";
	case VK_LAUNCH_MEDIA_SELECT: return "Select Media";
	case VK_LAUNCH_APP1: return "Start Application 1";
	case VK_LAUNCH_APP2: return "Start Application 2";
	case VK_OEM_1: return "OEM_1 (: ;)";
	case VK_OEM_PLUS: return "OEM_PLUS (+ =)";
	case VK_OEM_COMMA: return "OEM_COMMA (, <)";
	case VK_OEM_MINUS: return "OEM_MINUS (- _)";
	case VK_OEM_PERIOD: return "OEM_PERIOD (. >)";
	case VK_OEM_2: return "OEM_2 (? /)";
	case VK_OEM_3: return "OEM_3 (~ `)";
	case VK_OEM_4: return "OEM_4 ({ [)";
	case VK_OEM_5: return "OEM_5 (\\ |)";
	case VK_OEM_6: return "OEM_6 (} ])";
	case VK_OEM_7: return "OEM_7 (' \")";
	case VK_OEM_8: return "OEM_8 (, <)";
	case VK_OEM_AX: return "OEM_AX (AX)";
	case VK_OEM_102: return "OEM_102 (> <)";
	case VK_ICO_HELP: return "ICO_HELP (Help)";
	case VK_ICO_00: return "ICO_00 (*)";
	case VK_PROCESSKEY: return "IME Process";
	case VK_ICO_CLEAR: return "ICO_CLEAR";
	case VK_PACKET: return "Packet";
	case VK_OEM_RESET: return "OEM Reset";
	case VK_OEM_JUMP: return "OEM Jump";
	case VK_OEM_PA1: return "OEM_PA1";
	case VK_OEM_PA2: return "OEM_PA2";
	case VK_OEM_PA3: return "OEM_PA3";
	case VK_OEM_WSCTRL: return "OEM_WSCTRL";
	case VK_OEM_CUSEL: return "OEM_CUSEL";
	case VK_OEM_ATTN: return "OEM_ATTN";
	case VK_OEM_FINISH: return "OEM_FINISH";
	case VK_OEM_COPY: return "OEM_COPY";
	case VK_OEM_AUTO: return "OEM_AUTO";
	case VK_OEM_ENLW: return "OEM_ENLW";
	case VK_OEM_BACKTAB: return "OEM_BACKTAB";
	case VK_ATTN: return "Attn";
	case VK_CRSEL: return "CrSel";
	case VK_EXSEL: return "ExSel";
	case VK_EREOF: return "Erase EOF";
	case VK_PLAY: return "Play";
	case VK_ZOOM: return "Zoom";
	case VK_NONAME: return "Reserved";
	case VK_PA1: return "PA1";
	case VK_OEM_CLEAR: return "Clear";
	default: return "Unknown Key";
	}
}