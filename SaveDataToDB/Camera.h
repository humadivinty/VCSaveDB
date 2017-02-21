//#include "stdafx.h"
#pragma once
#include "CameraReault.h"
#include <afxtempl.h>
#include <list>
#include <string>
using namespace std;

#define  SAVE_TO_CACHE_DIRECTORY 1
#define  SAVE_TO_BACKUP_DIRECTORY 2
class CCamera
{
public:
	CCamera(void);
	CCamera(char* IP);	
	~CCamera(void);

	std::string m_strIp;									//设备IP

private:
	CLSID m_jpgClsid;
	HANDLE m_hStatusCheckThread;				//检查设备状态线程
	HANDLE m_hSaveResultThread;				//保存结果到硬盘线程

	bool m_bLogEnable;									//日志开关
	bool m_bDbEnable;									//数据库开关
	bool m_bMidDbEnable;								//中间数据库开关
	bool m_bSynTime;

	bool m_bExit;
	// 图片大小
	int m_iBigImgWidth;
	int m_iBigImgHeight;

	//特征图信息
	int m_iCarHeadWidthSave;	//特征图实际宽度
	int m_iCarHeadHeightSave;	//特征图实际高度

	// 压缩选项
	int m_iCompressEnable;
	int m_iCompressQuality;
	int m_iCompressSubQuality;
	int m_iCompressSize;
	int GetEncoderClsid(const WCHAR* format, CLSID* pClsid);

	CameraResult* m_Result;				//识别结果
	

	bool m_bResultComplete; // 结果是否已接完整

	//线程同步临界区
	CRITICAL_SECTION m_csResult;
	CRITICAL_SECTION m_csLog;
	int m_iDevType;  // 设备类型，0视频流收费站，1抓拍识别

	//结果保存在硬盘的路径
	char m_chCachePath[MAX_PATH];			//结果缓存路径
	char m_chRecordPath[MAX_PATH];			//结果备份路径

	//结果队列
	//CList<CameraResult*, CameraResult* &> m_ResultList;			//识别结果
	list<CameraResult*> m_ResultList;			//备份到本地的队列

public:
	list<CameraResult*>* g_lsLocalData;
	list<CameraResult*>* g_lsRemoteData;
	HANDLE* g_hLocalListMutex;
	HANDLE* g_hRemoteListMutex;
	CRITICAL_SECTION* g_csLocalCriticalSection;
	CRITICAL_SECTION* g_csRemoteCriticalSection;

	list<CameraResult*>* g_lsBackUpLocalData;
	list<CameraResult*>* g_lsBackUpRemoteData;
	CRITICAL_SECTION* g_csBackUpLocalCriticalSection;
	CRITICAL_SECTION* g_csBackUpRemoteCriticalSection;

	//结果信号量
	HANDLE m_hSemaphore;

public:
	char m_chDeviceID[18];							//DeviceID	这里将车道号的值赋值给DviceID
	string m_strRoadName;				//道路名
	_tagSafeModeInfo m_safeModeInfo;

	//------------Begin广州一张网项目数据库表Hve_Addr特有的东西---------------------
	char m_chDBKKInfo1[MAX_PATH];		//用于字符叠加的信息
	char m_chDBKKInfo2[MAX_PATH];		//用于字符叠加的信息
	int  m_iDirectionNo;								
	int m_iDiviceID;
	int m_iRoadCount;
	int m_iLaneStartNo;
	int m_iArraNo;
	int m_iRoadNo;
	//------------------------End广州一张网项目-----------------------------------

public:
	/* 对外接口 */
	int OpenDevice();
	int CloseDevice();
	int GetCameraStatus();
	int Capture();
	
	//时间同步
	int SyncTime(const CTime& tmTime);

	int WriteLog(char* chText);

	/* 内部使用 */
	int Connect();
	int DisConnect();
	bool m_bStatusCheckThreadExit; // 线程退出标志
	int m_iConnectStatus;   // 设备连接状态，-1非连接，0连接（取值方式与函数返回值的定义保持一致）
	HVAPI_HANDLE_EX m_hDevice;  // 设备连接句柄
	
	void AnalyseRecord(CameraResult* record);

	void CalCropRect(RECT& rectDst,
		RECT  cPlatePos,
		const int& iImgWidth,
		const int& iImgHeight,
		const int& iCropWidth,
		const int& iCropHeight,
		DWORD dwimgType = 0,
		DWORD dwNoneImage = 0);			//计算车头位置
	bool InterceptionSpecialImage(CameraResult * cResult);			//根据大图截取特征图

	bool SetListAndMutex(list<CameraResult*>* localList, HANDLE* localMutex, list<CameraResult*>* RemoteList, HANDLE* remoteMutex);
	
	bool SetListAndCriticalSection(list<CameraResult*>* localList, CRITICAL_SECTION* localCriticalSection, 
		list<CameraResult*>* RemoteList, CRITICAL_SECTION* remoteCriticalSection,
		list<CameraResult*>* BackUplocalList, CRITICAL_SECTION* BackUplocalCriticalSection,
		list<CameraResult*>* BackUpRemoteList, CRITICAL_SECTION* BackUpRemoteCriticalSection);

private:
	int WriteIniFile();		
	int ReadIniFile();
	int ReadHistoryIniFile();				//从配置文件中获取历史结果信息
	int WriteHistoryIniFile();				//将历史结果信息写入配置文件中

	bool saveImage(CameraResult* pRecord, int iSaveToNormalOrBackUp);		//将结果保存到本地硬盘中, 当iSaveToNormalOrBackUp值为1时，保存到缓存目录，当值为2时保存到备份目录


	BOOL MyResolveParamXml(char *pszXmlBuf, ParamValue *pParamValue);		//解析出路口名称和路口方向

	//bool SetTheFileName(CameraResult* record);

	void ReleaseResult(void);

	BOOL GetStreamLength( IStream* pStream, ULARGE_INTEGER* puliLenth );

	bool OverlayStringToImg(CameraIMG* recordSource, char* chText1, char* chText2,char* chText3,char* chText4 );

public:
	// ==================以下是回调函数========================
	// 识别结果开始回调函数
	static int _cdecl RecordInfoBeginCallBack(PVOID pUserData, DWORD dwCarID)
	{
		if ( pUserData == NULL )
			return 0;

		CCamera* pThis = (CCamera*)pUserData;
		return pThis->RecordInfoBegin(dwCarID);
	}
	int RecordInfoBegin(DWORD dwCarID);

	// 识别结果结束回调函数
	static int _cdecl RecordInfoEndCallBack(PVOID pUserData, DWORD dwCarID)
	{
		if ( pUserData == NULL )
			return 0;

		CCamera* pThis = (CCamera*)pUserData;
		return pThis->RecordInfoEnd(dwCarID);
	}
	int RecordInfoEnd(DWORD dwCarID);

	// 车牌信息回调
	static int __cdecl RecordInfoPlateCallBack(PVOID pUserData, 
		DWORD dwCarID, 
		LPCSTR pcPlateNo, 
		LPCSTR pcAppendInfo, 
		DWORD dwRecordType,
		DWORD64 dw64TimeMS)
	{
		if (pUserData == NULL)
			return 0;

		CCamera* pThis = (CCamera*)pUserData;
		return pThis->RecordInfoPlate(dwCarID, pcPlateNo, pcAppendInfo, dwRecordType, dw64TimeMS);
	}
	int RecordInfoPlate(DWORD dwCarID, 
		LPCSTR pcPlateNo, 
		LPCSTR pcAppendInfo, 
		DWORD dwRecordType,
		DWORD64 dw64TimeMS);

	// 大图回调
	static int __cdecl RecordInfoBigImageCallBack(PVOID pUserData,
		DWORD dwCarID, 
		WORD  wImgType,
		WORD  wWidth,
		WORD  wHeight,
		PBYTE pbPicData,
		DWORD dwImgDataLen,
		DWORD dwRecordType,
		DWORD64 dw64TimeMS)
	{
		if (pUserData == NULL)
			return 0;

		CCamera* pThis = (CCamera*)pUserData;
		return pThis->RecordInfoBigImage(dwCarID, wImgType, wWidth, wHeight, pbPicData, dwImgDataLen, dwRecordType, dw64TimeMS);
	}
	int RecordInfoBigImage(DWORD dwCarID, 
		WORD  wImgType,
		WORD  wWidth,
		WORD  wHeight,
		PBYTE pbPicData,
		DWORD dwImgDataLen,
		DWORD dwRecordType,
		DWORD64 dw64TimeMS);

	// 车牌小图回调
	static int __cdecl RecordInfoSmallImageCallBack(PVOID pUserData,
		DWORD dwCarID,
		WORD wWidth,
		WORD wHeight,
		PBYTE pbPicData,
		DWORD dwImgDataLen,
		DWORD dwRecordType,
		DWORD64 dw64TimeMS)
	{
		if (pUserData == NULL)
			return 0;

		CCamera* pThis = (CCamera*)pUserData;
		return pThis->RecordInfoSmallImage(dwCarID, wWidth, wHeight, pbPicData, dwImgDataLen, dwRecordType, dw64TimeMS);
	}
	int RecordInfoSmallImage(DWORD dwCarID,
		WORD wWidth,
		WORD wHeight,
		PBYTE pbPicData,
		DWORD dwImgDataLen,
		DWORD dwRecordType,
		DWORD64 dw64TimeMS);

	// 车牌二值图回调
	static int __cdecl RecordInfoBinaryImageCallBack(PVOID pUserData,
		DWORD dwCarID,
		WORD wWidth,
		WORD wHeight,
		PBYTE pbPicData,
		DWORD dwImgDataLen,
		DWORD dwRecordType,
		DWORD64 dw64TimeMS)
	{
		if (pUserData == NULL)
			return 0;

		CCamera* pThis = (CCamera*)pUserData;
		return pThis->RecordInfoBinaryImage(dwCarID, wWidth, wHeight, pbPicData, dwImgDataLen, dwRecordType, dw64TimeMS);
	}
	int RecordInfoBinaryImage(DWORD dwCarID,
		WORD wWidth,
		WORD wHeight,
		PBYTE pbPicData,
		DWORD dwImgDataLen,
		DWORD dwRecordType,
		DWORD64 dw64TimeMS);

	static int ThreadSaveResult(LPVOID lpParame)
	{
		if (NULL == lpParame)
		{
			return -1;
		}
		CCamera* pCameraThis =  (CCamera*)lpParame;
		return pCameraThis->SaveResultThreadFunction();
	}

	int SaveResultThreadFunction();
};