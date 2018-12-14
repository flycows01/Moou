
// UPack-V1Dlg.cpp : 实现文件
//

#include "stdafx.h"
#include "UPack-V1.h"
#include "UPack-V1Dlg.h"
#include "afxdialogex.h"
#include "../UPackDll/UPackDll.h"
#pragma comment(lib,"../Debug/UPackDll.lib")

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
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

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CUPackV1Dlg 对话框



CUPackV1Dlg::CUPackV1Dlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CUPackV1Dlg::IDD, pParent)
	, m_strEdit(_T("C:\\Users\\mayn\\Desktop\\mfctest.exe"))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CUPackV1Dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, m_strEdit);
}

BEGIN_MESSAGE_MAP(CUPackV1Dlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON1, &CUPackV1Dlg::OnBnClickedButton1)
	ON_WM_DROPFILES()
	ON_BN_CLICKED(IDC_BUTTON2, &CUPackV1Dlg::OnBnClickedButton2)
END_MESSAGE_MAP()


// CUPackV1Dlg 消息处理程序

BOOL CUPackV1Dlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO:  在此添加额外的初始化代码

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CUPackV1Dlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CUPackV1Dlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
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
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CUPackV1Dlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CUPackV1Dlg::OnBnClickedButton1()
{
	// TODO:  在此添加控件通知处理程序代码

	//必须初始化
	UpdateData(TRUE);
	//设置要浏览文件的类型
	static TCHAR BASED_CODE szFilter[] = _T("可执行文件 (*.exe)|*.exe|")
		_T("DLL文件 (*.dll)|*.dll|")
		_T("All Files (*.*)|*.*||");

	CFileDialog fileDlg(TRUE, _T("exe"), _T(""),
		OFN_FILEMUSTEXIST | OFN_HIDEREADONLY, szFilter);

	if (fileDlg.DoModal() == IDOK)
	{
		//将获取的路径信息赋给变量
		m_strEdit = fileDlg.GetPathName();
	}
	//更新控件显示信息
	UpdateData(FALSE);
	
}

void CUPackV1Dlg::OnDropFiles(HDROP hDropInfo)
{
	// TODO:  在此添加消息处理程序代码和/或调用默认值
	
	DragQueryFile(hDropInfo, 0, m_szDir, MAX_PATH);
	
	m_strEdit = m_szDir;
	DragFinish(hDropInfo);
	UpdateData(FALSE);

	CDialogEx::OnDropFiles(hDropInfo);
}


void CUPackV1Dlg::OnBnClickedButton2()
{
	// TODO:  在此添加控件通知处理程序代码
	LPTSTR lpsz = new TCHAR[m_strEdit.GetLength() + 1];
	_tcscpy(lpsz, m_strEdit);
	UPack(lpsz);
}
