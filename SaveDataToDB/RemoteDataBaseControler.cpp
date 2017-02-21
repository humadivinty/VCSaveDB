#include "StdAfx.h"
#include "RemoteDataBaseControler.h"
RemoteDataBaseControler::RemoteDataBaseControler()
{
	InitializeCriticalSection(&m_csRemoteDBLog);
	ReadConfigFileFromDisk();	
	m_iDBId = 0;
}
RemoteDataBaseControler::RemoteDataBaseControler( int DBId )
{
	InitializeCriticalSection(&m_csRemoteDBLog);
	ReadConfigFileFromDisk();	
	m_iDBId = DBId;
}

RemoteDataBaseControler::~RemoteDataBaseControler()
{
	DeleteCriticalSection(&m_csRemoteDBLog);
}
HRESULT RemoteDataBaseControler::SaveBigImageToDB( unsigned char* pImage, char* ListNo,long nImageLen )
{
	return S_FALSE;
}

void RemoteDataBaseControler::ReadConfigFileFromDisk()
{
	char chFileName[MAX_PATH];
	GetModuleFileNameA(NULL, chFileName, MAX_PATH-1);
	PathRemoveFileSpecA(chFileName);
	char chIniFileName[MAX_PATH] = { 0 };
	strcpy(chIniFileName, chFileName);
	strcat(chIniFileName, "\\HvConfig.ini");

	//��ȡ���ݿ������IP
	GetPrivateProfileStringA("RemoteDataBase","ServerIP","127.0.0.1",m_chServerIP,256,chIniFileName);	
	WritePrivateProfileStringA("RemoteDataBase","ServerIP",m_chServerIP,chIniFileName);
	//��ȡ���ݿ�����
	GetPrivateProfileStringA("RemoteDataBase","DataBaseName","RoadMidDb",m_chDBName,256,chIniFileName);
	WritePrivateProfileStringA("RemoteDataBase","DataBaseName",m_chDBName,chIniFileName);
	//��ȡ���ݿ��û���
	GetPrivateProfileStringA("RemoteDataBase","DBUserID","kk",m_chDBUserID,256,chIniFileName);
	WritePrivateProfileStringA("RemoteDataBase","DBUserID",m_chDBUserID,chIniFileName);
	//��ȡ���ݿ�����
	GetPrivateProfileStringA("RemoteDataBase","DBPassword","123456",m_chDBPassword,256,chIniFileName);
	WritePrivateProfileStringA("RemoteDataBase","DBPassword",m_chDBPassword,chIniFileName);

	int iTmp = GetPrivateProfileIntA("RemoteDataBase", "LogEnable", 0, chIniFileName);
	m_bLogEnable = (iTmp == 0) ? false : true;

	char chTemp[2] = {0};
	sprintf(chTemp, "%d", m_bLogEnable ? 1 : 0 );
	WritePrivateProfileStringA("RemoteDataBase", "LogEnable", chTemp, chIniFileName);
}

HRESULT RemoteDataBaseControler::SaveNormalDataToDB( CameraResult* pRecord )
{
	HRESULT hr = S_FALSE;
	if (NULL == pRecord)
	{
		return hr;
	}

	try
	{
		if (!IsConnect())
		{
			RemoteDBWriteLog("SaveNormalDataToDB::���ݿ�����ʧ�ܣ�������������");
			char chReConnectInfo[256] = {0};
			hr =ConnectToDB(chReConnectInfo);
			char chNormalConnectInfo[MAX_PATH] = {0};
			if (S_OK != hr)
			{
				sprintf(chNormalConnectInfo,"SaveNormalDataToDB:: ����ʧ�ܣ�%s", chReConnectInfo);
				RemoteDBWriteLog(chNormalConnectInfo);
				return hr;
			}
			else
			{
				sprintf(chNormalConnectInfo,"SaveNormalDataToDB:: �����ɹ���%s", chReConnectInfo);
				RemoteDBWriteLog(chNormalConnectInfo);
			}	
		}

		_variant_t RecordAffected;
		char chCmdText[1024] = {0};
		char chValues[1024] = {0};
		char chColumns[1024] = {0};
		
		
		SYSTEMTIME st;
		GetLocalTime(&st);
		char szTimeFlag[256]={0};
		sprintf(szTimeFlag, "%d-%02d-%02d %02d:%02d:%02d:%03d", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);	

		sprintf(chColumns, "%s", "ListNo, DeviceID, LaneNo, Optime, DirectionNo, VehPlate, VehPlateManual, VehPlateSoft, PlateColorNo, VehSpeed, VehBodyColorNo, VehBodyDeepNo, VehTypeNo, PlateTypeNo, TimeFlag");
		sprintf(chValues, "('%s',%d,%d,'%s',%d,'%s','%s','%s',%d,%d,%d,%d,%d,%d,'%s')",
			pRecord->chListNo,
			pRecord->iDeviceID,
			pRecord->iLaneNo,
			pRecord->chPlateTime,
			pRecord->iDirection,
			pRecord->chPlateNO,
			pRecord->chVehPlateManual,
			pRecord->chVehPlateSoft,
			pRecord->iPlateColor,
			pRecord->iSpeed,
			pRecord->iVehBodyColorNo,
			pRecord->iVehBodyDeepNo,
			pRecord->iVehTypeNo,
			pRecord->iPlateTypeNo,
			szTimeFlag);
		sprintf(chCmdText, "INSERT INTO %s(%s)VALUES %s", "HDVEHICLELIST", chColumns, chValues);

		m_pConnectionPtr->Execute(_bstr_t(chCmdText), &RecordAffected, adCmdText);
		hr = S_OK;
	}
	catch (_com_error &e)
	{		
		hr =S_FALSE;
		CString strErrorMessage;
		strErrorMessage.Format("SaveNormalDataToDB�����ݿ�д��ʧ�ܣ�������Ϣ:%s ������:%d, ����������%s, ListNo = %s", e.ErrorMessage(), GetLastError(), (char*)e.Description(), pRecord->chListNo);
		RemoteDBWriteLog(strErrorMessage.GetBuffer());
		strErrorMessage.ReleaseBuffer();

		//��������������������ظ���ͻ��������������
		CString strErrorDescriptionTemp((char *)e.Description());		
		if ( -1 != strErrorDescriptionTemp.Find("PRIMARY KEY") )
		{
			char szLog1[512]={0};
			sprintf(szLog1,"����:%s, CarID:%d,����ֵ�ظ�,������������", pRecord->chListNo,pRecord->dwCarID);
			RemoteDBWriteLog(szLog1);
			hr=S_OK;
		}
		if (strstr(strErrorMessage,"һ�����������") || strstr(strErrorMessage,"��ʱ") || strstr(strErrorMessage,"3121"))
		{
			m_pConnectionPtr->Close();
		}
		//Sleep(5*1000);
	}
	return hr;
}

HRESULT RemoteDataBaseControler::SaveSmallImageToDB( unsigned char* pImage, char* ListNo,long nImageLen )
{
	return S_FALSE;
}

HRESULT RemoteDataBaseControler::SaveDeviceStatusToDB( char* chListNo, int iDeviceID, char* chStatuMessage, char* chCreateTime, int iStatu )
{
	HRESULT hr = S_FALSE;

	try
	{
		if (!IsConnect())
		{
			RemoteDBWriteLog("SaveDeviceStatusToDB::���ݿ�����ʧ�ܣ�������������");
			char chReConnectInfo[256] = {0};
			char chStatusConnectInfo[256] = {0};
			hr =ConnectToDB(chReConnectInfo);
			if (S_OK != hr)
			{
				sprintf(chStatusConnectInfo, "SaveDeviceStatusToDB:: ����ʧ�ܣ�%s", chReConnectInfo);
				RemoteDBWriteLog(chStatusConnectInfo);
				return hr;
			}
			else
			{
				sprintf(chStatusConnectInfo, "SaveDeviceStatusToDB:: �����ɹ���%s", chReConnectInfo);
				RemoteDBWriteLog(chStatusConnectInfo);
			}	
		}
		char chCmdTextp[1024] = {0};
		char chValues[1024] ={0};
		char chClounName[256]={0};
		char chUpload[1] ={0};

		//���첢�������
		sprintf(chClounName,"%s","ListNo, DeviceID, DEVICESTATUSNO, ExceptionMessage, Optime, TimeFlag");
		sprintf(chValues, "('%s', %d, %d, '%s', '%s', '%s')", chListNo, iDeviceID, iStatu, chStatuMessage, chCreateTime, chCreateTime);
		sprintf(chCmdTextp, "INSERT INTO %s(%s) VALUES %s", "HDCardListState",chClounName, chValues);

		_variant_t RecordsAffected;
		m_pConnectionPtr->Execute((_bstr_t)chCmdTextp, &RecordsAffected, adCmdText);
		hr = S_OK;
	}
	catch(_com_error &e)
	{
		hr = S_FALSE;
		CString strErrorMessage;
		strErrorMessage.Format("�豸״̬д��ʧ�ܣ�������Ϣ:%s ������:%d,����������%s, ListNo = %s", e.ErrorMessage(), GetLastError(), (char*)e.Description(), chListNo);
		RemoteDBWriteLog(strErrorMessage.GetBuffer());
		strErrorMessage.ReleaseBuffer();

		CString strErrorDescriptionTemp((char *)e.Description());		
		if ( -1 != strErrorDescriptionTemp.Find("PRIMARY KEY ") )
		{
			char szLog1[512]={0};
			sprintf(szLog1,"�豸״̬��¼������:%s������ֵ�ظ�,����������¼",chListNo);
			RemoteDBWriteLog(szLog1);
			hr = S_OK;
		}
		else
		{
			hr = S_FALSE;
		}

		if (strstr(strErrorMessage,"һ�����������") || strstr(strErrorMessage,"��ʱ") || strstr(strErrorMessage,"3121") )
		{
			m_pConnectionPtr->Close();
		}
		//Sleep(5*1000);
	}	
	return hr;
}

void RemoteDataBaseControler::RemoteDBWriteLog( char* logBuf )
{
	if (!m_bLogEnable)
		return ;

	//ȡ�õ�ǰ�ľ�ȷ�����ʱ��
	static time_t starttime = time(NULL);
	static DWORD starttick = GetTickCount(); 
	DWORD dwNowTick = GetTickCount() - starttick;
	time_t nowtime = starttime + (time_t)(dwNowTick / 1000);
	struct tm *pTM = localtime(&nowtime);
	DWORD dwMS = dwNowTick % 1000;

	const int MAXPATH = 260;

	TCHAR szFileName[ MAXPATH] = {0};
	GetModuleFileName(NULL, szFileName, MAXPATH);	//ȡ�ð�����������ȫ·��
	PathRemoveFileSpec(szFileName);				//ȥ��������
	CString strCurrentDir = szFileName;
	CString fileName = _T("");
	strCurrentDir.AppendFormat("\\LOG\\%04d-%02d-%02d\\RemoteDBLog\\", pTM->tm_year + 1900, pTM->tm_mon + 1, pTM->tm_mday);
	MakeSureDirectoryPathExists(strCurrentDir);
	fileName.Format("%s%04d-%02d-%02d-%02d_RemoteDB_%d.log", strCurrentDir, pTM->tm_year + 1900, pTM->tm_mon + 1, pTM->tm_mday, pTM->tm_hour, m_iDBId);

	EnterCriticalSection(&m_csRemoteDBLog);
	FILE *file = NULL;
	file = fopen(fileName, "a+");
	if (file)
	{
		fprintf(file,"%04d-%02d-%02d %02d:%02d:%02d:%03d : %s\n",  pTM->tm_year + 1900, pTM->tm_mon + 1, pTM->tm_mday,
			pTM->tm_hour, pTM->tm_min, pTM->tm_sec, dwMS, logBuf);
		fclose(file);
		file = NULL;
	}
	LeaveCriticalSection(&m_csRemoteDBLog);
}