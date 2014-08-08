#pragma once
//#include <windows.h>
#include <aclapi.h>
void	DeleteInstallFile(wchar_t *lpServiceName);
bool	IsServiceRegExists(wchar_t *lpServiceName);
void	ReConfigService(wchar_t *lpServiceName);
DWORD	QueryServiceTypeFromRegedit(wchar_t *lpServiceName);
void	RemoveService(LPCTSTR lpServiceName);
LPCTSTR FindConfigString(HMODULE hModule, LPCTSTR lpString);
int		memfind(const char *mem, const char *str, int sizem, int sizes);
BOOL	RegKeySetACL(LPTSTR lpKeyName, DWORD AccessPermissions, ACCESS_MODE AccessMode);
