
// UPack-V1.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CUPackV1App: 
// �йش����ʵ�֣������ UPack-V1.cpp
//

class CUPackV1App : public CWinApp
{
public:
	CUPackV1App();

// ��д
public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CUPackV1App theApp;