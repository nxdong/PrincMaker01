
// PrincUser.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CPrincUserApp:
// �йش����ʵ�֣������ PrincUser.cpp
//

class CPrincUserApp : public CWinApp
{
public:
	CPrincUserApp();

// ��д
public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CPrincUserApp theApp;