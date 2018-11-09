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
          Shell_NotifyIcon(NIM_DELETE, &nid);
          //SendMessageA(WM_SYSCOMMAND, SC_CLOSE, 0);
          //SendMessageA(WM_SYSCOMMAND, SC_CLOSE, 0);
          Close();
        }
      else if(msg.pSender->GetName() == "minbtn")
        {
          Shell_NotifyIcon(NIM_ADD, &nid); //�����������ͼ��de����
          this->ShowWindow(false);
          //SendMessageA(WM_SYSCOMMAND, SC_MINIMIZE, 0);
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
              if(lp_start->fromtype == SOCKET_FROM_GAYWAY)
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
      else if(msg.pSender->GetName() == "checklamp")
        {
          int aa = 4;
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
  //addr.sin_addr.s_addr    = htons(ADDR);//inet_addr(ADDR);
  //addr.sin_port           = htons(PORT);
  addr.sin_addr.s_addr    = inet_addr(ip); //inet_addr(ip);//htons(ip);//inet_addr(ADDR);
  addr.sin_port           = htons(port);
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
      glog::GetInstance()->AddLine("listen fail! error info:%s", getErrorInfo(WSAGetLastError()).c_str());
      glog::traceErrorInfo("call bind()", WSAGetLastError());
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
                        lp_io->wsaBuf.len - 2 * (sizeof(SOCKADDR_IN) + 16),
                        //0,
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
BOOL CIOCP::HandleData(IOCP_IO_PTR lp_io, int nFlags, IOCP_KEY_PTR lp_key, DWORD dwByte)
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
          // m_pData->SetText(gstring::int2str((int)pListElement, 16).c_str());
          char vvv[20] = {0};
          int n = m_plistuser->GetCount();
          sprintf(vvv, "%d", n);
          pListElement->SetText(0, vvv);
          pListElement->SetText(1, szPeerAddress);
          sprintf(vvv, "%p", lp_io);
          pListElement->SetText(2, vvv);
          sprintf(vvv, "%p", lp_key);
          pListElement->SetText(3, vvv);
          glog::GetInstance()->AddLine("�ͻ�������:%s lp_io:%p     lp_key:%p", szPeerAddress, lp_io, lp_key);
          PostLog("�ͻ�������:%s lp_io:%p     lp_key:%p", szPeerAddress, lp_io, lp_key);
          lp_io->operation    = IOCP_READ;
        }
        break;

      case IOCP_COMPLETE_ACCEPT_READ:
        {
          if(SOCKET_STATE_CONNECT_AND_READ != lp_io->state)
            {
              lp_io->state = SOCKET_STATE_CONNECT_AND_READ;
            }

          char szPeerAddress[50] = {0};
          char szLocalAddress[50] = {0};
          SOCKADDR_IN *addrClient = NULL;
          SOCKADDR_IN *addrLocal = NULL;
          int nClientLen = sizeof(SOCKADDR_IN);
          int nLocalLen = sizeof(SOCKADDR_IN);
          lpGetAcceptExSockaddrs(lp_io->wsaBuf.buf, lp_io->wsaBuf.len - ((sizeof(SOCKADDR_IN) + 16) * 2), sizeof(SOCKADDR_IN) + 16, \
                                 sizeof(SOCKADDR_IN) + 16, (LPSOCKADDR*)&addrLocal, &nLocalLen, (LPSOCKADDR*)&addrClient, &nClientLen);
          char* ip1 = inet_ntoa(addrClient->sin_addr);
          sprintf(szPeerAddress, "%s:%d", ip1, addrClient->sin_port);
          char* localip = inet_ntoa(addrLocal ->sin_addr);
          SHORT localport = ntohs(addrLocal ->sin_port);
          sprintf(szLocalAddress, "%s:%d", localip, localport);
          glog::trace("����ip:%s Զ��ip:%s", szLocalAddress, szPeerAddress);
          CListTextElementUI* pListElement = new CListTextElementUI;
          m_plistuser->Add(pListElement);
          lp_io->pUserData = pListElement;
          char vvv[20] = {0};
          int n = m_plistuser->GetCount();
          sprintf(vvv, "%d", n);
          pListElement->SetText(0, vvv);
          pListElement->SetText(1, szPeerAddress);
          sprintf(vvv, "%p", lp_io);
          pListElement->SetText(2, vvv);
          sprintf(vvv, "%p", lp_key);
          pListElement->SetText(3, vvv);
          string data = gstring::char2hex(lp_io->buf, dwByte);
          // glog::GetInstance()->AddLine("������:%d ������:%s", dwByte, data.c_str());
          pListElement->SetText(5, data.c_str());
          pListElement->SetText(6, lp_io->buf);
          char lenstr[20] = {0};
          sprintf(lenstr, "%d", dwByte);
          pListElement->SetText(7, lenstr);
          glog::GetInstance()->AddLine("�ͻ�������:%s lp_io:%p     lp_key:%p", szPeerAddress, lp_io, lp_key);
          PostLog("�ͻ�������:%s lp_io:%p     lp_key:%p", szPeerAddress, lp_io, lp_key);
          string req = lp_io->buf;
          string res;
          int wsconn = wsHandshake(req, res);

          if(wsconn == WS_STATUS_CONNECT)
            {
              InitIoContext(lp_io);
              //lp_io->operation = IOCP_WRITE;
              lp_io->fromtype = SOCKET_FROM_WEBSOCKET;
              pListElement->SetText(8, "web�ͻ���(2)");
              strcpy(lp_io->gayway, "web�ͻ���(2)");
              memcpy(lp_io->buf, res.c_str(), res.size());
              PostLog("web������....");
              lp_io->wsaBuf.len = res.size();
              lp_io->operation = IOCP_WRITE;
              break;
            }

          if(checkFlag((BYTE*)lp_io->buf, dwByte))
            {
              buildcode((BYTE*)lp_io->buf, dwByte, lp_io);
              pListElement->SetText(8, lp_io->gayway);
              break;
            }

          lp_io->operation = IOCP_READ;
        }
        break;

      case IOCP_COMPLETE_READ:
        {
        }
        break;

      case IOCP_COMPLETE_WRITE:
        {
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

void CIOCP::DealWebsockMsg(IOCP_IO_PTR& lp_io, IOCP_KEY_PTR& lp_key, string jsondata, DWORD dwBytes)
{
  PostLog("����:%d web������:%s ", dwBytes, jsondata.c_str());
  Json::Value root;
  Json::Reader reader;

  if(reader.parse(jsondata.c_str(), root))
    {
      if(root.isObject())
        {
          Json::Value msgType = root["msg"];
          Json::Value isres = root["res"];

          if(msgType.isString())
            {
              Json::Value tosend = root["data"];

              if(tosend.isString() && tosend != "")
                {
                  if(msgType == "CheckLamp")
                    {
                      Json::Value pid = root["val"];

                      if(!pid.isNull())
                        {
                          memset(m_pid, 0, 216);
                          strcpy(m_pid, pid.asString().c_str());
                          PostThreadMessageA(ThreadId, WM_USER + 3, (WPARAM)m_pid, NULL);
                        }
                    }
                  else if(msgType == "CheckLoop")
                    {
                      Json::Value pid = root["val"];

                      if(!pid.isNull())
                        {
                          memset(m_pid, 0, 216);
                          strcpy(m_pid, pid.asString().c_str());
                          PostThreadMessageA(ThreadId, WM_USER + 4, (WPARAM)m_pid, NULL);
                        }
                    }
                  else if(msgType == "CheckOnline")
                    {
                      PostThreadMessageA(ThreadId, WM_USER + 5, NULL, NULL);
                    }

                  if(msgType == "AA" || msgType == "A4" || msgType == "A5" || msgType == "AC" || msgType == "00" || msgType == "FE" || msgType == "FF")
                    {
                      string data = tosend.asString();
                      data = gstring::replace(data, " ", "");
                      BYTE bitSend[512] = {0};
                      int len = hex2str(data, bitSend);

                      if(len > 0)
                        {
                          string addrarea = root["comaddr"].asString();
                          map<string, IOCP_IO_PTR>::iterator ite = m_mcontralcenter.find(addrarea);

                          if(ite != m_mcontralcenter.end())
                            {
                              BYTE seq = 0;
                              map<string, list<MSGPACK>>::iterator itmsg = m_MsgPack.find(addrarea);

                              if(itmsg != m_MsgPack.end())
                                {
                                  list<MSGPACK>v_msg = itmsg->second;
                                  seq = v_msg.begin() == v_msg.end() ? 0 : v_msg.back().seq + 1;
                                }
                              else
                                {
                                  seq = 0;
                                }

                              seq = seq > 0xf ? 0 : seq;

                              if(msgType != "00")
                                {
                                  _MSGPACK msg = {0};
                                  msg.lp_io = lp_io;
                                  msg.seq = seq;
                                  msg.timestamp = time(NULL);
                                  msg.root = root;

                                  if(itmsg == m_MsgPack.end())
                                    {
                                      list<_MSGPACK>v_msgpack;
                                      m_MsgPack.insert(pair<string, list<MSGPACK>>(addrarea, v_msgpack));
                                    }

                                  itmsg = m_MsgPack.find(addrarea);
                                  itmsg->second.push_back(msg);
                                  int ncount = itmsg->second.size();
                                  PostLog("����[%s] ��Ϣ����[%d] ֡���[%d]", itmsg->first.c_str(), itmsg->second.size(), seq);
                                  bitSend[13] =   bitSend[13] & 0xf0 | seq;
                                  bitSend[len - 2] = bitSend[len - 2] + seq;
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
BOOL CIOCP::IsBreakPack(IOCP_IO_PTR & lp_io, BYTE src[], int len)
{
  SHORT len1 = *(SHORT*)&src[1];
  SHORT len2 = *(SHORT*)&src[3];
  SHORT len3 = len1 >> 2;

  if((len3 > len - 8) && src[0] == 0x68)
    {
      return TRUE;
    }

  return FALSE;
  //if(len < 6)
  //  {
  //    return FALSE;
  //  }
  //if(src[0] == 0x68)
  //  {
  //    int aa = 44;
  //  }
  //SHORT len1 = *(SHORT*)&src[1];
  //SHORT len2 = *(SHORT*)&src[3];
  //if(src[0] == 0x68 && len1 == len2 && src[5] == 0x68)
  //  {
  //    BOOL bAllpack =  checkFlag(src, len);
  //    if(bAllpack == FALSE)
  //      {
  //        return TRUE;
  //      }
  //  }
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
  MSG msg;

  while(::GetMessage(&msg, NULL, 0, 0))
    {
      if(msg.message == WM_USER + 1)
        {
          //__try
          //  {
          IOCP_IO_PTR lo = (IOCP_IO_PTR)msg.wParam;
          // lp_this->PostLog("%X %X", lo, msg.lParam);

          if(lo)
            {
              time_t tmtamp;
              struct tm *tm1 = NULL;
              time(&tmtamp) ;
              tm1 = localtime(&tmtamp) ;
              tm1->tm_yday;
              BYTE ii = msg.lParam;
              int day = (int)(ii >> 4 & 0xf) * 10 + (int)(ii & 0xf);

              if(tm1->tm_mday == day)
                {
                  tm1->tm_mday--;
                  mktime(tm1);
                  char myday[30] = {0};
                  strftime(myday, sizeof(myday), "%Y-%m-%d", tm1);
                  char gayway[20] = {0};
                  strcpy(gayway, lo->gayway);

                  if(_stricmp(lo->day, myday) != 0)
                    {
                      vector<BYTE>v_b;
                      int n = 0;

                      if(lp_this->m_mcontralcenter.find(gayway) != lp_this->m_mcontralcenter.end())
                        {
                          //���������ѹ
                          lp_this->PostLog("����[%s] �������������ѹ����", gayway);
                          //unsigned char vol[24] = {0x68, 0x42, 0x00, 0x42, 0x00, 0x68, 0x04, comaddr[1], comaddr[0], comaddr[3], comaddr[2], 0x02, 0xAC, 0x7A, 0x00, 0x00, 0x04, 0x04, 0x00, 0x00, 0x01, 0x05, 0x55, 0x16};
                          BYTE vol[50] = {0};
                          n = lp_this->buidByte(gayway, 0x4, 0xAC, 0x71, 0, 0x404, v_b, vol);
                          lp_this->InitIoContext(lo);
                          memcpy(lo->buf, vol, n);
                          lo->wsaBuf.len = n; // sizeof(vol);
                          lo->wsaBuf.buf = lo->buf;
                          lo->operation = IOCP_WRITE;
                          lp_this->DataAction(lo, lo->lp_key);
                          //�����������
                          Sleep(10000);
                        }

                      if(lp_this->m_mcontralcenter.find(gayway) != lp_this->m_mcontralcenter.end())
                        {
                          lp_this->PostLog("����[%s]�������������������", gayway);
                          BYTE electric[50] = {0}; //{0x68, 0x32, 0x00, 0x32, 0x00, 0x68, 0x04, comaddr[1], comaddr[0], comaddr[3], comaddr[2], 0x02, 0xAC, 0x75, 0x00, 0x00, 0x20, 0x04, 0x66, 0x16 };
                          n = lp_this->buidByte(gayway, 0x4, 0xAC, 0x71, 0, 0x420, v_b, electric);
                          string data1 = gstring::char2hex((const char*)electric, n);
                          glog::GetInstance()->AddLine("�������Ͱ�:%s", data1.c_str());
                          lp_this->InitIoContext(lo);
                          memcpy(lo->buf, electric, n);
                          lo->wsaBuf.len = n;
                          lo->wsaBuf.buf = lo->buf;
                          lo->operation = IOCP_WRITE;
                          lp_this->DataAction(lo, lo->lp_key);
                          //���������й�����
                          Sleep(10000);
                        }

                      if(lp_this->m_mcontralcenter.find(gayway) != lp_this->m_mcontralcenter.end())
                        {
                          lp_this->PostLog("����[%s]�������������й���������", gayway);
                          unsigned char activepower[50] = {0}; // {0x68, 0x42, 0x00, 0x42, 0x00, 0x68, 0x04, comaddr[1], comaddr[0], comaddr[3], comaddr[2], 0x02, 0xAC, 0x76, 0x00, 0x00, 0x01, 0x03, 0x00, 0x00, 0x20, 0x04, 0x6B, 0x16 };
                          n = lp_this->buidByte(gayway, 0x4, 0xAC, 0x71, 0, 0x301, v_b, activepower);
                          lp_this->InitIoContext(lo);
                          memcpy(lo->buf, activepower, n);
                          lo->wsaBuf.len = n; //sizeof(activepower);
                          lo->wsaBuf.buf = lo->buf;
                          lo->operation = IOCP_WRITE;
                          lp_this->DataAction(lo, lo->lp_key);
                          ////�����ܹ�������
                          Sleep(10000);
                        }

                      if(lp_this->m_mcontralcenter.find(gayway) != lp_this->m_mcontralcenter.end())
                        {
                          lp_this->PostLog("����[%s]�������칦������", gayway);
                          unsigned char powerfactor[50] = {0}; //{0x68, 0x42, 0x00, 0x42, 0x00, 0x68, 0x04, comaddr[1], comaddr[0], comaddr[3], comaddr[2], 0x02, 0xAC, 0x78, 0x00, 0x00, 0x40, 0x03, 0x00, 0x00, 0x20, 0x04, 0xAC, 0x16 };
                          n = lp_this->buidByte(gayway, 0x4, 0xAC, 0x71, 0, 0x340, v_b, powerfactor);
                          lp_this->InitIoContext(lo);
                          memcpy(lo->buf, powerfactor, n);
                          lo->wsaBuf.len = n;
                          lo->wsaBuf.buf = lo->buf;
                          lo->operation = IOCP_WRITE;
                          lp_this->DataAction(lo, lo->lp_key);
                          //��������
                          Sleep(10000);
                        }

                      if(lp_this->m_mcontralcenter.find(gayway) != lp_this->m_mcontralcenter.end())
                        {
                          lp_this->PostLog("����[%s]����������������", gayway);
                          unsigned char power[50] = {0};//{0x68, 0x32, 0x00, 0x32, 0x00, 0x68, 0x04,  comaddr[1], comaddr[0], comaddr[3], comaddr[2], 0x02, 0xAC, 0x7B, 0x00, 0x00, 0x01, 0x05, 0x4E, 0x16 };
                          n = lp_this->buidByte(gayway, 0x4, 0xAC, 0x71, 0, 0x501, v_b, power);
                          lp_this->InitIoContext(lo);
                          memcpy(lo->buf, power, n);
                          lo->wsaBuf.len = n;
                          lo->wsaBuf.buf = lo->buf;
                          lo->operation = IOCP_WRITE;
                          lp_this->DataAction(lo, lo->lp_key);
                          strcpy(lo->day, myday);
                        }
                    }
                }
            }

          //  }
          //__except(GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
          //  {
          //  }
        }
      else if(msg.message == WM_USER + 2)
        {
          if(msg.wParam == 1000 && msg.lParam == 1000)
            {
              lp_this->PostLog("�������ݿ�");
              string pdir = lp_this->GetDataDir("config.ini");
              lp_this->dbopen->ReConnect(pdir);
            }
        }
      else if(msg.message == WM_USER + 3)
        {
          string pid = (char*)msg.wParam;
          lp_this->PostLog("projectId:%s", pid.c_str());
          string sql = "SELECT l_comaddr,l_code FROM   t_lamp tl\
					   WHERE  l_deplayment = 1 AND l_comaddr IN (SELECT comaddr AS l_comaddr FROM    t_baseinfo WHERE  online=1 AND  pid     = \'";
          sql.append(pid);
          sql.append("\') GROUP BY tl.l_comaddr,tl.l_code ");
          glog::trace("%s", sql.c_str());
          _RecordsetPtr rs = lp_this->dbopen->ExecuteWithResSQL(sql.c_str());
          map<string, list<SHORT>>m_lamp;

          while(rs && !rs->adoEOF)
            {
              try
                {
                  _variant_t l_comaddr = rs->GetCollect("l_comaddr");
                  _variant_t l_code = rs->GetCollect("l_code");
                  string comaddr = _com_util::ConvertBSTRToString(l_comaddr.bstrVal);
                  map<string, list<SHORT>>::iterator it = m_lamp.find(comaddr);

                  if(it != m_lamp.end())
                    {
                      it->second.push_back(l_code);
                    }
                  else
                    {
                      list<SHORT>v_l_code;
                      v_l_code.push_back(l_code);
                      m_lamp.insert(pair<string, list<SHORT>>(comaddr, v_l_code));
                    }

                  rs->MoveNext();
                }
              catch(_com_error e)
                {
                  break;
                }
            }

          if(m_lamp.size() > 0)
            {
              string sql1 = "UPDATE t_lamp SET presence = 0,electric=\'\',activepower=\'\',temperature=\'\',voltage=\'\',l_fault=0,\
							  l_faultdesc=\'\',status1=0,status2=0,status3=0,status4=0,l_value=0 WHERE  l_deplayment = 1 AND l_comaddr IN (SELECT comaddr AS l_comaddr FROM   t_baseinfo WHERE  pid = \'";
              sql1.append(pid);
              sql1.append("\'");
              sql1.append(")");
              _RecordsetPtr rs = lp_this->dbopen->ExecuteWithResSQL(sql1.c_str());
            }

          for(auto it = m_lamp.begin(); it != m_lamp.end(); it++)
            {
              list<SHORT>v_s = it->second;
              string l_comaddr = it->first;
              vector<BYTE>v_param;
              int z = 0;

              for(auto it = v_s.begin(); it != v_s.end();)
                {
                  SHORT s = *it;

                  if(z == 0)
                    {
                      v_param.push_back(v_s.size());
                    }

                  v_s.erase(it++);
                  z++;
                  BYTE a =  s >> 8 & 0x00ff;
                  BYTE b =  s & 0x00ff;
                  v_param.push_back(b);
                  v_param.push_back(a);

                  if(it == v_s.end())
                    {
                      BYTE vol[1024] = {0};
                      int  n = lp_this->buidByte(l_comaddr, 0x4, 0xAC, 0x71, 0, 0x0040, v_param, vol);
                      string  hh = gstring::char2hex((char*)vol, n);
                      glog::GetInstance()->AddLine("%s", hh.c_str());
                      map<string, IOCP_IO_PTR>::iterator itegay = lp_this->m_mcontralcenter.find(l_comaddr);

                      if(itegay != lp_this->m_mcontralcenter.end())
                        {
                          IOCP_IO_PTR lo = itegay->second;
                          lp_this->InitIoContext(lo);
                          memcpy(lo->buf, vol, n);
                          lo->wsaBuf.len = n; // sizeof(vol);
                          lo->wsaBuf.buf = lo->buf;
                          lo->operation = IOCP_WRITE;
                          lp_this->DataAction(lo, lo->lp_key);
                          Sleep(100);
                        }

                      break;
                    }

                  if(z == 50)
                    {
                      BYTE vol[1024] = {0};
                      v_param[0] = 50;
                      int  n = lp_this->buidByte(l_comaddr, 0x4, 0xAC, 0x71, 0, 0x0040, v_param, vol);
                      string  hh = gstring::char2hex((char*)vol, n);
                      glog::GetInstance()->AddLine("%s", hh.c_str());
                      map<string, IOCP_IO_PTR>::iterator itegay = lp_this->m_mcontralcenter.find(l_comaddr);

                      if(itegay != lp_this->m_mcontralcenter.end())
                        {
                          IOCP_IO_PTR lo = itegay->second;
                          lp_this->InitIoContext(lo);
                          memcpy(lo->buf, vol, n);
                          lo->wsaBuf.len = n; // sizeof(vol);
                          lo->wsaBuf.buf = lo->buf;
                          lo->operation = IOCP_WRITE;
                          lp_this->DataAction(lo, lo->lp_key);
                          Sleep(100);
                        }

                      z = 0;
                    }
                }
            }
        }
      else if(msg.message == WM_USER + 5)
        {
          for(auto ite = lp_this->m_mcontralcenter.begin(); ite != lp_this->m_mcontralcenter.end(); ite++)
            {
              string comaddr = ite->first;
              lp_this->setOnline(comaddr, 1);
            }
        }
    }

  return 1;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////
/*-----------------------------------------------------------------------------
    //-----------       Created with 010 Editor        -----------
    //------         www.sweetscape.com/010editor/          ------
    //
    // File    : Unti--------------------------------------------------------
�������ܣ���ʼ����ɶ˿ڼ���ص����ж�����������ÿһ��10������.
����˵����
�������أ��ɹ���TRUE��ʧ�ܣ�FALSE
-------------------------------------------------------------------------------------------*/
BOOL CIOCP::InitAll()
{
  CoInitialize(NULL);
  //------------------tled1
  // Address : 0 (0x0)
  // Size    : 16 (0x10)
  //------------------------------------------------------------
  //unsigned char hexData[16] = {
  // 0x81, 0xFE, 0x01, 0x3D, 0x69, 0x35, 0x6B, 0x58, 0x12, 0x17, 0x09, 0x3D, 0x0E, 0x5C, 0x05, 0x7A
  //};
  //string out="";
  //BOOL fullpack=FALSE;
  //wsDecodeFrame((char*)hexData,out,16,fullpack);
  //objeamil.SetEmailTitle(string("aaaa"));
  //objeamil.AddTargetEmail(string("277402131@qq.com"));
  //objeamil.SetContent(string("asdfsdfsdfsdfsdf"));
  //objeamil.SendVecotrEmail();
  //CSmtp smtp(25, "smtp.126.com", "z277402131@126.com", /*��������ַ*/"z277402131",/*��������*/"zhizhuchun@qq.com",/*Ŀ�������ַ*/"TEST",/*����*/"���Բ��ԣ��յ���ظ���"  /*�ʼ�����*/);
  //string filePath("D:\\����.txt");
  //smtp.AddAttachment(filePath);
  /*�����Ե���CSmtp::DeleteAttachment����ɾ������������һЩ�������Լ���ͷ�ļ���!*/
  //int err;
  //if((err = smtp.SendEmail_Ex()) != 0)
  //  {
  //    if(err == 1)
  //      cout << "����1: �������粻��ͨ������ʧ��!" << endl;
  //    if(err == 2)
  //      cout << "����2: �û�������,��˶�!" << endl;
  //    if(err == 3)
  //      cout << "����3: �û����������˶�!" << endl;
  //    if(err == 4)
  //      cout << "����4: ���鸽��Ŀ¼�Ƿ���ȷ���Լ��ļ��Ƿ����!" << endl;
  //  }
// return 0;
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
  //------------------------------------------------------------
  //-----------       Created with 010 Editor        -----------
  //------         www.sweetscape.com/010editor/          ------
  //
  // File    : Untitled1
  // Address : 0 (0x0)
  // Size    : 63 (0x3F)
  //------------------------------------------------------------
  //unsigned char hexData[63] =
  //{
  //  0xB8, 0x04, 0x0E, 0x22, 0xA9, 0x14, 0x0C, 0x33, 0xE8, 0x11, 0x1E, 0x24, 0xB9, 0x04, 0x0E, 0x23,
  //  0xA9, 0x14, 0x0E, 0x33, 0xB9, 0x15, 0x1E, 0x23, 0xBA, 0x04, 0x0D, 0x25, 0xA9, 0x14, 0x0E, 0x33,
  //  0xB9, 0x14, 0x1E, 0x24, 0xB9, 0x04, 0x0F, 0x25, 0xA9, 0x06, 0x12, 0x31, 0xE5, 0x41, 0x50, 0x31,
  //  0xB3, 0x12, 0x07, 0x3F, 0xAB, 0x41, 0x50, 0x77, 0xAB, 0x1E, 0x1C, 0x25, 0xC8, 0x06, 0x43
  //};
  //string outstring = "";
  //BOOL bfullpack = TRUE;
  //int n1 = wsDecodeFrame((char*)hexData, outstring, sizeof(hexData), bfullpack);
  //------------------------------------------------------------
  //-----------       Created with 010 Editor        -----------
  //------         www.sweetscape.com/010editor/          ------
  //------------------------------------------------------------
  //-----------       Created with 010 Editor        -----------
  //------         www.sweetscape.com/010editor/          ------
  //
  // File    : Untitled1
  // Address : 0 (0x0)
  // Size    : 31 (0x1F)
  //------------------------------------------------------------
  //unsigned char hexData[31] =
  //{
  //  0x68, 0x5E, 0x00, 0x5E, 0x00, 0x68, 0xC4, 0x02, 0x17, 0x01, 0x01, 0x04, 0x0E, 0x66, 0x00, 0x00,
  //  0x01, 0x00, 0x2C, 0x09, 0x20, 0x10, 0x31, 0x10, 0x18, 0x37, 0x00, 0x01, 0x00, 0x4E, 0x16
  //};
  //IOCP_IO_PTR pp;
  //buildcode(hexData, sizeof(hexData), pp);
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
  HANDLE hTreadTime = CreateThread(NULL, NULL, TimeThread, (LPVOID)this, NULL, &ThreadId);
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

      if(lp_start->fromtype == SOCKET_FROM_GAYWAY)
        {
          if(op != 0xffffffff)
            {
              int len = op - lp_start->timelen;

              if(len / 60 >= 2)
                {
                  // map<string, IOCP_IO_PTR>::iterator  it; m_mcontralcenter.find(lp_start)
                  glog::GetInstance()->AddLine("ͨ��ָ��:%p ����:%s ��ʱ%d�� �����ر� ��������:%d", lp_start, lp_start->gayway, len, m_io_group.GetCount());
                  //string sql = "update t_baseinfo set online=0 where comaddr=\'";
                  //sql.append(lp_start->gayway);
                  //sql.append("\'");
                  //glog::trace("\n%s", sql.c_str());
                  //_RecordsetPtr rs =   dbopen->ExecuteWithResSQL(sql.c_str());
                  EnterCriticalSection(&crtc_sec);
                  map<string, IOCP_IO_PTR>::iterator  it =  m_mcontralcenter.find(lp_start->gayway);

                  if(it == m_mcontralcenter.end())
                    {
                      setOnline(lp_start->gayway, 0);
                    }
                  else
                    {
                      if(it->second == lp_start)
                        {
                          setOnline(lp_start->gayway, 0);
                          m_mcontralcenter.erase(it);
                        }
                    }

                  //for(it = m_mcontralcenter.begin(); it != m_mcontralcenter.end();)
                  //{
                  //    if(it->second == lp_start)
                  //    {
                  //        m_mcontralcenter.erase(it++);   //erase ɾ����ָ����һ��������
                  //    }
                  //    else
                  //    {
                  //        it++;
                  //    }
                  //}
                  LeaveCriticalSection(&crtc_sec);
                  closesocket(lp_start->socket);
                  break;
                }
            }
        }

      if(lp_start->state == SOCKET_STATE_CONNECT || lp_start->state == SOCKET_STATE_CONNECT_AND_READ)
        {
          if(lp_start->fromtype == SOCKET_FROM_WEBSOCKET)
            {
              int len = op - lp_start->timelen;

              if(len / 60 >= 30)
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

      if(dwBytes == 0 && lp_key == 0 && lp_ov == 0)
        {
          int errcode = WSAGetLastError();
          lp_this->PostLog("��ɶ˿��ѹر�");
          break;
        }

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
          continue;
          //�黹IO�����continue;
        }

      //if (bRet==TRUE&&dwBytes==0)
      //{
      //  lp_this->PostLog("bRet=1 dwByte=0 errorcode:%s",lp_this->getErrorInfo(WSAGetLastError()).c_str());
      //}

      //���������һ����ɶ˿ڵ�һ��socket������ر��ˣ���GetQueuedCompletionStatus����ERROR_SUCCESS��Ҳ��0��,����lpNumberOfBytes����0
      //�˳�����
      if((lp_io != NULL) && (IOCP_ACCEPT != lp_io->operation) && (0 == dwBytes))
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
        }

      switch(lp_io->operation)
        {
          case IOCP_ACCEPT:
            {
              //lp_io->state = SOCKET_STATE_CONNECT;
              //if(dwBytes > 0)     lp_io->state = SOCKET_STATE_CONNECT_AND_READ;
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
                  //lp_io->wsaBuf.len = dwBytes;
                  lp_this->HandleData(lp_io, IOCP_COMPLETE_ACCEPT_READ, lp_new_key, dwBytes);
                  bRet = lp_this->DataAction(lp_io, lp_new_key);
                }
              else
                {
                  lp_this->HandleData(lp_io, IOCP_COMPLETE_ACCEPT, lp_new_key, dwBytes);
                  bRet = lp_this->DataAction(lp_io, lp_new_key);
                }
            }
            break;

          case IOCP_READ:
            {
              lp_this->dealRead(lp_io, lp_key, dwBytes);
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
              lp_this->HandleData(lp_io, IOCP_COMPLETE_WRITE, lp_new_key, dwBytes);
              bRet = lp_this->DataAction(lp_io, lp_new_key);
            }
            break;

          default:
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
int CIOCP::wsDecodeFrame(char inFrame[], string & outMessage, int len, BOOL & bBreakPack)
{
  int ret = WS_OPENING_FRAME;
  const char *msg = inFrame;
  const int frameLength = len;

  if(frameLength < 2)
    {
      ret = WS_ERROR_FRAME;
      return ret;
    }

  BYTE Fin =  msg[0] >> 7 & 1;
  BYTE RSV1 = msg[0] >> 6 & 0x01;
  BYTE RSV2 = msg[0] >> 5 & 0x01;
  BYTE RSV3 = msg[0] >> 4 & 0x01;
  BYTE Mask =  msg[1] >> 7 & 0x01;

  //FIN:1λ������������Ϣ�Ƿ���������Ϊ1�����ϢΪ��Ϣβ��,���Ϊ�����к������ݰ�;
  if(Fin != 1)
    {
      ret = WS_ERROR_FRAME;
      return ret;
    }

  // �����չλ������
  if(RSV1 == 1 || RSV2 == 1 || RSV3 == 1)
    {
      ret = WS_ERROR_FRAME;
      return ret;
    }

  // maskλ, Ϊ1��ʾ���ݱ�����
  if(Mask != 1)
    {
      ret = WS_ERROR_FRAME;
      return ret;
    }

  BYTE opcode = msg[0] & 0x0f;
  // ������
  uint16_t payloadLength = 0;
  uint8_t payloadFieldExtraBytes = 0;

  if(opcode == WS_TEXT_FRAME)
    {
      // ����utf-8������ı�֡
      payloadLength = static_cast<uint16_t >(msg[1] & 0x7f);

      if(payloadLength == 0x7e)   //0111 1110     //126 7e  �������ֽ��ǳ��� :  127  7f �������ֽ��ǳ���
        {
          uint16_t payloadLength16b = 0;
          payloadFieldExtraBytes = 2;
          memcpy(&payloadLength16b, &msg[2], payloadFieldExtraBytes);
          payloadLength = ntohs(payloadLength16b);
        }
      else if(payloadLength == 0x7f)
        {
          // ���ݹ���,�ݲ�֧��
          uint32_t payloadLength32b = 0;
          payloadFieldExtraBytes = 4;
          memcpy(&payloadLength32b, &msg[2], payloadFieldExtraBytes);
          payloadLength = ntohl(payloadLength32b);
          //ret = WS_ERROR_FRAME;
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
      if(payloadLength > (len - 2 - payloadFieldExtraBytes - 4))
        {
          bBreakPack = TRUE;
          return ret;
        }

      // header: 2�ֽ�, masking key: 4�ֽ�
      const char *maskingKey = &msg[2 + payloadFieldExtraBytes];
      char *payloadData = new char[payloadLength + 1];
      memset(payloadData, 0, payloadLength + 1);
      memcpy(payloadData, &msg[2 + payloadFieldExtraBytes + 4], payloadLength);

      for(int i = 0; i < payloadLength; i++)
        {
          payloadData[i] = payloadData[i] ^ maskingKey[i % 4];
        }

      outMessage = payloadData;
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

BOOL CIOCP::checkFlag(BYTE vv[], int len)
{
  if(len < 6)
    {
      return FALSE;
    }

  if(vv[0] == 0x68 && vv[5] == 0x68 && vv[len - 1] == 0x16)
    {
      SHORT len1 = *(SHORT*)&vv[1];
      SHORT len2 = *(SHORT*)&vv[3];
      SHORT len11 = len1 >> 2;
      SHORT len22 = len2 >> 2;

      if(len11 == len22 && len22 == len - 8)
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
void CIOCP::buildcode(BYTE src[], int srclen, IOCP_IO_PTR & lp_io)
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

          if(DA[0] == 0 && DA[1] == 0 && DT[1] == 0 && DT[0] == 1) //DT1��
            {
              lp_io->fromtype = SOCKET_FROM_GAYWAY;
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

              BYTE des[50] = {0};
              int deslen = 0;
              buildConCode(src, des, deslen, 1);
              InitIoContext(lp_io);
              memcpy(lp_io->buf, des, deslen);
              lp_io->wsaBuf.len = deslen;
              lp_io->operation = IOCP_WRITE;
            }
          else if(DA[0] == 0 && DA[1] == 0 && DT[1] == 0 && DT[0] == 4)
            {
              map<string, IOCP_IO_PTR>::iterator it = m_mcontralcenter.find(addrarea);

              if(it != m_mcontralcenter.end())
                {
                  BYTE day = src[22];
                  PostLog("����[%s] ����", addrarea);
                  lp_io->loginstatus = SOCKET_STATUS_LOGIN;
                  BYTE des[50] = {0};
                  int deslen = 0;
                  buildConCode(src, des, deslen, 1);
                  InitIoContext(lp_io);
                  memcpy(lp_io->buf, des, deslen);
                  lp_io->wsaBuf.len = deslen;
                  lp_io->operation = IOCP_WRITE;
                  PostThreadMessageA(ThreadId, WM_USER + 1, (WPARAM)lp_io, (LPARAM)day);
                }
            }
        }
    }
  else if(AFN == 0x00 || AFN == 0xAA || AFN == 0xA4 || AFN == 0xFF || AFN == 0xFE) //ȫ��ȷ��
    {
      if(DirPrmCode == 0x80 && con == 0x0 && FC == 0x8)   //  ���� �Ӷ� ��Ӧ֡   0x80 ���� �Ӷ�
        {
          // PostLog("��Ӧ AFN:%d", AFN);
          map<string, list<MSGPACK>>::iterator itmsg = m_MsgPack.find(addrarea);
          Json::Value root;

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
                      root = msgEnd.root;

                      if(itmsg->second.size() > 0)
                        {
                          MSGPACK msgEnd = itmsg->second.back();
                          time_t tmnow = time(NULL);
                          float t = (float)(tmnow - msgEnd.timestamp) / 60;
                          PostLog("�����Ϣ����֡:%d פ��������%0.2f��", msgEnd.seq, t);

                          if(t > 1)
                            {
                              itmsg->second.clear();
                            }
                        }
                    }
                  else
                    {
                      //EnterCriticalSection(&crtc_sec);
                      list<MSGPACK>::iterator it = itmsg->second.begin();
                      time_t tmnow = time(NULL);

                      while(it != itmsg->second.end())
                        {
                          MSGPACK pack = *it;
                          float t = float(tmnow - pack.timestamp) / 60;

                          if(t > 1)
                            {
                              PostLog("��Ϣ����֡:%d פ��������%0.2f��", pack.seq, t);
                              itmsg->second.erase((it++));
                              continue;
                            }

                          if(pack.seq == frame)
                            {
                              root = pack.root;
                              lp_io1 = pack.lp_io;
                              itmsg->second.erase((it++));
                            }

                          it++;
                        }

                      //LeaveCriticalSection(&crtc_sec);
                      //                list<MSGPACK>::reverse_iterator it = itmsg->second.rbegin();
                      //                while(it != itmsg->second.rend())
                      //                {
                      //time_t tmnow = time(NULL);
                      //                    MSGPACK pack = *it;
                      //                    float t = float(tmnow - pack.timestamp) / 60;
                      //                    PostLog("��Ϣ����֡:%d פ��������%0.2f��",pack.seq, t);
                      //                    if(t > 1)
                      //                    {
                      //                        itmsg->second.erase((++it).base());
                      //                        continue;
                      //                    }
                      //                    if(pack.seq == frame)
                      //                    {
                      //                        lp_io1 = pack.lp_io;
                      //                        itmsg->second.erase((++it).base());
                      //                        break;
                      //                    }
                      //                    it++;
                      //                }
                      //  LeaveCriticalSection(&crtc_sec);
                    }

                  if(lp_io1 != NULL)
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

          if(DA[0] == 0 && DA[1] == 0 && DT[0] == 0x40 && DT[1] == 0x00)
            {
              int z = 18;
              BYTE len = src[z];
              z = z + 1;

              for(int i = 0; i < len; i++)
                {
                  int l_code = src[z + 1] * 256 + src[z];
                  //��ѹ
                  z = z + 2;
                  int bw = src[z + 1] >> 4 & 0xf;
                  int sw = src[z + 1] & 0xf;
                  int gw = src[z] >> 4 & 0xf;
                  int sfw = src[z] & 0xf;
                  char voltage[20] = {0};
                  sprintf(voltage, "%d%d%d.%d", bw, sw, gw, sfw);
                  //����
                  z = z + 2;
                  sw = src[z + 1] >> 4 & 0xf;
                  gw = src[z + 1] & 0xf;
                  sfw = src[z] >> 4 & 0xf;
                  int bfw = src[z] & 0xf;
                  char electric[20] = {0};
                  sprintf(electric, "%d%d.%d%d", sw, gw, sfw, bfw);
                  //�й�����
                  z = z + 2;
                  int qw = src[z + 3] >> 4 & 0xf;
                  bw = src[z + 3] & 0xf;
                  sw = src[z + 2] >> 4 & 0xf;
                  gw = src[z + 2] & 0xf;
                  sfw = src[z + 1] >> 4 & 0xf;
                  bfw = src[z + 1] & 0xf;
                  int qfw = src[z] >> 4 & 0xf;
                  int wfw = src[z] & 0xf;
                  char activepower[20] = {0};
                  sprintf(activepower, "%d%d%d.%d%d%d%d", qw, bw, gw, sfw, bfw, qfw, wfw);
                  //�ƿ���״̬
                  z = z + 4;
                  int s1 = src[z];
                  int s2 = src[z + 1];
                  int s3 = src[z + 2];
                  int s4 = src[z + 3];
                  string faultdesc = "";

                  for(int u = 0; u < 8; u++)
                    {
                      //s3>>u&0x1;
                      if(s3 >> u & 0x1 == 0x1)
                        {
                          faultdesc = faultdesc + v_fault[u];
                        }
                    }

                  //����ֵ
                  z = z + 4;
                  int l_value = src[z];
                  z = z + 1;
                  //�¶�
                  char temperature[20] = {0};
                  SHORT temp = src[z + 1];
                  src[z] == 1 ? sprintf(temperature, "-%d", temp) : sprintf(temperature, "%d", temp);
                  //����ʱ��
                  z = z + 2;
                  BYTE ms = src[z + 3] >> 4 & 0xf;
                  BYTE mg = src[z + 3] & 0xf;
                  BYTE ds = src[z + 2] >> 4 & 0xf;
                  BYTE dg = src[z + 2] & 0xf;
                  BYTE hs = src[z + 1] >> 4 & 0xf;
                  BYTE hg = src[z + 1] & 0xf;
                  BYTE mins = src[z] >> 4 & 0xf;
                  BYTE ming = src[z] & 0xf;
                  BYTE m = ms * 10 + mg;
                  BYTE d = ds * 10 + dg;
                  BYTE h = hs * 10 + hg;
                  BYTE min = mins * 10 + ming;
                  time_t timecz = GetTickCZ(m, d, h, min);
                  // PostLog("����ʱ���볭��ʱ�����:%d����", timecz);
                  int ipresence = 0;

                  if(0 <= timecz && timecz <= 5)
                    {
                      ipresence = 1;
                    }
                  else
                    {
                      strcpy(voltage, "000.0");
                      strcpy(electric, "00.00");
                      strcpy(activepower, "000.0");
                      l_value = 0;
                      strcpy(activepower, "000.0000");
                    }

                  char readtime[30] = {0};
                  sprintf(readtime, "%d%d-%d%d %d%d:%d%d", ms, mg, ds, dg, hs, hg, mins, ming);
                  PostLog("����[%s] װ�ú�:%d ��ѹ:%s ����:%s �й�����:%s �¶�:%s ����ֵ:%d �������ʱ��:%s �ֲ�:%d status1:%d status2:%d status3:%d status4:%d ��������:%s", \
                          addrarea, l_code, voltage, electric, activepower, temperature, l_value, readtime, timecz, s1, s2, s3, s4, faultdesc.c_str());
                  z = z + 4;
                  map<string, _variant_t>m_var;
                  //dbopen->GetUpdateSql()
                  _variant_t  vvoltage(voltage);
                  _variant_t  velectric(electric);
                  _variant_t  vactivepower(activepower);
                  _variant_t  vl_value(l_value);
                  _variant_t  vtemperature(temperature);
                  _variant_t  vfaultdesc(faultdesc.c_str());
                  _variant_t  vs1(s1);
                  _variant_t  vs2(s2);
                  _variant_t  vs3(s3);
                  _variant_t  vs4(s4);
                  int ifault = faultdesc != "" ? 1 : 0;
                  _variant_t  vfault(ifault);
                  _variant_t  vreadtime(readtime);
                  _variant_t  presence(readtime);
                  m_var.insert(pair<string, _variant_t>("voltage", voltage));
                  m_var.insert(pair<string, _variant_t>("electric", velectric));
                  m_var.insert(pair<string, _variant_t>("activepower", vactivepower));
                  m_var.insert(pair<string, _variant_t>("temperature", vtemperature));
                  m_var.insert(pair<string, _variant_t>("l_faultdesc", vfaultdesc));
                  m_var.insert(pair<string, _variant_t>("newlyread", vreadtime));
                  m_var.insert(pair<string, _variant_t>("l_fault", vfault));
                  m_var.insert(pair<string, _variant_t>("status1", vs1));
                  m_var.insert(pair<string, _variant_t>("status2", vs2));
                  m_var.insert(pair<string, _variant_t>("status3", vs3));
                  m_var.insert(pair<string, _variant_t>("status4", vs4));
                  m_var.insert(pair<string, _variant_t>("l_value", vl_value));
                  m_var.insert(pair<string, _variant_t>("presence", ipresence));
                  string where = "";
                  where.append(" where l_code=");
                  where.append(gstring::int2str(l_code).c_str());   //�Ժ����
                  where.append(" and l_comaddr='");
                  where.append(addrarea);
                  where.append("\'");
                  string sql = dbopen->GetUpdateSql(m_var, "t_lamp", where);
                  _RecordsetPtr rs =   dbopen->ExecuteWithResSQL(sql.c_str());
                  //PostLog("sql:%s", sql.c_str());
                }
            }

          if(DA[0] == 0 && DA[1] == 0 && DT[0] == 0x20 && DT[1] == 0x00)
            {
              return;
            }

          map<string, list<MSGPACK>>::iterator itmsg = m_MsgPack.find(addrarea);
          Json::Value root;

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
                      root = msgEnd.root;

                      if(itmsg->second.size() > 0)
                        {
                          MSGPACK msgEnd = itmsg->second.back();
                          time_t tmnow = time(NULL);
                          float t = (float)(tmnow - msgEnd.timestamp) / 60;
                          PostLog("�����Ϣ����֡:%d פ��������%0.2f��", msgEnd.seq, t);

                          if(t > 1)
                            {
                              itmsg->second.clear();
                            }
                        }
                    }
                  else
                    {
                      //EnterCriticalSection(&crtc_sec);
                      list<MSGPACK>::iterator it = itmsg->second.begin();
                      time_t tmnow = time(NULL);

                      while(it != itmsg->second.end())
                        {
                          MSGPACK pack = *it;
                          float t = float(tmnow - pack.timestamp) / 60;

                          if(t > 1)
                            {
                              PostLog("��Ϣ����֡:%d פ��������%0.2f��", pack.seq, t);
                              itmsg->second.erase((it++));
                              continue;
                            }

                          if(pack.seq == frame)
                            {
                              root = pack.root;
                              lp_io1 = pack.lp_io;
                              itmsg->second.erase((it++));
                            }

                          it++;
                        }
                    }

                  if(lp_io1 != NULL)
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
  else if(AFN == 0x0E)             //�����͹����¼�
    {
      string hexdata = gstring::char2hex((const char*)src, srclen);
      glog::GetInstance()->AddLine("����:%s", hexdata.c_str());
      BYTE    con =    src[13] & 0x10;
      BYTE   DirPrmCode = src[6] & 0xc0;   //����  �Ӷ�          ����  ����    1100  c0
      BYTE   FC = src[6] & 0xF; //���������Ĺ�����
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
              // string hexdata = gstring::char2hex((const char*)&src[j + 2], datalen);
              sprintf(cmin, "%d%d", min >> 4 & 0x0f, min & 0xf);
              sprintf(chour, "%d%d", hour >> 4 & 0x0f, hour & 0xf);
              sprintf(cday, "%d%d", day >> 4 & 0x0f, day & 0xf);
              sprintf(cmonth, "%d%d", month >> 4 & 0x0f, month & 0xf);
              sprintf(cyear, "20%d%d", year >> 4 & 0x0f, year & 0xf);
              char date[30] = {0};
              sprintf(date, "%s-%s-%s %s:%s", cyear, cmonth, cday, chour, cmin);
              char err[20] = {0};
              sprintf(err, "ERC%d", errcode);
              map<string, _variant_t>m_var;
              _variant_t  vdate(date);
              _variant_t  vcomaddr(addrarea);
              _variant_t  verr(err);
              _variant_t  vdata(hexdata.c_str());
              _variant_t  vlen(datalen);
              m_var.insert(pair<string, _variant_t>("f_day", vdate));
              m_var.insert(pair<string, _variant_t>("f_comaddr", vcomaddr));
              m_var.insert(pair<string, _variant_t>("f_type", verr));
              m_var.insert(pair<string, _variant_t>("f_len", srclen));
              m_var.insert(pair<string, _variant_t>("f_data", vdata));
              char emailinfo[512] = {0};
              char l_factory[30] = {0};
              string sql3 = "SELECT tp.name as proname FROM t_project AS tp,t_baseinfo tb WHERE tb.pid=tp.code AND tb.comaddr= \'";
              sql3.append(addrarea);
              sql3.append("\'");
              string pid = "";
              _RecordsetPtr rspid = dbopen->ExecuteWithResSQL(sql3.c_str());

              while(rspid && !rspid->adoEOF)
                {
                  _variant_t vpid = rspid->GetCollect("proname");
                  pid  = _com_util::ConvertBSTRToString(vpid.bstrVal);
                  rspid->MoveNext();
                  break;
                }

              if(errcode >= 43 && errcode <= 48 || errcode == 50 || errcode == 51)
                {
                  if(errcode == 43 || errcode == 44)
                    {
                      string sql2 = "select * from t_lamp where l_comaddr=\'";
                      sql2.append(addrarea);
                      sql2.append("\' and l_code=");
                      int setcode1 = *(SHORT*)&src[25];
                      char l_code[20] = {0};
                      sprintf(l_code, "%d", setcode1);
                      sql2.append(l_code);
                      _RecordsetPtr rs1 = dbopen->ExecuteWithResSQL(sql2.c_str());

                      while(!rs1->adoEOF)
                        {
                          _variant_t vfactorycode = rs1->GetCollect("l_factorycode");
                          string  l_factorycode = _com_util::ConvertBSTRToString(vfactorycode.bstrVal);
                          strcpy(l_factory, l_factorycode.c_str());
                          rs1->MoveNext();
                          break;
                        }
                    }

                  int uptype = 1;

                  if(errcode == 43 || errcode == 45)
                    {
                      uptype = 0;
                    }

                  errcode % 2 == 1 ? 0 : 1;
                  int status1 = src[27];
                  int status2 = src[28];
                  _variant_t  vstatus1(status1);
                  _variant_t  vstatus2(status2);
                  _variant_t  vuptype(uptype);
                  int setcode = *(SHORT*)&src[25];
                  _variant_t  vsetcode(setcode);
                  m_var.insert(pair<string, _variant_t>("f_uptype", vuptype));
                  m_var.insert(pair<string, _variant_t>("f_status1", vstatus1));
                  m_var.insert(pair<string, _variant_t>("f_status2", vstatus2));
                  m_var.insert(pair<string, _variant_t>("f_setcode", vsetcode));
                  string setname = "";

                  switch(errcode)
                    {
                      case 43:
                        setname = "�ƿ���Ԥ���¼�";
                        break;

                      case 44:
                        setname = "�ƿ��������¼�";
                        break;

                      case 45:
                        setname = "��·Ԥ���¼�";
                        break;

                      case 46:
                        setname = "��·�����¼�";
                        break;

                      case 47:
                        setname = "������ſ��¼�";
                        break;

                      case 48:
                        setname = "�豸ͨ���쳣�����¼�";
                        break;

                      case 50:
                        setname = "��·����״̬�¼�";
                        break;

                      case 51:
                        setname = "����乩���쳣�¼�";
                        break;
                    }

                  _variant_t  vcomment(setname.c_str());
                  _variant_t  vfactorycode1(l_factory);
                  m_var.insert(pair<string, _variant_t>("f_comment", vcomment));
                  m_var.insert(pair<string, _variant_t>("l_factorycode", vfactorycode1));
                  sprintf(emailinfo, "��Ŀ:%s\r\n����:%s\r\n�¼�����:ERC%d\r\nװ�ú�:%d\r\n�ƾ߱��:%s\r\n״̬��1:%d \r\n״̬��2:%d \r\n�¼�����:%s \r\n����:%s \r\n�ϱ�����:%s", \
                          pid.c_str(), addrarea, (int)errcode, setcode, l_factory, status1, status2, setname.c_str(), hexdata.c_str(), date);
                }
              else
                {
                  string setname = "";

                  switch(errcode)
                    {
                      case 49:
                        setname = "�������͵ƿ���ͨѸ�ж�";
                        break;

                      case 52:
                        setname = "�ƿ���״̬�ı��ϱ�";
                        break;

                      case 53:
                        setname = "�������͵ƿ���ͨѸ�ָ�";
                        break;

                      case 54:
                        setname = "�������·����";
                        break;
                    }

                  _variant_t  vcomment(setname.c_str());
                  m_var.insert(pair<string, _variant_t>("f_comment", vcomment));
                  sprintf(emailinfo, "��Ŀ:%s\r\n����:%s\r\n�¼�����:ERC%d\r\n  �¼�����:%s \r\n����:%s \r\n", pid.c_str(), addrarea, (int)errcode, setname.c_str(), hexdata);
                }

              string sql = dbopen->GetInsertSql(m_var, "t_fault");
              PostLog("sql:%s", sql.c_str());
              _RecordsetPtr rs1 = dbopen->ExecuteWithResSQL(sql.c_str());

              if(!rs1)
                {
                  glog::GetInstance()->AddLine("���뱨���¼�ʧ��:sql %s", sql.c_str());
                }

              if(rs1)
                {
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
                      objeamil.SetEmailTitle(string("���ع��ϱ���"));
                      objeamil.SetContent(string(emailinfo));
                      objeamil.AddTargetEmail(email);
                      rs->MoveNext();
                    }
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
BOOL CIOCP::AppendByte(BYTE src[], int& len, pBREAKPCK pack, IOCP_IO_PTR & lp_io)
{
  if(pack != NULL && pack->len > 0)
    {
      int lenall = len + pack->len;

      if(lenall <= BUFFER_SIZE)
        {
          BYTE* allbyte = new BYTE[lenall];
          memset(allbyte, 0, lenall);
          memcpy(allbyte, pack->b, pack->len);
          memcpy(allbyte +  pack->len, src, len);
          delete pack->b;
          pack->b = allbyte;
          pack->len = lenall;
          len = lenall;
          memset(lp_io->wsaBuf.buf, 0, BUFFER_SIZE);
          memcpy(lp_io->wsaBuf.buf, allbyte, lenall);
          lp_io->wsaBuf.len = lenall;
        }
      else
        {
          delete pack->b;
          len = lenall;
        }
    }
  else
    {
      glog::GetInstance()->AddLine("�ϰ�ԭ���ݿ�");
    }

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
void CIOCP::ExitSocket(IOCP_IO_PTR & lp_io, IOCP_KEY_PTR & lp_key, int errcode)
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

  if(lp_io->fromtype == SOCKET_FROM_GAYWAY)
    {
      //�������ͻ�������
      string comaddr = lp_io->gayway;
      //�Ƴ�������
      map<string, IOCP_IO_PTR>::iterator  it = m_mcontralcenter.find(comaddr);

      if(it != m_mcontralcenter.end())
        {
          if(it->second == lp_io)
            {
              setOnline(comaddr, 0);
              m_mcontralcenter.erase(it);
            }
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
BOOL CIOCP::dealRead(IOCP_IO_PTR & lp_io, IOCP_KEY_PTR & lp_key, DWORD dwBytes)
{
  if(SOCKET_STATE_CONNECT_AND_READ != lp_io->state)
    {
      lp_io->state = SOCKET_STATE_CONNECT_AND_READ;
    }

  string towrite = "";
  int datalen = dwBytes;
  BYTE* src = (BYTE*)lp_io->buf;
  string data = gstring::char2hex(lp_io->buf, lp_io->ol.InternalHigh);
  lp_io->ol.InternalHigh > 0 ? glog::GetInstance()->AddLine("������:%d ������:%s", datalen, data.c_str()) : 0;
  towrite = data;
  CListTextElementUI* pElement = (CListTextElementUI*)lp_io->pUserData;
  map<IOCP_IO_PTR, pBREAKPCK>::iterator itepack;

  if(pElement)
    {
      pElement->SetText(8, lp_io->gayway);
      pElement->SetText(5, data.c_str());
      pElement->SetText(6, lp_io->buf);
      char lenstr[20] = {0};
      sprintf(lenstr, "%d", datalen);
      pElement->SetText(6, lp_io->buf);
      pElement->SetText(7, lenstr);
    }

  if(lp_io->fromtype == SOCKET_FROM_GAYWAY)
    {
      EnterCriticalSection(&crtc_sec);
      int alllenth = dwBytes;

      if(checkFlag(src, alllenth))
        {
          goto TO;
        }

      itepack =  m_pack.find(lp_io);

      if(itepack != m_pack.end())
        {
          pBREAKPCK pack = itepack->second;
          AppendByte(src, alllenth, pack, lp_io);
          PostLog("�ϰ���β:lp_io:%p ����:%d alllenght:%d", lp_io, dwBytes, alllenth);
          glog::GetInstance()->AddLine("�ϰ���β:lp_io:%p ����:%d", lp_io, datalen);

          if(alllenth > BUFFER_SIZE)
            {
              glog::GetInstance()->AddLine("����:[%s] ���յ������ݰ����� ����:%d ָ��:%p", lp_io->gayway, alllenth, lp_io);
              PostLog("����:[%s] ���յ������ݰ����� ����:%d ָ��:%p", lp_io->gayway, alllenth, lp_io);

              if(pack)
                {
                  delete pack;
                  pack = NULL;
                }

              m_pack.erase(itepack);
              goto RET;
              //return 1;
            }
        }

      if(IsBreakPack(lp_io, src, alllenth))
        {
          if(m_pack.find(lp_io) == m_pack.end())
            {
              pBREAKPCK pack = new BREAK_PACK;
              BYTE *b1 = new BYTE[datalen];
              memset(b1, 0, datalen);
              memcpy(b1, lp_io->buf, datalen);
              pack->b = b1;
              pack->len = datalen;
              m_pack.insert(make_pair(lp_io, pack));
              PostLog("�ϰ���ͷ:lp_io:%p ����:%d", lp_io, datalen);
              glog::GetInstance()->AddLine("�ϰ���ͷ:lp_io:%p ����:%d", lp_io, datalen);
            }
        }

      if(checkFlag(src, alllenth))
        {
TO:
          map<IOCP_IO_PTR, pBREAKPCK>::iterator itepack1 =  m_pack.find(lp_io);

          if(itepack1 != m_pack.end())
            {
              pBREAKPCK p1 = itepack->second;
              delete p1->b;
              delete p1;
              m_pack.erase(itepack);
            }

          char addrarea[20] = {0};
          sprintf(addrarea, "%02x%02x%02x%02x", src[8], src[7], src[10], src[9]); //���ص�ַ
          string datastr = gstring::char2hex((char*)src, alllenth);
          PostLog("����[%s] ������:%d ֡���:%d ������:%s ͨ��ָ��:%p", addrarea, alllenth, src[0xd] & 0x0f, datastr.c_str(), lp_io);
          buildcode(src, datalen, lp_io);
        }

RET:
      LeaveCriticalSection(&crtc_sec);
      return 1;
    }

  if(lp_io->fromtype == SOCKET_FROM_WEBSOCKET)
    {
      int complepack = wsPackCheck(src, dwBytes);
      int alllenth = dwBytes;
      int typepack = 0;
      map<IOCP_IO_PTR, pBREAKPCK>::iterator webite;

      if(complepack == WS_ALL_PACK)
        {
          goto COMPLETEPACK;
        }

      webite =  m_pack.find(lp_io);

      if(webite != m_pack.end())
        {
          pBREAKPCK pack = webite->second;
          AppendByte(src, alllenth, pack, lp_io);
          PostLog("web �ϰ���β:lp_io:%p ����:%d", lp_io, dwBytes);

          if(alllenth > BUFFER_SIZE)
            {
              PostLog("�����ȹ���:%d", alllenth);

              if(pack)
                {
                  delete pack;
                  pack = NULL;
                }

              m_pack.erase(webite);
              return 1;
            }
        }

      typepack = wsPackCheck(src, alllenth);

      if(typepack == WS_ALL_PACK)
        {
          map<IOCP_IO_PTR, pBREAKPCK>::iterator webite =  m_pack.find(lp_io);

          if(webite != m_pack.end())
            {
              m_pack.erase(webite);
            }

          goto  COMPLETEPACK;
        }
      else if(typepack == WS_BREAK_PACK)
        {
          map<IOCP_IO_PTR, pBREAKPCK>::iterator itepack =  m_pack.find(lp_io);

          if(itepack == m_pack.end())
            {
              //websocket�ϰ�����
              pBREAKPCK webpack = new BREAK_PACK;
              BYTE *b1 = new BYTE[datalen];
              memset(b1, 0, datalen);
              memcpy(b1, src, datalen);
              webpack->b = b1;
              webpack->len = datalen;
              m_pack.insert(make_pair(lp_io, webpack));
              PostLog("web �ϰ���ͷ:lp_io:%p ����:%d", lp_io, datalen);
            }
        }

COMPLETEPACK:
      string  strret = "";
      BOOL bBreadPack = FALSE;
      int lenread = wsDecodeFrame(lp_io->buf, strret, alllenth, bBreadPack);
      // PostLog("lenread:%d bBreadPack:%d", lenread, bBreadPack);

      if(lenread == WS_OPENING_FRAME && bBreadPack == FALSE)
        {
          map<IOCP_IO_PTR, pBREAKPCK>::iterator ite1 =  m_pack.find(lp_io);

          if(ite1 != m_pack.end())
            {
              m_pack.erase(ite1);
            }

          DealWebsockMsg(lp_io, lp_key, strret, alllenth);
        }
      else if(lenread == WS_CLOSING_FRAME)
        {
          PostLog("web���˳� ͨ��ָ��:%p", lp_io);
          lp_io->operation = IOCP_END;
        }

      return 1;
    }

  //if(lp_io->fromtype == SOCKET_FROM_GAYWAY)
  //{
  //    if(checkFlag((BYTE*)lp_io->buf, lp_io->ol.InternalHigh) == FALSE)
  //    {
  //        //�������ϰ�����
  //        map<IOCP_IO_PTR, pBREAKPCK>::iterator itepack =  m_pack.find(lp_io);
  //        if(itepack == m_pack.end())
  //        {
  //            if(IsBreakPack(lp_io, src, datalen))
  //            {
  //                pBREAKPCK pack = new BREAK_PACK;
  //                BYTE *b1 = new BYTE[datalen];
  //                memset(b1, 0, datalen);
  //                memcpy(b1, lp_io->buf, datalen);
  //                pack->b = b1;
  //                pack->len = datalen;
  //                m_pack.insert(make_pair(lp_io, pack));
  //                PostLog("�ϰ���ͷ:lp_io:%p ����:%d", lp_io, datalen);
  //                glog::GetInstance()->AddLine("�ϰ���ͷ:lp_io:%p ����:%d", lp_io, datalen);
  //            }
  //        }
  //        else
  //        {
  //            EnterCriticalSection(&crtc_sec);
  //            PostLog("�ϰ���β:lp_io:%p ����:%d", lp_io, datalen);
  //            glog::GetInstance()->AddLine("�ϰ���β:lp_io:%p ����:%d", lp_io, datalen);
  //            //EnterCriticalSection(&lp_this->crtc_sec);
  //            if(m_pack.find(lp_io) != m_pack.end())
  //            {
  //                AppendByte((BYTE*)lp_io->buf, lp_io->ol.InternalHigh, itepack->second, lp_io);
  //                m_pack.erase(itepack);
  //            }
  //            LeaveCriticalSection(&crtc_sec);
  //        }
  //    }
  //}
  //datalen = lp_io->ol.InternalHigh;
  //if(checkFlag(src, datalen) == TRUE)
  //{
  //    int datalen = lp_io->ol.InternalHigh;
  //    BYTE* src = (BYTE*)lp_io->buf;
  //    char addrarea[20] = {0};
  //    sprintf(addrarea, "%02x%02x%02x%02x", src[8], src[7], src[10], src[9]); //���ص�ַ
  //    string datastr = gstring::char2hex((char*)src, datalen);
  //    PostLog("����[%s] ������:%d ֡���:%d ������:%s ͨ��ָ��:%p", addrarea, datalen, src[0xd] & 0x0f, datastr.c_str(), lp_io);
  //    buildcode(src, datalen, lp_io);
  //    return 1;
  //}
  //if(lp_io->fromtype == SOCKET_FROM_UNKNOW)
  //{
  //    string strdata = lp_io->buf;
  //    string strret;
  //    int wsconn = wsHandshake(strdata, strret);
  //    if(wsconn == WS_STATUS_CONNECT)
  //    {
  //        InitIoContext(lp_io);
  //        lp_io->operation = IOCP_WRITE;
  //        lp_io->fromtype = SOCKET_FROM_WEBSOCKET;
  //        if(pElement)
  //        {
  //            pElement->SetText(8, "web�ͻ���");
  //        }
  //        strcpy(lp_io->gayway, "web�ͻ���(2)");
  //        lp_io->loginstatus = SOCKET_STATUS_LOGIN;
  //        memcpy(lp_io->buf, strret.c_str(), strret.size());
  //        PostLog("web������....");
  //        lp_io->wsaBuf.len = strret.size();
  //    }
  //}
  //else if(lp_io->fromtype == SOCKET_FROM_WEBSOCKET)
  //{
  //    string  strret = "";
  //    BOOL bFullPack = TRUE;
  //    int lenread = wsDecodeFrame(lp_io->buf, strret, lp_io->ol.InternalHigh, bFullPack);
  //    //PostLog("lenread:%d bFullPack:%d", lenread, bFullPack);
  //    if(lenread == WS_OPENING_FRAME && bFullPack == TRUE)
  //    {
  //        DealWebsockMsg(lp_io, lp_key, strret);
  //    }
  //    else if(lenread == WS_OPENING_FRAME && bFullPack == FALSE)
  //    {
  //        map<IOCP_IO_PTR, pBREAKPCK>::iterator itepack =  m_pack.find(lp_io);
  //        if(itepack == m_pack.end())
  //        {
  //            //websocket�ϰ�����
  //            pBREAKPCK webpack = new BREAK_PACK;
  //            BYTE *b1 = new BYTE[datalen];
  //            memset(b1, 0, datalen);
  //            memcpy(b1, src, datalen);
  //            webpack->b = b1;
  //            webpack->len = datalen;
  //            m_pack.insert(make_pair(lp_io, webpack));
  //            PostLog("web �ϰ���ͷ:lp_io:%p ����:%d", lp_io, datalen);
  //        }
  //    }
  //    else if(lenread == WS_CLOSING_FRAME)
  //    {
  //        PostLog("web���˳� ͨ��ָ��:%p", lp_io);
  //        lp_io->operation = IOCP_END;
  //    }
  //    else
  //    {
  //        map<IOCP_IO_PTR, pBREAKPCK>::iterator webite =  m_pack.find(lp_io);
  //        if(webite != m_pack.end())
  //        {
  //            pBREAKPCK pack = webite->second;
  //            // ����utf-8������ı�֡
  //            DWORD payloadLength = static_cast<uint16_t >(pack->b[1] & 0x7f);
  //            DWORD payloadFieldExtraBytes = 0;
  //            if(payloadLength == 0x7e)   //0111 1110     //126 7e  �������ֽ��ǳ��� :  127  7f �������ֽ��ǳ���
  //            {
  //                uint16_t payloadLength16b = 0;
  //                BYTE payloadFieldExtraBytes = 2;
  //                memcpy(&payloadLength16b, &pack->b[2], payloadFieldExtraBytes);
  //                payloadLength = ntohs(payloadLength16b);
  //            }
  //            int len = pack->len + datalen;
  //            if(payloadLength = (len - 2 - payloadFieldExtraBytes - 4))
  //            {
  //                AppendByte((BYTE*)lp_io->buf, lp_io->ol.InternalHigh, webite->second, lp_io);
  //                m_pack.erase(webite);
  //                PostLog("web �ϰ���β:lp_io:%p ����:%d", lp_io, datalen);
  //                string  strret = "";
  //                BOOL bFullPack = TRUE;
  //                int lenread1 = wsDecodeFrame(lp_io->buf, strret, lp_io->ol.InternalHigh, bFullPack);
  //                if(lenread1 == WS_OPENING_FRAME && bFullPack == TRUE)
  //                {
  //                    DealWebsockMsg(lp_io, lp_key, strret);
  //                }
  //                else if(lenread1 == WS_CLOSING_FRAME)
  //                {
  //                    PostLog("�ϰ� web���˳� ͨ��ָ��:%p", lp_io);
  //                    lp_io->operation = IOCP_END;
  //                }
  //            }
  //            else
  //            {
  //                glog::GetInstance()->AddLine("�жϰ�ͷû�жϰ�β");
  //            }
  //        }
  //    }
  //}
  return 1;
}


int CIOCP::wsPackCheck(BYTE src[], int len)
{
  int ret = FALSE;
  const char *msg = (const char*)src;
  const int frameLength = len;

  if(frameLength < 2)
    {
      return FALSE;
    }

  BYTE Fin =  msg[0] >> 7 & 1;
  BYTE RSV1 = msg[0] >> 6 & 0x01;
  BYTE RSV2 = msg[0] >> 5 & 0x01;
  BYTE RSV3 = msg[0] >> 4 & 0x01;
  BYTE Mask =  msg[1] >> 7 & 0x01;

  //FIN:1λ������������Ϣ�Ƿ���������Ϊ1�����ϢΪ��Ϣβ��,���Ϊ�����к������ݰ�;
  if(Fin != 1)
    {
      return ret;
    }

  // �����չλ������
  if(RSV1 == 1 || RSV2 == 1 || RSV3 == 1)
    {
      return ret;
    }

  // maskλ, Ϊ1��ʾ���ݱ�����
  if(Mask != 1)
    {
      return ret;
    }

  BYTE opcode = msg[0] & 0x0f;
  // ������
  uint16_t payloadLength = 0;
  uint8_t payloadFieldExtraBytes = 0;

  if(opcode == WS_TEXT_FRAME)
    {
      // ����utf-8������ı�֡
      payloadLength = static_cast<uint16_t >(msg[1] & 0x7f);

      if(payloadLength == 0x7e)   //0111 1110     //126 7e  �������ֽ��ǳ��� :  127  7f �������ֽ��ǳ���
        {
          uint16_t payloadLength16b = 0;
          payloadFieldExtraBytes = 2;
          memcpy(&payloadLength16b, &msg[2], payloadFieldExtraBytes);
          payloadLength = ntohs(payloadLength16b);
        }
      else if(payloadLength == 0x7f)
        {
          // ���ݹ���,�ݲ�֧��
          uint32_t payloadLength32b = 0;
          payloadFieldExtraBytes = 4;
          memcpy(&payloadLength32b, &msg[2], payloadFieldExtraBytes);
          payloadLength = ntohl(payloadLength32b);
          //ret = WS_ERROR_FRAME;
        }

      if(payloadLength == (len - 2 - payloadFieldExtraBytes - 4))
        {
          return WS_ALL_PACK;
        }

      if(payloadLength > len - 2 - payloadFieldExtraBytes - 4)
        {
          return WS_BREAK_PACK;
        }
    }

  return WS_ERROR_PACK;
}

int CIOCP::GetTickCZ(BYTE mon, BYTE day, BYTE hour, BYTE min)
{
  time_t tmtamp;
  struct tm *tm1 = NULL;
  time(&tmtamp) ;
  tm1 = localtime(&tmtamp) ;
  time_t t1 = mktime(tm1);
//     BYTE mon = 10;
//     BYTE day = 30;
//     BYTE hour = 14;
//     BYTE minute = 42;
  tm1->tm_year = tm1->tm_year;
  tm1->tm_mon =  mon - 1;
  tm1->tm_mday = day;
  tm1->tm_hour = hour;
  tm1->tm_min = min;
  tm1->tm_sec = 0;
  time_t t2 = mktime(tm1);
  time_t t3 = t1 - t2;
  BYTE m = t3 / 60;
  return m;
}
