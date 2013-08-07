// TList.h: interface for the CTList class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TLIST_H__0E347057_5168_4BA5_94DF_A3082EB87C56__INCLUDED_)
#define AFX_TLIST_H__0E347057_5168_4BA5_94DF_A3082EB87C56__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

typedef struct LNode {
    BYTE* pbBuffer;
    LONG lSize;
    double dwTimeStamp;
    struct LNode *next;
}LNode, *pNode;

class CTList  
{
public:
	CTList();
	virtual ~CTList();

    void Insert(pNode pN);
    BOOL getNode(pNode* ppN);
    void clrList();
    int getNdCount();

private:
    int iCount;
    pNode m_head;
    pNode m_tail;
    CRITICAL_SECTION critical_sec;

};

#endif // !defined(AFX_TLIST_H__0E347057_5168_4BA5_94DF_A3082EB87C56__INCLUDED_)
