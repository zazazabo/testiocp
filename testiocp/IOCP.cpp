
#include "IOCP.h"
#include "SHA1.h"
#pragma comment(lib, "ws2_32.lib")

/////////////////////////////////////////////////////////////////////////////////////////////

CIOCP::CIOCP()
{
    m_mcontralcenter.clear();
    ::InitializeCriticalSection(&crtc_sec);
}
//构造函数

CIOCP::~CIOCP()
{
    Close();
}
//析构函数
/*-------------------------------------------------------------------------------------------
函数功能：关闭并清除资源
函数说明：
函数返回：
-------------------------------------------------------------------------------------------*/
void CIOCP::Close()
{
    int                 i;
    IO_POS              pos;
    IOCP_IO_PTR lp_io;
    CloseHandle(m_h_iocp);
    m_io_group.GetHeadPosition(pos);

    //  while( pos != NULL )
    //  {
    //      lp_io = m_io_group.GetNext( pos );
    //
    //      closesocket( lp_io->socket );
    //  }

    for(i = 0; i < m_n_thread_count; i++)
    {
        CloseHandle(m_h_thread[i]);
        m_h_thread[i] = NULL;
    }
}

/*-------------------------------------------------------------------------------------------
函数功能：初始化IO结点
函数说明：
函数返回：
-------------------------------------------------------------------------------------------*/
void CIOCP::InitIoContext(IOCP_IO_PTR lp_io)
{
    memset(&lp_io->ol,  0, sizeof(WSAOVERLAPPED));
    memset(&lp_io->buf, 0, BUFFER_SIZE);
    lp_io->wsaBuf.buf       = lp_io->buf;
    lp_io->wsaBuf.len       = BUFFER_SIZE;
}

/*-------------------------------------------------------------------------------------------
函数功能：初始化侦听SOCKET端口，并和完成端口连接起来。
函数说明：
函数返回：成功，TRUE；失败，FALSE
-------------------------------------------------------------------------------------------*/
BOOL CIOCP::InitSocket()
{
    m_listen_socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);

    if(INVALID_SOCKET == m_listen_socket)
    {
        return FALSE;
    }

    IOCP_KEY_PTR  lp_key = m_key_group.GetBlank();
    lp_key->socket = m_listen_socket;
    HANDLE hRet = CreateIoCompletionPort((HANDLE)m_listen_socket, m_h_iocp, (DWORD)lp_key, 0);

    if(hRet == NULL)
    {
        closesocket(m_listen_socket);
        m_key_group.RemoveAt(lp_key);
        return FALSE;
    }

    return TRUE;
}

/*-------------------------------------------------------------------------------------------
函数功能：关闭所有线程
函数说明：
函数返回：
-------------------------------------------------------------------------------------------*/
void CIOCP::CloseThreadHandle(int count)
{
    if(count <= 0)
    {
        return;
    }

    for(int i = 0; i < count; i++)
    {
        CloseHandle(m_h_thread[i]);
        m_h_thread[i] = INVALID_HANDLE_VALUE;
    }
}

/*-------------------------------------------------------------------------------------------
函数功能：将侦听端口和自己的IP，PORT绑定，并开始侦听
函数说明：
函数返回：成功，TRUE；失败，FALSE
-------------------------------------------------------------------------------------------*/
BOOL CIOCP::BindAndListenSocket()
{
    SOCKADDR_IN addr;
    memset(&addr, 0, sizeof(SOCKADDR_IN));
    addr.sin_family         = AF_INET;
    addr.sin_addr.s_addr    = htons(ADDR);//inet_addr(ADDR);
    addr.sin_port           = htons(PORT);
    int nRet;
    nRet = bind(m_listen_socket, (SOCKADDR*)&addr, sizeof(SOCKADDR));

    if(SOCKET_ERROR == nRet)
    {
        cout << "bind fail!" << endl;
        return FALSE;
    }

    nRet = listen(m_listen_socket, 20);

    if(SOCKET_ERROR == nRet)
    {
        cout << "listen fail!" << endl;
        return FALSE;
    }

    return TRUE;
}


/*-------------------------------------------------------------------------------------------
函数功能：根据CPU的数目，启动相应数量的数据处理线程
函数说明：
函数返回：成功，TRUE；失败，FALSE
-------------------------------------------------------------------------------------------*/
BOOL CIOCP::StartThread()
{
    int i;
    SYSTEM_INFO sys_info;
    GetSystemInfo(&sys_info);
    m_n_thread_count = sys_info.dwNumberOfProcessors > MAXTHREAD_COUNT ? MAXTHREAD_COUNT : sys_info.dwNumberOfProcessors;

    for(i = 0; i < m_n_thread_count; i++)
    {
        m_h_thread[i] = CreateThread(NULL, 0, CompletionRoutine, (LPVOID)this, 0, NULL);

        if(NULL == m_h_thread[i])
        {
            CloseThreadHandle(i);
            CloseHandle(m_h_iocp);
            return FALSE;
        }

        MSG("start a thread");
    }

    return TRUE;
}


/*-------------------------------------------------------------------------------------------
函数功能：发出一定数量的连接
函数说明：
函数返回：成功，TRUE；失败，FALSE
-------------------------------------------------------------------------------------------*/
BOOL CIOCP::PostAcceptEx()
{
    int     count = 10;
    DWORD   dwBytes;
    BOOL    bRet;

    for(int i = 0; i < count; i++)
    {
        SOCKET socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);

        if(INVALID_SOCKET == socket)
        {
            glog::traceErrorInfo("WSASocket", WSAGetLastError());
            continue;
        }

        IOCP_IO_PTR lp_io = m_io_group.GetBlank();
        InitIoContext(lp_io);
        lp_io->socket           = socket;
        lp_io->operation        = IOCP_ACCEPT;
        lp_io->state            = SOCKET_STATE_NOT_CONNECT;
        lp_io->fromtype = SOCKET_FROM_UNKNOW;
        lp_io->loginstatus = SOCKET_STATUS_UNKNOW;
        lp_io->lp_key = NULL;
        lp_io->nexttime         = 0;
        lp_io->logintime        = 0;
        //glog::GetInstance()->AddLine("post accecptex socket:%d IOCP_IO_PTR:%p", socket, lp_io);
        /////////////////////////////////////////////////
        bRet = lpAcceptEx(m_listen_socket, lp_io->socket, lp_io->buf,
                          //lp_io->wsaBuf.len - 2 * (sizeof(SOCKADDR_IN) + 16),
                          0,
                          sizeof(SOCKADDR_IN) + 16,
                          sizeof(SOCKADDR_IN) + 16,
                          &dwBytes, &lp_io->ol);

        if((bRet == FALSE) && (WSA_IO_PENDING != WSAGetLastError()))
        {
            closesocket(socket);
            m_io_group.RemoveAt(lp_io);
            // cout << "post acceptex fail:" << WSAGetLastError() << endl;
            glog::traceErrorInfo("acceptex", WSAGetLastError());
            continue;
        }
    }

    return TRUE;
}

/*-------------------------------------------------------------------------------------------
函数功能：处理数据函数
函数说明：
函数返回：成功，TRUE；失败，FALSE
-------------------------------------------------------------------------------------------*/
BOOL CIOCP::HandleData(IOCP_IO_PTR lp_io, int nFlags, IOCP_KEY_PTR lp_key)
{
    switch(nFlags)
    {
        case IOCP_COMPLETE_ACCEPT:
            {
                cout << "Accept a link!*****************************" << endl;
                char szPeerAddress[50];
                SOCKADDR_IN *addrClient = NULL, *addrLocal = NULL;
                char ip[50] = {0};
                int nClientLen = sizeof(SOCKADDR_IN), nLocalLen = sizeof(SOCKADDR_IN);
                lpGetAcceptExSockaddrs(lp_io->buf, 0, sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, (LPSOCKADDR*)&addrLocal, &nLocalLen, (LPSOCKADDR*)&addrClient, &nClientLen);
                char* ip1 = inet_ntoa(addrClient->sin_addr);
                sprintf(szPeerAddress, "%s:%d", ip1, addrClient->sin_port);
                //InitIoContext(lp_io);
                //SOCKADDR_IN addr_conn = {0};
                //int nSize = sizeof(addr_conn);
                //int nret =  getsockname(lp_key->socket, (SOCKADDR *)&addr_conn, &nSize);
                m_listctr->insertItemText(szPeerAddress, 0, m_listctr->getRowCount());
                char socketstr[50] = {0};
                sprintf(socketstr, "%p", lp_io);
                m_listctr->setItemText(socketstr, m_listctr->getRowCount() - 1, 1);
                char socketkey[50] = {0};
                sprintf(socketkey, "%p", lp_key);
                m_listctr->setItemText(socketkey, m_listctr->getRowCount() - 1, 3);
                glog::trace("szPeerAddress:%s", szPeerAddress);
                lp_io->operation    = IOCP_READ;
            }
            break;

        case IOCP_COMPLETE_ACCEPT_READ:
            {
                cout << "read a data!*******************************data length is:" << endl;
                lp_io->operation    = IOCP_WRITE;
                GetAddrAndPort(lp_io->wsaBuf.buf, szAddress, uPort);
                MSG(lp_io->wsaBuf.len);
                memset(&lp_io->ol, 0, sizeof(lp_io->ol));
            }
            break;

        case IOCP_COMPLETE_READ:
            {
                //cout<<"read a data!"<<lp_io->buf<<endl;
                printf("read a data! socket:%d \n", lp_io->socket);
                // lp_io->operation    = IOCP_WRITE;
                memset(&lp_io->ol, 0, sizeof(lp_io->ol));
            }
            break;

        case IOCP_COMPLETE_WRITE:
            {
                cout << "write a data!******************************" << endl;
                InitIoContext(lp_io);
                //lp_io->operation  = IOCP_END;
                lp_io->operation    = IOCP_READ;
            }
            break;

        default:
            {
                cout << "handleData do nothing!*********************" << endl;
                return FALSE;
            }
    }

    return TRUE;
}


/*-------------------------------------------------------------------------------------------
函数功能：发出一些重叠动作
函数说明：
函数返回：成功，TRUE；失败，FALSE
-------------------------------------------------------------------------------------------*/
BOOL CIOCP::DataAction(IOCP_IO_PTR lp_io, IOCP_KEY_PTR lp_key)
{
    DWORD   dwBytes;
    int     nRet;
    DWORD   dwFlags;

    switch(lp_io->operation)
    {
        case IOCP_WRITE:
            {
                cout << "post a write data!---------------------------------------" << endl;
                nRet = WSASend(lp_io->socket, &lp_io->wsaBuf, 1, &dwBytes, 0, &lp_io->ol, NULL);

                if((nRet == SOCKET_ERROR) && (WSAGetLastError() != WSA_IO_PENDING))
                {
                    cout << "WSASend fail!----------------------------------------" << WSAGetLastError() << endl;
                    closesocket(lp_io->socket);
                    m_io_group.RemoveAt(lp_io);
                    m_key_group.RemoveAt(lp_key);
                    return FALSE;
                }
            }
            break;

        case IOCP_READ:
            {
                cout << "post a read data!-----------------------------------------" << endl;
                dwFlags = 0;
                nRet = WSARecv(lp_io->socket, &lp_io->wsaBuf, 1, &dwBytes, &dwFlags, &lp_io->ol, NULL);

                if((nRet == SOCKET_ERROR) && (WSAGetLastError() != WSA_IO_PENDING))
                {
                    cout << "WSARecv fail!-------------------------------------------" << WSAGetLastError() << endl;
                    closesocket(lp_io->socket);
                    m_io_group.RemoveAt(lp_io);
                    m_key_group.RemoveAt(lp_key);
                    return FALSE;
                }
            }
            break;

        case IOCP_END:
            {
                cout << "close a socket link!-------------------------------------------" << endl;
                closesocket(lp_io->socket);
                m_io_group.RemoveAt(lp_io);
                m_key_group.RemoveAt(lp_key);
            }
            break;

        default:
            {
                cout << "DataAction do nothing!------------------------------------------" << endl;
                return FALSE;
            }
    }

    return TRUE;
}

/*-------------------------------------------------------------------------------------------
函数功能：得到MS封装的SOCKET函数指针，这样可以提高速度
函数说明：
函数返回：成功，TRUE；失败，FALSE
-------------------------------------------------------------------------------------------*/
BOOL CIOCP::GetFunPointer()
{
    DWORD dwRet, nRet;
    nRet = WSAIoctl(
               m_listen_socket,
               SIO_GET_EXTENSION_FUNCTION_POINTER,
               &g_GUIDAcceptEx,
               sizeof(g_GUIDAcceptEx),
               &lpAcceptEx,
               sizeof(lpAcceptEx),
               &dwRet, NULL, NULL);

    if(SOCKET_ERROR == nRet)
    {
        closesocket(m_listen_socket);
        cout << "get acceptex fail!" << WSAGetLastError() << endl;
        return FALSE;
    }

    nRet = WSAIoctl(
               m_listen_socket,
               SIO_GET_EXTENSION_FUNCTION_POINTER,
               &g_GUIDTransmitFile,
               sizeof(g_GUIDTransmitFile),
               &lpTransmitFile,
               sizeof(lpTransmitFile),
               &dwRet, NULL, NULL);

    if(nRet == SOCKET_ERROR)
    {
        closesocket(m_listen_socket);
        cout << "get transmitfile fail!" << WSAGetLastError() << endl;
        return FALSE;
    }

    nRet = WSAIoctl(
               m_listen_socket,
               SIO_GET_EXTENSION_FUNCTION_POINTER,
               &g_GUIDGetAcceptExSockaddrs,
               sizeof(g_GUIDGetAcceptExSockaddrs),
               &lpGetAcceptExSockaddrs,
               sizeof(lpGetAcceptExSockaddrs),
               &dwRet, NULL, NULL);

    if(nRet == SOCKET_ERROR)
    {
        closesocket(m_listen_socket);
        cout << "get lpGetAcceptExSockaddrs fail!" << WSAGetLastError() << endl;
        return FALSE;
    }

    return TRUE;
}

/*-------------------------------------------------------------------------------------------
函数功能：看看是否有连接了，但很长时间没有数据的“无效连接”，有的话，就踢掉
函数说明：
函数返回：成功，TRUE；失败，FALSE
-------------------------------------------------------------------------------------------*/
void CIOCP::CheckForInvalidConnection()
{
    int         op, op_len, nRet;
    IOCP_IO_PTR lp_io = NULL;
    IO_POS      pos;
    // EnterCriticalSection(&crtc_sec);
    vector<IOCP_IO_PTR>v_socket;
    v_socket.clear();
    lp_io =  m_io_group.GetHeadPosition(pos);

    while(lp_io != NULL)
    {
        // glog::GetInstance()->AddLine("lp_io:%p", lp_io);
        if(lp_io->state == SOCKET_STATE_CONNECT_AND_READ)
        {
            if(lp_io->fromtype == SOCKET_FROM_Concentrator)
            {
                DWORD d = GetTickCount() - lp_io->nexttime;
                DWORD min = d / 60000;

                if(min > 1)
                {
                    glog::GetInstance()->AddLine("lp_io:%p 隔%d分钟末接收到消息 客户端类型:%d\n", lp_io, min, lp_io->fromtype);
                    v_socket.push_back(lp_io);
                }

//                 op_len = sizeof(op);
//                 nRet = getsockopt(lp_io->socket, SOL_SOCKET, SO_CONNECT_TIME, (char*)&op, &op_len);
//
//                 if(SOCKET_ERROR == nRet)
//                 {
//                     glog::trace("SO_CONNECT_TIME failed:");
//                     glog::traceErrorInfo("getsockopt", WSAGetLastError());
//                 }
//
//                 if(op != 0xffffffff && op > 80)
//                 {
//                     v_socket.push_back(lp_io);
//                 }
            }
        }

        lp_io = m_io_group.GetNext(pos);
        //  LeaveCriticalSection(&crtc_sec);
    }

    for(int i = 0; i < v_socket.size(); i++)
    {
        IOCP_IO_PTR lp_io1 = v_socket[i];

        if(lp_io1->fromtype == SOCKET_FROM_Concentrator)
        {
            map<string, IOCP_IO_PTR>::iterator  it;

            for(it = m_mcontralcenter.begin(); it != m_mcontralcenter.end();)
            {
                if(it->second == lp_io1)
                {
                    m_mcontralcenter.erase(it++);
                }
            }
        }

        //glog::GetInstance()->AddLine("移除集中器map 集完成");
        int n1 = m_listctr->getRowCount();

        //删除界面条项
        for(int i = 0; i < n1; i++)
        {
            string vv = m_listctr->getCellText(i, 1);
            char pp[50] = {0};
            sprintf(pp, "%p", lp_io1);

            if(_stricmp(pp, vv.c_str()) == 0)
            {
                m_listctr->deleteIndex(i);
                break;
            }
        }

        closesocket(lp_io1->socket);
    }

//     IO_POS posEnd;
//     m_io_group.GetEndPosition(posEnd);
    //while(pos != posEnd)
    //{
    //    IOCP_IO_PTR  lp_io = *pos;
    //    if(lp_io->state == SOCKET_STATE_CONNECT_AND_READ)
    //    {
    //        if(lp_io->fromtype == SOCKET_FROM_Concentrator)
    //        {
    //            op_len = sizeof(op);
    //            nRet = getsockopt(lp_io->socket, SOL_SOCKET, SO_CONNECT_TIME, (char*)&op, &op_len);
    //            if(SOCKET_ERROR == nRet)
    //            {
    //                glog::trace("SO_CONNECT_TIME failed:");
    //                glog::traceErrorInfo("getsockopt", WSAGetLastError());
    //                pos++;
    //                continue;
    //            }
    //            if(op != 0xffffffff && op > 20)
    //            {
    //                glog::GetInstance()->AddLine("开始移除集中器");
    //                EnterCriticalSection(&crtc_sec);
    //                //移除集中器
    //                //集中器客户端去端
    //                map<string, IOCP_IO_PTR>::iterator  it;
    //                for(it = m_mcontralcenter.begin(); it != m_mcontralcenter.end();)
    //                {
    //                    if(it->second == lp_io)
    //                    {
    //                        m_mcontralcenter.erase(it++);
    //                    }
    //                }
    //  glog::GetInstance()->AddLine("移除集中器map 集完成");
    //                int n1 = m_listctr->getRowCount();
    //                //删除界面条项
    // //               for(int i = 0; i < n1; i++)
    // //               {
    // //                   string vv = m_listctr->getCellText(i, 1);
    // //                   char pp[50] = {0};
    // //                   sprintf(pp, "%p", lp_io);
    // //                   if(_stricmp(pp, vv.c_str()) == 0)
    // //                   {
    // //                       m_listctr->deleteIndex(i);
    // //                       break;
    // //                   }
    // //               }
    //  //glog::GetInstance()->AddLine("移除界面项目完成");
    //                closesocket(lp_io->socket);
    //                m_io_group.RemoveAt(pos++);
    //                m_key_group.RemoveAt(lp_io->lp_key);
    //                LeaveCriticalSection(&crtc_sec);
    //        glog::GetInstance()->AddLine("有一个集中器连接，但长久没有接收到数据,已经被踢出去了");
    //                continue;
    //            }
    //        }
    //    }
    //    //glog::GetInstance()->AddLine("CheckForInvalidConnection:%p   state:%d", lp_io, lp_io->state);
    //    pos++;
    //}
}
/*-------------------------------------------------------------------------------------------
函数功能：注册FD_ACCEPTG事件到m_h_accept_event事件，以便所有发出去的连接耗耗尽时，得到通知。
函数说明：
函数返回：成功，TRUE；失败，FALSE
-------------------------------------------------------------------------------------------*/
BOOL CIOCP::RegAcceptEvent()
{
    int     nRet;
    m_h_accept_event = CreateEvent(NULL, FALSE, FALSE, NULL);

    if(NULL == m_h_accept_event)
    {
        return FALSE;
    }

    nRet = WSAEventSelect(m_listen_socket, m_h_accept_event, FD_ACCEPT);

    if(nRet != 0)
    {
        CloseHandle(m_h_accept_event);
        return FALSE;
    }

    return TRUE;
}
/*-------------------------------------------------------------------------------------------
函数功能：得到连接上来的客户端IP和PORT
函数说明：
函数返回：成功，TRUE；失败，FALSE
-------------------------------------------------------------------------------------------*/
BOOL CIOCP::GetAddrAndPort(char*buf, char ip[], UINT & port)
{
    int     len = BUFFER_SIZE - sizeof(SOCKADDR_IN) - 16;
    char    *lp_buf = buf + len;    //直接读取远端地址
    SOCKADDR_IN addr;
    memcpy(&addr, lp_buf, sizeof(SOCKADDR_IN));
    port    = ntohl(addr.sin_port);
    strcpy(ip, inet_ntoa(addr.sin_addr));
    MSG("客户IP为：");
    MSG(ip);
    MSG("客户端口为：");
    MSG(port);
    return TRUE;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////
/*-------------------------------------------------------------------------------------------
函数功能：初始化完成端口及相关的所有东西，并发出每一个10个连接.
函数说明：
函数返回：成功，TRUE；失败，FALSE
-------------------------------------------------------------------------------------------*/
BOOL CIOCP::Init()
{
    CoInitialize(NULL);

    while(!m_listmsg.empty())
    {
        m_listmsg.clear();
    }

    WSAData data;
    string pdir = GetDataDir("config.ini");
    char source[216] = {0};
    GetPrivateProfileStringA("Config", "source", "", source, 216, pdir.c_str());
    char database[216] = {0};
    GetPrivateProfileStringA("Config", "database", "", database, 216, pdir.c_str());
    char uname[216] = {0};
    GetPrivateProfileStringA("Config", "uname", "", uname, 216, pdir.c_str());
    char upass[216] = {0};
    GetPrivateProfileStringA("Config", "upass", "", upass, 216, pdir.c_str());
    BOOL bcon = dbopen.ConnToDB(source, database, uname, upass);
    //_RecordsetPtr rs =    dbopen.ExecuteWithResSQL("select * from t_lamp");

    if(WSAStartup(MAKEWORD(2, 2), &data) != 0)
    {
        cout << "WSAStartup fail!" << WSAGetLastError() << endl;
        return FALSE;
    }

    m_h_iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 0);

    if(NULL == m_h_iocp)
    {
        cout << "CreateIoCompletionPort() failed: " << GetLastError() << endl;
        return FALSE;
    }

    if(!StartThread())
    {
        cout << "start thread fail!" << endl;
        PostQueuedCompletionStatus(m_h_iocp, 0, NULL, NULL);
        CloseHandle(m_h_iocp);
        return FALSE;
    }

    if(!InitSocket())
    {
        PostQueuedCompletionStatus(m_h_iocp, 0, NULL, NULL);
        cout << "Init sociket fail!" << endl;
        CloseHandle(m_h_iocp);
        return FALSE;
    }

    if(!BindAndListenSocket())
    {
        PostQueuedCompletionStatus(m_h_iocp, 0, NULL, NULL);
        cout << "Init sociket fail!" << endl;
        CloseHandle(m_h_iocp);
        closesocket(m_listen_socket);
        return FALSE;
    }

    if(!GetFunPointer())
    {
        cout << "GetFunPointer fail!" << endl;
        PostQueuedCompletionStatus(m_h_iocp, 0, NULL, NULL);
        CloseHandle(m_h_iocp);
        closesocket(m_listen_socket);
        return FALSE;
    }

    if(!PostAcceptEx())
    {
        PostQueuedCompletionStatus(m_h_iocp, 0, NULL, NULL);
        cout << "PostAcceptEx fail!" << endl;
        CloseHandle(m_h_iocp);
        closesocket(m_listen_socket);
        return FALSE;
    }

    if(!RegAcceptEvent())
    {
        PostQueuedCompletionStatus(m_h_iocp, 0, NULL, NULL);
        cout << "RegAcceptEvent fail!" << endl;
        CloseHandle(m_h_iocp);
        closesocket(m_listen_socket);
        return FALSE;
    }

    return TRUE;
}
/*-------------------------------------------------------------------------------------------
函数功能：主循环
函数说明：
函数返回：成功，TRUE；失败，FALSE
-------------------------------------------------------------------------------------------*/
BOOL CIOCP::MainLoop()
{
    DWORD   dwRet;
    int     nCount = 0;
    cout << "Server is running.........." << nCount++ << " times" << endl;
    int ii = 0;

    while(TRUE)
    {
        dwRet = WaitForSingleObject(m_h_accept_event, 3000);

        switch(dwRet)
        {
            case WAIT_FAILED:
                {
                    PostQueuedCompletionStatus(m_h_iocp, 0, 0, NULL);
                    return FALSE;
                }
                break;

            case WAIT_TIMEOUT:
                {
                    //检测集中器超时处理
                    //cout << "Server is running.........." << nCount++ << " times" << endl;
                    CheckForInvalidConnection();
                }
                break;

            case WAIT_OBJECT_0:   //接收到了所有发出的连接都用光了的消息，再次发出连接
                {
                    if(!PostAcceptEx())
                    {
                        PostQueuedCompletionStatus(m_h_iocp, 0, 0, NULL);
                        return FALSE;
                    }
                }
                break;
        }
    }

    return TRUE;
}
/*-------------------------------------------------------------------------------------------
函数功能：数据处理线程函数
函数说明：
函数返回：
-------------------------------------------------------------------------------------------*/
DWORD CIOCP::CompletionRoutine(LPVOID lp_param)
{
    CIOCP*          lp_this         = (CIOCP*)lp_param;
    int             nRet;
    BOOL            bRet;
    DWORD           dwBytes         = 0;
    HANDLE          hRet;
    IOCP_KEY_PTR    lp_key          = NULL;
    IOCP_IO_PTR     lp_io           = NULL;
    LPWSAOVERLAPPED lp_ov           = NULL;
    IOCP_KEY_PTR    lp_new_key      = NULL;

    while(TRUE)
    {
        bRet = GetQueuedCompletionStatus(lp_this->m_h_iocp, &dwBytes, (LPDWORD)&lp_key, &lp_ov, INFINITE);  //
        lp_io   = (IOCP_IO_PTR)lp_ov;

        if(FALSE == bRet)
        {
            glog::traceErrorInfo("GetQueuedCompletionStatus", GetLastError());
            lp_this->m_io_group.RemoveAt(lp_io);
            lp_this->m_key_group.RemoveAt(lp_key);
            continue;
        }

        if(lp_io == NULL)
        {
            glog::trace("recv a null CIoContext!");
            continue;
        }

        if(NULL == lp_key)
        {
            return 0;
        }

        if((IOCP_ACCEPT != lp_io->operation) && (0 == dwBytes))
        {
            if(lp_io->fromtype == SOCKET_FROM_Concentrator)
            {
                glog::GetInstance()->AddLine("一个集中器客户端下线");
            }

            int n1 = lp_this-> m_listctr->getRowCount();
            int nrow = -1;
            string towrite = "";
            EnterCriticalSection(&lp_this->crtc_sec);

            //移除集中器
            if(lp_io->fromtype == SOCKET_FROM_Concentrator)
            {
                //集中器客户端去端
                map<string, IOCP_IO_PTR>::iterator  it;

                for(it = lp_this->m_mcontralcenter.begin(); it != lp_this->m_mcontralcenter.end();)
                {
                    if(it->second == lp_io)
                    {
                        lp_this->m_mcontralcenter.erase(it++);
                    }
                }
            }

            //删除界面条项
            for(int i = 0; i < n1; i++)
            {
                string vv = lp_this->m_listctr->getCellText(i, 1);
                char pp[50] = {0};
                sprintf(pp, "%p", lp_io);

                if(_stricmp(pp, vv.c_str()) == 0)
                {
                    lp_this->m_listctr->deleteIndex(i);
                    break;
                }
            }

            //消息队列删除   消息队列存的是网页客户端
            list<IOCP_IO*>::iterator it;

            for(it = lp_this->m_listmsg.begin(); it != lp_this->m_listmsg.end();)
            {
                IOCP_IO_PTR tem = *it;

                if(tem == lp_io)
                {
                    it = lp_this->m_listmsg.erase(it);
                }
                else
                {
                    it++;
                }
            }

            LeaveCriticalSection(&lp_this->crtc_sec);
            closesocket(lp_io->socket);
            lp_this->m_io_group.RemoveAt(lp_io);
            lp_this->m_key_group.RemoveAt(lp_key);
            glog::trace("一个用户退出了 operation:%d", lp_io->operation);
            continue;
        }

        switch(lp_io->operation)
        {
            case IOCP_ACCEPT:
                {
                    lp_io->state = SOCKET_STATE_CONNECT;

                    if(dwBytes > 0)
                    {
                        lp_io->state = SOCKET_STATE_CONNECT_AND_READ;
                    }

                    nRet = setsockopt(lp_io->socket, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, (char*)&lp_this->m_listen_socket, sizeof(lp_this->m_listen_socket));

                    if(SOCKET_ERROR == nRet)
                    {
                        closesocket(lp_io->socket);
                        lp_this->m_io_group.RemoveAt(lp_io);
                        glog::traceErrorInfo("setsockopt", WSAGetLastError());
                        continue;
                    }

                    lp_new_key = lp_this->m_key_group.GetBlank();

                    if(lp_new_key == NULL)
                    {
                        closesocket(lp_io->socket);
                        lp_this->m_io_group.RemoveAt(lp_io);
                        cout << "get a handle fail!" << GetLastError() << endl;
                        continue;
                    }

                    lp_new_key->socket = lp_io->socket;
                    lp_io->lp_key = lp_new_key;
                    //将新建立的SOCKET同完成端口关联起来。
                    hRet = CreateIoCompletionPort((HANDLE)lp_io->socket, lp_this->m_h_iocp, (DWORD)lp_new_key, 0);

                    if(NULL == hRet)
                    {
                        closesocket(lp_io->socket);
                        lp_this->m_key_group.RemoveAt(lp_new_key);
                        lp_this->m_io_group.RemoveAt(lp_io);
                        cout << "link to iocp fail!" << WSAGetLastError() << endl;
                        continue;
                    }

                    //处理读取到的数据
                    if(dwBytes > 0)
                    {
                        lp_io->wsaBuf.len = dwBytes;
                        lp_this->HandleData(lp_io, IOCP_COMPLETE_ACCEPT_READ, lp_new_key);
                        bRet = lp_this->DataAction(lp_io, lp_new_key);

                        if(FALSE == bRet)
                        {
                            continue;
                        }
                    }
                    else
                    {
                        lp_this->HandleData(lp_io, IOCP_COMPLETE_ACCEPT, lp_new_key);
                        bRet = lp_this->DataAction(lp_io, lp_new_key);

                        if(FALSE == bRet)
                        {
                            continue;
                        }
                    }
                }
                break;

            case IOCP_READ:
                {
                    if(SOCKET_STATE_CONNECT_AND_READ != lp_io->state)
                    {
                        lp_io->state = SOCKET_STATE_CONNECT_AND_READ;
                    }

                    //   lp_this->HandleData(lp_io, IOCP_COMPLETE_READ, lp_new_key);
                    int n1 = lp_this-> m_listctr->getRowCount();
                    int nrow = -1;
                    string towrite = "";

                    for(int i = 0; i < n1; i++)
                    {
                        string vv = lp_this->m_listctr->getCellText(i, 1);
                        char pp[50] = {0};
                        sprintf(pp, "%p", lp_io);

                        if(_stricmp(pp, vv.c_str()) == 0)
                        {
                            string data = gstring::char2hex(lp_io->buf, lp_io->ol.InternalHigh);
                            lp_this->m_listctr->setItemText(data.c_str(), i, 4);
                            char buff[1024] = {0};
                            memcpy(buff, lp_io->buf, lp_io->ol.InternalHigh);
                            lp_this->m_listctr->setItemText(buff, i, 5);
                            nrow = i;
                            towrite = data;
                        }
                    }

                    if(lp_this->checkFlag((BYTE*)lp_io->buf, lp_io->ol.InternalHigh))
                    {
                        lp_io->fromtype = SOCKET_FROM_Concentrator;
                        BYTE tosend[216] = {0};
                        int  deslen = 0;
                        BOOL bresponse = FALSE;
                        glog::GetInstance()->AddLine("接收集中器的包:%s", towrite.c_str());
                        lp_this->buildcode((BYTE*)lp_io->buf, lp_io->ol.InternalHigh, tosend, deslen, bresponse, lp_io);

                        //SetDlgItemTextA(lp_this->hWnd, IDC_STATIC1, "来自集中器");
                        if(nrow >= 0 && lp_io->fromtype == SOCKET_FROM_Concentrator)
                        {
                            lp_this->m_listctr->setItemText("来自集中器", nrow, 6);
                        }

                        if(nrow >= 0 && lp_io->loginstatus == SOCKET_STATUS_LOGIN)
                        {
                            lp_this->m_listctr->setItemText("集中器已经连在线", nrow, 7);
                        }

                        if(bresponse && deslen > 0)
                        {
                            lp_this->InitIoContext(lp_io);
                            string hex = gstring::char2hex((char*)tosend, deslen);
                            glog::GetInstance()->AddLine("响应集中器的包:%s", hex.c_str());
                            memcpy(lp_io->buf, tosend, deslen);
                            lp_io->wsaBuf.len = deslen;
                            lp_io->operation = IOCP_WRITE;
                        }
                    }
                    else   //websocket
                    {
                        string strdata = lp_io->buf;
                        string strret;
                        int wsconn = lp_this->wsHandshake(strdata, strret);

                        if(wsconn == WS_STATUS_CONNECT)
                        {
                            lp_this->InitIoContext(lp_io);
                            lp_io->operation = IOCP_WRITE;
                            lp_io->fromtype = SOCKET_FROM_WEBSOCKET;
                            lp_io->loginstatus = SOCKET_STATUS_LOGIN;
                            memcpy(lp_io->buf, strret.c_str(), strret.size());
                            lp_this->m_listctr->setItemText("来自网页", nrow, 6);
                            lp_io->wsaBuf.len = strret.size();
                            goto TOHear;
                        }

                        strret = "";
                        int lenread = lp_this->wsDecodeFrame(lp_io->buf, strret, lp_io->ol.InternalHigh);

                        if(lenread == WS_CLOSING_FRAME)
                        {
                            //closesocket(lp_io->socket);
                            //lp_this->m_io_group.RemoveAt(lp_io);
                            //lp_this->m_key_group.RemoveAt(lp_key);
                            //glog::trace("一个用户退出了 operation:%d", lp_io->operation);
                            int n1 = lp_this-> m_listctr->getRowCount();
                            int nrow = -1;
                            string towrite = "";
                            EnterCriticalSection(&lp_this->crtc_sec);
                            //删除消息队列
                            list<IOCP_IO*>::iterator it;

                            for(it = lp_this->m_listmsg.begin(); it != lp_this->m_listmsg.end();)
                            {
                                IOCP_IO_PTR tem = *it;

                                if(tem == lp_io)
                                {
                                    it = lp_this->m_listmsg.erase(it);
                                }
                                else
                                {
                                    it++;
                                }
                            }

                            for(int i = 0; i < n1; i++)
                            {
                                string vv = lp_this->m_listctr->getCellText(i, 1);
                                char pp[50] = {0};
                                sprintf(pp, "%p", lp_io);

                                if(_stricmp(pp, vv.c_str()) == 0)
                                {
                                    lp_this->m_listctr->deleteIndex(i);
                                    break;
                                }
                            }

                            LeaveCriticalSection(&lp_this->crtc_sec);
                            lp_io->operation = IOCP_END;
                        }
                        else if(lenread != WS_ERROR_FRAME)
                        {
                            glog::trace("%s", strret.c_str());
                            lp_this->dealws(lp_io, strret);
                            goto TOHear;
                        }
                    }

TOHear:
                    bRet = lp_this->DataAction(lp_io, lp_new_key);

                    if(FALSE == bRet)
                    {
                        continue;
                    }
                }
                break;

            case IOCP_WRITE:
                {
                    lp_this->HandleData(lp_io, IOCP_COMPLETE_WRITE, lp_new_key);
                    bRet = lp_this->DataAction(lp_io, lp_new_key);

                    if(FALSE == bRet)
                    {
                        continue;
                    }
                }
                break;

            default:
                {
                    continue;
                }
                break;
        }
    }

    return 0;
}
BOOL CIOCP::SendData(ULONG_PTR s, ULONG_PTR key)
{
    IOCP_IO_PTR  piocp_prt = (IOCP_IO_PTR)s;
    IOCP_KEY_PTR piocp_key = (IOCP_KEY_PTR)key;
    int n1 = m_listctr->getSelectIndex();
    string vvv = m_listctr->getCellText(n1, 2);
    vvv = gstring::replace(vvv, " ", "");
    char* p1 = (char*)vvv.c_str();
    BYTE b2[1024] = {0};
    int i = 0;

    while(*p1 != '\0')
    {
        char data[3] = {0};
        memcpy(data, p1, 2);
        b2[i] = strtol(data, NULL, 16);
        p1 += 2;
        i++;
    }

    //string tosend = gstring::hex2char(vvv.c_str(),strlen(vvv.c_str()));
//   strcpy(piocp_prt->buf, tosend.c_str());
    memcpy(piocp_prt->buf, b2, i);
    piocp_prt->operation = IOCP_WRITE;
    piocp_prt->wsaBuf.len = strlen(vvv.c_str()) / 2;
    piocp_prt->wsaBuf.buf = piocp_prt->buf;
    DataAction(piocp_prt, piocp_prt->lp_key);
    return TRUE;
}
BOOL CIOCP::SendWebsocket(ULONG_PTR s)
{
    IOCP_IO_PTR  lp_io = (IOCP_IO_PTR)s;
    string str = "bbb";
    Json::Value root;
    Json::Value item;
    item["msg"] = "abcdefg";
    root.append(item);
    string srcmsg = root.toStyledString();
    char outmsg[1024] = {};
    this->InitIoContext(lp_io);
    int len = 0;
    this->wsEncodeFrame(srcmsg.c_str(), outmsg, WS_TEXT_FRAME, len);
    lp_io->wsaBuf.len = len;
    memcpy(lp_io->buf, outmsg, len);
    lp_io->wsaBuf.buf = lp_io->buf;
    lp_io->operation = IOCP_WRITE;
    DataAction(lp_io, lp_io->lp_key);
    return TRUE;
}
int CIOCP::hex2str(string str, BYTE tosend[])
{
    string vvv = gstring::replace(str, " ", "");
    char* p1 = (char*)vvv.c_str();
    //BYTE b2[1024] = {0};
    int i = 0;

    while(*p1 != '\0')
    {
        char data[3] = {0};
        memcpy(data, p1, 2);
        tosend[i] = strtol(data, NULL, 16);
        p1 += 2;
        i++;
    }

    //memcpy(tosend,, i);
    return i;
}
int CIOCP::wsHandshake(string & request, string & response)
{
    // 解析http请求头信息
    int ret = WS_STATUS_UNCONNECT;
    std::istringstream stream(request.c_str());
    std::string reqType;
    std::getline(stream, reqType);

    if(reqType.substr(0, 4) != "GET ")
    {
        return ret;
    }

    std::string header;
    std::string::size_type pos = 0;
    std::string websocketKey;

    while(std::getline(stream, header) && header != "\r")
    {
        header.erase(header.end() - 1);
        pos = header.find(": ", 0);

        if(pos != std::string::npos)
        {
            std::string key = header.substr(0, pos);
            std::string value = header.substr(pos + 2);

            if(key == "Sec-WebSocket-Key")
            {
                ret = WS_STATUS_CONNECT;
                websocketKey = value;
                break;
            }
        }
    }

    if(ret != WS_STATUS_CONNECT)
    {
        return ret;
    }

    // 填充http响应头信息
    response = "HTTP/1.1 101 Switching Protocols\r\n";
    response += "Upgrade: websocket\r\n";
    response += "Connection: upgrade\r\n";
    response += "Sec-WebSocket-Accept: ";
    const std::string magicKey("258EAFA5-E914-47DA-95CA-C5AB0DC85B11");
    std::string serverKey = websocketKey + magicKey;
    char shaHash[32];
    memset(shaHash, 0, sizeof(shaHash));
    CSHA1 sha1;
    sha1.Update((unsigned char*)serverKey.c_str(), serverKey.size());
    sha1.Final();
    sha1.GetHash((unsigned char *)shaHash);
    serverKey = gstring::base64_encode((unsigned char*)shaHash, strlen(shaHash)) + "\r\n\r\n";
    string strtmp(serverKey.c_str());
    response += strtmp;
    return ret;
}
int CIOCP::wsDecodeFrame(char inFrame[], string & outMessage, int len)
{
    int ret = WS_OPENING_FRAME;
    const char *frameData = inFrame;
    const int frameLength = len;

    if(frameLength < 2)
    {
        ret = WS_ERROR_FRAME;
    }

    // 检查扩展位并忽略
    if((frameData[0] & 0x70) != 0x0)
    {
        ret = WS_ERROR_FRAME;
    }

    // fin位: 为1表示已接收完整报文, 为0表示继续监听后续报文
    ret = (frameData[0] & 0x80);  //1000 0000

    if((frameData[0] & 0x80) != 0x80)
    {
        ret = WS_ERROR_FRAME;
    }

    // mask位, 为1表示数据被加密
    if((frameData[1] & 0x80) != 0x80)
    {
        ret = WS_ERROR_FRAME;
    }

    // 操作码
    uint16_t payloadLength = 0;
    uint8_t payloadFieldExtraBytes = 0;
    uint8_t opcode = static_cast<uint8_t >(frameData[0] & 0x0f);

    if(opcode == WS_TEXT_FRAME)
    {
        // 处理utf-8编码的文本帧
        payloadLength = static_cast<uint16_t >(frameData[1] & 0x7f);

        if(payloadLength == 0x7e)//0111 1110     //126 7e  后面两字节是长度 :  127  7f 后面四字节是长度
        {
            uint16_t payloadLength16b = 0;
            payloadFieldExtraBytes = 2;
            memcpy(&payloadLength16b, &frameData[2], payloadFieldExtraBytes);
            payloadLength = ntohs(payloadLength16b);
        }
        else if(payloadLength == 0x7f)
        {
            // 数据过长,暂不支持
            ret = WS_ERROR_FRAME;
        }
    }
    else if(opcode == WS_BINARY_FRAME || opcode == WS_PING_FRAME || opcode == WS_PONG_FRAME)
    {
        // 二进制/ping/pong帧暂不处理
    }
    else if(opcode == WS_CLOSING_FRAME)
    {
        ret = WS_CLOSING_FRAME;
    }
    else
    {
        ret = WS_ERROR_FRAME;
    }

    // 数据解码
    if((ret != WS_ERROR_FRAME) && (payloadLength > 0))
    {
        // header: 2字节, masking key: 4字节
        const char *maskingKey = &frameData[2 + payloadFieldExtraBytes];
        char *payloadData = new char[payloadLength + 1];
        memset(payloadData, 0, payloadLength + 1);
        memcpy(payloadData, &frameData[2 + payloadFieldExtraBytes + 4], payloadLength);

        for(int i = 0; i < payloadLength; i++)
        {
            payloadData[i] = payloadData[i] ^ maskingKey[i % 4];
        }

        size_t len1 = 0;
        int num = payloadLength * 3;
        WCHAR* poutBuf = new WCHAR[num];
        memset(poutBuf, 0, num);
        char outbuff[4096] = {0};
        int lenout = payloadLength;
        BOOL bchar = gstring::UrlDecode(payloadData, outbuff, lenout);
        outMessage = outbuff;
        //  WCHAR* ddd = QXUtf82Unicode(payloadData, &len1, poutBuf);
        //outMessage=gstring::WStringToString(ddd);
        //    outMessage = payloadData;
        delete[] payloadData;
//         if(poutBuf)
//         {
//             delete[] ddd;
//         }
    }

    return ret;
}
int CIOCP::wsEncodeFrame(string inMessage, char outFrame[], enum WS_FrameType frameType, int& lenret)
{
    int ret = WS_EMPTY_FRAME;
    const uint32_t messageLength = inMessage.size();

    if(messageLength > 32767)
    {
        // 暂不支持这么长的数据
        return WS_ERROR_FRAME;
    }

    uint16_t payloadFieldExtraBytes = (messageLength <= 0x7d) ? 0 : 2;
    // header: 2字节, mask位设置为0(不加密), 则后面的masking key无须填写, 省略4字节
    uint8_t frameHeaderSize = 2 + payloadFieldExtraBytes;
    uint8_t *frameHeader = new uint8_t[frameHeaderSize];
    memset(frameHeader, 0, frameHeaderSize);
    // fin位为1, 扩展位为0, 操作位为frameType
    frameHeader[0] = static_cast<uint8_t>(0x80 | frameType);

    // 填充数据长度
    if(messageLength <= 0x7d)  //125->7d
    {
        frameHeader[1] = static_cast<uint8_t>(messageLength);
    }
    else if(messageLength < 65535)
    {
        frameHeader[1] = 0x7e;
        uint16_t len = htons(messageLength);
        memcpy(&frameHeader[2], &len, payloadFieldExtraBytes);
    }
    else
    {
    }

    // 填充数据
    uint32_t frameSize = frameHeaderSize + messageLength;
    char *frame = new char[frameSize + 1];
    memcpy(frame, frameHeader, frameHeaderSize);
    memcpy(frame + frameHeaderSize, inMessage.c_str(), messageLength);
    frame[frameSize] = '\0';
    memcpy(outFrame, frame, frameSize);
    lenret = frameSize;
    //outFrame = frame;
    delete[] frame;
    delete[] frameHeader;
    return ret;
}
void CIOCP::dealws(IOCP_IO_PTR & lp_io, string & jsondata)
{
    Json::Value root;
    Json::Reader reader;

    if(reader.parse(jsondata.c_str(), root))
    {
        Json::Value vtemp = root["msg"];

        if(vtemp.isNull())
        {
            glog::trace("\nthis is null data pack\n");
        }
        else if(vtemp.isArray())
        {
            glog::trace("\nthis is array data pack\n");
        }
        else if(vtemp.isObject())
        {
            glog::trace("\nthis is object data pack\n");
        }
        else if(vtemp.isString())
        {
            if(vtemp == "getStatus")
            {
                string addrarea = root["addr"].asString();
                map<string, IOCP_IO_PTR>::iterator ite = m_mcontralcenter.find(addrarea);

                if(ite != m_mcontralcenter.end())
                {
                    Json::Value row = root["row"];
                    root["data"] = TRUE;
                }

                string inmsg = root.toStyledString();
                //  string inmsg = root.toStyledString();
                char outmsg[1048] = {0};
                int lenret = 0;
                int len = wsEncodeFrame(inmsg, outmsg, WS_TEXT_FRAME, lenret);

                if(len != WS_ERROR_FRAME)
                {
                    memcpy(lp_io->buf, outmsg, lenret);
                    lp_io->wsaBuf.buf = lp_io->buf;
                    lp_io->wsaBuf.len = lenret;
                    lp_io->operation = IOCP_WRITE;
                }
            }
            else if(vtemp == "setParam")
            {
                Json::Value isres = root["res"];

                if(isres.asString() == "1")  //发给集中器要求有响应
                {
                    string addrarea = root["addr"].asString();
                    map<string, IOCP_IO_PTR>::iterator ite = m_mcontralcenter.find(addrarea);

                    if(ite != m_mcontralcenter.end())
                    {
                        m_listmsg.push_back(lp_io);
                        glog::GetInstance()->AddLine("总消息长度:%d 添加一个消息队列:%s 参赛设置命令", m_listmsg.size(), root["data"].asString().c_str());
                    }

                    //m_listwebsock.push_back(lp_io);
                    //m_qwebsock.push()
                }

                Json::Value tosend = root["data"];
                string data = tosend.asString();
                data = gstring::replace(data, " ", "");
                BYTE bitSend[512] = {0};
                int len = hex2str(data, bitSend);

                if(len > 0)
                {
                    string addrarea = root["addr"].asString();
                    map<string, IOCP_IO_PTR>::iterator ite = m_mcontralcenter.find(addrarea);

                    if(ite != m_mcontralcenter.end())
                    {
                        IOCP_IO_PTR lp_io1 = ite->second;
                        memcpy(lp_io1->buf, bitSend, len);
                        lp_io1->wsaBuf.buf = lp_io1->buf;
                        lp_io1->wsaBuf.len = len;
                        lp_io1->operation = IOCP_WRITE;
                        DataAction(lp_io1, lp_io1->lp_key);
                    }
                }
            }
            else if(vtemp == "contrParam")
            {
                Json::Value isres = root["res"];

                if(isres.asString() == "1")  //发给集中器要求有响应
                {
                    string addrarea = root["addr"].asString();
                    map<string, IOCP_IO_PTR>::iterator ite = m_mcontralcenter.find(addrarea);

                    if(ite != m_mcontralcenter.end())
                    {
                        m_listmsg.push_back(lp_io);
                        glog::GetInstance()->AddLine("总消息长度:%d 添加一个消息队列:%s 控制命令", m_listmsg.size(), root["data"].asString().c_str());
                    }

                    //m_listwebsock.push_back(lp_io);
                    //m_qwebsock.push()
                }

                Json::Value tosend = root["data"];
                string data = tosend.asString();
                data = gstring::replace(data, " ", "");
                BYTE bitSend[512] = {0};
                int len = hex2str(data, bitSend);

                if(len > 0)
                {
                    string addrarea = root["addr"].asString();
                    map<string, IOCP_IO_PTR>::iterator ite2 = m_mcontralcenter.find(addrarea);

                    if(ite2 != m_mcontralcenter.end())
                    {
                        IOCP_IO_PTR lp_io1 = ite2->second;
                        memcpy(lp_io1->buf, bitSend, len);
                        lp_io1->wsaBuf.buf = lp_io1->buf;
                        lp_io1->wsaBuf.len = len;
                        lp_io1->operation = IOCP_WRITE;
                        DataAction(lp_io1, lp_io1->lp_key);
                    }
                }
            }
        }
    }
}
void CIOCP::getall()
{
    ITERATOR ite;
    IOCP_IO_PTR p1 = m_io_group.GetHeadPosition(ite);

    while(p1 != NULL)
    {
        p1 = m_io_group.GetNext(ite);
    }
}
std::string CIOCP::GetDataDir(string name)
{
    char pdir[216] = {0};
    GetModuleFileNameA(NULL, pdir, 216);
    PCHAR  pfind = strrchr((char*)pdir, '\\');

    if(pfind)
    {
        memset(pfind + 1, 0, 40);
        strcat(pdir, name.c_str());
    }

    return string(pdir);
}
BOOL CIOCP::checkFlag(BYTE vv[], int len)
{
    if(len < 6)
    {
        return FALSE;
    }

    if(vv[0] == 0x68 && vv[5] == 0x68 && vv[len - 1] == 0x16)
    {
        int nbyte = len - 2 - 6;
        short n11 = (nbyte << 2) | 2;
        short to1 =  *(short*)&vv[1];
        short to2 =  *(short*)&vv[3];
        BYTE  bend = 0;

        for(int j = 6; j < len - 2; j++)
        {
            bend += vv[j];
        }

        if(bend == vv[len - 2] && n11 == to1 && n11 == to2)
        {
            return TRUE;
        }
    }

    return FALSE;
}
void  CIOCP::changeByte(char data[], BYTE vv[], int& len)
{
    char *p = data;
    int i = 0;

    while(*p != '\0')
    {
        char p1[3] = {0};
        memcpy(p1, p, 2);
        BYTE b1 = strtol(p1, NULL, 16);
        vv[i] = b1;
        p += 2;
        i++;
    }

    len = i;
}
//生成响应代码  src
/*
*  src 源收到的数据包
*  srclen 源包长度
*  des  生成目标的包
*/
void CIOCP::buildcode(BYTE src[], int srclen, BYTE des[], int& deslen, BOOL & isrespos, IOCP_IO_PTR & lp_io)
{
    //链路检测 登陆  控制域 c4: 1100 0100  功能码：0x02 src[13] da1 src[14] da2 src[15] dt0    p0  f1  登陆   6控制域  13帧序列 12
    //&&src[14]==0x0&&src[15]==0x0&&src[16]==0x01&&src[17]==1   1100 0000 1100 0000   1 dir  1 yn PRM  6控制域  帧是是否要回复 src[13]
    BYTE AFN = src[12];

    if(AFN == 0x2)      //链路检测
    {
        //src[6] == 0xc4 && src[13] & 0x10 == 0x10
        BYTE    con =    src[13] & 0x10;
        BYTE   DirPrmCode = src[6] & 0xc4;
        BYTE DA[2] = {0};
        BYTE DT[2] = {0};
        memcpy(DA, &src[14], 2);    //PN P0
        memcpy(DT, &src[16], 2);   //FN  1 登陆 | 3 心跳

        if(DirPrmCode == 0xc4 && con == 0x10)    //需要回复
        {
            USHORT Pn = (USHORT) * DA;
            USHORT Fn = (USHORT) * DT;
            isrespos = TRUE;

            if(Pn == 0)   //P0
            {
                if(DT[1] == 0)  //DT1组
                {
                    if(DT[0] == 1) //DT0 组功能点
                    {
                        glog::GetInstance()->AddLine("登陆包");
                        glog::trace("登陆包");
                        char addr1[10] = {0};
                        memcpy(addr1, &src[7], 4);
                        string addrarea = gstring::char2hex(addr1, 4);
                        map<string, IOCP_IO_PTR>::iterator it = m_mcontralcenter.find(addrarea);
                        lp_io->nexttime =  GetTickCount();

                        if(it == m_mcontralcenter.end())
                        {
                            m_mcontralcenter.insert(pair<string, IOCP_IO_PTR>(addrarea, lp_io));
                        }
                        else
                        {
								//m_mcontralcenter.erase(it);
							  // m_mcontralcenter.insert(pair<string, IOCP_IO_PTR>(addrarea, lp_io));

						}

                        buildConCode(src, des, deslen, 1);
                    }
                    else if(DT[0] == 4)
                    {
                        //02170101
                        //m_mcontralcenter
                        glog::GetInstance()->AddLine("心跳包");
                        glog::trace("心跳包\n");
                        lp_io->nexttime = GetTickCount();
                        lp_io->loginstatus = SOCKET_STATUS_LOGIN;
                        buildConCode(src, des, deslen, 1);
                    }
                }
            }
        }
    }
    else if(AFN == 0x00)              //全部确认
    {
        BYTE    con =    src[13] & 0x10;
        BYTE   DirPrmCode = src[6] & 0xc0;   //上行  从动
        BYTE   FC = src[6] & 0xF; //控制域名的功能码
        BYTE DA[2] = {0};
        BYTE DT[2] = {0};
        memcpy(DA, &src[14], 2);    //PN P0
        memcpy(DT, &src[16], 2);   //FN  1 登陆 | 3 心跳

        if(DirPrmCode == 0x80 && con == 0x0 && FC == 0x8) //  上行 从动 响应帧   0x80 上行 从动
        {
            BYTE frame = src[13] & 0xf;   //帧序号
            BYTE Fn = DT[1] * 8 + DT[0];
            glog::GetInstance()->AddLine("集中器响应帧:%d 确认 Fn:%d", frame, Fn);

            if(Fn == 1)
            {
                //全部确认
                if(!m_listmsg.empty())
                {
                    IOCP_IO_PTR lp_io1 = m_listmsg.back();
                    m_listmsg.pop_back();
                    string strret = "";
                    int lenread = wsDecodeFrame(lp_io1->buf, strret, lp_io1->ol.InternalHigh);
                    Json::Value root;
                    Json::Reader reader;

                    if(reader.parse(strret.c_str(), root))
                    {
                        SHORT setnum = (SHORT) * (src + 18); //错误的装置号
                        BYTE  errcode = (BYTE) * (src + 20);
                        root["status"] = "success";
                        root["frame"] = frame;
                        root["setnum"] = setnum;
                        root["errcode"] = errcode;
                        root["data"] = gstring::char2hex((const char*)src, srclen);
                        root["length"] = srclen;
                        string inmsg = root.toStyledString();
                        char outmsg[1048] = {0};
                        int lenret = 0;
                        int len = wsEncodeFrame(inmsg, outmsg, WS_TEXT_FRAME, lenret);

                        if(len != WS_ERROR_FRAME)
                        {
                            memcpy(lp_io1->buf, outmsg, lenret);
                            lp_io1->wsaBuf.buf = lp_io1->buf;
                            lp_io1->wsaBuf.len = lenret;
                            lp_io1->operation = IOCP_WRITE;
                            DataAction(lp_io1, lp_io1->lp_key);
                        }
                    }
                }
            }
            else if(Fn == 2)   //全部否认
            {
                //全部否认
                if(!m_listmsg.empty())
                {
                    IOCP_IO_PTR lp_io1 = m_listmsg.back();
                    m_listmsg.pop_back();
                    Json::Value jsonRoot; //定义根节点
                    Json::Value jsonItem; //定义一个子对象
                    jsonItem["msg"] = "fail "; //添加数据
                    jsonItem["frame"] = frame;
                    jsonRoot.append(jsonItem);
                    jsonRoot["data"] = gstring::char2hex((const char*)src, srclen);
                    jsonRoot["length"] = srclen;
                    string inmsg = jsonRoot.toStyledString();
                    char outmsg[1048] = {0};
                    int lenret = 0;
                    int len = wsEncodeFrame(inmsg, outmsg, WS_TEXT_FRAME, lenret);

                    if(len != WS_ERROR_FRAME)
                    {
                        memcpy(lp_io1->buf, outmsg, lenret);
                        lp_io1->wsaBuf.buf = lp_io1->buf;
                        lp_io1->wsaBuf.len = lenret;
                        lp_io1->operation = IOCP_WRITE;
                        DataAction(lp_io1, lp_io1->lp_key);
                    }
                }
            }
            else if(Fn == 4)      //按单元标识事件确认
            {
                if(!m_listmsg.empty())
                {
                    IOCP_IO_PTR lp_io1 = m_listmsg.back();
                    m_listmsg.pop_back();
                    string strret = "";
                    int lenread = wsDecodeFrame(lp_io1->buf, strret, lp_io1->ol.InternalHigh);
                    Json::Value root;
                    Json::Reader reader;

                    if(reader.parse(strret.c_str(), root))
                    {
                        SHORT setnum = (SHORT) * (src + 18); //错误的装置号
                        BYTE  errcode = (BYTE) * (src + 20);
                        root["status"] = "fail";
                        root["frame"] = frame;
                        root["setnum"] = setnum;
                        root["errcode"] = errcode;
                        root["data"] = gstring::char2hex((const char*)src, srclen);
                        root["length"] = srclen;
                        string inmsg = root.toStyledString();
                        char outmsg[1048] = {0};
                        int lenret = 0;
                        int len = wsEncodeFrame(inmsg, outmsg, WS_TEXT_FRAME, lenret);

                        if(len != WS_ERROR_FRAME)
                        {
                            memcpy(lp_io1->buf, outmsg, lenret);
                            lp_io1->wsaBuf.buf = lp_io1->buf;
                            lp_io1->wsaBuf.len = lenret;
                            lp_io1->operation = IOCP_WRITE;
                            DataAction(lp_io1, lp_io1->lp_key);
                        }
                    }
                }
            }
            else if(Fn == 3)
            {
                IOCP_IO_PTR lp_io1 = m_listmsg.back();
                m_listmsg.pop_back();
            }
        }
    }
    else if(AFN == 0xAC)
    {
        glog::GetInstance()->AddLine("请求1类数据命令");
        BYTE    con =    src[13] & 0x10;
        BYTE   DirPrmCode = src[6] & 0xc0;   //上行  从动
        BYTE   FC = src[6] & 0xF; //控制域名的功能码
        BYTE DA[2] = {0};
        BYTE DT[2] = {0};
        memcpy(DA, &src[14], 2);    //PN P0
        memcpy(DT, &src[16], 2);   //F35  三相电压   00 00 04 04 昨天三相电压

        if(DirPrmCode == 0x80 && con == 0x0 && FC == 0x8) //  上行 从动 响应帧   0x80 上行 从动
        {
            //三相电压
            if(DA[0] == 0 && DA[1] == 0 && DT[0] == 0x4 && DT[1] == 0x4)
            {
                BYTE* p1 = src;
                Json::Value jsonRoot;
                int p = 0;
                string strA;
                string strB;
                string strC;

                for(int i = 18; i < srclen - 2; i += 6)
                {
                    BYTE A1[2] = {0};
                    BYTE B1[2] = {0};
                    BYTE C1[2] = {0};
                    memcpy(A1, &src[i], 2);
                    memcpy(B1, &src[i + 2], 2);
                    memcpy(C1, &src[i + 4], 2);
                    char strA1[16] = {0};
                    char strB1[16] = {0};
                    char strC1[16] = {0};
                    BYTE b = A1[1] >> 4 & 0x0f;
                    BYTE s = A1[1] & 0x0f;
                    BYTE g = A1[0] >> 4 & 0x0f;
                    BYTE sfw = A1[0] & 0x0f;
                    sprintf(strA1, "%d%d%d.%d", b, s, g, sfw);
                    b = B1[1] >> 4 & 0x0f;
                    s = B1[1] & 0x0f;
                    g = B1[0] >> 4 & 0x0f;
                    sfw = B1[0] & 0x0f;
                    sprintf(strB1, "%d%d%d.%d", b, s, g, sfw);
                    b = C1[1] >> 4 & 0x0f;
                    s = C1[1] & 0x0f;
                    g = C1[0] >> 4 & 0x0f;
                    sfw = C1[0] & 0x0f;
                    sprintf(strC1, "%d%d%d.%d", b, s, g, sfw);
                    strA.append(strA1);
                    strA.append("|");
                    strB.append(strB1);
                    strB.append("|");
                    strC.append(strC1);
                    strC.append("|");
                    p += 1;
                }

                jsonRoot["len"] = p;
                jsonRoot["A"] = strA;
                jsonRoot["B"] = strB;
                jsonRoot["C"] = strC;
                string inmsg = jsonRoot.toStyledString();
                string sql = "select * from t_records where 1=1 and CONVERT(Nvarchar, day, 23)=\'2017-07-29\'";
                _RecordsetPtr rs = this->dbopen.ExecuteWithResSQL(sql.c_str());

                if(rs && this->dbopen.GetNum(rs) == 0)
                {
                    sql = "insert into t_records(day,voltage) values(";
                    sql.append("\'2018-07-29\',\'");
                    sql.append(inmsg.c_str());
                    sql.append("\'");
                    sql.append(")");
                    _RecordsetPtr rs = this->dbopen.ExecuteWithResSQL(sql.c_str());
                    glog::trace("%s", sql.c_str());

                    if(rs)
                    {
                    }
                }
                else if(rs && this->dbopen.GetNum(rs) == 1)
                {
                    sql = "update t_records set voltage=\'";
                    sql.append(inmsg.c_str());
                    sql.append("\'");
                    sql.append(" where day=\'");
                    sql.append("2018-07-29");   //以后会变的
                    sql.append("\'");
                    _RecordsetPtr rs = this->dbopen.ExecuteWithResSQL(sql.c_str());
                    glog::trace("%s", sql.c_str());

                    if(rs)
                    {
                    }
                }
            }

            //三相电流
            if(DA[0] == 0 && DA[1] == 0 && DT[0] == 0x20 && DT[1] == 0x4)
            {
                BYTE* p1 = src;
                Json::Value jsonRoot;
                int p = 0;
                string strA;
                string strB;
                string strC;

                for(int i = 18; i < srclen - 2; i += 9)
                {
                    BYTE A1[3] = {0};
                    BYTE B1[3] = {0};
                    BYTE C1[3] = {0};
                    memcpy(A1, &src[i], 3);
                    memcpy(B1, &src[i + 3], 3);
                    memcpy(C1, &src[i + 6], 3);
                    char strA1[16] = {0};
                    char strB1[16] = {0};
                    char strC1[16] = {0};
                    BYTE b = A1[2] >> 4 & 0x0f;
                    BYTE s = A1[2] & 0x0f;
                    BYTE g = A1[1] >> 4 & 0x0f;
                    BYTE sfw = A1[1] & 0x0f;
                    BYTE bfw = A1[0] >> 4 & 0x0f;
                    BYTE qfw = A1[0] & 0x0f;
                    sprintf(strA1, "%d%d%d.%d%d%d", b, s, g, sfw, bfw, qfw);
                    b = B1[2] >> 4 & 0x0f;
                    s = B1[2] & 0x0f;
                    g = B1[1] >> 4 & 0x0f;
                    sfw = B1[1] & 0x0f;
                    bfw = B1[0] >> 4 & 0x0f;
                    qfw = B1[0] & 0x0f;
                    sprintf(strB1, "%d%d%d.%d%d%d", b, s, g, sfw, bfw, qfw);
                    b = C1[2] >> 4 & 0x0f;
                    s = C1[2] & 0x0f;
                    g = C1[1] >> 4 & 0x0f;
                    sfw = C1[1] & 0x0f;
                    bfw = C1[0] >> 4 & 0x0f;
                    qfw = C1[0] & 0x0f;
                    sprintf(strC1, "%d%d%d.%d%d%d", b, s, g, sfw, bfw, qfw);
                    strA.append(strA1);
                    strA.append("|");
                    strB.append(strB1);
                    strB.append("|");
                    strC.append(strC1);
                    strC.append("|");
                    p += 1;
                }

                jsonRoot["len"] = p;
                jsonRoot["A"] = strA;
                jsonRoot["B"] = strB;
                jsonRoot["C"] = strC;
                string inmsg = jsonRoot.toStyledString();
                string sql = "select * from t_records where 1=1 and CONVERT(Nvarchar, day, 23)=\'2017-07-29\'";
                _RecordsetPtr rs = this->dbopen.ExecuteWithResSQL(sql.c_str());

                if(rs && this->dbopen.GetNum(rs) == 0)
                {
                    sql = "insert into t_records(day,electric) values(";
                    sql.append("\'2018-07-29\',\'");
                    sql.append(inmsg.c_str());
                    sql.append("\'");
                    sql.append(")");
                    _RecordsetPtr rs = this->dbopen.ExecuteWithResSQL(sql.c_str());
                    glog::trace("%s", sql.c_str());

                    if(rs)
                    {
                    }
                }
                else if(rs && this->dbopen.GetNum(rs) == 1)
                {
                    sql = "update t_records set electric=\'";
                    sql.append(inmsg.c_str());
                    sql.append("\' ");
                    sql.append(" where day=\'");
                    sql.append("2018-07-29");
                    sql.append("\'");
                    _RecordsetPtr rs = this->dbopen.ExecuteWithResSQL(sql.c_str());
                    glog::trace("%s", sql.c_str());

                    if(rs)
                    {
                    }
                }
            }

            //三相有功功率
            if(DA[0] == 0 && DA[1] == 0 && DT[0] == 0x01 && DT[1] == 0x03)
            {
                BYTE* p1 = src;
                Json::Value jsonRoot;
                int p = 0;
                string strA;
                string strB;
                string strC;

                for(int i = 18; i < srclen - 2; i += 9)
                {
                    BYTE A1[3] = {0};
                    BYTE B1[3] = {0};
                    BYTE C1[3] = {0};
                    memcpy(A1, &src[i], 3);
                    memcpy(B1, &src[i + 3], 3);
                    memcpy(C1, &src[i + 6], 3);
                    char strA1[16] = {0};
                    char strB1[16] = {0};
                    char strC1[16] = {0};
                    BYTE s = A1[2] >> 4 & 0x0f;
                    BYTE g = A1[2] & 0x0f;
                    BYTE sfw = A1[1] >> 4 & 0x0f;
                    BYTE bfw = A1[1] & 0x0f;
                    BYTE qfw = A1[0] >> 4 & 0x0f;
                    BYTE wfw = A1[0] & 0x0f;
                    sprintf(strA1, "%d%d.%d%d%d%d", s, g, sfw, bfw, qfw, wfw);
                    s = B1[2] >> 4 & 0x0f;
                    g = B1[2] & 0x0f;
                    sfw = B1[1] >> 4 & 0x0f;
                    bfw = B1[1] & 0x0f;
                    qfw = B1[0] >> 4 & 0x0f;
                    wfw = B1[0] & 0x0f;
                    sprintf(strB1, "%d%d.%d%d%d%d", s, g, sfw, bfw, qfw, wfw);
                    s = C1[2] >> 4 & 0x0f;
                    g = C1[2] & 0x0f;
                    sfw = C1[1] >> 4 & 0x0f;
                    bfw = C1[1] & 0x0f;
                    qfw = C1[0] >> 4 & 0x0f;
                    wfw = C1[0] & 0x0f;
                    sprintf(strC1, "%d%d.%d%d%d%d", s, g, sfw, bfw, qfw, wfw);
                    strA.append(strA1);
                    strA.append("|");
                    strB.append(strB1);
                    strB.append("|");
                    strC.append(strC1);
                    strC.append("|");
                    p += 1;
                }

                jsonRoot["len"] = p;
                jsonRoot["A"] = strA;
                jsonRoot["B"] = strB;
                jsonRoot["C"] = strC;
                string inmsg = jsonRoot.toStyledString();
                string sql = "select * from t_records where 1=1 and CONVERT(Nvarchar, day, 23)=\'2017-07-29\'";
                _RecordsetPtr rs = this->dbopen.ExecuteWithResSQL(sql.c_str());

                if(rs && this->dbopen.GetNum(rs) == 0)
                {
                    sql = "insert into t_records(day,activepower) values(";
                    sql.append("\'2018-07-29\',\'");
                    sql.append(inmsg.c_str());
                    sql.append("\'");
                    sql.append(")");
                    _RecordsetPtr rs = this->dbopen.ExecuteWithResSQL(sql.c_str());
                    glog::trace("%s", sql.c_str());

                    if(rs)
                    {
                    }
                }
                else if(rs && this->dbopen.GetNum(rs) == 1)
                {
                    sql = "update t_records set activepower=\'";
                    sql.append(inmsg.c_str());
                    sql.append("\' ");
                    sql.append(" where day=\'");
                    sql.append("2018-07-29");
                    sql.append("\'");
                    _RecordsetPtr rs = this->dbopen.ExecuteWithResSQL(sql.c_str());
                    glog::trace("%s", sql.c_str());

                    if(rs)
                    {
                    }
                }
            }

            //三相功率因数
            if(DA[0] == 0 && DA[1] == 0 && DT[0] == 0x40 && DT[1] == 0x03)
            {
                BYTE* p1 = src;
                Json::Value jsonRoot;
                int p = 0;
                string strA;
                string strB;
                string strC;
                string strD;

                for(int i = 18; i < srclen - 2; i += 8)
                {
                    BYTE A1[2] = {0};
                    BYTE B1[2] = {0};
                    BYTE C1[2] = {0};
                    BYTE D1[2] = {0};
                    memcpy(A1, &src[i], 2);
                    memcpy(B1, &src[i + 2], 2);
                    memcpy(C1, &src[i + 4], 2);
                    memcpy(D1, &src[i + 6], 2);
                    char strA1[16] = {0};
                    char strB1[16] = {0};
                    char strC1[16] = {0};
                    char strD1[16] = {0};
                    BYTE g = A1[1] >> 4 & 0x0f;
                    BYTE sfw = A1[1] & 0x0f;
                    BYTE bfw = A1[0] >> 4 & 0x0f;
                    BYTE qfw = A1[0] & 0x0f;
                    sprintf(strA1, "%d.%d%d%d", g, sfw, bfw, qfw);
                    g = B1[1] >> 4 & 0x0f;
                    sfw = B1[1] & 0x0f;
                    bfw = B1[0] >> 4 & 0x0f;
                    qfw = B1[0] & 0x0f;
                    sprintf(strB1, "%d.%d%d%d", g, sfw, bfw, qfw);
                    g = C1[1] >> 4 & 0x0f;
                    sfw = C1[1] & 0x0f;
                    bfw = C1[0] >> 4 & 0x0f;
                    qfw = C1[0] & 0x0f;
                    sprintf(strC1, "%d.%d%d%d", g, sfw, bfw, qfw);
                    g = D1[1] >> 4 & 0x0f;
                    sfw = D1[1] & 0x0f;
                    bfw = D1[0] >> 4 & 0x0f;
                    qfw = D1[0] & 0x0f;
                    sprintf(strD1, "%d.%d%d%d", g, sfw, bfw, qfw);
                    strA.append(strA1);
                    strA.append("|");
                    strB.append(strB1);
                    strB.append("|");
                    strC.append(strC1);
                    strC.append("|");
                    strD.append(strD1);
                    strD.append("|");
                    p += 1;
                }

                jsonRoot["len"] = p;
                jsonRoot["A"] = strA;
                jsonRoot["B"] = strB;
                jsonRoot["C"] = strC;
                jsonRoot["D"] = strD;
                string inmsg = jsonRoot.toStyledString();
                string sql = "select * from t_records where 1=1 and CONVERT(Nvarchar, day, 23)=\'2017-07-29\'";
                _RecordsetPtr rs = this->dbopen.ExecuteWithResSQL(sql.c_str());

                if(rs && this->dbopen.GetNum(rs) == 0)
                {
                    sql = "insert into t_records(day,powerfactor) values(";
                    sql.append("\'2018-07-29\',\'");
                    sql.append(inmsg.c_str());
                    sql.append("\'");
                    sql.append(")");
                    _RecordsetPtr rs = this->dbopen.ExecuteWithResSQL(sql.c_str());
                    glog::trace("%s", sql.c_str());

                    if(rs)
                    {
                    }
                }
                else if(rs && this->dbopen.GetNum(rs) == 1)
                {
                    sql = "update t_records set powerfactor=\'";
                    sql.append(inmsg.c_str());
                    sql.append("\' ");
                    sql.append(" where day=\'");
                    sql.append("2018-07-29");
                    sql.append("\'");
                    _RecordsetPtr rs = this->dbopen.ExecuteWithResSQL(sql.c_str());
                    glog::trace("%s", sql.c_str());

                    if(rs)
                    {
                    }
                }
            }

            //正向有功电能量
            if(DA[0] == 0 && DA[1] == 0 && DT[0] == 0x01 && DT[1] == 0x05)
            {
                BYTE* p1 = src;
                Json::Value jsonRoot;
                int p = 0;
                string strA;

                for(int i = 18; i < srclen - 2; i += 4)
                {
                    BYTE A1[4] = {0};
                    memcpy(A1, &src[i], 4);
                    char strA1[16] = {0};
                    BYTE sw = A1[3] >> 4 & 0x0f;
                    BYTE w = A1[3]  & 0x0f;
                    BYTE q = A1[2] >> 4 & 0x0f;
                    BYTE b = A1[2]  & 0x0f;
                    BYTE s = A1[1] >> 4 & 0x0f;
                    BYTE g = A1[1] & 0x0f;
                    BYTE sfw = A1[0] >> 4 & 0x0f;
                    BYTE bfw = A1[0] & 0x0f;
                    sprintf(strA1, "%d%d%d%d%d%d.%d%d", sw, w, q, b, s, g, sfw, bfw);
                    strA.append(strA1);
                    strA.append("|");
                    p += 1;
                }

                jsonRoot["len"] = p;
                jsonRoot["A"] = strA;
                string inmsg = jsonRoot.toStyledString();
                string sql = "select * from t_records where 1=1 and CONVERT(Nvarchar, day, 23)=\'2018-07-29\'";
                _RecordsetPtr rs = this->dbopen.ExecuteWithResSQL(sql.c_str());

                if(rs && this->dbopen.GetNum(rs) == 0)
                {
                    sql = "insert into t_records(day,power) values(";
                    sql.append("\'2018-07-29\',\'");
                    sql.append(inmsg.c_str());
                    sql.append("\'");
                    sql.append(")");
                    _RecordsetPtr rs = this->dbopen.ExecuteWithResSQL(sql.c_str());
                    glog::trace("%s", sql.c_str());

                    if(rs)
                    {
                    }
                }
                else if(rs && this->dbopen.GetNum(rs) == 1)
                {
                    sql = "update t_records set power=\'";
                    sql.append(inmsg.c_str());
                    sql.append("\' ");
                    sql.append(" where day=\'");
                    sql.append("2018-07-29");
                    sql.append("\'");
                    _RecordsetPtr rs = this->dbopen.ExecuteWithResSQL(sql.c_str());
                    glog::trace("%s", sql.c_str());

                    if(rs)
                    {
                    }
                }
            }
        }
    }
}
void CIOCP::buildConCode(BYTE src[], BYTE res[], int& len, BYTE bcon)
{
    BYTE frame = src[13] & 0xf;   //帧序号
    BYTE btemp[216] = {0};
    btemp[0] = 0x68;
    int nbyte1 = 20 - 2 - 6;     //20自定义长度
    short n111 = (nbyte1 << 2) | 2;
    memcpy(&btemp[1], &n111, 2);
    memcpy(&btemp[3], &n111, 2);
    btemp[5] = 0x68;
    btemp[6] = 0x04;                //控制域 0000 0100   0100下行,从动      04是发送回去无需应答
    memcpy(&btemp[7], &src[7], 5);  //地址域
    btemp[12] = 0x00;
    btemp[13] = frame;
    btemp[19] = 0x16;
    memcpy(&btemp[14], &src[14], 4);
    BYTE  checksum = 0;

    for(int j = 6; j < 18; j++)
    {
        checksum += btemp[j];
    }

    btemp[18] = checksum;
    memcpy(res, btemp, 20);
    len = 20;
}