
// USysLoadDlg.cpp: 实现文件
//

#include "stdafx.h"
#include "USysLoad.h"
#include "USysLoadDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CUSysLoadDlg 对话框



CUSysLoadDlg::CUSysLoadDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_USYSLOAD_DIALOG, pParent)
	, m_FilePath(_T(""))
	, m_LastError(_T(""))
	, m_SymName(_T(""))
	, m_FileSize(_T(""))
	, m_Result(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CUSysLoadDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, m_FilePath);
	DDX_Text(pDX, IDC_EDIT2, m_SymName);
	DDX_Text(pDX, IDC_EDIT3, m_FileSize);
	DDX_Text(pDX, IDC_EDIT4, m_Result);
	DDX_Text(pDX, IDC_EDIT5, m_LastError);
}

BEGIN_MESSAGE_MAP(CUSysLoadDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_DROPFILES()
	ON_BN_CLICKED(IDC_BUTTON1, &CUSysLoadDlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, &CUSysLoadDlg::OnBnClickedButton2)
	ON_BN_CLICKED(IDC_BUTTON3, &CUSysLoadDlg::OnBnClickedButton3)
	ON_BN_CLICKED(IDC_BUTTON4, &CUSysLoadDlg::OnBnClickedButton4)
END_MESSAGE_MAP()


// CUSysLoadDlg 消息处理程序

BOOL CUSysLoadDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
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

	// TODO: 在此添加额外的初始化代码

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CUSysLoadDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CUSysLoadDlg::OnPaint()
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
HCURSOR CUSysLoadDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CUSysLoadDlg::OnDropFiles(HDROP hDropInfo)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	DragQueryFile(hDropInfo, 0, m_szDir, MAX_PATH);

	m_FilePath = m_szDir;
	DragFinish(hDropInfo);
	DWORD dwLen = wcslen(m_szDir);
	WCHAR DriverName[20] = {0};
	int j = 0;
	for (int i = dwLen - 5; dwLen > 0; i--)
	{
		if (m_szDir[i] == 92)
		{
			break;
		}
		DriverName[j] = m_szDir[i];
		++j;
	}
	_wcsrev(DriverName);
	m_lpszDriverName = DriverName;
	m_SymName = (CString)DriverName;
	UpdateData(FALSE);

	CDialogEx::OnDropFiles(hDropInfo);
}


void CUSysLoadDlg::OnBnClickedButton1()
{
	// TODO: 在此添加控件通知处理程序代码
	m_Result = L"";
	//使用OpenSCManager函数打开SCM
	m_hServiceMgr = OpenSCManager(NULL,NULL,SC_MANAGER_ALL_ACCESS);
	//使用CreateService函数利用SCM句柄创建一个服务
	m_hServiceDDK = CreateService(m_hServiceMgr,    //SMC句柄
		                           m_lpszDriverName,
		                           m_lpszDriverName,
		                           SERVICE_ALL_ACCESS,              //驱动服务权限
		                           SERVICE_KERNEL_DRIVER,           //服务类型
		                           SERVICE_DEMAND_START,            //启动方式
		                           SERVICE_ERROR_IGNORE,            //错误控制
		                           m_FilePath,
		                           NULL,                            //加载组命令
		                           NULL,                            //TagId
		                           NULL,                            //依存关系
		                           NULL,                            //服务启动名
		                           NULL                             //密码
		                            );
	if (m_hServiceDDK == NULL)
	{
		DWORD dwRtn = GetLastError();
		if (dwRtn != ERROR_IO_PENDING && dwRtn != ERROR_SERVICE_EXISTS)
		{
			//由于其他原因创建服务失败
			m_Result = L"加载服务失败！";
			char ErrCode[5] = {};
			sprintf(ErrCode,"%d",dwRtn);
			m_LastError = (CString)ErrCode;
			UpdateData(FALSE);
			return;
		}
		else
		{
			//服务创建失败，是由于服务已经创立过
			m_hServiceDDK = OpenService(m_hServiceMgr, m_lpszDriverName, SERVICE_ALL_ACCESS);
			m_Result = L"服务已加载过，现已开启！";
			UpdateData(FALSE);
			return;
		}
	}
	else
	{
		m_Result = L"加载服务成功！";
		UpdateData(FALSE);
		return;
	}
}


void CUSysLoadDlg::OnBnClickedButton2()
{
	// TODO: 在此添加控件通知处理程序代码
	m_Result = L"";
	m_LastError = L"";
	if (StartService(m_hServiceDDK, NULL, NULL) == FALSE)
	{
		m_Result = L"启动服务失败！";
		DWORD dwRtn = GetLastError();
		char ErrCode[5] = {};
		sprintf(ErrCode, "%d", dwRtn);
		m_LastError = (CString)ErrCode;
		UpdateData(FALSE);
		return;
	}
	//等待服务启动完成
	while (QueryServiceStatus(m_hServiceDDK, &m_Status) == TRUE)
	{
		Sleep(m_Status.dwWaitHint);
		if (m_Status.dwCurrentState == SERVICE_RUNNING)
		{
			m_Result = L"启动服务成功！";
			UpdateData(FALSE);
			return;
		}
	}
}



void CUSysLoadDlg::OnBnClickedButton3()
{
	// TODO: 在此添加控件通知处理程序代码
	m_Result = L"";
	m_LastError = L"";
	if (ControlService(m_hServiceDDK, SERVICE_CONTROL_STOP, &m_Status) == FALSE)
	{
		m_Result = L"停止服务失败！";
		DWORD dwRtn = GetLastError();
		char ErrCode[5] = {};
		sprintf(ErrCode, "%d", dwRtn);
		m_LastError = (CString)ErrCode;
		UpdateData(FALSE);
		return;
	}
	//等待服务停止
	while (QueryServiceStatus(m_hServiceDDK, &m_Status) == TRUE)
	{
		Sleep(m_Status.dwWaitHint);
		if (m_Status.dwCurrentState == SERVICE_STOPPED)
		{
			m_Result = L"停止服务成功！";
			UpdateData(FALSE);
			return;
		}
	}
}


void CUSysLoadDlg::OnBnClickedButton4()
{
	// TODO: 在此添加控件通知处理程序代码
	m_Result = L"";
	m_LastError = L"";
	if (!DeleteService(m_hServiceDDK))
	{
		CloseServiceHandle(m_hServiceDDK);
		CloseServiceHandle(m_hServiceMgr);
		m_Result = L"卸载服务失败！";
		DWORD dwRtn = GetLastError();
		char ErrCode[5] = {};
		sprintf(ErrCode, "%d", dwRtn);
		m_LastError = (CString)ErrCode;
		UpdateData(FALSE);
		return;
	}
	else
	{
		CloseServiceHandle(m_hServiceDDK);
		CloseServiceHandle(m_hServiceMgr);
		m_Result = L"卸载服务成功！";
		UpdateData(FALSE);
		return;
	}
	//等待服务停止
}
