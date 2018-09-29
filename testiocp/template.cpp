// dnf.cpp : Defines the entry point for the DLL application.
//
#include "IOCP.h"
#include "template.h"  //改成的头文件名
#include <commctrl.h>
#include <VECTOR>
#include <Windows.h>
#include <IO.H>
#include <TlHelp32.h>

#include <assert.h>
#include "E:\\code\\glib\\h\\gstring.h"
#include "E:\\code\\glib\\h\\MemoryModule.h"
#include "E:\\code\\glib\\h\\glog.h"
#include "E:\\code\\glib\\h\\gprocess.h"
#ifdef _DEBUG
#ifdef _WIN64

#else
#pragma comment(lib,"E:\\code\\glib\\lib\\glib.lib")
#endif
#else
#ifdef _WIN64


#else

#endif
#endif


#pragma comment(lib, "ws2_32.lib")
// #include <zlib.h>
// #include <zconf.h>


using namespace std;

#pragma comment(lib,"comctl32.lib")

HWND  hWindow;
HINSTANCE hInst;
//gListCtr list1;
//CIOCP io;
//void dealiocp()
//{
//    if(io.InitAll() == FALSE)
//    {
//        return;
//    }
//
//    io.m_listctr = &list1;
//    io.hWnd = hWindow;
//    io.MainLoop();
//}



void dealCommand(int wid)
{
    if(wid == IDC_BUTTON1)
    {
        //DWORD tid = 0;
        //HANDLE h1 = CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)dealiocp, NULL, NULL, &tid);
    }
    else if(wid == IDC_BUTTON8)
    {
        //int n1 = list1.getSelectIndex();

        //if(n1 >= 0)
        //{
        //    string socket = list1.getCellText(n1, 1);
        //    ULONG_PTR s =  strtol(socket.c_str(), NULL, 16);
        //    string socketkey = list1.getCellText(n1, 3);
        //    ULONG_PTR key = strtol(socketkey.c_str(), NULL, 16);
        //    io.CloseMySocket((IOCP_IO_PTR)s);
        //}
    }
    else if(wid == IDC_BUTTON2)
    {
        //int n1 = list1.getSelectIndex();

        //if(n1 >= 0)
        //{
        //    char data[1024] = {0};
        //    GetDlgItemText(hWindow, IDC_EDIT1, data, 1024);
        //    list1.setItemText(data, n1, 2);
        //}
    }
    else if(wid == IDC_BUTTON3)
    {
        //int n1 = list1.getSelectIndex();

        //if(n1 >= 0)
        //{
        //    string socket = list1.getCellText(n1, 1);
        //    ULONG_PTR s =  strtol(socket.c_str(), NULL, 16);
        //    string socketkey = list1.getCellText(n1, 3);
        //    ULONG_PTR key = strtol(socketkey.c_str(), NULL, 16);
        //    io.SendData(s, key);
        //}
    }
    else if(wid == IDC_BUTTON7)
    {
        //int n1 = list1.getSelectIndex();

        //if(n1 >= 0)
        //{
        //    string socket = list1.getCellText(n1, 1);
        //    ULONG_PTR s =  strtol(socket.c_str(), NULL, 16);
        //    string socketkey = list1.getCellText(n1, 3);
        //    ULONG_PTR key = strtol(socketkey.c_str(), NULL, 16);
        //    io.SendWebsocket(s);
        //}
    }
    else if(wid == IDC_BUTTON4)
    {
        //int nnn = list1.getSelectIndex();

        //if(nnn >= 0)
        //{
        //    string val = list1.getCellText(nnn, 4);
        //    gstring::copyToclip(val.c_str(), val.size());
        //    list1.setItemText("", nnn, 4);
        //    ;
        //}  
    }
    else if(wid == IDC_BUTTON5)
    {
        int nnn =0;// list1.getSelectIndex();

        if(nnn >= -1)
        {
//             string val = "684a004a0068c4021701010402f2000004000115141805052716"; //list1.getCellText(nnn, 4);
//             BYTE vv[1024] = {0};
//             int len = 0;
//             changeByte((char*)val.c_str(), vv, len);
//             if(checkFlag(vv, len)) {
//              gstring::tip("aaa");
//             }
//
//             int i = 0;
            /* while(*p != '\0') {
                 char p1[3] = {0};
                 memcpy(p1, p, 2);
                 BYTE b1 = strtol(p1, NULL, 16);
                 vv[i] = b1;
                 p += 2;
                 i++;
             }*/
            //    if(vv[0] == 0x68 && vv[5] == 0x68 && vv[i - 1] == 0x16) {
            //        int nbyte = i - 2 - 6;
            //        short n11 = (nbyte << 2) | 2;
            //        short to1 =  *(short*)&vv[1];
            //        BYTE  bend = 0;
            //        for(int j = 6; j < i - 2; j++) {
            //            bend += vv[j];
            //        }
            //        if(bend == vv[i - 2] && n11 == to1) {
            //            BYTE b2 = vv[13];
            //            BYTE bz = b2 & 0xf;
            //            if(vv[12] == 0x2 && vv[6] == 0xc4 && vv[16] == 0x01 && vv[17] == 0x00) { //链路检测 登陆
            //                int vvvv = 5;
            //                BYTE btemp[1024] = {0};
            //                btemp[0] = 0x68;
            //                btemp[5] = 0x68;
            //                btemp[6] = 0x04;
            //                memcpy(&btemp[7], &vv[7], 5);
            //                btemp[12] = 0x00;
            //                btemp[13] = bz;
            //                btemp[19] = 0x16;
            //                memcpy(&btemp[14], &vv[14], 4);
            //                int nbyte1 = 20 - 2 - 6;
            //                short n111 = (nbyte1 << 2) | 2;
            //                memcpy(&btemp[1], &n111, 2);
            //                memcpy(&btemp[3], &n111, 2);
            //                BYTE bh = 0;
            //                for(int j = 6; j < 18; j++) {
            //                    bh += btemp[j];
            //                }
            //                btemp[18] = bh;
            //                string ddd = gstring::char2hex((char*)btemp, 20);
            //                SetDlgItemTextA(hWindow, IDC_EDIT2, ddd.c_str());
            //                int vvvvv = 5;
            //            }
            //        }
            //    }
        }
    }
}
DWORD WINAPI ThreadProc();
int CALLBACK DialogProc(
    HWND hwndDlg,  // handle to dialog box
    UINT uMsg,     // message
    WPARAM wParam, // first message parameter
    LPARAM lParam  // second message parameter
);
HANDLE hThread;
DWORD Threadid;
void show();
void init(HWND hwnd)
{
    //vector<string>v_head;
    //v_head.push_back("ip");
    //v_head.push_back("socket");
    //v_head.push_back("设置发送数据");
    //v_head.push_back("key");
    //v_head.push_back("收到的数据");
    //v_head.push_back("收到asic数据");
    //v_head.push_back("客户端类型");
    //v_head.push_back("登陆状态");
    //list1.initList(GetDlgItem(hwnd, IDC_LIST1));
    //list1.insertHead(v_head);
    //list1.setColumWide(0, 180);
    //list1.setColumWide(2, 120);
}

typedef std::list<int > TESTLIST;

int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{


	SHORT vv=0x420;
	BYTE b1= vv>>8&0x00ff;  //04
	BYTE b2= vv&0x00ff;   //20

	glog::setOpenLog(TRUE);
	CoInitialize(NULL);
	show();
    return TRUE;
}
void show()
{


	int iret = 5;
	HRESULT Hr = ::CoInitialize(NULL);

	if(FAILED(Hr)) return ;

	LPVOID  a1=NULL;
	int     size1=0;

	gstring::getResInfo(a1, size1, IDR_ZIP1, "zip", hInst);
#ifdef _DEBUG
	CPaintManagerUI::SetResourcePath("skin");
#else
	CPaintManagerUI::SetResourceZip(a1, size1);
#endif

	CIOCP* pAddCard = new CIOCP("main.xml");
	//CMainFrame* pAddCard = new CMainFrame("main.xml");
	if(pAddCard == NULL) return;
	LPVOID pRes = NULL;
	int uSize = 0;
	HWND hwnd = pAddCard->Create(NULL, _T(""), UI_WNDSTYLE_DIALOG, WS_EX_WINDOWEDGE);

	RECT rc;
	pAddCard->CenterWindow();
	::ShowWindow(hwnd, SW_NORMAL);

	::UpdateWindow(hwnd);
	pAddCard->ShowModal();









	//MSG msg;
	//hWindow = CreateDialog(hInst, MAKEINTRESOURCE(IDD_DIALOG1), NULL, DLGPROC(DialogProc));
	//ShowWindow(hWindow, SW_NORMAL);
	//UpdateWindow(hWindow);
	//ShowCursor(TRUE);
	//DWORD progID = GetWindowThreadProcessId(hWindow, NULL);

	//while(GetMessage(&msg, NULL, 0, 0))
	//{
	//	TranslateMessage(&msg);
	//	DispatchMessage(&msg);
	//}
}
int CALLBACK DialogProc(
    HWND hwndDlg,  // handle to dialog box
    UINT uMsg,     // message
    WPARAM wParam, // first message parameter
    LPARAM lParam  // second message parameter
)
{
    int wID;
    int btime = FALSE;
    POINT pt;

    switch(uMsg)
    {
        case WM_INITDIALOG:
            init(hwndDlg);
            break;

        case WM_KEYDOWN:
            break;

        case WM_HOTKEY:
            break;

        case WM_NOTIFY:
            break;

        case WM_COMMAND:  //低位ID
            dealCommand(LOWORD(wParam));
            break;

        case WM_CLOSE:
            DestroyWindow(hwndDlg);
            TerminateProcess(GetCurrentProcess(), 0);
            break;
//  default:
            //  return DefWindowProc(hwndDlg,uMsg,wParam,lParam);
    }

    return FALSE;
}
DWORD WINAPI ThreadProc()
{
    //  MessageBox(NULL,"进入线程","ddddd",MB_OK);
    show();
    return 1;
}