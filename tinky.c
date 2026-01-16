#include "tinky.h"


// Installe le service dans windows manager
void    install_service(void)
{
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
                                BINARY_PATH,
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

    return 0;
}