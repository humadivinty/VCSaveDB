#include "StdAfx.h"
#include "BaseDataBaseControl.h"

DWORD WINAPI DBStatusCheckThread(LPVOID lpParam);


BaseDataBaseControl::BaseDataBaseControl()
{	
	InitializeCriticalSection(&m_csConnect);
	HRESULT hr = CoInitialize(NULL);
	m_pConnectionPtr = NULL;
	m_bIsConnect =false;
	m_bExitConnect=false;
	m_bLogEnable= false;
	memset(m_chServerIP, 0, sizeof(m_chServerIP));
	memset(m_chDBName, 0, sizeof(m_chServerIP));
	memset(m_chDBUserID, 0, sizeof(m_chServerIP));
	memset(m_chDBPassword, 0, sizeof(m_chServerIP));	
	m_hDBConnectHandle = NULL;	
	//m_hDBConnectHandle = CreateThread(NULL, 0, DBStatusCheckThread, this, 0, NULL);

}

BaseDataBaseControl::~BaseDataBaseControl()
{
	CoUninitialize();
	m_bExitConnect = true;

	if(m_hDBConnectHandle)
	{
		int iWatTimes = 0;
		int MAX_WAIT_TIME = 10;
		while(WaitForSingleObject(m_hDBConnectHandle, 5000) == WAIT_TIMEOUT && iWatTimes < MAX_WAIT_TIME)
		{
			iWatTimes++;
		}
		if(iWatTimes >= MAX_WAIT_TIME)
		{
			TerminateThread(m_hDBConnectHandle, 0);			
		}
		CloseHandle(m_hDBConnectHandle);
		m_hDBConnectHandle = NULL;
		CloseDBConnect();
	}
	DeleteCriticalSection(&m_csConnect);
}

HRESULT BaseDataBaseControl::ConnectToDB(char* ConnectInfo)
{
	HRESULT hr = S_FALSE;
	try
	{
		if (NULL != m_pConnectionPtr)
		{
			hr = CloseDBConnect();
		}
		EnterCriticalSection(&m_csConnect);		
		hr = m_pConnectionPtr.CreateInstance(_uuidof(Connection));
		if (S_OK != hr)
		{
			LeaveCriticalSection(&m_csConnect);
			StrCat(ConnectInfo, "连接实例化失败");
			return hr;
		}
		char chConnectString[MAX_PATH] = {0};
		m_pConnectionPtr->CommandTimeout = 200;
		m_pConnectionPtr->CursorLocation = adUseClient;
		m_pConnectionPtr->IsolationLevel = adXactReadCommitted;
		//sprintf(chConnectString, "Provider = SQLOLEDB; Server = %s; DataBase=%s; uid=%s;pwd=%s", m_chServerIP, m_chDBName, m_chDBUserID, m_chDBPassword);
		sprintf(chConnectString, "Provider=SQLOLEDB; Persist Security Info=False; Initial Catalog=%s; Data Source=%s; User ID=%s; Password=%s", m_chDBName, m_chServerIP, m_chDBUserID, m_chDBPassword);
		hr = m_pConnectionPtr->Open(_bstr_t(chConnectString), "", "", adConnectUnspecified);

		if (S_OK == hr)
		{
			m_bIsConnect = true;
			StrCat(ConnectInfo, "数据库连接成功");
		}
		else
		{
			m_bIsConnect = false;
			StrCat(ConnectInfo, "数据库连接失败");
		}
		LeaveCriticalSection(&m_csConnect);
	}
	catch (_com_error &e)
	{
		LeaveCriticalSection(&m_csConnect);
		m_bIsConnect = false;
		char errorInfo[MAX_PATH] = {0};
		sprintf(errorInfo,"数据库连接失败, 错误信息：%s, 错误描述：%s ",e.ErrorMessage(), (char*)e.Description());
		memcpy(ConnectInfo, errorInfo, strlen(errorInfo));
	}
	return hr;
}

HRESULT BaseDataBaseControl::CloseDBConnect()
{
	HRESULT hr = S_FALSE;
	try
	{
		EnterCriticalSection(&m_csConnect);
		if (NULL != m_pConnectionPtr && m_pConnectionPtr->State)
		{
			hr = m_pConnectionPtr->Close();
			m_pConnectionPtr = NULL;
		}
		LeaveCriticalSection(&m_csConnect);
	}catch(_com_error e)
	{
		LeaveCriticalSection(&m_csConnect);
	}
	return hr;
}

bool BaseDataBaseControl::IsConnect()
{
	try
	{
		if (NULL != m_pConnectionPtr && m_pConnectionPtr->State)
		{

			return true;		//连接状态
		}

	}catch(_com_error e)
	{
		//AfxMessageBox(e.ErrorMessage());
	}
	return false;			//断开连接状态
}

SAFEARRAY* BaseDataBaseControl::SetPictureToVariant( VARIANT &pvList, unsigned char *PictureData,int nLen )
{
	SAFEARRAYBOUND saBound[1];
	saBound[0].cElements = nLen;
	saBound[0].lLbound = 0;
	SAFEARRAY* psa = SafeArrayCreate(VT_UINT, 1, saBound);
	unsigned char* imgBuff = NULL;
	SafeArrayAccessData(psa, (void **)&imgBuff);
	for (long lIndex = 0; lIndex< nLen;lIndex++)
	{
		imgBuff[lIndex] = PictureData[lIndex];
	}
	SafeArrayUnaccessData(psa);
	pvList.vt = VT_UI1|VT_ARRAY;
	pvList.parray = psa;

	return psa;
}

void BaseDataBaseControl::CheckDataBaseStatus()
{
	while(!m_bExitConnect)
	{
		if(m_pConnectionPtr !=NULL && adStateClosed ==m_pConnectionPtr->State)
		{
			char chConnectInfo[MAX_PATH] = {0};
			HRESULT hr = ConnectToDB(chConnectInfo);
		}
		Sleep(3*1000);
	}
}

DWORD WINAPI DBStatusCheckThread( LPVOID Lparam )
{
	if (NULL != Lparam)
	{
		BaseDataBaseControl* pBaseDBControl = (BaseDataBaseControl*)Lparam;
		pBaseDBControl->CheckDataBaseStatus();
	}
	return 0;
}
