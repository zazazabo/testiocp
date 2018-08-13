#ifndef __DOUBLE_LIST_H_
#define __DOUBLE_LIST_H_
#pragma once
#include <map>
#include <list>
#include <string>
#include <vector>
#include <iterator>
using namespace std;
///////////////////////////////////////////////////////////////
#define MSG(msg) { cout << (msg) <<endl; }
///////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////

template<class T, int COUNT>
class CDoubleList
{
public:
    typedef list<T*> LIST;
    typedef  typename list<T*>::iterator ITERATOR;
public:
    CDoubleList();
    virtual ~CDoubleList();
    LIST                m_0_list;
    LIST                m_1_list;
private:
    CRITICAL_SECTION    m_critical_section;
protected:

private:
    inline void         Base_InitSize(LIST &list, int count);
    inline void         Base_RemoveAll(LIST &list);

    inline void         Init();
    inline void         RemoveAll();
public:
    int                 GetCount();
    int                 GetBlankCount();

    T*                  GetBlank();
    T*                  RemoveAt(ITERATOR   pos);
    T*                  RemoveAt(T*         lp_t);

    T*                  RemoveAtUse(T*         lp_t);

    T*                  GetHeadPosition(ITERATOR& pos);


    T*                  GetEndPosition(ITERATOR& pos);


    BOOL                bGetIteratior(ITERATOR&pos);
    T*                  GetNext(ITERATOR& pos);

    T*                  GetHead();
    T*                  RemoveHead();

    T*                  GetOwnCycleNext();

    void                PutToBlank();

    ////////////////////////////////////////////////////////////

    T*                  GetFromBlank();
    T*                  PutToBlank(T*lp_t);
    T*                  GetFromUse();
    T*                  PutToUse(T*lp_t);
};

template<class T, int COUNT>
T* CDoubleList<T, COUNT>::GetEndPosition(ITERATOR& pos)
{
    T* lp_t = NULL;
    EnterCriticalSection(&m_critical_section);
    pos = m_1_list.end();
    LeaveCriticalSection(&m_critical_section);
    return NULL;
}


//template<class T, int COUNT>
//T* CDoubleList<T, COUNT>::GetNext_(PVOID& pos)
//{
//    T*  lp_t = NULL;
//    EnterCriticalSection(&m_critical_section);
//    ITERATOR ite = (ITERATOR)pos;
//    ite++;
//
//    if(ite == m_1_list.end())
//    {
//        ite = NULL;
//    }
//    else
//    {
//        lp_t = *ite;
//    }
//
//    LeaveCriticalSection(&m_critical_section);
//    return lp_t;
//}

template<class T, int COUNT>
BOOL CDoubleList<T, COUNT>::bGetIteratior(ITERATOR&pos)
{
    if(pos == m_1_list.end())
    {
        return FALSE;
    }

    return TRUE;
}

//
// //////////////////////////////////////////////////////////////////////////////////////////////////////////////
// /*---------------------------------------------------------------------------------
// ���ܣ����캯��
// ˵����
// ���أ�
// ---------------------------------------------------------------------------------*/
template<class T, int COUNT>
CDoubleList<T, COUNT>::CDoubleList()
{
    ::InitializeCriticalSection(&m_critical_section);
    Init();
}

/*---------------------------------------------------------------------------------
���ܣ���������
˵����
���أ�
---------------------------------------------------------------------------------*/
template<class T, int COUNT>
CDoubleList<T, COUNT>::~CDoubleList()
{
//  RemoveAll();
    ::DeleteCriticalSection(&m_critical_section);
}

/*---------------------------------------------------------------------------------
���ܣ���ʼ��һ���б�������������ڴ�
˵����
���أ�
---------------------------------------------------------------------------------*/
template<class T, int COUNT>
void CDoubleList<T, COUNT>::Base_InitSize(LIST &list, int count)
{
    T   *lp_t = NULL;

    for(int i = 0; i < count; i++)
    {
        lp_t = new T;
        list.push_back(lp_t);
    }
}

/*---------------------------------------------------------------------------------
���ܣ�ɾ��һ���б�������Ԫ�أ����ͷ��ڴ�
˵����
���أ�
---------------------------------------------------------------------------------*/
template<class T, int COUNT>
void CDoubleList<T, COUNT>::Base_RemoveAll(LIST &list)
{
    for(ITERATOR i = list.begin(); i != list.end(); i++)
    {
        delete *i;
    }

    list.clear();
}




/*---------------------------------------------------------------------------------
���ܣ���ʼ��
˵����
���أ�
---------------------------------------------------------------------------------*/
template<class T, int COUNT>
void CDoubleList<T, COUNT>::Init()
{
    EnterCriticalSection(&m_critical_section);
    Base_RemoveAll(m_0_list);
    Base_RemoveAll(m_1_list);
    Base_InitSize(m_0_list, COUNT);
    LeaveCriticalSection(&m_critical_section);
}

/*---------------------------------------------------------------------------------
���ܣ���������ڴ�
˵����
���أ�
---------------------------------------------------------------------------------*/
template<class T, int COUNT>
void CDoubleList<T, COUNT>::RemoveAll()
{
    EnterCriticalSection(&m_critical_section);
    Base_RemoveAll(m_0_list);
    Base_RemoveAll(m_1_list);
    LeaveCriticalSection(&m_critical_section);
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*---------------------------------------------------------------------------------
���ܣ��õ�ʹ�ñ�Ľ������
˵����
���أ�
---------------------------------------------------------------------------------*/
template<class T, int COUNT>
int CDoubleList<T, COUNT>::GetCount()
{
    int     count = 0;
    EnterCriticalSection(&m_critical_section);
    count = m_1_list.size();
    LeaveCriticalSection(&m_critical_section);
    return count;
}

/*---------------------------------------------------------------------------------
���ܣ��õ��ձ�Ľ������
˵����
���أ�
---------------------------------------------------------------------------------*/
template<class T, int COUNT>
int CDoubleList<T, COUNT>::GetBlankCount()
{
    int     count = 0;
    EnterCriticalSection(&m_critical_section);
    count = m_0_list.size();
    LeaveCriticalSection(&m_critical_section);
    return count;
}

/*---------------------------------------------------------------------------------
���ܣ��ӿձ�������ϵͳ����һ�����
˵��������ձ��Ѿ��޽�㣬��ϵͳ���޿����ڴ棬�˺����Ż�ʧ�ܡ�
���أ��ɹ�����������Ľ�㣻ʧ�ܣ�NULL
---------------------------------------------------------------------------------*/
template<class T, int COUNT>
T* CDoubleList<T, COUNT>::GetBlank()
{
    T*  lp_t =  NULL;
    ITERATOR    pos;
    EnterCriticalSection(&m_critical_section);
    pos = m_0_list.begin();

    if(pos == m_0_list.end())     //������ʱ�����ڵ�
    {
        lp_t = new T;
    }
    else
    {
        lp_t = *pos;
        m_0_list.pop_front();
    }

    //��Ϊ�п����ڴ治��������ڴ�ʧ��
    if(lp_t != NULL)
    {
        m_1_list.push_back(lp_t);
    }
    LeaveCriticalSection(&m_critical_section);
    return lp_t;
}



/*---------------------------------------------------------------------------------
���ܣ����ݽ��ָ�룬ɾ��ʹ�ñ��е�һ�����
˵����
���أ�
---------------------------------------------------------------------------------*/
template<class T, int COUNT>
T* CDoubleList<T, COUNT>::RemoveAtUse(T* lp_t)
{
    EnterCriticalSection(&m_critical_section);
    m_1_list.remove(lp_t);
    //m_0_list.push_back(lp_t);
    LeaveCriticalSection(&m_critical_section);
    return lp_t;
}



/*---------------------------------------------------------------------------------
���ܣ����ݵ�������ɾ��ʹ�ñ��е�һ�����
˵����
���أ�
---------------------------------------------------------------------------------*/
template<class T, int COUNT>
T* CDoubleList<T, COUNT>::RemoveAt(ITERATOR pos)
{
    T*  lp_t = NULL;
    EnterCriticalSection(&m_critical_section);
    lp_t = *pos;
    m_1_list.erase(pos);
    m_0_list.push_back(lp_t);
    LeaveCriticalSection(&m_critical_section);
    return lp_t;
}



/*---------------------------------------------------------------------------------
���ܣ����ݽ��ָ�룬ɾ��ʹ�ñ��е�һ�����
˵����
���أ�
---------------------------------------------------------------------------------*/
template<class T, int COUNT>
T* CDoubleList<T, COUNT>::RemoveAt(T* lp_t)
{
    EnterCriticalSection(&m_critical_section);
    int n1 = m_1_list.size();
    m_1_list.remove(lp_t);
    int n2 = m_1_list.size();

    if(n1 != n2)
    {
        m_0_list.push_back(lp_t);
    }

    LeaveCriticalSection(&m_critical_section);
    return lp_t;
}







/*---------------------------------------------------------------------------------
���ܣ��õ�ʹ�ñ�ͷ���ĵ�����
˵����
���أ�
---------------------------------------------------------------------------------*/
template<class T, int COUNT>
T* CDoubleList<T, COUNT>::GetHeadPosition(ITERATOR& pos)
{
    T* lp_t = NULL;
    EnterCriticalSection(&m_critical_section);
    pos = m_1_list.begin();

    if(pos == m_1_list.end())
    {
        pos = m_1_list.end();
        lp_t = NULL;
    }
    else
    {
        lp_t = *pos;
    }

    LeaveCriticalSection(&m_critical_section);
    return lp_t;
}





/*---------------------------------------------------------------------------------
���ܣ����ݵ��������õ���㡣��ʹ���������β�����ƶ�һ�����
˵�����������NULL��˵���Ѿ����˱�β
���أ�
---------------------------------------------------------------------------------*/
template<class T, int COUNT>
T* CDoubleList<T, COUNT>::GetNext(ITERATOR&pos)
{
    T*  lp_t = NULL;
    EnterCriticalSection(&m_critical_section);
    //if(pos == m_1_list.end())
    //{
    //    lp_t = NULL;
    //    return lp_t;
    //}
    pos++;

    if(pos == m_1_list.end())
    {
        lp_t = NULL;
    }
    else
    {
        lp_t = *pos;
    }

    LeaveCriticalSection(&m_critical_section);
    return lp_t;
}

/*---------------------------------------------------------------------------------
���ܣ��õ�ʹ�ñ�ı�ͷ��㣬��������
˵����
���أ�
---------------------------------------------------------------------------------*/
template<class T, int COUNT>
T* CDoubleList<T, COUNT>::GetHead()
{
    T*  lp_t = NULL;
    ITERATOR    pos;
    EnterCriticalSection(&m_critical_section);
    pos = m_1_list.begin();

    if(pos != m_1_list.end())
    {
        lp_t = *pos;
    }

    LeaveCriticalSection(&m_critical_section);
    return lp_t;
}




/*---------------------------------------------------------------------------------
���ܣ�����ʹ�ñ�ı�ͷ��㵽�ձ��С�
˵����
���أ�
---------------------------------------------------------------------------------*/
template<class T, int COUNT>
T* CDoubleList<T, COUNT>::RemoveHead()
{
    T*          lp_t = NULL;
    ITERATOR    pos;
    EnterCriticalSection(&m_critical_section);
    pos = m_1_list.begin();

    if(pos != m_1_list.end())
    {
        lp_t = *pos;
        m_1_list.pop_front();
        m_0_list.push_back(lp_t);
    }

    LeaveCriticalSection(&m_critical_section);
    return lp_t;
}

/*---------------------------------------------------------------------------------
���ܣ��ӿձ�ʹ�ñ���ȡ��һ����㣬�ŵ�ʹ�ñ�ı�ͷ
˵����
���أ�
---------------------------------------------------------------------------------*/
template<class T, int COUNT>
T* CDoubleList<T, COUNT>::GetOwnCycleNext()
{
    T*          lp_t = NULL;
    ITERATOR    pos;
    EnterCriticalSection(&m_critical_section);
    pos = m_0_list.begin();

    if(pos != m_0_list.end())
    {
        lp_t = *pos;
        m_0_list.pop_front();
    }
    else
    {
        pos = m_1_list.begin();

        if(pos != m_1_list.end())
        {
            lp_t = *pos;
            m_1_list.pop_front();
        }
    }

    if(lp_t != NULL)
    {
        m_1_list.push_back(lp_t);
    }

    LeaveCriticalSection(&m_critical_section);
    return lp_t;
}

/*---------------------------------------------------------------------------------
���ܣ���ʹ�ñ��е����н�㶼�Ƶ��ձ���
˵����
���أ�
---------------------------------------------------------------------------------*/
template<class T, int COUNT>
void CDoubleList<T, COUNT>::PutToBlank()
{
    ITERATOR    pos;
    EnterCriticalSection(&m_critical_section);

    for(pos = m_1_list.begin(); pos != m_1_list.end(); pos++)
    {
        m_0_list.push_back(*pos);
    }

    m_1_list.clear();
    LeaveCriticalSection(&m_critical_section);
}

/*---------------------------------------------------------------------------------
���ܣ��ӿձ�ȡ��һ����㣬�������ŵ�ʹ�ñ��С��˽����ʱ����
˵����
���أ�
---------------------------------------------------------------------------------*/
template<class T, int COUNT>
T* CDoubleList<T, COUNT>::GetFromBlank()
{
    T*  lp_t =  NULL;
    ITERATOR    pos;
    EnterCriticalSection(&m_critical_section);
    pos = m_0_list.begin();

    if(pos == m_0_list.end())
    {
        lp_t = new T;
    }
    else
    {
        lp_t = *pos;
        m_0_list.pop_front();
    }

    LeaveCriticalSection(&m_critical_section);
    return lp_t;
}

/*---------------------------------------------------------------------------------
���ܣ���һ�����ŵ��ձ��С�
˵����
���أ�
---------------------------------------------------------------------------------*/
template<class T, int COUNT>
T* CDoubleList<T, COUNT>::PutToBlank(T* lp_t)
{
    EnterCriticalSection(&m_critical_section);
    m_0_list.push_back(lp_t);
    LeaveCriticalSection(&m_critical_section);
    return lp_t;
}

/*---------------------------------------------------------------------------------
���ܣ���ʹ�ñ�ȡ��һ����㣬�������ŵ��ձ��С��˽����ʱ����
˵����
���أ�
---------------------------------------------------------------------------------*/
template<class T, int COUNT>
T* CDoubleList<T, COUNT>::GetFromUse()
{
    T*  lp_t =  NULL;
    ITERATOR    pos;
    EnterCriticalSection(&m_critical_section);
    pos = m_1_list.begin();

    if(pos != m_1_list.end())
    {
        lp_t = *pos;
        m_1_list.pop_front();
    }

    LeaveCriticalSection(&m_critical_section);
    return lp_t;
}

/*---------------------------------------------------------------------------------
���ܣ���һ�����ŵ�ʹ�ñ��С�
˵����
���أ�
---------------------------------------------------------------------------------*/
template<class T, int COUNT>
T* CDoubleList<T, COUNT>::PutToUse(T* lp_t)
{
    EnterCriticalSection(&m_critical_section);
    m_1_list.push_back(lp_t);
    LeaveCriticalSection(&m_critical_section);
    return lp_t;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif  //__DOUBLE_LIST_H_