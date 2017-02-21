#include "stdafx.h"
#include "CameraReault.h"


IMPLEMENT_SERIAL(CameraIMG, CObject, 1)
CameraIMG::CameraIMG()
{
	wImgWidth = 0;
	wImgHeight = 0;
	pbImgData = NULL;
	dwImgSize = 0;

	for (int i = 0; i< sizeof(chSavePath); i++)
	{
		chSavePath[i] = 0;
	}
}

CameraIMG::CameraIMG( const CameraIMG& CaIMG )
{
	wImgWidth = CaIMG.wImgWidth;
	wImgHeight = CaIMG.wImgHeight;
	dwImgSize = CaIMG.dwImgSize;
	wImgType = CaIMG.wImgType;
	for (int i = 0; i< sizeof(chSavePath); i++)
	{
		chSavePath[i] = CaIMG.chSavePath[i];
	}
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
		for (int i = 0; i< sizeof(chSavePath); i++)
		{
			chSavePath[i] = CaIMG.chSavePath[i];
		}
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
	iLaneNo = 0;
	iPlateColor = 0;
	iVehTypeNo = 0;
	iDirection = 0;				//·�ڷ���
	iAreaNo = 0;
	iVehBodyColorNo = 0;
	iPlateTypeNo = 0;
	iVehBodyDeepNo = 0;
	iDeviceID = 0;
	iRoadNo = 0;
	iSpeed = 0;
	pcAppendInfo = NULL;

	for (int i = 0; i < sizeof(chDeviceIp); i++)
	{
		chDeviceIp[i] = 0;
	}

	for (int i = 0; i < sizeof(chPlateNO); i++)
	{
		chPlateNO[i] = 0;
	}

	for (int i = 0; i < sizeof(chListNo); i++)
	{
		chListNo[i] = 0;
	}

	for (int i = 0; i < sizeof(chPlateTime); i++)
	{
		chPlateTime[i] = 0;
	}

	for (int i = 0; i < sizeof(chVehPlateManual); i++)
	{
		chVehPlateManual[i] = 0;
	}

	for (int i = 0; i < sizeof(chVehPlateSoft); i++)
	{
		chVehPlateSoft[i] = 0;
	}
}

CameraResult::CameraResult( const CameraResult& CaRESULT )
{
	iLaneNo = CaRESULT.iLaneNo;
	iPlateColor = CaRESULT.iPlateColor;
	iVehTypeNo = CaRESULT.iVehTypeNo;
	iDirection = CaRESULT.iDirection;				//·�ڷ���
	iAreaNo = CaRESULT.iAreaNo;
	iVehBodyColorNo = CaRESULT.iVehBodyColorNo;
	iPlateTypeNo = CaRESULT.iPlateTypeNo;
	iVehBodyDeepNo = CaRESULT.iVehBodyDeepNo;
	iDeviceID = CaRESULT.iDeviceID;
	iRoadNo = CaRESULT.iRoadNo;
	iSpeed = CaRESULT.iSpeed;
	pcAppendInfo = NULL;

	//sprintf(chDeviceIp, "%s", CaRESULT.chDeviceIp);
	//sprintf(chPlateNO, "%s", CaRESULT.chPlateNO);
	//sprintf(chListNo, "%s", CaRESULT.chListNo);
	//sprintf(chPlateTime, "%s", CaRESULT.chPlateTime);
	//sprintf(chVehPlateManual, "%s", CaRESULT.chVehPlateManual);
	//sprintf(chVehPlateSoft, "%s", CaRESULT.chVehPlateSoft);

	for (int i = 0; i < sizeof(chDeviceIp); i++)
	{
		chDeviceIp[i] = CaRESULT.chDeviceIp[i];
	}

	for (int i = 0; i < sizeof(chPlateNO); i++)
	{
		chPlateNO[i] = CaRESULT.chPlateNO[i];
	}

	for (int i = 0; i < sizeof(chListNo); i++)
	{
		chListNo[i] = CaRESULT.chListNo[i];
	}

	for (int i = 0; i < sizeof(chPlateTime); i++)
	{
		chPlateTime[i] = CaRESULT.chPlateTime[i];
	}

	for (int i = 0; i < sizeof(chVehPlateManual); i++)
	{
		chVehPlateManual[i] = CaRESULT.chVehPlateManual[i];
	}

	for (int i = 0; i < sizeof(chVehPlateSoft); i++)
	{
		chVehPlateSoft[i] = CaRESULT.chVehPlateSoft[i];
	}

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
	iLaneNo = CaRESULT.iLaneNo;
	iPlateColor = CaRESULT.iPlateColor;
	iVehTypeNo = CaRESULT.iVehTypeNo;
	iDirection = CaRESULT.iDirection;				//·�ڷ���
	iAreaNo = CaRESULT.iAreaNo;
	iVehBodyColorNo = CaRESULT.iVehBodyColorNo;
	iPlateTypeNo = CaRESULT.iPlateTypeNo;
	iVehBodyDeepNo = CaRESULT.iVehBodyDeepNo;
	iDeviceID = CaRESULT.iDeviceID;
	iRoadNo = CaRESULT.iRoadNo;
	iSpeed = CaRESULT.iSpeed;
	pcAppendInfo = NULL;
	for (int i = 0; i < sizeof(chDeviceIp); i++)
	{
		chDeviceIp[i] = CaRESULT.chDeviceIp[i];
	}

	for (int i = 0; i < sizeof(chPlateNO); i++)
	{
		chPlateNO[i] = CaRESULT.chPlateNO[i];
	}

	for (int i = 0; i < sizeof(chListNo); i++)
	{
		chListNo[i] = CaRESULT.chListNo[i];
	}

	for (int i = 0; i < sizeof(chPlateNO); i++)
	{
		chPlateNO[i] = CaRESULT.chPlateNO[i];
	}

	for (int i = 0; i < sizeof(chVehPlateManual); i++)
	{
		chVehPlateManual[i] = CaRESULT.chVehPlateManual[i];
	}

	for (int i = 0; i < sizeof(chVehPlateSoft); i++)
	{
		chVehPlateSoft[i] = CaRESULT.chVehPlateSoft[i];
	}

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
