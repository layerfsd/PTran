
// PTranDlg.cpp : implementation file
//

#include "stdafx.h"
#include "PTran.h"
#include "PTranDlg.h"
#include "afxdialogex.h"
#include "Transmit.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

	// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	// Implementation
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


// CPTranDlg dialog




CPTranDlg::CPTranDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CPTranDlg::IDD, pParent)
	, m_PortIn(0)
	, m_PortOut(0)
	, m_Key(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDI_ICON1);
	m_IPAdressOut = 0;
	m_PortIn = 0;
	m_PortOut = 0;
}

void CPTranDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_IPAddress(pDX, IDC_IPADDRESS_Out, m_IPAdressOut);
	DDX_Text(pDX, IDC_EDIT_PortIn, m_PortIn);
	DDV_MinMaxInt(pDX, m_PortIn, 1, 65535);
	DDX_Text(pDX, IDC_EDIT_PortOut, m_PortOut);
	DDV_MinMaxInt(pDX, m_PortOut, 1, 65535);
	DDX_Text(pDX, IDC_EDIT_Key, m_Key);
	DDV_MaxChars(pDX, m_Key, 30);
}

BEGIN_MESSAGE_MAP(CPTranDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_Start, &CPTranDlg::OnBnClickedButtonStart)
	ON_BN_CLICKED(IDC_BUTTON_Stop, &CPTranDlg::OnBnClickedButtonStop)
	ON_MESSAGE(WM_NC,&CPTranDlg::OnNotifyIcon)
	ON_BN_CLICKED(IDC_BUTTON_Hide,CPTranDlg::OnBnClickedButtonHide)
END_MESSAGE_MAP()


// CPTranDlg message handlers

BOOL CPTranDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
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

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here

	m_IPAdressOut=0x7f000001;//default IP
	m_PortIn=2222;
	m_PortOut=8087;
	m_Key="";//default key
	m_stat=SERVICE_OFF;
	m_ConCount=0;
	UpdateData(FALSE);
	m_servicePara=new servicePara();
	//System Tray
	NotifyIcon.cbSize=sizeof(NOTIFYICONDATA);
	NotifyIcon.hIcon=AfxGetApp()->LoadIcon(IDI_ICON1);
	NotifyIcon.hWnd=m_hWnd;
	lstrcpy(NotifyIcon.szTip,_T("PTran"));
	NotifyIcon.uCallbackMessage=WM_NC;
	NotifyIcon.uFlags=NIF_ICON | NIF_MESSAGE | NIF_TIP;
	Shell_NotifyIcon(NIM_ADD,&NotifyIcon);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CPTranDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CPTranDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CPTranDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CPTranDlg::OnBnClickedButtonStart()
{
	// TODO: Add your control notification handler code here
	if(m_stat!=SERVICE_OFF)//SERVICE ON
		return;
	UpdateData(TRUE);
	HANDLE hThread=NULL;
	unsigned dwThreadID;
	//set Parameters	
	m_servicePara->portIn=m_PortIn;
	m_servicePara->portOut=m_PortOut;
	//convert IP Address
	in_addr add;
	add.S_un.S_addr=htonl(m_IPAdressOut);	
	strcpy_s(m_ip,inet_ntoa(add));
	m_servicePara->hostOut=m_ip;
	m_servicePara->key=(LPSTR)(LPCTSTR)m_Key;//Encrypt key
	m_ctrl[0]='o';
	m_ConCount=0;	
	m_ConCountStr.Format(_T("%d"),m_ConCount);
	GetDlgItem(IDC_STATIC_CNT)->SetWindowText(m_ConCountStr);
	m_servicePara->cmd=m_ctrl;//control buffer
	m_servicePara->dlgUi=this;//ui handle
	//start service Thread
	hThread=(HANDLE)_beginthreadex(NULL, 0, &ServiceMain, (LPVOID)m_servicePara, 0, &dwThreadID);
	CloseHandle(hThread);
	//set Flag
	m_stat=SERVICE_WAIT;
	GetDlgItem(IDC_STATIC_ST)->SetWindowText(_T("等待建立连接"));
	((CEdit *)GetDlgItem(IDC_EDIT_PortIn))->SetReadOnly(TRUE);
	((CEdit *)GetDlgItem(IDC_EDIT_PortOut))->SetReadOnly(TRUE);
	((CEdit *)GetDlgItem(IDC_EDIT_Key))->SetReadOnly(TRUE);
	((CIPAddressCtrl *)GetDlgItem(IDC_IPADDRESS_Out))->EnableWindow(FALSE);
	((CButton *)GetDlgItem(IDC_BUTTON_Stop))->EnableWindow(TRUE);
	((CButton *)GetDlgItem(IDC_BUTTON_Start))->EnableWindow(FALSE);
}


void CPTranDlg::OnBnClickedButtonStop()
{
	// TODO: Add your control notification handler code here
	m_ctrl[0]='x'; //set control  flag to stop Service	
	((CEdit *)GetDlgItem(IDC_EDIT_PortIn))->SetReadOnly(FALSE);
	((CEdit *)GetDlgItem(IDC_EDIT_PortOut))->SetReadOnly(FALSE);
	((CEdit *)GetDlgItem(IDC_EDIT_Key))->SetReadOnly(FALSE);
	((CIPAddressCtrl *)GetDlgItem(IDC_IPADDRESS_Out))->EnableWindow(TRUE);
	((CButton *)GetDlgItem(IDC_BUTTON_Start))->EnableWindow(TRUE);
	((CButton *)GetDlgItem(IDC_BUTTON_Stop))->EnableWindow(FALSE);	
	if(m_stat!=SERVICE_OFF){//make a connection to local port to release the blocked accept
		SOCKET sock;		
		if((sock=Create_Socket())==0) 
		{
			closesocket(sock);
			return;	
		}
		char local[32]="127.0.0.1";
		if(Client_Connect(sock,local,m_PortIn)==0)
		{
			closesocket(sock);					
			return;
		}
	}
	else
		GetDlgItem(IDC_STATIC_IPIN)->SetWindowText(_T(""));
}

BOOL CPTranDlg::PreTranslateMessage(MSG* pMsg)
{
	// ENTER key
	if(pMsg->message == WM_KEYDOWN){
		if(pMsg->wParam == VK_RETURN){
			OnBnClickedButtonStart();
			return TRUE;
		}
		if(pMsg->wParam == VK_ESCAPE){			
			return TRUE;
		}
	}
	return CDialog::PreTranslateMessage(pMsg);
}

void CPTranDlg::OnOK()
{
	// TODO: Add your specialized code here and/or call the base class

	if(MessageBox(_T("确认关闭服务并退出？"),_T("慎重！"),MB_YESNO|MB_ICONQUESTION|MB_DEFBUTTON2)==IDNO)
	{
		return;
	}
	OnBnClickedButtonStop();
	while(m_stat!=SERVICE_OFF)
		Sleep(100);
	delete m_servicePara;//free mem
	Shell_NotifyIcon(NIM_DELETE,&NotifyIcon);
	CDialogEx::OnOK();
}


void CPTranDlg::OnCancel()
{
	// TODO: Add your specialized code here and/or call the base class	
	if(MessageBox(_T("确认关闭服务并退出？"),_T("慎重！"),MB_YESNO|MB_ICONQUESTION|MB_DEFBUTTON2)==IDNO)
	{
		return;
	}
	OnBnClickedButtonStop();
	while(m_stat!=SERVICE_OFF)
		Sleep(100);
	delete m_servicePara;//free mem
	Shell_NotifyIcon(NIM_DELETE,&NotifyIcon);
	CDialogEx::OnCancel();
}


void CPTranDlg::OnBnClickedButtonHide()
{
	// TODO: Add your control notification handler code here
	ShowWindow(SW_HIDE);
}
LRESULT CPTranDlg::OnNotifyIcon(WPARAM wParam,LPARAM IParam){
	if ((IParam == WM_LBUTTONDOWN) || (IParam == WM_RBUTTONDOWN))
	{
		ModifyStyleEx(0,WS_EX_TOPMOST);
		ShowWindow(SW_SHOW);
	}
	return TRUE;
}