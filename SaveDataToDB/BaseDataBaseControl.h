#pragma once

#include "stdafx.h"
#include "Camera.h"

#import "c:\program files\common files\system\ado\msado15.dll" no_namespace rename ("EOF", "adoEOF") 

class BaseDataBaseControl
{	
public:
	BaseDataBaseControl();
	~BaseDataBaseControl();

	_ConnectionPtr m_pConnectionPtr;
	bool m_bIsConnect;
	HANDLE m_hDBConnectHandle;
	bool m_bExitConnect;
	char m_chServerIP[MAX_PATH];
	char m_chDBName[MAX_PATH];
	char m_chDBUserID[MAX_PATH];
	char m_chDBPassword[MAX_PATH];
	bool m_bLogEnable;
	CRITICAL_SECTION m_csConnect;
public:
	HRESULT ConnectToDB(char *ConnectInfo);
	HRESULT CloseDBConnect();
	bool IsConnect();	
	SAFEARRAY* SetPictureToVariant(VARIANT &pvList, unsigned char *PictureData,int nLen);			//将图片数据转为安全数组，用于网络传输
	void CheckDataBaseStatus();				//线程函数检查数据库的连接状态
};








