
// UPack-V1Dlg.h : ͷ�ļ�
//

#pragma once


// CUPackV1Dlg �Ի���
class CUPackV1Dlg : public CDialogEx
{
// ����
public:
	CUPackV1Dlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
	enum { IDD = IDD_UPACKV1_DIALOG };

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
	afx_msg void OnBnClickedButton1();
	afx_msg void OnDropFiles(HDROP hDropInfo);
	CString m_strEdit;
	TCHAR m_szDir[MAX_PATH];
	afx_msg void OnBnClickedButton2();
};
