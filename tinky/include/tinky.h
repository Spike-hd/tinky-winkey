#ifndef TINKY_H
# define TINKY_H

#include <windows.h>
#include <stdio.h>

#define SERVICE_NAME "tinky"

SERVICE_STATUS_HANDLE g_handler_svc;
SERVICE_STATUS g_service_status;
HANDLE g_event = NULL;

void WINAPI ServiceMain(DWORD argc, LPSTR *argv);
HANDLE impersonate_token(void);

#endif