/*
Algorithm: Micro-cluster based DBSCAN
Author: Aditya Sarma
email: asaditya1195@gmail.com
*/

#include "MuC_RTree.h"
#include "MuC.h"
#include "vectorc.h"
#include <stdbool.h>
#include <unistd.h>

Boolean GisOverLap(Region rgnRectOne, Region rgnRectTwo)
{   

	int iDim = 0;
	for(iDim = 0; iDim < DIMENSION; iDim++)
		if(rgnRectOne->iTopRight[iDim] <= rgnRectTwo->iBottomLeft[iDim] || rgnRectTwo->iTopRight[iDim] < rgnRectOne->iBottomLeft[iDim])
			return FALSE;
	return TRUE;
}

Region GinitRgnRect(Dimension iBottomLeft, Dimension iTopRight)
{   

	Region rgnRect = (Region)calloc(1, sizeof(struct region));
	// assert(rgnRect!=NULL);
    
    if(iBottomLeft != NULL)
		rgnRect->iBottomLeft = iBottomLeft;
	else{
		rgnRect->iBottomLeft = (Dimension)calloc(DIMENSION, sizeof(double));
		// assert(rgnRect->iBottomLeft!=NULL);
	}

	if(iTopRight != NULL)
		rgnRect->iTopRight = iTopRight;
	else{
		rgnRect->iTopRight = (Dimension)calloc(DIMENSION, sizeof(double));
		// assert(rgnRect->iTopRight!=NULL);

	}

	return rgnRect;
}

GTreeNode GinitIntNd(Dimension iBottomLeft, Dimension iTopRight)
{   

	Region rgnRect = GinitRgnRect(iBottomLeft, iTopRight);

	GTreeData tdInfo = (GTreeData)calloc(1, sizeof(*tdInfo));
	// assert(tdInfo!=NULL);

	tdInfo->rgnRect = rgnRect;
	tdInfo->groupElem = -1;
	
    GTreeNode tnInfo = (GTreeNode)calloc(1, sizeof(*tnInfo));
    // assert(tnInfo!=NULL);

	tnInfo->ndType = INTNODE;
	tnInfo->tdInfo = tdInfo;

	return tnInfo;

}

GTreeNode GinitExtNd(Group groupElem)
{
	// assert(groupElem!=NULL);

	GTreeNode tnInfo = (GTreeNode)calloc(1, sizeof(*tnInfo));
	// assert(tnInfo!=NULL);
	
	GTreeData tdInfo = (GTreeData)calloc(1, sizeof(*tdInfo));
    // assert(tdInfo!=NULL);

	tdInfo->groupElem = groupElem->id;
	tdInfo->rgnRect = NULL;

    tnInfo->ndType = EXTNODE;	
	tnInfo->tdInfo = tdInfo;

	return tnInfo;
}

//Both INTERNAL or both EXTERNAL
void GsetRect(GLstNd lstNd, GTreeNode tnInfo)
{   
	// if(lstNd->tnInfo->ndType)
	// 	fprintf(stderr, "------------------TreeNode: %d\t InsertingNode: %d------------------\n", lstNd->tnInfo->ndType, tnInfo->ndType);

    int iCnt = 0;
    switch(tnInfo->ndType)
    {   
    	case INTNODE:
			for(iCnt = 0; iCnt < DIMENSION; iCnt++)
	        {   
	        	lstNd->tnInfo->tdInfo->rgnRect->iBottomLeft[iCnt] = tnInfo->tdInfo->rgnRect->iBottomLeft[iCnt];
				lstNd->tnInfo->tdInfo->rgnRect->iTopRight[iCnt] = tnInfo->tdInfo->rgnRect->iTopRight[iCnt];
	     	}
			break;
        case EXTNODE:
			for(iCnt = 0; iCnt < DIMENSION; iCnt++)
	        {   
	        	lstNd->tnInfo->tdInfo->rgnRect->iBottomLeft[iCnt] = groupList->groupItems[tnInfo->tdInfo->groupElem]->bcell->minOriginalBoundary[iCnt];
				lstNd->tnInfo->tdInfo->rgnRect->iTopRight[iCnt] = groupList->groupItems[tnInfo->tdInfo->groupElem]->bcell->maxOriginalBoundary[iCnt];
			}
			break;
	}

	return;
}

GLstNd GpickChild(GHdrNd ptrChildLst, GTreeNode tnInfo)
{  
    if(ptrChildLst == NULL)
		return NULL;

	GLstNd lstNdTemp = ptrChildLst->ptrFirstNd;
	GLstNd lstNdChild = NULL;
	double dMinExp = -1;

    Region rgnNewRect = GinitRgnRect(NULL, NULL);
    Region rgnFinalRect = GinitRgnRect(NULL, NULL);

    int iCnt;

	while(lstNdTemp != NULL)
    {   
		if(GexpansionArea(lstNdTemp->tnInfo->tdInfo->rgnRect, tnInfo, &dMinExp, rgnNewRect))
        {
               if(dMinExp < 0)
               {     dMinExp = 0 - dMinExp;
                     if(Garea(lstNdChild->tnInfo->tdInfo->rgnRect) > Garea(lstNdTemp->tnInfo->tdInfo->rgnRect))
                     {
                     	lstNdChild = lstNdTemp;
                     	    for(iCnt =0; iCnt< DIMENSION; iCnt++)
					        {	rgnFinalRect->iBottomLeft[iCnt] = rgnNewRect->iBottomLeft[iCnt];
								rgnFinalRect->iTopRight[iCnt] = rgnNewRect->iTopRight[iCnt];
							}
                     }
                           
               }
			   else
			   {
			   		lstNdChild = lstNdTemp;
				    for(iCnt =0; iCnt< DIMENSION; iCnt++)
		         	{	rgnFinalRect->iBottomLeft[iCnt] = rgnNewRect->iBottomLeft[iCnt];
						rgnFinalRect->iTopRight[iCnt] = rgnNewRect->iTopRight[iCnt];
				 	}
			   }
		}
    	lstNdTemp = lstNdTemp->ptrNextNd;
	}

    Region rgnRectTemp = lstNdChild->tnInfo->tdInfo->rgnRect;
	lstNdChild->tnInfo->tdInfo->rgnRect = rgnFinalRect;

	free(rgnRectTemp->iBottomLeft);
	free(rgnRectTemp->iTopRight);
	free(rgnRectTemp);

	free(rgnNewRect->iBottomLeft);
	free(rgnNewRect->iTopRight);
	free(rgnNewRect);

	return lstNdChild;
}


Boolean GexpansionArea(Region rgnRect, GTreeNode tnInfo, Double ptrDMinExp, Region rgnNewRect)
{
    int iCnt = 0;
    Region rgnRectTemp = GinitRgnRect(NULL, NULL);

    for(iCnt = 0; iCnt < DIMENSION; iCnt++)
    {   
    	switch(tnInfo->ndType)
        {   
        	case INTNODE:
				rgnRectTemp->iTopRight[iCnt] = (tnInfo->tdInfo->rgnRect->iTopRight[iCnt] > rgnRect->iTopRight[iCnt]) ? tnInfo->tdInfo->rgnRect->iTopRight[iCnt] : rgnRect->iTopRight[iCnt];
				rgnRectTemp->iBottomLeft[iCnt] = (tnInfo->tdInfo->rgnRect->iBottomLeft[iCnt] < rgnRect->iBottomLeft[iCnt]) ? tnInfo->tdInfo->rgnRect->iBottomLeft[iCnt] : rgnRect->iBottomLeft[iCnt];
	            break;

		    case EXTNODE:
				rgnRectTemp->iTopRight[iCnt] = (groupList->groupItems[tnInfo->tdInfo->groupElem]->bcell->maxOriginalBoundary[iCnt] > rgnRect->iTopRight[iCnt]) ? groupList->groupItems[tnInfo->tdInfo->groupElem]->bcell->maxOriginalBoundary[iCnt] : rgnRect->iTopRight[iCnt];
				rgnRectTemp->iBottomLeft[iCnt] = (groupList->groupItems[tnInfo->tdInfo->groupElem]->bcell->minOriginalBoundary[iCnt] < rgnRect->iBottomLeft[iCnt]) ? groupList->groupItems[tnInfo->tdInfo->groupElem]->bcell->minOriginalBoundary[iCnt] : rgnRect->iBottomLeft[iCnt];
				break;
		}
	}

	double dExp = Garea(rgnRectTemp) - Garea(rgnRect);

	if(*ptrDMinExp == -1 || dExp <= *ptrDMinExp)
    {   if(dExp == *ptrDMinExp)
			*ptrDMinExp = 0 - dExp;
		else
			*ptrDMinExp = dExp;
        for(iCnt =0; iCnt< DIMENSION; iCnt++)
        {	rgnNewRect->iBottomLeft[iCnt] = rgnRectTemp->iBottomLeft[iCnt];
			rgnNewRect->iTopRight[iCnt] = rgnRectTemp->iTopRight[iCnt];
		}
		free(rgnRectTemp->iBottomLeft);
		free(rgnRectTemp->iTopRight);
		free(rgnRectTemp);

		return TRUE;
	}

	free(rgnRectTemp->iBottomLeft);
	free(rgnRectTemp->iTopRight);
	free(rgnRectTemp);

	return FALSE;
}

double Garea(Region rgnRect)
{   
    if(rgnRect == NULL)
		return 0;
    double dArea = 1;
	int iCnt = 0;
	for(iCnt = 0; iCnt < DIMENSION; iCnt++)
		dArea = dArea * (rgnRect->iTopRight[iCnt] - rgnRect->iBottomLeft[iCnt]);
	return dArea;
}

Boolean GinsertTree(GHdrNd hdrNdTree, GTreeNode tnInfo, int gMinEntries, int gMaxEntries)
{   
	int iCnt = 0;

	if(hdrNdTree->ptrFirstNd == NULL || hdrNdTree->ptrFirstNd->tnInfo->ndType == EXTNODE)
    {    
		GinsertLstElem(hdrNdTree, tnInfo);
		return(hdrNdTree->uiCnt > gMaxEntries) ? TRUE : FALSE;
    }
    
	if(hdrNdTree->ptrFirstNd->ptrChildLst->uiCnt == 0)
		GsetRect(hdrNdTree->ptrFirstNd, tnInfo);

	GLstNd lstNdTemp = GpickChild(hdrNdTree, tnInfo);

	if(GinsertTree(lstNdTemp->ptrChildLst, tnInfo, gMinEntries, gMaxEntries))
    {   
        GsplitNode(lstNdTemp,gMinEntries);
	    hdrNdTree->uiCnt++;
	    return (hdrNdTree->uiCnt > gMaxEntries) ? TRUE : FALSE;
	}
	return FALSE;
}

GHdrNd GcreateRoot(GHdrNd hdrNdTree)
{  
    GHdrNd hdrNdRoot = GinitHdrNd();
    double* iBottomLeft = (double*)calloc(DIMENSION, sizeof(double));
    // assert(iBottomLeft != NULL);

	double* iTopRight = (double*)calloc(DIMENSION,sizeof(double));
	// assert(iTopRight != NULL);

	GLstNd lstNdTemp = hdrNdTree->ptrFirstNd;
	int iCnt = 0;
	Boolean bIsFirst = TRUE;

	while(lstNdTemp != NULL)
    {	
    	for(iCnt = 0; iCnt < DIMENSION; iCnt++)
        {   
        	if(bIsFirst)
            {   
            	iBottomLeft[iCnt] = lstNdTemp->tnInfo->tdInfo->rgnRect->iBottomLeft[iCnt];
				iTopRight[iCnt] = lstNdTemp->tnInfo->tdInfo->rgnRect->iTopRight[iCnt];
         	}
			else
            {   
            	if(lstNdTemp->tnInfo->tdInfo->rgnRect->iBottomLeft[iCnt] < iBottomLeft[iCnt])
					iBottomLeft[iCnt] = lstNdTemp->tnInfo->tdInfo->rgnRect->iBottomLeft[iCnt];

				if(lstNdTemp->tnInfo->tdInfo->rgnRect->iTopRight[iCnt] > iTopRight[iCnt])
					iTopRight[iCnt] = lstNdTemp->tnInfo->tdInfo->rgnRect->iTopRight[iCnt];
    	    }
		}
		lstNdTemp = lstNdTemp->ptrNextNd;
		bIsFirst = FALSE;
	}

	hdrNdRoot->ptrFirstNd = GinitLstNd(GinitIntNd(iBottomLeft, iTopRight));
    hdrNdRoot->ptrFirstNd->ptrChildLst = hdrNdTree;
	hdrNdRoot->uiCnt = 1;

	return hdrNdRoot;
}

void GsplitNode(GLstNd ptrChild, int gminEntries)
{    
	if(ptrChild == NULL || GisLstEmpty(ptrChild->ptrChildLst))
		return;

	GLstNd lstNdOne = NULL;
	GLstNd lstNdTwo = NULL;
	GTreeNode tnInfoTemp = NULL;

	GLstNd lstNdTemp = NULL;

	double dExpOne = -1;
	double dExpTwo = -1;

	GpickSeeds(ptrChild->ptrChildLst, &lstNdOne, &lstNdTwo);

	if(lstNdOne == NULL || lstNdTwo == NULL)
		return;

	GLstNd ptrChildTemp = GinitLstNd(GinitIntNd(NULL, NULL));
	GLstNd ptrChildSib = GinitLstNd(GinitIntNd(NULL, NULL));

	ptrChildTemp->ptrChildLst = GinitHdrNd();
	ptrChildSib->ptrChildLst = GinitHdrNd();
	ptrChildSib->ptrNextNd = ptrChild->ptrNextNd;

	GinsertLstNd(ptrChildTemp->ptrChildLst, lstNdOne);
	GsetRect(ptrChildTemp, lstNdOne->tnInfo);

	GinsertLstNd(ptrChildSib->ptrChildLst, lstNdTwo);
	GsetRect(ptrChildSib, lstNdTwo->tnInfo);

	Region rgnNewRectOne = GinitRgnRect(NULL, NULL);
	Region rgnNewRectTwo = GinitRgnRect(NULL, NULL);

	Boolean bIsOne = FALSE;
	Boolean bIsNdOneInComp = FALSE;
	Boolean bIsNdTwoInComp = FALSE;

	int iCnt = 0;

	lstNdTemp = GdeleteLstFirst(ptrChild->ptrChildLst);

	while(lstNdTemp != NULL)
    {   
		if(ptrChildTemp->ptrChildLst->uiCnt + ptrChild->ptrChildLst->uiCnt == gminEntries - 1)
			bIsNdOneInComp = TRUE;

		if(ptrChildSib->ptrChildLst->uiCnt + ptrChild->ptrChildLst->uiCnt == gminEntries - 1)
			bIsNdTwoInComp = TRUE;

		if(!bIsNdOneInComp && !bIsNdTwoInComp)
        {   
        	dExpOne = -1;
		    dExpTwo = -1;

		    GexpansionArea(ptrChildTemp->tnInfo->tdInfo->rgnRect, lstNdTemp->tnInfo, &dExpOne, rgnNewRectOne);
		    GexpansionArea(ptrChildSib->tnInfo->tdInfo->rgnRect, lstNdTemp->tnInfo, &dExpTwo, rgnNewRectTwo);


		    if(dExpOne < dExpTwo)
			     bIsOne = TRUE;

	        if(dExpOne > dExpTwo)
		         bIsOne = FALSE;

	        if(dExpOne == dExpTwo)
            {    
		         double dAreaOne = Garea(ptrChildTemp->tnInfo->tdInfo->rgnRect);
			     double dAreaTwo = Garea(ptrChildSib->tnInfo->tdInfo->rgnRect);

			     if(dAreaOne < dAreaTwo)
                      bIsOne = TRUE;

                 if(dAreaOne > dAreaTwo)
                      bIsOne = FALSE;

                 if(dAreaOne == dAreaTwo)
                 {    
                      if(ptrChildTemp->ptrChildLst->uiCnt < ptrChildSib->ptrChildLst->uiCnt)
                           bIsOne = TRUE;
                      else
					       bIsOne = FALSE;
                 }
           }
		} 
		
		else
        {   
		    if(bIsNdOneInComp)
                  bIsOne = TRUE;

		    if(bIsNdTwoInComp)
			      bIsOne = FALSE;
		}

		if(bIsOne)
        {   
			GinsertLstNd(ptrChildTemp->ptrChildLst, lstNdTemp);
            if(bIsNdOneInComp)
            {   
            	dExpOne = -1;
				GexpansionArea(ptrChildTemp->tnInfo->tdInfo->rgnRect, lstNdTemp->tnInfo, &dExpOne, rgnNewRectOne);
			}
			for(iCnt = 0; iCnt < DIMENSION; iCnt++)
            {	
            	ptrChildTemp->tnInfo->tdInfo->rgnRect->iBottomLeft[iCnt] = rgnNewRectOne->iBottomLeft[iCnt];
				ptrChildTemp->tnInfo->tdInfo->rgnRect->iTopRight[iCnt] = rgnNewRectOne->iTopRight[iCnt];
			}
		}
		else
        {   
			GinsertLstNd(ptrChildSib->ptrChildLst, lstNdTemp);
            if(bIsNdTwoInComp)
            {   
            	dExpTwo = -1;
				GexpansionArea(ptrChildSib->tnInfo->tdInfo->rgnRect, lstNdTemp->tnInfo, &dExpTwo, rgnNewRectTwo);
			}

			for(iCnt = 0; iCnt < DIMENSION; iCnt++)
            {	
            	ptrChildSib->tnInfo->tdInfo->rgnRect->iBottomLeft[iCnt] = rgnNewRectTwo->iBottomLeft[iCnt];
				ptrChildSib->tnInfo->tdInfo->rgnRect->iTopRight[iCnt] = rgnNewRectTwo->iTopRight[iCnt];
			}
		}
		lstNdTemp = GdeleteLstFirst(ptrChild->ptrChildLst);
	}

	ptrChildTemp->ptrChildLst->ptrParentNd = ptrChildTemp;
	ptrChildSib->ptrChildLst->ptrParentNd = ptrChildSib;
	ptrChildTemp->ptrNextNd = ptrChildSib;
	ptrChild->ptrChildLst->ptrParentNd = NULL;
	free(ptrChild->ptrChildLst);

	free(ptrChild->tnInfo->tdInfo->rgnRect->iBottomLeft);
	free(ptrChild->tnInfo->tdInfo->rgnRect->iTopRight);
	free(ptrChild->tnInfo->tdInfo->rgnRect);

	free(ptrChild->tnInfo->tdInfo);
	free(ptrChild->tnInfo);

	ptrChild->tnInfo = ptrChildTemp->tnInfo;
	ptrChild->ptrChildLst = ptrChildTemp->ptrChildLst;
	ptrChild->ptrNextNd = ptrChildTemp->ptrNextNd;

	ptrChildTemp->ptrNextNd = NULL;
	ptrChildTemp->ptrChildLst = NULL;
	ptrChildTemp->tnInfo = NULL;
	free(ptrChildTemp);

	free(rgnNewRectOne->iBottomLeft);
	free(rgnNewRectOne->iTopRight);
	free(rgnNewRectOne);
	free(rgnNewRectTwo->iBottomLeft);
	free(rgnNewRectTwo->iTopRight);
	free(rgnNewRectTwo);

	return;
}

void GpickSeeds(GHdrNd ptrChildLst, GLstNd *lstNdChildOne, GLstNd *lstNdChildTwo)
{    
	if(ptrChildLst == NULL)
		return;
	GTreeNode* tnInfoMin = (GTreeNode *)calloc(DIMENSION, sizeof(GTreeNode));
	// assert(tnInfoMin != NULL);

	GTreeNode* tnInfoMax = (GTreeNode *)calloc(DIMENSION, sizeof(GTreeNode));
	// assert(tnInfoMax != NULL);

	GLstNd lstNdTemp = NULL;
    int iCnt = 0;
	Boolean bIsFirst = TRUE;

	Region rgnRectTemp;
	Region rgnRectOut;

	double dNormSep = 0;
	double dMaxNormSep = 0;
	dimension dimMaxNormSep = 0;
	GTreeNode temp;

	switch(ptrChildLst->ptrFirstNd->tnInfo->ndType)
    {	
    	case INTNODE:   
			lstNdTemp = ptrChildLst->ptrFirstNd;
	        rgnRectTemp = GinitRgnRect(NULL, NULL);

	        rgnRectOut = GinitRgnRect(NULL, NULL);
	        while(lstNdTemp != NULL)
	        {     
	        	for(iCnt = 0; iCnt < DIMENSION; iCnt++)
	            {     
	              	if(bIsFirst)
	                {     
						rgnRectTemp->iBottomLeft[iCnt] = lstNdTemp->tnInfo->tdInfo->rgnRect->iBottomLeft[iCnt];
						rgnRectTemp->iTopRight[iCnt] = lstNdTemp->tnInfo->tdInfo->rgnRect->iTopRight[iCnt];
						rgnRectOut->iBottomLeft[iCnt] = lstNdTemp->tnInfo->tdInfo->rgnRect->iBottomLeft[iCnt];
						rgnRectOut->iTopRight[iCnt] = lstNdTemp->tnInfo->tdInfo->rgnRect->iTopRight[iCnt];
						tnInfoMin[iCnt] = lstNdTemp->tnInfo;
						tnInfoMax[iCnt] = lstNdTemp->tnInfo;
						continue;
	                }
	                if(lstNdTemp->tnInfo->tdInfo->rgnRect->iBottomLeft[iCnt] > rgnRectTemp->iBottomLeft[iCnt])
	                {     
	                    rgnRectTemp->iBottomLeft[iCnt] = lstNdTemp->tnInfo->tdInfo->rgnRect->iBottomLeft[iCnt];
	                    tnInfoMin[iCnt] = lstNdTemp->tnInfo;
	                }
	                if(lstNdTemp->tnInfo->tdInfo->rgnRect->iTopRight[iCnt] < rgnRectTemp->iTopRight[iCnt])
	                {     
	                    rgnRectTemp->iTopRight[iCnt] = lstNdTemp->tnInfo->tdInfo->rgnRect->iTopRight[iCnt];
	                    tnInfoMax[iCnt] = lstNdTemp->tnInfo;
	                }

	                if(lstNdTemp->tnInfo->tdInfo->rgnRect->iBottomLeft[iCnt] < rgnRectOut->iBottomLeft[iCnt])
	                    rgnRectOut->iBottomLeft[iCnt] = lstNdTemp->tnInfo->tdInfo->rgnRect->iBottomLeft[iCnt];

	                if(lstNdTemp->tnInfo->tdInfo->rgnRect->iTopRight[iCnt] > rgnRectOut->iTopRight[iCnt])
                        rgnRectOut->iTopRight[iCnt] = lstNdTemp->tnInfo->tdInfo->rgnRect->iTopRight[iCnt];
	            }
               
                lstNdTemp = lstNdTemp->ptrNextNd;
                if(bIsFirst)
                	bIsFirst = FALSE;
	        }

	        dNormSep = 0;
	        dMaxNormSep = 0;
	        dimMaxNormSep = 0;

	        for(iCnt = 0; iCnt < DIMENSION; iCnt++)
	        {     
	            dNormSep = fabs(rgnRectTemp->iBottomLeft[iCnt] - rgnRectTemp->iTopRight[iCnt]) / fabs(rgnRectOut->iTopRight[iCnt] - rgnRectOut->iBottomLeft[iCnt]);
	            if(dNormSep > dMaxNormSep)
	            {   
	            	dMaxNormSep = dNormSep;
	                dimMaxNormSep = iCnt;
	            }
	        }
	         
			if(tnInfoMin[(int)dimMaxNormSep] == tnInfoMax[(int)dimMaxNormSep])
			{
				lstNdTemp = ptrChildLst->ptrFirstNd;
				temp=tnInfoMax[(int)dimMaxNormSep];
				if(temp != lstNdTemp->tnInfo)
				{
					rgnRectTemp->iTopRight[(int)dimMaxNormSep] = lstNdTemp->tnInfo->tdInfo->rgnRect->iTopRight[(int)dimMaxNormSep];
					tnInfoMax[(int)dimMaxNormSep] = lstNdTemp->tnInfo;
					lstNdTemp = lstNdTemp->ptrNextNd;
				}
				else
				{
					rgnRectTemp->iTopRight[(int)dimMaxNormSep] = lstNdTemp->ptrNextNd->tnInfo->tdInfo->rgnRect->iTopRight[(int)dimMaxNormSep];
					tnInfoMax[(int)dimMaxNormSep] = lstNdTemp->ptrNextNd->tnInfo;
					lstNdTemp = lstNdTemp->ptrNextNd->ptrNextNd;
				}
				while(lstNdTemp != NULL)
				{
					if(lstNdTemp->tnInfo->tdInfo->rgnRect->iTopRight[(int)dimMaxNormSep] < rgnRectTemp->iTopRight[(int)dimMaxNormSep] && temp != lstNdTemp->tnInfo)
					{
						rgnRectTemp->iTopRight[(int)dimMaxNormSep] = lstNdTemp->tnInfo->tdInfo->rgnRect->iTopRight[(int)dimMaxNormSep];
			            tnInfoMax[(int)dimMaxNormSep] = lstNdTemp->tnInfo;
					}
					lstNdTemp = lstNdTemp->ptrNextNd;
				} 
			}
	         
			// if(tnInfoMin[(int)dimMaxNormSep]==tnInfoMax[(int)dimMaxNormSep])
			// {
			// 	fprintf(stderr, "error in the code\n");
			// 	exit(-1);
			// }

			*lstNdChildOne = GdeleteLstElem(ptrChildLst, tnInfoMin[(int)dimMaxNormSep]);

			*lstNdChildTwo = GdeleteLstElem(ptrChildLst, tnInfoMax[(int)dimMaxNormSep]);

			free(rgnRectTemp->iBottomLeft);
			free(rgnRectTemp->iTopRight);
			free(rgnRectTemp);
			free(rgnRectOut->iBottomLeft);
			free(rgnRectOut->iTopRight);
			free(rgnRectOut);
	    break;


		case EXTNODE:        
			lstNdTemp = ptrChildLst->ptrFirstNd;
			rgnRectTemp = GinitRgnRect(NULL, NULL);
			rgnRectOut = GinitRgnRect(NULL, NULL);

			while(lstNdTemp != NULL)
			{     
				for(iCnt = 0; iCnt < DIMENSION; iCnt++)
			    {     
				  	if(bIsFirst)
				    {     
							rgnRectTemp->iBottomLeft[iCnt] = groupList->groupItems[lstNdTemp->tnInfo->tdInfo->groupElem]->bcell->minOriginalBoundary[iCnt];
							rgnRectTemp->iTopRight[iCnt] = groupList->groupItems[lstNdTemp->tnInfo->tdInfo->groupElem]->bcell->maxOriginalBoundary[iCnt];
							rgnRectOut->iBottomLeft[iCnt] = groupList->groupItems[lstNdTemp->tnInfo->tdInfo->groupElem]->bcell->minOriginalBoundary[iCnt];
							rgnRectOut->iTopRight[iCnt] = groupList->groupItems[lstNdTemp->tnInfo->tdInfo->groupElem]->bcell->maxOriginalBoundary[iCnt];
							tnInfoMin[iCnt] = lstNdTemp->tnInfo;
							tnInfoMax[iCnt] = lstNdTemp->tnInfo;
							continue;
				    }
			        if(groupList->groupItems[lstNdTemp->tnInfo->tdInfo->groupElem]->bcell->minOriginalBoundary[iCnt] > rgnRectTemp->iBottomLeft[iCnt])
			        {   
			            rgnRectTemp->iBottomLeft[iCnt] = groupList->groupItems[lstNdTemp->tnInfo->tdInfo->groupElem]->bcell->minOriginalBoundary[iCnt];
			            tnInfoMin[iCnt] = lstNdTemp->tnInfo;
			        }
			        if(groupList->groupItems[lstNdTemp->tnInfo->tdInfo->groupElem]->bcell->maxOriginalBoundary[iCnt] < rgnRectTemp->iTopRight[iCnt])
			        {     
			            rgnRectTemp->iTopRight[iCnt] = groupList->groupItems[lstNdTemp->tnInfo->tdInfo->groupElem]->bcell->maxOriginalBoundary[iCnt];
			            tnInfoMax[iCnt] = lstNdTemp->tnInfo;
			        }
			        if(groupList->groupItems[lstNdTemp->tnInfo->tdInfo->groupElem]->bcell->minOriginalBoundary[iCnt] < rgnRectOut->iBottomLeft[iCnt])
			            rgnRectOut->iBottomLeft[iCnt] = groupList->groupItems[lstNdTemp->tnInfo->tdInfo->groupElem]->bcell->minOriginalBoundary[iCnt];
			        if(groupList->groupItems[lstNdTemp->tnInfo->tdInfo->groupElem]->bcell->maxOriginalBoundary[iCnt] > rgnRectOut->iTopRight[iCnt])
			            rgnRectOut->iTopRight[iCnt] = groupList->groupItems[lstNdTemp->tnInfo->tdInfo->groupElem]->bcell->maxOriginalBoundary[iCnt];
				}

				lstNdTemp = lstNdTemp->ptrNextNd;
				if(bIsFirst)
			    	bIsFirst = FALSE;
			}	//while

			dNormSep = 0;
			dMaxNormSep = 0;
			dimMaxNormSep = 0;
			for(iCnt = 0; iCnt < DIMENSION; iCnt++)
			{     
			   dNormSep = fabs(rgnRectTemp->iBottomLeft[iCnt] - rgnRectTemp->iTopRight[iCnt]) / fabs(rgnRectOut->iTopRight[iCnt] - rgnRectOut->iBottomLeft[iCnt]);
			   if(dNormSep > dMaxNormSep)
			   {   
			   	   dMaxNormSep = dNormSep;
			       dimMaxNormSep = iCnt;
			   }
			}

			if(tnInfoMin[(int)dimMaxNormSep] == tnInfoMax[(int)dimMaxNormSep])
			{
				lstNdTemp = ptrChildLst->ptrFirstNd;
				temp=tnInfoMax[(int)dimMaxNormSep];
				if(temp != lstNdTemp->tnInfo)
				{
					rgnRectTemp->iTopRight[(int)dimMaxNormSep] = groupList->groupItems[lstNdTemp->tnInfo->tdInfo->groupElem]->bcell->maxOriginalBoundary[(int)dimMaxNormSep];
					 tnInfoMax[(int)dimMaxNormSep] = lstNdTemp->tnInfo;
					lstNdTemp = lstNdTemp->ptrNextNd;
				}
				else
				{
					rgnRectTemp->iTopRight[(int)dimMaxNormSep] = groupList->groupItems[lstNdTemp->ptrNextNd->tnInfo->tdInfo->groupElem]->bcell->maxOriginalBoundary[(int)dimMaxNormSep];
					tnInfoMax[(int)dimMaxNormSep] = lstNdTemp->ptrNextNd->tnInfo;
					lstNdTemp = lstNdTemp->ptrNextNd->ptrNextNd;
				}
				while(lstNdTemp != NULL)
				{
					if(groupList->groupItems[lstNdTemp->tnInfo->tdInfo->groupElem]->bcell->maxOriginalBoundary[(int)dimMaxNormSep] < rgnRectTemp->iTopRight[(int)dimMaxNormSep] && temp != lstNdTemp->tnInfo)
					{
						rgnRectTemp->iTopRight[(int)dimMaxNormSep] = groupList->groupItems[lstNdTemp->tnInfo->tdInfo->groupElem]->bcell->maxOriginalBoundary[(int)dimMaxNormSep];
			              tnInfoMax[(int)dimMaxNormSep] = lstNdTemp->tnInfo;
					}
						lstNdTemp = lstNdTemp->ptrNextNd;
				} 
			}

			*lstNdChildOne = GdeleteLstElem(ptrChildLst, tnInfoMin[(int)dimMaxNormSep]);
			*lstNdChildTwo = GdeleteLstElem(ptrChildLst, tnInfoMax[(int)dimMaxNormSep]);

			free(rgnRectTemp->iBottomLeft);
			free(rgnRectTemp->iTopRight);
			free(rgnRectTemp);
			free(rgnRectOut->iBottomLeft);
			free(rgnRectOut->iTopRight);
			free(rgnRectOut);
		break;
	}

	free(tnInfoMin);
	free(tnInfoMax);

	return;
}

double distance(DataPoint point1, DataPoint point2)
{
	double sum = 0;
	int i;

	for(i=0;i<DIMENSION;i++)
		sum += ( (point1[i]- point2[i])*(point1[i] - point2[i]) );
	
	return sqrt(sum);
}

int GisEpsDistant(Region rgnRect, Group groupElem, double eps)
{   
	int iCnt = 0;
	bool assignflag = false;

	int bIsContains = 0;
	double* point = (Dimension)calloc(DIMENSION, sizeof(double));
	// assert(point != NULL);
	for(iCnt = 0; iCnt < DIMENSION; iCnt++)
    {   
    	point[iCnt] = (rgnRect->iBottomLeft[iCnt] + rgnRect->iTopRight[iCnt])/(double)2;
	}
	// DataPoint d = (dataList->dataClstElem + groupElem->master_id)->iData;

	if(distance(point, groupElem->master) <= eps + DBL_EPSILON)bIsContains = 1;
	else if(distance(point, groupElem->master) < 2*eps + DBL_EPSILON)bIsContains = -1;

	free(point);
	return bIsContains;
}

Assign GcanAssign2(Data currDataPoint, GHdrNd hdrNdTree, Region x2rgnRect, double eps)
{
	Assign assignStruct;

	// if(GisLstEmpty(hdrNdTree) || rgnRect == NULL || rgnRect->iBottomLeft == NULL || rgnRect->iTopRight == NULL)
	if(GisLstEmpty(hdrNdTree))
	{
		assignStruct = (Assign)calloc(1, sizeof(struct assign));
		// assert(assignStruct != NULL);
		assignStruct->canBeAssigned = -5;
		assignStruct->groupid = -1;
		return assignStruct;
	}
 	
 	unsigned int uiRecCnt = 0;
	int iCnt = 0;
    double t;
    static int flag = 0;
	GLstNd lstNdTemp = hdrNdTree->ptrFirstNd;
	int aflag = -2;
	bool eps2 = 0;

	// Assign assignStruct;
	while(lstNdTemp != NULL)
    {	
    	int assignflag;
		switch(lstNdTemp->tnInfo->ndType)
        {   
        	case INTNODE:   

	            if(RisContains(lstNdTemp->tnInfo->tdInfo->rgnRect, currDataPoint->iData))
	            // if(GisOverLap(, x2rgnRect))
	            {
	            	assignStruct = GcanAssign2(currDataPoint, lstNdTemp->ptrChildLst, x2rgnRect, eps);
	            	if(assignStruct->canBeAssigned==1)
	            	{
            			return assignStruct;
	            	}
	            	else if(assignStruct->canBeAssigned==-1)
	            	{
	            		eps2 = -1;
	            	}
            		free(assignStruct);
	          	}              
            break;

            case EXTNODE:  
            	// fprintf(stderr, "%lf\n", eps); 
				aflag = GisEpsDistant(x2rgnRect, groupList->groupItems[lstNdTemp->tnInfo->tdInfo->groupElem], eps);
				assignStruct = (Assign)calloc(1, sizeof(struct assign));
				// assert(assignStruct != NULL);
				if(aflag == 1)
				{
					assignStruct->canBeAssigned = 1;
					assignStruct->groupid = lstNdTemp->tnInfo->tdInfo->groupElem;
					return assignStruct;
				}
				free(assignStruct);        				    				
            break;
		}

		lstNdTemp = lstNdTemp->ptrNextNd;
	}

	//When eps2=0 it means that there is no group in which this point can be put but you can create a group for this point.
	//When eps=-1 it means that there is no group in which this point can be put and this point will go into the unassigned list.

	assignStruct = (Assign)calloc(1, sizeof(struct assign));
	// assert(assignStruct != NULL);
	assignStruct->canBeAssigned = eps2;
	assignStruct->groupid = -1;

	return assignStruct;
}

void findNeighboursGroup(RHdrNd hdrNdTree, Region rgnRect, Data data, Neighbours n, double eps)
{
	if(RisLstEmpty(hdrNdTree) || rgnRect == NULL || rgnRect->iBottomLeft == NULL || rgnRect->iTopRight == NULL)
		return;

	unsigned int uiRecCnt = 0;
	int iCnt = 0;
	double t;
	static int flag = 0;
	RLstNd lstNdTemp = hdrNdTree->ptrFirstNd;
	int nop;
	double two = 2.0;

	while(lstNdTemp != NULL)
	{	
		switch(lstNdTemp->tnInfo->ndType)
		{   
			case INTNODE:
										 ;
			    findNeighboursGroup(lstNdTemp->ptrChildLst, rgnRect, data, n, eps);
			    break;
			            
			case EXTNODE:   //incase of external node if the child node is contanied in serach rectangle increment the count and return the count to the parent
				nop = 10;
				if(RisContains(rgnRect, lstNdTemp->tnInfo->tdInfo->dataClstElem->iData))
			    {      
	                if(lstNdTemp->tnInfo->tdInfo->dataClstElem->iData != data->iData || lstNdTemp->tnInfo->tdInfo->dataClstElem->id!=data->id)
	                {     
	                	t = distance(lstNdTemp->tnInfo->tdInfo->dataClstElem->iData, data->iData);
					    if(t <= eps)
		                {
		                	Data neighbour = lstNdTemp->tnInfo->tdInfo->dataClstElem;
		                	addHelper[0] = lstNdTemp->tnInfo->tdInfo->dataClstElem->id;
		                	addHelperDouble[0] = t;
		                	if(neighbour->haloPoint == TRUE)
		                	{
		                		
		                		// VECTOR_ADD(n->neighbours_remote_distance, addHelperDouble);
		                		if(t <= (eps/two))
		                		{
		                			VECTOR_ADD(n->neighbours_remote_inner, addHelper);
		                			// VECTOR_ADD(n->neighbours_remote_distance, addHelperDouble);
		                		}
		                		else
		                		{
		                			VECTOR_ADD(n->neighbours_remote_outer, addHelper);
		                		}
		                	}
		                	else
		                	{
		                		if(t <= (eps/two))
		                		{
		                			VECTOR_ADD(n->neighbours_local_inner, addHelper);
		                			// VECTOR_ADD(n->neighbours_remote_distance, addHelperDouble);
		                		}
		                		else
		                		{
		                			VECTOR_ADD(n->neighbours_local_outer, addHelper);
		                		}
		                	}
		                }
		            }
                }
                break;
		}
				lstNdTemp = lstNdTemp->ptrNextNd;
	}  
}


Neighbours findNeighbours3(DataHdr dataList, int pointid, double eps, vector < vector <int > >* p_cur_insert)
{

	findNeighbourCount++;
	Data point = dataList->dataClstElem + pointid;

	// if(point->haloPoint == TRUE)
	// {
	// 	fprintf(stderr, "Unexpected condition! FindNeighbours called on Remote Point!");
	// 	exit(-1);
	// }

	Group group = groupList->groupItems[point->group_id];

	Neighbours n = (Neighbours)calloc(1, sizeof(struct neighbours));
	// assert(n != NULL);
	
	n->id = pointid;

	n->neighbours_local_outer = (vectorc*)malloc(sizeof(struct vectorc));
	// assert(n->neighbours_local != NULL);
	VECTOR_INIT(n->neighbours_local_outer, INTEGER);

	n->neighbours_remote_outer = (vectorc*)malloc(sizeof(struct vectorc));
	// assert(n->neighbours_remote != NULL);
	VECTOR_INIT(n->neighbours_remote_outer, INTEGER);

	n->neighbours_local_inner=(vectorc*)malloc(sizeof(struct vectorc));
	// assert(n->neighbours_local_distance != NULL);
	VECTOR_INIT(n->neighbours_local_inner, INTEGER);

	n->neighbours_remote_inner = (vectorc*)malloc(sizeof(struct vectorc));
	// assert(n->neighbours_remote_distance != NULL);
	VECTOR_INIT(n->neighbours_remote_inner, INTEGER);

	Region region = createCellRegOfPoint(point, eps);
	Region potentialRegion;

	int inner_neighbours = 0;
	int total_neighbours = 0;
	double two = 2;
	Data temp;

	int i=0, j=0;

	//FIRST SEARCH FOR NEIGHBOURS IN THE PRESENT GROUP.
	RHdrNd hdrNdTree = group->bcell->auxCellRTree;
	findNeighboursGroup(hdrNdTree, region, point, n, eps);
	int p;
	i=0;

	// if(VECTOR_TOTAL(n->neighbours_local) != VECTOR_TOTAL(n->neighbours_local_distance) || VECTOR_TOTAL(n->neighbours_remote) != VECTOR_TOTAL(n->neighbours_remote_distance))
	// {
	// 	fprintf(stderr, "findNeighbours3 doesn't not conform to the standard!\n");
	// 	exit(-1);
	// }

	//THEN SEARCH FOR NEIGHBOURS IN THE FILTERED REACHABLE GROUPS
	Group reachableGroup,filteredGroup;
	DataPoint d;
	for(i=0; i<VECTOR_TOTAL(group->reachable_groups); i++)
	{
		int id = group->reachable_groups->intItems[i];
		Group reachableGroup = groupList->groupItems[id];
		DataPoint d = (dataList->dataClstElem + reachableGroup->master_id)->iData;
		potentialRegion = createCellRegOfDataPoint(d, eps);

		if(GisOverLap(region, potentialRegion))
		{
			filteredGroup = groupList->groupItems[id];
			j=0;
			hdrNdTree = filteredGroup->bcell->auxCellRTree;
			findNeighboursGroup(hdrNdTree, region, point, n, eps);
		}

		free(potentialRegion->iBottomLeft);
		free(potentialRegion->iTopRight);
		free(potentialRegion);
	}

	inner_neighbours = VECTOR_TOTAL(n->neighbours_remote_inner) + VECTOR_TOTAL(n->neighbours_local_inner);
	total_neighbours = VECTOR_TOTAL(n->neighbours_remote_outer) + VECTOR_TOTAL(n->neighbours_local_outer) + inner_neighbours;

	if(inner_neighbours >= MINPOINTS)
	{
		if(point->haloPoint == FALSE && point->core_tag == FALSE) totalCore++;

		point->core_tag = TRUE;
		point->ClusterID = 1;
		point->isProcessed = TRUE;

		Group g = groupList->groupItems[point->group_id];

		if(visited[point->id] != 1)
        {
            visited[point->id] = 1;
            addHelper[0] = point->id;
            VECTOR_ADD(g->corepoints, addHelper);
            VECTOR_ADD(unprocessedCore, addHelper);
        }

        int root = point->id;
        Data data;
        Data dataroot1;
        Data dataroot2;
        for(i=0;i<VECTOR_TOTAL(n->neighbours_local_inner);i++)
        {
			data = dataList->dataClstElem + n->neighbours_local_inner->intItems[i];
			if(data->haloPoint == FALSE && data->core_tag == FALSE) totalCore++;
			
			data->core_tag = TRUE;
			data->isProcessed = TRUE;
			data->ClusterID = 1;
			
			g = groupList->groupItems[data->group_id];

			if(visited[data->id] != 1)
        	{
                visited[data->id] = 1;
                addHelper[0] = data->id;
                VECTOR_ADD(g->corepoints, addHelper);
              	VECTOR_ADD(unprocessedCore, addHelper);
        	}

        	int root1 = data->id;
			int root2 = root;

			dataroot1 = dataList->dataClstElem + root1;
            dataroot2 = dataList->dataClstElem + root2;

			unionFindMerge(dataList, root1, root2, dataroot1, dataroot2);
        }

        for(i=0;i<VECTOR_TOTAL(n->neighbours_local_outer);i++)
        {
        	data = dataList->dataClstElem + n->neighbours_local_outer->intItems[i];

			if(data->ClusterID == 0 || data->core_tag == TRUE)
			{
				int root1 = data->id;
				int root2 = root;
				data->ClusterID = 1;
				dataroot1 = dataList->dataClstElem + root1;
                dataroot2 = dataList->dataClstElem + root2;

                unionFindMerge(dataList, root1, root2, dataroot1, dataroot2);
                // REMS algorithm to (union) merge the trees
                // UNION OPERATION
			}
        }

        Data dataremote;
        for(i=0;i<VECTOR_TOTAL(n->neighbours_remote_inner);i++)
        {
        	data = dataList->dataClstElem + n->neighbours_remote_inner->intItems[i];

			if(visited[data->id] != 1)
        	{
                visited[data->id] = 1;
                addHelper[0] = data->id;
                VECTOR_ADD(g->corepoints, addHelper);
              	VECTOR_ADD(unprocessedCore, addHelper);
        	}

			data->core_tag = TRUE;
			data->isProcessed = TRUE;
			data->ClusterID = 1;

			Data dataremote = data;
            (*p_cur_insert)[dataremote->actualProcessId].push_back(point->id);
            (*p_cur_insert)[dataremote->actualProcessId].push_back(dataremote->remoteIndex);
        }


        for(i=0;i<VECTOR_TOTAL(n->neighbours_remote_outer);i++)
        {
        	Data data = dataList->dataClstElem + n->neighbours_remote_outer->intItems[i];

			data->ClusterID = 1;
			dataremote = data;
            (*p_cur_insert)[dataremote->actualProcessId].push_back(point->id);
            (*p_cur_insert)[dataremote->actualProcessId].push_back(dataremote->remoteIndex);
		}
	}

	if(total_neighbours >= MINPOINTS)
	{
		Group g = groupList->groupItems[point->group_id];

		if(point->haloPoint == FALSE && point->core_tag == FALSE) totalCore++;
		
		point->core_tag = TRUE;
		point->ClusterID = 1;

		if(visited[point->id] != 1)
        {
	        visited[point->id] = 1;
	        addHelper[0] = point->id;
	        VECTOR_ADD(g->corepoints, addHelper);
            VECTOR_ADD(unprocessedCore, addHelper);
	    }
	}

	free(region->iBottomLeft);
	free(region->iTopRight);
	free(region);

	return n;
}


void findReachableGroupsofGroupG(DataHdr dataList, GHdrNd GRTree, int i)
{
	if(GisLstEmpty(GRTree))
	{
		return ;
	}
	
	Group G = groupList->groupItems[i];
 	unsigned int uiRecCnt = 0;
	int iCnt = 0;
    double t;
    static int flag = 0;

	GLstNd lstNdTemp = GRTree->ptrFirstNd;
	Group currGroup = NULL;
	
	DataPoint d = (dataList->dataClstElem + G->master_id)->iData;
	Region x3rgnRect = createCellRegOfDataPoint(d, 3*EPS);

	while(lstNdTemp != NULL)
    {
		switch(lstNdTemp->tnInfo->ndType)
        {
           case INTNODE:   
				if(GisOverLap(lstNdTemp->tnInfo->tdInfo->rgnRect, x3rgnRect))
	            {
	            	//if((dataList->dataClstElem + G->master_id)->debugPoint==TRUE){
						//printf("%lf\t%lf\t%lf\tRank Is %d\t Overlaps Group ID%d\n",d[0],d[1],d[2],rank,(dataList->dataClstElem + G->master_id)->group_id);
					//}
	            	findReachableGroupsofGroupG(dataList, lstNdTemp->ptrChildLst, i);
	            }
            break;
            case EXTNODE:   
				if(G->id != lstNdTemp->tnInfo->tdInfo->groupElem && distance(d, (dataList->dataClstElem + (groupList->groupItems[lstNdTemp->tnInfo->tdInfo->groupElem])->master_id)->iData) <= 3*EPS)
				{
						addHelper[0] = lstNdTemp->tnInfo->tdInfo->groupElem;
						VECTOR_ADD(G->reachable_groups, addHelper);
				}      			            				
            break;
		}
		lstNdTemp = lstNdTemp->ptrNextNd;
	}

	free(x3rgnRect->iBottomLeft);
	free(x3rgnRect->iTopRight);
	free(x3rgnRect);
}

void insertPointintoGroup(Group currGroup, Data currDataPoint)
{
	// Data master = data
	DataPoint d = currGroup->master;

	double dist = distance(currDataPoint->iData, d);
	
	currGroup->total_count++;
	addHelper[0] = currDataPoint->id;

	VECTOR_ADD(currGroup->total_points, addHelper);
	double two = 2;

	if(dist <= EPS/two)
	{
		currGroup->inner_count++;
		addHelper[0] = currDataPoint->id;
		VECTOR_ADD(currGroup->inner_points, addHelper);
	}

	if(currGroup->threshold < dist) currGroup->threshold = dist;

	currDataPoint->group_id = currGroup->id;

	BCell currBCell = currGroup->bcell;
	int x,i;

	if(VECTOR_TOTAL(currGroup->total_points) == 1)
	{
		for(x = 0; x < DIMENSION; x++)
		{
			currBCell->minActualBoundary[x] = currDataPoint->iData[x];
			currBCell->maxActualBoundary[x] = currDataPoint->iData[x];
		}		
	}
	else
	{
		for(i = 0; i< DIMENSION; i++)
		{
			if(currBCell->minActualBoundary[i] > currDataPoint->iData[i])
			{
				currBCell->minActualBoundary[i] = currDataPoint->iData[i];
			}

			if(currBCell->maxActualBoundary[i] < currDataPoint->iData[i])
			{
				currBCell->maxActualBoundary[i] = currDataPoint->iData[i];
			}
		}
	}

	return;
}

GHdrNd populateMuCRTree(DataHdr dataHdrLst, int gMinEntries, int gMaxEntries)
{
	GHdrNd hdrNdTree = GinitHdrNd();
	hdrNdTree->ptrFirstNd = GinitLstNd(GinitIntNd(NULL, NULL));
	hdrNdTree->uiCnt++;
	hdrNdTree->ptrFirstNd->ptrChildLst = GinitHdrNd();

	int cnt = 0, i;
	vectorc* unassigned = (vectorc*)malloc(sizeof(vectorc));
	VECTOR_INIT(unassigned, INTEGER);
	// int alloc = 0;

	Data currDataPoint;
	Group newGroup;
	for(i=0;i<dataHdrLst->uiCnt;i++)
	{	
		currDataPoint = dataHdrLst->dataClstElem + i; 
		Region currRegion = createCellRegOfPoint(currDataPoint, EPS);
		Region x2currRegion = createCellRegOfPoint(currDataPoint, 2*EPS);

		

		struct assign* assignStruct = GcanAssign2(currDataPoint, hdrNdTree, x2currRegion, EPS);

		if(assignStruct->canBeAssigned == 1) //Implies the current point is in the eps neighborhood of an existing group.
		{
			Group currGroup = groupList->groupItems[assignStruct->groupid];

			insertPointintoGroup(currGroup, currDataPoint);
			currDataPoint->group_id = currGroup->id;

			// if(currGroup != NULL)
			// {
			// 	insertPointintoGroup(currGroup, currDataPoint);
			// 	currDataPoint->group_id = currGroup->id;
			// 	// if(currDataPoint->debugPoint == TRUE)
			// 	// {
			// 	// 	printf("%lf\t%lf\t%lf\t populateMuCRTree Method Group ID %d\n",currDataPoint->iData[0],currDataPoint->iData[1],currDataPoint->iData[2],currDataPoint->group_id);
			// 	// }
			// }
			// else
			// {
			// 	printf("\n\n\nGcanAssign is giving WRONG OUTPUT!!!\n\n\n");
			// 	exit(0);
			// }
		}
		else if(assignStruct->canBeAssigned==0) //Implies the current point is not in the eps nbhd as well as 2*eps nbhd of existing groups.
		{

			newGroup = initGroup(currDataPoint);
			newGroup->bcell = initBCell(currRegion);
			BCell newBCell = newGroup->bcell;
			insertPointintoGroup(newGroup, currDataPoint);
			
			currDataPoint->group_id = newGroup->id;
			insertGroupIntoGroupList(newGroup);
			hdrNdTree = insertGroupIntoRTree(hdrNdTree, newGroup, gMinEntries, gMaxEntries);
			// if(currDataPoint->debugPoint == TRUE)
			// {
			// 	printf("%lf\t%lf\t%lf\t populateMuCRTree Method Can Not Be assigned To any existing group%d\n",currDataPoint->iData[0],currDataPoint->iData[1],currDataPoint->iData[2],currDataPoint->group_id);
			// }

		}			
		else
		{
			addHelper[0] = i;
			VECTOR_ADD(unassigned, addHelper);
		}

		free(currRegion->iBottomLeft);
   		free(currRegion->iTopRight);
    	free(currRegion);
    	free(x2currRegion->iBottomLeft);
   		free(x2currRegion->iTopRight);
    	free(x2currRegion);
	}
	
	// FOR UNASSIGNED POINTS, DON'T CONSTRUCT AUXILLARY R TREE....
	// CONSTRUCT AUX R TREES ALONG WITH INSERTION
	Region currRegion = NULL;
	Region x2currRegion = NULL;
	//Data currDataPoint;
	//Group newGroup;
	for(i=0; i<VECTOR_TOTAL(unassigned); i++)
	{
		
			currDataPoint = dataHdrLst->dataClstElem + unassigned->intItems[i];

			currRegion = createCellRegOfPoint(currDataPoint, EPS);
			x2currRegion = createCellRegOfPoint(currDataPoint, 2*EPS);

			struct assign* assignStruct = GcanAssign2(currDataPoint, hdrNdTree, x2currRegion, EPS);
			
			if(assignStruct->canBeAssigned==1)
			{
				Group currGroup = groupList->groupItems[assignStruct->groupid];

				insertPointintoGroup(currGroup, currDataPoint);
				currDataPoint->group_id=currGroup->id;

				//Modify the implementation of GfindCell to return BCell only when distance between data point and center is less than or equal to Epsilon
				// if(currGroup != NULL){
				// 	insertPointintoGroup(currGroup, currDataPoint);
				// 	currDataPoint->group_id=currGroup->id;			
				// }
				// else{
				// 	printf("\n\n\n GcanAssign is giving WRONG OUTPUT\n\n\n");
				// }
			}
			else
			{
				// createa a new Group and insert it into the R Tree
				// create a BCell and insert it into the R Tree
				// printf("Creating a new BCell and inserting the new point into it\n");
					
				newGroup = initGroup(currDataPoint);//(Group)malloc(sizeof(struct group));
				newGroup->bcell = initBCell(currRegion);
				BCell newBCell = newGroup->bcell;

				insertPointintoGroup(newGroup, currDataPoint);
					
				currDataPoint->group_id = newGroup->id;
				insertGroupIntoGroupList(newGroup);

				hdrNdTree = insertGroupIntoRTree(hdrNdTree, newGroup, gMinEntries, gMaxEntries);
			}			
		
		if(currRegion != NULL)
		{
			free(currRegion->iBottomLeft);
	   		free(currRegion->iTopRight);
	    	free(currRegion);
	    }

	    if(x2currRegion != NULL)
	    {
	    	free(x2currRegion->iBottomLeft);
	   		free(x2currRegion->iTopRight);
	    	free(x2currRegion);	
    	}
	}
	VECTOR_FREE(unassigned);
	free(unassigned);
	return hdrNdTree;
}

void populateAuxRTrees(DataHdr dataList, struct vectorc* groupList)
{
	int cnt = 0,i = 0;
	int j=0;
	RHdrNd hdrNdTree = NULL;
	Group g;
	Data data;
	for(i=0; i < VECTOR_TOTAL(groupList); i++)
	{
		g = groupList->groupItems[i];
		
		hdrNdTree = RinitHdrNd();
		
		hdrNdTree->ptrFirstNd = RinitLstNd(RinitIntNd(NULL, NULL));
		hdrNdTree->uiCnt++;
		hdrNdTree->ptrFirstNd->ptrChildLst = RinitHdrNd();		

		for(j=0;j<VECTOR_TOTAL(g->total_points);j++)
		{
			data = dataList->dataClstElem + g->total_points->intItems[j];

			// if(data->debugPoint == TRUE){
			// 	printf("%lf\t%lf\t%lf\t populateAuxRTrees Method is getting inserted in rtree groupId%d\n",data->iData[0],data->iData[1],data->iData[2],g->total_points->intItems[j]);
			// }

			RinsertTree(hdrNdTree, RinitExtNd(data));
					
			if(hdrNdTree->uiCnt > 1)
				hdrNdTree = RcreateRoot(hdrNdTree);

		}
		// if(hdrNdTree == NULL)fprintf(stderr, "Errorrrrrr!\n");
		g->bcell->auxCellRTree = hdrNdTree;
		// isCorrectRTree(g->bcell->auxCellRTree);

	}

	return;
}

void insertGroupIntoGroupList(Group newGroup)
{	
	VECTOR_ADD(groupList, newGroup);
	return;
}


Region createCellRegOfDataPoint(DataPoint currDataPoint, double eps)
{
	Region tempRegion = (Region) calloc(1, sizeof(struct region));
	// assert(tempRegion!=NULL);

	tempRegion->iBottomLeft = (Dimension)calloc(DIMENSION, sizeof(double));
	// assert(tempRegion->iBottomLeft!=NULL);

	tempRegion->iTopRight = (Dimension)calloc(DIMENSION, sizeof(double));
	// assert(tempRegion->iTopRight!=NULL);

	int i = 0;

	for(i=0;i<DIMENSION;i++)
	{
		tempRegion->iBottomLeft[i] = currDataPoint[i] - eps;		
		tempRegion->iTopRight[i] = currDataPoint[i] + eps; 		
	}

	return tempRegion;
}

Region createCellRegOfPoint(Data currDataPoint, double eps)
{
	Region tempRegion = (Region) calloc(1, sizeof(struct region));
	// assert(tempRegion!=NULL);

	tempRegion->iBottomLeft = (Dimension)calloc(DIMENSION, sizeof(double));
	// assert(tempRegion->iBottomLeft!=NULL);

	tempRegion->iTopRight = (Dimension)calloc(DIMENSION, sizeof(double));
	// assert(tempRegion->iTopRight!=NULL);

	int i = 0;
	for(i=0;i<DIMENSION;i++)
	{
		tempRegion->iBottomLeft[i] = currDataPoint->iData[i] - eps;		
		tempRegion->iTopRight[i] = currDataPoint->iData[i] + eps; 		
	}

	return tempRegion;
}


GHdrNd insertGroupIntoRTree(GHdrNd hdrNdTree, Group groupNode, int gMinEntries, int gMaxEntries)
{
	GinsertTree(hdrNdTree, GinitExtNd(groupNode), gMinEntries, gMaxEntries);
		
	if(hdrNdTree->uiCnt > 1)
		hdrNdTree = GcreateRoot(hdrNdTree);

	return hdrNdTree;
}

void freeGRTree(GHdrNd hdrNdTree)
{

	if(hdrNdTree == NULL) 
		return;

	else if(hdrNdTree->uiCnt == 0)
	{
		free(hdrNdTree);
		return;
	}
	
	GLstNd lstNdTemp = hdrNdTree->ptrFirstNd;
	GLstNd lstNdNextTemp;
	
	if(lstNdTemp!=NULL)
	{
		while(lstNdTemp != NULL)
    	{
			switch(lstNdTemp->tnInfo->ndType)
        	{   
        		case INTNODE:   
					lstNdNextTemp = lstNdTemp->ptrNextNd;
	    			free(lstNdTemp->tnInfo->tdInfo->rgnRect->iBottomLeft);
	    			free(lstNdTemp->tnInfo->tdInfo->rgnRect->iTopRight);
	    			free(lstNdTemp->tnInfo->tdInfo->rgnRect);
	    			free(lstNdTemp->tnInfo->tdInfo);
	    			free(lstNdTemp->tnInfo);
	    			freeGRTree(lstNdTemp->ptrChildLst);                            	
	            	free(lstNdTemp);
            	break;
            	case EXTNODE:   	
		            lstNdNextTemp = lstNdTemp->ptrNextNd;
		            free(lstNdTemp->tnInfo->tdInfo);

		            free(lstNdTemp->tnInfo);
		            freeGRTree(lstNdTemp->ptrChildLst);
                	free(lstNdTemp);
                break;

			}
			
			lstNdTemp = lstNdNextTemp;
		}
	}	
	
	free(hdrNdTree);	
	return;
}

void isCorrectGRTree(GHdrNd hdrNdTree)
{
    // fprintf(stderr, "Checking correctness\n");
    GLstNd lstNdTemp = hdrNdTree->ptrFirstNd,temp;
    int iCnt = 0,flag;

    while(lstNdTemp != NULL)
    {   
        if(lstNdTemp->tnInfo->ndType == INTNODE)
         {   
            temp=lstNdTemp->ptrChildLst->ptrFirstNd;
            while(temp!=NULL)
            {
            	switch(temp->tnInfo->ndType)
            	{
            		case INTNODE:
            			flag=0;
		                for(iCnt=0;iCnt<DIMENSION;iCnt++)
		                {
		                    if(temp->tnInfo->tdInfo->rgnRect->iBottomLeft[iCnt] < lstNdTemp->tnInfo->tdInfo->rgnRect->iBottomLeft[iCnt])
		                        flag=1;
		                }
		                for(iCnt=0;iCnt<DIMENSION;iCnt++)
		                {
		                    if(temp->tnInfo->tdInfo->rgnRect->iTopRight[iCnt]>lstNdTemp->tnInfo->tdInfo->rgnRect->iTopRight[iCnt])
		                        flag=1;
		                }
		                // if(flag==1)
		                // {
		                //     fprintf(stderr, "WRONG!!!\n");
		                //     for(iCnt=0;iCnt<DIMENSION;iCnt++)
		                //     {
		                //         fprintf(stderr, "%lf %lf %lf %lf\n",temp->tnInfo->tdInfo->rgnRect->iBottomLeft[iCnt],temp->tnInfo->tdInfo->rgnRect->iTopRight[iCnt],lstNdTemp->tnInfo->tdInfo->rgnRect->iBottomLeft[iCnt],lstNdTemp->tnInfo->tdInfo->rgnRect->iTopRight[iCnt]);
		                //     	exit(-1);
		                //     }
		                // }
                		temp=temp->ptrNextNd;
            		break;

            		case EXTNODE:
            			flag=0;
		                for(iCnt=0;iCnt<DIMENSION;iCnt++)
		                {
		                    if(groupList->groupItems[temp->tnInfo->tdInfo->groupElem]->bcell->minOriginalBoundary[iCnt] < lstNdTemp->tnInfo->tdInfo->rgnRect->iBottomLeft[iCnt])
		                        flag=1;
		                }
		                for(iCnt=0;iCnt<DIMENSION;iCnt++)
		                {
		                    if(groupList->groupItems[temp->tnInfo->tdInfo->groupElem]->bcell->maxOriginalBoundary[iCnt]>lstNdTemp->tnInfo->tdInfo->rgnRect->iTopRight[iCnt])
		                        flag=1;
		                }
		                // if(flag==1)
		                // {
		                //     fprintf(stderr, "WRONG!!!\n");
		                //     for(iCnt=0;iCnt<DIMENSION;iCnt++)
		                //     {
		                //         fprintf(stderr, "%lf %lf %lf %lf\n",groupList->groupItems[temp->tnInfo->tdInfo->groupElem]->bcell->minOriginalBoundary[iCnt],groupList->groupItems[temp->tnInfo->tdInfo->groupElem]->bcell->maxOriginalBoundary[iCnt], lstNdTemp->tnInfo->tdInfo->rgnRect->iBottomLeft[iCnt],lstNdTemp->tnInfo->tdInfo->rgnRect->iTopRight[iCnt]);
		                //     	exit(-1);
		                //     }
		                // }
                		temp=temp->ptrNextNd;
            		break;
            	}
                
            }
            isCorrectGRTree(lstNdTemp->ptrChildLst);
        }
        lstNdTemp = lstNdTemp->ptrNextNd;
    }
    return;
}