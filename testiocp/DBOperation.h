#pragma once

#include "E:\\code\\glib\\h\\glog.h"

#ifdef _DEBUG
#ifdef _WIN64
#else
#pragma comment(lib,"E:\\code\\glib\\lib\\glib_d.lib")
#endif
#else
#ifdef _WIN64
#pragma comment(lib,"E:\\code\\glib\\x64\\Release\\glib.lib")

#else
#pragma comment(lib,"E:\\code\\glib\\Release\\glib.lib")
#endif
#endif
#include <map>
#include <string>
#import "C:\\Program Files (x86)\\Common Files\\System\\ado\\msado15.dll" no_namespace rename("EOF", "adoEOF")
//#import "F:\\msado15.dll" no_namespace rename("EOF", "adoEOF")
using namespace std;
class CDBOperation
{
public:
    //���ݿ�������Ҫ�����ӡ������������
    _ConnectionPtr m_pConnection;
    _CommandPtr m_pCommand;

    //��ʼ�����ݿ������Ҫ�Ķ���
    CDBOperation(void);
    ~CDBOperation(void);
    //���������ݿ�
    bool CDBOperation::ConnToDB(const char source[], const char database[], const char UserID[], const char Password[]);
    //���ݿ��������
    //��ѯ���� ɾ���Լ����
    _RecordsetPtr ExecuteWithResSQL(const char*);

    _RecordsetPtr ExecuteWithResSQLProc(const char*);

    //bool ExecuteNoResSQL(const char *);//delete and add

    BOOL  IsNUll(_RecordsetPtr& rs);
    int   GetNum(_RecordsetPtr& rs);
    BOOL  IsHasdata(_RecordsetPtr& rs);
    string GetInsertSql(map<string, _variant_t>&m_str, string table);
    string GetUpdateSql(map<string, _variant_t>&m_str, string table, string strwhere);

private:
    void PrintErrorInfo(_com_error&);
    //��ʼ�����ݿ����ӡ������¼��
    _ConnectionPtr  CreateConnPtr();
    _CommandPtr     CreateCommPtr();
    _RecordsetPtr  CreateRecsetPtr();

};