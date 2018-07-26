#include "DBOperation.h"
#include <stdio.h>
CDBOperation::CDBOperation(void)
{
    CoInitialize(NULL);
    m_pConnection = CreateConnPtr();
    m_pCommand = CreateCommPtr();
}

CDBOperation::~CDBOperation(void)
{
    if(m_pConnection->State) {
        m_pConnection->Close();
    }

    //
}

bool CDBOperation::ConnToDB(const char source[], const char database[], const char UserID[], const char Password[])
{
    //SQLOLEDB
    char connectch[1024] = "Provider=SQLOLEDB; Server=";//(local)\\GSQL;Database=HPTSoft";
    strcat(connectch, source);
    strcat(connectch, ";Database=");
    strcat(connectch, database);
    strcat(connectch, ";");

    if(NULL == m_pConnection)
        return false;

    try {
        HRESULT hr = m_pConnection->Open(connectch, UserID, Password, NULL);

        if(TRUE == FAILED(hr))
            return false;

        m_pCommand->ActiveConnection = m_pConnection;
        return true;
    } catch(_com_error e) {
        PrintErrorInfo(e);
        return false;
    }
}

_RecordsetPtr CDBOperation::ExecuteWithResSQL(const char *sql)
{
    //已经在连接至数据库的时候进行判断了
    if(NULL == m_pCommand || 0 == m_pConnection->State) {
        printf("Failed to create command OR the state of connection is zero\n");
        return NULL;
    }

    //char *query = new char;
    //strcpy(query, sql);
    try {
        m_pCommand->CommandText = _bstr_t(sql);
        _RecordsetPtr pRst = m_pCommand->Execute(NULL, NULL, adCmdText);
        return pRst;
        //_variant_t ra;
        //_RecordsetPtr pRst = m_pConnection->Execute((_bstr_t)query, &ra, adCmdText);
    } catch(_com_error &e) {
        PrintErrorInfo(e);
        return NULL;
    }
}



BOOL CDBOperation::IsNUll(_RecordsetPtr& rs)
{
    if(rs->adoEOF && rs->BOF) {
        return TRUE;
    }

    return FALSE;
}

int CDBOperation::GetNum(_RecordsetPtr& rs)
{
    __try {
        int i = 0;
        BOOL bret = FALSE;

        while(!rs->adoEOF) {
            i++;
            rs->MoveNext();
        }

        if(i > 0) {
            rs->MoveFirst();
        }

        return i;
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        ULONG code = GetExceptionCode();
        return 0;
    }
}

BOOL CDBOperation::IsHasdata(_RecordsetPtr& rs)
{
    BOOL bret = FALSE;

    while(!rs->adoEOF) {
        bret = TRUE;
        rs->MoveNext();
    }

    return bret;
}

string CDBOperation::GetInsertSql(map<string, _variant_t>&m_str, string table)
{
    map<string, _variant_t>::iterator ite;
    map<string, _variant_t>::iterator ite1;
    string str1 = "(";
    string str2 = "(";
    string uu = "insert into ";

    for(ite = m_str.begin(); ite != m_str.end(); ite++)  {
        ite1 = ite;
        string s1 = ite->first;
        _variant_t v0 = ite->second;

        if(++ite1 != m_str.end()) {
            str1 = str1 + s1 + string(",");

            if(v0.vt == VT_BSTR) {
                string s2 = _com_util::ConvertBSTRToString(v0.bstrVal);

                if(s2.find("(") == s2.npos) {
                    str2 = str2 + string("'") + s2 + string("'") + string(",");
                } else {
                    str2 = str2 + s2 +  string(",");
                }
            }

            if(v0.vt == VT_INT) {
                char pval[216] = {0};
                string val = _itoa(v0.iVal, pval, 10);
                str2 = str2 + val  + string(",");
            }

            if(v0.vt == VT_R8) {
                char pval[216] = {0};
				sprintf(pval,"%0.2f",v0.dblVal);
                str2 = str2 + pval  + string(",");
            }
        } else {
            str1 = str1 + s1 + string(")");

            if(v0.vt == VT_BSTR) {
                string s2 = _com_util::ConvertBSTRToString(v0.bstrVal);

                if(s2.find("(") == s2.npos) {
                    str2 = str2 + string("'") + s2 + string("'") + string(")");;
                } else {
                    str2 = str2  + s2  + string(")");;
                }
            }

            if(v0.vt == VT_INT) {
                char pval[216] = {0};
                string val = _itoa(v0.iVal, pval, 10);
                str2 = str2 + val + string(")");;
            }
        }
    }

    uu.append(table);
    uu.append(str1.c_str());
    uu.append(" values");
    uu.append(str2.c_str());
    return uu;
}

void CDBOperation::PrintErrorInfo(_com_error &e)
{
    printf("Error infomation are as follows\n");
    printf("ErrorNo: %d\nError Message:%s\nError Source:%s\nDescription:%s\n", e.Error(), (LPCTSTR)e.ErrorMessage(), (LPCTSTR) e.Source(), (LPCTSTR)e.Description());
}

_ConnectionPtr CDBOperation::CreateConnPtr()
{
    HRESULT hr;
    _ConnectionPtr connPtr;
    hr = connPtr.CreateInstance(__uuidof(Connection));

    if(FAILED(hr) == TRUE)
        return NULL;

    return connPtr;
}

_CommandPtr CDBOperation::CreateCommPtr()
{
    HRESULT hr;
    _CommandPtr commPtr;
    hr = commPtr.CreateInstance(__uuidof(Command));

    if(FAILED(hr) == TRUE)
        return NULL;

    return commPtr;
}

_RecordsetPtr CDBOperation::CreateRecsetPtr()
{
    HRESULT hr;
    _RecordsetPtr recsetPtr;
    hr = recsetPtr.CreateInstance(__uuidof(Command));

    if(FAILED(hr) == TRUE)
        return NULL;

    return recsetPtr;
}

_RecordsetPtr CDBOperation::ExecuteWithResSQLProc(const char* sql)
{
    //已经在连接至数据库的时候进行判断了
    if(NULL == m_pCommand || 0 == m_pConnection->State) {
        printf("Failed to create command OR the state of connection is zero\n");
        return NULL;
    }

    //char *query = new char;
    //strcpy(query, sql);
    try {
//oneValule = m_pCommand->CreateParameter("oneValule ",adVarChar,adParamInput,10,(_variant_t)“传入的值”);//给参数设置各属性
        m_pCommand->CommandText = _bstr_t(sql);
        m_pCommand->CommandType = adCmdStoredProc;
        _RecordsetPtr pRst =    m_pCommand->Execute(NULL, NULL, NULL);
        return pRst;
//         _RecordsetPtr pRst = m_pCommand->Execute(NULL, NULL, NULL);
//         return pRst;
        //_variant_t ra;
        //_RecordsetPtr pRst = m_pConnection->Execute((_bstr_t)query, &ra, adCmdText);
    } catch(_com_error &e) {
        LPCTSTR ss = (LPCTSTR)e.Description();
        PrintErrorInfo(e);
        return NULL;
    }
}

std::string CDBOperation::GetUpdateSql(map<string, _variant_t>&m_str, string table, string strwhere)
{
    map<string, _variant_t>::iterator ite;
    map<string, _variant_t>::iterator ite1;
    string str1 = "";
    string str2 = "";
    string uu = "update " + table + string(" set ");

    for(ite = m_str.begin(); ite != m_str.end(); ite++)  {
        ite1 = ite;
        string s1 = ite->first ;
        _variant_t v0 = ite->second;

        if(++ite1 != m_str.end()) {
            if(v0.vt == VT_BSTR) {
                string s2 = _com_util::ConvertBSTRToString(v0.bstrVal);

                if(s2.find("(") == s2.npos) {
                    str1 = str1 + s1 + string("='") + s2 + string("'") + string(",");
                } else {
                    //str2 = str2  + s2  + string(")");
                    str1 = str1 + s1 + string("=") + s2 + string("") + string(",");
                }
            } else if(v0.vt == VT_INT) {
                char data[20] = {0};
                sprintf(data, "%d", v0.iVal);
                string s2 = data;
                str1 = str1 + s1 + string("=") + s2  + string(",");
            }
        } else {
            if(v0.vt == VT_BSTR) {
                string s2 = _com_util::ConvertBSTRToString(v0.bstrVal);
                str1 = str1 + s1 + string("='") + s2 + string("'");
            } else if(v0.vt == VT_INT) {
                char data[20] = {0};
                sprintf(data, "%d", v0.iVal);
                string s2 = data;
                str1 = str1 + s1 + string("=") + s2;
            }
        }
    }

    uu.append(str1);
    uu.append("   ");
    uu.append(strwhere);
    return uu;
}
