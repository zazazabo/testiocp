#include "MainFrame.h"

#include <map>
#include "DBOperation.h"
#include "WINSCARD.H"
#include <comutil.h>
#include <stdio.h>
#include <atlbase.h>
#include <MsHTML.h>
#include <winternl.h>
#include <locale.h>;
#include <time.h>
#pragma comment(lib,"WinInet.Lib")
#pragma comment(lib,"comsuppw.lib")
#pragma comment(lib,"ws2_32.lib")
#include <WinInet.h>
#include <iostream>

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

SCARDCONTEXT            hContext;
SCARDHANDLE             hCard;
unsigned long           dwActProtocol;
LPCBYTE                 pbSend;
DWORD                   dwSend, dwRecv, size = 64;
LPBYTE                  pbRecv;
SCARD_IO_REQUEST        ioRequest;
int                     retCode;
char                    readerName [256];
DWORD                   SendLen, RecvLen, ByteRet;;
BYTE                    SendBuff[262], RecvBuff[262];
SCARD_IO_REQUEST        IO_REQ;
unsigned char           HByteArray[16];



CMainFrame::CMainFrame(void)
{
}

CMainFrame::CMainFrame(string skin)
{
  HWND  hh = FindWindowA("UIPMainFrame", "");

  if(hh)
    {
      m_hParanWnd = hh;
    }

  m_strskin = skin;
}


CMainFrame::~CMainFrame(void)
{
}





void CMainFrame::Init()
{
  m_pRishLog = static_cast<CRichEditUI*>(m_pm.FindControl(_T("opera")));
  m_plistuser = static_cast<CListUI*>(m_pm.FindControl(_T("socketlist")));
  m_pData = static_cast<CEditUI*>(m_pm.FindControl(_T("data")));
  m_plistuser->RemoveAll();
  // CListTextElementUI* pListElement = new CListTextElementUI;
  // m_plistuser->Add(pListElement);
  // char vvv[20] = {0};
  // sprintf(vvv, "%d", 1);
  // pListElement->SetText(0, vvv);
  //pListElement->SetText(1,"aaa");
  this->m_hParanWnd = this->m_hWnd;
  this->CenterWindow();
}


void CMainFrame::Notify(TNotifyUI& msg)
{
  if(msg.sType == _T("windowinit"))
    OnPrepare(msg);
  else if(msg.sType == _T("click"))
    {
      if(msg.pSender->GetName() == "closebtn")
        {
          //SendMessageA(WM_SYSCOMMAND, SC_CLOSE, 0);
          //SendMessageA(WM_SYSCOMMAND, SC_CLOSE, 0);
          Close();
        }
      else if(msg.pSender->GetName() == "exitsys")
        {
          Close();
          //SendMessageA(WM_SYSCOMMAND, SC_CLOSE, 0);
          //this->Close();
        }
      else if(msg.pSender->GetName() == "istrue")
        {
          Close(1);
        }
      else if(msg.pSender->GetName() == "isfalse")
        {
          Close(0);
        }
      else if(msg.pSender->GetName() == "start")
        {
          //PostLog("dddd");
        }
    }
}

LRESULT CMainFrame::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  BOOL bHandled = TRUE;
  LRESULT lRes = 0;

  switch(uMsg)
    {
      case WM_CREATE:
        lRes = OnCreate(uMsg, wParam, lParam, bHandled);
        break;

      case WM_CLOSE:
        lRes = OnClose(uMsg, wParam, lParam, bHandled);
        break;

      case WM_NCACTIVATE:
        lRes = OnNcActivate(uMsg, wParam, lParam, bHandled);
        break;

      case WM_NCCALCSIZE:
        lRes = OnNcCalcSize(uMsg, wParam, lParam, bHandled);
        break;

      case WM_NCPAINT:
        lRes = OnNcPaint(uMsg, wParam, lParam, bHandled);
        break;

      case WM_NCHITTEST:
        lRes = OnNcHitTest(uMsg, wParam, lParam, bHandled);
        break;

      default:
        bHandled = FALSE;
    }

  if(bHandled) return lRes;

  lRes = HandleCustomMessage(uMsg, wParam, lParam, bHandled);

  if(bHandled) return lRes;

  if(m_pm.MessageHandler(uMsg, wParam, lParam, lRes)) return lRes;

  return CWindowWnd::HandleMessage(uMsg, wParam, lParam);
}

LRESULT CMainFrame::OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
  bHandled = FALSE;
  return 0;
}


LRESULT CMainFrame::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
  ::PostQuitMessage(0L);
  bHandled = FALSE;
  return 0;
}

LRESULT CMainFrame::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandle)
{
  m_pm.Init(m_hWnd);
  CDialogBuilder builder;
  CControlUI* pRoot = builder.Create(m_strskin.c_str(), (UINT)0, NULL, &m_pm);
  ASSERT(pRoot && "Failed to parse XML");
  m_pm.AttachDialog(pRoot);
  m_pm.AddNotifier(this);
  Init();
  return 0;
}

LRESULT CMainFrame::OnNcActivate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandle)
{
  if(::IsIconic(*this)) bHandle = FALSE;

  return (wParam == 0) ? TRUE : FALSE;
}

LRESULT CMainFrame::OnNcCalcSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandle)
{
  return 0;
}

LRESULT CMainFrame::OnNcPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandle)
{
  return 0;
}

LRESULT CMainFrame::OnNcHitTest(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
  POINT pt;
  pt.x = GET_X_LPARAM(lParam);
  pt.y = GET_Y_LPARAM(lParam);
  ::ScreenToClient(*this, &pt);
  RECT rcClient;
  ::GetClientRect(*this, &rcClient);

  if(!::IsZoomed(*this))
    {
      RECT rcSizeBox = m_pm.GetSizeBox();

      if(pt.y < rcClient.top + rcSizeBox.top)
        {
          if(pt.x < rcClient.left + rcSizeBox.left) return HTTOPLEFT;

          if(pt.x > rcClient.right - rcSizeBox.right) return HTTOPRIGHT;

          return HTTOP;
        }
      else if(pt.y > rcClient.bottom - rcSizeBox.bottom)
        {
          if(pt.x < rcClient.left + rcSizeBox.left) return HTBOTTOMLEFT;

          if(pt.x > rcClient.right - rcSizeBox.right) return HTBOTTOMRIGHT;

          return HTBOTTOM;
        }

      if(pt.x < rcClient.left + rcSizeBox.left) return HTLEFT;

      if(pt.x > rcClient.right - rcSizeBox.right) return HTRIGHT;
    }

  RECT rcCaption = m_pm.GetCaptionRect();

  if(pt.x >= rcClient.left + rcCaption.left && pt.x < rcClient.right - rcCaption.right \
      && pt.y >= rcCaption.top && pt.y < rcCaption.bottom)
    {
      CControlUI* pControl = static_cast<CControlUI*>(m_pm.FindControl(pt));

      if(pControl && _tcscmp(pControl->GetClass(), _T("ButtonUI")) != 0 &&
          _tcscmp(pControl->GetClass(), _T("OptionUI")) != 0 &&
          _tcscmp(pControl->GetClass(), _T("TextUI")) != 0)
        return HTCAPTION;
    }

  return HTCLIENT;
}

LRESULT CMainFrame::OnSysCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
  // 有时会在收到WM_NCDESTROY后收到wParam为SC_CLOSE的WM_SYSCOMMAND
  if(wParam == SC_CLOSE)
    {
      ::PostQuitMessage(0L);
      bHandled = TRUE;
      return 0;
    }

  BOOL bZoomed = ::IsZoomed(*this);
  LRESULT lRes = CWindowWnd::HandleMessage(uMsg, wParam, lParam);

  if(::IsZoomed(*this) != bZoomed)
    {
      if(!bZoomed)
        {
          CControlUI* pControl = static_cast<CControlUI*>(m_pm.FindControl(_T("maxbtn")));

          if(pControl) pControl->SetVisible(false);

          pControl = static_cast<CControlUI*>(m_pm.FindControl(_T("restorebtn")));

          if(pControl) pControl->SetVisible(true);
        }
      else
        {
          CControlUI* pControl = static_cast<CControlUI*>(m_pm.FindControl(_T("maxbtn")));

          if(pControl) pControl->SetVisible(true);

          pControl = static_cast<CControlUI*>(m_pm.FindControl(_T("restorebtn")));

          if(pControl) pControl->SetVisible(false);
        }
    }

  return lRes;
}

LRESULT CMainFrame::OnGetMinMaxInfo(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
  MONITORINFO oMonitor = {};
  oMonitor.cbSize = sizeof(oMonitor);
  ::GetMonitorInfo(::MonitorFromWindow(*this, MONITOR_DEFAULTTOPRIMARY), &oMonitor);
  CDuiRect rcWork = oMonitor.rcWork;
  rcWork.Offset(-oMonitor.rcMonitor.left, -oMonitor.rcMonitor.top);
  LPMINMAXINFO lpMMI = (LPMINMAXINFO) lParam;
  lpMMI->ptMaxPosition.x  = rcWork.left;
  lpMMI->ptMaxPosition.y  = rcWork.top;
  lpMMI->ptMaxSize.x      = rcWork.right;
  lpMMI->ptMaxSize.y      = rcWork.bottom;
  bHandled = FALSE;
  return 0;
}

void CMainFrame::OnPrepare(TNotifyUI& msg)
{
}
int iiii = 0;
LRESULT CMainFrame::OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
  return 1;
}





std::string CMainFrame::GetDataDir(string name)

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
int CMainFrame::IsNum(char s[])
{
  int i;

  for(i = 0; i < strlen(s); i++)
    {
      if(s[i] < '0' || s[i] > '9')
        {
          return 0;
        }
    }

  return 1;
}
void CMainFrame::PostLog(const char* pData, ...)
{
  //char pp[216] = {0};
  int     n = 0;
  va_list args = NULL;
  memset(chlog, 0, 2048);
  va_start(args, pData);
  n = vsprintf(chlog, pData, args);
  va_end(args);
  ::SendMessageA(this->m_hParanWnd, WM_USER + 1, (WPARAM)chlog, (LPARAM)0);
}



LRESULT CMainFrame::HandleCustomMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
  LONG lRes = 0;

  if(uMsg == WM_USER + 1)
    {
      OnUser(uMsg, wParam, lParam, bHandled);
      bHandled = false;
      return 0;
    }

  if(uMsg == WM_TIMER)
    {
      lRes = OnTimer(uMsg, wParam, lParam, bHandled);
    }

  if(lRes)
    {
      return lRes;
    }

  bHandled = FALSE;
  return 0;
}




LRESULT CMainFrame::OnUser(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
  //gstring::tip("%s",wParam);
  time_t rawtime;
  time(&rawtime);
  struct tm* tmif;
  char timeinfo[80];
  tmif = localtime(&rawtime);
  sprintf(timeinfo, "[%4d-%02d-%02d %02d:%02d:%02d]:", \
          tmif->tm_year + 1900, tmif->tm_mon + 1, tmif->tm_mday, tmif->tm_hour, tmif->tm_min, tmif->tm_sec);

  if(_stricmp(m_pRishLog->GetText(), "") == 0)
    {
      string strlog = timeinfo;
      strlog.append((char*)wParam);
      m_pRishLog->AppendText(strlog.c_str());
      m_pRishLog->EndDown();
    }
  else
    {
      m_pRishLog->AppendText("\n");
      string strlog = timeinfo;
      strlog.append((char*)wParam);
      m_pRishLog->AppendText(strlog.c_str());
      m_pRishLog->EndDown();
    }

  m_pRishLog->EndDown();
  return 1;
}

std::string CMainFrame::getItemText(CListUI* pControl, int irow, int icolum)
{
  if(irow == -1)
    {
      return "";
    }

  CListTextElementUI* pText = (CListTextElementUI*)pControl->GetItemAt(irow);
  string ret = pText->GetText(icolum);
  return ret;
}

void CMainFrame::setItemText(CListUI* pControl, int irow, int icolum, string data)
{
  if(irow == -1)
    {
      return;
    }

  CListTextElementUI* pText = (CListTextElementUI*)pControl->GetItemAt(irow);
  pText->SetText(icolum, data.c_str());
}
