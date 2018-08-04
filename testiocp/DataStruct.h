#pragma once
#include <WinSock2.h>
#include "DoubleList.h"
//////////////////////////////////////////////////////////////////////////

#define    BUFFER_SIZE           4096
#define    MAXTHREAD_COUNT       8

#define    PORT                  5050//6369
#define    ADDR					INADDR_ANY //"127.0.0.1"

//////////////////////////////////////////////////////////////////////////

//
//��ɶ˿���ɵ��¼�
//
typedef enum
{
		IOCP_COMPLETE_READ,
		IOCP_COMPLETE_ACCEPT_READ,
		IOCP_COMPLETE_WRITE,
		IOCP_COMPLETE_ACCEPT
}IOCP_COMPLETE_STATE;

//
//�Զ���ö���������ͣ�������ʶ�׽���IO��������
//
typedef enum 
{
        IOCP_ACCEPT, //AcceptEx/accept
		IOCP_READ,   //WSARecv/recv/ReadFile
		IOCP_WRITE,   //WSASend/send/WriteFile
		IOCP_END
}IOCP_OPERATION, *IOCP_OPERATION_PTR;


//
//�Զ���ṹ��������ɼ���(���������)
//
typedef struct
{
	SOCKET                     socket;
}IOCP_KEY,*IOCP_KEY_PTR;

//
//��־SOCKET�ĵ�ǰ״̬
//
enum
{
	SOCKET_STATE_NOT_CONNECT,
	SOCKET_STATE_CONNECT,
	SOCKET_STATE_CONNECT_AND_READ,
	SOCKET_STATE_TIMEOUT
};


typedef enum
{
	SOCKET_FROM_UNKNOW,
	SOCKET_FROM_Concentrator,
	SOCKET_FROM_WEBSOCKET,
	SOCKET_FROM_APPLICATION
	
}IOCP_FROM_CLIEND;

typedef enum
{
	SOCKET_STATUS_UNKNOW,
	SOCKET_STATUS_LOGIN,
}IOCP_LOGIN_STATUS;
//
//��IO���ݣ���չ��WSAOVERLAPPED
//
typedef struct 
{
	WSAOVERLAPPED				ol;
	char						buf[BUFFER_SIZE];
	WSABUF						wsaBuf;
	SOCKET						socket;
	IOCP_OPERATION				operation;
	IOCP_FROM_CLIEND            fromtype;
	IOCP_LOGIN_STATUS           loginstatus;
	IOCP_KEY_PTR                lp_key;
	DWORD						logintime;
	DWORD						nexttime;
	volatile int				state;
}IOCP_IO,*IOCP_IO_PTR;

typedef CDoubleList<IOCP_IO,10>	IO_GROUP;
typedef CDoubleList<IOCP_KEY,0>	KEY_GROUP;

typedef  IO_GROUP::ITERATOR	IO_POS;
typedef  KEY_GROUP::ITERATOR KEY_POS;

