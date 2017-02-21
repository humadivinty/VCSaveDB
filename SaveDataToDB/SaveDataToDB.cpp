// SaveDataToDB.cpp : ����Ӧ�ó��������Ϊ��
//

#include "stdafx.h"
#include "SaveDataToDB.h"
#include "SaveDataToDBDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CSaveDataToDBApp

BEGIN_MESSAGE_MAP(CSaveDataToDBApp, CWinApp)
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()


// CSaveDataToDBApp ����

CSaveDataToDBApp::CSaveDataToDBApp()
{
	// TODO: �ڴ˴���ӹ�����룬
	// ��������Ҫ�ĳ�ʼ�������� InitInstance ��
}


// Ψһ��һ�� CSaveDataToDBApp ����

CSaveDataToDBApp theApp;


// CSaveDataToDBApp ��ʼ��

BOOL CSaveDataToDBApp::InitInstance()
{
	::CreateMutex(NULL, TRUE, "SaceDataToDB");
	if(ERROR_ALREADY_EXISTS == GetLastError())
	{
		AfxMessageBox("�ó����Ѿ�����,���ȹر������еĳ����ٴ򿪣�");
		exit(0);
	}

	// ���һ�������� Windows XP �ϵ�Ӧ�ó����嵥ָ��Ҫ
	// ʹ�� ComCtl32.dll �汾 6 ����߰汾�����ÿ��ӻ���ʽ��
	//����Ҫ InitCommonControls()�����򣬽��޷��������ڡ�
	InitCommonControls();

	CWinApp::InitInstance();

	AfxEnableControlContainer();

	// ��׼��ʼ��
	// ���δʹ����Щ���ܲ�ϣ����С
	// ���տ�ִ���ļ��Ĵ�С����Ӧ�Ƴ�����
	// ����Ҫ���ض���ʼ������
	// �������ڴ洢���õ�ע�����
	// TODO: Ӧ�ʵ��޸ĸ��ַ�����
	// �����޸�Ϊ��˾����֯��
	SetRegistryKey(_T("Ӧ�ó��������ɵı���Ӧ�ó���"));

	CSaveDataToDBDlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: �ڴ˷��ô����ʱ�á�ȷ�������ر�
		//�Ի���Ĵ���
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: �ڴ˷��ô����ʱ�á�ȡ�������ر�
		//�Ի���Ĵ���
	}

	// ���ڶԻ����ѹرգ����Խ����� FALSE �Ա��˳�Ӧ�ó���
	// ����������Ӧ�ó������Ϣ�á�
	return FALSE;
}
