// SaveDataToDB.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error �ڰ������� PCH �Ĵ��ļ�֮ǰ������stdafx.h��
#endif

#include "resource.h"		// ������


// CSaveDataToDBApp:
// �йش����ʵ�֣������ SaveDataToDB.cpp
//

class CSaveDataToDBApp : public CWinApp
{
public:
	CSaveDataToDBApp();

// ��д
	public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CSaveDataToDBApp theApp;
