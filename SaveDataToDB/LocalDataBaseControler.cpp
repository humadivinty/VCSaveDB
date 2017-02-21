#include "StdAfx.h"
#include "LocalDataBaseControler.h"

LocalDataBaseControler::LocalDataBaseControler()
{
	InitializeCriticalSection(&m_csLocalDBLog);
	ReadConfigFileFromDisk();	
	m_iDBId = 0;
}

LocalDataBaseControler::LocalDataBaseControler( int DBId )
{
	InitializeCriticalSection(&m_csLocalDBLog);
	ReadConfigFileFromDisk();
	m_iDBId = DBId;
}

LocalDataBaseControler::~LocalDataBaseControler()
{
	DeleteCriticalSection(&m_csLocalDBLog);
}

void LocalDataBaseControler::ReadConfigFileFromDisk()
{
	char chFileName[MAX_PATH];
	GetModuleFileNameA(NULL, chFileName, MAX_PATH-1);
	PathRemoveFileSpecA(chFileName);
	char chIniFileName[MAX_PATH] = { 0 };
	strcpy(chIniFileName, chFileName);
	strcat(chIniFileName, "\\HvConfig.ini");

	//��ȡ���ݿ������IP
	GetPrivateProfileStringA("LocalDataBase","ServerIP","127.0.0.1",m_chServerIP,256,chIniFileName);	
	WritePrivateProfileStringA("LocalDataBase","ServerIP",m_chServerIP,chIniFileName);
	//��ȡ���ݿ�����
	GetPrivateProfileStringA("LocalDataBase","DataBaseName","KKDB",m_chDBName,256,chIniFileName);
	WritePrivateProfileStringA("LocalDataBase","DataBaseName",m_chDBName,chIniFileName);
	//��ȡ���ݿ��û���
	GetPrivateProfileStringA("LocalDataBase","DBUserID","kk",m_chDBUserID,256,chIniFileName);
	WritePrivateProfileStringA("LocalDataBase","DBUserID",m_chDBUserID,chIniFileName);
	//��ȡ���ݿ�����
	GetPrivateProfileStringA("LocalDataBase","DBPassword","123456",m_chDBPassword,256,chIniFileName);
	WritePrivateProfileStringA("LocalDataBase","DBPassword",m_chDBPassword,chIniFileName);

	int iTmp = GetPrivateProfileIntA("LocalDataBase", "LogEnable", 0, chIniFileName);
	m_bLogEnable = (iTmp == 0) ? false : true;
	
	char chTemp[2] = {0};
	sprintf(chTemp, "%d", m_bLogEnable ? 1 : 0 );
	WritePrivateProfileStringA("LocalDataBase", "LogEnable", chTemp, chIniFileName);
}

HRESULT LocalDataBaseControler::SaveBigImageToDB(unsigned char* pImage, char* ListNo,long nImageLen )
{
	HRESULT hr = S_FALSE;

	if(pImage==NULL) return hr;

	try
	{
		if(!IsConnect())
		{
			LocalDBWriteLog("SaveBigImageToDB::���ݿ�����ʧ�ܣ�������������");
			char chReConnectInfo[256] = {0};
			hr =ConnectToDB(chReConnectInfo);
			char chBigImgConnectInfo[MAX_PATH] = {0};
			if (S_OK != hr)
			{
				sprintf(chBigImgConnectInfo, "SaveBigImageToDB:: ����ʧ�ܣ�%s", chReConnectInfo);
				LocalDBWriteLog(chBigImgConnectInfo);
				return hr;
			}
			else
			{
				sprintf(chBigImgConnectInfo, "SaveBigImageToDB:: �����ɹ���%s", chReConnectInfo);
				LocalDBWriteLog(chBigImgConnectInfo);
			}			
		}
		char szUpLoad[256]={0};
		sprintf(szUpLoad,"%d",1);

		_RecordsetPtr    pRecordset;
		//int nTick=::GetTickCount();
		pRecordset.CreateInstance(__uuidof(Recordset));

			//�����ͼ����
		pRecordset->Open("select top 1 * from CarFullView",_variant_t((IDispatch*)m_pConnectionPtr),adOpenStatic,adLockOptimistic,adCmdText);
	
		pRecordset->AddNew();
		CString str=pRecordset->Fields->Item["UpLoad"]->Value ;

		pRecordset->Fields->Item["UpLoad"]->Value = (_variant_t)szUpLoad;//jpgid
		pRecordset->Fields->Item["ListNo"]->Value = (_variant_t)ListNo;		//��ˮ��   ��Ϊselect* ���Ա���ø������ֶθ�ֵ����update��ʱ������

		VARIANT pvList;
		SAFEARRAY* pSA=  SetPictureToVariant(pvList,(unsigned char *)pImage,nImageLen);    

		pRecordset->Fields->Item[_variant_t("FullViewPhoto")]->AppendChunk(pvList);                  //JPGͼ���ļ�
		if(S_OK!=pRecordset->Update())
		{
			CString strLog;
			strLog.Format("SaveBigImageToDB:::[%d]ͼƬ���ݲ���ʧ��", GetCurrentThreadId());
			hr=S_FALSE;
			LocalDBWriteLog(str.GetBuffer());
			str.ReleaseBuffer();
		}
		pRecordset->MoveNext();

		pRecordset->Close();
		pRecordset = NULL;
		SafeArrayUnaccessData(pSA);
		SafeArrayDestroyData(pSA);	
		SafeArrayDestroy(pSA);  
		hr = S_OK;
	}catch(_com_error e)
	{
		hr=S_FALSE;

		m_bIsConnect = false;
		CString strErrorMessage;
		strErrorMessage.Format("SaveBigImageToDB::����ͼƬʧ�ܣ�������Ϣ:%s ������:%d,  ����������%s, ListNo = %s", e.ErrorMessage(), GetLastError(), (char*)e.Description(), ListNo);
		LocalDBWriteLog(strErrorMessage.GetBuffer());
		LocalDBWriteLog(e.Description());
		strErrorMessage.ReleaseBuffer();
		
		CString strError="IDispatch error #3092";
		if(strError==e.ErrorMessage())
		{
			LocalDBWriteLog("SaveBigImageToDB::sqlִ��������﷨����");
			//exit(0);
			//AfxMessageBox("SaveBigImageToDB::sqlִ��������﷨����",0,0);
		}

		if(strstr(strErrorMessage,"�洢�ռ䲻��"))
		{
			LocalDBWriteLog("SaveBigImageToDB::�洢�ռ䲻�� �˳�");
			exit(1);
		}
		LocalDBWriteLog("SaveBigImageToDB::˯5����");

		if (strstr(strErrorMessage,"һ�����������") || strstr(strErrorMessage,"��ʱ") || strstr(strError,"3121"))
		{
			LocalDBWriteLog("SaveBigImageToDB::��ѯ��ʱ");
			if (adStateOpen == m_pConnectionPtr->GetState())
			{
				m_pConnectionPtr->Close();
			}
		}
		//Sleep(5*1000);
		//exit(0);
		return hr;
	}

	return hr;
}

HRESULT LocalDataBaseControler::SaveSmallImageToDB( unsigned char* pImage, char* ListNo,long nImageLen )
{
	HRESULT hr = S_FALSE;

	if(pImage==NULL) return hr;

	try
	{
		if(!IsConnect())
		{
			LocalDBWriteLog("SaveSmallImageToDB::���ݿ�����ʧ�ܣ�������������");
			char chReConnectInfo[256] = {0};
			hr =ConnectToDB(chReConnectInfo);
			char chSmallImgConnectInfo[MAX_PATH] = {0};
			if (S_OK != hr)
			{
				sprintf(chSmallImgConnectInfo,"SaveNormalDataToDB:: ����ʧ�ܣ�%s", chReConnectInfo);
				LocalDBWriteLog(chSmallImgConnectInfo);
				return hr;
			}
			else
			{
				sprintf(chSmallImgConnectInfo,"SaveSmallImageToDB:: �����ɹ���%s", chReConnectInfo);
				LocalDBWriteLog(chSmallImgConnectInfo);
			}	
		}
		char szUpLoad[256]={0};
		sprintf(szUpLoad,"%d",1);

		_RecordsetPtr    pRecordset;
		int nTick=::GetTickCount();

		pRecordset.CreateInstance(__uuidof(Recordset));
			//����Сͼ����
			pRecordset->Open("select top 1*  from CarCloseUp",_variant_t((IDispatch*)m_pConnectionPtr),adOpenStatic,adLockOptimistic,adCmdText);
		
		pRecordset->AddNew();
		CString str=pRecordset->Fields->Item["UpLoad"]->Value ;

		pRecordset->Fields->Item["UpLoad"]->Value = (_variant_t)szUpLoad;//jpgid
		pRecordset->Fields->Item["ListNo"]->Value = (_variant_t)ListNo;//��ˮ��   ��Ϊselect* ���Ա���ø������ֶθ�ֵ����update��ʱ������

		VARIANT pvList;
		SAFEARRAY* pSA=  SetPictureToVariant(pvList,(unsigned char *)pImage,nImageLen);    

		pRecordset->Fields->Item["CloseUpPhoto"]->AppendChunk(pvList);                    //JPGͼ���ļ�
		
		if(S_OK!=pRecordset->Update())
		{
			str.Format("SaveSmallImageToDB:[%d]��������ʧ��", GetCurrentThreadId());
			hr=S_FALSE;
			LocalDBWriteLog(str.GetBuffer());
			str.ReleaseBuffer();
		}
		pRecordset->MoveNext();

		pRecordset->Close();
		pRecordset = NULL;
		SafeArrayUnaccessData(pSA);
		SafeArrayDestroyData(pSA);	
		SafeArrayDestroy(pSA);  
		hr = S_OK;

	}catch(_com_error e)
	{
		hr=S_FALSE;

		m_bIsConnect=false;
		CString strErrorMessage;
		strErrorMessage.Format("SaveSmallImageToDB::����ͼƬʧ�ܣ�������Ϣ:%s ������:%d,  ����������%s, ListNo = %s", e.ErrorMessage(), GetLastError(), (char*)e.Description(), ListNo);
		LocalDBWriteLog(strErrorMessage.GetBuffer());
		LocalDBWriteLog(e.Description());
		strErrorMessage.ReleaseBuffer();
		
		CString strError="IDispatch error #3092";
		if(strError==e.ErrorMessage())
		{
			LocalDBWriteLog("SaveSmallImageToDB::sqlִ��������﷨����");
			//exit(0);
			//AfxMessageBox("SaveSmallImageToDB::������������������ͷŴ��̿ռ�",0,0);
		}

		if (strstr(strErrorMessage,"һ�����������") || strstr(strErrorMessage,"��ʱ") || strstr(strError,"3121"))
		{
			LocalDBWriteLog("SaveSmallImageToDB::��ѯ��ʱ");
			if (adStateOpen == m_pConnectionPtr->GetState())
			{
				m_pConnectionPtr->Close();
			}
		}
		//Sleep(5*1000);
		//exit(0);
		//return hr;
	}

	return hr;
}

HRESULT LocalDataBaseControler::SaveDeviceStatusToDB( char* chListNo, int iDeviceID, char* chStatuMessage, char* chCreateTime, int iStatu )
{
	HRESULT hr = S_FALSE;

	try
	{
		if (!IsConnect())
		{
			LocalDBWriteLog("SaveDeviceStatusToDB::���ݿ�����ʧ�ܣ�������������");
			char chReConnectInfo[256] = {0};
			hr =ConnectToDB(chReConnectInfo);
			char  chStatusConnectInfo[MAX_PATH] = {0};
			if (S_OK != hr)
			{
				sprintf(chStatusConnectInfo, "SaveDeviceStatusToDB:: ����ʧ�ܣ�%s", chReConnectInfo);
				LocalDBWriteLog(chStatusConnectInfo);
				return hr;
			}
			else
			{
				sprintf(chStatusConnectInfo, "SaveDeviceStatusToDB:: �����ɹ���%s", chReConnectInfo);
				LocalDBWriteLog(chStatusConnectInfo);
			}	
		}
		char chCmdTextp[1024] = {0};
		char chValues[1024] ={0};
		char chClounName[256]={0};
		char chUpload[1] ={0};
		
		//���첢�������
		sprintf(chClounName,"%s","ListNo,DeviceStatusNo,DeviceID,ExceptionMessage,Optime,UpLoad");
		sprintf(chValues, "('%s', %d, %d, '%s', '%s', '%s')", chListNo, iStatu, iDeviceID, chStatuMessage, chCreateTime, chUpload);
		sprintf(chCmdTextp, "INSERT INTO %s(%s) VALUES %s", "BayonetStatus",chClounName, chValues);

		_variant_t RecordsAffected;
		m_pConnectionPtr->Execute((_bstr_t)chCmdTextp, &RecordsAffected, adCmdText);
		hr = S_OK;
	}
	catch(_com_error &e)
	{
		hr = S_FALSE;
		CString strErrorMessage;
		strErrorMessage.Format("SaveDeviceStatusToDB::�豸״̬д��ʧ�ܣ�������Ϣ:%s ������:%d,  ����������%s, LisNo = %s", e.ErrorMessage(), GetLastError(), (char*)e.Description(), chListNo);
		LocalDBWriteLog(strErrorMessage.GetBuffer());
		strErrorMessage.ReleaseBuffer();

		CString strErrorDescriptionTemp((char *)e.Description());		
		if ( -1 != strErrorDescriptionTemp.Find("PRIMARY KEY ") )
		{
			char szLog1[512]={0};
			sprintf(szLog1,"�豸״̬��¼������:%s������ֵ�ظ�,����������¼",chListNo);
			LocalDBWriteLog(szLog1);
			hr = S_OK;
		}
		else
		{
			hr = S_FALSE;
		}
		if (strstr(strErrorMessage,"һ�����������") || strstr(strErrorMessage,"��ʱ") || strstr(strErrorMessage,"3121") )
		{
			LocalDBWriteLog("SaveDeviceStatusToDB:: ��ѯ��ʱ");
			if (adStateOpen == m_pConnectionPtr->GetState())
			{
				m_pConnectionPtr->Close();
			}
		}
		//Sleep(5*1000);
	}
	return hr;
}

HRESULT LocalDataBaseControler::SaveNormalDataToDB( CameraResult* pRecord )
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
			LocalDBWriteLog("SaveNormalDataToDB::���ݿ�����ʧ�ܣ�������������");
			char chReConnectInfo[256] = {0};
			hr =ConnectToDB(chReConnectInfo);
			char strConnectInfo[MAX_PATH] = {0};
			if (S_OK != hr)
			{
				sprintf(strConnectInfo, "SaveNormalDataToDB:: ����ʧ�ܣ�%s", chReConnectInfo);
				LocalDBWriteLog(strConnectInfo);

				return hr;
			}
			else
			{
				sprintf(strConnectInfo, "SaveNormalDataToDB:: �����ɹ���%s", chReConnectInfo);
				LocalDBWriteLog(strConnectInfo);
			}	
		}

		_variant_t RecordAffected;
		char chCmdText[1024] = {0};
		char chValues[1024] = {0};
		char chColumns[1024] = {0};

		char chUpLoad[2]={0};
		sprintf(chUpLoad,"%d",1);

		sprintf(chColumns, "%s", "ListNo,DeviceID,LaneNo ,AreaNo ,RoadNo,Optime,DirectionNo,VehPlate,VehPlateManual,VehPlateSoft,PlateColorNo,VehSpeed,VehBodyColorNo	,VehBodyDeepNo,VehTypeNo ,PlateTypeNo	,UpLoad ");
		sprintf(chValues, "('%s', %d, %d, %d, %d, '%s', %d, '%s', '%s', '%s',  %d, %d, %d, %d, %d, %d, %s)",
			pRecord->chListNo,
			pRecord->iDeviceID,
			pRecord->iLaneNo,
			pRecord->iAreaNo,
			pRecord->iRoadNo,
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
			chUpLoad);
		sprintf(chCmdText, "INSERT INTO %s(%s)VALUES %s", "CaptureList", chColumns, chValues);
		//m_pConnectionPtr->Execute(_bstr_t("select * from CaptureList"), &RecordAffected, adCmdText);
		m_pConnectionPtr->Execute(_bstr_t(chCmdText), &RecordAffected, adCmdText);
		hr = S_OK;
	}
	catch (_com_error &e)
	{		
		hr =S_FALSE;
		CString strErrorMessage;
		strErrorMessage.Format("SaveNormalDataToDB�����ݿ�д��ʧ�ܣ�������Ϣ:%s ������:%d , ��������: %s , ��ˮ��Ϊ��%s ",
			e.ErrorMessage(), GetLastError(), (char*)e.Description(), pRecord->chListNo);
		LocalDBWriteLog(strErrorMessage.GetBuffer());
		strErrorMessage.ReleaseBuffer();

		//��������������������ظ���ͻ��������������
		CString strErrorDescriptionTemp((char *)e.Description());		
		if ( -1 != strErrorDescriptionTemp.Find("PRIMARY KEY") )
		{
			char szLog1[512]={0};
			sprintf(szLog1,"����:%s, CarID:%d,����ֵ�ظ�,������������", pRecord->chListNo,pRecord->dwCarID);
			LocalDBWriteLog(szLog1);
			hr=S_OK;
		}

		if (strstr(strErrorMessage,"һ�����������") || strstr(strErrorMessage,"��ʱ") || strstr(strErrorMessage,"3121"))
		{
			LocalDBWriteLog("SaveNormalDataToDB����ѯ��ʱ");
			if (adStateOpen == m_pConnectionPtr->GetState())
			{
				m_pConnectionPtr->Close();
			}
		}
		//Sleep(5*1000);
	}
	return hr;
}


void LocalDataBaseControler::LocalDBWriteLog( char* logBuf )
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
	strCurrentDir.AppendFormat("\\LOG\\%04d-%02d-%02d\\LocalDBLog\\", pTM->tm_year + 1900, pTM->tm_mon + 1, pTM->tm_mday);
	MakeSureDirectoryPathExists(strCurrentDir);
	fileName.Format("%s%04d-%02d-%02d-%02d_LocalDB_%d.log", strCurrentDir, pTM->tm_year + 1900, pTM->tm_mon + 1, pTM->tm_mday, pTM->tm_hour, m_iDBId);

	EnterCriticalSection(&m_csLocalDBLog);

	FILE *file = NULL;
	file = fopen(fileName, "a+");
	if (file)
	{
		fprintf(file,"%04d-%02d-%02d %02d:%02d:%02d:%03d : %s\n",  pTM->tm_year + 1900, pTM->tm_mon + 1, pTM->tm_mday,
			pTM->tm_hour, pTM->tm_min, pTM->tm_sec, dwMS, logBuf);
		fclose(file);
		file = NULL;
	}

	LeaveCriticalSection(&m_csLocalDBLog);
}

HRESULT LocalDataBaseControler::InitCameraGroup( CCamera** CameraGroup )
{
	HRESULT HResult =S_FALSE;
	if (NULL == CameraGroup)
	{
		return HResult;
	}
	if (!IsConnect())
	{
		//���ݿ�δ����
		LocalDBWriteLog("InitCameraGroup::���ݿ�����ʧ�ܣ�������������");
		char chReConnectInfo[256] = {0};
		char chConnectLogInfo[256] = {0};
		HResult =ConnectToDB(chReConnectInfo);
		if (S_OK != HResult )
		{
			sprintf(chConnectLogInfo, "InitCameraGroup:: ����ʧ�ܣ�%s",chReConnectInfo);
			LocalDBWriteLog(chConnectLogInfo);
			return HResult;
		}
		else
		{
			sprintf(chConnectLogInfo, "InitCameraGroup:: �����ɹ���%s",chReConnectInfo);
			LocalDBWriteLog(chConnectLogInfo);			
		}
	}
	try
	{
		_RecordsetPtr pCameraSetRecord = NULL;
		if (S_OK != pCameraSetRecord.CreateInstance(__uuidof(Recordset)) )
		{
			//���ݼ�����ʧ��
			HResult = S_FALSE;	
			return HResult;
		}
		HRESULT hrOpen = S_FALSE;
		hrOpen = pCameraSetRecord->Open((const _variant_t)("select * from  Hve_Addr"), 
			(_variant_t)((IDispatch *)m_pConnectionPtr),
			adOpenKeyset,
			adLockOptimistic,
			adCmdText);

		if (S_OK != hrOpen)
		{
			return S_FALSE;
		}
		pCameraSetRecord->MoveFirst();

		while(VARIANT_FALSE == pCameraSetRecord->adoEOF)
		{
			CString strHveAddr = (char*)_bstr_t(pCameraSetRecord->GetCollect("IpAddr"));
			CString strKKInfo1= (char*)_bstr_t(pCameraSetRecord->GetCollect("KKInfo1"));
			CString strKKInfo2= (char*)_bstr_t(pCameraSetRecord->GetCollect("KKInfo2"));

			int    nDirection =(int)pCameraSetRecord->GetCollect("DirectionNo");
			int    nDevicID=(int)pCameraSetRecord->GetCollect("DevicID");
			int    nRoadCount=(int) pCameraSetRecord->GetCollect("RoadCount");
			int    nLaneStartNo=(int) pCameraSetRecord->GetCollect("LaneStartNo");
			int    nAreaNo=(int) pCameraSetRecord->GetCollect("AreaNo");
			int    nRoadNo=(int)pCameraSetRecord->GetCollect("RoadNo");

			strHveAddr.TrimRight();
			strKKInfo1.TrimRight();
			strKKInfo2.TrimRight();

			(*CameraGroup) = new (std::nothrow) CCamera(strHveAddr.GetBuffer());
			strHveAddr.ReleaseBuffer();
			if (NULL != (*CameraGroup))
			{
				sprintf((*CameraGroup)->m_chDBKKInfo1, "%s", strKKInfo1.GetBuffer());
				strKKInfo1.ReleaseBuffer();
				sprintf((*CameraGroup)->m_chDBKKInfo2, "%s", strKKInfo2.GetBuffer());
				strKKInfo2.ReleaseBuffer();
				(*CameraGroup)->m_iDirectionNo = nDirection;
				(*CameraGroup)->m_iDiviceID = nDevicID;
				(*CameraGroup)->m_iRoadCount = nRoadCount;
				(*CameraGroup)->m_iLaneStartNo = nLaneStartNo;
				(*CameraGroup)->m_iArraNo = nAreaNo;
				(*CameraGroup)->m_iRoadNo = nRoadNo;
			}
			pCameraSetRecord->MoveNext();
			CameraGroup++;
		}
		pCameraSetRecord->Close();
		pCameraSetRecord = NULL;

		HResult = S_OK;
	}
	catch(_com_error &e)
	{
		HResult=S_FALSE;
		CString strLog;
		strLog.Format("InitCameraGroup::��ʼ���豸ʧ��,������Ϣ:%s, ����������%s",e.ErrorMessage(), (char*)e.Description());		
		LocalDBWriteLog(strLog.GetBuffer());
		strLog.ReleaseBuffer();
	}
	return HResult;
}
