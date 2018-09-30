#ifndef __IOCP_H_
#define __IOCP_H_

//////////////////////////////////////////////////////////////////////////

#include "resource.h"
#include "smtp.h"
#include <winsock2.h>
#include <mswsock.h>
#include <windows.h>
#include <iostream>
#include <stdint.h>
#include "DataStruct.h"
#include <json.h>
#include <queue>
#include <list>
#include <map>
#include "DBOperation.h"
#include "MainFrame.h"


#include "E:\\code\\glib\\h\\glistctr.h"
#include "E:\\code\\glib\\h\\gstring.h"
#include "E:\\code\\glib\\h\\MemoryModule.h"
#include "E:\\code\\glib\\h\\glog.h"
#include "E:\\code\\glib\\h\\gprocess.h"

#ifdef _DEBUG
#ifdef _WIN64
#else
#pragma comment(lib,"E:\\code\\glib\\lib\\glib_d.lib")
#pragma comment(lib,"lib_json.lib")
#endif
#else
#ifdef _WIN64
#pragma comment(lib,"E:\\code\\glib\\x64\\Release\\glib.lib")
#pragma comment(lib,"E:\\code\\Detours Pro v3.0.316\\lib.X64\\detours.lib")
#pragma comment(lib,"E:\\code\\zlib\\x64\\Release\\zlib.lib")

#else
#pragma comment(lib,"E:\\code\\glib\\Release\\glib.lib")
#pragma comment(lib,"E:\\code\\Detours Pro v3.0.316\\lib.X86\\detours.lib")
#pragma comment(lib,"E:\\code\\zlib\\Release\\zlib.lib")
#endif
#endif



//////////////////////////////////////////////////////////////////////////
#pragma comment(lib,"ws2_32.lib")

static  GUID g_GUIDAcceptEx     = WSAID_ACCEPTEX;
static  GUID g_GUIDTransmitFile = WSAID_TRANSMITFILE;
static  GUID g_GUIDGetAcceptExSockaddrs = WSAID_GETACCEPTEXSOCKADDRS;

//////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////

enum WS_Status
{
    WS_STATUS_CONNECT = 0,
    WS_STATUS_UNCONNECT = 1,
};

enum WS_FrameType
{
    WS_EMPTY_FRAME = 0xF0,
    WS_ERROR_FRAME = 0xF1,
    WS_TEXT_FRAME   = 0x01,
    WS_BINARY_FRAME = 0x02,
    WS_PING_FRAME = 0x09,
    WS_PONG_FRAME = 0x0A,
    WS_OPENING_FRAME = 0xF3,
    WS_CLOSING_FRAME = 0x08
};



class CIOCP:public CMainFrame
{



public:
    CIOCP();
	CIOCP(string skin);
    virtual ~CIOCP();
public:
	IO_GROUP            m_io_group;
	KEY_GROUP           m_key_group;


	 void    Notify(TNotifyUI& msg);



    typedef   list<IOCP_IO_PTR>::iterator ITERATOR;
	map<string,_COMADDRVISITE>m_day;

	typedef struct MSGPACK
	{
		IOCP_IO_PTR lp_io;
		BYTE seq;
		char comaddr[20];
	}_MSGPACK;

	map<string,vector<MSGPACK>>m_MsgPack;

	HANDLE              m_h_iocp;
	SOCKET              m_listen_socket;

    list<MSGPACK>      m_listmsg;        //消息列表
    map<string, IOCP_IO_PTR> m_mcontralcenter;  //集中器队列

	map<IOCP_IO_PTR,pBREAKPCK>m_pack;			//断包结构
//
//
//
    CRITICAL_SECTION    crtc_sec;
	LPFN_TRANSMITFILE   lpTransmitFile;
	LPFN_ACCEPTEX       lpAcceptEx;
	LPFN_GETACCEPTEXSOCKADDRS lpGetAcceptExSockaddrs;

	UINT                uPort;
	char                szAddress[20];
	//CDBOperation        dbopen;
	int                 m_n_thread_count;
	HANDLE              m_h_thread[MAXTHREAD_COUNT];
	HANDLE              m_h_accept_event;
	void				Init();
	gListCtr*           m_listctr;
	HWND                hWnd;
public:
	BOOL                InitAll();
    BOOL                MainLoop();
    BOOL                SendData(ULONG_PTR s, ULONG_PTR key,string vvv);
    BOOL                SendWebsocket(ULONG_PTR s);

    int                 hex2str(string str, BYTE tosend[]);
    int                 wsHandshake(string &request, string &response);
    int                 wsDecodeFrame(char inFrame[], string &outMessage, int len,BOOL& fullpack);
    int                 wsEncodeFrame(string inMessage, char outFrame[], enum WS_FrameType frameType,int& lenret);
    void                dealws(IOCP_IO_PTR& lp_io, string& jsondata);
    //集中器
    BOOL                checkFlag(BYTE vv[], int len);
    void                buildcode(BYTE src[], int srclen, BYTE des[], int& deslen, BOOL& isrespos, IOCP_IO_PTR& lp_io);
    void                buildConCode(BYTE src[], BYTE res[], int& len, BYTE bcon);
	BOOL				CloseMySocket(IOCP_IO_PTR lp_io);
private:
	void                InitIoContext(IOCP_IO_PTR lp_io);
    void                Close1();
	BOOL                RegAcceptEvent();

	BOOL                DataAction(IOCP_IO_PTR lp_io, IOCP_KEY_PTR lp_key);
	BOOL                HandleData(IOCP_IO_PTR lp_io, int nFlags, IOCP_KEY_PTR lp_key);
	BOOL                GetFunPointer();
	BOOL                StartThread();
	BOOL                BindAndListenSocket();
	void                CloseThreadHandle(int count);
	BOOL                InitSocket();
	static DWORD WINAPI CompletionRoutine(LPVOID lp_param);
	BOOL                PostAcceptEx();
	BOOL                GetAddrAndPort(char*buf, char ip[], UINT &port);
	void				DealWebsockMsg(IOCP_IO_PTR& lp_io,IOCP_KEY_PTR& lp_key,BYTE msg[],int len,BOOL bAnswer);
	BOOL				IsBreakPack(BYTE src[],int len);

	BOOL				AppendByte(BYTE src[],int len,pBREAKPCK pack,IOCP_IO_PTR& lp_io);
	static DWORD WINAPI	TimeThread(LPVOID lp_param);
	int  buidByte(string comaddr,BYTE C, BYTE AFN,BYTE SEQ,SHORT DA,SHORT DT,vector<BYTE>&v_b,BYTE des[]);
 void		CheckForInvalidConnection();
};

//////////////////////////////////////////////////////////////////////////

#endif  //__IOCP_H_

