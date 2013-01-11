#include "stdafx.h"
#include "Transmit.h"

int Create_Socket()
{  
	int sockfd;
	sockfd=socket(AF_INET,SOCK_STREAM,0);
	if(sockfd<0)
	{
		//Create socket error
		return(0);
	}

	return(sockfd);	
}

int Create_Server(int sockfd, int port)
{
	struct sockaddr_in srvaddr;
	int on=1;

	memset(&srvaddr, 0, sizeof(struct sockaddr));

	srvaddr.sin_port=htons(port);
	srvaddr.sin_family=AF_INET;
	srvaddr.sin_addr.s_addr=htonl(INADDR_ANY);
	//SO_REUSEADDR  to rebind the port
	setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR|SO_LINGER, (char*)&on,sizeof(on)); 

	if(bind(sockfd,(struct sockaddr *)&srvaddr,sizeof(struct sockaddr))<0)
	{
		//Socket bind error
		return(0);
	}
	if(listen(sockfd, CONNECTNUM)<0)
	{
		//Socket Listen error
		return(0);
	}

	return(1);
}

int Client_Connect(int sockfd,char* server,int port)
{
	struct sockaddr_in cliaddr;
	struct hostent *host;

	if(!(host=gethostbyname(server)))
	{
		//Gethostbyname  error
		return(0);
	}  	

	memset(&cliaddr, 0, sizeof(struct sockaddr));
	cliaddr.sin_family=AF_INET;
	cliaddr.sin_port=htons(port);
	cliaddr.sin_addr=*((struct in_addr *)host->h_addr);

	if(connect(sockfd,(struct sockaddr *)&cliaddr,sizeof(struct sockaddr))<0)
	{
		//Connect error
		return(0);
	}
	return(1);
}

void CloseAll()
{
	int i;
	for(i=3; i<256; i++)
	{
		closesocket(i);	
	}

}

inline void swap(unsigned char &a,unsigned char &b){
	if(&a==&b) return;
	a^=b^=a^=b;
}

inline void RC4(int &i,int &j,int n,char *key,char *in,char *out,unsigned char *ss){
	for(int k=0;k<n;k++){
		i=i+1;
		while(i>=256)
			i-=256;
		j=j+ss[i];
		while(j>=256)
			j-=256;
		swap(ss[i],ss[j]);
		int t=ss[i]+ss[j];
		while(t>=256)
			t-=256;
		out[k]=ss[t]^in[k];
	}
}

unsigned __stdcall ServiceMain(LPVOID data)
{
	servicePara *para=(struct servicePara *)data;
	int portIn=para->portIn;
	int portOut=para->portOut;
	char *hostOut=para->hostOut;
	char *key=para->key;
	char *cmd=para->cmd;
	CPTranDlg * st=para->dlgUi;

	WSADATA wsd;
	if( WSAStartup( MAKEWORD( 1, 1), &wsd) != 0)
	{		
		st->GetDlgItem(IDC_STATIC_ST)->SetWindowText(_T("WSAStartup 失败！"));
		return 0;
	}

	SOCKET sockfd,sockfd1,sockfd2;
	struct sockaddr_in remote;
	int size;
	int maxfdp=NULL;  
	fd_set fds;  
	struct timeval timeOut;
	timeOut.tv_sec=ACCEPT_TIMEOUT;
	timeOut.tv_usec=0;

	if (portIn > 65535 || portIn < 1)
	{		
		st->GetDlgItem(IDC_STATIC_ST)->SetWindowText(_T("源端口不合法！"));
		st->m_stat=SERVICE_OFF;
		st->OnBnClickedButtonStop();
		return 0;
	}

	if (portOut > 65535 || portOut < 1)
	{	
		st->GetDlgItem(IDC_STATIC_ST)->SetWindowText(_T("目的端口不合法！"));
		st->m_stat=SERVICE_OFF;
		st->OnBnClickedButtonStop();
		return 0;
	}


	if((sockfd=Create_Socket()) == INVALID_SOCKET){
		st->m_stat=SERVICE_OFF;
		st->OnBnClickedButtonStop();
		return 0;
	}

	if(Create_Server(sockfd, portIn) == 0) 
	{
		st->GetDlgItem(IDC_STATIC_ST)->SetWindowText(_T("监听端口失败！"));
		closesocket(sockfd);
		st->m_stat=SERVICE_OFF;
		st->OnBnClickedButtonStop();
		return 0;
	}

	size=sizeof(struct sockaddr);
	maxfdp=sockfd+1;
	while(cmd[0]=='o'){
		if(st->m_ConCount==0){
			st->m_stat=SERVICE_WAIT;
			st->GetDlgItem(IDC_STATIC_ST)->SetWindowText(_T("等待连接，服务运行中..."));
		}
		FD_ZERO(&fds);  
		FD_SET(sockfd,&fds);  
		if (select(maxfdp,&fds,NULL,NULL,&timeOut)){  
			if((sockfd1=accept(sockfd,(struct sockaddr *)&remote,&size))<0)
			{			
				st->GetDlgItem(IDC_STATIC_ST)->SetWindowText(_T("接收连接失败!"));
				continue;
			}
			if(cmd[0]!='o')//stop
				break;
			char *ip=inet_ntoa(remote.sin_addr);
			_bstr_t bstrTmp(ip);
			st->GetDlgItem(IDC_STATIC_IPIN)->SetWindowText((LPCTSTR)(LPTSTR)bstrTmp);
			if((sockfd2=Create_Socket())==0) 
			{
				closesocket(sockfd1);
				st->m_stat=SERVICE_OFF;
				st->OnBnClickedButtonStop();
				return 0;	
			}
			if(Client_Connect(sockfd2,hostOut,portOut)==0)
			{
				closesocket(sockfd2);
				st->GetDlgItem(IDC_STATIC_ST)->SetWindowText(_T("连接到目的端口失败！"));
				closesocket(sockfd1);
				st->m_stat=SERVICE_OFF;
				st->OnBnClickedButtonStop();
				return 0;
			}
			st->m_stat=SERVICE_ON;
			st->GetDlgItem(IDC_STATIC_ST)->SetWindowText(_T("连接成功，服务运行中..."));
			st->m_ConCountStr.Format(_T("%d"),++st->m_ConCount);
			st->GetDlgItem(IDC_STATIC_CNT)->SetWindowText(st->m_ConCountStr);
			HANDLE hThread=NULL;
			unsigned dwThreadID;
			transPara *para=new transPara();
			para->fd1=sockfd1;
			para->fd2=sockfd2;
			para->key=key;
			para->cmd=cmd;
			para->st=st;
			//Start a Thread
			hThread=(HANDLE)_beginthreadex(NULL, 0, &TransmitData, (LPVOID)para, 0, &dwThreadID);
			CloseHandle(hThread);			
		}
	}
	shutdown(sockfd,0);
	closesocket(sockfd);
	st->m_stat=SERVICE_OFF;
	st->GetDlgItem(IDC_STATIC_ST)->SetWindowText(_T("服务已终止")); 
	st->m_ConCount=0;
	st->GetDlgItem(IDC_STATIC_IPIN)->SetWindowText(_T(""));
	st->m_ConCountStr.Format(_T("%d"),st->m_ConCount);
	st->GetDlgItem(IDC_STATIC_CNT)->SetWindowText(st->m_ConCountStr);

	WSACleanup();	

	return 0;
}

unsigned __stdcall TransmitData(LPVOID data)
{
	transPara *para=(transPara *)data;
	SOCKET fd1=para->fd1;
	SOCKET fd2=para->fd2;
	char *key=para->key;
	char *cmd=para->cmd;
	CPTranDlg *st=para->st;
	struct timeval timeset;
	fd_set readfd;
	int result,i=0;
	char read_in1[BUFFER_SIZE],send_out1[BUFFER_SIZE];
	char read_in2[BUFFER_SIZE],send_out2[BUFFER_SIZE];
	int read1=0,totalread1=0,send1=0;
	int read2=0,totalread2=0,send2=0;
	int sendcount1,sendcount2;
	int maxfd;
	struct sockaddr_in client1,client2;
	int structsize1,structsize2;
	char host1[20],host2[20];
	int portIn=0,portOut=0;

	//RC4 Init
	unsigned char *ss1=NULL,*ss2=NULL;
	int l=strlen(key),streamIndex1=0,streamIndexj1=0,streamIndex2=0,streamIndexj2=0;
	if(l>0){
		ss1=new unsigned char[256];
		ss2=new unsigned char[256];		
		for(int i=0;i<256;i++)	// initialize S 
			ss1[i]=i;
		for(int j,i=0;i<l;i++){	// permutation
			j=i+ss1[i]+key[i];
			while(j>=256)
				j-=256;
			swap(ss1[i],ss1[j]);
		}
		for(int i=0;i<256;++i) //Initialize ss2
			ss2[i]=ss1[i];
	}
	//end
	memset(host1,0,20);
	memset(host2,0,20);
	//memset(tmpbuf,0,100);

	structsize1=sizeof(struct sockaddr);
	structsize2=sizeof(struct sockaddr);

	if(getpeername(fd1,(struct sockaddr *)&client1,&structsize1)<0)
	{
		strcpy_s(host1, "fd1");
	}
	else
	{	
		strcpy_s(host1, inet_ntoa(client1.sin_addr));
		portIn=ntohs(client1.sin_port);
	}

	if(getpeername(fd2,(struct sockaddr *)&client2,&structsize2)<0)
	{
		strcpy_s(host2,"fd2");
	}
	else
	{	
		strcpy_s(host2, inet_ntoa(client2.sin_addr));
		portOut=ntohs(client2.sin_port);
	}
	maxfd=max(fd1,fd2)+1;
	memset(read_in1,0,BUFFER_SIZE);
	memset(read_in2,0,BUFFER_SIZE);
	memset(send_out1,0,BUFFER_SIZE);
	memset(send_out2,0,BUFFER_SIZE);

	timeset.tv_sec=TRANSMIT_TIMEOUT;
	timeset.tv_usec=0;

	while(cmd[0]=='o')
	{
		FD_ZERO(&readfd);

		FD_SET((UINT)fd1, &readfd);
		FD_SET((UINT)fd2, &readfd);

		result=select(maxfd,&readfd,NULL,NULL, &timeset);
		if((result<0) && (errno!=EINTR))
		{
			st->GetDlgItem(IDC_STATIC_ST)->SetWindowText(_T("Select Error!"));
			break;
		}
		//Recv Data from fd1
		if(FD_ISSET(fd1, &readfd))
		{
			if(totalread1<BUFFER_SIZE)
			{
				read1=recv(fd1, read_in1, BUFFER_SIZE-totalread1, 0); 
				if(read1==0) break;
				if((read1==SOCKET_ERROR))
				{
					//st->GetDlgItem(IDC_STATIC_ST)->SetWindowText("数据接收错误！");
					break;
				}
				//Crypto
				if(ss1!=NULL)
					RC4(streamIndex1,streamIndexj1,read1,key,read_in1,read_in1,ss1);				

				memcpy(send_out1+totalread1,read_in1,read1);
				totalread1+=read1;
				memset(read_in1,0,BUFFER_SIZE);
			}
		}
		//Send Data to fd2
		int err=0;
		sendcount1=0;
		while(totalread1>0)
		{
			send1=send(fd2, send_out1+sendcount1, totalread1, 0);
			if(send1==0)break;
			if((send1<0) && (errno!=EINTR))
			{
				//st->GetDlgItem(IDC_STATIC_ST)->SetWindowText("发送错误！");
				err=1;
				break;
			}

			if((send1<0) && (errno==ENOSPC)) break;
			sendcount1+=send1;
			totalread1-=send1; 
		}

		if(err==1) break;
		if((totalread1>0) && (sendcount1>0))
		{
			//move data
			memcpy(send_out1,send_out1+sendcount1,totalread1);
			memset(send_out1+totalread1,0,BUFFER_SIZE-totalread1);
		}
		else
			memset(send_out1,0,BUFFER_SIZE);

		//Recv Data from fd2
		if(FD_ISSET(fd2, &readfd))
		{
			if(totalread2<BUFFER_SIZE)
			{
				read2=recv(fd2,read_in2,BUFFER_SIZE-totalread2, 0); 
				if(read2==0)break;
				if((read2<0) && (errno!=EINTR))
				{
					//st->GetDlgItem(IDC_STATIC_ST)->SetWindowText("接收错误！");
					break;
				}
				//Crypto
				if(ss2!=NULL)
					RC4(streamIndex2,streamIndexj2,read2,key,read_in2,read_in2,ss2);

				memcpy(send_out2+totalread2,read_in2,read2);
				totalread2+=read2;
				memset(read_in2,0,BUFFER_SIZE);
			}
		}

		//Send Data to fd1
		int err2=0;
		sendcount2=0;
		while(totalread2>0)
		{
			send2=send(fd1, send_out2+sendcount2, totalread2, 0);
			if(send2==0)break;
			if((send2<0) && (errno!=EINTR))
			{
				//st->GetDlgItem(IDC_STATIC_ST)->SetWindowText("发送错误！");				
				err2=1;
				break;
			}
			if((send2<0) && (errno==ENOSPC)) break;
			sendcount2+=send2;
			totalread2-=send2; 
		}
		if(err2==1) break;
		if((totalread2>0) && (sendcount2 > 0))
		{
			//move data
			memcpy(send_out2, send_out2+sendcount2, totalread2);
			memset(send_out2+totalread2, 0, BUFFER_SIZE-totalread2);
		}
		else
			memset(send_out2,0,BUFFER_SIZE);

	} 
	//free mem
	if(ss1!=NULL){
		delete []ss1;
		delete []ss2;
	}
	shutdown(fd1, 0);
	shutdown(fd2, 0);
	closesocket(fd1);
	closesocket(fd2);
	if(st->m_stat!=SERVICE_OFF){
		st->m_ConCountStr.Format(_T("%d"),--st->m_ConCount);
		st->GetDlgItem(IDC_STATIC_CNT)->SetWindowText(st->m_ConCountStr);
		if(st->m_ConCount==0){
			st->m_stat=SERVICE_WAIT;
			st->GetDlgItem(IDC_STATIC_ST)->SetWindowText(_T("等待连接，服务运行中..."));
		}
	}	
	delete data;//free mem
	_endthreadex(0);
	return 0;
}
