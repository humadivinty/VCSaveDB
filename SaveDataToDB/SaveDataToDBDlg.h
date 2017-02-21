// SaveDataToDBDlg.h : ͷ�ļ�
//

#pragma once

#include <list>
#include "CameraReault.h"
#include "afxcmn.h"

// CSaveDataToDBDlg �Ի���
class CSaveDataToDBDlg : public CDialog
{
// ����
public:
	CSaveDataToDBDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
	enum { IDD = IDD_SAVEDATATODB_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��

// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

public:
	ULONG_PTR m_gdiplusToken;
	Gdiplus::GdiplusStartupInput StartupInput;

	HANDLE m_hReadLocal;
	HANDLE m_hSaveLocal;
	HANDLE m_hRLocalMutex;
	HANDLE m_HWLocalMutex;
	std::list<CameraResult*> m_lsReadLocal;
	std::list<CameraResult*> m_lsSaveLocal;

	HANDLE m_hReadRemote;
	HANDLE m_hSaveRemote;
	HANDLE m_hRRemoteMutex;
	HANDLE m_HWRemoteMutex;
	std::list<CameraResult*>m_lsReadRemote;
	std::list<CameraResult*>m_lsSaveRemote;

	HANDLE m_hSaveLocalDB;
	HANDLE m_hSaveRemoteDB;

	HANDLE m_hSaveStatusToDB;

	HANDLE m_hCircleDelete;

	bool m_bLocalDBEnable;
	bool m_bRemoteDBEnable;
	bool m_bLogEnable;

	CString m_strLocalFilePath;			//�������ݻ����ļ�
	CString m_strRemoteFilePath;		//�м�����ݻ����ļ�

	CString m_strBackUpResultPath;	//�����ļ�Ŀ¼
	CString m_strBackUpLocalResultPath;
	CString m_strBackUpRemolResultPath;
	CString m_strLogPath;

	bool m_bExit;
	bool m_bSaveLocalThreadExit;
	bool m_bSaveRemoteThreadExit;
	int m_iStatusUpdateInterval;
	int m_iBackUpResultDays;
	int m_iBackUpLogDays;

	CRITICAL_SECTION m_csDlgLog;
	CRITICAL_SECTION m_csDlgRemoteLog;

	char m_chServerIP[MAX_PATH];
	char m_chDBName[MAX_PATH];
	char m_chDBUserID[MAX_PATH];
	char m_chDBPassword[MAX_PATH];

	CRITICAL_SECTION m_csReadLocal;
	CRITICAL_SECTION m_csSaveLocal;
	CRITICAL_SECTION m_csReadRemote;
	CRITICAL_SECTION m_csSaveRemote;


public:
	static unsigned __stdcall ThreadSaveLocal(void* TheParam);
	void SaveLocal(void);
	static unsigned __stdcall ThreadReadLocal(void* TheParam);
	void ReadLocal(void);

	static unsigned __stdcall ThreadSaveRemote(void* TheParam);
	void SaveRemote(void);
	static unsigned __stdcall ThreadReadRemote(void* TheParam);
	void ReadRemote(void);

	static unsigned __stdcall ThreadSaveLocalDB(void* TheParam);
	void SaveLocalDB(void);
	static unsigned __stdcall ThreadSaveRemoteDB(void* TheParam);
	void SaveRemoteDB(void);

	static unsigned __stdcall ThreadSafeStatuToDB(void* TheParam);
	void SafeStatuToDB( void);

	static unsigned __stdcall ThreadCirclelaryDelete(void*  TheParam);
	bool GetDiskFreeSpaceT(char* szDiskChar, ULONGLONG& DiskFreeSpace);
	int CirclelaryDelete(char* folderPath, int iBackUpDays);
	bool DeleteDirectory(char* strDirName);

	void StopToSaveDBData();

	//��ȡ��ز���
	void ReadInitFileDlg(void);
	void WriteDlgLog(char* logBuf);							//�������������־
	void WriteDlgRemotLog(char* logBuf);					//�м�����������־
	void ShowMessage(char* messageBuf);
	void ShowRemotDBMessage(char* messageBuf);
	// //��ʾ���ؿ������Ϣ
	CListBox m_MessageListBox;
	// //��ʾ�м��������Ϣ
	CListBox m_listBoxRemoteDB;
	afx_msg void OnClose();
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedButtonShowimg();
	void ShowImg(CWnd * pWnd, PBYTE PBImgData, long iImgDataLen);


	//long m_lDlgWidth;
	//long m_lDlgHeight;
	//long m_lWidth;
	//long m_lHeight;
	//float m_flMutipleWidth;
	//float m_flMutipleHeight;
	//bool m_bsizeChangeFlag;
	//void ReSize(int iID);
	/*afx_msg void OnSize(UINT nType, int cx, int cy);*/
	afx_msg void OnBnClickedButtonDbtestconnect();
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedCancel();
};
