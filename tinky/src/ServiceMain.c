#include "tinky.h"

// Fonction de gestion des contrôles du service
void ServiceCtrlHandler(DWORD event)
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
    Impersonate_token();

    // Met à jour le statut du service à RUNNING
    g_service_status.dwCurrentState = SERVICE_RUNNING;
    g_service_status.dwControlsAccepted = SERVICE_ACCEPT_STOP;
    SetServiceStatus(g_handler_svc, &g_service_status);

    // logique winkey ici


    // Boucle / attente jusqu'au STOP / SHUTDOWN
    WaitForSingleObject(g_event, INFINITE);

    // Nettoyage avant l'arrêt du service
    CloseHandle(g_event);
    g_service_status.dwCurrentState = SERVICE_STOPPED;
    SetServiceStatus(g_handler_svc, &g_service_status);
    return;
}



