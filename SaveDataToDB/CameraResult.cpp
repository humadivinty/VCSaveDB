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
    iLaneNo = 0;				//·�ڷ���
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
    iLaneNo = CaRESULT.iLaneNo;				//·�ڷ���
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
    iLaneNo = CaRESULT.iLaneNo;				//·�ڷ���
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
		archive<<dwCarID;				    // ����ID
		archive<<iPlateColor;					//������ɫ����
		archive<<iPlateTypeNo;				//�������ʹ���
		archive<<iVehTypeNo;					//�������ʹ���
		archive<<iLaneNo;						//������
		archive<<iDirection;					//·�ڷ���	
		archive<<iVehBodyColorNo;		//������ɫ
		archive<<iVehBodyDeepNo;		//������ǳ
		archive<<dw64TimeMS;	//���ƻ�ȡʱ��
		archive<<iDeviceID;					//�豸ID
		archive<<iAreaNo;
		archive<<iRoadNo;
		archive<<iSpeed;

		archive.Write(chDeviceIp,32);	//ʶ���豸IP
		archive.Write(chListNo,256);		//������ɵ�һ�����к�
		archive.Write(chPlateNO,32);		//���ƺ�
		archive.Write(chPlateTime,256);	//����ص�������������deTime���󣬾��õ�ʱ�ı���ʱ��������	
		archive.Write(chVehPlateManual,20);		//�˹�ʶ������־
		archive.Write(chVehPlateSoft,20);				//���ʶ������־

	}
	else
	{
		archive>>dwCarID;				    // ����ID
		archive>>iPlateColor;					//������ɫ����
		archive>>iPlateTypeNo;				//�������ʹ���
		archive>>iVehTypeNo;					//�������ʹ���
		archive>>iLaneNo;						//������
		archive>>iDirection;					//·�ڷ���	
		archive>>iVehBodyColorNo;		//������ɫ
		archive>>iVehBodyDeepNo;		//������ǳ
		archive>>dw64TimeMS;	//���ƻ�ȡʱ��
		archive>>iDeviceID;					//�豸ID
		archive>>iAreaNo;
		archive>>iRoadNo;
		archive>>iSpeed;

		archive.Read(chDeviceIp,32);	//ʶ���豸IP
		archive.Read(chListNo,256);		//������ɵ�һ�����к�
		archive.Read(chPlateNO,32);		//���ƺ�
		archive.Read(chPlateTime,256);	//����ص�������������deTime���󣬾��õ�ʱ�ı���ʱ��������	
		archive.Read(chVehPlateManual,20);		//�˹�ʶ������־
		archive.Read(chVehPlateSoft,20);				//���ʶ������־
	}
}
