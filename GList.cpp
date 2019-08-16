/*

Algorithm: Micro-cluster based DBSCAN
Author: Aditya Sarma
email: asaditya1195@gmail.com


*/

#include "GList.h"

GHdrNd GinitHdrNd()
{   
	GHdrNd HdrNdLst = (GHdrNd)calloc(1, sizeof(struct GhdrNd));
	// assert(HdrNdLst!=NULL);    

	HdrNdLst->uiCnt = 0;
	HdrNdLst->ptrFirstNd = NULL;
	HdrNdLst->ptrParentNd = NULL;
	
	return HdrNdLst;
}

GLstNd GinitLstNd(GTreeNode tnInfo)
{   
	GLstNd LstNdElem = (GLstNd)calloc(1, sizeof(struct GlstNd));
	// assert(LstNdElem!=NULL);	

	LstNdElem->tnInfo = tnInfo;
	LstNdElem->ptrChildLst = NULL;
	LstNdElem->ptrNextNd = NULL;
	
	return LstNdElem;
		
}

void GinsertLstElem(GHdrNd HdrNdLst, GTreeNode tnInfo)
{	
	GinsertLstNd(HdrNdLst, GinitLstNd(tnInfo));
	return;
}

void GinsertLstNd(GHdrNd HdrNdLst, GLstNd LstNdElem)
{   
	if(LstNdElem == NULL || HdrNdLst == NULL)
		return;
    LstNdElem->ptrNextNd = HdrNdLst->ptrFirstNd;
	HdrNdLst->ptrFirstNd = LstNdElem;
	HdrNdLst->uiCnt++;
	
	return;
}

Boolean GisLstEmpty(GHdrNd HdrNdLst)
{   

	return(HdrNdLst->ptrFirstNd == NULL || HdrNdLst->uiCnt == 0) ? TRUE : FALSE;
}

GLstNd GdeleteLstElem(GHdrNd HdrNdLst, GTreeNode tnInfo)
{ if(GisLstEmpty(HdrNdLst))
    return NULL;
  GTreeNode tnInfoTemp = NULL;
  Boolean bIsFound = FALSE;
  GLstNd LstNdTemp = HdrNdLst->ptrFirstNd;
  if(LstNdTemp->tnInfo == tnInfo)
  { 
    	return GdeleteLstFirst(HdrNdLst);
  }

  while(LstNdTemp->ptrNextNd != NULL)
  {   
  	if(LstNdTemp->ptrNextNd->tnInfo == tnInfo)
    {   
      	bIsFound = TRUE;
    	return GdeleteNextNd(HdrNdLst, LstNdTemp);
	}   
    
    LstNdTemp = LstNdTemp->ptrNextNd;
  }
  return NULL;
}

GLstNd GdeleteLstFirst(GHdrNd HdrNdLst)
{
   if(GisLstEmpty(HdrNdLst))
		return NULL;
	GTreeNode tnInfoTemp = NULL;
	GLstNd lstNdTemp = HdrNdLst->ptrFirstNd;

	HdrNdLst->ptrFirstNd = lstNdTemp->ptrNextNd;
	HdrNdLst->uiCnt--;
	lstNdTemp->ptrNextNd = NULL;

		
	return lstNdTemp;
}

GLstNd GdeleteNextNd(GHdrNd HdrNdLst, GLstNd LstNdElem)
{
   if(GisLstEmpty(HdrNdLst) || LstNdElem == NULL || LstNdElem->ptrNextNd == NULL)	
		return NULL;

	GLstNd LstNdTemp = LstNdElem->ptrNextNd;
    LstNdElem->ptrNextNd = LstNdTemp->ptrNextNd;
	LstNdTemp->ptrNextNd = NULL;	
	HdrNdLst->uiCnt--;

	return LstNdTemp;
}
