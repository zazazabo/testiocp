#pragma once
#include "gSmtp.h"
#include "smtp.h"
#include <windows.h>
#include "resource.h"
#include <UIlib.h>
#include "DBOperation.h"
using namespace DuiLib;
#pragma comment(lib,"wininet.lib")
#pragma comment(lib,"DuiLib.lib")



#pragma pack(1)


#define WM_TRAYICON (WM_USER + 100)

class CMainFrame  : public CWindowWnd, public INotifyUI
{
public:


  DWORD  ThreadId;
  HWND                      m_hParanWnd;
  CRichEditUI* m_pRishLog;
  CLabelUI*     m_lbgaywayNum;
  CLabelUI*     m_lbWebNum;
  CPaintManagerUI m_pm;
  SHORT     port;
  char      ip[20];
  char      m_pid[216];
  HMENU   hMenu;
  NOTIFYICONDATA nid;    //NOTIFYICONDATA结构包含了系统用来处理托盘图标的信息，
public:
  void ShowTrayIcon();
  CMainFrame(void);
  CMainFrame(string skin);
  CSmtp objeamil;
  gSmtp m_objemail;
  ~CMainFrame(void);
  vector<string>v_fault;
  string m_strskin;
  CDBOperation*   dbopen;
  CListUI*   m_plistuser;
  CEditUI* m_pData;
  CLabelUI* m_pDate;
  CLabelUI* m_pUserName;
// char m_configTime[216];             //采集时间
  char m_sql[4096];
  char                    chlog[2048];
  LPCTSTR GetWindowClassName() const
  {
    return _T("UISERVERWND");
  };
  UINT GetClassStyle() const
  {
    return UI_CLASSSTYLE_FRAME | CS_DBLCLKS;
  };
  void OnFinalMessage(HWND /*hWnd*/)
  {
    delete this;
  };

  void    Init();
  void    OnPrepare(TNotifyUI& msg);
  void    Notify(TNotifyUI& msg);
  LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
  LRESULT OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
  LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
  LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandle);
  LRESULT OnNcActivate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandle);
  LRESULT OnNcCalcSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandle);
  LRESULT OnNcPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandle);
  LRESULT OnNcHitTest(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
  LRESULT OnSysCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
  LRESULT OnGetMinMaxInfo(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
  LRESULT OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
  LRESULT HandleCustomMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
  LRESULT OnUser(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);


  string  GetDataDir(string name);
  int IsNum(char s[]);
  void PostLog(const char* pData, ...);

  string getItemText(CListUI* pControl, int irow, int icolum);
  void setItemText(CListUI* pControl, int irow, int icolum, string data);
  void clearEndChar(string& str, string c);
  void dealSqlRecords(string addrarea, string myday, string inmsg, string colum);
  void dealSqlPower(string addrarea, string myday, string power00, string power45);
  void setOnline(string comaddr, int status);
  string getErrorInfo(int ercode);
  void  DeleteByIo(ULONG_PTR io);
};
