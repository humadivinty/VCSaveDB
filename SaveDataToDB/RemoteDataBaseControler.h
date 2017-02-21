#pragma once

#include "stdafx.h"
#include "BaseDataBaseControl.h"

class RemoteDataBaseControler :public BaseDataBaseControl
{
public:
	RemoteDataBaseControler();
	RemoteDataBaseControler(int DBId);	
	~RemoteDataBaseControler();
public:
	void ReadConfigFileFromDisk();
	HRESULT SaveNormalDataToDB(CameraResult* pRecord);
	HRESULT SaveBigImageToDB(unsigned char* pImage, char* ListNo,long nImageLen);
	HRESULT SaveSmallImageToDB(unsigned char* pImage, char* ListNo,long nImageLen);
	HRESULT SaveDeviceStatusToDB(char* chListNo, int iDeviceID, char* chStatuMessage, char* chCreateTime, int iStatu);
	void RemoteDBWriteLog(char* logBuf);

private:
	CRITICAL_SECTION m_csRemoteDBLog;
	bool m_bLogEnable;
	int m_iDBId;
};


