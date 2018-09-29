#pragma once

#include <windows.h>
#include <UIlib.h>
#include "DBOperation.h"
using namespace DuiLib;
#pragma comment(lib,"wininet.lib")
#pragma comment(lib,"DuiLib.lib")



//BOOL CheckAuthenticate(BYTE uBlock);//ÑéÖ¤¿é



#pragma pack(1)




class CMainFrame  : public CWindowWnd, public INotifyUI
{
public:



    HWND                      m_hParanWnd;
    CRichEditUI* m_pRishLog;
    CPaintManagerUI m_pm;

public:

    CMainFrame(void);
    CMainFrame(string skin);
    ~CMainFrame(void);

    string m_strskin;
    CDBOperation*   m_pdbOpen;
	 CListUI*   m_plistuser;
	 CEditUI* m_pData;
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
    // void    Dealdataskin(string btn, TNotifyUI& msg);

    string  GetDataDir(string name);
    int IsNum(char s[]);
    void PostLog(const char* pData, ...);
	string getItemText(CListUI* pControl,int irow,int icolum);
	void setItemText(CListUI* pControl,int irow,int icolum,string data);
};
