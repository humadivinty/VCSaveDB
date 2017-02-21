#include "stdafx.h"
#include "Camera.h"
#include<atltime.h>
//#include <vld.h>
#include <new>
#define MAX_LIST_COUNT 200

extern CRITICAL_SECTION g_csFile;

DWORD WINAPI StatusCheckThread(LPVOID lpParam);

CCamera::CCamera(void)
{
	m_bExit = false;

	m_hStatusCheckThread = NULL;
	m_bLogEnable = false;

	m_strIp = "172.18.1.1";

	m_Result = NULL;
	m_bDbEnable =false;
	m_bMidDbEnable = false;


	m_iBigImgWidth = 1600;
	m_iBigImgHeight = 1200;
	m_iCompressEnable = 0;
	m_iCompressQuality = 70;
	m_iCompressSubQuality = 5;
	m_iCompressSize = 200 * 1024;

	m_iCarHeadWidthSave = 1920;	//����ͼʵ�ʿ��
	m_iCarHeadHeightSave = 1080;	//����ͼʵ�ʸ߶�

	m_hDevice = NULL;
	m_iConnectStatus = -1;
	m_iDevType = 0;
	m_bStatusCheckThreadExit = true;
	m_bSynTime = false;

	InitializeCriticalSection( &m_csResult );
	InitializeCriticalSection (&m_csLog);

	//memset(&m_HvResult, 0, sizeof(m_HvResult));
	memset(&m_safeModeInfo, 0, sizeof(m_safeModeInfo));
	memset(m_chDeviceID, 0, sizeof(m_chDeviceID));

	

	m_bResultComplete = false;
	//setDeviceID();
	GetEncoderClsid(L"image/jpeg", &m_jpgClsid);

	m_hSaveResultThread = NULL;
	m_hSemaphore = NULL;

	g_lsLocalData = NULL;
	g_lsRemoteData = NULL;
	g_hLocalListMutex = NULL;
	g_hRemoteListMutex = NULL;
	g_csLocalCriticalSection = NULL;
	g_csRemoteCriticalSection = NULL;
}

CCamera::CCamera(char* IP)
{
	m_bExit = false;
	m_hStatusCheckThread = NULL;
	m_bLogEnable = false;

	m_strIp = IP;

	m_Result = NULL;

	m_iCarHeadWidthSave = 1920;	//����ͼʵ�ʿ��
	m_iCarHeadHeightSave = 1080;	//����ͼʵ�ʸ߶�

	m_iBigImgWidth = 1600;
	m_iBigImgHeight = 1200;
	m_iCompressEnable = 0;
	m_iCompressQuality = 70;
	m_iCompressSubQuality = 5;
	m_iCompressSize = 200 * 1024;

	//m_iDeviceID = 0;

	m_hDevice = NULL;
	m_iConnectStatus = -1;
	m_iDevType = 0;
	m_bStatusCheckThreadExit = true;
	m_bSynTime = false;

	InitializeCriticalSection( &m_csResult );
	InitializeCriticalSection (&m_csLog);


	memset(&m_safeModeInfo, 0, sizeof(m_safeModeInfo));
	memset(m_chDeviceID, 0, sizeof(m_chDeviceID));

	m_bResultComplete = false;
	//setDeviceID();
	GetEncoderClsid(L"image/jpeg", &m_jpgClsid);

	m_hSaveResultThread = NULL;
	m_hSemaphore = NULL;

	g_lsLocalData = NULL;
	g_lsRemoteData = NULL;
	g_hLocalListMutex = NULL;
	g_hRemoteListMutex = NULL;
	g_csLocalCriticalSection = NULL;
	g_csRemoteCriticalSection = NULL;
}

CCamera::~CCamera(void)
{	
	CloseDevice();
	
	ReleaseResult();
	m_bExit = true;
	if (NULL != m_Result)
	{
		delete m_Result;
		m_Result = NULL;
	}
	DeleteCriticalSection( &m_csResult );
	DeleteCriticalSection(&m_csLog);
	g_lsLocalData = NULL;
	g_lsRemoteData = NULL;
	g_hLocalListMutex = NULL;
	g_hRemoteListMutex = NULL;
	g_csLocalCriticalSection = NULL;
	g_csRemoteCriticalSection = NULL;
}
// ʶ������ʼ�ص�����
int CCamera::RecordInfoBegin(DWORD dwCarID)
{
	EnterCriticalSection(&m_csResult);
	try
	{
		m_bResultComplete = false;

		m_Result = NULL;
		m_Result = new CameraResult();

		if (NULL != m_Result)
		{
			m_Result->dwCarID = dwCarID;
			sprintf(m_Result->chDeviceIp, m_strIp.c_str());

			//�������к�
			SYSTEMTIME st;	
			GetLocalTime(&st);
			WORD wMilliseconds_Temp = st.wMilliseconds/10;
			GUID guid;

			std::string strIPTemp(m_Result->chDeviceIp);
			int iTempIP1 = 0;
			int iTempIP2 = 0;
			int iTempIP3 = 0;
			int ITempIP4 = 0;
			sscanf(strIPTemp.c_str(),"%d.%d.%d.%d",&iTempIP1,&iTempIP2,&iTempIP3,&ITempIP4);

			if (S_OK == ::CoCreateGuid(&guid))
			{
				sprintf(m_Result->chListNo, "%04X%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X"
					,st.wYear, st.wMonth, st.wDay
					,st.wHour, st.wMinute
					,st.wSecond, wMilliseconds_Temp
					,iTempIP1, iTempIP2
					,iTempIP3, ITempIP4, guid.Data4[4], guid.Data4[5]
					,guid.Data4[6], guid.Data4[7]
					);
			}

		}
	}
	catch (...)
	{
		WriteLog("�����������ʱ�����쳣�������ý��");
		if (NULL != m_Result)
		{
			delete m_Result;
			m_Result = NULL;
		}
	}
	LeaveCriticalSection(&m_csResult);
	return 0;
}

// ʶ���������ص�����
int CCamera::RecordInfoEnd(DWORD dwCarID)
{
	EnterCriticalSection(&m_csResult);
	//������ʷ��¼��Ϣ
	if (m_safeModeInfo.iEableSafeMode == 1)
	{
		SYSTEMTIME st;	
		GetLocalTime(&st);
		sprintf(m_safeModeInfo.chBeginTime, "%d.%d.%d_%d", st.wYear, st.wMonth, st.wDay, st.wHour);
		WriteHistoryIniFile();
	}
	if (NULL == m_Result)
	{
		return -1;
	}
	
	m_bResultComplete = true;

	//������������
	//if (m_ResultList.GetCount() < MAX_LIST_COUNT)
	if (m_ResultList.size() < MAX_LIST_COUNT)
	{
		//int resultCount = m_ResultList.GetCount();
		//m_ResultList.AddTail(m_Result);

		m_ResultList.push_back(m_Result);

		long previousCount;			

		if (NULL != m_Result)
		{
			//��������������
			//WaitForSingleObject(g_hLocalListMutex, 70l);				//2015-01-19
			if (m_bDbEnable)
			{
				CameraResult *tempLocalResult =NULL;
				EnterCriticalSection(g_csLocalCriticalSection);
				tempLocalResult = new CameraResult(*m_Result);
				if(NULL != tempLocalResult)
				{
					g_lsLocalData->push_back(tempLocalResult);
				}
				LeaveCriticalSection(g_csLocalCriticalSection);
			}			
			//ReleaseMutex(g_hLocalListMutex);							//2015-01-19
			

			//WaitForSingleObject(g_hRemoteListMutex, 70l);			//2015-01-19
			if (m_bMidDbEnable)
			{
				EnterCriticalSection(g_csRemoteCriticalSection);
				CameraResult *tempRemoteResult = NULL;
				tempRemoteResult =new CameraResult(*m_Result);
				if (NULL != tempRemoteResult)
				{
					g_lsRemoteData->push_back(tempRemoteResult);
				}
				LeaveCriticalSection(g_csRemoteCriticalSection);
			}
			//ReleaseMutex(g_hRemoteListMutex);						//2015-01-19
			
		}
		m_Result = NULL;
		ReleaseSemaphore(m_hSemaphore,1,&previousCount);	
	}
	else
	{
		char szLog[256] = {0};
		sprintf(szLog, "�������ˣ�ֱ��ɾ����� < CarID : %08d, ���� : %s > \n", m_Result->dwCarID, m_Result->chPlateNO);
		WriteLog(szLog);

		if (m_Result != NULL)
		{
			delete m_Result;
			m_Result = NULL;
		}
	}

	LeaveCriticalSection(&m_csResult);
	return 0;
}

// ������Ϣ�ص�
int CCamera::RecordInfoPlate(DWORD dwCarID, 
							 LPCSTR pcPlateNo, 
							 LPCSTR pcAppendInfo, 
							 DWORD dwRecordType,
							 DWORD64 dw64TimeMS)
{
	EnterCriticalSection(&m_csResult);

	m_bResultComplete = false;

	if (NULL == m_Result)
	{
		return -1;
	}

	char*  tempPtr;
	if (m_Result->dwCarID == dwCarID)
	{
		if (NULL != m_Result->pcAppendInfo)
		{
			delete [] m_Result->pcAppendInfo;
			m_Result->pcAppendInfo = NULL;
		}
		m_Result->dw64TimeMS = dw64TimeMS;
		sprintf(m_Result->chPlateNO, "%s",pcPlateNo);

		tempPtr = new char[4048];
		memset(tempPtr, NULL,4048);
		HVAPIUTILS_ParsePlateXmlStringEx(pcAppendInfo, tempPtr, 4096);
		m_Result->pcAppendInfo = tempPtr;
	}
	//�Խ�����д���
	AnalyseRecord(m_Result);

	LeaveCriticalSection(&m_csResult);
	return 0;
}

// ��ͼ�ص�
int CCamera::RecordInfoBigImage(DWORD dwCarID, 
								WORD  wImgType,
								WORD  wWidth,
								WORD  wHeight,
								PBYTE pbPicData,
								DWORD dwImgDataLen,
								DWORD dwRecordType,
								DWORD64 dw64TimeMS)
{
	EnterCriticalSection(&m_csResult);

	m_bResultComplete = false;

	if (NULL == m_Result)
	{
		return -1;
	}

	if (m_Result->dwCarID == dwCarID)
	{
		if (NULL != m_Result->CIMG_FullImage.pbImgData)
		{
			delete[] m_Result->CIMG_FullImage.pbImgData;
			m_Result->CIMG_FullImage.pbImgData = NULL;
		}

		m_Result->CIMG_FullImage.pbImgData = new(std::nothrow) BYTE[dwImgDataLen];
		m_Result->CIMG_FullImage.dwImgSize = 0;
		if (  NULL != m_Result->CIMG_FullImage.pbImgData)
		{
			memcpy(m_Result->CIMG_FullImage.pbImgData, pbPicData, dwImgDataLen);	
			m_Result->CIMG_FullImage.wImgWidth = wWidth;
			m_Result->CIMG_FullImage.wImgHeight = wHeight;		
			m_Result->CIMG_FullImage.dwImgSize = dwImgDataLen;
			m_Result->CIMG_FullImage.wImgType = wImgType;


			//���ͼ�����ַ�
			char chText3[256]={0};
			sprintf(chText3,"%s  %d����/Сʱ", m_Result->chPlateNO, m_Result->iSpeed);
			bool bOverlayResult = OverlayStringToImg(&(m_Result->CIMG_FullImage), m_chDBKKInfo1, m_chDBKKInfo2, chText3, m_Result->chPlateTime);


			//��ȡ����ͼ
			//bool bRet = false;
			//bRet = InterceptionSpecialImage(m_Result);
			//if (false == bRet)
			//{
			//	if (NULL != m_Result->CIMG_SpecialImage.pbImgData)
			//	{
			//		delete [] m_Result->CIMG_SpecialImage.pbImgData;
			//		m_Result->CIMG_SpecialImage.pbImgData = NULL;
			//	}

			//	//�����ȡʧ�ܣ����ô�ͼ����
			//	m_Result->CIMG_SpecialImage.pbImgData = new BYTE[dwImgDataLen];
			//	m_Result->CIMG_SpecialImage.dwImgSize = 0;

			//	WriteLog("����ͼ��ȡʧ�ܣ�ʹ�ô�ͼ����");
			//	memcpy(m_Result->CIMG_SpecialImage.pbImgData, pbPicData, dwImgDataLen);	
			//	m_Result->CIMG_SpecialImage.wImgWidth = wWidth;
			//	m_Result->CIMG_SpecialImage.wImgHeight = wHeight;		
			//	m_Result->CIMG_SpecialImage.dwImgSize = dwImgDataLen;
			//	m_Result->CIMG_SpecialImage.wImgType = wImgType;
			//}
		}
	}

	LeaveCriticalSection(&m_csResult);
	return 0;
}

// ����Сͼ�ص�
int CCamera::RecordInfoSmallImage(DWORD dwCarID,
								  WORD wWidth,
								  WORD wHeight,
								  PBYTE pbPicData,
								  DWORD dwImgDataLen,
								  DWORD dwRecordType,
								  DWORD64 dw64TimeMS)
{
	EnterCriticalSection(&m_csResult);

	m_bResultComplete = false;

	if (NULL == m_Result)
	{
		return -1;
	}

	int iBuffLen = 100 * 1024;
	if (m_Result->dwCarID == dwCarID)
	{
		if (NULL != m_Result->CIMG_PlateImage.pbImgData)
		{
			delete[] m_Result->CIMG_PlateImage.pbImgData;
			m_Result->CIMG_PlateImage.pbImgData = NULL;
		}
		m_Result->CIMG_PlateImage.pbImgData = new BYTE[iBuffLen];
		if (m_Result->CIMG_PlateImage.pbImgData != NULL)
		{
			memset(m_Result->CIMG_PlateImage.pbImgData, 0 ,102400);
			HRESULT Hr = HVAPIUTILS_SmallImageToBitmapEx(pbPicData,
				wWidth,
				wHeight,
				m_Result->CIMG_PlateImage.pbImgData,
				&iBuffLen);
			if ( Hr == S_OK)
			{
				m_Result->CIMG_PlateImage.wImgWidth = wWidth;
				m_Result->CIMG_PlateImage.wImgHeight = wHeight;
				m_Result->CIMG_PlateImage.dwImgSize = iBuffLen;
			}
		}
	}

	LeaveCriticalSection(&m_csResult);
	return 0;
}

// ���ƶ�ֵͼ�ص�
int CCamera::RecordInfoBinaryImage(DWORD dwCarID,
								   WORD wWidth,
								   WORD wHeight,
								   PBYTE pbPicData,
								   DWORD dwImgDataLen,
								   DWORD dwRecordType,
								   DWORD64 dw64TimeMS)
{
	EnterCriticalSection(&m_csResult);

	m_bResultComplete = false;

	if (NULL == m_Result)
	{
		return -1;
	}

	if (m_Result->dwCarID == dwCarID)
	{
		if (NULL != m_Result->CIMG_BinImage.pbImgData)
		{
			delete[] m_Result->CIMG_BinImage.pbImgData;
			m_Result->CIMG_BinImage.pbImgData = NULL;
		}

		m_Result->CIMG_BinImage.pbImgData = new BYTE[dwImgDataLen];
		m_Result->CIMG_BinImage.dwImgSize = 0;
		if (m_Result->CIMG_BinImage.pbImgData != NULL)
		{
			memcpy(m_Result->CIMG_BinImage.pbImgData, pbPicData, dwImgDataLen);
			m_Result->CIMG_BinImage.dwImgSize = dwImgDataLen;
			m_Result->CIMG_BinImage.wImgHeight = wHeight;
			m_Result->CIMG_BinImage.wImgWidth = wWidth;
		}
	}
	LeaveCriticalSection(&m_csResult);
	return 0;
}

// ȡͼƬ����GLSID
int CCamera::GetEncoderClsid( const WCHAR* format, CLSID* pClsid )
{
	UINT  num = 0;
	UINT  size = 0;

	ImageCodecInfo* pImageCodecInfo = NULL;

	GetImageEncodersSize(&num, &size);
	if(size == 0)
		return -1;

	pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
	if(pImageCodecInfo == NULL)
		return -1;

	GetImageEncoders(num, size, pImageCodecInfo);

	for(UINT j = 0; j < num; ++j)
	{
		if( wcscmp(pImageCodecInfo[j].MimeType, format) == 0 )
		{
			*pClsid = pImageCodecInfo[j].Clsid;
			free(pImageCodecInfo);
			return j;
		}    
	}
	free(pImageCodecInfo);
	return -1;
}


// д����
int CCamera::WriteIniFile()
{
	char chFileName[MAX_PATH];
	GetModuleFileNameA(NULL, chFileName, MAX_PATH-1);
	PathRemoveFileSpecA(chFileName);
	char chIniFileName[MAX_PATH] = { 0 };
	sprintf(chIniFileName, "%s", chFileName);	
	strcat(chIniFileName, "\\HvConfig.ini");	

	char chTemp[256] = {0};

	//д�ļ�ʵ�ʴ洢·��
	WritePrivateProfileStringA("ResultInfo","SavePath",m_chRecordPath,chIniFileName);
	//дȡ�����ļ�·��
	WritePrivateProfileStringA("ResultInfo","CachePath",m_chCachePath,chIniFileName);		

	sprintf(chTemp, "%d", m_bLogEnable ? 1 : 0 );
	WritePrivateProfileStringA("LOG", "Enable", chTemp, chIniFileName);

	sprintf(chTemp, "%d", m_bDbEnable ? 1 : 0 );
	WritePrivateProfileStringA("LocalDataBase", "SaveEnable", chTemp, chIniFileName);

	sprintf(chTemp, "%d", m_bMidDbEnable ? 1 : 0 );
	WritePrivateProfileStringA("RemoteDataBase", "SaveEnable", chTemp, chIniFileName);

	return 0;
}

//��ȡ��ʷ��¼�����ļ�
int CCamera::ReadHistoryIniFile()
{
	char FileName[MAX_PATH];
	GetModuleFileNameA(NULL, FileName, MAX_PATH-1);

	PathRemoveFileSpecA(FileName);
	char iniFileName[MAX_PATH] = {0};
	char iniDeviceInfoName[MAX_PATH] = {0};
	strcat(iniFileName, FileName);
	strcat(iniFileName,"\\SafeModeConfig.ini");

	//��ȡ�ɿ��������ļ�
	m_safeModeInfo.iEableSafeMode = GetPrivateProfileIntA(this->m_strIp.c_str(), "SafeModeEnable", 0, iniFileName);
	GetPrivateProfileStringA(this->m_strIp.c_str(),"BeginTime", "0", m_safeModeInfo.chBeginTime, 256, iniFileName);
	GetPrivateProfileStringA(this->m_strIp.c_str(), "EndTime", "0", m_safeModeInfo.chEndTime, 256, iniFileName);
	m_safeModeInfo.index = GetPrivateProfileIntA(this->m_strIp.c_str(), "Index", 0, iniFileName);
	m_safeModeInfo.DataInfo = GetPrivateProfileIntA(this->m_strIp.c_str(), "DataType", 0, iniFileName);

	return 0;
}

int CCamera::WriteHistoryIniFile()
{
	char fileName[MAX_PATH];
	GetModuleFileNameA(NULL, fileName, MAX_PATH-1);

	PathRemoveFileSpecA(fileName);
	char iniFileName[MAX_PATH] = {0};
	char iniDeviceInfoName[MAX_PATH] = {0};
	strcat(iniFileName, fileName);
	strcat(iniFileName, "\\SafeModeConfig.ini");

	//��ȡ�����ļ�
	char chTemp[256] = {0};
	sprintf(chTemp, "%d", m_safeModeInfo.iEableSafeMode);

	if(m_safeModeInfo.iEableSafeMode == 0)
	{
		SYSTEMTIME st;	
		GetLocalTime(&st);
		sprintf(m_safeModeInfo.chBeginTime, "%d.%d.%d_%d", st.wYear, st.wMonth, st.wDay, st.wHour);
	}
	WritePrivateProfileStringA(this->m_strIp.c_str(), "SafeModeEnable", chTemp, iniFileName);
	WritePrivateProfileStringA(this->m_strIp.c_str(), "BeginTime", m_safeModeInfo.chBeginTime, iniFileName);
	WritePrivateProfileStringA(this->m_strIp.c_str(), "EndTime", m_safeModeInfo.chEndTime, iniFileName);
	sprintf(chTemp, "%d", m_safeModeInfo.index);
	WritePrivateProfileStringA(this->m_strIp.c_str(), "Index", chTemp, iniFileName);

	return 0;
}
// ������
int CCamera::ReadIniFile()
{
	char chFileName[MAX_PATH];
	GetModuleFileNameA(NULL, chFileName, MAX_PATH-1);
	PathRemoveFileSpecA(chFileName);
	char chIniFileName[MAX_PATH] = { 0 };
	strcpy(chIniFileName, chFileName);
	strcat(chIniFileName, "\\HvConfig.ini");

	//��ȡ�ļ�ʵ�ʴ洢·��
	GetPrivateProfileStringA("ResultInfo","SavePath","D:\\SaveRecord",m_chRecordPath,1024,chIniFileName);
	//strcat(m_chRecordPath, "\\");
	//��ȡ�����ļ�·��
	GetPrivateProfileStringA("ResultInfo","CachePath","D:\\CacheRecord",m_chCachePath,1024,chIniFileName);
	//strcat(m_chCachePath, "\\");

	int iSynTime = GetPrivateProfileIntA("ResultInfo", "SynTime", 0, chIniFileName);
	m_bSynTime = (iSynTime == 0) ? false : true;
	
	int iLocalDB = GetPrivateProfileIntA("LocalDataBase", "SaveEnable", 0, chIniFileName);
	m_bDbEnable = (iLocalDB == 0) ? false : true;

	int iRemoteDB = GetPrivateProfileIntA("RemoteDataBase", "SaveEnable", 0, chIniFileName);
	m_bMidDbEnable = (iRemoteDB == 0) ? false : true;

	int iTmp = GetPrivateProfileIntA("LOG", "Enable", 0, chIniFileName);
	m_bLogEnable = (iTmp == 0) ? false : true;

	return 0;
}


int CCamera::GetCameraStatus()
{	
	return m_iConnectStatus;
}

DWORD WINAPI StatusCheckThread(LPVOID lpParam)
{
	CCamera* pThis = (CCamera*)lpParam;
	DWORD dwStatus = -1;
	while(!pThis->m_bStatusCheckThreadExit)
	{
		if ( HVAPI_GetConnStatusEx(pThis->m_hDevice, CONN_TYPE_RECORD, &dwStatus) == S_OK )
		{
			if (dwStatus == CONN_STATUS_NORMAL 
				/*||  dwStatus == CONN_STATUS_RECONN*/
				|| dwStatus == CONN_STATUS_RECVDONE)
			{
				pThis->m_iConnectStatus = 0;
			}
			else if (dwStatus == CONN_STATUS_RECONN)
			{
				pThis->m_iConnectStatus = -1;
			}
			else
			{
				pThis->m_iConnectStatus = -1;
				pThis->Connect();
			}
		}
		else
		{
			pThis->m_iConnectStatus = -1;
			pThis->Connect();
		}

		Sleep(1000);
	}
	return 0;
}

//����������Ϣ
void CCamera::AnalyseRecord(CameraResult* record)
{	
	if (strstr(record->chPlateNO, "�޳���"))
	{
		record->iPlateColor = 0;
		record->iPlateTypeNo=0;
	}
	else
	{
		char tmpPlateColor[20] = {0};
		strncpy(tmpPlateColor, record->chPlateNO,2);
		if (strstr(tmpPlateColor, "��"))
		{
			record->iPlateColor = 3;
			record->iPlateTypeNo=2;
		}
		else if (strstr(tmpPlateColor, "��"))
		{
			record->iPlateColor = 2;
			record->iPlateTypeNo=1;
		}
		else if (strstr(tmpPlateColor, "��"))
		{
			record->iPlateColor = 4;
			record->iPlateTypeNo=3;
		}
		else if (strstr(tmpPlateColor, "��"))
		{
			record->iPlateColor = 1;
			record->iPlateTypeNo=6;
		}
		else
		{
			record->iPlateColor = 0;
			record->iPlateTypeNo=0;
		}
		//��ȡ���ƺ�
		char sztempPlate[20] = {0};
		sprintf(sztempPlate, "%s", record->chPlateNO + 2);
		//sprintf(sztempPlate, "%s", record->chPlateNO);
		if (NULL != sztempPlate)
		{
			sprintf(record->chPlateNO,"%s",sztempPlate);
		}
	}
	char * pchObj = NULL;
	pchObj = (char *)strstr(record->pcAppendInfo, "����");
	if (pchObj)
	{
		sscanf(pchObj, "����:%d", &(record->iLaneNo));
	}
	else
	{
		record->iLaneNo = 0;
	}
	pchObj = (char *)strstr(record->pcAppendInfo, "·�ڷ���");
	if (pchObj)
	{
		sscanf(pchObj, "·�ڷ���:%d", &(record->iDirection));
	}
	else
	{
		record->iDirection = 0;
	}
	if (0 != record->dw64TimeMS)
	{
		CTime tm(record->dw64TimeMS/1000);
		sprintf(record->chPlateTime, "%d-%02d-%02d %02d:%02d:%02d", tm.GetYear(), tm.GetMonth(), tm.GetDay(), tm.GetHour(), tm.GetMinute(), tm.GetSecond());
	}
	else
	{
		SYSTEMTIME st;
		GetLocalTime(&st);
		sprintf(record->chPlateTime, "%d-%02d-%02d %02d:%02d:%02d", st.wYear, st.wMonth, st.wDay	,st.wHour, st.wMinute,st.wSecond);
	}

	//------------����
	pchObj = (char*)strstr(record->pcAppendInfo, "��������");
	if(pchObj)
	{
		char szCarType[20] = {0};
		sscanf(pchObj, "��������:%s", szCarType);
		if (strstr(szCarType,"��"))
		{
			record->iVehTypeNo=2;
		}
		if (strstr(szCarType,"��"))
		{
			record->iVehTypeNo=2;
		}
		if (strstr(szCarType,"С"))
		{
			record->iVehTypeNo=1;
		}
		if(strstr(szCarType,"�ͳ�"))
		{
			record->iVehTypeNo=1;
		}
		if(strstr(szCarType,"����"))
		{
			record->iVehTypeNo=2;
		}
		if(strstr(szCarType,"ǣ����"))
		{
			record->iVehTypeNo=3;
		}
		if(strstr(szCarType,"ר����ҵ��"))
		{
			record->iVehTypeNo=4;
		}
		if(strstr(szCarType,"�糵"))
		{
			record->iVehTypeNo=5;
		}
		if(strstr(szCarType,"Ħ�г�"))
		{
			record->iVehTypeNo=6;
		}
		if(strstr(szCarType,"��������"))
		{
			record->iVehTypeNo=7;
		}
		if(strstr(szCarType,"������"))
		{
			record->iVehTypeNo=8;
		}
		if(strstr(szCarType,"��ʽ��е"))
		{
			record->iVehTypeNo=9;
		}
		if(strstr(szCarType,"ȫ�ҳ�"))
		{
			record->iVehTypeNo=10;
		}
		if(strstr(szCarType,"��ҳ�"))
		{
			record->iVehTypeNo=11;
		}
		if(strstr(szCarType,"����"))
		{
			record->iVehTypeNo=12;
		}
	}	

	//---------------������ɫ
	pchObj= (char*)strstr(record->pcAppendInfo,"������ɫ:");

	record->iVehBodyColorNo=0;
	if(pchObj)
	{
		char szBodyColour[256]={0};
		sscanf(pchObj,"������ɫ:%s",szBodyColour);
		if(strstr(szBodyColour,"��"))
			record->iVehBodyColorNo=1;

		if(strstr(szBodyColour,"��"))
			record->iVehBodyColorNo=2;

		if(strstr(szBodyColour,"��"))
			record->iVehBodyColorNo=3;

		if(strstr(szBodyColour,"��"))
			record->iVehBodyColorNo=4;

		if(strstr(szBodyColour,"��"))
			record->iVehBodyColorNo=5;

		if(strstr(szBodyColour,"��"))
			record->iVehBodyColorNo=6;

		if(strstr(szBodyColour,"��"))
			record->iVehBodyColorNo=7;

		if(strstr(szBodyColour,"��"))
			record->iVehBodyColorNo=8;

		if(strstr(szBodyColour,"��"))
			record->iVehBodyColorNo=9;

		if(strstr(szBodyColour,"��"))
			record->iVehBodyColorNo=10;
	}

	//---------������ǳ��
	pchObj = (char*)(strstr(record->pcAppendInfo,"��ǳ��:"));
	if(pchObj)
	{
		char szBodyColourDeep[256]={0};
		sscanf(pchObj,"��ǳ��:%s",szBodyColourDeep);

		if(strstr(szBodyColourDeep,"��"))
			record->iVehBodyDeepNo=1;

		if(strstr(szBodyColourDeep,"ǳ"))
			record->iVehBodyDeepNo=0;
	}
	//--------------------����
	pchObj = (char*)(strstr(record->pcAppendInfo,"����:"));
	if(pchObj)
	{
		int iCarSpeed = 0;
		sscanf(pchObj,"����:%d Km/h",&iCarSpeed);
		record->iSpeed = iCarSpeed;
	}
	record->iAreaNo = m_iArraNo;
	record->iRoadNo = m_iRoadNo;
	record->iDirection = m_iDirectionNo;
	sprintf(record->chVehPlateSoft,"%d", 1);
	sprintf(record->chVehPlateManual,"%d", 0);
	record->iDeviceID = m_iDiviceID;
}
int CCamera::Connect()
{
	if (m_hDevice != NULL)
	{
		DisConnect();
	}

	m_hDevice = HVAPI_OpenEx(m_strIp.c_str(), NULL);

	if (NULL == m_hDevice)
	{
		m_iConnectStatus = -1;
		return -1;
	}

	//��ȡ·�����Ƽ�·�ڷ��򣬽�·����������ΪDeviceID
	//char pchXmlBuf[XML_SIZE] = {0};
	//int iLen = XML_SIZE;
	//int iRetLen = 0;
	//HRESULT hr = HVAPI_GetParamEx(m_hDevice, pchXmlBuf, iLen, &iRetLen);
	//if ( S_OK == hr &&  iRetLen != 0 )
	//{
	//	ParamValue paramValueTemp={0};
	//	if (TRUE == MyResolveParamXml(pchXmlBuf, &paramValueTemp) )
	//	{
	//		//m_iDeviceID = paramValueTemp.szRoadDirection;
	//		sprintf(m_chDeviceID, "%s",paramValueTemp.szRoadName);
	//		m_strRoadName = paramValueTemp.szRoadName;
	//	}
	//}

	// ��ȡģʽ
	PROTOCOL_VERSION emProtocolVersion;
	HVAPI_GetXmlVersionEx(m_hDevice, &emProtocolVersion);

	char szRetBuf[512] = {0};
	int nRetLen = 0;
	if ( HVAPI_ExecCmdEx(m_hDevice,"OptWorkMode",szRetBuf,512, &nRetLen) == S_OK)
	{
		char szInfoValue[256] = {0};
		if ( HVAPIUTILS_GetExeCmdRetInfoEx(true, szRetBuf, "OptWorkMode", "WorkMode", szInfoValue) == S_OK )
			m_iDevType = atoi(szInfoValue);

		char cmd[256] = {0};
		sprintf(cmd, "GetWorkModeIndex,WorkModeIndex[%d]", m_iDevType);
		nRetLen = 0;
		memset(szRetBuf, 0, 512);
		memset(szInfoValue, 0, 256);
		if ( HVAPI_ExecCmdEx(m_hDevice, cmd, szRetBuf, 512, &nRetLen) == S_OK)
		{
			HVAPIUTILS_GetExeCmdRetInfoEx( (emProtocolVersion == PROTOCOL_VERSION_1 ? false : true ), szRetBuf, "GetWorkModeIndex", "WorkModeName", szInfoValue);
			if ( strstr(szInfoValue, "ץ��ʶ��") != NULL )
			{	
				//WriteLog("ץ��ʶ��");
				m_iDevType = 1;
			}
			else if (strstr(szInfoValue, "����") != NULL)
			{
				//WriteLog("ץ��ʶ��");
				m_iDevType = 1;
			}
			else
			{
				//WriteLog("��Ƶ���շ�վ");
				m_iDevType = 0;
			}
		}
	}
	//��ȡ��ʷ��¼���� �����������
	ReadHistoryIniFile();
	WriteHistoryIniFile();
	char szCommand[1024] = {0};
	sprintf(szCommand, "DownloadRecord,BeginTime[%s],Index[%d],Enable[%d],EndTime[%s],DataInfo[%d]", m_safeModeInfo.chBeginTime, m_safeModeInfo.index, m_safeModeInfo.iEableSafeMode, m_safeModeInfo.chEndTime, m_safeModeInfo.DataInfo);

	if ((HVAPI_SetCallBackEx(m_hDevice, RecordInfoBeginCallBack, this, 0, CALLBACK_TYPE_RECORD_INFOBEGIN, szCommand) != S_OK)
		|| (HVAPI_SetCallBackEx(m_hDevice, RecordInfoEndCallBack, this, 0, CALLBACK_TYPE_RECORD_INFOEND, szCommand) != S_OK)
		|| (HVAPI_SetCallBackEx(m_hDevice, RecordInfoBigImageCallBack, this, 0, CALLBACK_TYPE_RECORD_BIGIMAGE, szCommand) != S_OK)
		|| (HVAPI_SetCallBackEx(m_hDevice, RecordInfoSmallImageCallBack, this, 0, CALLBACK_TYPE_RECORD_SMALLIMAGE, szCommand) != S_OK)
		|| (HVAPI_SetCallBackEx(m_hDevice, RecordInfoBinaryImageCallBack, this, 0, CALLBACK_TYPE_RECORD_BINARYIMAGE, szCommand) != S_OK)
		|| (HVAPI_SetCallBackEx(m_hDevice, RecordInfoPlateCallBack, this, 0, CALLBACK_TYPE_RECORD_PLATE, szCommand) != S_OK)
		)
	{
		DisConnect();
		m_iConnectStatus = -1;
		return -1;
	}
	else
	{
		m_iConnectStatus = 0;
	}

	return 0;
}

int CCamera::OpenDevice()
{
	int iRet = -1;

	ReadIniFile();
	WriteIniFile();

	// �������߳�����������豸ǰ����

	//�����ź���
	m_hSemaphore = CreateSemaphore(NULL, 0, MAX_LIST_COUNT+1, NULL);

	if (m_hSemaphore != NULL)
	{
		m_hSaveResultThread = CreateThread(NULL, 0 ,(LPTHREAD_START_ROUTINE)ThreadSaveResult, this, 0, NULL);
	}

	if (m_hSaveResultThread == NULL)
	{
		char chOpenDeviceLog1[MAX_PATH] = {0};
		sprintf(chOpenDeviceLog1, "�豸%s ����SaveResultThread�߳�ʧ��", m_strIp.c_str());
		WriteLog(chOpenDeviceLog1);
		DisConnect();
		iRet = -1;

		// �������̶߳�����ʧ���ˣ������豸Ҳû������
		return iRet;
	}
	
	iRet = Connect();

	m_bStatusCheckThreadExit = false;
	m_hStatusCheckThread = CreateThread(NULL, 0, StatusCheckThread, this, 0, NULL);
	if ( m_hStatusCheckThread)
	{		
		iRet = 0;
	}
	else
	{
		m_bStatusCheckThreadExit = true;

		char chOpenDeviceLog2[MAX_PATH] = {0};
		sprintf(chOpenDeviceLog2, "�豸%s ����StatusCheckThread�߳�ʧ��", m_strIp.c_str());
		WriteLog(chOpenDeviceLog2);

		DisConnect();
		iRet = -1;
		return -1;
	}	

	return iRet;
}
//ץ�ĺ���
int CCamera::Capture()
{
	if (NULL == m_hDevice)
		return -1;

	int iRet = 0;
	char chRetBuf[1024] = {0};
	int nRetLen = 0;
	if (0 == m_iDevType)
	{
		if (HVAPI_ExecCmdEx(m_hDevice, "ForceSend", chRetBuf, 1024, &nRetLen) != S_OK)
		{
			iRet = -1;
			char chCaptureLog1[MAX_PATH] = {0};
			sprintf(chCaptureLog1, "�豸%s �ֶ�ץ��ʧ��", m_strIp.c_str());
			WriteLog(chCaptureLog1);
		}
		else
		{
			char chCaptureLog2[MAX_PATH] = {0};
			sprintf(chCaptureLog2, "�豸%s �ֶ�ץ�ĳɹ�", m_strIp.c_str());
			WriteLog(chCaptureLog2);
		}
	}
	else
	{
		if (HVAPI_ExecCmdEx(m_hDevice, "SoftTriggerCapture", chRetBuf, 1024, &nRetLen) != S_OK)
		{
			iRet = -1;
			char chCaptureLog3[MAX_PATH] = {0};
			sprintf(chCaptureLog3, "�豸%s �ֶ�ץ��SoftTriggerCaptureʧ��", m_strIp.c_str());
			WriteLog(chCaptureLog3);
		}
		else
		{
			char chCaptureLog4[MAX_PATH] = {0};
			sprintf(chCaptureLog4, "�豸%s �ֶ�ץ��SoftTriggerCapture�ɹ�", m_strIp.c_str());
			WriteLog(chCaptureLog4);
		}
	}

	return iRet;
}

// �豸�Ͽ�
int CCamera::DisConnect()
{
	if (NULL == m_hDevice)
		return -1;

	HVAPI_CloseEx(m_hDevice);
	m_hDevice = NULL;
	m_iConnectStatus = -1;

	return 0;
}
int CCamera::CloseDevice()
{
	DisConnect();

	m_bStatusCheckThreadExit = true;	

	//Sleep(1000);
	if (WaitForSingleObject(m_hStatusCheckThread, 2000) == WAIT_OBJECT_0)
	{
	}	

	if (m_hStatusCheckThread != NULL)
	{
		CloseHandle(m_hStatusCheckThread);
		m_hStatusCheckThread = NULL;
	}
	
	//������������߳�
	while(m_ResultList.size()>0)
	{
		Sleep(500);
	}
	m_bExit = true;
	if (WaitForSingleObject(m_hSaveResultThread, 2000) == WAIT_OBJECT_0)
	{
		
	}
	
	if (m_hSaveResultThread != NULL)
	{
		TerminateThread(m_hSaveResultThread, -1);
		CloseHandle(m_hSaveResultThread);
		m_hSaveResultThread = NULL;
		WriteLog("�豸�������߳��˳����");
	}

	//ɾ���ź�������
	if ( m_hSemaphore != NULL )
	{
		CloseHandle(m_hSemaphore);
		m_hSemaphore = NULL;
	}	

	char chDisConnectLog1[MAX_PATH] = {0};
	sprintf(chDisConnectLog1, "�豸%s �Ͽ����ӳɹ�", m_strIp.c_str());
	WriteLog(chDisConnectLog1);

	return 0;
}


//������·�����ƺ�·�ڷ���
BOOL CCamera::MyResolveParamXml(char *pszXmlBuf, ParamValue *pParamValue)
{
	if ( pszXmlBuf == NULL || pParamValue == NULL )
		return FALSE;

	TiXmlDocument myDocument;
	if ( !myDocument.Parse(pszXmlBuf))
	{
		//OutputDebugString( " !myDocument.Parse(szResultInfo) \n");
		return FALSE;
	}

	TiXmlElement*  pRootElement = myDocument.RootElement();
	if ( pRootElement == NULL )
		return FALSE;


	TiXmlElement* pHvParam = pRootElement->FirstChildElement("HvParam");
	if ( pHvParam == NULL )
		return FALSE;

	
	/*
	TiXmlElement* pTempElment = pHvParam->FirstChildElement("Identify");
	if (pTempElment == NULL )
		return FALSE;*/
	

	TiXmlElement* pTempElment = pHvParam->FirstChildElement();
	TiXmlElement* pIdentifyElment = NULL;

	//

	while (pTempElment)
	{
		TiXmlAttribute* attribute = pTempElment->FirstAttribute();

		if ( attribute != NULL )
		{
			const char*pTemp = attribute->Value();
			if (strcmp(pTemp, "HvDsp") == 0 )
			{
				//OutputDebugString("Find Identify RoadName\n");
				//��Identify

				TiXmlElement* pDspElment = pTempElment->FirstChildElement();
				while (pDspElment)
				{
					TiXmlAttribute* attribute = pDspElment->FirstAttribute();
					if (attribute)
					{
						const char* pTemp = attribute->Value();
						if ( strcmp(pTemp, "Identify") == 0 )
						{
							pIdentifyElment = pDspElment;
							break ;
						}
					}
					pDspElment = pDspElment->NextSiblingElement();
				}
				break ;
			}
		}

		//pTempElment->FirstAttribute()
	/*	while (attribute)
		{
			//cout<<attribute->Name()<<":"<<attribute->Value()<<endl;
			OutputDebugString(attribute->Name());
			OutputDebugString(attribute->value())
			attribute = attribute->Next();
		}

		TiXmlElement* phoneElement = pTempElment->FirstChildElement();
		cout<<phoneElement->GetText()<<endl;
		TiXmlElement* addressElement = phoneElement->NextSiblingElement();
		cout<<addressElement->GetText()<<endl;*/

		pTempElment = pTempElment->NextSiblingElement();
	}


	if (pIdentifyElment == NULL)
		return FALSE;

	//
	TiXmlElement *pKeyElement = pIdentifyElment->FirstChildElement("KEY");
	while(pKeyElement != NULL )
	{

		TiXmlAttribute* attribute = pKeyElement->FirstAttribute();
		if (attribute)
		{
			const char* pTemp = attribute->Value();
			if (strcmp(pTemp, "StreetName") == 0)
			{
				TiXmlNode*  pNode = pKeyElement->FirstChild("VALUE");
				TiXmlElement* pElement = pNode->ToElement();
				if ( pElement != NULL )
				{
					const char *pTemp = pElement->GetText();
					if ( pTemp != NULL )
					{
						strcpy(pParamValue->szRoadName, pTemp);
					}
					//OutputDebugString(pTemp);
				}
			}
			else if (strcmp(pTemp, "StreetDirection") == 0)
			{
				TiXmlNode*  pNode = pKeyElement->FirstChild("VALUE");
				TiXmlElement* pElement = pNode->ToElement();
				if ( pElement != NULL )
				{
					const char *pTemp = pElement->GetText();
					if ( pTemp != NULL )
					{
						strcpy(pParamValue->szRoadDirection, pTemp);
					}
					//OutputDebugString(pTemp);
				}
			}
		}
		/*TiXmlNode*  pNode = pKeyElement->FirstChild("CHNAME");
		if ( pNode != NULL)
		{
			TiXmlElement* pElement = pNode->ToElement();
			const char *pTemp = pElement->GetText();
			if (strcmp(pTemp, "·������") == 0 )
			{
				pKeyElement->FirstChild("")
			}
		}*/

		pKeyElement= pKeyElement->NextSiblingElement();
	}

	return TRUE;

}



bool CCamera::saveImage(CameraResult* pRecord)
{
	//˼·��
	//	1�����ļ�������Ϊ��ˮ��
	//	2�����ļ����浽�����ļ������õ�·����
	//	3��·����������һ���Ǳ����ļ�·����һ����Ҫ�����м����ļ�·��
	//	4��·���½�������Ϊ��λ�������ļ���
	
	CString strBackUpMidFileName;
	CString strCacheMidFileName;

	CString strBackUpNormalFileName;
	CString strCacheNormalFileName;
	
	
	strBackUpNormalFileName.Append(m_chRecordPath);
	strCacheNormalFileName.Append(m_chCachePath);

	strBackUpMidFileName.Append(m_chRecordPath);
	strCacheMidFileName.Append(m_chCachePath);

	SYSTEMTIME st;	
	GetLocalTime(&st);

	if (m_bDbEnable)
	{
		strBackUpNormalFileName.AppendFormat("\\Local\\%d-%d-%d\\", st.wYear, st.wMonth, st.wDay);
		strCacheNormalFileName.Append("\\Local\\");

		MakeSureDirectoryPathExists(strBackUpNormalFileName);
		MakeSureDirectoryPathExists(strCacheNormalFileName);
		
		//���汸���ļ�
		CFile BackUpFile;
		strBackUpNormalFileName.AppendFormat("%s.dat", pRecord->chListNo);
		BOOL bOpenSuccess = BackUpFile.Open(strBackUpNormalFileName, CFile::modeCreate|CFile::modeWrite);
		CArchive BpNormalArchive(&BackUpFile, CArchive::store);
		pRecord->Serialize(BpNormalArchive);
		BpNormalArchive.Close();
		BackUpFile.Close();		
		if (bOpenSuccess)
		{
			char chSaveImageLog1[MAX_PATH] = {0};
			sprintf(chSaveImageLog1, "��ˮ %s �����ļ����ݱ���ɹ�", pRecord->chListNo);
			WriteLog(chSaveImageLog1);
		}
		else
		{
			char chSaveImageLog2[MAX_PATH] = {0};
			sprintf(chSaveImageLog2, "��ˮ %s �����ļ����ݱ���ʧ��", pRecord->chListNo);
			WriteLog(chSaveImageLog2);
		}

		//���滺���ļ�
		//CFile CacheNormalFile;
		//strCacheNormalFileName.AppendFormat("%s.dat", pRecord->chListNo);
		//BOOL bOpenSuccessCache = CacheNormalFile.Open(strCacheNormalFileName, CFile::modeCreate|CFile::modeWrite);
		//CArchive CacheNormalArchive(&CacheNormalFile, CArchive::store);
		//pRecord->Serialize(CacheNormalArchive);
		//CacheNormalArchive.Close();
		//CacheNormalFile.Close();
		//if (bOpenSuccessCache)
		//{
		//	char chSaveImageLog3[MAX_PATH] = {0};
		//	sprintf(chSaveImageLog3, "��ˮ %s �����ļ����ݱ���ɹ�", pRecord->chListNo);
		//	WriteLog(chSaveImageLog3);
		//}
		//else
		//{
		//	char chSaveImageLog4[MAX_PATH] = {0};
		//	sprintf(chSaveImageLog4, "��ˮ %s �����ļ����ݱ���ʧ��", pRecord->chListNo);
		//	WriteLog(chSaveImageLog4);

		//}
	}


	if (m_bMidDbEnable)
	{
		strBackUpMidFileName.AppendFormat("\\MID\\%d-%d-%d\\", st.wYear, st.wMonth, st.wDay);
		strCacheMidFileName.Append("\\MID\\");

		MakeSureDirectoryPathExists(strBackUpMidFileName);
		MakeSureDirectoryPathExists(strCacheMidFileName);

		//�����м�ⱸ���ļ�
		CFile BackUpMidFile;
		strBackUpMidFileName.AppendFormat("%s.dat", pRecord->chListNo);
		BOOL bOpenMidSuccess = BackUpMidFile.Open(strBackUpMidFileName, CFile::modeCreate|CFile::modeWrite);
		CArchive BpMidArchive(&BackUpMidFile, CArchive::store);
		pRecord->Serialize(BpMidArchive);
		BpMidArchive.Close();
		BackUpMidFile.Close();
		if (bOpenMidSuccess)
		{
			char chSaveImageLog5[MAX_PATH] = {0};
			sprintf(chSaveImageLog5, "��ˮ %s �����м���ļ����ݱ���ɹ�", pRecord->chListNo);
			WriteLog(chSaveImageLog5);
		}
		else
		{
			char chSaveImageLog6[MAX_PATH] = {0};
			sprintf(chSaveImageLog6, "��ˮ %s �����м���ļ����ݱ���ʧ��", pRecord->chListNo);
			WriteLog(chSaveImageLog6);
		}

		//�����м�⻺���ļ�
		//CFile CacheMidFile;
		//strCacheMidFileName.AppendFormat("%s.dat", pRecord->chListNo);
		//BOOL bOpenMidSuccessCache = CacheMidFile.Open(strCacheMidFileName, CFile::modeCreate|CFile::modeWrite);
		//CArchive CacheMidArchive(&CacheMidFile, CArchive::store);
		//pRecord->Serialize(CacheMidArchive);
		//CacheMidArchive.Close();
		//CacheMidFile.Close();
		//if (bOpenMidSuccessCache)
		//{
		//	char chSaveImageLog7[MAX_PATH] = {0};
		//	sprintf(chSaveImageLog7, "��ˮ %s �����м���ļ�����ʧ�ɹ�", pRecord->chListNo);
		//	WriteLog(chSaveImageLog7);
		//}
		//else
		//{
		//	char chSaveImageLog8[MAX_PATH] = {0};
		//	sprintf(chSaveImageLog8, "��ˮ %s �����м���ļ�����ʧ��", pRecord->chListNo);
		//	WriteLog(chSaveImageLog8);
		//}
	}

	return true;
}

int CCamera::SyncTime( const CTime& tmTime )
{
	if (NULL == m_hDevice || m_iConnectStatus != 0)
		return -1;
	if (!m_bSynTime)
	{
		return -1;
	}

	char chTemp[256]={ 0 };
	sprintf(chTemp, "SetTime,Date[%d.%02d.%02d],Time[%02d:%02d:%02d]", 
		tmTime.GetYear(), tmTime.GetMonth(), tmTime.GetDay(), 
		tmTime.GetHour(), tmTime.GetMinute(), tmTime.GetSecond());

	char szRetBuf[1024] = {0};
	int nRetLen = 0;
	if (m_hDevice != NULL)
	{
		try
		{
			if (HVAPI_ExecCmdEx(m_hDevice, chTemp, szRetBuf, 1024, &nRetLen) != S_OK)
			{
				char chSynTimeLogBuf1[MAX_PATH] = {0};
				sprintf(chSynTimeLogBuf1, "�豸%s ͬ��ʱ��ʧ��", m_strIp.c_str());
				WriteLog(chSynTimeLogBuf1);
				return -1;
			}
			else
			{
				char chSynTimeLogBuf2[MAX_PATH] = {0};
				sprintf(chSynTimeLogBuf2, "�豸%s ͬ��ʱ��ɹ�", m_strIp.c_str());
				WriteLog(chSynTimeLogBuf2);
			}
		} catch (...)
		{
			char chSynTimeLogBuf3[MAX_PATH] = {0};
			sprintf(chSynTimeLogBuf3, "�豸%s ͬ��ʱ���쳣", m_strIp.c_str());
			WriteLog(chSynTimeLogBuf3);
		}
	}

	return 0;
}

int CCamera::WriteLog( char* chText )
{
	if (!m_bLogEnable)
		return 0;

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
	strCurrentDir.AppendFormat("\\LOG\\%04d-%02d-%02d\\CamerLog\\%s\\", pTM->tm_year + 1900, pTM->tm_mon + 1, pTM->tm_mday,m_strIp.c_str());
	MakeSureDirectoryPathExists(strCurrentDir);
	fileName.Format("%s%04d-%02d-%02d-%02d-%s-Camera.log", strCurrentDir, pTM->tm_year + 1900, pTM->tm_mon + 1,pTM->tm_mday , pTM->tm_hour, m_strIp.c_str());

	EnterCriticalSection(&m_csLog);

	FILE *file = NULL;
	file = fopen(fileName, "a+");
	if (file)
	{
		fprintf(file,"%04d-%02d-%02d %02d:%02d:%02d:%03d : %s\n",  pTM->tm_year + 1900, pTM->tm_mon + 1, pTM->tm_mday,
			pTM->tm_hour, pTM->tm_min, pTM->tm_sec, dwMS, chText);
		fclose(file);
		file = NULL;
	}

	LeaveCriticalSection(&m_csLog);

	return 0;
}


int CCamera::SaveResultThreadFunction()
{
	while(!m_bExit)
	{
		if ( WaitForSingleObject(m_hSemaphore, 2000) != WAIT_OBJECT_0 )
		{
			continue;
		}

		//CameraResult *tempResult = m_ResultList.RemoveHead();
		CameraResult *tempResult = NULL;
		tempResult = m_ResultList.front();
		m_ResultList.pop_front();

		if ( tempResult == NULL )
			continue;
		 // ������
		saveImage(tempResult);

		// ɾ���ɵĽ��
		if (tempResult != NULL)
		{
			delete tempResult;
			tempResult = NULL;
		}
	}

	return 0;
}

void CCamera::ReleaseResult( void )
{
	//ɾ���б�����Ľ��
	//while (!m_ResultList.IsEmpty())
	//{
	//	CameraResult *pResult = NULL;
	//	pResult = m_ResultList.RemoveHead();
	//	if (pResult != NULL)
	//	{
	//		delete pResult;
	//		pResult = NULL;
	//	}
	//}

	while(m_ResultList.size() > 0)
	{
			CameraResult *pResult = NULL;
			pResult = m_ResultList.front();
			m_ResultList.pop_front();
			if (pResult != NULL)
			{
				delete pResult;
				pResult = NULL;
			}
	}
}

bool CCamera::InterceptionSpecialImage( CameraResult * tmpResult )
{
	if (NULL == tmpResult->CIMG_FullImage.pbImgData)
	{
		WriteLog("InterceptionSpecialImage::��ͼ����Ϊ�գ��޷���ȡ����ͼ");
		return false;
	}
	//��ȡ��������
	int iPlatePos[4] = {0};
	int iLen = 16;
	HVAPI_GetExtensionInfoEx(m_hDevice, tmpResult->CIMG_FullImage.wImgType, iPlatePos, &iLen);
	RECT rcPlate;
	RECT rcCrop;

	//����Ҫ��ȡ�ĳ�ͷ��������
	rcPlate.left = iPlatePos[1];
	rcPlate.top = iPlatePos[0];
	rcPlate.right = iPlatePos[3];
	rcPlate.bottom = iPlatePos[2];

	if (0 == rcPlate.left)
	{
		rcPlate.left = 53;
	}
	if (0 == rcPlate.right)
	{
		rcPlate.right = 65;
	}
	if (0 == rcPlate.top)
	{
		rcPlate.top = 57;
	}
	if (0 == rcPlate.bottom)
	{
		rcPlate.bottom = 66;
	}

	//���ݳ�����������ȡ�ĳ�ͷ����
	CalCropRect(rcCrop, rcPlate, tmpResult->CIMG_FullImage.wImgWidth, tmpResult->CIMG_FullImage.wImgHeight, m_iCarHeadWidthSave, m_iCarHeadHeightSave);

	int iImageWidth = m_iCarHeadWidthSave;
	int iImageHeight = m_iCarHeadHeightSave;
	RectF rfFeature(0, 0, (REAL)iImageWidth, (REAL)iImageHeight);

	//���쳵���ͼ
	IStream* pStream = NULL;
	CreateStreamOnHGlobal(NULL, TRUE, &pStream);
	if (NULL == pStream)
	{
		WriteLog("InterceptionSpecialImage:: pStream����ʧ�ܣ�ֹͣ����ͼ��ȡ");
		return false;
	}
	pStream->Write(tmpResult->CIMG_FullImage.pbImgData, tmpResult->CIMG_FullImage.dwImgSize, NULL);
	Bitmap bmp(pStream);

	Bitmap* pDest = new Bitmap(iImageWidth, iImageHeight, bmp.GetPixelFormat());
	if (NULL == pDest)
	{
		pStream->Release();
		pStream = NULL;

		WriteLog("InterceptionSpecialImage:: Bitmap����ʧ�ܣ�ֹͣ����ͼ��ȡ");
		return false;
	}
	Graphics grMain(pDest);
	grMain.DrawImage(&bmp,
		rfFeature,
		(REAL)rcCrop.left,
		(REAL)rcCrop.top,
		(REAL)(rcCrop.right - rcCrop.left),
		(REAL)(rcCrop.bottom - rcCrop.top),
		UnitPixel);

	IStream* pStreamOut = NULL;
	if (S_OK != CreateStreamOnHGlobal(NULL, TRUE, &pStreamOut))
	{
		delete pDest;
		pDest = NULL;
		pStream->Release();
		pStream = NULL;

		WriteLog("InterceptionSpecialImage:: pStream����ʧ�ܣ�ֹͣ����ͼ��ȡ");
		return false;
	}
	LARGE_INTEGER liTemp = {0};
	ULARGE_INTEGER uLiZero = {0};
	pStreamOut->SetSize(uLiZero);
	pStreamOut->Seek(liTemp, STREAM_SEEK_SET, NULL);

	pDest->Save(pStreamOut, &m_jpgClsid, 0);
	pStreamOut->Seek(liTemp, STREAM_SEEK_SET, NULL);

	int iSize = 500*1024;
	if (NULL != tmpResult->CIMG_SpecialImage.pbImgData)
	{
		delete [] tmpResult->CIMG_SpecialImage.pbImgData;
		tmpResult->CIMG_SpecialImage.pbImgData = NULL;
	}
	tmpResult->CIMG_SpecialImage.pbImgData = new BYTE[iSize];
	if (NULL == tmpResult->CIMG_SpecialImage.pbImgData)
	{
		delete pDest;
		pDest = NULL;

		pStreamOut->Release();
		pStreamOut = NULL;

		pStream->Release();
		pStream = NULL;

		WriteLog("InterceptionSpecialImage:: ����ͼ���洴��ʧ�ܣ�ֹͣ����ͼ��ȡ");
		return false;
	}

	ULONG iLastSize = 0;
	if (S_OK == pStreamOut->Read(tmpResult->CIMG_SpecialImage.pbImgData, iSize, &iLastSize))
	{
		tmpResult->CIMG_SpecialImage.dwImgSize = iLastSize;
		WriteLog("InterceptionSpecialImage:: ����ͼ��ȡ�ɹ�");
	}

	if (NULL != pStream)
	{
		pStream->Release();
		pStream = NULL;
	}
	if (NULL != pStreamOut)
	{
		pStreamOut->Release();
		pStreamOut = NULL;
	}
	if (NULL != pDest)
	{
		delete pDest;
		pDest = NULL;
	}

	return true;
}

//************************************
// ����˵��:    �����ͼλ�ô�С
// ����˵��:    RECT & rectDst  ���������
// ����˵��:    RECT cPlatePos  ����λ��
// ����˵��:    const int & iImgWidth       ԭͼƬ�Ŀ�
// ����˵��:    const int & iImgHeight      ԭͼƬ�ĸ�
// ����˵��:    const int & iCropWidth      Ҫ��ȡ�Ŀ�
// ����˵��:    const int & iCropHeight     Ҫ��ȡ�ĸ�
// ����˵��:    DWORD dwimgType
// ����˵��:    DWORD dwNoneImage
// ����ֵ:      void
//************************************
void CCamera::CalCropRect( RECT& rectDst, RECT cPlatePos, const int& iImgWidth, const int& iImgHeight, const int& iCropWidth, const int& iCropHeight, DWORD dwimgType /*= 0*/, DWORD dwNoneImage /*= 0*/ )
{
	if(cPlatePos.top < 101 && cPlatePos.bottom < 101 && cPlatePos.left < 101 && cPlatePos.right < 101)
	{
		cPlatePos.top = ((DWORD)(cPlatePos.top * iImgHeight))/100;
		cPlatePos.bottom = ((DWORD)(cPlatePos.bottom * iImgHeight))/100;
		cPlatePos.left = ((DWORD)(cPlatePos.left * iImgWidth))/100;
		cPlatePos.right = ((DWORD)(cPlatePos.right * iImgWidth))/100;
	}
	int iPosWidth = cPlatePos.right - cPlatePos.left;
	int iPosHeigh  = cPlatePos.bottom - cPlatePos.top;

	int iTemp = iCropWidth - iPosWidth;
	iTemp >>= 1;

	int iLeftDst = cPlatePos.left - iTemp;
	int iRightDst = cPlatePos.right + iTemp;
	iRightDst = iRightDst + (iCropWidth - (iRightDst - iLeftDst) );

	if( iCropWidth >= iImgWidth )
	{
		rectDst.left = 0;
		rectDst.right = iImgWidth;
	}
	else
	{
		if( iLeftDst < 0 )
		{
			iRightDst -= iLeftDst;
			iLeftDst = 0;
		}
		if( iRightDst > iImgWidth )
		{
			iLeftDst -= (iRightDst - iImgWidth);
			iRightDst = iImgWidth;
		}
		if( (iLeftDst & 1) == 1 )
		{
			iLeftDst -= 1;
			iRightDst -= 1;
		}
		rectDst.left = iLeftDst;
		rectDst.right = iRightDst;
	}
	int iTopDst;
	int iBottomDst;
	iTemp = iCropHeight - iPosHeigh;
	iTemp >>= 2;

	if(((DWORD)(cPlatePos.bottom - cPlatePos.top)*100) / iImgHeight >= 5)
	{	
		if(dwNoneImage == 1)
		{
			iTopDst = cPlatePos.top - (iTemp * 2);
			iBottomDst = cPlatePos.bottom + (iTemp * 2);
			iBottomDst =  iBottomDst + (iCropHeight - (iBottomDst - iTopDst));
		}
		else if(dwimgType == RECORD_BIGIMG_LAST_SNAPSHOT)
		{	
			if(iTemp > 0)
			{
				iTopDst = cPlatePos.top - iTemp + (int)(iImgHeight*0.4) ;
				iBottomDst = cPlatePos.bottom + (iTemp * 3) + (int)(iImgHeight*0.4);
				iBottomDst =  iBottomDst + (iCropHeight - (iBottomDst - iTopDst));
			}
			else
			{
				iTopDst = cPlatePos.top  + (int)(iImgHeight*0.4) ;
				iBottomDst = cPlatePos.bottom  + (int)(iImgHeight*0.4);
				iTopDst =  iTopDst - (iCropHeight - (iBottomDst - iTopDst));
			}
		}
		else
		{
			if(iTemp > 0)
			{
				if(iImgHeight > 1200)
				{
					iTopDst = cPlatePos.top - (iTemp * 2) + (int)(iImgHeight*0.15);
					iBottomDst = cPlatePos.bottom + (iTemp * 2) + (int)(iImgHeight*0.15);
				}
				else
				{
					iTopDst = cPlatePos.top - (iTemp * 2);// + (int)(iImgHeight*0.1);
					iBottomDst = cPlatePos.bottom + (iTemp * 2);// + (int)(iImgHeight*0.1);
				}
				iBottomDst =  iBottomDst + (iCropHeight - (iBottomDst - iTopDst));
			}
			else
			{
				if(iImgHeight > 1200)
				{
					iTopDst = cPlatePos.top + (int)(iImgHeight*0.15);
					iBottomDst = cPlatePos.bottom + (int)(iImgHeight*0.15);
				}
				else
				{
					iTopDst = cPlatePos.top;// + (int)(iImgHeight*0.1);
					iBottomDst = cPlatePos.bottom;// + (int)(iImgHeight*0.1);
				}
				iTopDst =  iTopDst - (iCropHeight - (iBottomDst - iTopDst));
			}
		}
	}
	else
	{
		iTopDst = cPlatePos.top - (iTemp * 3);
		iBottomDst = cPlatePos.bottom + iTemp;
		iBottomDst =  iBottomDst + (iCropHeight - (iBottomDst - iTopDst));
	}

	if( iCropHeight >= iImgHeight )
	{
		rectDst.top = 0;
		rectDst.bottom = iImgHeight;
	}
	else
	{
		if( iTopDst < 0 )
		{
			iBottomDst -= iTopDst;
			iTopDst = 0;
		}
		if( iBottomDst > iImgHeight )
		{
			iTopDst -= (iBottomDst - iImgHeight);
			iBottomDst = iImgHeight;
		}
		rectDst.top = iTopDst;
		rectDst.bottom = iBottomDst;
	}
}

BOOL CCamera::GetStreamLength( IStream* pStream, ULARGE_INTEGER* puliLenth )
{
	if (pStream == NULL)
		return FALSE;

	LARGE_INTEGER liMov;
	liMov.QuadPart = 0;

	ULARGE_INTEGER uliEnd, uliBegin;

	HRESULT hr = S_FALSE;

	hr = pStream->Seek(liMov, STREAM_SEEK_END, &uliEnd);
	if (FAILED(hr))
		return FALSE;

	hr = pStream->Seek(liMov, STREAM_SEEK_SET, &uliBegin);
	if (FAILED(hr))
		return FALSE;

	// ��ֵ�������ĳ���
	puliLenth->QuadPart = uliEnd.QuadPart - uliBegin.QuadPart;

	return TRUE;
}

bool CCamera::OverlayStringToImg( CameraIMG* recordSource, char* chText1, char* chText2,char* chText3,char* chText4 )
{
	bool bResult = false;
	IStream *pStreamIn = NULL, *pStreamOut = NULL;

	CreateStreamOnHGlobal( NULL, TRUE, &pStreamIn);
	CreateStreamOnHGlobal( NULL, TRUE, &pStreamOut);
	if (pStreamIn == NULL || pStreamOut == NULL )
	{
		if (pStreamIn != NULL) pStreamIn->Release();
		if (pStreamOut != NULL) pStreamOut->Release();
		WriteLog("OverlayStringToImg::������ʧ�ܣ�ȡ���ַ�����");
		return false;
	}
	LARGE_INTEGER liTemp = {0};
	ULARGE_INTEGER uLiZero = {0};

	pStreamIn->Seek(liTemp, STREAM_SEEK_SET, NULL);
	pStreamIn->SetSize(uLiZero);

	pStreamOut->Seek(liTemp, STREAM_SEEK_SET, NULL);
	pStreamOut->SetSize(uLiZero);

	HRESULT hrWrite = pStreamIn->Write(recordSource->pbImgData, recordSource->dwImgSize, NULL);
	if (S_OK != hrWrite)
	{
		pStreamIn->Release();
		pStreamIn = NULL;
		bResult = false;
		return bResult;
	}

	Bitmap firstBmp(pStreamIn);

	//�����ַ����������������С

	FontFamily fFamily(L"����");
	int iFontSize = 20;
	//PointF point(0, 0);
	Gdiplus::Font fontTmp(&fFamily, (REAL)iFontSize);

	StringFormat fmtString1;
	StringFormat fmtString2;
	StringFormat fmtString3;
	StringFormat fmtString4;

	fmtString1.SetLineAlignment(StringAlignmentNear);
	fmtString2.SetLineAlignment(StringAlignmentNear);
	fmtString3.SetLineAlignment(StringAlignmentNear);
	fmtString4.SetLineAlignment(StringAlignmentNear);

	SolidBrush solidBrush2(Color(255, 0, 0,255 )); //��ɫ
	Bitmap bmpDest(firstBmp.GetWidth(), firstBmp.GetHeight(), firstBmp.GetPixelFormat());

	if (firstBmp.GetLastStatus() == Ok && bmpDest.GetLastStatus() == Ok)
	{
		//CLSID jpgClsid;
		//GetEncoderClsid(L"image/jpeg", &jpgClsid);
		LARGE_INTEGER liTemp = {0};
		ULARGE_INTEGER uliTemp = { 0 };
		RectF rfMString1(20,20, (REAL)firstBmp.GetWidth(), (REAL)firstBmp.GetHeight());
		RectF rfMString2(20,50, (REAL)firstBmp.GetWidth(), (REAL)firstBmp.GetHeight());
		RectF rfMString3(1150,20, (REAL)firstBmp.GetWidth(), (REAL)firstBmp.GetHeight());
		RectF rfMString4(1150,50, (REAL)firstBmp.GetWidth(), (REAL)firstBmp.GetHeight());
		Graphics grone(&bmpDest);

		Rect rect(10,10,350,80);
		Rect rect1(1150,10,350,80);
		grone.DrawImage(&firstBmp,0,0,firstBmp.GetWidth(),firstBmp.GetHeight());
		grone.FillRectangle(                                        //����ɫ����
			&SolidBrush(Color(128, 255, 255, 255)),
			rect
			);
		grone.FillRectangle(                                        //����ɫ����
			&SolidBrush(Color(128, 255, 255, 255)),
			rect1
			);

		grone.DrawString(CStringW(chText1), -1 ,&fontTmp, rfMString1,&fmtString1, &solidBrush2); 
		grone.DrawString(CStringW(chText2), -1 ,&fontTmp, rfMString2,&fmtString2, &solidBrush2);
		grone.DrawString(CStringW(chText3), -1 ,&fontTmp, rfMString3,&fmtString3, &solidBrush2); 
		grone.DrawString(CStringW(chText4), -1 ,&fontTmp, rfMString4,&fmtString4, &solidBrush2);
		pStreamOut->SetSize(uliTemp);
		pStreamOut->Seek(liTemp, STREAM_SEEK_SET, NULL );
		//if (Ok != bmpDest.Save(pStreamOut, &jpgClsid) )
		if (Ok != bmpDest.Save(pStreamOut, &m_jpgClsid) )
		{
			bResult = false;
			if (NULL != pStreamIn)
			{
				pStreamIn->Release();
				pStreamIn = NULL;
			}
			if (NULL != pStreamOut)
			{
				pStreamOut->Release();
				pStreamOut = NULL;
			}
			WriteLog("OverlayStringToImg:: ��bmp���浽��ʧ�ܣ��˳��ַ�����");
			return bResult;
		}
		ULARGE_INTEGER uiLength;
		int iLastSize = 0;
		if (GetStreamLength(pStreamOut, &uiLength))
		{
			iLastSize = (int)uiLength.QuadPart;
		}
		//���ԭ�������ݣ�Ԥ�������µ�����
		if(NULL != recordSource->pbImgData)
		{
			delete[] recordSource->pbImgData;
			recordSource->pbImgData = NULL;
		}
		if (NULL == recordSource->pbImgData)
		{
			recordSource->pbImgData = new BYTE[iLastSize];
			recordSource->dwImgSize = iLastSize;
		}
		pStreamOut->Seek(liTemp, STREAM_SEEK_SET, NULL );
		ULONG ulSize = 0;
		if ( pStreamOut->Read(recordSource->pbImgData, recordSource->dwImgSize, &ulSize)== Ok)
		{
			recordSource->dwImgSize = ulSize;
			bResult = true;
		}
		else
		{
			recordSource->dwImgSize = 0;
			bResult = false;
		}		
	}
	else
	{
		bResult = false;
	}

	if (NULL != pStreamIn)
	{
		pStreamIn->Release();
		pStreamIn = NULL;
	}

	if (NULL != pStreamOut)
	{
		pStreamOut->Release();
		pStreamOut = NULL;
	}
	return bResult;
}

bool CCamera::SetListAndMutex( list<CameraResult*>* localList, HANDLE* localMutex, list<CameraResult*>* RemoteList, HANDLE* remoteMutex )
{
	if (NULL == localList || NULL ==localMutex || NULL == RemoteList || NULL == remoteMutex)
	{
		return false;
	}
	g_lsLocalData = localList;
	g_hLocalListMutex = localMutex;
	g_lsRemoteData = RemoteList;
	g_hRemoteListMutex = remoteMutex;

	return true;
}

bool CCamera::SetListAndCriticalSection( list<CameraResult*>* localList, CRITICAL_SECTION* localCriticalSection, list<CameraResult*>* RemoteList, CRITICAL_SECTION* remoteCriticalSection )
{
	if (NULL == localList || NULL ==localCriticalSection || NULL == RemoteList || NULL == remoteCriticalSection)
	{
		return false;
	}
	g_lsLocalData = localList;
	g_csLocalCriticalSection = localCriticalSection;
	g_lsRemoteData = RemoteList;
	g_csRemoteCriticalSection = remoteCriticalSection;

	return true;
}
