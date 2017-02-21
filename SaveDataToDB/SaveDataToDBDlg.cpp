// SaveDataToDBDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "SaveDataToDB.h"
#include "SaveDataToDBDlg.h"
#include ".\savedatatodbdlg.h"

#include <new>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define MAX_CAMERA_COUNT 100
#define MAX_RESULT_COUNT 400

#import "c:\program files\common files\system\ado\msado15.dll" no_namespace rename ("EOF", "adoEOF") 

bool asscendingSort(CString& str1,CString &str2);

CCamera *g_CameraGroup[MAX_CAMERA_COUNT];


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CSaveDataToDBDlg 对话框



CSaveDataToDBDlg::CSaveDataToDBDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSaveDataToDBDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CSaveDataToDBDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_L_MESSAGE, m_MessageListBox);
	DDX_Control(pDX, IDC_LIST_Remote_MESSAGE, m_listBoxRemoteDB);
	DDX_Control(pDX, IDC_LIST_DEVICE, m_lsListCtrl);
	DDX_Control(pDX, IDC_CHECK_AUTOCONECT, m_buttonCheckBox);
}

BEGIN_MESSAGE_MAP(CSaveDataToDBDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_WM_CLOSE()
	ON_BN_CLICKED(ID_BUTTON_SHOWIMG, OnBnClickedButtonShowimg)
	ON_BN_CLICKED(IDC_BUTTON_DBTestConnect, OnBnClickedButtonDbtestconnect)
	ON_BN_CLICKED(IDC_BUTTON_StarUpDateDB, OnBnClickedButton1)
	ON_BN_CLICKED(IDCANCEL, OnBnClickedCancel)
	ON_BN_CLICKED(IDC_CHECK_AUTOCONECT, OnBnClickedCheckAutoconect)
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_LIST_DEVICE, OnLvnColumnclickListDevice)
//	ON_WM_SIZE()
ON_WM_SETCURSOR()
END_MESSAGE_MAP()


// CSaveDataToDBDlg 消息处理程序

BOOL CSaveDataToDBDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// 将\“关于...\”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码	
	GetDlgItem(IDC_STATIC_EXITSHOW)->ShowWindow(SW_HIDE);
	

	GdiplusStartup(&m_gdiplusToken,&StartupInput,NULL); 

	InitializeCriticalSection(&m_csDlgLog);
	InitializeCriticalSection(&m_csDlgRemoteLog);
	InitializeCriticalSection(&m_csReadLocal);
	InitializeCriticalSection(&m_csSaveLocal);
	InitializeCriticalSection(&m_csReadRemote);
	InitializeCriticalSection(&m_csSaveRemote);
	InitializeCriticalSection(&m_csUpdateListCtrl);

	m_hReadLocal = NULL;
	m_hSaveLocal = NULL;
	m_hReadRemote = NULL;
	m_hSaveRemote = NULL;
	for(int i = 0; i<MAX_THREADCOUNT; i++)
	{
		m_hSaveLocalDB[i] = NULL;
		m_hSaveRemoteDB[i] =NULL;
	}
	m_hSaveStatusToDB = NULL;
	m_hCircleDelete = NULL;


	DWORD dwStyle = GetWindowLong(m_lsListCtrl.m_hWnd, GWL_STYLE);
	dwStyle &= ~(LVS_TYPEMASK);
	dwStyle &= ~(LVS_EDITLABELS);
	dwStyle |= LVS_REPORT;
	SetWindowLong(m_lsListCtrl.m_hWnd, GWL_STYLE, dwStyle);

	//选中某行使整行高亮
	dwStyle = m_lsListCtrl.GetExtendedStyle();
	dwStyle |= LVS_EX_FULLROWSELECT;
	dwStyle |= LVS_EX_GRIDLINES;	
	m_lsListCtrl.SetExtendedStyle(dwStyle);

	m_lsListCtrl.InsertColumn(0,"序号",LVCFMT_CENTER, 50);
	m_lsListCtrl.InsertColumn(1,"DeviceID",LVCFMT_CENTER, 100);
	m_lsListCtrl.InsertColumn(2,"设备IP",LVCFMT_CENTER, 100);
	m_lsListCtrl.InsertColumn(3,"连接状态",LVCFMT_CENTER,100);
	m_lsListCtrl.InsertColumn(4,"上传本地库结果数量",LVCFMT_CENTER,150);
	m_lsListCtrl.InsertColumn(5,"上传中间库结果数量",LVCFMT_CENTER, 150);

//m_lsListCtrl.InsertItem(0, "kkk");
//m_lsListCtrl.SetItemText(0,1, "kkkk");
//
//int thesize =  m_lsListCtrl.GetItemCount();


	for (int i = 0; i< MAX_CAMERA_COUNT; i++)
	{
		g_CameraGroup[i] = NULL;
	}

	::CoInitialize(NULL);	
	ReadInitFileDlg();
	m_bExit  = false;
	m_bSaveLocalThreadExit = false;				//保存本地库数据队列的数据到硬盘的标志位
	m_bSaveRemoteThreadExit = false;			//保存中间库数据队列的数据到硬盘的标志位

	if (m_bAutoInsert)
	{
		m_buttonCheckBox.SetCheck(1);
		OnBnClickedButton1();
	}

	return TRUE;  // 除非设置了控件的焦点，否则返回 TRUE
}

void CSaveDataToDBDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CSaveDataToDBDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标显示。
HCURSOR CSaveDataToDBDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

unsigned __stdcall CSaveDataToDBDlg::ThreadSaveLocal( void* TheParam )
{
	if(NULL  == TheParam)
		return -1;
	CSaveDataToDBDlg* pThis = (CSaveDataToDBDlg*)TheParam;
	pThis->SaveLocal();
	return 0;
}

void CSaveDataToDBDlg::SaveLocal( void )
{
	CString strLocalFilePath;
	while(!m_bSaveLocalThreadExit)
	{
		EnterCriticalSection(&m_csSaveLocal);
		if (m_lsSaveLocal.size() > 0)
		{
			//WaitForSingleObject(m_hSaveLocal, 70l);			//2015-01-19
			
			CameraResult* tempResult = NULL;
			tempResult = m_lsSaveLocal.front();
			m_lsSaveLocal.pop_front();
			if (NULL == tempResult)
			{
				//ReleaseMutex(m_hSaveLocal);		//2015-01-19
				LeaveCriticalSection(&m_csSaveLocal);
				continue;
			}
			if (strlen(tempResult->chListNo)< 0)
			{
				//ReleaseMutex(m_hSaveLocal);		//2015-01-19
				LeaveCriticalSection(&m_csSaveLocal);
				continue;
			}
			strLocalFilePath.Format("%s\\%s.dat", m_strLocalFilePath, tempResult->chListNo);
			CFile theFile;
			theFile.Open(strLocalFilePath, CFile::modeCreate|CFile::modeWrite);
			CArchive theArchive(&theFile, CArchive::store);
			(*tempResult).Serialize(theArchive);
			theArchive.Close();
			theFile.Close();
			if (NULL != tempResult)
			{
				delete tempResult;
				tempResult = NULL;
			}
			//ReleaseMutex(m_hSaveLocal);		//2015-01-19
			LeaveCriticalSection(&m_csSaveLocal);
		}
		else
		{
			LeaveCriticalSection(&m_csSaveLocal);
			Sleep(2000);
		}
	}
	return;
}

unsigned __stdcall CSaveDataToDBDlg::ThreadReadLocal( void* TheParam )
{
	if(NULL  == TheParam)
		return -1;
	CSaveDataToDBDlg* pThis = (CSaveDataToDBDlg*)TheParam;
	pThis->ReadLocal();
	return 0;
}

void CSaveDataToDBDlg::ReadLocal( void )
{
	CString tmpFilePath;
	tmpFilePath.Format("%s\\*.dat",m_strLocalFilePath);
	CString strtmpFileName;
	while(!m_bExit)
	{
		Sleep(2000);
		CFileFind finder;
		BOOL bWorking = finder.FindFile(tmpFilePath);
		int Count = 0;
		while(bWorking)
		{
			Sleep(100);
			bWorking = finder.FindNextFile();
			if (finder.IsDots())
			{
				continue;
			}
			if (finder.IsDirectory())
			{
				continue;
			}
			//信号量
			BOOL bSuccess = FALSE;
			//WaitForSingleObject(m_hRLocalMutex, 70l);				//2015-01-19
			EnterCriticalSection(&m_csReadLocal);

			strtmpFileName = finder.GetFilePath();
			CameraResult* tempResult = new CameraResult();
			CFile theLoadFile;
			bSuccess = theLoadFile.Open(strtmpFileName, CFile::modeRead);
			if (!bSuccess)
			{
				if (NULL != tempResult)
				{
					delete tempResult;
					tempResult = NULL;
				}
				//ReleaseMutex(m_hRLocalMutex);					//2015-01-19
				
				LeaveCriticalSection(&m_csReadLocal);
				continue;
			}
			ULONGLONG ulLenth = theLoadFile.GetLength();
			if (ulLenth<= 100)
			{
				theLoadFile.Close();
				DeleteFile(strtmpFileName);

				char chLogBuf[256] = {0};
				sprintf(chLogBuf, "%s数据长度异常，丢弃该数据", strtmpFileName.GetBuffer());
				strtmpFileName.ReleaseBuffer();
				WriteDlgLog(chLogBuf);

				LeaveCriticalSection(&m_csReadLocal);
				continue;
			}
			CArchive theLoadArchive(&theLoadFile, CArchive::load);
			(*tempResult).Serialize(theLoadArchive);
			theLoadArchive.Close();
			theLoadFile.Close();
			if (m_lsReadLocal.size()>=0 && m_lsReadLocal.size()< MAX_RESULT_COUNT)
			{
				m_lsReadLocal.push_back(tempResult);
			}
			else
			{
				bSuccess = FALSE;
				if (NULL != tempResult)
				{
					delete tempResult;
					tempResult = NULL;
				}
			}

			if (bSuccess)
			{
				DeleteFile(strtmpFileName);
			}

			//ReleaseMutex(m_hRLocalMutex);			//2015-01-19
			LeaveCriticalSection(&m_csReadLocal);
		}

		finder.Close();
	}
	return;
}

unsigned __stdcall CSaveDataToDBDlg::ThreadSaveRemote( void* TheParam )
{
	if(NULL  == TheParam)
		return -1;
	CSaveDataToDBDlg* pThis = (CSaveDataToDBDlg*)TheParam;
	pThis->SaveRemote();
	return 0;
}

void CSaveDataToDBDlg::SaveRemote( void )
{
	
	while(!m_bSaveRemoteThreadExit)
	{
		EnterCriticalSection(&m_csSaveRemote);
		if (m_lsSaveRemote.size() > 0)
		{
			//WaitForSingleObject(m_hSaveRemote, 70l);			//2015-01-19
			
			CameraResult* tempResult =NULL;
			tempResult = m_lsSaveRemote.front();
			m_lsSaveRemote.pop_front();
			CString strRemoteFilePath;
			if (NULL == tempResult)
			{				
				//ReleaseMutex(m_hSaveRemote);		//2015-01-19
				LeaveCriticalSection(&m_csSaveRemote);
				continue;
			}
			if (strlen(tempResult->chListNo) < 0)
			{
				//ReleaseMutex(m_hSaveRemote);		//2015-01-19
				LeaveCriticalSection(&m_csSaveRemote);
				continue;
			}
			strRemoteFilePath.Format("%s\\%s.dat", m_strRemoteFilePath, tempResult->chListNo);
			CFile theFile;
			theFile.Open(strRemoteFilePath, CFile::modeCreate|CFile::modeWrite);
			CArchive theArchive(&theFile, CArchive::store);
			(*tempResult).Serialize(theArchive);
			theArchive.Close();
			theFile.Close();

			if (NULL != tempResult)
			{
				delete tempResult;
				tempResult = NULL;
			}
			//ReleaseMutex(m_hSaveRemote);				//2015-01-19
			LeaveCriticalSection(&m_csSaveRemote);
		}
		else
		{
			LeaveCriticalSection(&m_csSaveRemote);
			Sleep(2000);
		}
	}
	return;
}

unsigned __stdcall CSaveDataToDBDlg::ThreadReadRemote( void* TheParam )
{
	if(NULL  == TheParam)
		return -1;
	CSaveDataToDBDlg* pThis = (CSaveDataToDBDlg*)TheParam;
	pThis->ReadRemote();
	return 0;
}

void CSaveDataToDBDlg::ReadRemote( void )
{
	CString tmpFilePath;
	tmpFilePath.Format("%s\\*.dat",m_strRemoteFilePath);
	CString strtmpFileName;
	while(!m_bExit)
	{
		Sleep(2000);
		CFileFind finder;
		BOOL bWorking = finder.FindFile(tmpFilePath);
		int Count = 0;
		while(bWorking)
		{
			Sleep(100);
			bWorking = finder.FindNextFile();
			if (finder.IsDots())
			{
				continue;
			}
			if (finder.IsDirectory())
			{
				continue;
			}
			//信号量
			BOOL bSuccess = FALSE;
			//WaitForSingleObject(m_hRRemoteMutex, 70l);			//2015-01-19
			EnterCriticalSection(&m_csReadRemote);

			strtmpFileName = finder.GetFilePath();
			CameraResult* tempResult = new CameraResult();
			CFile theRemoteFile;
			bSuccess = theRemoteFile.Open(strtmpFileName, CFile::modeRead);
			if (!bSuccess)
			{
				if (NULL != tempResult)
				{
					delete tempResult;
					tempResult = NULL;
				}
				//ReleaseMutex(m_hRRemoteMutex);			//2015-01-19
				LeaveCriticalSection(&m_csReadRemote);
				continue;
			}
			//if (theRemoteFile.GetLength() <= 10)
			ULONGLONG ulLenth = theRemoteFile.GetLength();
			if ( ulLenth<= 100)
			{
				theRemoteFile.Close();
				DeleteFile(strtmpFileName);

				char chLogBuf[256] = {0};
				sprintf(chLogBuf, "%s数据长度异常，丢弃该数据", strtmpFileName.GetBuffer());
				strtmpFileName.ReleaseBuffer();
				WriteDlgRemotLog(chLogBuf);

				LeaveCriticalSection(&m_csReadRemote);
				continue;
			}
			CArchive theLoadArchive(&theRemoteFile, CArchive::load);
			(*tempResult).Serialize(theLoadArchive);
			theLoadArchive.Close();
			theRemoteFile.Close();
			if (m_lsReadRemote.size()>= 0 && m_lsReadRemote.size() < MAX_RESULT_COUNT)
			{
				m_lsReadRemote.push_back(tempResult);
			}
			else
			{
				bSuccess = FALSE;
				if (NULL != tempResult)
				{
					delete tempResult;
					tempResult = NULL;
				}
			}

			//ReleaseMutex(m_hRRemoteMutex);			//2015-01-19

			if (bSuccess)
			{
				DeleteFile(strtmpFileName);
			}
			LeaveCriticalSection(&m_csReadRemote);
		}
		finder.Close();
	}
	return;
}

unsigned __stdcall CSaveDataToDBDlg::ThreadSaveLocalDB( void* TheParam )
{
	if(NULL  == TheParam)
		return -1;
	CSaveDataToDBDlg* pThis = (CSaveDataToDBDlg*)TheParam;
	pThis->SaveLocalDB();
	return 0;
}

void CSaveDataToDBDlg::SaveLocalDB( void )
{
	HRESULT hr1= S_FALSE, hr2 = S_FALSE, hr3 = S_FALSE;
	char chReConnectInfo[256] = {0};
	LocalDataBaseControler LocalDB;
	::CoInitialize(NULL);
	hr1 = LocalDB.ConnectToDB(chReConnectInfo);
	ShowMessage(chReConnectInfo);

	char chNormalDataBuf[256] = {0};

	while(!m_bExit)
	{
		EnterCriticalSection(&m_csReadLocal);
		if (m_lsReadLocal.size() > 0)
		{			
			CameraResult* tempResult = NULL;
			tempResult = m_lsReadLocal.front();
			m_lsReadLocal.pop_front();

			if (NULL == tempResult)
			{
				WriteDlgLog("获取的结果为NULL");
				LeaveCriticalSection(&m_csReadLocal);
				continue;
			}

			hr1 = LocalDB.SaveNormalDataToDB(tempResult);
			if (S_OK == hr1)
			{

				sprintf(chNormalDataBuf, "本地库保存成功, listNo = %s", tempResult->chListNo);
				ShowMessage(chNormalDataBuf);
				WriteDlgLog(chNormalDataBuf);

				CString strDeviceID;
				strDeviceID.Format("%d", tempResult->iDeviceID);
				CString strDeviceIP(tempResult->chDeviceIp);
				CString strDeviceStatus;
				CString strLocalResultCount("1");
				CString strRemoteResultCount;
				UpdateListCtrlView(strDeviceID, strDeviceIP, strDeviceStatus, strLocalResultCount, strRemoteResultCount);


				hr1 = LocalDB.SaveBigImageToDB(tempResult->CIMG_FullImage.pbImgData, tempResult->chListNo, tempResult->CIMG_FullImage.dwImgSize);
				if(S_OK != hr1)
				{
					sprintf(chNormalDataBuf, "图片本地库保存失败, listNo = %s", tempResult->chListNo);
					ShowMessage(chNormalDataBuf);
					WriteDlgLog(chNormalDataBuf);
				}
				else
				{
					sprintf(chNormalDataBuf, "图片本地库保存成功,listNo = %s", tempResult->chListNo);
					ShowMessage(chNormalDataBuf);
					WriteDlgLog(chNormalDataBuf);
				}
				if (NULL != tempResult->CIMG_PlateImage.pbImgData)
				{
					if (S_OK != LocalDB.SaveSmallImageToDB(tempResult->CIMG_PlateImage.pbImgData, tempResult->chListNo, tempResult->CIMG_PlateImage.dwImgSize))
					{
						sprintf(chNormalDataBuf, "车牌小图本地库保存失败,listNo = %s", tempResult->chListNo);
						ShowMessage(chNormalDataBuf);
						WriteDlgLog(chNormalDataBuf);
					}
					else
					{
						sprintf(chNormalDataBuf, "车牌小图本地库保存成功,listNo = %s", tempResult->chListNo);
						ShowMessage(chNormalDataBuf);
						WriteDlgLog(chNormalDataBuf);
					}
				}
				delete tempResult;
				tempResult = NULL;
				//ReleaseMutex(m_hReadLocal);		//2015-01-19
			}
			else
			{
				sprintf(chNormalDataBuf, "流水信息保存失败，缓存结果到本地,listNo = %s", tempResult->chListNo);
				ShowMessage(chNormalDataBuf);
				WriteDlgLog(chNormalDataBuf);

				//WaitForSingleObject(m_hReadLocal, 70l);		//2015-01-19
				EnterCriticalSection(&m_csSaveLocal);
				m_lsSaveLocal.push_back(tempResult);
				//ReleaseMutex(m_hReadLocal);			//2015-01-19
				LeaveCriticalSection(&m_csSaveLocal);
			}
			LeaveCriticalSection(&m_csReadLocal);
		}
		else
		{
			LeaveCriticalSection(&m_csReadLocal);
			Sleep(2000);
		}
	}
	LocalDB.CloseDBConnect();
}

unsigned __stdcall CSaveDataToDBDlg::ThreadSaveRemoteDB( void* TheParam )
{
	//存数据入中间库
	if(NULL  == TheParam)
		return -1;
	CSaveDataToDBDlg* pThis = (CSaveDataToDBDlg*)TheParam;
	pThis->SaveRemoteDB();
	return 0;
}

void CSaveDataToDBDlg::SaveRemoteDB( void )
{
	char chReConnectInfo[256] = {0};
	HRESULT hr1= S_FALSE, hr2 = S_FALSE, hr3 = S_FALSE;
	::CoInitialize(NULL);
	RemoteDataBaseControler RemoteDB;
	hr1 = RemoteDB.ConnectToDB(chReConnectInfo);

	ShowRemotDBMessage(chReConnectInfo);
	WriteDlgRemotLog(chReConnectInfo);

	char chRemotDBInfo[256] = {0};

	while(!m_bExit)
	{
		EnterCriticalSection(&m_csReadRemote);
		if (m_lsReadRemote.size() > 0)
		{			
			
			CameraResult* tempResult = m_lsReadRemote.front();
			m_lsReadRemote.pop_front();

			if (NULL == tempResult)
			{
				WriteDlgRemotLog("获取的结果为NULL");
				LeaveCriticalSection(&m_csReadRemote);
				continue;
			}

			hr1 = RemoteDB.SaveNormalDataToDB(tempResult);
			if (S_OK == hr1)
			{
				CString strDeviceID;
				strDeviceID.Format("%d", tempResult->iDeviceID);
				CString strDeviceIP(tempResult->chDeviceIp);
				CString strDeviceStatus;
				CString strLocalResultCount;
				CString strRemoteResultCount("1");
				UpdateListCtrlView(strDeviceID, strDeviceIP, strDeviceStatus, strLocalResultCount, strRemoteResultCount);
				
				sprintf(chRemotDBInfo, "流水信息保存成功，流水号LisNo = %s ", tempResult->chListNo);
				ShowRemotDBMessage(chRemotDBInfo);
				WriteDlgRemotLog(chRemotDBInfo);
				delete tempResult;
				tempResult = NULL;
			}
			else
			{
				sprintf(chRemotDBInfo, "流水信息保存失败，流水号LisNo = %s ", tempResult->chListNo);
				ShowRemotDBMessage(chRemotDBInfo);
				WriteDlgRemotLog(chRemotDBInfo);

				//WaitForSingleObject(m_hReadRemote, 70l);			//2015-01-19
				EnterCriticalSection(&m_csSaveRemote);
				m_lsSaveRemote.push_back(tempResult);
				LeaveCriticalSection(&m_csSaveRemote);
				//ReleaseMutex(m_hReadRemote);				//2015-01-19
			}
			LeaveCriticalSection(&m_csReadRemote);
		}
		else
		{
			LeaveCriticalSection(&m_csReadRemote);
			Sleep(2000);
		}
	}
	RemoteDB.CloseDBConnect();
}

unsigned __stdcall CSaveDataToDBDlg::ThreadSafeStatuToDB( void* TheParam )
{
	if(NULL  == TheParam)
		return -1;
	CSaveDataToDBDlg* pThis = (CSaveDataToDBDlg*)TheParam;
	pThis->SafeStatuToDB();
	return 0;
}

void CSaveDataToDBDlg::SafeStatuToDB( void )
{
	char chReConnectInfo[256] = {0};
	char chLoConnectInfo[256] = {0};
	HRESULT hr1= S_FALSE, hr2 = S_FALSE;
	LocalDataBaseControler LocalDB;
	RemoteDataBaseControler RemoteDB;

	::CoInitialize(NULL);
	if (m_bLocalDBEnable)
	{
		hr1 = LocalDB.ConnectToDB(chLoConnectInfo);
	}

	if (m_bRemoteDBEnable)
	{
		hr2 = RemoteDB.ConnectToDB(chReConnectInfo);
	}

	//WriteDlgLog(strcat("SaveRemoteDB:: 连接失败：",chReConnectInfo));
	DWORD iLastStatusUpdateTick =0;
	while(!m_bExit)
	{
		bool bUpdate = GetTickCount() - iLastStatusUpdateTick >m_iStatusUpdateInterval*1000;
		if (0 == iLastStatusUpdateTick || bUpdate)
		{			
			iLastStatusUpdateTick = GetTickCount();
			for (int i = 0; i<MAX_CAMERA_COUNT; i++)
			{
				if (NULL != g_CameraGroup[i])
				{
					char szGUID[256]={0};
					GUID guid;
					if (S_OK == ::CoCreateGuid(&guid))
					{
						_snprintf(szGUID, 256
							,"%08X-%04X-%04x-%02X%02X-%02X%02X%02X%02X%02X%02X"
							,guid.Data1
							,guid.Data2
							,guid.Data3
							,guid.Data4[0], guid.Data4[1]
							,guid.Data4[2], guid.Data4[3], guid.Data4[4], guid.Data4[5]
							,guid.Data4[6], guid.Data4[7]
							);
					}

					char strCreateTime[256] = {0};
					CTime t=CTime::GetCurrentTime();
					sprintf(strCreateTime,"%d-%02d-%02d %02d:%02d:%02d",t.GetYear(),t.GetMonth(),t.GetDay(),t.GetHour(), t.GetMinute(), t.GetSecond());
					
					char chCameraStatu[128] = {};
					int iStatu = g_CameraGroup[i]->GetCameraStatus();
					//时间同步
					CTime currentTime=CTime::GetCurrentTime();
					g_CameraGroup[i]->SyncTime(currentTime);

					if ( 0 != iStatu )
					{
						sprintf(chCameraStatu, "连接已断开");
					}
					else
					{
						sprintf(chCameraStatu, "连接正常");
					}
					//UpdateListCtrlView()

					//2015-01-27 用于显示设备列表的代码
					CString strDeviceID;
					strDeviceID.Format("%d", g_CameraGroup[i]->m_iDiviceID);
					CString strDeviceIP(g_CameraGroup[i]->m_strIp.c_str());
					CString strDeviceStatus(chCameraStatu);
					CString strResultCount;
					UpdateListCtrlView(strDeviceID, strDeviceIP, strDeviceStatus, strResultCount, strResultCount);


					if (m_bLocalDBEnable)
					{
						LocalDB.SaveDeviceStatusToDB(szGUID, g_CameraGroup[i]->m_iDiviceID, chCameraStatu, strCreateTime, iStatu);
					}
					if (m_bRemoteDBEnable)
					{
						RemoteDB.SaveDeviceStatusToDB(szGUID, g_CameraGroup[i]->m_iDiviceID, chCameraStatu, strCreateTime, iStatu);
					}
				}
			}
		}		
		Sleep(2000);
	}
	if (m_bLocalDBEnable)
	{
		LocalDB.CloseDBConnect();
	}
	if (m_bRemoteDBEnable)
	{
		RemoteDB.CloseDBConnect();
	}	
}

unsigned __stdcall CSaveDataToDBDlg::ThreadCirclelaryDelete( void* TheParam )
{
	if (NULL == TheParam)
	{
		return -1;
	}	
	CSaveDataToDBDlg *  pDlgThis = (CSaveDataToDBDlg* )TheParam;
	pDlgThis->WriteDlgLog("环覆盖线程启动");
	while(!pDlgThis->m_bExit)
	{
		//获取备份文件所在盘符的根目录
		string strBackupResult(pDlgThis->m_strBackUpResultPath.GetBuffer());
		pDlgThis->m_strBackUpResultPath.ReleaseBuffer();
		string::size_type sIndex = strBackupResult.find("\\");
		strBackupResult = strBackupResult.substr(0, sIndex+1);
		char chBackupResultRootPath[30] = {0};
		sprintf(chBackupResultRootPath, "%s", strBackupResult.c_str());

		char szLog1[MAX_PATH] = {0};
		sprintf(szLog1, "查询根目录为%s ", chBackupResultRootPath);
		pDlgThis->WriteDlgLog(szLog1);

		//查询磁盘剩余空间，当可用空间小于1G时进行循环覆盖
		ULONGLONG uDiskSize  = 0;
		bool bCheckSizeSuccess = pDlgThis->GetDiskFreeSpaceT(chBackupResultRootPath, uDiskSize);
		if (bCheckSizeSuccess && uDiskSize >= 0 && uDiskSize/1024/1024 <= 1024)
		{
			pDlgThis->WriteDlgLog("磁盘剩余空间小于1G ，开始循环覆盖,只保留一天的数据");
			//当磁盘容量小于1G时，只保留一天的文件数据
			pDlgThis->CirclelaryDelete(pDlgThis->m_strBackUpLocalResultPath.GetBuffer(), 1);
			pDlgThis->m_strBackUpLocalResultPath.ReleaseBuffer();

			pDlgThis->CirclelaryDelete(pDlgThis->m_strBackUpRemolResultPath.GetBuffer(), 1);
			pDlgThis->m_strBackUpRemolResultPath.ReleaseBuffer();
		}
		else if(bCheckSizeSuccess && uDiskSize >= 0 && uDiskSize/1024/1024 >= 1024)
		{
			//当磁盘容量大于1G时，按照设定的备份天数来删除文件
			char chDisckSize[50] = {0};
			sprintf(chDisckSize, "磁盘%s容量大小为: %d G ", chBackupResultRootPath, uDiskSize/1024/1024/1024);
			pDlgThis->WriteDlgLog(chDisckSize);
			//删除本地库的备份文件
			pDlgThis->CirclelaryDelete(pDlgThis->m_strBackUpLocalResultPath.GetBuffer(), pDlgThis->m_iBackUpResultDays);
			pDlgThis->m_strBackUpLocalResultPath.ReleaseBuffer();

			//删除中间库的备份文件
			pDlgThis->CirclelaryDelete(pDlgThis->m_strBackUpRemolResultPath.GetBuffer(), pDlgThis->m_iBackUpResultDays);
			pDlgThis->m_strBackUpRemolResultPath.ReleaseBuffer();
		}
		else if (!bCheckSizeSuccess)
		{
			pDlgThis->WriteDlgLog("磁盘空间查询失败");
		}

		//删除日志文件
		pDlgThis->CirclelaryDelete(pDlgThis->m_strLogPath.GetBuffer(), pDlgThis->m_iBackUpLogDays);
		pDlgThis->m_strLogPath.ReleaseBuffer();

		Sleep(10*60*1000);		//间隔10分钟查询一次
	}
	pDlgThis->WriteDlgLog("环覆盖线程退出");
	return 0;
}

//************************************
// Method:    GetDiskFreeSpaceT 获取指定目录的磁盘的剩余空间大小
// FullName:  CSaveDataToDBDlg::GetDiskFreeSpaceT
// Access:    public 
// Returns:   bool
// Qualifier: 
// Parameter: char * szDiskChar  指定磁盘路径
// Parameter: ULONGLONG & DiskFreeSpace  剩余空间大小
//************************************
bool CSaveDataToDBDlg::GetDiskFreeSpaceT( char* szDiskChar, ULONGLONG& DiskFreeSpace )
{
	DiskFreeSpace = 0;
	if (NULL == szDiskChar)
	{
		return false;
	}
	_ULARGE_INTEGER lpFreeByteAvailableToCaller, lpTotalNumberOfBytes, lpTotalNumberOfFreeBytes;
	BOOL bRet = GetDiskFreeSpaceEx(szDiskChar, &lpFreeByteAvailableToCaller, &lpTotalNumberOfBytes, &lpTotalNumberOfFreeBytes);
	DiskFreeSpace = lpTotalNumberOfFreeBytes.QuadPart;
	return bRet?true : false;
}

int CSaveDataToDBDlg::CirclelaryDelete( char* folderPath, int iBackUpDays )
{
	WriteDlgLog("进入环覆盖线程主函数,开始查找制定目录下的文件夹");
	char myPath[MAX_PATH] = {0};
	sprintf(myPath, "%s\\*", folderPath);

	CTime tmCurrentTime = CTime::GetCurrentTime();
	CTime tmLastMonthTime = tmCurrentTime - CTimeSpan(iBackUpDays, 0, 0, 0);
	int Last_Year = tmLastMonthTime.GetYear() ;
	int Last_Month = tmLastMonthTime.GetMonth();
	int Last_Day = tmLastMonthTime.GetDay();
	//cout<<Last_Year<<"-"<<Last_Month<<"-"<<Last_Day<<endl;

	CFileFind myFileFind;
	BOOL bFinded = myFileFind.FindFile(myPath);
	char DirectoryName[MAX_PATH] = {0};
	while(bFinded)
	{
		bFinded = myFileFind.FindNextFileA();
		if (!myFileFind.IsDots())
		{
			sprintf(DirectoryName, "%s",myFileFind.GetFileName().GetBuffer());
			if (myFileFind.IsDirectory())
			{
				int iYear,iMonth,iDay;
				iYear = iMonth  = iDay = 0;
				sscanf(DirectoryName,"%d-%d-%d",&iYear, &iMonth, &iDay);
				if (iYear < Last_Year )
				{
					sprintf(DirectoryName,"%s\\%s", folderPath, myFileFind.GetFileName().GetBuffer());
					printf("delete the DirectoryB :%s\n",DirectoryName);
					DeleteDirectory(DirectoryName);

					char chLog[MAX_PATH] = {0};
					sprintf(chLog, "年份小于当前年份，删除文件夹%s", DirectoryName);
					WriteDlgLog(chLog);
				} 
				else if (iYear == Last_Year)
				{
					if (iMonth < Last_Month)
					{
						sprintf(DirectoryName,"%s\\%s", folderPath, myFileFind.GetFileName().GetBuffer());
						printf("delete the DirectoryB :%s\n",DirectoryName);
						DeleteDirectory(DirectoryName);

						char chLog[MAX_PATH] = {0};
						sprintf(chLog, "月份小于上一月，删除文件夹%s", DirectoryName);
						WriteDlgLog(chLog);
					}
					else if (iMonth == Last_Month)
					{
						if (iDay < Last_Day)
						{
							sprintf(DirectoryName,"%s\\%s", folderPath, myFileFind.GetFileName().GetBuffer());
							printf("delete the DirectoryB :%s\n",DirectoryName);
							DeleteDirectory(DirectoryName);

							char chLog[MAX_PATH] = {0};
							sprintf(chLog, "日号小于指定天数，删除文件夹%s", DirectoryName);
							WriteDlgLog(chLog);
						}
					}
				}
			}
		}
	}
	myFileFind.Close();
	WriteDlgLog("查询结束，退出环覆盖线程主函数..");
	return 0;
}

bool CSaveDataToDBDlg::DeleteDirectory( char* strDirName )
{
	CFileFind tempFind;
   
    char strTempFileFind[MAX_PATH];

    sprintf(strTempFileFind,"%s//*.*", strDirName);

    BOOL IsFinded = tempFind.FindFile(strTempFileFind);

    while (IsFinded)
    {
        IsFinded = tempFind.FindNextFile();

        if (!tempFind.IsDots())
        {
            char strFoundFileName[MAX_PATH];

            strcpy(strFoundFileName, tempFind.GetFileName().GetBuffer(MAX_PATH));

            if (tempFind.IsDirectory())
            {
                char strTempDir[MAX_PATH];

                sprintf(strTempDir,"%s//%s", strDirName, strFoundFileName);

                DeleteDirectory(strTempDir);
            }
            else
            {
                char strTempFileName[MAX_PATH];

                sprintf(strTempFileName,"%s//%s", strDirName, strFoundFileName);

                DeleteFile(strTempFileName);
            }
        }
    }

    tempFind.Close();

    if(!RemoveDirectory(strDirName))
    {
        return FALSE;
    }

    return TRUE;
}

void CSaveDataToDBDlg::ReadInitFileDlg( void )
{
	char chFileName[MAX_PATH];
	GetModuleFileNameA(NULL, chFileName, MAX_PATH-1);
	PathRemoveFileSpecA(chFileName);
	char chIniFileName[MAX_PATH] = { 0 };
	strcpy(chIniFileName, chFileName);
	strcat(chIniFileName, "\\HvConfig.ini");

	//日志文件存储路径
	m_strLogPath.Format("%s\\LOG\\", chFileName);

	//获取缓存文件存储路径
	char chNormalRecordPath[MAX_PATH] = {0};
	GetPrivateProfileStringA("ResultInfo","CachePath","D:\\CacheRecord",chNormalRecordPath,1024,chIniFileName);
	m_strLocalFilePath.Format("%s\\Local\\", chNormalRecordPath);
	m_strRemoteFilePath.Format("%s\\MID\\", chNormalRecordPath);

	//获取备份文件存储路径
	GetPrivateProfileStringA("ResultInfo","SavePath","D:\\SaveRecord",chNormalRecordPath,1024,chIniFileName);
	m_strBackUpResultPath.Format("%s\\", chNormalRecordPath);
	m_strBackUpLocalResultPath.Format("%s\\Local\\", chNormalRecordPath);
	m_strBackUpRemolResultPath.Format("%s\\MID\\", chNormalRecordPath);


	//是否上传数据到本地库
	int iLocalDB = GetPrivateProfileIntA("LocalDataBase", "SaveEnable", 0, chIniFileName);
	m_bLocalDBEnable = (iLocalDB == 0) ? false : true;
	//是否上传数据到中间库
	int iRemoteDB = GetPrivateProfileIntA("RemoteDataBase", "SaveEnable", 0, chIniFileName);
	m_bRemoteDBEnable = (iRemoteDB == 0) ? false : true;

	//日志开关
	int iTmp = GetPrivateProfileIntA("LOG", "Enable", 0, chIniFileName);
	m_bLogEnable = (iTmp == 0) ? false : true;

	//设备状态插入数据库的间隔时间
	int iStatusUpdateInterval =GetPrivateProfileIntA("STATUS", "IntervalTime", 30, chIniFileName);
	m_iStatusUpdateInterval = (iStatusUpdateInterval > 0) ? iStatusUpdateInterval: 30;

	//日志文件保留天数
	int iBackupLogDays =GetPrivateProfileIntA("BACKUP", "BackupLogDays", 10, chIniFileName);
	m_iBackUpLogDays = (iBackupLogDays > 0) ? iBackupLogDays: 10;
	//数据备份文件保留天数
	int iBackupResultDays =GetPrivateProfileIntA("BACKUP", "BackupResultDays", 10, chIniFileName);
	m_iBackUpResultDays = (iBackupResultDays > 0) ? iBackupResultDays: 1;

	//获取数据库服务器IP
	GetPrivateProfileStringA("LocalDataBase","ServerIP","127.0.0.1",m_chServerIP,256,chIniFileName);	
	WritePrivateProfileStringA("LocalDataBase","ServerIP",m_chServerIP,chIniFileName);
	//获取数据库名称
	GetPrivateProfileStringA("LocalDataBase","DataBaseName","KKDB",m_chDBName,256,chIniFileName);
	WritePrivateProfileStringA("LocalDataBase","DataBaseName",m_chDBName,chIniFileName);
	//获取数据库用户名
	GetPrivateProfileStringA("LocalDataBase","DBUserID","kk",m_chDBUserID,256,chIniFileName);
	WritePrivateProfileStringA("LocalDataBase","DBUserID",m_chDBUserID,chIniFileName);
	//获取数据库密码
	GetPrivateProfileStringA("LocalDataBase","DBPassword","123456",m_chDBPassword,256,chIniFileName);
	WritePrivateProfileStringA("LocalDataBase","DBPassword",m_chDBPassword,chIniFileName);

	//下次运行时自启动数据录入
	int iAutoRun =GetPrivateProfileIntA("AutoRun", "RunEnable", 0, chIniFileName);	
	m_bAutoInsert = (iAutoRun > 0) ?  true : false;
}

void CSaveDataToDBDlg::WriteDlgLog( char* logBuf )
{
	if (!m_bLogEnable)
		return ;

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
	strCurrentDir.AppendFormat("\\LOG\\%04d-%02d-%02d\\DlgLog\\", pTM->tm_year + 1900, pTM->tm_mon + 1, pTM->tm_mday);
	MakeSureDirectoryPathExists(strCurrentDir);
	fileName.Format("%s%04d-%02d-%02d-%02d_DlgLog.log", strCurrentDir, pTM->tm_year + 1900, pTM->tm_mon + 1, pTM->tm_mday, pTM->tm_hour);

	EnterCriticalSection(&m_csDlgLog);
	FILE *file = NULL;
	file = fopen(fileName, "a+");
	if (file)
	{
		fprintf(file,"%04d-%02d-%02d %02d:%02d:%02d:%03d : %s\n",  pTM->tm_year + 1900, pTM->tm_mon + 1, pTM->tm_mday,
			pTM->tm_hour, pTM->tm_min, pTM->tm_sec, dwMS, logBuf);
		fclose(file);
		file = NULL;
	}
	LeaveCriticalSection(&m_csDlgLog);
}

void CSaveDataToDBDlg::ShowMessage( char* messageBuf )
{
	CString tempStr(messageBuf);
	if (m_MessageListBox.GetCount() > 100)
	{
		m_MessageListBox.ResetContent();
	}
	m_MessageListBox.AddString(tempStr);
	m_MessageListBox.SetCurSel(m_MessageListBox.GetCount()-1);
}

void CSaveDataToDBDlg::ShowRemotDBMessage( char* messageBuf )
{
	CString tempStr(messageBuf);
	if (m_listBoxRemoteDB.GetCount() > 100)
	{
		m_listBoxRemoteDB.ResetContent();
	}
	m_listBoxRemoteDB.AddString(tempStr);
	m_listBoxRemoteDB.SetCurSel(m_listBoxRemoteDB.GetCount()-1);
}

void CSaveDataToDBDlg::OnClose()
{
	if(AfxMessageBox(_T("真的要退出吗？"),MB_YESNO) != IDYES)
	{
		return;
	}
	// TODO: Add your message handler code here and/or call default
	CMenu* psysMenu = GetSystemMenu(FALSE);
	if(NULL != psysMenu)
	{
		psysMenu->EnableMenuItem(SC_CLOSE, TRUE);
	}
	GetDlgItem(IDC_BUTTON_DBTestConnect)->EnableWindow(FALSE);
	GetDlgItem(ID_BUTTON_SHOWIMG)->EnableWindow(FALSE);	
	
	GetDlgItem(IDCANCEL)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_STATIC_EXITSHOW)->ShowWindow(SW_SHOW);

	m_bExit  = true;	

	for (int i = 0; i< MAX_CAMERA_COUNT; i++)
	{
		if (g_CameraGroup[i])
		{			
			delete g_CameraGroup[i];
			g_CameraGroup[i] = NULL;
		}
	}
	StopToSaveDBData();
	m_bSaveLocalThreadExit = true ;
	m_bSaveRemoteThreadExit = true;
	if (m_hReadLocal)
	{
		CloseHandle(m_hReadLocal);
		m_hReadLocal = NULL;
	}
	if (m_hSaveLocal)
	{
		CloseHandle(m_hSaveLocal);
		m_hSaveLocal = NULL;
	}
	if (m_hReadRemote)
	{
		CloseHandle(m_hReadRemote);
		m_hReadRemote = NULL;
	}
	if (m_hSaveRemote)
	{
		CloseHandle(m_hSaveRemote);
		m_hSaveRemote = NULL;
	}
	for (int i = 0; i<MAX_THREADCOUNT; i++)
	{
		if (m_hSaveLocalDB[i])
		{
			CloseHandle(m_hSaveLocalDB[i]);
			m_hSaveLocalDB[i] = NULL;
		}
		if (m_hSaveRemoteDB[i])
		{
			CloseHandle(m_hSaveRemoteDB[i]);
			m_hSaveRemoteDB[i] =NULL;
		}
	}

	if (m_hSaveStatusToDB)
	{
		CloseHandle(m_hSaveStatusToDB);
		m_hSaveStatusToDB = NULL;
	}
	if (m_hCircleDelete)
	{
		CloseHandle(m_hCircleDelete);
		m_hCircleDelete = NULL;
	}

	DeleteCriticalSection(&m_csReadLocal);
	DeleteCriticalSection(&m_csSaveLocal);
	DeleteCriticalSection(&m_csReadRemote);
	DeleteCriticalSection(&m_csSaveRemote);
	

	Gdiplus::GdiplusShutdown(m_gdiplusToken);
	::CoUninitialize();
	DeleteCriticalSection(&m_csDlgLog);
	DeleteCriticalSection(&m_csDlgRemoteLog);
	DeleteCriticalSection(&m_csUpdateListCtrl);
	CDialog::OnClose();
}



void CSaveDataToDBDlg::OnBnClickedButtonShowimg()
{
	// TODO: Add your control notification handler code here
	GetDlgItem(ID_BUTTON_SHOWIMG)->EnableWindow(FALSE);
	HRESULT hr = S_FALSE;
	_ConnectionPtr pMyConnect = NULL;
	CString strListNo;
	GetDlgItem(IDC_EDIT_LISTNO)->GetWindowText(strListNo);
	if ("" == strListNo)
	{
		MessageBox("请输入正确的流水号");
		GetDlgItem(ID_BUTTON_SHOWIMG)->EnableWindow(TRUE);
		return;
	}

	try
	{		
		//hr = pMyConnect.CreateInstance(__uuidof(Connection));
		//CoInitialize(NULL);
		hr = pMyConnect.CreateInstance("ADODB.Connection");
		if (S_OK != hr)
		{
			MessageBox("实例化连接对象失败");
			GetDlgItem(ID_BUTTON_SHOWIMG)->EnableWindow(TRUE);
			return;
		}

		char chConnectString[MAX_PATH] = {0};		
		sprintf(chConnectString, "Provider=SQLOLEDB; Persist Security Info=False; Initial Catalog=%s; Data Source=%s; User ID=%s; Password=%s", m_chDBName, m_chServerIP, m_chDBUserID, m_chDBPassword);
		pMyConnect->CursorLocation =adUseClient;
		pMyConnect->IsolationLevel = adXactReadCommitted;
		hr = pMyConnect->Open(_bstr_t(chConnectString), "", "", adModeUnknown);

		if (FAILED(hr))
		{
			GetDlgItem(ID_BUTTON_SHOWIMG)->EnableWindow(TRUE);
			return;
		}

		_CommandPtr tmpCmd;
		_ParameterPtr Param1;
		_RecordsetPtr pMyRecord;

		if (FAILED(pMyRecord.CreateInstance(__uuidof(Recordset))))
		{
			MessageBox("实例化数据集失败");
			GetDlgItem(ID_BUTTON_SHOWIMG)->EnableWindow(TRUE);
			return;
		}

		hr = tmpCmd.CreateInstance(__uuidof(Command));
		tmpCmd->ActiveConnection = pMyConnect;

		tmpCmd->CommandText = _bstr_t("SELECT *  FROM CarFullView where ListNo = ?");		
		Param1 = tmpCmd->CreateParameter(_bstr_t(""), adVarChar, adParamInput,strListNo.GetLength(), _variant_t(strListNo.GetBuffer()));
		Param1->Value = _variant_t(strListNo.GetBuffer());
		tmpCmd->Prepared = VARIANT_TRUE;
		//tmpCmd->NamedParameters = VARIANT_TRUE;
		
		tmpCmd->Parameters->Append(Param1);		

		_variant_t recordsAffected;  
		pMyRecord = tmpCmd->Execute(&recordsAffected, NULL, adCmdText);
	
		pMyRecord->MoveFirst();

		while(VARIANT_FALSE == pMyRecord->adoEOF)
		{
			_variant_t vListNo = pMyRecord->Fields->GetItem("ListNo")->Value;
			long lacSize = pMyRecord->Fields->GetItem("ListNo")->ActualSize;

			const long lImgAcSize = pMyRecord->Fields->GetItem("FullViewPhoto")->ActualSize;
			_variant_t ImgData= pMyRecord->Fields->GetItem("FullViewPhoto")->GetChunk(lImgAcSize);

			BYTE* pByteImgData = NULL;
			SafeArrayAccessData(ImgData.parray, (void**)&pByteImgData);

			FILE* tempkk=fopen(".\\tempjpg.jpg","wb+");
			size_t kk2 = fwrite(pByteImgData, 1, lImgAcSize, tempkk);
			fclose(tempkk);
			tempkk = NULL;

			CWnd* ptmpCwnd = GetDlgItem( IDC_STATIC_SHOWPICTURE);
			ShowImg( GetDlgItem( IDC_STATIC_SHOWPICTURE),pByteImgData, lImgAcSize);
			SafeArrayUnaccessData(ImgData.parray);
			pMyRecord->MoveNext();
		}

		pMyRecord->Close();
		pMyRecord = NULL;
	}
	catch(_com_error e)
	{
		char errorInfo[MAX_PATH] = {0};
		sprintf(errorInfo,"查询失败, 错误信息：%s, 错误描述：%s ",e.ErrorMessage(), (char*)e.Description());		
		MessageBox(errorInfo);
	}
	pMyConnect->Close();
	pMyConnect = NULL;
	GetDlgItem(ID_BUTTON_SHOWIMG)->EnableWindow(TRUE);
}

void CSaveDataToDBDlg::ShowImg(CWnd * pWnd, PBYTE PBImgData, long iImgDataLen)
{
	if (NULL ==pWnd || NULL == PBImgData || iImgDataLen <1)
	{
		return;
	}
	IStream* pStream = NULL;
	CreateStreamOnHGlobal(NULL, TRUE, &pStream);
	if (NULL == pStream)
	{
		return;
	}

	LARGE_INTEGER LiTemp = {0};
	ULARGE_INTEGER ULiZero = {0};
	ULONG ulRealSize = 0;
	pStream->Seek(LiTemp, STREAM_SEEK_SET, NULL);
	pStream->SetSize(ULiZero);

	//将图片写入流中
	pStream->Write(PBImgData, iImgDataLen, &ulRealSize);
	//创建位图
	Bitmap bmpSrc(pStream);

	//设置显示图片的窗口的宽度和高度
	RECT rtWnd;
	pWnd->GetWindowRect(&rtWnd);
	int iWndWidth = rtWnd.right - rtWnd.left;
	int iWndHeight = rtWnd.bottom - rtWnd.top;
	CDC* pwnDC = NULL;
	pwnDC = pWnd->GetDC();
	if (NULL != pwnDC)
	{
		Graphics grDraw(pwnDC->m_hDC);
		RectF rtDst((REAL)0, (REAL)0, (REAL)iWndWidth, (REAL)iWndHeight);
		grDraw.DrawImage(&bmpSrc, rtDst, (REAL)0, (REAL)0, (REAL)bmpSrc.GetWidth(), (REAL)bmpSrc.GetHeight(), UnitPixel);
		pwnDC->ReleaseAttribDC();
	}
	pStream->Release();
	pStream= NULL;
	return;
}


void CSaveDataToDBDlg::OnBnClickedButtonDbtestconnect()
{
	// TODO: Add your control notification handler code here
	GetDlgItem(IDC_BUTTON_DBTestConnect)->EnableWindow(FALSE);
	

	CString strDBServerIP, strDBName, strDBUserID, strDBPassword;

	GetDlgItem(IDC_IPADDRESS_DBSever)->GetWindowText(strDBServerIP);
	GetDlgItem(IDC_EDIT_DBName)->GetWindowText(strDBName);
	GetDlgItem(IDC_EDIT_DBUserID)->GetWindowText(strDBUserID);
	GetDlgItem(IDC_EDIT_DBPassWord)->GetWindowText(strDBPassword);

	if ((0 ==  strDBServerIP.Compare("0.0.0.0")) || "" == strDBName || "" == strDBUserID || "" == strDBPassword)
	{
		MessageBox("请完整填入参数");
		GetDlgItem(IDC_BUTTON_DBTestConnect)->EnableWindow(TRUE);
		return;
	}

	HRESULT hr = S_FALSE;
	_ConnectionPtr tempDBConnect;

	try
	{
		hr = tempDBConnect.CreateInstance(_uuidof(Connection));
		if (S_OK != hr)
		{

			MessageBox("连接实例化失败");
			GetDlgItem(IDC_BUTTON_DBTestConnect)->EnableWindow(TRUE);
			return;
		}
		char chConnectString[MAX_PATH] = {0};		
		sprintf(chConnectString, "Provider=SQLOLEDB; Persist Security Info=False; Initial Catalog=%s; Data Source=%s; User ID=%s; Password=%s", strDBName, strDBServerIP, strDBUserID, strDBPassword);
		tempDBConnect->PutCommandTimeout(3);		//设置这个连接超时，似乎不起作用
		//tempDBConnect->CursorLocation = adUseClient;
		hr = tempDBConnect->Open(_bstr_t(chConnectString), "", "", adConnectUnspecified);

		if (S_OK == hr)
		{
			MessageBox("数据库可正常连接");		
		}
		else
		{
			MessageBox("数据库连接失败，请检查网络设置或参数是否输入正确");	
		}
		tempDBConnect->Close();
		tempDBConnect = NULL;
	}
	catch (_com_error &e)
	{
		char errorInfo[MAX_PATH] = {0};
		sprintf(errorInfo,"数据库连接失败, 错误信息：%s, 错误描述：%s ",e.ErrorMessage(), (char*)e.Description());
		MessageBox(errorInfo);
	}
	GetDlgItem(IDC_BUTTON_DBTestConnect)->EnableWindow(TRUE);
}

//************************************
// Method:    StopToSaveDBData
// FullName:  CSaveDataToDBDlg::StopToSaveDBData
// Description: 将仍在队列中的数据保存到本地，防止数据丢失
// Access:    public 
// Returns:   void
// Qualifier:
//************************************
void CSaveDataToDBDlg::StopToSaveDBData()
{
	m_bExit = true;
	while (m_lsReadLocal.size()> 0)
	{
		//本地数据库的保存
		//WaitForSingleObject(m_hSaveLocal, 70l);			//2015-01-19
		EnterCriticalSection(&m_csReadLocal);
		CameraResult* tempLocalResult =NULL;
		tempLocalResult = m_lsReadLocal.front();
		m_lsReadLocal.pop_front();			//从“读本地”队列取出数据

		EnterCriticalSection(&m_csSaveLocal);
		m_lsSaveLocal.push_back(tempLocalResult);		//存入“写本地”队列中
		LeaveCriticalSection(&m_csSaveLocal);

		LeaveCriticalSection(&m_csReadLocal);
		//ReleaseMutex(m_hSaveLocal);			//2015-01-19
	}
	while(m_lsReadRemote.size() > 0)
	{
		//中间库的数据保存
		//WaitForSingleObject(m_hSaveRemote, 70l);				//2015-01-19
		EnterCriticalSection(&m_csReadRemote);
		CameraResult* tempRemoteResult =NULL;
		tempRemoteResult = m_lsReadRemote.front();
		m_lsReadRemote.pop_front();			//从“读本地”队列取出数据

		EnterCriticalSection(&m_csSaveRemote);
		m_lsSaveRemote.push_back(tempRemoteResult);		//存入“写本地”队列中
		LeaveCriticalSection(&m_csSaveRemote);

		LeaveCriticalSection(&m_csReadRemote);
		//ReleaseMutex(m_hSaveRemote);
	}
	
	while(m_lsSaveLocal.size() > 0 || m_lsSaveRemote.size() > 0)
	{
		Sleep(3000);				//等待将队列中的数据保存到本地
	}
	m_bSaveLocalThreadExit = true ;
	m_bSaveRemoteThreadExit = true;
}

void CSaveDataToDBDlg::OnBnClickedButton1()
{
	// TODO: Add your control notification handler code here
	GetDlgItem(IDC_BUTTON_StarUpDateDB)->EnableWindow(FALSE);
	GetDlgItem(IDCANCEL)->EnableWindow(FALSE);
	SetDlgItemText(IDC_BUTTON_StarUpDateDB, "设备连接中，请稍后。。。");

	CMenu* psysMenu = GetSystemMenu(FALSE);
	if(NULL != psysMenu)
	{
		psysMenu->EnableMenuItem(SC_CLOSE, TRUE);
	}

	//连接设备
	_beginthreadex( NULL, 0, &ThreadConnectDevice, this, 0, NULL );

	////创建本地数据库插入的相关线程
	//m_hReadLocal = (HANDLE)_beginthreadex( NULL, 0, &ThreadReadLocal, this, 0, NULL );
	//m_hSaveLocal = (HANDLE)_beginthreadex( NULL, 0, &ThreadSaveLocal, this, 0, NULL );
	//m_hSaveLocalDB = (HANDLE)_beginthreadex( NULL, 0, &ThreadSaveLocalDB, this, 0, NULL );

	////创建远端数据库插入的相关线程
	//m_hReadRemote = (HANDLE)_beginthreadex( NULL, 0, &ThreadReadRemote, this, 0, NULL );
	//m_hSaveRemote = (HANDLE)_beginthreadex( NULL, 0, &ThreadSaveRemote, this, 0, NULL );
	//m_hSaveRemoteDB = (HANDLE)_beginthreadex( NULL, 0, &ThreadSaveRemoteDB, this, 0, NULL );

	if (m_bLocalDBEnable)
	{
		//创建本地数据库插入的相关线程
		m_hReadLocal = (HANDLE)_beginthreadex( NULL, 0, &ThreadReadLocal, this, 0, NULL );
		m_hSaveLocal = (HANDLE)_beginthreadex( NULL, 0, &ThreadSaveLocal, this, 0, NULL );
		for(int i = 0; i<MAX_THREADCOUNT; i++)
		{
			if (m_hSaveLocalDB[i] == NULL)
			{
				m_hSaveLocalDB[i] =(HANDLE)_beginthreadex( NULL, 0, &ThreadSaveLocalDB, this, 0, NULL );
			}	
		}
	}

	if (m_bRemoteDBEnable)
	{
		//创建远端数据库插入的相关线程
		m_hReadRemote = (HANDLE)_beginthreadex( NULL, 0, &ThreadReadRemote, this, 0, NULL );
		m_hSaveRemote = (HANDLE)_beginthreadex( NULL, 0, &ThreadSaveRemote, this, 0, NULL );
		for(int i = 0; i<MAX_THREADCOUNT; i++)
		{
			if (m_hSaveRemoteDB[i] == NULL)
			{
				m_hSaveRemoteDB[i] =(HANDLE)_beginthreadex( NULL, 0, &ThreadSaveRemoteDB, this, 0, NULL );
			}	
		}
	}

	//创建状态插入线程
	m_hSaveStatusToDB = (HANDLE)_beginthreadex(NULL, 0, &ThreadSafeStatuToDB, this, 0,NULL);
	//创建循环覆盖线程
	m_hCircleDelete = (HANDLE)_beginthreadex(NULL, 0, &ThreadCirclelaryDelete, this, 0, NULL);

	SetDlgItemText(IDC_BUTTON_StarUpDateDB, "启动数据录入");
	GetDlgItem(IDCANCEL)->EnableWindow(TRUE);
}

void CSaveDataToDBDlg::OnBnClickedCancel()
{
	// TODO: Add your control notification handler code here
	GetDlgItem(IDCANCEL)->EnableWindow(FALSE);
	SetDlgItemText(IDCANCEL, "程序退出中，请稍后。。。");
	GetDlgItem(IDC_STATIC_EXITSHOW)->ShowWindow(SW_SHOW);

	GetDlgItem(IDC_BUTTON_DBTestConnect)->EnableWindow(FALSE);
	GetDlgItem(ID_BUTTON_SHOWIMG)->EnableWindow(FALSE);

	CMenu* psysMenu = GetSystemMenu(FALSE);
	if(NULL != psysMenu)
	{
		psysMenu->EnableMenuItem(SC_CLOSE, TRUE);
	}

	m_bExit = true;
	for (int i = 0; i< MAX_CAMERA_COUNT; i++)
	{
		if (g_CameraGroup[i])
		{
			delete g_CameraGroup[i];
			g_CameraGroup[i] = NULL;
		}
	}
	
	StopToSaveDBData();
	GetDlgItem(IDC_BUTTON_StarUpDateDB)->EnableWindow(TRUE);
	OnCancel();
}

void CSaveDataToDBDlg::WriteDlgRemotLog( char* logBuf )
{
	if (!m_bLogEnable)
		return ;

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
	strCurrentDir.AppendFormat("\\LOG\\%04d-%02d-%02d\\DlgLog\\", pTM->tm_year + 1900, pTM->tm_mon + 1, pTM->tm_mday);
	MakeSureDirectoryPathExists(strCurrentDir);
	fileName.Format("%s%04d-%02d-%02d-%02d_DlgRemoteLog.log", strCurrentDir, pTM->tm_year + 1900, pTM->tm_mon + 1, pTM->tm_mday, pTM->tm_hour);

	EnterCriticalSection(&m_csDlgRemoteLog);
	FILE *file = NULL;
	file = fopen(fileName, "a+");
	if (file)
	{
		fprintf(file,"%04d-%02d-%02d %02d:%02d:%02d:%03d : %s\n",  pTM->tm_year + 1900, pTM->tm_mon + 1, pTM->tm_mday,
			pTM->tm_hour, pTM->tm_min, pTM->tm_sec, dwMS, logBuf);
		fclose(file);
		file = NULL;
	}
	LeaveCriticalSection(&m_csDlgRemoteLog);
}

bool CSaveDataToDBDlg::UpdateListCtrlView( CString& strDeviceID, CString& strCameraIP, CString& strCamerStatus,CString& strLocalResultCount, CString& strRemoteResultCount )
{
	EnterCriticalSection(&m_csUpdateListCtrl);

	int i = 0, j = 0;
	bool bFindListItem = false, bFindDataItem = false ;
	for ( i = 0; i< m_lsListCtrl.GetItemCount(); i++)
	{
		if (m_lsListCtrl.GetItemText(i, 1) ==  strDeviceID)
		{
			bFindListItem = true;
			break;
		}
	}

	for (j = 0; j < MAX_PATH; j++)
	{
		if (m_ListCtrlDataGroup[j].strDeviceID ==  strDeviceID)
		{
			bFindDataItem = true;
			break;
		}
	}

	if (bFindListItem)
	{
		if ("" != strCameraIP)
		{
			m_ListCtrlDataGroup[j].strDeviceIP = strCameraIP;
			m_lsListCtrl.SetItemText(i, 2, m_ListCtrlDataGroup[j].strDeviceIP);
		}
		if ("" != strCamerStatus)
		{
			m_ListCtrlDataGroup[j].strDeviceStatus = strCamerStatus;
			m_lsListCtrl.SetItemText(i, 3, m_ListCtrlDataGroup[j].strDeviceStatus);
		}
		if ("" != strLocalResultCount)
		{
			CString tempCount = m_lsListCtrl.GetItemText(i, 4);
			int iCount = _tstoi(tempCount);
			iCount++;
			tempCount.Format("%d", iCount);
			m_ListCtrlDataGroup[j].strLocalResultCount = tempCount;
			m_lsListCtrl.SetItemText(i, 4, m_ListCtrlDataGroup[j].strLocalResultCount);
		}
		if ("" != strRemoteResultCount)
		{
			CString tempCount = m_lsListCtrl.GetItemText(i, 5);
			int iCount = _tstoi(tempCount);
			iCount++;
			tempCount.Format("%d", iCount);			
			m_ListCtrlDataGroup[j].strRemoteResultCount = tempCount;
			m_lsListCtrl.SetItemText(i, 5, m_ListCtrlDataGroup[j].strRemoteResultCount);
		}
	}
	else
	{
		int i = 0;
		for (i; i < MAX_PATH; i++)
		{
			if ("" == m_ListCtrlDataGroup[i].strDeviceID)
			{
				break;
			}
		}

		char chLineNum[128] = {0};
		sprintf(chLineNum, "%d", i);

		m_ListCtrlDataGroup[i].strLineNum = chLineNum;
		m_ListCtrlDataGroup[i].strDeviceID = strDeviceID;
		m_ListCtrlDataGroup[i].strDeviceIP = strCameraIP;
		m_ListCtrlDataGroup[i].strDeviceStatus = strCamerStatus;

		LV_ITEM templvi;
		templvi.mask = LVIF_TEXT|LVIF_PARAM|LVIF_IMAGE;
		templvi.iItem = m_lsListCtrl.GetItemCount();	

		templvi.iSubItem = 0;
		templvi.pszText = (LPSTR)(LPCSTR)m_ListCtrlDataGroup[i].strLineNum;
		templvi.lParam = (LPARAM)&m_ListCtrlDataGroup[i];

		if (-1 == m_lsListCtrl.InsertItem(&templvi))
		{
			return false;
		}
		m_lsListCtrl.SetItemText(i, 0, chLineNum);
		//设置DeviceID
		templvi.iSubItem = 1;
		templvi.pszText = (LPSTR)(LPCSTR)m_ListCtrlDataGroup[i].strDeviceID;
		m_lsListCtrl.SetItem(&templvi);
		m_lsListCtrl.SetItemText(i, 1, m_ListCtrlDataGroup[i].strDeviceID);
		//templvi.lParam = (LPARAM)&m_ListCtrlDataGroup[i];

		if ("" != strCameraIP)
		{
			templvi.iSubItem  = 2;
			templvi.pszText = (LPSTR)(LPCSTR)m_ListCtrlDataGroup[i].strDeviceIP;
			m_lsListCtrl.SetItem(&templvi);
			m_lsListCtrl.SetItemText(i, 2, m_ListCtrlDataGroup[i].strDeviceIP);
		}
		if ("" != strCamerStatus)
		{
			templvi.iSubItem  = 3;
			templvi.pszText = (LPSTR)(LPCSTR)m_ListCtrlDataGroup[i].strDeviceStatus;
			m_lsListCtrl.SetItem(&templvi);
			m_lsListCtrl.SetItemText(i, 3, m_ListCtrlDataGroup[i].strDeviceStatus);
		}
		if ("" != strLocalResultCount)
		{
			CString tempCount = m_lsListCtrl.GetItemText(i, 4);
			int iCount = _tstoi(tempCount);
			iCount++;
			tempCount.Format("%d", iCount);
			m_ListCtrlDataGroup[i].strLocalResultCount = tempCount;

			templvi.iSubItem  = 4;
			templvi.pszText = (LPSTR)(LPCSTR)m_ListCtrlDataGroup[i].strLocalResultCount;
			m_lsListCtrl.SetItem(&templvi);
			m_lsListCtrl.SetItemText(i, 4, m_ListCtrlDataGroup[i].strLocalResultCount);
		}
		else
		{
			templvi.iSubItem  = 4;			
			m_ListCtrlDataGroup[i].strLocalResultCount.Format("0");
			templvi.pszText = (LPSTR)(LPCSTR)m_ListCtrlDataGroup[i].strLocalResultCount;
			m_lsListCtrl.SetItem(&templvi);
			m_lsListCtrl.SetItemText(i, 4, m_ListCtrlDataGroup[i].strLocalResultCount);
		}

		if ("" != strRemoteResultCount)
		{
			CString tempCount = m_lsListCtrl.GetItemText(i, 5);
			int iCount = _tstoi(tempCount);
			iCount++;
			tempCount.Format("%d", iCount);
			m_ListCtrlDataGroup[i].strRemoteResultCount= tempCount;

			templvi.iSubItem  = 5;
			templvi.pszText =(LPSTR)(LPCSTR)m_ListCtrlDataGroup[i].strRemoteResultCount;
			m_lsListCtrl.SetItem(&templvi);
			m_lsListCtrl.SetItemText(i, 5, m_ListCtrlDataGroup[i].strRemoteResultCount);
		}
		else
		{
			templvi.iSubItem  = 5;
			m_ListCtrlDataGroup[i].strRemoteResultCount.Format("0");
			templvi.pszText = (LPSTR)(LPCSTR)m_ListCtrlDataGroup[i].strRemoteResultCount;
			m_lsListCtrl.SetItem(&templvi);
			m_lsListCtrl.SetItemText(i, 5, m_ListCtrlDataGroup[i].strRemoteResultCount);
		}
	}
	LeaveCriticalSection(&m_csUpdateListCtrl);
	return true;
}

void CSaveDataToDBDlg::OnBnClickedCheckAutoconect()
{

	char chFileName[MAX_PATH];
	GetModuleFileNameA(NULL, chFileName, MAX_PATH-1);
	PathRemoveFileSpecA(chFileName);
	char chIniFileName[MAX_PATH] = { 0 };
	strcpy(chIniFileName, chFileName);
	strcat(chIniFileName, "\\HvConfig.ini");

	// TODO: Add your control notification handler code here
	if (BST_CHECKED == IsDlgButtonChecked(IDC_CHECK_AUTOCONECT))
	{
		WritePrivateProfileStringA("AutoRun","RunEnable","1",chIniFileName);
	}
	else
	{
		WritePrivateProfileStringA("AutoRun","RunEnable","0",chIniFileName);
	}
}

void CSaveDataToDBDlg::OnLvnColumnclickListDevice(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	// TODO: Add your control notification handler code here
	m_lsListCtrl.SortItems(MyCompareProc, pNMLV->iSubItem);
	*pResult = 0;
}

//对IP进行升序排列
bool asscendingSort(CString& str1,CString &str2)
{
	if (str1 == str2)
	{
		return true;
	}
	int a1,a2,a3,a4,b1,b2,b3,b4;
	sscanf(str1.GetBuffer(),"%d.%d.%d.%d",&a1,&a2,&a3,&a4);
	sscanf(str2.GetBuffer(),"%d.%d.%d.%d",&b1,&b2,&b3,&b4);
	if(a1 == b1)
	{
		if (a2 == b2)
		{
			if (a3 == b3)
			{
				if (a4 == b4)
				{
					return true;
				} 
				else
				{
					return (a4 < b4 ? true : false);
				}
			} 
			else
			{
				return (a3 < b3 ? true : false);
			}
		}
		else
		{
			return (a2 < b2 ? true : false);
		}
	}
	else
	{
		return (a1 < b1 ? true : false);
	}
}
int CALLBACK CSaveDataToDBDlg::MyCompareProc( LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort )
{
	ListItemData* pItem1 = (ListItemData*)lParam1;
	ListItemData* pItem2 = (ListItemData*)lParam2;

	int nRet = 0;
	switch(lParamSort)
	{
	case (0):
		if (pItem1->strLineNum < pItem2->strLineNum)
		{
			nRet = -1;
		}
		else if (pItem1->strLineNum > pItem2->strLineNum)
		{
			nRet = 1;
		}
		else
		{
			nRet = 0;
		}
		break;
	case (1):
		if (pItem1->strDeviceID < pItem2->strDeviceID)
		{
			nRet = -1;			
		}
		else if(pItem1->strDeviceID > pItem2->strDeviceID)
		{
			nRet = 1;
		}
		else
		{
			nRet = 0;
		}
		break;
	case (2):
		if (pItem1->strDeviceIP == pItem2->strDeviceIP )
		{
			nRet =0;			
		}
		else if( asscendingSort(pItem1->strDeviceIP,  pItem2->strDeviceIP))
		{
			nRet = -1;
		}
		else
		{
			nRet = 1;
		}
		break;
	case (3):
		if ("连接正常" == pItem1->strDeviceStatus  && "连接正常" !=  pItem2->strDeviceStatus)
		{
			nRet = -1;			
		}
		else if(pItem1->strDeviceStatus  ==  pItem2->strDeviceStatus)
		{
			nRet = 0;
		}
		else
		{
			nRet = 1;
		}
		break;
	case (4):
		if (pItem1->strLocalResultCount < pItem2->strLocalResultCount)
		{
			nRet = -1;			
		}
		else if(pItem1->strLocalResultCount > pItem2->strLocalResultCount)
		{
			nRet = 1;
		}
		else
		{
			nRet = 0;
		}
		break;
	case (5):
		if (pItem1->strRemoteResultCount < pItem2->strRemoteResultCount)
		{
			nRet = -1;			
		}
		else if(pItem1->strRemoteResultCount > pItem2->strRemoteResultCount)
		{
			nRet = 1;
		}
		else
		{
			nRet = 0;
		}
		break;
	default:
		break;
	}

	return nRet;
}

BOOL CSaveDataToDBDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	// TODO: Add your message handler code here and/or call default

	if (m_bExit)
	{
		HCURSOR newCurser = LoadCursor(NULL, IDC_WAIT);
		::SetCursor(newCurser);
		return TRUE;
	}

	return CDialog::OnSetCursor(pWnd, nHitTest, message);
}

unsigned __stdcall CSaveDataToDBDlg::ThreadConnectDevice( void* TheParam )
{
	if(NULL  == TheParam)
		return -1;
	CSaveDataToDBDlg* pThis = (CSaveDataToDBDlg*)TheParam;
	pThis->ConnectDevice();
	return 0;
}

void CSaveDataToDBDlg::ConnectDevice( void )
{
	LocalDataBaseControler tempDB;
	char chTempConnectInfo[256] = {0};
	tempDB.ConnectToDB(chTempConnectInfo);
	tempDB.InitCameraGroup(g_CameraGroup);
	WriteDlgLog(chTempConnectInfo);
	tempDB.CloseDBConnect();
	for (int i = 0; i< MAX_CAMERA_COUNT; i++)
	{
		if (g_CameraGroup[i])
		{
			CString strDeviceID;
			strDeviceID.Format("%d", g_CameraGroup[i]->m_iDiviceID);
			CString strDeviceIP(g_CameraGroup[i]->m_strIp.c_str());
			CString strDeviceStatus;
			CString strResultCount("");
			//UpdateListCtrlView(strDeviceID, strDeviceIP, strDeviceStatus, strResultCount, strResultCount);
			//if (!g_CameraGroup[i]->SetListAndMutex(&m_lsReadLocal, &m_hReadLocal, &m_lsReadRemote, &m_hReadRemote))
			if( !g_CameraGroup[i]->SetListAndCriticalSection(&m_lsReadLocal, &m_csReadLocal, &m_lsReadRemote, &m_csReadRemote) )
			{
				continue;
			}
			int iRet = g_CameraGroup[i]->OpenDevice();
			if (0 == iRet)
			{
				strDeviceStatus.Format("连接正常");
			}
			else
			{
				strDeviceStatus.Format("连接已断开");
			}
			UpdateListCtrlView(strDeviceID, strDeviceIP, strDeviceStatus, strResultCount, strResultCount);
		}
	}

	GetDlgItem(IDCANCEL)->EnableWindow(TRUE);
	CMenu* psysMenu = GetSystemMenu(FALSE);
	if(NULL != psysMenu)
	{
		psysMenu->EnableMenuItem(SC_CLOSE, FALSE);
	}
}
