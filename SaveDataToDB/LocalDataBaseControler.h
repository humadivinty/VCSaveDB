#pragma once

#include "stdafx.h"
#include "BaseDataBaseControl.h"

class LocalDataBaseControler :public BaseDataBaseControl
{
public:
	LocalDataBaseControler();
	LocalDataBaseControler(int DBId);
	~LocalDataBaseControler();
public:
	void ReadConfigFileFromDisk();	
	HRESULT InitCameraGroup(CCamera**  CameraGroup);
	HRESULT SaveNormalDataToDB(CameraResult* pRecord);
	HRESULT SaveBigImageToDB(unsigned char* pImage, char* ListNo,long nImageLen);
	HRESULT SaveSmallImageToDB(unsigned char* pImage, char* ListNo,long nImageLen);
	HRESULT SaveDeviceStatusToDB(char* chListNo, int iDeviceID, char* chStatuMessage, char* chCreateTime, int iStatu);
	void LocalDBWriteLog(char* logBuf);

private:
	CRITICAL_SECTION m_csLocalDBLog;
	bool m_bLogEnable;
	int m_iDBId;
};