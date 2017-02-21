#pragma once
#include "stdafx.h"
//使用MFC的序列化时，需要让被序列化的类继承CObject
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

	DWORD dwCarID;				    // 车辆ID
	int iPlateColor;					//车牌颜色代码
	int iPlateTypeNo;				//车牌类型代码
	int iVehTypeNo;					//车辆类型代码
	int iLaneNo;						//车道号
	int iDirection;					//路口方向	
	int iVehBodyColorNo;		//车身颜色
	int iVehBodyDeepNo;		//车身深浅
	DWORD64 dw64TimeMS;	//车牌获取时间
	int iDeviceID;					//设备ID
	int iAreaNo;
	int iRoadNo;
	int iSpeed;

	char chDeviceIp[32];			//识别设备IP
	char chPlateNO[32];			//车牌号
	char chListNo[256];			//随机生成的一个序列号	
	char chPlateTime[256];
	char chVehPlateManual[20];		//人工识别结果标志
	char chVehPlateSoft[20];				//软件识别结果标志

	char* pcAppendInfo;	//z 附加信息

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