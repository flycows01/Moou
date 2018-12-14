
// USysLoadDlg.h: 头文件
//

#pragma once
#include <Winsvc.h>

// CUSysLoadDlg 对话框
class CUSysLoadDlg : public CDialogEx
{
// 构造
public:
	CUSysLoadDlg(CWnd* pParent = nullptr);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_USYSLOAD_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	// 驱动路径
	CString m_FilePath;
	CString m_SymName;
	CString m_FileSize;
	CString m_Result;
	CString m_LastError;
	WCHAR m_szDir[MAX_PATH];
	SC_HANDLE m_hServiceDDK;
	SC_HANDLE m_hServiceMgr;
	SERVICE_STATUS m_Status;
	LPCTSTR m_lpszDriverName;

	afx_msg void OnDropFiles(HDROP hDropInfo);
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedButton2();
	afx_msg void OnBnClickedButton3();
	afx_msg void OnBnClickedButton4();
};
