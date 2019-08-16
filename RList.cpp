
/*

Algorithm: Micro-cluster based DBSCAN
Author: Aditya Sarma
email: asaditya1195@gmail.com

*/

#include "RList.h"

RHdrNd RinitHdrNd()
{   
    RHdrNd HdrNdLst = (RHdrNd)calloc(1, sizeof(struct RhdrNd));
    // assert(HdrNdLst != NULL);    

    HdrNdLst->uiCnt = 0;
    HdrNdLst->ptrFirstNd = NULL;
    HdrNdLst->ptrParentNd = NULL;
    
    return HdrNdLst;
}

RLstNd RinitLstNd(RTreeNode tnInfo)
{   
    RLstNd LstNdElem = (RLstNd)calloc(1, sizeof(struct RlstNd));
    // assert(LstNdElem != NULL);    

    LstNdElem->tnInfo = tnInfo;
    LstNdElem->ptrChildLst = NULL;
    LstNdElem->ptrNextNd = NULL;
    
    return LstNdElem;   
}

void RinsertLstElem(RHdrNd HdrNdLst, RTreeNode tnInfo)
{   
    RinsertLstNd(HdrNdLst, RinitLstNd(tnInfo));
    return;
}

void RinsertLstNd(RHdrNd HdrNdLst, RLstNd LstNdElem)
{   
    if(LstNdElem == NULL || HdrNdLst == NULL)
        return;
    LstNdElem->ptrNextNd = HdrNdLst->ptrFirstNd;
    HdrNdLst->ptrFirstNd = LstNdElem;
    HdrNdLst->uiCnt++;
    
    return;
}

Boolean RisLstEmpty(RHdrNd HdrNdLst)
{   
    if(HdrNdLst!=NULL)
    {
        if(HdrNdLst->ptrFirstNd == NULL)
        return TRUE;

        else if(HdrNdLst->uiCnt == 0)
            return TRUE;

        else
            return FALSE;
    
    }
    
    return TRUE;
    
}

RLstNd RdeleteLstElem(RHdrNd HdrNdLst, RTreeNode tnInfo)
{ 

  if(RisLstEmpty(HdrNdLst))
    return NULL;
  RTreeNode tnInfoTemp = NULL;
  Boolean bIsFound = FALSE;
  RLstNd LstNdTemp = HdrNdLst->ptrFirstNd;
  if(LstNdTemp->tnInfo == tnInfo)
    { return RdeleteLstFirst(HdrNdLst);  }

  while(LstNdTemp->ptrNextNd != NULL)
    {   if(LstNdTemp->ptrNextNd->tnInfo == tnInfo)
        {   bIsFound = TRUE;
        
      return RdeleteNextNd(HdrNdLst, LstNdTemp);
    }   
    LstNdTemp = LstNdTemp->ptrNextNd;
  }
  
  return NULL;
}

RLstNd RdeleteLstFirst(RHdrNd HdrNdLst)
{   
    if(RisLstEmpty(HdrNdLst))
        return NULL;
    RTreeNode tnInfoTemp = NULL;
    RLstNd lstNdTemp = HdrNdLst->ptrFirstNd;

    HdrNdLst->ptrFirstNd = lstNdTemp->ptrNextNd;
    HdrNdLst->uiCnt--;
    lstNdTemp->ptrNextNd = NULL;

        
    return lstNdTemp;
}

RLstNd RdeleteNextNd(RHdrNd HdrNdLst, RLstNd LstNdElem)
{   
    if(RisLstEmpty(HdrNdLst) || LstNdElem == NULL || LstNdElem->ptrNextNd == NULL)  
        return NULL;

    RLstNd LstNdTemp = LstNdElem->ptrNextNd;
    LstNdElem->ptrNextNd = LstNdTemp->ptrNextNd;
    LstNdTemp->ptrNextNd = NULL;    
    HdrNdLst->uiCnt--;

    return LstNdTemp;
}


RNbHdr RinitNbHdr()
{
    RNbHdr nbHdrInfo = (RNbHdr)calloc(1, sizeof(struct RnbHdr));
    if(nbHdrInfo == NULL)
        return NULL;

    nbHdrInfo->nbhCnt = 0;
    nbHdrInfo->nbFirst =NULL;
    nbHdrInfo->nbLast = NULL;

    return nbHdrInfo;
}

Boolean RisNbLstEmpty(RNbHdr nbHdrInfo)
{    
  
    return(nbHdrInfo == NULL || nbHdrInfo->nbhCnt == 0) ? TRUE : FALSE;
}
