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

	m_iCarHeadWidthSave = 1920;	//特征图实际宽度
	m_iCarHeadHeightSave = 1080;	//特征图实际高度

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

	m_iCarHeadWidthSave = 1920;	//特征图实际宽度
	m_iCarHeadHeightSave = 1080;	//特征图实际高度

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
// 识别结果开始回调函数
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

			//生成序列号
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
		WriteLog("创建结果对象时出现异常，丢弃该结果");
		if (NULL != m_Result)
		{
			delete m_Result;
			m_Result = NULL;
		}
	}
	LeaveCriticalSection(&m_csResult);
	return 0;
}

// 识别结果结束回调函数
int CCamera::RecordInfoEnd(DWORD dwCarID)
{
	EnterCriticalSection(&m_csResult);
	//更新历史记录信息
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

	//加入结果队列中
	//if (m_ResultList.GetCount() < MAX_LIST_COUNT)
	if (m_ResultList.size() < MAX_LIST_COUNT)
	{
		//int resultCount = m_ResultList.GetCount();
		//m_ResultList.AddTail(m_Result);

		m_ResultList.push_back(m_Result);

		long previousCount;			

		if (NULL != m_Result)
		{
			//将结果加入队列中
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
		sprintf(szLog, "队列满了，直接删除结果 < CarID : %08d, 车牌 : %s > \n", m_Result->dwCarID, m_Result->chPlateNO);
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

// 车牌信息回调
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
	//对结果进行处理
	AnalyseRecord(m_Result);

	LeaveCriticalSection(&m_csResult);
	return 0;
}

// 大图回调
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


			//向大图叠加字符
			char chText3[256]={0};
			sprintf(chText3,"%s  %d公里/小时", m_Result->chPlateNO, m_Result->iSpeed);
			bool bOverlayResult = OverlayStringToImg(&(m_Result->CIMG_FullImage), m_chDBKKInfo1, m_chDBKKInfo2, chText3, m_Result->chPlateTime);


			//获取特征图
			//bool bRet = false;
			//bRet = InterceptionSpecialImage(m_Result);
			//if (false == bRet)
			//{
			//	if (NULL != m_Result->CIMG_SpecialImage.pbImgData)
			//	{
			//		delete [] m_Result->CIMG_SpecialImage.pbImgData;
			//		m_Result->CIMG_SpecialImage.pbImgData = NULL;
			//	}

			//	//如果获取失败，则用大图代替
			//	m_Result->CIMG_SpecialImage.pbImgData = new BYTE[dwImgDataLen];
			//	m_Result->CIMG_SpecialImage.dwImgSize = 0;

			//	WriteLog("特征图获取失败，使用大图代替");
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

// 车牌小图回调
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

// 车牌二值图回调
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

// 取图片类型GLSID
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


// 写配置
int CCamera::WriteIniFile()
{
	char chFileName[MAX_PATH];
	GetModuleFileNameA(NULL, chFileName, MAX_PATH-1);
	PathRemoveFileSpecA(chFileName);
	char chIniFileName[MAX_PATH] = { 0 };
	sprintf(chIniFileName, "%s", chFileName);	
	strcat(chIniFileName, "\\HvConfig.ini");	

	char chTemp[256] = {0};

	//写文件实际存储路径
	WritePrivateProfileStringA("ResultInfo","SavePath",m_chRecordPath,chIniFileName);
	//写取缓存文件路径
	WritePrivateProfileStringA("ResultInfo","CachePath",m_chCachePath,chIniFileName);		

	sprintf(chTemp, "%d", m_bLogEnable ? 1 : 0 );
	WritePrivateProfileStringA("LOG", "Enable", chTemp, chIniFileName);

	sprintf(chTemp, "%d", m_bDbEnable ? 1 : 0 );
	WritePrivateProfileStringA("LocalDataBase", "SaveEnable", chTemp, chIniFileName);

	sprintf(chTemp, "%d", m_bMidDbEnable ? 1 : 0 );
	WritePrivateProfileStringA("RemoteDataBase", "SaveEnable", chTemp, chIniFileName);

	return 0;
}

//读取历史记录配置文件
int CCamera::ReadHistoryIniFile()
{
	char FileName[MAX_PATH];
	GetModuleFileNameA(NULL, FileName, MAX_PATH-1);

	PathRemoveFileSpecA(FileName);
	char iniFileName[MAX_PATH] = {0};
	char iniDeviceInfoName[MAX_PATH] = {0};
	strcat(iniFileName, FileName);
	strcat(iniFileName,"\\SafeModeConfig.ini");

	//读取可靠性配置文件
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

	//读取配置文件
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
// 读配置
int CCamera::ReadIniFile()
{
	char chFileName[MAX_PATH];
	GetModuleFileNameA(NULL, chFileName, MAX_PATH-1);
	PathRemoveFileSpecA(chFileName);
	char chIniFileName[MAX_PATH] = { 0 };
	strcpy(chIniFileName, chFileName);
	strcat(chIniFileName, "\\HvConfig.ini");

	//获取文件实际存储路径
	GetPrivateProfileStringA("ResultInfo","SavePath","D:\\SaveRecord",m_chRecordPath,1024,chIniFileName);
	//strcat(m_chRecordPath, "\\");
	//获取缓存文件路径
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

//解析附加信息
void CCamera::AnalyseRecord(CameraResult* record)
{	
	if (strstr(record->chPlateNO, "无车牌"))
	{
		record->iPlateColor = 0;
		record->iPlateTypeNo=0;
	}
	else
	{
		char tmpPlateColor[20] = {0};
		strncpy(tmpPlateColor, record->chPlateNO,2);
		if (strstr(tmpPlateColor, "蓝"))
		{
			record->iPlateColor = 3;
			record->iPlateTypeNo=2;
		}
		else if (strstr(tmpPlateColor, "黄"))
		{
			record->iPlateColor = 2;
			record->iPlateTypeNo=1;
		}
		else if (strstr(tmpPlateColor, "黑"))
		{
			record->iPlateColor = 4;
			record->iPlateTypeNo=3;
		}
		else if (strstr(tmpPlateColor, "白"))
		{
			record->iPlateColor = 1;
			record->iPlateTypeNo=6;
		}
		else
		{
			record->iPlateColor = 0;
			record->iPlateTypeNo=0;
		}
		//获取车牌号
		char sztempPlate[20] = {0};
		sprintf(sztempPlate, "%s", record->chPlateNO + 2);
		//sprintf(sztempPlate, "%s", record->chPlateNO);
		if (NULL != sztempPlate)
		{
			sprintf(record->chPlateNO,"%s",sztempPlate);
		}
	}
	char * pchObj = NULL;
	pchObj = (char *)strstr(record->pcAppendInfo, "车道");
	if (pchObj)
	{
		sscanf(pchObj, "车道:%d", &(record->iLaneNo));
	}
	else
	{
		record->iLaneNo = 0;
	}
	pchObj = (char *)strstr(record->pcAppendInfo, "路口方向");
	if (pchObj)
	{
		sscanf(pchObj, "路口方向:%d", &(record->iDirection));
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

	//------------车道
	pchObj = (char*)strstr(record->pcAppendInfo, "车辆类型");
	if(pchObj)
	{
		char szCarType[20] = {0};
		sscanf(pchObj, "车辆类型:%s", szCarType);
		if (strstr(szCarType,"大"))
		{
			record->iVehTypeNo=2;
		}
		if (strstr(szCarType,"中"))
		{
			record->iVehTypeNo=2;
		}
		if (strstr(szCarType,"小"))
		{
			record->iVehTypeNo=1;
		}
		if(strstr(szCarType,"客车"))
		{
			record->iVehTypeNo=1;
		}
		if(strstr(szCarType,"货车"))
		{
			record->iVehTypeNo=2;
		}
		if(strstr(szCarType,"牵引车"))
		{
			record->iVehTypeNo=3;
		}
		if(strstr(szCarType,"专项作业车"))
		{
			record->iVehTypeNo=4;
		}
		if(strstr(szCarType,"电车"))
		{
			record->iVehTypeNo=5;
		}
		if(strstr(szCarType,"摩托车"))
		{
			record->iVehTypeNo=6;
		}
		if(strstr(szCarType,"三轮汽车"))
		{
			record->iVehTypeNo=7;
		}
		if(strstr(szCarType,"拖拉机"))
		{
			record->iVehTypeNo=8;
		}
		if(strstr(szCarType,"轮式机械"))
		{
			record->iVehTypeNo=9;
		}
		if(strstr(szCarType,"全挂车"))
		{
			record->iVehTypeNo=10;
		}
		if(strstr(szCarType,"半挂车"))
		{
			record->iVehTypeNo=11;
		}
		if(strstr(szCarType,"其他"))
		{
			record->iVehTypeNo=12;
		}
	}	

	//---------------车身颜色
	pchObj= (char*)strstr(record->pcAppendInfo,"车身颜色:");

	record->iVehBodyColorNo=0;
	if(pchObj)
	{
		char szBodyColour[256]={0};
		sscanf(pchObj,"车身颜色:%s",szBodyColour);
		if(strstr(szBodyColour,"白"))
			record->iVehBodyColorNo=1;

		if(strstr(szBodyColour,"灰"))
			record->iVehBodyColorNo=2;

		if(strstr(szBodyColour,"黄"))
			record->iVehBodyColorNo=3;

		if(strstr(szBodyColour,"粉"))
			record->iVehBodyColorNo=4;

		if(strstr(szBodyColour,"红"))
			record->iVehBodyColorNo=5;

		if(strstr(szBodyColour,"紫"))
			record->iVehBodyColorNo=6;

		if(strstr(szBodyColour,"绿"))
			record->iVehBodyColorNo=7;

		if(strstr(szBodyColour,"蓝"))
			record->iVehBodyColorNo=8;

		if(strstr(szBodyColour,"棕"))
			record->iVehBodyColorNo=9;

		if(strstr(szBodyColour,"黑"))
			record->iVehBodyColorNo=10;
	}

	//---------车身深浅度
	pchObj = (char*)(strstr(record->pcAppendInfo,"深浅度:"));
	if(pchObj)
	{
		char szBodyColourDeep[256]={0};
		sscanf(pchObj,"深浅度:%s",szBodyColourDeep);

		if(strstr(szBodyColourDeep,"深"))
			record->iVehBodyDeepNo=1;

		if(strstr(szBodyColourDeep,"浅"))
			record->iVehBodyDeepNo=0;
	}
	//--------------------测速
	pchObj = (char*)(strstr(record->pcAppendInfo,"测速:"));
	if(pchObj)
	{
		int iCarSpeed = 0;
		sscanf(pchObj,"测速:%d Km/h",&iCarSpeed);
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

	//获取路口名称及路口方向，将路口名称设置为DeviceID
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

	// 获取模式
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
			if ( strstr(szInfoValue, "抓拍识别") != NULL )
			{	
				//WriteLog("抓拍识别");
				m_iDevType = 1;
			}
			else if (strstr(szInfoValue, "卡口") != NULL)
			{
				//WriteLog("抓拍识别");
				m_iDevType = 1;
			}
			else
			{
				//WriteLog("视频流收费站");
				m_iDevType = 0;
			}
		}
	}
	//读取历史记录参数 构造相关命令
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

	// 保存结果线程最好在连接设备前创建

	//生成信号量
	m_hSemaphore = CreateSemaphore(NULL, 0, MAX_LIST_COUNT+1, NULL);

	if (m_hSemaphore != NULL)
	{
		m_hSaveResultThread = CreateThread(NULL, 0 ,(LPTHREAD_START_ROUTINE)ThreadSaveResult, this, 0, NULL);
	}

	if (m_hSaveResultThread == NULL)
	{
		char chOpenDeviceLog1[MAX_PATH] = {0};
		sprintf(chOpenDeviceLog1, "设备%s 创建SaveResultThread线程失败", m_strIp.c_str());
		WriteLog(chOpenDeviceLog1);
		DisConnect();
		iRet = -1;

		// 保存结果线程都创建失败了，连接设备也没意义了
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
		sprintf(chOpenDeviceLog2, "设备%s 创建StatusCheckThread线程失败", m_strIp.c_str());
		WriteLog(chOpenDeviceLog2);

		DisConnect();
		iRet = -1;
		return -1;
	}	

	return iRet;
}
//抓拍函数
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
			sprintf(chCaptureLog1, "设备%s 手动抓拍失败", m_strIp.c_str());
			WriteLog(chCaptureLog1);
		}
		else
		{
			char chCaptureLog2[MAX_PATH] = {0};
			sprintf(chCaptureLog2, "设备%s 手动抓拍成功", m_strIp.c_str());
			WriteLog(chCaptureLog2);
		}
	}
	else
	{
		if (HVAPI_ExecCmdEx(m_hDevice, "SoftTriggerCapture", chRetBuf, 1024, &nRetLen) != S_OK)
		{
			iRet = -1;
			char chCaptureLog3[MAX_PATH] = {0};
			sprintf(chCaptureLog3, "设备%s 手动抓拍SoftTriggerCapture失败", m_strIp.c_str());
			WriteLog(chCaptureLog3);
		}
		else
		{
			char chCaptureLog4[MAX_PATH] = {0};
			sprintf(chCaptureLog4, "设备%s 手动抓拍SoftTriggerCapture成功", m_strIp.c_str());
			WriteLog(chCaptureLog4);
		}
	}

	return iRet;
}

// 设备断开
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
	
	//结束结果保存线程
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
		WriteLog("设备保存结果线程退出完毕");
	}

	//删除信号量对象
	if ( m_hSemaphore != NULL )
	{
		CloseHandle(m_hSemaphore);
		m_hSemaphore = NULL;
	}	

	char chDisConnectLog1[MAX_PATH] = {0};
	sprintf(chDisConnectLog1, "设备%s 断开连接成功", m_strIp.c_str());
	WriteLog(chDisConnectLog1);

	return 0;
}


//解析出路口名称和路口方向
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
				//找Identify

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
			if (strcmp(pTemp, "路口名称") == 0 )
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
	//思路：
	//	1、将文件名设置为流水号
	//	2、把文件保存到配置文件中配置的路径中
	//	3、路径分两个，一个是备份文件路径，一个是要存入中间库的文件路径
	//	4、路径下建立以日为单位命名的文件夹
	
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
		
		//保存备份文件
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
			sprintf(chSaveImageLog1, "流水 %s 备份文件数据保存成功", pRecord->chListNo);
			WriteLog(chSaveImageLog1);
		}
		else
		{
			char chSaveImageLog2[MAX_PATH] = {0};
			sprintf(chSaveImageLog2, "流水 %s 备份文件数据保存失败", pRecord->chListNo);
			WriteLog(chSaveImageLog2);
		}

		//保存缓存文件
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
		//	sprintf(chSaveImageLog3, "流水 %s 缓存文件数据保存成功", pRecord->chListNo);
		//	WriteLog(chSaveImageLog3);
		//}
		//else
		//{
		//	char chSaveImageLog4[MAX_PATH] = {0};
		//	sprintf(chSaveImageLog4, "流水 %s 缓存文件数据保存失败", pRecord->chListNo);
		//	WriteLog(chSaveImageLog4);

		//}
	}


	if (m_bMidDbEnable)
	{
		strBackUpMidFileName.AppendFormat("\\MID\\%d-%d-%d\\", st.wYear, st.wMonth, st.wDay);
		strCacheMidFileName.Append("\\MID\\");

		MakeSureDirectoryPathExists(strBackUpMidFileName);
		MakeSureDirectoryPathExists(strCacheMidFileName);

		//保存中间库备份文件
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
			sprintf(chSaveImageLog5, "流水 %s 备份中间库文件数据保存成功", pRecord->chListNo);
			WriteLog(chSaveImageLog5);
		}
		else
		{
			char chSaveImageLog6[MAX_PATH] = {0};
			sprintf(chSaveImageLog6, "流水 %s 备份中间库文件数据保存失败", pRecord->chListNo);
			WriteLog(chSaveImageLog6);
		}

		//保存中间库缓存文件
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
		//	sprintf(chSaveImageLog7, "流水 %s 缓存中间库文件保存失成功", pRecord->chListNo);
		//	WriteLog(chSaveImageLog7);
		//}
		//else
		//{
		//	char chSaveImageLog8[MAX_PATH] = {0};
		//	sprintf(chSaveImageLog8, "流水 %s 缓存中间库文件保存失败", pRecord->chListNo);
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
				sprintf(chSynTimeLogBuf1, "设备%s 同步时间失败", m_strIp.c_str());
				WriteLog(chSynTimeLogBuf1);
				return -1;
			}
			else
			{
				char chSynTimeLogBuf2[MAX_PATH] = {0};
				sprintf(chSynTimeLogBuf2, "设备%s 同步时间成功", m_strIp.c_str());
				WriteLog(chSynTimeLogBuf2);
			}
		} catch (...)
		{
			char chSynTimeLogBuf3[MAX_PATH] = {0};
			sprintf(chSynTimeLogBuf3, "设备%s 同步时间异常", m_strIp.c_str());
			WriteLog(chSynTimeLogBuf3);
		}
	}

	return 0;
}

int CCamera::WriteLog( char* chText )
{
	if (!m_bLogEnable)
		return 0;

	//取得当前的精确毫秒的时间
	static time_t starttime = time(NULL);
	static DWORD starttick = GetTickCount(); 
	DWORD dwNowTick = GetTickCount() - starttick;
	time_t nowtime = starttime + (time_t)(dwNowTick / 1000);
	struct tm *pTM = localtime(&nowtime);
	DWORD dwMS = dwNowTick % 1000;

	const int MAXPATH = 260;

	TCHAR szFileName[ MAXPATH] = {0};
	GetModuleFileName(NULL, szFileName, MAXPATH);	//取得包括程序名的全路径
	PathRemoveFileSpec(szFileName);				//去掉程序名
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
		 // 保存结果
		saveImage(tempResult);

		// 删除旧的结果
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
	//删除列表里面的结果
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
		WriteLog("InterceptionSpecialImage::大图数据为空，无法获取特征图");
		return false;
	}
	//获取车牌坐标
	int iPlatePos[4] = {0};
	int iLen = 16;
	HVAPI_GetExtensionInfoEx(m_hDevice, tmpResult->CIMG_FullImage.wImgType, iPlatePos, &iLen);
	RECT rcPlate;
	RECT rcCrop;

	//计算要截取的车头矩形坐标
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

	//根据车牌坐标计算截取的车头矩形
	CalCropRect(rcCrop, rcPlate, tmpResult->CIMG_FullImage.wImgWidth, tmpResult->CIMG_FullImage.wImgHeight, m_iCarHeadWidthSave, m_iCarHeadHeightSave);

	int iImageWidth = m_iCarHeadWidthSave;
	int iImageHeight = m_iCarHeadHeightSave;
	RectF rfFeature(0, 0, (REAL)iImageWidth, (REAL)iImageHeight);

	//构造车身截图
	IStream* pStream = NULL;
	CreateStreamOnHGlobal(NULL, TRUE, &pStream);
	if (NULL == pStream)
	{
		WriteLog("InterceptionSpecialImage:: pStream创建失败，停止特征图截取");
		return false;
	}
	pStream->Write(tmpResult->CIMG_FullImage.pbImgData, tmpResult->CIMG_FullImage.dwImgSize, NULL);
	Bitmap bmp(pStream);

	Bitmap* pDest = new Bitmap(iImageWidth, iImageHeight, bmp.GetPixelFormat());
	if (NULL == pDest)
	{
		pStream->Release();
		pStream = NULL;

		WriteLog("InterceptionSpecialImage:: Bitmap创建失败，停止特征图截取");
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

		WriteLog("InterceptionSpecialImage:: pStream创建失败，停止特征图截取");
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

		WriteLog("InterceptionSpecialImage:: 特征图缓存创建失败，停止特征图截取");
		return false;
	}

	ULONG iLastSize = 0;
	if (S_OK == pStreamOut->Read(tmpResult->CIMG_SpecialImage.pbImgData, iSize, &iLastSize))
	{
		tmpResult->CIMG_SpecialImage.dwImgSize = iLastSize;
		WriteLog("InterceptionSpecialImage:: 特征图截取成功");
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
// 方法说明:    计算截图位置大小
// 参数说明:    RECT & rectDst  保存计算结果
// 参数说明:    RECT cPlatePos  车牌位置
// 参数说明:    const int & iImgWidth       原图片的宽
// 参数说明:    const int & iImgHeight      原图片的高
// 参数说明:    const int & iCropWidth      要截取的宽
// 参数说明:    const int & iCropHeight     要截取的高
// 参数说明:    DWORD dwimgType
// 参数说明:    DWORD dwNoneImage
// 返回值:      void
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

	// 差值即是流的长度
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
		WriteLog("OverlayStringToImg::流申请失败，取消字符叠加");
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

	//根据字符串长度设置字体大小

	FontFamily fFamily(L"宋体");
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

	SolidBrush solidBrush2(Color(255, 0, 0,255 )); //蓝色
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
		grone.FillRectangle(                                        //填充白色背景
			&SolidBrush(Color(128, 255, 255, 255)),
			rect
			);
		grone.FillRectangle(                                        //填充白色背景
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
			WriteLog("OverlayStringToImg:: 从bmp保存到流失败，退出字符叠加");
			return bResult;
		}
		ULARGE_INTEGER uiLength;
		int iLastSize = 0;
		if (GetStreamLength(pStreamOut, &uiLength))
		{
			iLastSize = (int)uiLength.QuadPart;
		}
		//清空原来的数据，预备放入新的数据
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
