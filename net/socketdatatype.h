#pragma once

#ifdef _MSC_VER
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#define FD_SETSIZE 1024
#include <Winsock2.h>
typedef SOCKET VTW_SocketHandle;
#else // !_MSC_VER
typedef int VTW_SocketHandle;
#endif
