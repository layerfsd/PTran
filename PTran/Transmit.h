#ifndef _TRANSMIT_H
#define _TRANSMIT_H

#include "PTranDlg.h"
#include "Resource.h"


#define CONNECTNUM				3
#define ACCEPT_TIMEOUT			10
#define TRANSMIT_TIMEOUT			1
#define BUFFER_SIZE				8192

int Create_Socket();
int Create_Server(int sockfd, int port);
int Client_Connect(int sockfd, char* server, int port);
void CloseAll();

unsigned __stdcall ServiceMain(LPVOID data);
unsigned __stdcall TransmitData(LPVOID data);

struct servicePara{
	int portIn;
	char *hostOut;
	int portOut;
	char *key;
	char *cmd;
	CPTranDlg *dlgUi;
};

struct transPara{
	SOCKET fd1;
	SOCKET fd2;
	char *key;
	char *cmd;
	CPTranDlg *st;
};

#endif 