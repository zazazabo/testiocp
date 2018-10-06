#include "IOCP.h"
#include "SHA1.h"
#pragma comment(lib, "ws2_32.lib")

/////////////////////////////////////////////////////////////////////////////////////////////

CIOCP::CIOCP(string skin)
{
    // m_mcontralcenter.clear();
    m_strskin = skin;
    ::InitializeCriticalSection(&crtc_sec);
}



//���캯��

CIOCP::~CIOCP()
{
    Close1();
}








void dealiocp(LPVOID lp)
{
    CIOCP* io = (CIOCP*)lp;

    if(io->InitAll() == FALSE)
    {
        return;
    }

    io->MainLoop();
}



void CIOCP::Notify(TNotifyUI& msg)
{
    if(msg.sType == _T("windowinit"))
        OnPrepare(msg);
    else if(_tcsicmp(msg.sType, "itemactivate") == 0)
    {
        string pSenderName = msg.pSender->GetName();
        string pClassName = msg.pSender->GetClass();

        if(pClassName == "ListTextElementUI")
        {
            CListTextElementUI* p1 = (CListTextElementUI*)msg.pSender;
            string str = p1->GetText(5);
            gstring::copyToclip(str.c_str(), str.size());
            p1->SetText(5, "");
            //    string val = list1.getCellText(nnn, 4);
            //    gstring::copyToclip(val.c_str(), val.size());
            //    list1.setItemText("", nnn, 4);
            //p1->GetText()
        }

        //MessageBoxA(m_hWnd, 0, 0, 0);
    }
    else if(msg.sType == _T("click"))
    {
        if(msg.pSender->GetName() == "closebtn")
        {
            //SendMessageA(WM_SYSCOMMAND, SC_CLOSE, 0);
            //SendMessageA(WM_SYSCOMMAND, SC_CLOSE, 0);
            Close();
        }
        else if(msg.pSender->GetName() == "minbtn")
        {
            SendMessageA(WM_SYSCOMMAND, SC_MINIMIZE, 0);
        }
        else if(msg.pSender->GetName() == "maxbtn")
        {
            SendMessageA(WM_SYSCOMMAND, SC_MAXIMIZE, 0);
        }
        else if(msg.pSender->GetName() == "closesocket")
        {
            int n = m_plistuser->GetCurSel();

            if(n == -1)
            {
                gstring::tip("�빴ѡ����");
                return;
            }

            CListTextElementUI* pText = (CListTextElementUI*) m_plistuser->GetItemAt(n);
            string lpio = pText->GetText(2);
            DWORD lp = strtol(lpio.c_str(), NULL, 16);
            IOCP_IO_PTR pIo = (IOCP_IO_PTR)lp;
            closesocket(pIo->socket);
        }
        else if(msg.pSender->GetName() == "gaywaylist")
        {
            int         op, op_len, nRet;
            IOCP_IO_PTR lp_start = NULL;
            IO_POS      pos;
            lp_start =  m_io_group.GetHeadPosition(pos);

            if(lp_start == NULL)
            {
                PostLog("����˻�ĩ����");
                return;
            }

            //glog::trace("come on CheckForInvalidConnection");
            while(lp_start != NULL)
            {
                if(lp_start->fromtype == SOCKET_FROM_Concentrator)
                {
                    op_len = sizeof(op);
                    nRet = getsockopt(lp_start->socket, SOL_SOCKET, SO_CONNECT_TIME, (char*)&op, &op_len);
                    int len = 0;

                    if(op != 0xffffffff)
                    {
                        len = op - lp_start->timelen;
                    }

                    PostLog("����:%s ���߼���յ�����Ϣ:%d�� ͨ��ָ��:%p", lp_start->gayway, len, lp_start);
                }

                lp_start = m_io_group.GetNext(pos);
            }

            PostLog("��������:%d", m_mcontralcenter.size());
        }
        else if(msg.pSender->GetName() == "weblist")
        {
            string  str = "aaaa";
            _RecordsetPtr rs = dbopen->ExecuteWithResSQL(str.c_str());
        }
        else if(msg.pSender->GetName() == "start")
        {
            DWORD tid = 0;
            HANDLE h1 = CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)dealiocp, this, NULL, &tid);
            CloseHandle(h1);
            //PostLog("dddd");
        }
        else if(msg.pSender->GetName() == "clear")
        {
            m_pRishLog->SetText("");
        }
        else if(msg.pSender->GetName() == "senddata")
        {
            int n = m_plistuser->GetCurSel();
            string lpiostr =   getItemText(m_plistuser, m_plistuser->GetCurSel(), 2);
            string lpkeystr =   getItemText(m_plistuser, m_plistuser->GetCurSel(), 3);
            string data =   getItemText(m_plistuser, m_plistuser->GetCurSel(), 4);
            ULONG_PTR io = (ULONG_PTR)strtol(lpiostr.c_str(), NULL, 16);
            ULONG_PTR ik = (ULONG_PTR)strtol(lpiostr.c_str(), NULL, 16);

            if(io == 0 || ik == 0)
            {
                PostLog("��ѡ���б�");
                return;
            }

            SendData(io, ik, data);
        }
        else if(msg.pSender->GetName() == "setdata")
        {
            string data = m_pData->GetText();
            setItemText(m_plistuser, m_plistuser->GetCurSel(), 4, data);
        }
        else if(msg.pSender->GetName() == "deletebtn")
        {
            CControlUI* p1 = (CControlUI*) strtol(m_pData->GetText(), NULL, 16);
            int iIndex = m_plistuser->GetItemIndex(p1);

            if(iIndex != -1)
            {
                m_plistuser->Remove(p1);
            }
        }
    }
}

//��������
/*-------------------------------------------------------------------------------------------
�������ܣ��رղ������Դ
����˵����
�������أ�
-------------------------------------------------------------------------------------------*/
void CIOCP::Close1()
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
//
/*-------------------------------------------------------------------------------------------
    �������ܣ���ʼ��IO���
    ����˵����
    �������أ�
    -------------------------------------------------------------------------------------------*/
void CIOCP::InitIoContext(IOCP_IO_PTR lp_io)
{
    memset(&lp_io->ol,  0, sizeof(WSAOVERLAPPED));
    memset(&lp_io->buf, 0, BUFFER_SIZE);
    lp_io->wsaBuf.buf       = lp_io->buf;
    lp_io->wsaBuf.len       = BUFFER_SIZE;
}

/*-------------------------------------------------------------------------------------------
    �������ܣ���ʼ������SOCKET�˿ڣ�������ɶ˿�����������
    ����˵����
    �������أ��ɹ���TRUE��ʧ�ܣ�FALSE
    -------------------------------------------------------------------------------------------*/
BOOL CIOCP::InitSocket()
{
    m_listen_socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);

    if(INVALID_SOCKET == m_listen_socket)
    {
        glog::traceErrorInfo("call WSASocket", WSAGetLastError());
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
    �������ܣ��ر������߳�
    ����˵����
    �������أ�
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
�������ܣ��������˿ں��Լ���IP��PORT�󶨣�����ʼ����
����˵����
�������أ��ɹ���TRUE��ʧ�ܣ�FALSE
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
        glog::traceErrorInfo("call bind()", WSAGetLastError());
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
�������ܣ�����CPU����Ŀ��������Ӧ���������ݴ����߳�
����˵����
�������أ��ɹ���TRUE��ʧ�ܣ�FALSE
-------------------------------------------------------------------------------------------*/
BOOL CIOCP::StartThread()
{
    int i;
    SYSTEM_INFO sys_info;
    GetSystemInfo(&sys_info);
    m_n_thread_count = sys_info.dwNumberOfProcessors > MAXTHREAD_COUNT ? MAXTHREAD_COUNT : sys_info.dwNumberOfProcessors;
    //m_n_thread_count = 1;

    for(i = 0; i < m_n_thread_count; i++)
    {
        DWORD tid = 0;
        m_h_thread[i] = CreateThread(NULL, 0, CompletionRoutine, (LPVOID)this, 0, &tid);
        glog::GetInstance()->AddLine("i:%d ThreadId:%d", i, tid);

        if(NULL == m_h_thread[i])
        {
            CloseThreadHandle(i);
            CloseHandle(m_h_iocp);
            return FALSE;
        }

        //PostLog("start %d thread:%d", i + 1, tid);
    }

    return TRUE;
}


/*-------------------------------------------------------------------------------------------
�������ܣ�����һ������������
����˵����
�������أ��ɹ���TRUE��ʧ�ܣ�FALSE
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
        lp_io->timelen = 0;
        memset(lp_io->day, 0, 20);
        memset(lp_io->gayway, 0, 20);
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
�������ܣ��������ݺ���
����˵����
�������أ��ɹ���TRUE��ʧ�ܣ�FALSE
-------------------------------------------------------------------------------------------*/
BOOL CIOCP::HandleData(IOCP_IO_PTR lp_io, int nFlags, IOCP_KEY_PTR lp_key)
{
    switch(nFlags)
    {
        case IOCP_COMPLETE_ACCEPT:
            {
                char szPeerAddress[50];
                SOCKADDR_IN *addrClient = NULL, *addrLocal = NULL;
                char ip[50] = {0};
                int nClientLen = sizeof(SOCKADDR_IN), nLocalLen = sizeof(SOCKADDR_IN);
                lpGetAcceptExSockaddrs(lp_io->buf, 0, sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, (LPSOCKADDR*)&addrLocal, &nLocalLen, (LPSOCKADDR*)&addrClient, &nClientLen);
                char* ip1 = inet_ntoa(addrClient->sin_addr);
                sprintf(szPeerAddress, "%s:%d", ip1, addrClient->sin_port);
                CListTextElementUI* pListElement = new CListTextElementUI;
                m_plistuser->Add(pListElement);
                lp_io->pUserData = pListElement;
                m_pData->SetText(gstring::int2str((int)pListElement, 16).c_str());
                char vvv[20] = {0};
                int n = m_plistuser->GetCount();
                sprintf(vvv, "%d", n);
                pListElement->SetText(0, vvv);
                pListElement->SetText(1, szPeerAddress);
                sprintf(vvv, "%p", lp_io);
                pListElement->SetText(2, vvv);
                sprintf(vvv, "%p", lp_key);
                pListElement->SetText(3, vvv);
                //sprintf(vvv,"%p",lp_key);
                //pListElement->SetText(4,vvv);
                //InitIoContext(lp_io);
                //SOCKADDR_IN addr_conn = {0};
                //int nSize = sizeof(addr_conn);
                //int nret =  getsockname(lp_key->socket, (SOCKADDR *)&addr_conn, &nSize);
                //m_listctr->insertItemText(szPeerAddress, 0, m_listctr->getRowCount());
                //char socketstr[50] = {0};
                //sprintf(socketstr, "%p", lp_io);
                //m_listctr->setItemText(socketstr, m_listctr->getRowCount() - 1, 1);
                //char socketkey[50] = {0};
                //sprintf(socketkey, "%p", lp_key);
                //m_listctr->setItemText(socketkey, m_listctr->getRowCount() - 1, 3);
                glog::GetInstance()->AddLine("�ͻ�������:%s lp_io:%p     lp_key:%p", szPeerAddress, lp_io, lp_key);
                PostLog("�ͻ�������:%s lp_io:%p     lp_key:%p", szPeerAddress, lp_io, lp_key);
                lp_io->operation    = IOCP_READ;
            }
            break;

        case IOCP_COMPLETE_ACCEPT_READ:
            {
                lp_io->operation    = IOCP_WRITE;
                GetAddrAndPort(lp_io->wsaBuf.buf, szAddress, uPort);
                MSG(lp_io->wsaBuf.len);
                memset(&lp_io->ol, 0, sizeof(lp_io->ol));
            }
            break;

        case IOCP_COMPLETE_READ:
            {
                ////cout<<"read a data!"<<lp_io->buf<<endl;
                //printf("read a data! socket:%d \n", lp_io->socket);
                //// lp_io->operation    = IOCP_WRITE;
                //memset(&lp_io->ol, 0, sizeof(lp_io->ol));
            }
            break;

        case IOCP_COMPLETE_WRITE:
            {
                //PostLog("write a data!");
                lp_io->operation    = IOCP_READ;
                InitIoContext(lp_io);
            }
            break;

        default:
            {
                glog::trace("handleData do nothing!");
                return FALSE;
            }
    }

    return TRUE;
}


///*-------------------------------------------------------------------------------------------
/*�������ܣ�����һЩ�ص�����
����˵����
�������أ��ɹ���TRUE��ʧ�ܣ�FALSE
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
                nRet = WSASend(lp_io->socket, &lp_io->wsaBuf, 1, &dwBytes, 0, &lp_io->ol, NULL);

                if((nRet == SOCKET_ERROR) && (WSAGetLastError() != WSA_IO_PENDING))
                {
                    closesocket(lp_io->socket);
                    m_io_group.RemoveAt(lp_io);
                    m_key_group.RemoveAt(lp_key);
                    return FALSE;
                }
            }
            break;

        case IOCP_READ:
            {
                dwFlags = 0;
                nRet = WSARecv(lp_io->socket, &lp_io->wsaBuf, 1, &dwBytes, &dwFlags, &lp_io->ol, NULL);

                // Sleep(1000);

                if((nRet == SOCKET_ERROR) && (WSAGetLastError() != WSA_IO_PENDING))
                {
                    closesocket(lp_io->socket);
                    m_io_group.RemoveAt(lp_io);
                    m_key_group.RemoveAt(lp_key);
                    return FALSE;
                }
            }
            break;

        case IOCP_END:
            {
                ExitSocket(lp_io, lp_key, GetLastError());
                //PostLog(" DataAction->IOCP_END  �ر�socket:%p   ", lp_io);
                //closesocket(lp_io->socket);
                //m_io_group.RemoveAt(lp_io);
                //m_key_group.RemoveAt(lp_key);
                //int n = m_io_group.GetCount();
                //int n1 = m_io_group.GetBlankCount();
                //PostLog("IOCP_END lp_io:%p  \nlist1 count:%d list0 count:%d from:%d", lp_io, n, n1, lp_io->fromtype);
            }
            break;

        default:
            {
                PostLog("DataAction do nothing!------------------------------------------");
                return FALSE;
            }
    }

    return TRUE;
}

/*-------------------------------------------------------------------------------------------
�������ܣ��õ�MS��װ��SOCKET����ָ�룬������������ٶ�
����˵����
�������أ��ɹ���TRUE��ʧ�ܣ�FALSE
-------------------------------------------------------------------------------------------*/
BOOL CIOCP::GetFunPointer()
{
    DWORD dwRet, nRet;
    nRet = WSAIoctl(m_listen_socket, SIO_GET_EXTENSION_FUNCTION_POINTER,
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

    nRet = WSAIoctl(m_listen_socket, SIO_GET_EXTENSION_FUNCTION_POINTER,
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
�������ܣ�ע��FD_ACCEPTG�¼���m_h_accept_event�¼����Ա����з���ȥ�����Ӻĺľ�ʱ���õ�֪ͨ��
����˵����
�������أ��ɹ���TRUE��ʧ�ܣ�FALSE
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
�������ܣ��õ����������Ŀͻ���IP��PORT
����˵����
�������أ��ɹ���TRUE��ʧ�ܣ�FALSE
-------------------------------------------------------------------------------------------*/
BOOL CIOCP::GetAddrAndPort(char*buf, char ip[], UINT & port)
{
    int     len = BUFFER_SIZE - sizeof(SOCKADDR_IN) - 16;
    char    *lp_buf = buf + len;    //ֱ�Ӷ�ȡԶ�˵�ַ
    SOCKADDR_IN addr;
    memcpy(&addr, lp_buf, sizeof(SOCKADDR_IN));
    port    = ntohl(addr.sin_port);
    strcpy(ip, inet_ntoa(addr.sin_addr));
    MSG("�ͻ�IPΪ��");
    MSG(ip);
    MSG("�ͻ��˿�Ϊ��");
    MSG(port);
    return TRUE;
}

void CIOCP::DealWebsockMsg(IOCP_IO_PTR& lp_io, IOCP_KEY_PTR& lp_key, BYTE msg[], int len, BOOL bAnswer)
{
    string  strret = "";
    BOOL bFullPack = TRUE;
    int lenread = wsDecodeFrame(lp_io->buf, strret, len, bFullPack);
    //glog::trace("\n%s", strret.c_str());
    PostLog("web������:%s", strret.c_str());

    if(lenread == WS_CLOSING_FRAME)
    {
        PostLog("web���˳� ͨ��ָ��:%p", lp_io);
        lp_io->operation = IOCP_END;
        //ExitSocket(lp_io,lp_key);
        //  string towrite = "";
        //  EnterCriticalSection(&crtc_sec);
        //  //ɾ����Ϣ����
        //map<string,list<MSGPACK>>::iterator itemsg=m_MsgPack.begin();
        //while (itemsg!=m_MsgPack.end())
        //{
        // list<MSGPACK>l_msg=itemsg->second;
        // list<MSGPACK>::iterator it=l_msg.begin();
        // while (it!=l_msg.end())
        // {
        //  it++;
        // }
        //
        //}
        //  list<MSGPACK>::iterator it;
        //  for(it = m_listmsg.begin(); it != m_listmsg.end();)
        //    {
        //      MSGPACK msg = *it;
        //      if(msg.lp_io == lp_io)
        //        {
        //          it = m_listmsg.erase(it);
        //        }
        //      else
        //        {
        //          it++;
        //        }
        //    }
        //  DeleteByIo((ULONG_PTR)lp_io->pUserData);
        //LeaveCriticalSection(&crtc_sec);
        //lp_io->operation = IOCP_END;
    }

    if(lenread == WS_ERROR_FRAME)
    {
        PostLog("web�˴���֡");
        lp_io->operation = IOCP_END;
    }

    if(lenread != WS_ERROR_FRAME && lenread != WS_CLOSING_FRAME)
    {
        Json::Value root;
        Json::Reader reader;

        if(reader.parse(strret.c_str(), root))
        {
            Json::Value msgType = root["msg"];
            Json::Value isres = root["res"];

            if(msgType.isString())
            {
                Json::Value tosend = root["data"];

                if(tosend.isString() && tosend != "")
                {
                    if(msgType == "AA" || msgType == "A4" || msgType == "A5" || msgType == "AC" || msgType == "00" || msgType == "FE")
                    {
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
                                BYTE seq = 0;
                                map<string, list<MSGPACK>>::iterator itmsg = m_MsgPack.find(addrarea);

                                if(itmsg != m_MsgPack.end())
                                {
                                    list<MSGPACK>v_msg = itmsg->second;
                                    //if (v_msg.begin()==v_msg.end())
                                    //{
                                    //}
                                    seq = v_msg.begin() == v_msg.end() ? 0 : v_msg.back().seq + 1; //v_msg[v_msg.size() - 1].seq;
                                }
                                else
                                {
                                    seq = 0;
                                }

                                //                        BYTE seq = m_listmsg.size() == 0 ? 0 : m_listmsg.back().seq+1;
                                //BYTE seq = m_MsgPack.size() == 0 ? 0 : m_listmsg.back().seq+1;
                                seq = seq > 0xf ? 0 : seq;

                                if(msgType != "00")
                                {
                                    _MSGPACK msg = {0};
                                    // strcpy(msg.comaddr, addrarea.c_str());
                                    msg.lp_io = lp_io;
                                    msg.seq = seq;

                                    if(itmsg == m_MsgPack.end())
                                    {
                                        list<_MSGPACK>v_msgpack;
                                        m_MsgPack.insert(pair<string, list<MSGPACK>>(addrarea, v_msgpack));
                                    }

                                    itmsg = m_MsgPack.find(addrarea);
                                    itmsg->second.push_back(msg);
                                    int ncount = itmsg->second.size();
                                    PostLog("����[%s] ��Ϣ����[%d] ĩ֡����[%d]", itmsg->first.c_str(), itmsg->second.size(), seq);
                                    //vector<_MSGPACK>v_vmsg;
                                    //v_vmsg.push_back(msg);
                                    //m_listmsg.push_back(msg);
                                    bitSend[13] =   bitSend[13] & 0xf0 | seq;
                                    bitSend[len - 2] = bitSend[len - 2] + seq;
                                    //m_listmsg.back();
                                }

                                string tosenddata = gstring::char2hex((const char*)bitSend, len);
                                PostLog("ת����:%s ����:%d", tosenddata.c_str(), tosenddata.size() / 2);
                                IOCP_IO_PTR lp_io1 = ite->second;
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
    }
}

BOOL CIOCP::IsBreakPack(BYTE src[], int len)
{
    if(len < 6)
    {
        return FALSE;
    }

    if(src[0] == 0x68)
    {
        int aa = 44;
    }

    SHORT len1 = *(SHORT*)&src[1];
    SHORT len2 = *(SHORT*)&src[3];

    if(src[0] == 0x68 && len1 == len2 && src[5] == 0x68)
    {
        BOOL bAllpack =  checkFlag(src, len);

        if(bAllpack == FALSE)
        {
            return TRUE;
        }
    }

    return FALSE;
}



BOOL CIOCP::CloseMySocket(IOCP_IO_PTR lp_io)
{
    closesocket(lp_io->socket);
    m_io_group.RemoveAt(lp_io);
    m_key_group.RemoveAt(lp_io->lp_key);
    int n = m_io_group.GetCount();
    int n1 = m_io_group.GetBlankCount();
    glog::trace("\n CloseMySocket  lp_io:%p  list1 count:%d list0 count:%d from:%d", lp_io, n, n1, lp_io->fromtype);
    return TRUE;
}

DWORD CIOCP::TimeThread(LPVOID lp_param)
{
    CIOCP*          lp_this         = (CIOCP*)lp_param;
    string strtime = lp_this->m_configTime;
    vector<string>v_str;
    gstring::split(strtime, v_str, ":");
    int h = atoi(v_str[0].c_str());
    int m = atoi(v_str[1].c_str());
    int allm = h * 60 + m;

//  vector<string>v_str;
//  string strtime=m_configTime;
//  gstring::split(v_str,m_configTime,":");

    while(TRUE)
    {
        time_t tmtamp;
        struct tm *tm1 = NULL;
        time(&tmtamp) ;
        tm1 = localtime(&tmtamp) ;
        int allm1 = tm1->tm_hour * 60 + tm1->tm_min;
        int difftime1 = allm1 - allm;
        tm1->tm_mday--;
        mktime(tm1);
        char myday[30] = {0};
        strftime(myday, sizeof(myday), "%Y-%m-%d", tm1);
        map<string, IOCP_IO_PTR>::iterator  it1 = lp_this->m_mcontralcenter.begin();
        int len1 = lp_this->m_mcontralcenter.size();

        while(it1 != lp_this->m_mcontralcenter.end())
        {
            // glog::trace("\n����:%s", it1->first.c_str());
            //lp_this->PostLog("�ɼ�������");
            IOCP_IO_PTR lo = it1->second;

            if(_stricmp(lo->day, myday) != 0)
            {
                //���������ѹ
                lp_this->PostLog("����:[%s] �������������ѹ����", it1->first.c_str());
                //unsigned char vol[24] = {0x68, 0x42, 0x00, 0x42, 0x00, 0x68, 0x04, comaddr[1], comaddr[0], comaddr[3], comaddr[2], 0x02, 0xAC, 0x7A, 0x00, 0x00, 0x04, 0x04, 0x00, 0x00, 0x01, 0x05, 0x55, 0x16};
                vector<BYTE>v_b;
                BYTE vol[50] = {0};
                int n = lp_this->buidByte(it1->first, 0x4, 0xAC, 0x71, 0, 0x404, v_b, vol);
                lp_this->InitIoContext(lo);
                memcpy(lo->buf, vol, n);
                lo->wsaBuf.len = n; // sizeof(vol);
                lo->wsaBuf.buf = lo->buf;
                lo->operation = IOCP_WRITE;
                lp_this->DataAction(lo, lo->lp_key);
                //�����������
                Sleep(10000);
                lp_this->PostLog("����:[%s]�������������������", it1->first.c_str());
                BYTE electric[50] = {0}; //{0x68, 0x32, 0x00, 0x32, 0x00, 0x68, 0x04, comaddr[1], comaddr[0], comaddr[3], comaddr[2], 0x02, 0xAC, 0x75, 0x00, 0x00, 0x20, 0x04, 0x66, 0x16 };
                n = lp_this->buidByte(it1->first, 0x4, 0xAC, 0x71, 0, 0x420, v_b, electric);
                string data1 = gstring::char2hex((const char*)electric, n);
                //glog::GetInstance()->AddLine("�������Ͱ�:%s", data1.c_str());
                lp_this->InitIoContext(lo);
                memcpy(lo->buf, electric, n);
                lo->wsaBuf.len = n;
                lo->wsaBuf.buf = lo->buf;
                lo->operation = IOCP_WRITE;
                lp_this->DataAction(lo, lo->lp_key);
                //���������й�����
                Sleep(10000);
                lp_this->PostLog("����:[%s]�������������й���������", it1->first.c_str());
                unsigned char activepower[50] = {0}; // {0x68, 0x42, 0x00, 0x42, 0x00, 0x68, 0x04, comaddr[1], comaddr[0], comaddr[3], comaddr[2], 0x02, 0xAC, 0x76, 0x00, 0x00, 0x01, 0x03, 0x00, 0x00, 0x20, 0x04, 0x6B, 0x16 };
                n = lp_this->buidByte(it1->first, 0x4, 0xAC, 0x71, 0, 0x301, v_b, activepower);
                lp_this->InitIoContext(lo);
                memcpy(lo->buf, activepower, n);
                lo->wsaBuf.len = n; //sizeof(activepower);
                lo->wsaBuf.buf = lo->buf;
                lo->operation = IOCP_WRITE;
                lp_this->DataAction(lo, lo->lp_key);
                ////�����ܹ�������
                Sleep(10000);
                lp_this->PostLog("����:[%s]�������칦������", it1->first.c_str());
                unsigned char powerfactor[50] = {0}; //{0x68, 0x42, 0x00, 0x42, 0x00, 0x68, 0x04, comaddr[1], comaddr[0], comaddr[3], comaddr[2], 0x02, 0xAC, 0x78, 0x00, 0x00, 0x40, 0x03, 0x00, 0x00, 0x20, 0x04, 0xAC, 0x16 };
                n = lp_this->buidByte(it1->first, 0x4, 0xAC, 0x71, 0, 0x340, v_b, powerfactor);
                lp_this->InitIoContext(lo);
                memcpy(lo->buf, powerfactor, n);
                lo->wsaBuf.len = n;
                lo->wsaBuf.buf = lo->buf;
                lo->operation = IOCP_WRITE;
                lp_this->DataAction(lo, lo->lp_key);
                //��������
                Sleep(10000);
                lp_this->PostLog("����:[%s]����������������", it1->first.c_str());
                unsigned char power[50] = {0};//{0x68, 0x32, 0x00, 0x32, 0x00, 0x68, 0x04,  comaddr[1], comaddr[0], comaddr[3], comaddr[2], 0x02, 0xAC, 0x7B, 0x00, 0x00, 0x01, 0x05, 0x4E, 0x16 };
                n = lp_this->buidByte(it1->first, 0x4, 0xAC, 0x71, 0, 0x501, v_b, power);
                lp_this->InitIoContext(lo);
                memcpy(lo->buf, power, n);
                lo->wsaBuf.len = n;
                lo->wsaBuf.buf = lo->buf;
                lo->operation = IOCP_WRITE;
                lp_this->DataAction(lo, lo->lp_key);
                strcpy(lo->day, myday);
            }

            it1++;
        }

        Sleep(10000);
    }

    return 1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
/*-------------------------------------------------------------------------------------------
�������ܣ���ʼ����ɶ˿ڼ���ص����ж�����������ÿһ��10������.
����˵����
�������أ��ɹ���TRUE��ʧ�ܣ�FALSE
-------------------------------------------------------------------------------------------*/
BOOL CIOCP::InitAll()
{
    CoInitialize(NULL);
    // objeamil.AddTargetEmail();
    //objeamil.SetEmailTitle(string("aaaa"));
    //objeamil.AddTargetEmail(string("277402131@qq.com"));
    //objeamil.SetContent(string("asdfsdfsdfsdfsdf"));
    //objeamil.SendVecotrEmail();
    //CSmtp smtp(25, "smtp.126.com","z277402131@126.com", /*��������ַ*/"z277402131",/*��������*/"zhizhuchun@qq.com",/*Ŀ�������ַ*/"TEST",/*����*/"���Բ��ԣ��յ���ظ���"  /*�ʼ�����*/);
    //string filePath("D:\\����.txt");
    //smtp.AddAttachment(filePath);
    /*�����Ե���CSmtp::DeleteAttachment����ɾ������������һЩ�������Լ���ͷ�ļ���!*/
    //int err;
    //if((err = smtp.SendEmail_Ex()) != 0) {
    //  if(err == 1)
    //      cout << "����1: �������粻��ͨ������ʧ��!" << endl;
    //  if(err == 2)
    //      cout << "����2: �û�������,��˶�!" << endl;
    //  if(err == 3)
    //      cout << "����3: �û����������˶�!" << endl;
    //  if(err == 4)
    //      cout << "����4: ���鸽��Ŀ¼�Ƿ���ȷ���Լ��ļ��Ƿ����!" << endl;
    //}
    //string strTarEmail = "12345678@qq.com";
    //smtp.AddTargetEmail(strTarEmail);
    //if((err = smtp.SendVecotrEmail()) != 0) {
    //    if(err == -1)
    //        cout << "����-1: û��Ŀ�������ַ!" << endl;
    //    if(err == 1)
    //        cout << "����1: �������粻��ͨ������ʧ��!" << endl;
    //    if(err == 2)
    //        cout << "����2: �û�������,��˶�!" << endl;
    //    if(err == 3)
    //        cout << "����3: �û����������˶�!" << endl;
    //    if(err == 4)
    //        cout << "����4: ���鸽��Ŀ¼�Ƿ���ȷ���Լ��ļ��Ƿ����!" << endl;
    //}
    WSAData data;

    if(WSAStartup(MAKEWORD(2, 2), &data) != 0)
    {
        cout << "WSAStartup fail!" << WSAGetLastError() << endl;
        return FALSE;
    }

    m_h_iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 0);

    if(NULL == m_h_iocp)
    {
        PostLog("CreateIoCompletionPort() failed: errcode:%d", GetLastError());
        return FALSE;
    }

    if(!StartThread())
    {
        PostLog("start thread fail! errcode:%d", GetLastError());
        PostQueuedCompletionStatus(m_h_iocp, 0, NULL, NULL);
        CloseHandle(m_h_iocp);
        return FALSE;
    }

    //��ʱ�ɼ��߳�
    DWORD tid = 0;
    HANDLE hTreadTime = CreateThread(NULL, NULL, TimeThread, (LPVOID)this, NULL, &tid);
    CloseHandle(hTreadTime);
    //��ʱ�ɼ��߳�
    DWORD tidEmail = 0;
    HANDLE hTreadEmail = CreateThread(NULL, NULL, TimeEmail, (LPVOID)this, NULL, &tidEmail);
    CloseHandle(hTreadEmail);

    if(!InitSocket())
    {
        PostQueuedCompletionStatus(m_h_iocp, 0, NULL, NULL);
        PostLog("Init sociket fail! errcode:%d", GetLastError());
        CloseHandle(m_h_iocp);
        return FALSE;
    }

    if(!BindAndListenSocket())
    {
        PostQueuedCompletionStatus(m_h_iocp, 0, NULL, NULL);
        PostLog("BindAndListenSocket sociket fail! errcode:%d", GetLastError());
        CloseHandle(m_h_iocp);
        closesocket(m_listen_socket);
        return FALSE;
    }

    if(!GetFunPointer())
    {
        PostLog("GetFunPointer sociket fail! errcode:%d", GetLastError());
        PostQueuedCompletionStatus(m_h_iocp, 0, NULL, NULL);
        CloseHandle(m_h_iocp);
        closesocket(m_listen_socket);
        return FALSE;
    }

    if(!PostAcceptEx())
    {
        PostQueuedCompletionStatus(m_h_iocp, 0, NULL, NULL);
        PostLog("PostAcceptEx sociket fail! errcode:%d", GetLastError());
        CloseHandle(m_h_iocp);
        closesocket(m_listen_socket);
        return FALSE;
    }

    if(!RegAcceptEvent())
    {
        PostQueuedCompletionStatus(m_h_iocp, 0, NULL, NULL);
        PostLog("RegAcceptEvent sociket fail! errcode:%d", GetLastError());
        CloseHandle(m_h_iocp);
        closesocket(m_listen_socket);
        return FALSE;
    }

    return TRUE;
}
/*-------------------------------------------------------------------------------------------
�������ܣ���ѭ��
����˵����
�������أ��ɹ���TRUE��ʧ�ܣ�FALSE
-------------------------------------------------------------------------------------------*/
BOOL CIOCP::MainLoop()
{
    DWORD   dwRet;
    int     nCount = 0;
    PostLog("���������...");
    //cout << "Server is running.........." << nCount++ << " times" << endl;
    int ii = 0;

    while(TRUE)
    {
        dwRet = WaitForSingleObject(m_h_accept_event, 10000);

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
                    //��⼯������ʱ����
                    //cout << "Server is running.........." << nCount++ << " times" << endl;
                    CheckForInvalidConnection();
                }
                break;

            case WAIT_OBJECT_0:   //���յ������з��������Ӷ��ù��˵���Ϣ���ٴη�������
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
�������ܣ������Ƿ��������ˣ����ܳ�ʱ��û�����ݵġ���Ч���ӡ����еĻ������ߵ�
����˵����
�������أ��ɹ���TRUE��ʧ�ܣ�FALSE
-------------------------------------------------------------------------------------------*/
void CIOCP::CheckForInvalidConnection()
{
    int         op, op_len, nRet;
    IOCP_IO_PTR lp_start = NULL;
    IO_POS      pos;
    lp_start =  m_io_group.GetHeadPosition(pos);
    //IOCP_IO_PTR lp_end =       m_io_group.GetEndPosition(pos);
    map<IOCP_IO_PTR, DWORD> m_io;
    m_io.clear();
    map<IOCP_IO_PTR, int> m_mio;
    m_mio.clear();

    //glog::trace("come on CheckForInvalidConnection");
    while(lp_start != NULL)
    {
        op_len = sizeof(op);
        nRet = getsockopt(lp_start->socket, SOL_SOCKET, SO_CONNECT_TIME, (char*)&op, &op_len);

        if(SOCKET_ERROR == nRet)
        {
            glog::traceErrorInfo("CheckForInvalidConnection getsockopt", WSAGetLastError());
            lp_start = m_io_group.GetNext(pos);
            continue;
        }

        if(lp_start->fromtype == SOCKET_FROM_Concentrator)
        {
            if(op != 0xffffffff)
            {
                int len = op - lp_start->timelen;

                if(len / 60 >= 2)
                {
                    // map<string, IOCP_IO_PTR>::iterator  it; m_mcontralcenter.find(lp_start)
                    glog::GetInstance()->AddLine("ͨ��ָ��:%p ����:%s ��ʱ%d�� �����ر� ��������:%d", lp_start, lp_start->gayway, len, m_io_group.GetCount());
                    closesocket(lp_start->socket);
                    string sql = "update t_baseinfo set online=0 where comaddr=\'";
                    sql.append(lp_start->gayway);
                    sql.append("\'");
                    glog::trace("\n%s", sql.c_str());
                    _RecordsetPtr rs =   dbopen->ExecuteWithResSQL(sql.c_str());
                    map<string, IOCP_IO_PTR>::iterator  it;

                    for(it = m_mcontralcenter.begin(); it != m_mcontralcenter.end();)
                    {
                        if(it->second == lp_start)
                        {
                            m_mcontralcenter.erase(it++);   //erase ɾ����ָ����һ��������
                        }
                        else
                        {
                            it++;
                        }
                    }

                    break;
                }
            }
        }

        if(lp_start->state == SOCKET_STATE_CONNECT || lp_start->state == SOCKET_STATE_CONNECT_AND_READ)
        {
            if(lp_start->fromtype == SOCKET_FROM_WEBSOCKET)
            {
                int len = op - lp_start->timelen;

                if(len / 60 >= 20)
                {
                    glog::GetInstance()->AddLine("�����ر���ҳ�ͻ���");
                    closesocket(lp_start->socket);
                    break;
                }

                //��ҳ20���������ɵ�
            }
            else if(lp_start->fromtype == SOCKET_FROM_UNKNOW)
            {
                int len = op - lp_start->timelen;

                if(len / 60 >= 5)
                {
                    glog::GetInstance()->AddLine("�����ر�ĩ֪�ͻ���");
                    closesocket(lp_start->socket);
                    break;
                }
            }
        }

        lp_start = m_io_group.GetNext(pos);
    }
}
//
/*-------------------------------------------------------------------------------------------
�������ܣ����ݴ����̺߳���
����˵����
�������أ�
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

        //*lpOverlappedΪ�ղ��Һ���û�д���ɶ˿�ȡ����ɰ�������ֵ��Ϊ0�������򲻻���lpNumberOfBytes and lpCompletionKey��ָ��Ĳ����д洢��Ϣ��
        if(bRet == 0 && lp_io == NULL)
        {
            int errcode = WSAGetLastError();
            //��� *lpOverlappedΪ�ղ��Һ���û�д���ɶ˿�ȡ����ɰ�������ֵ��Ϊ0�������򲻻���lpNumberOfBytes and lpCompletionKey��ָ��Ĳ����д洢��Ϣ
            glog::GetInstance()->AddLine("GetQueuedCompletionStatus lp_io is NULL ErrorCode:%d", WSAGetLastError());
            lp_this->PostLog("GetQueuedCompletionStatus lp_io:%p bRet:%d �������:%d ������Ϣ:%s ", lp_io, bRet, errcode, lp_this->getErrorInfo(errcode).c_str());
            continue;
        }

        //���: ��� *lpOverlapped��Ϊ�ղ��Һ�������ɶ˿ڳ���һ��ʧ��I/O��������ɰ���
        //����ֵΪ0��������ָ��lpNumberOfBytesTransferred, lpCompletionKey, and lpOverlapped�Ĳ���ָ���д洢�����Ϣ������GetLastError���Եõ���չ������Ϣ
        if(FALSE == bRet && lp_io != NULL)
        {
            //��� *lpOverlapped��Ϊ�ղ��Һ�������ɶ˿ڳ���һ��ʧ��I/O��������ɰ�������ֵΪ0��������ָ��lpNumberOfBytesTransferred, lpCompletionKey, and lpOverlapped�Ĳ���ָ���д洢�����Ϣ
            int errcode = WSAGetLastError();
            glog::GetInstance()->AddLine("GetQueuedCompletionStatus lp_io:%p bRet:%d �������:%d ������Ϣ:%s ", lp_io, bRet, errcode, lp_this->getErrorInfo(errcode).c_str());
            lp_this->PostLog("GetQueuedCompletionStatus lp_io:%p bRet:%d �������:%d ������Ϣ:%s operation:%d", lp_io, bRet, errcode, lp_this->getErrorInfo(errcode).c_str(), lp_io->operation);
            lp_this->PostLog("�쳣�˳�");
            lp_this->ExitSocket(lp_io, lp_key, errcode);
            //EnterCriticalSection(&lp_this->crtc_sec);
            //closesocket(lp_io->socket);
            //lp_this->m_io_group.RemoveAt(lp_io);
            //lp_this->m_key_group.RemoveAt(lp_key);
            //lp_this->DeleteByIo((ULONG_PTR)lp_io->pUserData);
            //LeaveCriticalSection(&lp_this->crtc_sec);
            continue;
            //�黹IO�����continue;
        }

        //if (bRet==TRUE&&dwBytes==0)
        //{
        //  lp_this->PostLog("bRet=1 dwByte=0 errorcode:%s",lp_this->getErrorInfo(WSAGetLastError()).c_str());
        //}

        //���������һ����ɶ˿ڵ�һ��socket������ر��ˣ���GetQueuedCompletionStatus����ERROR_SUCCESS��Ҳ��0��,����lpNumberOfBytes����0
        //�˳�����
        if((IOCP_ACCEPT != lp_io->operation) && (0 == dwBytes))
        {
            lp_this->PostLog("�����˳�");
            lp_this->ExitSocket(lp_io, lp_key, GetLastError());
            continue;
        }

        //socket ͨ��ʱ��
        int op_len = 0;
        int op = 0;
        op_len = sizeof(op);
        nRet = getsockopt(lp_io->socket, SOL_SOCKET, SO_CONNECT_TIME, (char*)&op, &op_len);

        if(SOCKET_ERROR == nRet)
        {
            lp_this->PostLog("lp_io:%p errorcode:%d getsockopt", lp_io, WSAGetLastError(), lp_this->m_io_group.GetCount());
            closesocket(lp_io->socket);
            //continue;
        }

        if(op != 0xffffffff)
        {
            lp_io->timelen = op;
            //glog::traceErrorInfo("getsockopt",WSAGetLastError());
            //glog::trace("\nlp_io:%p timelen:%d",lp_io,lp_io->timelen);
        }

        switch(lp_io->operation)
        {
            case IOCP_ACCEPT:
                {
                    lp_io->state = SOCKET_STATE_CONNECT;

                    if(dwBytes > 0)     lp_io->state = SOCKET_STATE_CONNECT_AND_READ;

                    nRet = setsockopt(lp_io->socket, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, (char*)&lp_this->m_listen_socket, sizeof(lp_this->m_listen_socket));

                    if(SOCKET_ERROR == nRet)
                    {
                        glog::GetInstance()->AddLine("CompletionRoutine->IOCP_ACCEPT setsockopt ErroCode:%d", WSAGetLastError());
                        closesocket(lp_io->socket);
                        lp_this->m_io_group.RemoveAt(lp_io);
                        glog::traceErrorInfo("setsockopt", WSAGetLastError());
                        continue;
                    }

                    lp_new_key = lp_this->m_key_group.GetBlank();

                    if(lp_new_key == NULL)
                    {
                        glog::GetInstance()->AddLine("CompletionRoutine->IOCP_ACCEPT m_key_group.GetBlank ErroCode:%d", WSAGetLastError());
                        glog::traceErrorInfo("GetBlank��", WSAGetLastError());
                        closesocket(lp_io->socket);
                        lp_this->m_io_group.RemoveAt(lp_io);
                        continue;
                    }

                    lp_new_key->socket = lp_io->socket;
                    lp_io->lp_key = lp_new_key;
                    //���½�����SOCKETͬ��ɶ˿ڹ���������
                    hRet = CreateIoCompletionPort((HANDLE)lp_io->socket, lp_this->m_h_iocp, (DWORD)lp_new_key, 0);

                    if(NULL == hRet)
                    {
                        glog::GetInstance()->AddLine("CompletionRoutine->IOCP_ACCEPT CreateIoCompletionPort ErroCode:%d", WSAGetLastError());
                        glog::traceErrorInfo("CreateIoCompletionPort", WSAGetLastError());
                        closesocket(lp_io->socket);
                        lp_this->m_key_group.RemoveAt(lp_new_key);
                        lp_this->m_io_group.RemoveAt(lp_io);
                        continue;
                    }

                    //�����ȡ��������
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

                    string towrite = "";
                    int lenghth = lp_io->ol.InternalHigh;
                    //if(lp_io->fromtype == SOCKET_FROM_Concentrator)
                    //{
                    string data = gstring::char2hex(lp_io->buf, lp_io->ol.InternalHigh);
                    lenghth > 0 ? glog::GetInstance()->AddLine("������:%d ������:%s", lenghth, data.c_str()) : 1;
                    towrite = data;
                    CListTextElementUI* pElement = (CListTextElementUI*)lp_io->pUserData;

                    if(pElement)
                    {
                        pElement->SetText(8, lp_io->gayway);
                        pElement->SetText(5, data.c_str());
                        pElement->SetText(6, lp_io->buf);
                        char lenstr[20] = {0};
                        sprintf(lenstr, "%d", lenghth);
                        pElement->SetText(6, lp_io->buf);
                        pElement->SetText(7, lenstr);
                    }

                    //���Ǽ�����   �ж��ǲ��Ƕϰ� //�ȼ���ǲ��Ƕϰ� �ϰ���ƴ������
                    if(lp_this->checkFlag((BYTE*)lp_io->buf, lp_io->ol.InternalHigh) == FALSE)
                    {
                        //�������ϰ�����
                        map<IOCP_IO_PTR, pBREAKPCK>::iterator itepack =  lp_this->m_pack.find(lp_io);

                        if(itepack == lp_this->m_pack.end())
                        {
                            //�������ϰ���ͷ
                            BOOL bBreakPack =    lp_this->IsBreakPack((BYTE*)lp_io->buf, lp_io->ol.InternalHigh);

                            if(bBreakPack)
                            {
                                pBREAKPCK b = new BREAK_PACK;
                                BYTE *b1 = new BYTE[lp_io->ol.InternalHigh];
                                memset(b1, 0, lp_io->ol.InternalHigh);
                                memcpy(b1, lp_io->buf, lp_io->ol.InternalHigh);
                                b->b = b1;
                                b->len = lp_io->ol.InternalHigh;
                                lp_this->m_pack.insert(make_pair(lp_io, b));
                                lp_this->PostLog("�ϰ���ͷ:lp_io:%p ����:%d", lp_io, lenghth);
                                glog::GetInstance()->AddLine("�ϰ���ͷ:lp_io:%p ����:%d", lp_io, lenghth);
                            }

                            if(lp_io->fromtype == SOCKET_FROM_WEBSOCKET)
                            {
                                string  strret = "";
                                BOOL bFullPack = TRUE;
                                int lenread = lp_this->wsDecodeFrame(lp_io->buf, strret, lp_io->ol.InternalHigh, bFullPack);

                                if(bFullPack == FALSE)
                                {
                                    //websocket�ϰ�����
                                    map<IOCP_IO_PTR, pBREAKPCK>::iterator itepack =  lp_this->m_pack.find(lp_io);

                                    if(itepack == lp_this->m_pack.end())
                                    {
                                        pBREAKPCK b = new BREAK_PACK;
                                        BYTE *b1 = new BYTE[lp_io->ol.InternalHigh];
                                        memset(b1, 0, lp_io->ol.InternalHigh);
                                        memcpy(b1, lp_io->buf, lp_io->ol.InternalHigh);
                                        b->b = b1;
                                        b->len = lp_io->ol.InternalHigh;
                                        lp_this->m_pack.insert(make_pair(lp_io, b));
                                        lp_this->PostLog("Web �ϰ���ͷ:lp_io:%p ����:%d", lp_io, lenghth);
                                    }
                                }
                            }
                        }
                        else
                        {
                            if(lp_io->fromtype == SOCKET_FROM_WEBSOCKET)
                            {
                                lp_this->PostLog("web �ϰ���β:lp_io:%p ����:%d", lp_io, lenghth);
                            }

                            if(lp_io->fromtype == SOCKET_FROM_Concentrator)
                            {
                                lp_this->PostLog("�ϰ���β:lp_io:%p ����:%d", lp_io, lenghth);
                                glog::GetInstance()->AddLine("�ϰ���β:lp_io:%p ����:%d", lp_io, lenghth);
                            }

                            lp_this->AppendByte((BYTE*)lp_io->buf, lp_io->ol.InternalHigh, itepack->second, lp_io);
                            BREAK_PACK* pack = (BREAK_PACK*)itepack->second;
                            lenghth = pack->len;
                            delete pack;
                            lp_this->m_pack.erase(itepack);
                        }
                    }

                    if(lp_this->checkFlag((BYTE*)lp_io->buf, lenghth))
                    {
                        lp_io->fromtype = SOCKET_FROM_Concentrator;
                        BYTE tosend[216] = {0};
                        int  deslen = 0;
                        BOOL bresponse = FALSE;
                        string datastr = gstring::char2hex(lp_io->buf, lenghth);
                        lp_this->PostLog("����:[%s] ������:%d ������:%s ͨ��ָ��:%p", lp_io->gayway, lenghth, datastr.c_str(), lp_io);
                        lp_this->buildcode((BYTE*)lp_io->buf, lenghth, tosend, deslen, bresponse, lp_io);

                        if(bresponse && deslen > 0)
                        {
                            lp_this->InitIoContext(lp_io);
                            string hex = gstring::char2hex((char*)tosend, deslen);
                            //glog::GetInstance()->AddLine("��Ӧ�������İ�:%s", hex.c_str());
                            memcpy(lp_io->buf, tosend, deslen);
                            lp_io->wsaBuf.len = deslen;
                            lp_io->operation = IOCP_WRITE;
                        }
                    }
                    else
                    {
                        if(lp_io->fromtype == SOCKET_FROM_WEBSOCKET)
                        {
                            BOOL bAnswer = FALSE;
                            lp_this->DealWebsockMsg(lp_io, lp_key, (BYTE*)lp_io->buf, lenghth, bAnswer);

                            if(lp_io->operation == IOCP_END)
                            {
                                goto ToMsg;
                            }
                        }

                        string strdata = lp_io->buf;
                        string strret;
                        //����Э��
                        int wsconn = lp_this->wsHandshake(strdata, strret);

                        if(wsconn == WS_STATUS_CONNECT)
                        {
                            lp_this->InitIoContext(lp_io);
                            lp_io->operation = IOCP_WRITE;
                            lp_io->fromtype = SOCKET_FROM_WEBSOCKET;

                            if(pElement)
                            {
                                pElement->SetText(8, "web�ͻ���");
                            }

                            strcpy(lp_io->gayway, "web�ͻ���(2)");
                            lp_io->loginstatus = SOCKET_STATUS_LOGIN;
                            memcpy(lp_io->buf, strret.c_str(), strret.size());
                            lp_this->PostLog("web������....");
                            lp_io->wsaBuf.len = strret.size();
                        }
                    }

ToMsg:
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
BOOL CIOCP::SendData(ULONG_PTR s, ULONG_PTR key, string vvv)
{
    IOCP_IO_PTR  piocp_prt = (IOCP_IO_PTR)s;
    IOCP_KEY_PTR piocp_key = (IOCP_KEY_PTR)key;
    //int n1 = m_listctr->getSelectIndex();
    //string vvv = m_listctr->getCellText(n1, 2);
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
    InitIoContext(piocp_prt);
    memcpy(piocp_prt->buf, b2, i);
    piocp_prt->wsaBuf.len = strlen(vvv.c_str()) / 2;
    piocp_prt->wsaBuf.buf = piocp_prt->buf;
    piocp_prt->operation = IOCP_WRITE;
    DataAction(piocp_prt, piocp_prt->lp_key);
    // piocp_prt->operation = IOCP_READ;
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
    // ����http����ͷ��Ϣ
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

    // ���http��Ӧͷ��Ϣ
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
int CIOCP::wsDecodeFrame(char inFrame[], string & outMessage, int len, BOOL& fullpack)
{
    int ret = WS_OPENING_FRAME;
    const char *frameData = inFrame;
    const int frameLength = len;

    if(frameLength < 2)
    {
        ret = WS_ERROR_FRAME;
    }

    // �����չλ������
    if((frameData[0] & 0x70) != 0x0)
    {
        ret = WS_ERROR_FRAME;
    }

    // finλ: Ϊ1��ʾ�ѽ�����������, Ϊ0��ʾ����������������
    ret = (frameData[0] & 0x80);  //1000 0000

    if((frameData[0] & 0x80) != 0x80)
    {
        ret = WS_ERROR_FRAME;
    }

    // maskλ, Ϊ1��ʾ���ݱ�����
    if((frameData[1] & 0x80) != 0x80)
    {
        ret = WS_ERROR_FRAME;
    }

    // ������
    uint16_t payloadLength = 0;
    uint8_t payloadFieldExtraBytes = 0;
    uint8_t opcode = static_cast<uint8_t >(frameData[0] & 0x0f);

    if(opcode == WS_TEXT_FRAME)
    {
        // ����utf-8������ı�֡
        payloadLength = static_cast<uint16_t >(frameData[1] & 0x7f);

        if(payloadLength == 0x7e)   //0111 1110     //126 7e  �������ֽ��ǳ��� :  127  7f �������ֽ��ǳ���
        {
            uint16_t payloadLength16b = 0;
            payloadFieldExtraBytes = 2;
            memcpy(&payloadLength16b, &frameData[2], payloadFieldExtraBytes);
            payloadLength = ntohs(payloadLength16b);
        }
        else if(payloadLength == 0x7f)
        {
            // ���ݹ���,�ݲ�֧��
            ret = WS_ERROR_FRAME;
        }
    }
    else if(opcode == WS_BINARY_FRAME || opcode == WS_PING_FRAME || opcode == WS_PONG_FRAME)
    {
        // ������/ping/pong֡�ݲ�����
    }
    else if(opcode == WS_CLOSING_FRAME)
    {
        ret = WS_CLOSING_FRAME;
    }
    else
    {
        ret = WS_ERROR_FRAME;
    }

    // ���ݽ���
    if((ret != WS_ERROR_FRAME) && (payloadLength > 0))
    {
        // header: 2�ֽ�, masking key: 4�ֽ�
        const char *maskingKey = &frameData[2 + payloadFieldExtraBytes];
        char *payloadData = new char[payloadLength + 1];
        memset(payloadData, 0, payloadLength + 1);
        memcpy(payloadData, &frameData[2 + payloadFieldExtraBytes + 4], payloadLength);

        for(int i = 0; i < payloadLength; i++)
        {
            payloadData[i] = payloadData[i] ^ maskingKey[i % 4];
        }

        string begin = "{\"begin\":\"6A\"";
        string end = "\"end\":\"6A\"}";

        if(_strnicmp(begin.c_str(), payloadData, begin.size()) == 0)
        {
            int n1 = payloadLength - end.size();

            if(n1 >= 0 && _strnicmp(end.c_str(), &payloadData[n1], begin.size()) == 0)
            {
                //glog::trace("\nwebsocket is one pack");
                outMessage = payloadData;
                fullpack = TRUE;
            }
            else
            {
                //glog::trace("\nwebsocket is break pack");
                fullpack = FALSE;
            }
        }

//         size_t len1 = 0;
//         int num = payloadLength * 3;
//         WCHAR* poutBuf = new WCHAR[num];
//         memset(poutBuf, 0, num);
//         char outbuff[4096] = {0};
//         int lenout = payloadLength;
//         BOOL bchar = gstring::UrlDecode(payloadData, outbuff, lenout);
//         outMessage = outbuff;
//         delete[] payloadData;
    }

    return ret;
}
int CIOCP::wsEncodeFrame(string inMessage, char outFrame[], enum WS_FrameType frameType, int& lenret)
{
    int ret = WS_EMPTY_FRAME;
    const uint32_t messageLength = inMessage.size();

    if(messageLength > 32767)
    {
        // �ݲ�֧����ô��������
        return WS_ERROR_FRAME;
    }

    uint16_t payloadFieldExtraBytes = (messageLength <= 0x7d) ? 0 : 2;
    // header: 2�ֽ�, maskλ����Ϊ0(������), ������masking key������д, ʡ��4�ֽ�
    uint8_t frameHeaderSize = 2 + payloadFieldExtraBytes;
    uint8_t *frameHeader = new uint8_t[frameHeaderSize];
    memset(frameHeader, 0, frameHeaderSize);
    // finλΪ1, ��չλΪ0, ����λΪframeType
    frameHeader[0] = static_cast<uint8_t>(0x80 | frameType);

    // ������ݳ���
    if(messageLength <= 0x7d)   //125->7d
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

    // �������
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
//     Json::Value root;
//     Json::Reader reader;
//
//     if(reader.parse(jsondata.c_str(), root))
//     {
//         Json::Value vtemp = root["msg"];
//
//         if(vtemp.isNull())
//         {
//             glog::trace("\nthis is null data pack\n");
//         }
//         else if(vtemp.isArray())
//         {
//             glog::trace("\nthis is array data pack\n");
//         }
//         else if(vtemp.isObject())
//         {
//             glog::trace("\nthis is object data pack\n");
//         }
//         else if(vtemp.isString())
//         {
//             if(vtemp == "getStatus")
//             {
//                 string addrarea = root["addr"].asString();
//                 map<string, IOCP_IO_PTR>::iterator ite = m_mcontralcenter.find(addrarea);
//
//                 if(ite != m_mcontralcenter.end())
//                 {
//                     Json::Value row = root["row"];
//                     root["data"] = TRUE;
//                 }
//
//                 glog::trace("\naddress:%s", addrarea.c_str());
//                 string inmsg = root.toStyledString();
//                 //  string inmsg = root.toStyledString();
//                 char outmsg[1048] = {0};
//                 int lenret = 0;
//                 int len = wsEncodeFrame(inmsg, outmsg, WS_TEXT_FRAME, lenret);
//
//                 if(len != WS_ERROR_FRAME)
//                 {
//                     memcpy(lp_io->buf, outmsg, lenret);
//                     lp_io->wsaBuf.buf = lp_io->buf;
//                     lp_io->wsaBuf.len = lenret;
//                     lp_io->operation = IOCP_WRITE;
//                 }
//             }
//             else if(vtemp == "Online")
//             {
//                 root["count"] = m_mcontralcenter.size();
//                 root["status"] = "success";
//                 string inmsg = root.toStyledString();
//                 char outmsg[1048] = {0};
//                 int lenret = 0;
//                 int len = wsEncodeFrame(inmsg, outmsg, WS_TEXT_FRAME, lenret);
//
//                 if(len != WS_ERROR_FRAME)
//                 {
//                     memcpy(lp_io->buf, outmsg, lenret);
//                     lp_io->wsaBuf.buf = lp_io->buf;
//                     lp_io->wsaBuf.len = lenret;
//                     lp_io->operation = IOCP_WRITE;
//                 }
//             }
//             else if(vtemp == "AA")          //������ѯ
//             {
//                 Json::Value isres = root["res"];
//
//                 if(isres.asString() == "1")   //����������Ҫ������Ӧ
//                 {
//                     string addrarea = root["addr"].asString();
//                     map<string, IOCP_IO_PTR>::iterator ite = m_mcontralcenter.find(addrarea);
//
//                     if(ite != m_mcontralcenter.end())
//                     {
//                         m_listmsg.push_back(lp_io);
//                         glog::GetInstance()->AddLine("����Ϣ����:%d ���һ����Ϣ����:%s ������ѯ����", m_listmsg.size(), root["data"].asString().c_str());
//                     }
//
//                     //m_listwebsock.push_back(lp_io);
//                     //m_qwebsock.push()
//                 }
//
//                 Json::Value tosend = root["data"];
//                 string data = tosend.asString();
//                 data = gstring::replace(data, " ", "");
//                 BYTE bitSend[512] = {0};
//                 int len = hex2str(data, bitSend);
//
//                 if(len > 0)
//                 {
//                     string addrarea = root["addr"].asString();
//                     map<string, IOCP_IO_PTR>::iterator ite = m_mcontralcenter.find(addrarea);
//
//                     if(ite != m_mcontralcenter.end())
//                     {
//                         IOCP_IO_PTR lp_io1 = ite->second;
//                         memcpy(lp_io1->buf, bitSend, len);
//                         lp_io1->wsaBuf.buf = lp_io1->buf;
//                         lp_io1->wsaBuf.len = len;
//                         lp_io1->operation = IOCP_WRITE;
//                         DataAction(lp_io1, lp_io1->lp_key);
//                     }
//                 }
//             }
//             else if(vtemp == "A4")          //��������
//             {
//                 Json::Value isres = root["res"];
//
//                 if(isres.asString() == "1")   //����������Ҫ������Ӧ
//                 {
//                     string addrarea = root["addr"].asString();
//                     map<string, IOCP_IO_PTR>::iterator ite = m_mcontralcenter.find(addrarea);
//
//                     if(ite != m_mcontralcenter.end())
//                     {
//                         m_listmsg.push_back(lp_io);
//                         glog::GetInstance()->AddLine("����Ϣ����:%d ���һ����Ϣ����:%s ������������", m_listmsg.size(), root["data"].asString().c_str());
//                     }
//
//                     //m_listwebsock.push_back(lp_io);
//                     //m_qwebsock.push()
//                 }
//
//                 Json::Value tosend = root["data"];
//                 string data = tosend.asString();
//                 data = gstring::replace(data, " ", "");
//                 BYTE bitSend[512] = {0};
//                 int len = hex2str(data, bitSend);
//
//                 if(len > 0)
//                 {
//                     string addrarea = root["addr"].asString();
//                     map<string, IOCP_IO_PTR>::iterator ite = m_mcontralcenter.find(addrarea);
//
//                     if(ite != m_mcontralcenter.end())
//                     {
//                         IOCP_IO_PTR lp_io1 = ite->second;
//                         memcpy(lp_io1->buf, bitSend, len);
//                         lp_io1->wsaBuf.buf = lp_io1->buf;
//                         lp_io1->wsaBuf.len = len;
//                         lp_io1->operation = IOCP_WRITE;
//                         DataAction(lp_io1, lp_io1->lp_key);
//                     }
//                 }
//             }
//             else if(vtemp == "A5")                  //��������
//             {
//                 Json::Value isres = root["res"];
//
//                 if(isres.asString() == "1")   //����������Ҫ������Ӧ
//                 {
//                     string addrarea = root["addr"].asString();
//                     map<string, IOCP_IO_PTR>::iterator ite = m_mcontralcenter.find(addrarea);
//
//                     if(ite != m_mcontralcenter.end())
//                     {
//                         IOCP_IO_PTR  lp = ite->second;
//
//                         if(lp->operation == IOCP_END)
//                         {
//                             m_mcontralcenter.erase(ite);
//                         }
//                         else
//                         {
//                             m_listmsg.push_back(lp_io);
//                             glog::GetInstance()->AddLine("����Ϣ����:%d ���һ����Ϣ����:%s ��������", m_listmsg.size(), root["data"].asString().c_str());
//                         }
//                     }
//
//                     //m_listwebsock.push_back(lp_io);
//                     //m_qwebsock.push()
//                 }
//
//                 Json::Value tosend = root["data"];
//                 string data = tosend.asString();
//                 data = gstring::replace(data, " ", "");
//                 BYTE bitSend[512] = {0};
//                 int len = hex2str(data, bitSend);
//
//                 if(len > 0)
//                 {
//                     string addrarea = root["addr"].asString();
//                     map<string, IOCP_IO_PTR>::iterator ite2 = m_mcontralcenter.find(addrarea);
//
//                     if(ite2 != m_mcontralcenter.end())
//                     {
//                         IOCP_IO_PTR lp_io1 = ite2->second;
//                         memcpy(lp_io1->buf, bitSend, len);
//                         lp_io1->wsaBuf.buf = lp_io1->buf;
//                         lp_io1->wsaBuf.len = len;
//                         lp_io1->operation = IOCP_WRITE;
//                         DataAction(lp_io1, lp_io1->lp_key);
//                     }
//                 }
//             }
//             else if(vtemp == "00")
//             {
//                 Json::Value tosend = root["data"];
//                 string data = tosend.asString();
//                 data = gstring::replace(data, " ", "");
//                 BYTE bitSend[512] = {0};
//                 int len = hex2str(data, bitSend);
//
//                 if(len > 0)
//                 {
//                     string addrarea = root["addr"].asString();
//                     map<string, IOCP_IO_PTR>::iterator ite = m_mcontralcenter.find(addrarea);
//
//                     if(ite != m_mcontralcenter.end())
//                     {
//                         IOCP_IO_PTR lp_io1 = ite->second;
//                         memcpy(lp_io1->buf, bitSend, len);
//                         lp_io1->wsaBuf.buf = lp_io1->buf;
//                         lp_io1->wsaBuf.len = len;
//                         lp_io1->operation = IOCP_WRITE;
//                         DataAction(lp_io1, lp_io1->lp_key);
//                     }
//                 }
//             }
//             else if(vtemp == "AC")
//             {
//                 Json::Value isres = root["res"];
//
//                 if(isres.asString() == "1")   //����������Ҫ������Ӧ
//                 {
//                     string addrarea = root["addr"].asString();
//                     map<string, IOCP_IO_PTR>::iterator ite = m_mcontralcenter.find(addrarea);
//
//                     if(ite != m_mcontralcenter.end())
//                     {
//                         m_listmsg.push_back(lp_io);
//                         glog::GetInstance()->AddLine("����Ϣ����:%d ���һ����Ϣ����:%s ��ѯ����", m_listmsg.size(), root["data"].asString().c_str());
//                     }
//
//                     //m_listwebsock.push_back(lp_io);
//                     //m_qwebsock.push()
//                 }
//
//                 Json::Value tosend = root["data"];
//                 string data = tosend.asString();
//                 data = gstring::replace(data, " ", "");
//                 BYTE bitSend[512] = {0};
//                 int len = hex2str(data, bitSend);
//
//                 if(len > 0)
//                 {
//                     string addrarea = root["addr"].asString();
//                     map<string, IOCP_IO_PTR>::iterator ite = m_mcontralcenter.find(addrarea);
//
//                     if(ite != m_mcontralcenter.end())
//                     {
//                         IOCP_IO_PTR lp_io1 = ite->second;
//                         memcpy(lp_io1->buf, bitSend, len);
//                         lp_io1->wsaBuf.buf = lp_io1->buf;
//                         lp_io1->wsaBuf.len = len;
//                         lp_io1->operation = IOCP_WRITE;
//                         DataAction(lp_io1, lp_io1->lp_key);
//                     }
//                 }
//             }
//         }
//     }
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


//������Ӧ����  src
/*
*  src Դ�յ������ݰ�
*  srclen Դ������
*  des  ����Ŀ��İ�
*/
void CIOCP::buildcode(BYTE src[], int srclen, BYTE des[], int& deslen, BOOL & isrespos, IOCP_IO_PTR & lp_io)
{
    //��·��� ��½  ������ c4: 1100 0100  �����룺0x02 src[13] da1 src[14] da2 src[15] dt0    p0  f1  ��½   6������  13֡���� 12
    //&&src[14]==0x0&&src[15]==0x0&&src[16]==0x01&&src[17]==1   1100 0000 1100 0000   1 dir  1 yn PRM  6������  ֡���Ƿ�Ҫ�ظ� src[13]
    BYTE AFN = src[12];
    char addr1[4] = {0};
    memcpy(addr1, &src[7], 4); //��ַ
    char addrarea[20] = {0};
    sprintf(addrarea, "%02x%02x%02x%02x", addr1[1], addr1[0], addr1[3], addr1[2]); //���ص�ַ
    BYTE frame = src[13] & 0x0f;   //��
    BYTE    con =    src[13] & 0x10;
    BYTE   DirPrmCode = src[6] & 0xc0;   //����  �Ӷ�
    BYTE   FC = src[6] & 0xF; //���������Ĺ�����
    BYTE DA[2] = {0};
    BYTE DT[2] = {0};
    //string addrarea = gstring::char2hex(addr1, 4);
    memcpy(DA, &src[14], 2);    //PN P0
    memcpy(DT, &src[16], 2);   //F35  �����ѹ   00 00 04 04 ���������ѹ

    //��·���
    if(AFN == 0x02)      //��·���
    {
        //src[6] == 0xc4 && src[13] & 0x10 == 0x10
        BYTE    con =    src[13] & 0x10;
        BYTE   DirPrmCode = src[6] & 0xc4;
        BYTE DA[2] = {0};
        BYTE DT[2] = {0};
        memcpy(DA, &src[14], 2);    //PN P0
        memcpy(DT, &src[16], 2);   //FN  1 ��½ | 3 ����

        if(DirPrmCode == 0xc4 && con == 0x10)    //��Ҫ�ظ�
        {
            USHORT Pn = (USHORT) * DA;
            USHORT Fn = (USHORT) * DT;
            isrespos = TRUE;

            if(DA[0] == 0 && DA[1] == 0 && DT[1] == 0 && DT[0] == 1) //DT1��
            {
                glog::GetInstance()->AddLine("����[%s] ��½", addrarea);
                PostLog("����[%s] ��½", addrarea);
                strcpy(lp_io->gayway, addrarea);
                setOnline(addrarea, 1);
                map<string, IOCP_IO_PTR>::iterator it = m_mcontralcenter.find(addrarea);

                if(it == m_mcontralcenter.end())
                {
                    m_mcontralcenter.insert(pair<string, IOCP_IO_PTR>(addrarea, lp_io));
                }
                else
                {
                    it->second = lp_io;
                }

                buildConCode(src, des, deslen, 1);
            }
            else if(DA[0] == 0 && DA[1] == 0 && DT[1] == 0 && DT[0] == 4)
            {
                PostLog("����[%s] ����", addrarea);
                lp_io->loginstatus = SOCKET_STATUS_LOGIN;
                buildConCode(src, des, deslen, 1);
            }
        }
    }
    else if(AFN == 0x00 || AFN == 0xAA || AFN == 0xA4 || AFN == 0xFF || AFN == 0xFE) //ȫ��ȷ��
    {
        if(DirPrmCode == 0x80 && con == 0x0 && FC == 0x8)   //  ���� �Ӷ� ��Ӧ֡   0x80 ���� �Ӷ�
        {
            map<string, list<MSGPACK>>::iterator itmsg = m_MsgPack.find(addrarea);

            if(itmsg != m_MsgPack.end())
            {
                list<MSGPACK>v_msg = itmsg->second;

                if(v_msg.size() > 0)
                {
                    MSGPACK msgEnd = v_msg.back();
                    IOCP_IO_PTR lp_io1 = NULL;

                    if(msgEnd.seq == frame)
                    {
                        lp_io1 = msgEnd.lp_io;
                        itmsg->second.pop_back();
                    }
                    else
                    {
                        // EnterCriticalSection(&crtc_sec);
                        list<MSGPACK>::reverse_iterator it = itmsg->second.rbegin();

                        while(it != itmsg->second.rend())
                        {
                            MSGPACK pack = *it;

                            if(pack.seq == frame)
                            {
                                lp_io1 = pack.lp_io;
                                itmsg->second.erase((++it).base());
                                break;
                            }

                            it++;
                        }

                        //  LeaveCriticalSection(&crtc_sec);
                    }

                    if(lp_io1 != NULL)
                    {
                        string strret = "";
                        BOOL bFullPack = FALSE;
                        int lenread = wsDecodeFrame(lp_io1->buf, strret, lp_io1->ol.InternalHigh, bFullPack);
                        Json::Value root;
                        Json::Reader reader;

                        if(reader.parse(strret.c_str(), root))
                        {
                            root["status"] = "success";
                            root["frame"] = frame;
                            root["comaddr"] = addrarea;
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
            }
        }
    }
    else if(AFN == 0xAC)
    {
        //����һ������
        time_t tmtamp;
        struct tm *tm1 = NULL;
        time(&tmtamp) ;
        tm1 = localtime(&tmtamp) ;
        tm1->tm_mday--;
        mktime(tm1);
        char myday[30] = {0};
        strftime(myday, sizeof(myday), "%Y-%m-%d", tm1);
        PostLog("��������:%s", myday);

        if(DirPrmCode == 0x80 && con == 0x0 && FC == 0x8)   //  ���� �Ӷ� ��Ӧ֡   0x80 ���� �Ӷ�
        {
            //�����ѹ
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

                clearEndChar(strA, "|");
                clearEndChar(strB, "|");
                clearEndChar(strC, "|");
                jsonRoot["len"] = p;
                jsonRoot["A"] = strA;
                jsonRoot["B"] = strB;
                jsonRoot["C"] = strC;
                string inmsg = jsonRoot.toStyledString();
                dealSqlRecords(addrarea, myday, inmsg, "voltage");
                return;
            }

            //�������
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

                clearEndChar(strA, "|");
                clearEndChar(strB, "|");
                clearEndChar(strC, "|");
                jsonRoot["len"] = p;
                jsonRoot["A"] = strA;
                jsonRoot["B"] = strB;
                jsonRoot["C"] = strC;
                string inmsg = jsonRoot.toStyledString();
                dealSqlRecords(addrarea, myday, inmsg, "electric");
                return;
            }

            //�����й�����
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

                clearEndChar(strA, "|");
                clearEndChar(strB, "|");
                clearEndChar(strC, "|");
                jsonRoot["len"] = p;
                jsonRoot["A"] = strA;
                jsonRoot["B"] = strB;
                jsonRoot["C"] = strC;
                string inmsg = jsonRoot.toStyledString();
                dealSqlRecords(addrarea, myday, inmsg, "activepower");
                return;
            }

            //���๦������
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

                clearEndChar(strA, "|");
                clearEndChar(strB, "|");
                clearEndChar(strC, "|");
                clearEndChar(strD, "|");
                jsonRoot["len"] = p;
                jsonRoot["A"] = strA;
                jsonRoot["B"] = strB;
                jsonRoot["C"] = strC;
                jsonRoot["D"] = strD;
                string inmsg = jsonRoot.toStyledString();
                dealSqlRecords(addrarea, myday, inmsg, "powerfactor");
                return;
            }

            //�����й�������
            if(DA[0] == 0 && DA[1] == 0 && DT[0] == 0x01 && DT[1] == 0x05)
            {
                BYTE* p1 = src;
                Json::Value jsonRoot;
                int p = 0;
                string strA;
                float fbegin = 0;
                float fend = 0;
                char power00[20] = {0};
                char power45[20] = {0};

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

                    if(i == 18)
                    {
                        fbegin = atof(strA1);
                        strcpy(power00, strA1);
                    }

                    int n1 = i + 4;

                    if(n1 >= srclen - 2)
                    {
                        fend = atof(strA1);
                        strcpy(power45, strA1);
                    }
                }

                PostLog("begin:%0.2f  end:%0.2f", fbegin, fend);
                clearEndChar(strA, "|");
                float fenergy = fend - fbegin;
                char  strenergy[20] = {0};
                sprintf(strenergy, "%0.2f", fenergy);
                jsonRoot["len"] = p;
                jsonRoot["A"] = strA;
                jsonRoot["energy"] = strenergy;
                string inmsg = jsonRoot.toStyledString();
                dealSqlRecords(addrarea, myday, inmsg, "power");
                dealSqlPower(addrarea, myday, power00, power45);
                string sql = "UPDATE t_power SET POWER45=\'";
                sql.append(power00);
                sql.append("\'");
                sql.append(" WHERE id=(select top 1 id from t_power  WHERE DAY < CONVERT(datetime,\'");
                sql.append(myday);
                sql.append("\',101)");
                sql.append(" and comaddr=\'");
                sql.append(addrarea);
                sql.append("\')");
                _RecordsetPtr rs = dbopen->ExecuteWithResSQL(sql.c_str());

                if(rs == NULL)
                {
                    glog::GetInstance()->AddLine("���µ���������:%s", sql.c_str());
                }

                //::SendMessageA(this->m_hParanWnd, WM_USER + 2, (WPARAM)sql.c_str(), (LPARAM)0);
                return;
            }

            map<string, list<MSGPACK>>::iterator itmsg = m_MsgPack.find(addrarea);

            if(itmsg != m_MsgPack.end())
            {
                list<MSGPACK>v_msg = itmsg->second;

                if(v_msg.size() > 0)
                {
                    MSGPACK msgEnd = v_msg.back();
                    IOCP_IO_PTR lp_io1 = NULL;

                    if(msgEnd.seq == frame)
                    {
                        lp_io1 = msgEnd.lp_io;
                        itmsg->second.pop_back();
                    }
                    else
                    {
                        // EnterCriticalSection(&crtc_sec);
                        list<MSGPACK>::reverse_iterator it = itmsg->second.rbegin();

                        while(it != itmsg->second.rend())
                        {
                            MSGPACK pack = *it;

                            if(pack.seq == frame)
                            {
                                lp_io1 = pack.lp_io;
                                itmsg->second.erase((++it).base());
                                break;
                            }

                            it++;
                        }

                        //  LeaveCriticalSection(&crtc_sec);
                    }

                    if(lp_io1 != NULL)
                    {
                        string strret = "";
                        BOOL bFullPack = FALSE;
                        int lenread = wsDecodeFrame(lp_io1->buf, strret, lp_io1->ol.InternalHigh, bFullPack);
                        Json::Value root;
                        Json::Reader reader;

                        if(reader.parse(strret.c_str(), root))
                        {
                            root["status"] = "success";
                            root["frame"] = frame;
                            root["comaddr"] = addrarea;
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
            }
        }
    }
    else if(AFN == 0x0E)             //�����͹����¼�
    {
        string a1 = gstring::char2hex((const char*)src, srclen);
        glog::GetInstance()->AddLine("����:%s", a1.c_str());
        BYTE    con =    src[13] & 0x10;
        BYTE   DirPrmCode = src[6] & 0xc0;   //����  �Ӷ�          ����  ����    1100  c0
        BYTE   FC = src[6] & 0xF; //���������Ĺ�����
        char addr1[10] = {0};
        memcpy(addr1, &src[7], 4);
        string addrarea = gstring::char2hex(addr1, 4);
        BYTE DA[2] = {0};
        BYTE DT[2] = {0};
        memcpy(DA, &src[14], 2);
        memcpy(DT, &src[16], 2);

        if(DirPrmCode == 0xC0 && con == 0)   //���� ����վ     �����ϱ����Ϻ�Ԥ��  ����Ҫ��Ӧ
        {
            if(DA[0] == 0 && DA[1] == 0 && DT[0] == 0x01 && DT[1] == 0x00)
            {
                int j = 18;
                BYTE errcode = src[j + 0];
                int datalen = src[j + 1];
                BYTE min = src[j + 2];
                BYTE hour = src[j + 3];
                BYTE day = src[j + 4];
                BYTE month = src[j + 5];
                BYTE year = src[j + 6];
                char cmin[20] = {0};
                char chour[20] = {0};
                char cday[20] = {0};
                char cmonth[20] = {0};
                char cyear[20] = {0};
                //5�ֽ� ʱ��  ��������
                string hexdata = gstring::char2hex((const char*)&src[j + 2], datalen);
                sprintf(cmin, "%d%d", min >> 4 & 0x0f, min & 0xf);
                sprintf(chour, "%d%d", hour >> 4 & 0x0f, hour & 0xf);
                sprintf(cday, "%d%d", day >> 4 & 0x0f, day & 0xf);
                sprintf(cmonth, "%d%d", month >> 4 & 0x0f, month & 0xf);
                sprintf(cyear, "20%d%d", year >> 4 & 0x0f, year & 0xf);
                char date[30] = {0};
                sprintf(date, "%s-%s-%s", cyear, cmonth, cday);
                char err[20] = {0};
                sprintf(err, "ERC%d", errcode);
                string sql = "select * from t_fault where 1=1 and CONVERT(Nvarchar, f_day, 23)=\'";
                sql.append(date);
                sql.append("\' and f_comaddr='");
                sql.append(addrarea);
                sql.append("\' and f_type=\'");
                sql.append(err);
                sql.append("\'");
                _RecordsetPtr rs = dbopen->ExecuteWithResSQL(sql.c_str());

                if(rs && dbopen->GetNum(rs) == 0)
                {
                    map<string, _variant_t>m_var;
                    _variant_t  vdate(date);
                    _variant_t  vcomaddr(addrarea.c_str());
                    _variant_t  verr(err);
                    _variant_t  vdata(hexdata.c_str());
                    _variant_t  vlen(datalen);
                    m_var.insert(pair<string, _variant_t>("f_day", vdate));
                    m_var.insert(pair<string, _variant_t>("f_comaddr", vcomaddr));
                    m_var.insert(pair<string, _variant_t>("f_type", verr));
                    m_var.insert(pair<string, _variant_t>("f_len", vlen));
                    m_var.insert(pair<string, _variant_t>("f_data", vdata));
                    string sql = dbopen->GetInsertSql(m_var, "t_fault");
                    _RecordsetPtr rs1 = dbopen->ExecuteWithResSQL(sql.c_str());

                    if(!rs1)
                    {
                        glog::GetInstance()->AddLine("���뱨���¼�ʧ��:sql %s", sql.c_str());
                    }

                    string sql1 = "select * from t_people where u_pid in (select pid from t_baseinfo where comaddr=\'";
					sql1.append(addrarea);
                    sql1.append("\')");
                    _RecordsetPtr rs = dbopen->ExecuteWithResSQL(sql1.c_str());

                    while(!rs->adoEOF)
                    {
                        _variant_t vname = rs->GetCollect("u_name");
                        _variant_t vemail = rs->GetCollect("u_email");
                        string name = _com_util::ConvertBSTRToString(vname.bstrVal);
                        string email = _com_util::ConvertBSTRToString(vemail.bstrVal);
                        objeamil.SetEmailTitle(string(err));
                        objeamil.SetContent(a1);
                        objeamil.AddTargetEmail(email);
                        rs->MoveNext();
                    }

                    //objeamil.AddTargetEmail();
                }
            }
        }
    }
}

void CIOCP::buildConCode(BYTE src[], BYTE res[], int& len, BYTE bcon)
{
    BYTE frame = src[13] & 0xf;   //֡���
    BYTE btemp[216] = {0};
    btemp[0] = 0x68;
    int nbyte1 = 20 - 2 - 6;     //20�Զ��峤��
    short n111 = (nbyte1 << 2) | 2;
    memcpy(&btemp[1], &n111, 2);
    memcpy(&btemp[3], &n111, 2);
    btemp[5] = 0x68;
    btemp[6] = 0x04;                //������ 0000 0100   0100����,�Ӷ�      04�Ƿ��ͻ�ȥ����Ӧ��
    memcpy(&btemp[7], &src[7], 5);  //��ַ��
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
BOOL CIOCP::AppendByte(BYTE src[], int len, pBREAKPCK pack, IOCP_IO_PTR & lp_io)
{
    int lenall = len + pack->len;
    BYTE* allbyte = new BYTE[lenall];
    memset(allbyte, 0, lenall);
    memcpy(allbyte, pack->b, pack->len);
    memcpy(allbyte +  pack->len, src, len);
    int n1 = lenall > 1024 ? 1024 : lenall;
    delete pack->b;
    pack->b = allbyte;
    pack->len = n1;
    memset(lp_io->wsaBuf.buf, 0, 1024);
    memcpy(lp_io->wsaBuf.buf, allbyte, n1);
    lp_io->wsaBuf.len = n1;
    return TRUE;
}
/*
*  comaddr ���ص�ַ
*  C       ������
*  AFN  ������
*  SEQ  ������
*  DA
*  DT
*  v_B  ��������
* des  ���յ�������
*/
int CIOCP::buidByte(string comaddr, BYTE C, BYTE AFN, BYTE SEQ, SHORT DA, SHORT DT, vector<BYTE>&v_b, BYTE des[])
{
    BYTE addrArea[4] = {0};

    if(comaddr.size() != 8)
    {
        PostLog("ͨ�ŵ�ַ���ϸ�ʽ");
        return 0;
    }

    int n =  hex2str(comaddr, addrArea);

    if(n > 0)
    {
        BYTE hexData[256] = {0};
        hexData[0] = 0x68;
        hexData[5] = 0x68;
        hexData[6] = C;//0x4;   //������  �������ǴӶ�  ���л������� 0x4  0000 0100
        hexData[7] = addrArea[1]; //parseInt(sprintf("0x%02d", addrArea[1]), 16)             //��ַ��
        hexData[8] = addrArea[0];              //parseInt(sprintf("0x%02d", addrArea[0]), 16)   //��ַ��
        hexData[9] = addrArea[3];                           //parseInt(sprintf("0x%02d", addrArea[3]), 16)
        hexData[10] = addrArea[2];         //parseInt(sprintf("0x%02d", addrArea[2]), 16)
        hexData[11] = 0x02;  //��ַC  ����ַ�����ַ
        hexData[12] = AFN;  //������
        hexData[13] = SEQ;  //֡����
        hexData[15] = DA >> 8 & 0x00ff; //   DA1
        hexData[14] = DA & 0x00ff;
        hexData[17] = DT >> 8 & 0x00ff;
        hexData[16] = DT & 0x00ff;

        for(int i = 0; i < v_b.size(); i++)
        {
            hexData[18 + i] = v_b[i];
        }

        int len1 = 18 - 6 + v_b.size();   //18�ǹ̶�����  6����ͷ
        int len2 = len1 << 2 | 2;
        int  a = len2 >> 8 & 0x000F;
        int b = len2 & 0x00ff;
        hexData[1] = b;               //len1 << 2 | 2;
        hexData[2] = a;
        hexData[3] = b;         //len1 << 2 | 2;
        hexData[4] = a;
        int v1 = 0;
        int len3 = 18 + v_b.size();

        for(int i = 6; i < len3; i++)
        {
            v1 = v1 + hexData[i];
        }

        int jyw = 18 + v_b.size();
        hexData[len3] = v1 % 256;
        hexData[len3 + 1] = 0x16;
        memcpy(des, hexData, len3 + 2);
        //string aaa=   gstring::char2hex((const char*)hexData,len3+2);
        //glog::GetInstance()->AddLine("%s",aaa.c_str());
        return len3 + 2;
    }
}

void CIOCP::ExitSocket(IOCP_IO_PTR& lp_io, IOCP_KEY_PTR& lp_key, int errcode)
{
    string towrite = "";
    PostLog("�пͻ������� ͨ��ָ��:%p  �ͻ�������:%d", lp_io, lp_io->fromtype);

    if(errcode != 1236)
    {
        lp_io->operation = IOCP_DEFAULT;
        closesocket(lp_io->socket);
    }

    //int n11 = m_io_group.GetCount();
    //int n00 = m_io_group.GetBlankCount();
    //PostLog("ExitSocket  lp_io:%p  List1 count:%d List2 count:%d �ͻ�������:%d", lp_io, n11, n00, lp_io->fromtype);
    m_io_group.RemoveAt(lp_io);
    m_key_group.RemoveAt(lp_key);
    int n11 = m_io_group.GetCount();
    int n00 = m_io_group.GetBlankCount();
    PostLog("ExitSocket  lp_io:%p  List1 count:%d List2 count:%d �ͻ�������:%d", lp_io, n11, n00, lp_io->fromtype);
    EnterCriticalSection(&crtc_sec);

    if(lp_io->fromtype == SOCKET_FROM_Concentrator)
    {
        //�������ͻ�������
        string comaddr = lp_io->gayway;
        setOnline(comaddr, 0);
        //�Ƴ�������
        map<string, IOCP_IO_PTR>::iterator  it = m_mcontralcenter.find(comaddr);

        if(it != m_mcontralcenter.end())
        {
            m_mcontralcenter.erase(it);
        }

        map<string, list<MSGPACK>>::iterator itMsg = m_MsgPack.find(comaddr);

        if(itMsg != m_MsgPack.end())
        {
            itMsg->second.clear();
            m_MsgPack.erase(itMsg);
        }
    }

    if(lp_io->fromtype == SOCKET_FROM_WEBSOCKET)
    {
        map<string, list<MSGPACK>>::iterator it = m_MsgPack.begin();
        //  EnterCriticalSection(&crtc_sec);

        while(it != m_MsgPack.end())
        {
            for(auto iter = it->second.begin(); iter != it->second.end();)
            {
                MSGPACK msg = *iter;

                if(msg.lp_io == lp_io)
                {
                    it->second.erase(iter++);
                    continue;
                    //break;
                }

                iter++;
            }

            it++;
        }

        //  LeaveCriticalSection(&crtc_sec);
    }

    //��Ϣ����ɾ��   ��Ϣ���д������ҳ�ͻ���
    //  EnterCriticalSection(&crtc_sec);
    DeleteByIo((ULONG_PTR)lp_io->pUserData);
    LeaveCriticalSection(&crtc_sec);
}

DWORD WINAPI CIOCP::TimeEmail(LPVOID lp_param)
{
    while(TRUE)
    {
        CIOCP* pThis = (CIOCP*)lp_param;
        pThis->objeamil.SendVecotrEmail();
        Sleep(5000);
    }

    return 1;
}
