
// PrincUserDlg.h : ͷ�ļ�
//

#pragma once


// CPrincUserDlg �Ի���
typedef void (_cdecl *ServiceFunc)(wchar_t *serviceName );
class CPrincUserDlg : public CDialogEx
{
// ����
public:
	CPrincUserDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
	enum { IDD = IDD_PRINCUSER_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��


// ʵ��
protected:
	HICON m_hIcon;
	HMODULE m_hDll;     //handle for loaded dll
	ServiceFunc m_fService;
	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButton1();
};
