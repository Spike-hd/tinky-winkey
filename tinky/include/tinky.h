#ifndef TINKY_H
# define TINKY_H

#include <windows.h>
#include <stdio.h>

#define SERVICE_NAME "tinky"

extern SERVICE_STATUS_HANDLE g_handler_svc;
extern SERVICE_STATUS g_service_status;
extern HANDLE g_event;

void WINAPI ServiceMain(DWORD argc, LPSTR *argv);
HANDLE impersonate_token(void);

#endif