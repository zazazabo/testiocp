#pragma once
#include <WinSock2.h>
#include "DoubleList.h"
//////////////////////////////////////////////////////////////////////////

#define    BUFFER_SIZE           2048
#define    MAXTHREAD_COUNT       8

#define    PORT                  5050//6369
#define    ADDR					INADDR_ANY //"127.0.0.1"

//////////////////////////////////////////////////////////////////////////

//
//完成端口完成的事件
//
typedef enum
{
		IOCP_COMPLETE_READ,
		IOCP_COMPLETE_ACCEPT_READ,
		IOCP_COMPLETE_WRITE,
		IOCP_COMPLETE_ACCEPT
}IOCP_COMPLETE_STATE;

//
//自定义枚举数据类型，用来标识套接字IO动作类型
//
typedef enum 
{
        IOCP_ACCEPT, //AcceptEx/accept
		IOCP_READ,   //WSARecv/recv/ReadFile
		IOCP_WRITE,   //WSASend/send/WriteFile
		IOCP_END,
		IOCP_DEFAULT
}IOCP_OPERATION, *IOCP_OPERATION_PTR;


//
//自定义结构，即“完成键”(单句柄数据)
//
typedef struct
{
	SOCKET                     socket;
}IOCP_KEY,*IOCP_KEY_PTR;

//
//标志SOCKET的当前状态
//
enum
{
	SOCKET_STATE_NOT_CONNECT,
	SOCKET_STATE_CONNECT,
	SOCKET_STATE_CONNECT_AND_READ,
	SOCKET_STATE_TIMEOUT
};
enum WS_PACK
{
	WS_ERROR_PACK = 0x0,
	WS_BREAK_PACK = 0x1,
	WS_ALL_PACK = 0x2

};

typedef enum
{
	SOCKET_FROM_UNKNOW,					//末知
	SOCKET_FROM_GAYWAY,		   //1
	SOCKET_FROM_WEBSOCKET,            //2
	SOCKET_FROM_APPLICATION
	
}IOCP_FROM_CLIEND;

typedef enum
{
	SOCKET_STATUS_UNKNOW,
	SOCKET_STATUS_LOGIN,
}IOCP_LOGIN_STATUS;
//
//单IO数据，扩展的WSAOVERLAPPED
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
	char						day[20];
	char						gayway[20];
	PVOID						pUserData;
	int							timelen;
	volatile int				state;
	
}IOCP_IO,*IOCP_IO_PTR;


typedef struct _BREAK_PACK
{
	BYTE*   b;
	int		len;
}BREAK_PACK,*pBREAKPCK;


typedef struct _COMADDRVISITE
{
	char comaddr[20];
	BOOL bcollect;
}ComaddrViste,*pComaddrViste;


typedef CDoubleList<IOCP_IO,10>	IO_GROUP;
typedef CDoubleList<IOCP_KEY,0>	KEY_GROUP;

typedef  IO_GROUP::ITERATOR	IO_POS;
typedef  KEY_GROUP::ITERATOR KEY_POS;

