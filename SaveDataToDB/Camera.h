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

	std::string m_strIp;									//�豸IP

private:
	CLSID m_jpgClsid;
	HANDLE m_hStatusCheckThread;				//����豸״̬�߳�
	HANDLE m_hSaveResultThread;				//��������Ӳ���߳�

	bool m_bLogEnable;									//��־����
	bool m_bDbEnable;									//���ݿ⿪��
	bool m_bMidDbEnable;								//�м����ݿ⿪��
	bool m_bSynTime;

	bool m_bExit;
	// ͼƬ��С
	int m_iBigImgWidth;
	int m_iBigImgHeight;

	//����ͼ��Ϣ
	int m_iCarHeadWidthSave;	//����ͼʵ�ʿ��
	int m_iCarHeadHeightSave;	//����ͼʵ�ʸ߶�

	// ѹ��ѡ��
	int m_iCompressEnable;
	int m_iCompressQuality;
	int m_iCompressSubQuality;
	int m_iCompressSize;
	int GetEncoderClsid(const WCHAR* format, CLSID* pClsid);

	CameraResult* m_Result;				//ʶ����
	

	bool m_bResultComplete; // ����Ƿ��ѽ�����

	//�߳�ͬ���ٽ���
	CRITICAL_SECTION m_csResult;
	CRITICAL_SECTION m_csLog;
	int m_iDevType;  // �豸���ͣ�0��Ƶ���շ�վ��1ץ��ʶ��

	//���������Ӳ�̵�·��
	char m_chCachePath[MAX_PATH];			//�������·��
	char m_chRecordPath[MAX_PATH];			//�������·��

	//�������
	//CList<CameraResult*, CameraResult* &> m_ResultList;			//ʶ����
	list<CameraResult*> m_ResultList;			//���ݵ����صĶ���

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

	//����ź���
	HANDLE m_hSemaphore;

public:
	char m_chDeviceID[18];							//DeviceID	���ｫ�����ŵ�ֵ��ֵ��DviceID
	string m_strRoadName;				//��·��
	_tagSafeModeInfo m_safeModeInfo;

	//------------Begin����һ������Ŀ���ݿ��Hve_Addr���еĶ���---------------------
	char m_chDBKKInfo1[MAX_PATH];		//�����ַ����ӵ���Ϣ
	char m_chDBKKInfo2[MAX_PATH];		//�����ַ����ӵ���Ϣ
	int  m_iDirectionNo;								
	int m_iDiviceID;
	int m_iRoadCount;
	int m_iLaneStartNo;
	int m_iArraNo;
	int m_iRoadNo;
	//------------------------End����һ������Ŀ-----------------------------------

public:
	/* ����ӿ� */
	int OpenDevice();
	int CloseDevice();
	int GetCameraStatus();
	int Capture();
	
	//ʱ��ͬ��
	int SyncTime(const CTime& tmTime);

	int WriteLog(char* chText);

	/* �ڲ�ʹ�� */
	int Connect();
	int DisConnect();
	bool m_bStatusCheckThreadExit; // �߳��˳���־
	int m_iConnectStatus;   // �豸����״̬��-1�����ӣ�0���ӣ�ȡֵ��ʽ�뺯������ֵ�Ķ��屣��һ�£�
	HVAPI_HANDLE_EX m_hDevice;  // �豸���Ӿ��
	
	void AnalyseRecord(CameraResult* record);

	void CalCropRect(RECT& rectDst,
		RECT  cPlatePos,
		const int& iImgWidth,
		const int& iImgHeight,
		const int& iCropWidth,
		const int& iCropHeight,
		DWORD dwimgType = 0,
		DWORD dwNoneImage = 0);			//���㳵ͷλ��
	bool InterceptionSpecialImage(CameraResult * cResult);			//���ݴ�ͼ��ȡ����ͼ

	bool SetListAndMutex(list<CameraResult*>* localList, HANDLE* localMutex, list<CameraResult*>* RemoteList, HANDLE* remoteMutex);
	
	bool SetListAndCriticalSection(list<CameraResult*>* localList, CRITICAL_SECTION* localCriticalSection, 
		list<CameraResult*>* RemoteList, CRITICAL_SECTION* remoteCriticalSection,
		list<CameraResult*>* BackUplocalList, CRITICAL_SECTION* BackUplocalCriticalSection,
		list<CameraResult*>* BackUpRemoteList, CRITICAL_SECTION* BackUpRemoteCriticalSection);

private:
	int WriteIniFile();		
	int ReadIniFile();
	int ReadHistoryIniFile();				//�������ļ��л�ȡ��ʷ�����Ϣ
	int WriteHistoryIniFile();				//����ʷ�����Ϣд�������ļ���

	bool saveImage(CameraResult* pRecord, int iSaveToNormalOrBackUp);		//��������浽����Ӳ����, ��iSaveToNormalOrBackUpֵΪ1ʱ�����浽����Ŀ¼����ֵΪ2ʱ���浽����Ŀ¼


	BOOL MyResolveParamXml(char *pszXmlBuf, ParamValue *pParamValue);		//������·�����ƺ�·�ڷ���

	//bool SetTheFileName(CameraResult* record);

	void ReleaseResult(void);

	BOOL GetStreamLength( IStream* pStream, ULARGE_INTEGER* puliLenth );

	bool OverlayStringToImg(CameraIMG* recordSource, char* chText1, char* chText2,char* chText3,char* chText4 );

public:
	// ==================�����ǻص�����========================
	// ʶ������ʼ�ص�����
	static int _cdecl RecordInfoBeginCallBack(PVOID pUserData, DWORD dwCarID)
	{
		if ( pUserData == NULL )
			return 0;

		CCamera* pThis = (CCamera*)pUserData;
		return pThis->RecordInfoBegin(dwCarID);
	}
	int RecordInfoBegin(DWORD dwCarID);

	// ʶ���������ص�����
	static int _cdecl RecordInfoEndCallBack(PVOID pUserData, DWORD dwCarID)
	{
		if ( pUserData == NULL )
			return 0;

		CCamera* pThis = (CCamera*)pUserData;
		return pThis->RecordInfoEnd(dwCarID);
	}
	int RecordInfoEnd(DWORD dwCarID);

	// ������Ϣ�ص�
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

	// ��ͼ�ص�
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

	// ����Сͼ�ص�
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

	// ���ƶ�ֵͼ�ص�
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