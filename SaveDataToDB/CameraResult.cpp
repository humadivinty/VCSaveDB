#include "stdafx.h"
#include "CameraReault.h"


IMPLEMENT_SERIAL(CameraIMG, CObject, 1)
CameraIMG::CameraIMG()
{
	wImgWidth = 0;
	wImgHeight = 0;
	pbImgData = NULL;
	dwImgSize = 0;

    memset(chSavePath, 0, 256);

    //	for (int i = 0; i< sizeof(chSavePath); i++)
    //	{
    //		chSavePath[i] = 0;
    //	}
}

CameraIMG::CameraIMG( const CameraIMG& CaIMG )
{
	wImgWidth = CaIMG.wImgWidth;
	wImgHeight = CaIMG.wImgHeight;
	dwImgSize = CaIMG.dwImgSize;
	wImgType = CaIMG.wImgType;
    memcpy(chSavePath, CaIMG.chSavePath, 256);
    //	for (int i = 0; i< sizeof(chSavePath); i++)
    //	{
    //		chSavePath[i] = CaIMG.chSavePath[i];
    //	}
	if (NULL != CaIMG.pbImgData)
	{
		pbImgData = new BYTE[CaIMG.dwImgSize];
		for (DWORD j = 0; j< dwImgSize; j++)
		{
			pbImgData[j] = CaIMG.pbImgData[j];
		}
	}
	else
	{
		pbImgData = NULL;
	}
}

CameraIMG::~CameraIMG()
{
	wImgWidth = 0;
	wImgHeight = 0;
	if (NULL != pbImgData)
	{
		delete[] pbImgData;
		pbImgData = NULL;
	}
	dwImgSize = 0;
	memset(chSavePath, 0, sizeof(chSavePath));
}

CameraIMG& CameraIMG::operator = (const CameraIMG& CaIMG)
{
	if (this != &CaIMG)
	{
		wImgWidth = CaIMG.wImgWidth;
		wImgHeight = CaIMG.wImgHeight;
		dwImgSize = CaIMG.dwImgSize;
		wImgType = CaIMG.wImgType;
        memcpy(chSavePath, CaIMG.chSavePath, 256);
        //		for (int i = 0; i< sizeof(chSavePath); i++)
        //		{
        //			chSavePath[i] = CaIMG.chSavePath[i];
        //		}
        if (NULL != CaIMG.pbImgData)
        {
            pbImgData = new BYTE[CaIMG.dwImgSize];
            for (DWORD j = 0; j< dwImgSize; j++)
            {
                pbImgData[j] = CaIMG.pbImgData[j];
            }
        }
        else
        {
            pbImgData = NULL;
        }
    }
    return *this;
}

void CameraIMG::Serialize(CArchive& archive)
{
	CObject::Serialize(archive);
	if (archive.IsStoring())
	{
		archive<<wImgWidth;
		archive<< wImgHeight;
		archive<< dwImgSize;
		archive<<  wImgType;
		archive.Write(chSavePath,256);
		archive.Write(pbImgData,dwImgSize);

	}
	else
	{
		archive>>wImgWidth;
		archive>> wImgHeight;
		archive>> dwImgSize;
		archive>>  wImgType; 		
		archive.Read(chSavePath, 256);
		if (NULL != pbImgData)
		{
			delete []pbImgData;
			pbImgData = NULL;
		}
		pbImgData = new BYTE[dwImgSize];
		if (NULL != pbImgData)
		{
			archive.Read(pbImgData, dwImgSize);
		}
	}
}



IMPLEMENT_SERIAL(CameraResult, CObject, 1)

CameraResult::CameraResult()
{
    dwCarID = 0;
    iPlateColor = 0;
    iPlateTypeNo = 0;
    iVehTypeNo = 0;
    iLaneNo = 0;				//路口方向
    iDirection = 0;
    iVehBodyColorNo = 0;
    iVehBodyDeepNo = 0;
    dw64TimeMS = 0;
    iDeviceID = 0;
    iAreaNo = 0;
    iRoadNo = 0;
    iSpeed = 0;

    pcAppendInfo = NULL;

    memset(chDeviceIp, 32, 0);
    memset(chPlateNO, 32, 0);
    memset(chListNo, 256, 0);
    memset(chPlateTime, 256, 0);
    memset(chVehPlateManual, 20, 0);
    memset(chVehPlateSoft, 20, 0);
}

CameraResult::CameraResult( const CameraResult& CaRESULT )
{
    dwCarID = CaRESULT.dwCarID;
    iPlateColor = CaRESULT.iPlateColor;
    iPlateTypeNo = CaRESULT.iPlateTypeNo;
    iVehTypeNo = CaRESULT.iVehTypeNo;
    iLaneNo = CaRESULT.iLaneNo;				//路口方向
    iDirection = CaRESULT.iDirection;
    iVehBodyColorNo = CaRESULT.iVehBodyColorNo;
    iVehBodyDeepNo = CaRESULT.iVehBodyDeepNo;
    dw64TimeMS = CaRESULT.dw64TimeMS;
    iDeviceID = CaRESULT.iDeviceID;
    iAreaNo = CaRESULT.iAreaNo;
    iRoadNo = CaRESULT.iRoadNo;
    iSpeed = CaRESULT.iSpeed;

    pcAppendInfo = NULL;

    memcpy(chDeviceIp, CaRESULT.chDeviceIp, sizeof(chDeviceIp));
    memcpy(chPlateNO, CaRESULT.chPlateNO, sizeof(chPlateNO));
    memcpy(chListNo, CaRESULT.chListNo, sizeof(chListNo));
    memcpy(chPlateTime, CaRESULT.chPlateTime, sizeof(chPlateTime));
    memcpy(chVehPlateManual, CaRESULT.chVehPlateManual, sizeof(chVehPlateManual));
    memcpy(chVehPlateSoft, CaRESULT.chVehPlateSoft, sizeof(chVehPlateSoft));

	CIMG_SpecialImage = CaRESULT.CIMG_SpecialImage;
	CIMG_FullImage = CaRESULT.CIMG_FullImage;
	CIMG_PlateImage = CaRESULT.CIMG_PlateImage;
	CIMG_BinImage = CaRESULT.CIMG_BinImage;
}

CameraResult::~CameraResult()
{
	iLaneNo = 0;
	iVehTypeNo = 0;
	iPlateColor = -1;
	iDirection = 0;
	iVehBodyColorNo = 0;
	iPlateTypeNo = 0;
	iVehBodyDeepNo = 0;
	iDeviceID = 0;
	iRoadNo = 0;
	iSpeed = 0;
	memset(chDeviceIp, 0, sizeof(chDeviceIp));
	memset(chPlateNO, 0, sizeof(chPlateNO));
	memset(chListNo, 0, sizeof(chListNo));
	memset(chPlateTime, 0, sizeof(chPlateNO));
	memset(chVehPlateManual, 0, sizeof(chVehPlateManual));
	memset(chVehPlateSoft, 0, sizeof(chVehPlateSoft));

	if (NULL != pcAppendInfo)
	{
		delete[] pcAppendInfo;
		pcAppendInfo = NULL;
	}
}

CameraResult& CameraResult::operator=( const CameraResult& CaRESULT )
{
    dwCarID = CaRESULT.dwCarID;
    iPlateColor = CaRESULT.iPlateColor;
    iPlateTypeNo = CaRESULT.iPlateTypeNo;
    iVehTypeNo = CaRESULT.iVehTypeNo;
    iLaneNo = CaRESULT.iLaneNo;				//路口方向
    iDirection = CaRESULT.iDirection;
    iVehBodyColorNo = CaRESULT.iVehBodyColorNo;
    iVehBodyDeepNo = CaRESULT.iVehBodyDeepNo;
    dw64TimeMS = CaRESULT.dw64TimeMS;
    iDeviceID = CaRESULT.iDeviceID;
    iAreaNo = CaRESULT.iAreaNo;
    iRoadNo = CaRESULT.iRoadNo;
    iSpeed = CaRESULT.iSpeed;

    pcAppendInfo = NULL;

    memcpy(chDeviceIp, CaRESULT.chDeviceIp, sizeof(chDeviceIp));
    memcpy(chPlateNO, CaRESULT.chPlateNO, sizeof(chPlateNO));
    memcpy(chListNo, CaRESULT.chListNo, sizeof(chListNo));
    memcpy(chPlateTime, CaRESULT.chPlateTime, sizeof(chPlateTime));
    memcpy(chVehPlateManual, CaRESULT.chVehPlateManual, sizeof(chVehPlateManual));
    memcpy(chVehPlateSoft, CaRESULT.chVehPlateSoft, sizeof(chVehPlateSoft));

	CIMG_SpecialImage = CaRESULT.CIMG_SpecialImage;
	CIMG_FullImage = CaRESULT.CIMG_FullImage;
	CIMG_PlateImage = CaRESULT.CIMG_PlateImage;
	CIMG_BinImage = CaRESULT.CIMG_BinImage;

	return *this;
}
void CameraResult::Serialize( CArchive& archive )
{
	CObject::Serialize(archive);
	
	CIMG_SpecialImage.Serialize(archive);
	CIMG_FullImage.Serialize(archive);
	CIMG_PlateImage.Serialize(archive);
	CIMG_BinImage.Serialize(archive);

	if (archive.IsStoring())
	{
		archive<<dwCarID;				    // 车辆ID
		archive<<iPlateColor;					//车牌颜色代码
		archive<<iPlateTypeNo;				//车牌类型代码
		archive<<iVehTypeNo;					//车辆类型代码
		archive<<iLaneNo;						//车道号
		archive<<iDirection;					//路口方向	
		archive<<iVehBodyColorNo;		//车身颜色
		archive<<iVehBodyDeepNo;		//车身深浅
		archive<<dw64TimeMS;	//车牌获取时间
		archive<<iDeviceID;					//设备ID
		archive<<iAreaNo;
		archive<<iRoadNo;
		archive<<iSpeed;

		archive.Write(chDeviceIp,32);	//识别设备IP
		archive.Write(chListNo,256);		//随机生成的一个序列号
		archive.Write(chPlateNO,32);		//车牌号
		archive.Write(chPlateTime,256);	//如果回调函数返回来的deTime错误，就用当时的本机时间来代替	
		archive.Write(chVehPlateManual,20);		//人工识别结果标志
		archive.Write(chVehPlateSoft,20);				//软件识别结果标志

	}
	else
	{
		archive>>dwCarID;				    // 车辆ID
		archive>>iPlateColor;					//车牌颜色代码
		archive>>iPlateTypeNo;				//车牌类型代码
		archive>>iVehTypeNo;					//车辆类型代码
		archive>>iLaneNo;						//车道号
		archive>>iDirection;					//路口方向	
		archive>>iVehBodyColorNo;		//车身颜色
		archive>>iVehBodyDeepNo;		//车身深浅
		archive>>dw64TimeMS;	//车牌获取时间
		archive>>iDeviceID;					//设备ID
		archive>>iAreaNo;
		archive>>iRoadNo;
		archive>>iSpeed;

		archive.Read(chDeviceIp,32);	//识别设备IP
		archive.Read(chListNo,256);		//随机生成的一个序列号
		archive.Read(chPlateNO,32);		//车牌号
		archive.Read(chPlateTime,256);	//如果回调函数返回来的deTime错误，就用当时的本机时间来代替	
		archive.Read(chVehPlateManual,20);		//人工识别结果标志
		archive.Read(chVehPlateSoft,20);				//软件识别结果标志
	}
}
