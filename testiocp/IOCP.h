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



class CIOCP
{
public:
    CIOCP();
    virtual ~CIOCP();
public:
    IO_GROUP            m_io_group;
    KEY_GROUP           m_key_group;
	char m_configTime[216];				//采集时间
    typedef   list<IOCP_IO_PTR>::iterator ITERATOR;
	map<string,_COMADDRVISITE>m_day;

//     typedef struct _WEBSOCKET
//     {
//         IOCP_IO_PTR lp_io;
//         IOCP_KEY_PTR lp_key;
//     }WEBSOCKET;


    HANDLE              m_h_iocp;
    SOCKET              m_listen_socket;

    list<IOCP_IO_PTR>      m_listmsg;        //消息列表
    map<string, IOCP_IO_PTR> m_mcontralcenter;  //集中器队列

	map<IOCP_IO_PTR,pBREAKPCK>m_pack;			//断包结构



    CRITICAL_SECTION    crtc_sec;
    LPFN_TRANSMITFILE   lpTransmitFile;
    LPFN_ACCEPTEX       lpAcceptEx;
    LPFN_GETACCEPTEXSOCKADDRS lpGetAcceptExSockaddrs;

    UINT                uPort;
    char                szAddress[20];
    CDBOperation        dbopen;
    int                 m_n_thread_count;
    HANDLE              m_h_thread[MAXTHREAD_COUNT];
    HANDLE              m_h_accept_event;
    gListCtr*           m_listctr;
    HWND                hWnd;
public:
    BOOL                Init();
    BOOL                MainLoop();
    BOOL                SendData(ULONG_PTR s, ULONG_PTR key);
    BOOL                SendWebsocket(ULONG_PTR s);

    int                 hex2str(string str, BYTE tosend[]);
    int                 wsHandshake(string &request, string &response);
    int                 wsDecodeFrame(char inFrame[], string &outMessage, int len,BOOL& fullpack);
    int                 wsEncodeFrame(string inMessage, char outFrame[], enum WS_FrameType frameType,int& lenret);
    void                dealws(IOCP_IO_PTR& lp_io, string& jsondata);
    string              GetDataDir(string filename);
    //集中器
    BOOL                checkFlag(BYTE vv[], int len);
    void                changeByte(char data[], BYTE vv[], int& len);
    void                buildcode(BYTE src[], int srclen, BYTE des[], int& deslen, BOOL& isrespos, IOCP_IO_PTR& lp_io);
    void                buildConCode(BYTE src[], BYTE res[], int& len, BYTE bcon);
	BOOL				CloseMySocket(IOCP_IO_PTR lp_io);
private:
    void                InitIoContext(IOCP_IO_PTR lp_io);
    void                Close();
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

	BOOL				IsBreakPack(BYTE src[],int len);
	BOOL				IsTailPack(BYTE src[],int len,pBREAKPCK pack,IOCP_IO_PTR& lp_io);
	BOOL				IsTailPackWeb(BYTE src[], int len, pBREAKPCK pack, IOCP_IO_PTR& lp_io);
	BOOL				AppendByte(BYTE src[],int len,pBREAKPCK pack,IOCP_IO_PTR& lp_io);
	 static DWORD WINAPI	TimeThread(LPVOID lp_param);
	 void		CheckForInvalidConnection();
};

//////////////////////////////////////////////////////////////////////////

#endif  //__IOCP_H_

