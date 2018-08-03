#ifndef WINDOWS_H
#define WINDOWS_H

#include <cstdint>

extern "C" {

typedef wchar_t *LPSTR;
typedef const wchar_t *LPCSTR;
typedef void *HANDLE, **LPHANDLE, *LPVOID;
typedef unsigned int DWORD, *LPDWORD;
typedef long LONG;
typedef unsigned long ULONG;
typedef uintptr_t ULONG_PTR;
typedef intptr_t LONG_PTR;
typedef HANDLE HKEY;

#define ERROR_SUCCESS ((LONG)1)

#define INVALID_HANDLE_VALUE ((HANDLE) (LONG_PTR)-1)

#define RRF_RT_REG_SZ ((ULONG)1)

#define HKEY_CLASSES_ROOT ((HKEY) (ULONG_PTR)((LONG)0x80000000))
#define HKEY_CURRENT_USER ((HKEY) (ULONG_PTR)((LONG)0x80000001))
#define HKEY_LOCAL_MACHINE ((HKEY) (ULONG_PTR)((LONG)0x80000002))
#define HKEY_PERFORMANCE_NLSTEXT ((HKEY) (ULONG_PTR)((LONG)0x80000060))

#define KEY_ALL_ACCESS ((DWORD) 0x00000001)

int RegOpenKeyEx(HANDLE, LPCSTR, DWORD, DWORD, LPHANDLE) {
   return ERROR_SUCCESS;
}
int RegCloseKey(HANDLE) {
   return ERROR_SUCCESS;
}
int RegGetValue(HANDLE, LPCSTR, LPCSTR, ULONG, LPVOID, LPSTR, LPDWORD) {
   return ERROR_SUCCESS;
}

}

#endif
