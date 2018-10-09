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
#pragma comment(lib,"E:\\code\\zlib\\x64\\Release\\zlib.lib")

#else
#pragma comment(lib,"lib_json.lib")
#pragma comment(lib,"E:\\code\\glib\\Release\\glib.lib")
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
  m_pDate =  static_cast<CLabelUI*>(m_pm.FindControl(_T("nowdate")));
  m_pUserName = static_cast<CLabelUI*>(m_pm.FindControl(_T("loguser")));
  m_pDate->SetText(gstring::getday().c_str());
  m_pUserName->SetText("....");
  m_pData->SetText("683200320068040117660002ac7100002004c516");
  m_plistuser->RemoveAll();
  string pdir = GetDataDir("config.ini");
  GetPrivateProfileStringA("Config", "time", "", m_configTime, 216, pdir.c_str());
  char source[216] = {0};
  GetPrivateProfileStringA("Config", "source", "", source, 216, pdir.c_str());
  char database[216] = {0};
  GetPrivateProfileStringA("Config", "database", "", database, 216, pdir.c_str());
  char uname[216] = {0};
  GetPrivateProfileStringA("Config", "uname", "", uname, 216, pdir.c_str());
  char upass[216] = {0};
  GetPrivateProfileStringA("Config", "upass", "", upass, 216, pdir.c_str());
  dbopen = new CDBOperation();
  BOOL bcon = dbopen->ConnToDB(source, database, uname, upass);
  setOnline("1=1", 0);
  //dealSql("17020101", "2018-09-28", "bbb", "activepower");
  this->m_hParanWnd = this->m_hWnd;
  this->CenterWindow();
  CoInitialize(NULL);
  char temp[216] = {0};
  GetPrivateProfileStringA("Config", "smtp", "", temp, 216, pdir.c_str());
  string strdomain = temp;
  objeamil.SetSrvDomain(strdomain);
  GetPrivateProfileStringA("Config", "emailuser", "", temp, 216, pdir.c_str());
  string stremail = temp;
  objeamil.SetUserName(stremail);
  GetPrivateProfileStringA("Config", "emailpass", "", temp, 216, pdir.c_str());
  string strpass = temp;
  objeamil.SetPass(strpass);

  GetPrivateProfileStringA("Config", "ip", "", ip, 216, pdir.c_str());

   GetPrivateProfileStringA("Config", "port", "", temp, 216, pdir.c_str());
   port=atoi(temp);

  //CSmtp smtp(25, "smtp.126.com","z277402131@126.com", /*你的邮箱地址*/"z277402131",/*邮箱密码*/"277402131@qq.com",/*目的邮箱地址*/"TEST",/*主题*/"测试测试！收到请回复！"  /*邮件正文*/);
  //添加附件时注意,\一定要写成\\，因为转义字符的缘故
  //     string filePath("D:\\附件.txt");
  //     smtp.AddAttachment(filePath);
  /*还可以调用CSmtp::DeleteAttachment函数删除附件，还有一些函数，自己看头文件吧!*/
  //单个发送
  //int err;
  //if((err = smtp.SendEmail_Ex()) != 0) {
  //  if(err == 1)
  //      cout << "错误1: 由于网络不畅通，发送失败!" << endl;
  //  if(err == 2)
  //      cout << "错误2: 用户名错误,请核对!" << endl;
  //  if(err == 3)
  //      cout << "错误3: 用户密码错误，请核对!" << endl;
  //  if(err == 4)
  //      cout << "错误4: 请检查附件目录是否正确，以及文件是否存在!" << endl;
  //}
  //群发
  //     string strTarEmail = "12345678@qq.com";
  //     smtp.AddTargetEmail(strTarEmail);
  //
  //     if((err = smtp.SendVecotrEmail()) != 0) {
  //         if(err == -1)
  //             cout << "错误-1: 没有目地邮箱地址!" << endl;
  //
  //         if(err == 1)
  //             cout << "错误1: 由于网络不畅通，发送失败!" << endl;
  //
  //         if(err == 2)
  //             cout << "错误2: 用户名错误,请核对!" << endl;
  //
  //         if(err == 3)
  //             cout << "错误3: 用户密码错误，请核对!" << endl;
  //
  //         if(err == 4)
  //             cout << "错误4: 请检查附件目录是否正确，以及文件是否存在!" << endl;
  //     }
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
  sprintf(timeinfo, "[%02d:%02d:%02d]:", \
          tmif->tm_hour, tmif->tm_min, tmif->tm_sec);

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

  if(m_pRishLog->GetLineCount() > 500)
    {
      m_pRishLog->SetText("");
      m_pRishLog->Clear();
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

void CMainFrame::clearEndChar(string& str, string c)
{
  int n1 = str.find_last_of(c);

  if(n1 == str.size() - c.length())
    {
      str = str.substr(0, n1);
    }
}

void CMainFrame::dealSqlRecords(string addrarea, string myday, string inmsg, string colum)
{
  string sql = "select * from t_records where 1=1 and CONVERT(Nvarchar, day, 23)=\'";
  sql.append(myday);
  sql.append("\' and comaddr='");
  sql.append(addrarea);
  sql.append("'");
  _RecordsetPtr rs = this->dbopen->ExecuteWithResSQL(sql.c_str());
  map<string, _variant_t>m_var;
  m_var.clear();
  _variant_t  vday(myday.c_str());
  _variant_t  vcomaddr(addrarea.c_str());
  _variant_t  vcolum(inmsg.c_str());

  if(rs && this->dbopen->GetNum(rs) == 0)
    {
      m_var.insert(pair<string, _variant_t>("day", vday));
      m_var.insert(pair<string, _variant_t>("comaddr", vcomaddr));
      m_var.insert(pair<string, _variant_t>(colum.c_str(), vcolum));
      string sql = dbopen->GetInsertSql(m_var, "t_records");

      if(sql.size() > 4096)
        {
          PostLog("sql 字符过大 字段:%s", sql.c_str());
          return;
        }

      memset(m_sql, 0, 4096);
      strcpy(m_sql, sql.c_str());
      _RecordsetPtr rs = dbopen->ExecuteWithResSQL(m_sql);

      if(rs == NULL)
        {
          glog::GetInstance()->AddLine("出错 sql:%s", sql.c_str());
        }
    }
  else if(rs && this->dbopen->GetNum(rs) == 1)
    {
      _variant_t  vcolum(inmsg.c_str());
      m_var.insert(pair<string, _variant_t>(colum.c_str(), inmsg.c_str()));
      string where = "";
      where.append(" where day=\'");
      where.append(myday);   //以后会变的
      where.append("\' and comaddr='");
      where.append(addrarea);
      where.append("\'");
      string sql = dbopen->GetUpdateSql(m_var, "t_records", where);

      if(sql.size() > 4096)
        {
          PostLog("sql 字符过大 字段:%s", sql.c_str());
          return;
        }

      memset(m_sql, 0, 4096);
      strcpy(m_sql, sql.c_str());
      _RecordsetPtr rs = dbopen->ExecuteWithResSQL(m_sql);

      if(rs == NULL)
        {
          glog::GetInstance()->AddLine("出错 sql:%s", sql.c_str());
        }
    }
}

void CMainFrame::dealSqlPower(string addrarea, string myday, string power00, string power45)
{
  string sql = "select * from t_power where 1=1 and CONVERT(Nvarchar, day, 23)=\'";
  sql.append(myday);
  sql.append("\' and comaddr='");
  sql.append(addrarea);
  sql.append("'");
  _RecordsetPtr rs = this->dbopen->ExecuteWithResSQL(sql.c_str());
  map<string, _variant_t>m_var;
  m_var.clear();
  _variant_t  vday(myday.c_str());
  _variant_t  vcomaddr(addrarea.c_str());
  _variant_t  vpower00(power00.c_str());
  _variant_t  vpower45(power45.c_str());

  if(rs && this->dbopen->GetNum(rs) == 0)
    {
      m_var.insert(pair<string, _variant_t>("day", vday));
      m_var.insert(pair<string, _variant_t>("comaddr", vcomaddr));
      m_var.insert(pair<string, _variant_t>("power00", vpower00));
      m_var.insert(pair<string, _variant_t>("power45", vpower45));
      string sql = dbopen->GetInsertSql(m_var, "t_power");

      if(sql.size() > 4096)
        {
          PostLog("sql 字符过大 字段:%s", sql.c_str());
          return;
        }

      memset(m_sql, 0, 4096);
      strcpy(m_sql, sql.c_str());
      _RecordsetPtr rs = dbopen->ExecuteWithResSQL(m_sql);

      if(rs == NULL)
        {
          glog::GetInstance()->AddLine("出错 sql:%s", sql.c_str());
        }
    }
  else if(rs && this->dbopen->GetNum(rs) == 1)
    {
      m_var.insert(pair<string, _variant_t>("power00", vpower00));
      m_var.insert(pair<string, _variant_t>("power45", vpower45));
      string where = "";
      where.append(" where day=\'");
      where.append(myday);   //以后会变的
      where.append("\' and comaddr='");
      where.append(addrarea);
      where.append("\'");
      string sql = dbopen->GetUpdateSql(m_var, "t_power", where);

      if(sql.size() > 4096)
        {
          PostLog("sql 字符过大 字段:%s", sql.c_str());
          return;
        }

      memset(m_sql, 0, 4096);
      strcpy(m_sql, sql.c_str());
      _RecordsetPtr rs = dbopen->ExecuteWithResSQL(m_sql);

      if(rs == NULL)
        {
          glog::GetInstance()->AddLine("出错 sql:%s", sql.c_str());
        }
    }
}

void CMainFrame::setOnline(string comaddr, int status)
{
  map<string, _variant_t>m_var;
  _variant_t vonline(status);
  m_var.insert(pair<string, _variant_t>("online", vonline));
  string strwhrere = "where 1=1 and";

  if(comaddr == "1=1")
    {
      strwhrere.append(" 1=1");
    }
  else
    {
      strwhrere.append(" comaddr=\'");
      strwhrere.append(comaddr);
      strwhrere.append("\'");
    }

  string sql = dbopen->GetUpdateSql(m_var, "t_baseinfo", strwhrere);
  _RecordsetPtr rs =   dbopen->ExecuteWithResSQL(sql.c_str());

  if(rs == NULL)
    {
      glog::GetInstance()->AddLine("更新在线状态失败");
    }
}

std::string CMainFrame::getErrorInfo(int ercode)
{
  char Temp[200] = {0}; // = new char[200];
  LPVOID lpMsgBuf;
  FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL,
                ercode,
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
                (LPTSTR) &lpMsgBuf,
                0,
                NULL
               );
  //sprintf(Temp, "WARNING:%s Failed with the following error: /n%s/nPort: %d/n", (char*)ErrorText, lpMsgBuf, m_nPortNr);
  sprintf(Temp, "%s", lpMsgBuf);
  string ss = Temp;
  LocalFree(lpMsgBuf);
  gstring::replace(ss, "\r\n", "");
  return ss;
}

void CMainFrame::DeleteByIo(ULONG_PTR io)
{
  CControlUI* p1 = (CControlUI*)io;
  //if(p1 != NULL)
  //  {
  //    m_plistuser->Remove(p1);
  //  }
  int ncount = m_plistuser->GetCount();

  for(int i = 0; i < ncount; i++)
    {
      CControlUI* p2 =  m_plistuser->GetItemAt(i);

      if(p1 == p2)
        {
          m_plistuser->RemoveAt(i);
          break;
        }

      //string iostr = getItemText(m_plistuser, i, 2);
      //char pio[20] = {0};
      //sprintf(pio, "%08x", io);
      //if(_stricmp(pio, iostr.c_str()) == 0)
      //{
      // m_plistuser->RemoveAt(i);
      // break;
      //}
    }
}
