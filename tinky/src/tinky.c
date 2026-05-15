#include "tinky.h"


// Installe le service dans windows manager
void    install_service(void)
{
    char binary_path[MAX_PATH];
    GetModuleFileNameA(NULL, binary_path, MAX_PATH);

    SC_HANDLE handle_scm = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (!handle_scm)
    {
        printf("OpenSCManager failed (%lu)\n", GetLastError());
        return;
    }

    SC_HANDLE handle_svc =  CreateServiceA (
                                handle_scm,
                                SERVICE_NAME,
                                SERVICE_NAME,
                                SERVICE_ALL_ACCESS,
                                SERVICE_WIN32_OWN_PROCESS,
                                SERVICE_AUTO_START,
                                SERVICE_ERROR_CRITICAL,
                                binary_path,
                                NULL, NULL, NULL, NULL, NULL
    );
    if (!handle_svc)
    {
        printf("CreateService failed (%lu)\n", GetLastError());
        CloseServiceHandle(handle_scm);
        return;
    }

    CloseServiceHandle(handle_svc);
    CloseServiceHandle(handle_scm);
    printf("Service installed successfully.\n");
}


// Start le service 
void    start_service(void)
{
    SC_HANDLE handle_scm = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (!handle_scm)
    {
        printf("OpenSCManager failed (%lu)\n", GetLastError());
        return;
    }

    SC_HANDLE handle_open_svc = OpenService(handle_scm, SERVICE_NAME, SERVICE_ALL_ACCESS);
    if (!handle_open_svc)
    {
        printf("OpenService failed (%lu)\n", GetLastError());
        CloseServiceHandle(handle_scm);
        return;
    }

    BOOL start_svc = StartService(handle_open_svc, 0, NULL);
    if (!start_svc)
    {
        printf("StartService failed (%lu)\n", GetLastError());
        CloseServiceHandle(handle_scm);
        return;
    }

    printf("Service started successfully.\n");
    CloseServiceHandle(handle_scm);
    CloseServiceHandle(handle_open_svc);
    return ;

}

// Stop le service 
void    stop_service(void)
{
    SC_HANDLE handle_scm = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (!handle_scm)
    {
        printf("OpenSCManager failed (%lu)\n", GetLastError());
        return;
    }

    SC_HANDLE handle_open_svc = OpenService(handle_scm, SERVICE_NAME, SERVICE_ALL_ACCESS);
    if (!handle_open_svc)
    {
        printf("OpenService failed (%lu)\n", GetLastError());
        CloseServiceHandle(handle_scm);
        return;
    }

    SERVICE_STATUS status;
    if (!ControlService(handle_open_svc, SERVICE_CONTROL_STOP, &status))
        printf("ControlService Stop failed (%lu)\n", GetLastError());
    else
        printf("Service stopped successfully.\n");

    CloseServiceHandle(handle_open_svc);
    CloseServiceHandle(handle_scm);
}

// Delete le service
void    delete_service(void)
{
    SC_HANDLE handle_scm = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (!handle_scm)
    {
        printf("OpenSCManager failed (%lu)\n", GetLastError());
        return;
    }

    SC_HANDLE handle_open_svc = OpenService(handle_scm, SERVICE_NAME, SERVICE_ALL_ACCESS);
    if (!handle_open_svc)
    {
        printf("OpenService failed (%lu)\n", GetLastError());
        CloseServiceHandle(handle_scm);
        return;
    }

    if (!DeleteService(handle_open_svc))
        printf("DeleteService failed (%lu)\n", GetLastError());
    else
        printf("Service deleted successfully.\n");

    CloseServiceHandle(handle_open_svc);
    CloseServiceHandle(handle_scm);
}


int main(int ac, char **av)
{
    SERVICE_TABLE_ENTRYA service_table[] =
    {
        {SERVICE_NAME, ServiceMain},
        {NULL, NULL}
    };

    if (StartServiceCtrlDispatcher(service_table)) return 0;

    if (ac != 2) {
        printf("Usage :\nCommand allowed : install | start | stop | delete\n");
        return 1;
    }

    if (strcmp(av[1], "install") == 0)
        install_service();
    else if (strcmp(av[1], "start") == 0)
        start_service();
    else if (strcmp(av[1], "stop") == 0)
        stop_service();
    else if (strcmp(av[1], "delete") == 0)
        delete_service();
    else
    {
        printf("Unknown command.\nCommand allowed : install | start | stop | delete\n");
        return 1;
    }

    return 0;
}