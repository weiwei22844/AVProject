// TList.cpp: implementation of the CTList class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TList.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CTList::CTList()
{
    iCount = 0;
    m_head = NULL;
    m_tail = NULL;
    InitializeCriticalSection(&critical_sec);
}

CTList::~CTList()
{
    clrList();
    DeleteCriticalSection(&critical_sec);
}

void CTList::Insert(pNode pN)
{
    EnterCriticalSection(&critical_sec);
    if(m_head == NULL)
    {
        m_head = pN;
        m_tail = pN;
        m_tail->next = NULL;
    }else{
        m_tail->next = pN;
        m_tail = pN;
        m_tail->next = NULL;
    }
    iCount ++;
    LeaveCriticalSection(&critical_sec);
}

BOOL CTList::getNode(pNode* ppN)
{
    EnterCriticalSection(&critical_sec);
    if(m_head == NULL)
    {
        *ppN = NULL;
        LeaveCriticalSection(&critical_sec);
        return FALSE;
    }else{
        *ppN = m_head;
        m_head = m_head->next;
        iCount --;
        LeaveCriticalSection(&critical_sec);
        return TRUE;
    }
}

void CTList::clrList()
{
    pNode pT = NULL;
    EnterCriticalSection(&critical_sec);
    while(m_head)
    {
        pT = m_head;
        m_head = m_head->next;
        delete pT->pbBuffer;
        pT->pbBuffer = NULL;
    }
    iCount = 0;
    LeaveCriticalSection(&critical_sec);
}

int CTList::getNdCount()
{
    int count = 0;
    EnterCriticalSection(&critical_sec);
    count = iCount;
    LeaveCriticalSection(&critical_sec);
    return count;
}