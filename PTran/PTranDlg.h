
// PTranDlg.h : header file
//

#pragma once

#include "Resource.h"

#define SERVICE_ON  1
#define SERVICE_OFF 2
#define SERVICE_WAIT 3
#define WM_NC (WM_USER+1001)

struct servicePara;//pre declaration

// CPTranDlg dialog
class CPTranDlg : public CDialogEx
{
// Construction
public:
	CPTranDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_PTRAN_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButtonStart();
	afx_msg void OnBnClickedButtonStop();
	afx_msg LRESULT OnNotifyIcon(WPARAM wParam,LPARAM IParam);
	BOOL PreTranslateMessage(MSG* pMsg);
	DWORD m_IPAdressOut;
	int m_PortIn;
	int m_PortOut;
	CString m_Key;
	CString m_ConCountStr;
	char m_ctrl[4];
	int m_stat;
	int m_ConCount;
	char m_ip[32];
	servicePara *m_servicePara;
	NOTIFYICONDATA NotifyIcon;
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnBnClickedButtonHide();
};
