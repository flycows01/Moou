
// UPack-V1Dlg.h : 头文件
//

#pragma once


// CUPackV1Dlg 对话框
class CUPackV1Dlg : public CDialogEx
{
// 构造
public:
	CUPackV1Dlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_UPACKV1_DIALOG };

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
	afx_msg void OnBnClickedButton1();
	afx_msg void OnDropFiles(HDROP hDropInfo);
	CString m_strEdit;
	TCHAR m_szDir[MAX_PATH];
	afx_msg void OnBnClickedButton2();
};
