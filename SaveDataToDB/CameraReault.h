#pragma once
#include "stdafx.h"
//ʹ��MFC�����л�ʱ����Ҫ�ñ����л�����̳�CObject
//typedef struct CameraIMG :public CObject
class CameraIMG :public CObject
{
public:
	CameraIMG();
	CameraIMG(const CameraIMG& CaIMG);
	~CameraIMG();
	DECLARE_SERIAL(CameraIMG)
	WORD wImgWidth;
	WORD wImgHeight;
	DWORD dwImgSize;
	WORD  wImgType; 
	char chSavePath[256];
	PBYTE pbImgData;
	void Serialize(CArchive& archive);

	CameraIMG& operator = (const CameraIMG& CaIMG);
};





//typedef struct CameraResult :public CObject
class CameraResult :public CObject
{
public:

	CameraResult();
	CameraResult(const CameraResult& CaRESULT);
	~CameraResult();
	DECLARE_SERIAL(CameraResult)

	DWORD dwCarID;				    // ����ID
	int iPlateColor;					//������ɫ����
	int iPlateTypeNo;				//�������ʹ���
	int iVehTypeNo;					//�������ʹ���
	int iLaneNo;						//������
	int iDirection;					//·�ڷ���	
	int iVehBodyColorNo;		//������ɫ
	int iVehBodyDeepNo;		//������ǳ
	DWORD64 dw64TimeMS;	//���ƻ�ȡʱ��
	int iDeviceID;					//�豸ID
	int iAreaNo;
	int iRoadNo;
	int iSpeed;

	char chDeviceIp[32];			//ʶ���豸IP
	char chPlateNO[32];			//���ƺ�
	char chListNo[256];			//������ɵ�һ�����к�	
	char chPlateTime[256];
	char chVehPlateManual[20];		//�˹�ʶ������־
	char chVehPlateSoft[20];				//���ʶ������־

	char* pcAppendInfo;	//z ������Ϣ

	CameraIMG CIMG_SpecialImage;
	CameraIMG CIMG_FullImage;
	CameraIMG CIMG_PlateImage;
	CameraIMG CIMG_BinImage;
	
	void Serialize(CArchive& archive);

	CameraResult& operator = (const CameraResult& CaRESULT);
};


typedef struct _tagSafeModeInfo
{
	int iEableSafeMode;
	char chBeginTime[256];
	char chEndTime[256];
	int index;
	int DataInfo;
	_tagSafeModeInfo()
	{
		iEableSafeMode = 0;
		memset(chBeginTime, 0, sizeof(chBeginTime));
		memset(chEndTime, 0, sizeof(chEndTime));
	}
}_tagSafeModeInfo;

typedef struct ParamValue
{
	char szRoadName[256];
	char szRoadDirection[256];
}ParamValue;