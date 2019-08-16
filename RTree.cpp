/*

Algorithm: Micro-cluster based DBSCAN
Author: Aditya Sarma
email: asaditya1195@gmail.com

*/

#include "RTree.h"

Region RinitRgnRect(Dimension iBottomLeft, Dimension iTopRight)
{   
	Region rgnRect = (Region)calloc(1, sizeof(struct region));
	// assert(rgnRect != NULL);
    if(rgnRect == NULL)
		return NULL;
		
    if(iBottomLeft != NULL)
		rgnRect->iBottomLeft = iBottomLeft;
	else
		rgnRect->iBottomLeft = (double*)calloc(DIMENSION, sizeof(double));
		// assert(rgnRect->iBottomLeft != NULL);

	if(rgnRect->iBottomLeft == NULL)
    {   
    	free(rgnRect);
		return NULL;
    }

	if(iTopRight != NULL)
		rgnRect->iTopRight = iTopRight;
	else
		rgnRect->iTopRight = (double*)calloc(DIMENSION, sizeof(double));
		// assert(rgnRect->iTopRight != NULL);

	if(rgnRect->iTopRight == NULL)
    {   
    	if(rgnRect->iBottomLeft == NULL);
		free(rgnRect);
		return NULL;
	}

	return rgnRect;
}

void RsetRect(RLstNd lstNd, RTreeNode tnInfo)
{
    int iCnt = 0;
    switch(tnInfo->ndType)
    {   case INTNODE:
		for(iCnt = 0; iCnt < DIMENSION; iCnt++)
        {   lstNd->tnInfo->tdInfo->rgnRect->iBottomLeft[iCnt] = tnInfo->tdInfo->rgnRect->iBottomLeft[iCnt];
			lstNd->tnInfo->tdInfo->rgnRect->iTopRight[iCnt] = tnInfo->tdInfo->rgnRect->iTopRight[iCnt];
     	}
		break;
        case EXTNODE:
		for(iCnt = 0; iCnt < DIMENSION; iCnt++)
        {   lstNd->tnInfo->tdInfo->rgnRect->iBottomLeft[iCnt] = tnInfo->tdInfo->dataClstElem->iData[iCnt];
			lstNd->tnInfo->tdInfo->rgnRect->iTopRight[iCnt] = tnInfo->tdInfo->dataClstElem->iData[iCnt];
		}
		break;
	}

	return;
}

RTreeNode RinitIntNd(Dimension iBottomLeft, Dimension iTopRight)
{
	Region rgnRect = RinitRgnRect(iBottomLeft, iTopRight);
	// assert(rgnRect != NULL);

	RTreeData tdInfo = (RTreeData)calloc(1, sizeof(*tdInfo));
	// assert(tdInfo != NULL);

	if(tdInfo == NULL)
		return NULL;

	tdInfo->rgnRect = rgnRect;
	tdInfo->dataClstElem = NULL;
    RTreeNode tnInfo = (RTreeNode)calloc(1, sizeof(*tnInfo));
    // assert(tnInfo != NULL);

	if(tnInfo == NULL)
		return NULL;

	tnInfo->ndType = INTNODE;
	tnInfo->tdInfo = tdInfo;

	return tnInfo;
}

RTreeNode RinitExtNd(Data dataClstElem)
{  
   if(dataClstElem == NULL)
		return NULL;

	RTreeNode tnInfo = (RTreeNode)calloc(1, sizeof(*tnInfo));
	// assert(tnInfo!= NULL);

	RTreeData tdInfo = (RTreeData)calloc(1, sizeof(*tdInfo));
	// assert(tdInfo!=NULL);

	tdInfo->dataClstElem = dataClstElem;
	tdInfo->rgnRect = NULL;

	tnInfo->ndType = EXTNODE;
	tnInfo->tdInfo = tdInfo;

	return tnInfo;
}

RHdrNd RcreateRoot(RHdrNd hdrNdTree)
{
    RHdrNd hdrNdRoot = RinitHdrNd();
    Dimension iBottomLeft = (Dimension)calloc(DIMENSION, sizeof(double));
    // assert(iBottomLeft != NULL);

	Dimension iTopRight = (Dimension)calloc(DIMENSION,sizeof(double));
	// assert(iTopRight != NULL);

	RLstNd lstNdTemp = hdrNdTree->ptrFirstNd;
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

	hdrNdRoot->ptrFirstNd = RinitLstNd(RinitIntNd(iBottomLeft, iTopRight));
    hdrNdRoot->ptrFirstNd->ptrChildLst = hdrNdTree;
	hdrNdRoot->uiCnt = 1;

	return hdrNdRoot;
}

Boolean RexpansionArea(Region rgnRect, RTreeNode tnInfo, Double ptrDMinExp, Region rgnNewRect)
{
    int iCnt = 0;
    Region rgnRectTemp = RinitRgnRect(NULL, NULL);
    // assert(rgnRectTemp != NULL);

    for(iCnt = 0; iCnt < DIMENSION; iCnt++)
    {   switch(tnInfo->ndType)
        {   case INTNODE:
			rgnRectTemp->iTopRight[iCnt] = (tnInfo->tdInfo->rgnRect->iTopRight[iCnt] > rgnRect->iTopRight[iCnt]) ? tnInfo->tdInfo->rgnRect->iTopRight[iCnt] : rgnRect->iTopRight[iCnt];
			rgnRectTemp->iBottomLeft[iCnt] = (tnInfo->tdInfo->rgnRect->iBottomLeft[iCnt] < rgnRect->iBottomLeft[iCnt]) ? tnInfo->tdInfo->rgnRect->iBottomLeft[iCnt] : rgnRect->iBottomLeft[iCnt];
            break;

		    case EXTNODE:
			rgnRectTemp->iTopRight[iCnt] = (tnInfo->tdInfo->dataClstElem->iData[iCnt] > rgnRect->iTopRight[iCnt]) ? tnInfo->tdInfo->dataClstElem->iData[iCnt] : rgnRect->iTopRight[iCnt];
			rgnRectTemp->iBottomLeft[iCnt] = (tnInfo->tdInfo->dataClstElem->iData[iCnt] < rgnRect->iBottomLeft[iCnt]) ? tnInfo->tdInfo->dataClstElem->iData[iCnt] : rgnRect->iBottomLeft[iCnt];
			break;
		}
	}
	double dExp = Rarea(rgnRectTemp) - Rarea(rgnRect);
	if(*ptrDMinExp == -1 || dExp <= *ptrDMinExp)
    {   
    	if(dExp == *ptrDMinExp)
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

double Rarea(Region rgnRect)
{
    if(rgnRect == NULL)
		return 0;
    double dArea = 1;
	int iCnt = 0;
	for(iCnt = 0; iCnt < DIMENSION; iCnt++)
		dArea = dArea * (rgnRect->iTopRight[iCnt] - rgnRect->iBottomLeft[iCnt]);
	return dArea;
}

RLstNd RpickChild(RHdrNd ptrChildLst, RTreeNode tnInfo)
{
    if(ptrChildLst == NULL)
		return NULL;

	RLstNd lstNdTemp = ptrChildLst->ptrFirstNd;
	RLstNd lstNdChild = NULL;
	double dMinExp = -1;
	int iCnt;
    Region rgnNewRect = RinitRgnRect(NULL, NULL);
    // assert(rgnNewRect != NULL);

    Region rgnFinalRect = RinitRgnRect(NULL, NULL);
    // assert(rgnFinalRect != NULL);

	while(lstNdTemp != NULL)
	{
		if(RexpansionArea(lstNdTemp->tnInfo->tdInfo->rgnRect, tnInfo, &dMinExp, rgnNewRect))
		{
               if(dMinExp < 0)
               {     dMinExp = 0 - dMinExp;
                     if(Rarea(lstNdChild->tnInfo->tdInfo->rgnRect) > Rarea(lstNdTemp->tnInfo->tdInfo->rgnRect))
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

void RpickSeeds(RHdrNd ptrChildLst, RLstNd *lstNdChildOne, RLstNd *lstNdChildTwo)
{
	if(ptrChildLst == NULL)
		return;
	RTreeNode* tnInfoMin = (RTreeNode *)calloc(DIMENSION, sizeof(struct RtreeNode));
	// assert(tnInfoMin != NULL);

	RTreeNode* tnInfoMax = (RTreeNode *)calloc(DIMENSION, sizeof(struct RtreeNode));
	// assert(tnInfoMax != NULL);
	RTreeNode temp;
	RLstNd lstNdTemp = NULL;
    int iCnt = 0;
	Boolean bIsFirst = TRUE;
	double dNormSep = 0;
	double dMaxNormSep = 0;
	dimension dimMaxNormSep = 0;
	Region rgnRectTemp = RinitRgnRect(NULL, NULL);
	// assert(rgnRectTemp != NULL);

	Region rgnRectOut = RinitRgnRect(NULL, NULL);
	// assert(rgnRectOut != NULL);

	switch(ptrChildLst->ptrFirstNd->tnInfo->ndType)
    {	
    	case INTNODE:   
    		lstNdTemp = ptrChildLst->ptrFirstNd;

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
                
                /*if(tnInfoMin[(int)dimMaxNormSep]==tnInfoMax[(int)dimMaxNormSep])
                {
                   	printf("error in the code\n");
                   	exit(-1);
                }*/
                        
	            *lstNdChildOne = RdeleteLstElem(ptrChildLst, tnInfoMin[(int)dimMaxNormSep]);
	  
	            *lstNdChildTwo = RdeleteLstElem(ptrChildLst, tnInfoMax[(int)dimMaxNormSep]);
	                     
	            free(rgnRectTemp->iBottomLeft);
	            free(rgnRectTemp->iTopRight);
	            free(rgnRectTemp);
	            free(rgnRectOut->iBottomLeft);
	            free(rgnRectOut->iTopRight);
	            free(rgnRectOut);
                break;

		case EXTNODE:        
	        lstNdTemp = ptrChildLst->ptrFirstNd;
	        DataPoint iDataMin = (DataPoint)calloc(DIMENSION, sizeof(dataPoint));
	        // assert( iDataMin != NULL);

	        DataPoint iDataMax = (DataPoint)calloc(DIMENSION, sizeof(dataPoint));
	        // assert(iDataMax != NULL);

            bIsFirst = TRUE;
	        while(lstNdTemp != NULL)
            {     
            	for(iCnt = 0; iCnt < DIMENSION; iCnt++)
                {   
                	if(bIsFirst)
                    {    
                        iDataMin[iCnt] = lstNdTemp->tnInfo->tdInfo->dataClstElem->iData[iCnt];
				        iDataMax[iCnt] = lstNdTemp->tnInfo->tdInfo->dataClstElem->iData[iCnt];
                        tnInfoMin[iCnt] = lstNdTemp->tnInfo;
				        tnInfoMax[iCnt] = lstNdTemp->tnInfo;
			            continue;
                    }

			        if(lstNdTemp->tnInfo->tdInfo->dataClstElem->iData[iCnt] <= iDataMin[iCnt])
                    {   
                    	iDataMin[iCnt] = lstNdTemp->tnInfo->tdInfo->dataClstElem->iData[iCnt];
				        tnInfoMin[iCnt] = lstNdTemp->tnInfo;
                    }
		            
		            if(lstNdTemp->tnInfo->tdInfo->dataClstElem->iData[iCnt] > iDataMax[iCnt])
                    {    
                    	iDataMax[iCnt] = lstNdTemp->tnInfo->tdInfo->dataClstElem->iData[iCnt];
                        tnInfoMax[iCnt]  = lstNdTemp->tnInfo;
                    }
                }
                lstNdTemp = lstNdTemp->ptrNextNd;
                if(bIsFirst)
                    bIsFirst = FALSE;
            } 
	        double dSep = 0;
	        double dMaxSep = 0;
	        dimension dimMaxSep = 0;

	        for(iCnt = 0; iCnt < DIMENSION; iCnt++)
            {      
            	dSep = fabs(iDataMax[iCnt] - iDataMin[iCnt]);
		        if(dSep > dMaxSep)
                {   
                	dMaxSep = dSep;
			        dimMaxSep = iCnt;
	            }
            }
	        
	        *lstNdChildOne = RdeleteLstElem(ptrChildLst, tnInfoMin[(int)dimMaxSep]);
	        
	        *lstNdChildTwo = RdeleteLstElem(ptrChildLst, tnInfoMax[(int)dimMaxSep]);
	        free(rgnRectTemp->iBottomLeft);
	        free(rgnRectTemp->iTopRight);
	        free(rgnRectTemp);
	        free(rgnRectOut->iBottomLeft);
	        free(rgnRectOut->iTopRight);
	        free(rgnRectOut);
	        free(iDataMin);
	        free(iDataMax);
            break;
	}

	free(tnInfoMin);
	free(tnInfoMax);

	return;
}

void RsplitNode(RLstNd ptrChild)
{    
	if(ptrChild == NULL || RisLstEmpty(ptrChild->ptrChildLst))
		return;

	RLstNd lstNdOne = NULL;
	RLstNd lstNdTwo = NULL;
	RTreeNode tnInfoTemp = NULL;

	RLstNd lstNdTemp = NULL;

	double dExpOne = -1;
	double dExpTwo = -1;

	RpickSeeds(ptrChild->ptrChildLst, &lstNdOne, &lstNdTwo);

	if(lstNdOne == NULL || lstNdTwo == NULL)
		return;

	RLstNd ptrChildTemp = RinitLstNd(RinitIntNd(NULL, NULL));
	RLstNd ptrChildSib = RinitLstNd(RinitIntNd(NULL, NULL));

	ptrChildTemp->ptrChildLst = RinitHdrNd();
	ptrChildSib->ptrChildLst = RinitHdrNd();
	ptrChildSib->ptrNextNd = ptrChild->ptrNextNd;

	RinsertLstNd(ptrChildTemp->ptrChildLst, lstNdOne);
	RsetRect(ptrChildTemp, lstNdOne->tnInfo);
	RinsertLstNd(ptrChildSib->ptrChildLst, lstNdTwo);
	RsetRect(ptrChildSib, lstNdTwo->tnInfo);

	Region rgnNewRectOne = RinitRgnRect(NULL, NULL);
	// assert(rgnNewRectOne != NULL);

	Region rgnNewRectTwo = RinitRgnRect(NULL, NULL);
	// assert(rgnNewRectTwo != NULL);

	Boolean bIsOne = FALSE;
	Boolean bIsNdOneInComp = FALSE;
	Boolean bIsNdTwoInComp = FALSE;

	int iCnt = 0;

	lstNdTemp = RdeleteLstFirst(ptrChild->ptrChildLst);

	while(lstNdTemp != NULL)
	{
		if(ptrChildTemp->ptrChildLst->uiCnt + ptrChild->ptrChildLst->uiCnt == RMINENTRIES - 1)
			bIsNdOneInComp = TRUE;

		if(ptrChildSib->ptrChildLst->uiCnt + ptrChild->ptrChildLst->uiCnt == RMINENTRIES - 1)
			bIsNdTwoInComp = TRUE;
		if(!bIsNdOneInComp && !bIsNdTwoInComp)
        {   
        	dExpOne = -1;
		    dExpTwo = -1;
		    RexpansionArea(ptrChildTemp->tnInfo->tdInfo->rgnRect, lstNdTemp->tnInfo, &dExpOne, rgnNewRectOne);
		    RexpansionArea(ptrChildSib->tnInfo->tdInfo->rgnRect, lstNdTemp->tnInfo, &dExpTwo, rgnNewRectTwo);

		    if(dExpOne < dExpTwo)
			     bIsOne = TRUE;
	        if(dExpOne > dExpTwo)
		         bIsOne = FALSE;
	        if(dExpOne == dExpTwo)
	        {
		         double dAreaOne = Rarea(ptrChildTemp->tnInfo->tdInfo->rgnRect);
			     double dAreaTwo = Rarea(ptrChildSib->tnInfo->tdInfo->rgnRect);
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
			RinsertLstNd(ptrChildTemp->ptrChildLst, lstNdTemp);
            if(bIsNdOneInComp)
            {   dExpOne = -1;
				RexpansionArea(ptrChildTemp->tnInfo->tdInfo->rgnRect, lstNdTemp->tnInfo, &dExpOne, rgnNewRectOne);
			}
			for(iCnt = 0; iCnt < DIMENSION; iCnt++)
            {	ptrChildTemp->tnInfo->tdInfo->rgnRect->iBottomLeft[iCnt] = rgnNewRectOne->iBottomLeft[iCnt];
				ptrChildTemp->tnInfo->tdInfo->rgnRect->iTopRight[iCnt] = rgnNewRectOne->iTopRight[iCnt];
			}
		}
		else
		{
			RinsertLstNd(ptrChildSib->ptrChildLst, lstNdTemp);
            if(bIsNdTwoInComp)
            {   dExpTwo = -1;
				RexpansionArea(ptrChildSib->tnInfo->tdInfo->rgnRect, lstNdTemp->tnInfo, &dExpTwo, rgnNewRectTwo);
			}
			for(iCnt = 0; iCnt < DIMENSION; iCnt++)
            {	ptrChildSib->tnInfo->tdInfo->rgnRect->iBottomLeft[iCnt] = rgnNewRectTwo->iBottomLeft[iCnt];
				ptrChildSib->tnInfo->tdInfo->rgnRect->iTopRight[iCnt] = rgnNewRectTwo->iTopRight[iCnt];
			}
		}
		lstNdTemp = RdeleteLstFirst(ptrChild->ptrChildLst);
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

Boolean RinsertTree(RHdrNd hdrNdTree, RTreeNode tnInfo)
{
	int iCnt = 0;

	if(hdrNdTree->ptrFirstNd == NULL || hdrNdTree->ptrFirstNd->tnInfo->ndType == EXTNODE)
    {   

		
		RinsertLstElem(hdrNdTree, tnInfo);

		
		return(hdrNdTree->uiCnt > RMAXENTRIES) ? TRUE : FALSE;
    }

	if(hdrNdTree->ptrFirstNd->ptrChildLst->uiCnt == 0)
		RsetRect(hdrNdTree->ptrFirstNd, tnInfo);


	RLstNd lstNdTemp = RpickChild(hdrNdTree, tnInfo);

	if(RinsertTree(lstNdTemp->ptrChildLst, tnInfo))
	{

        RsplitNode(lstNdTemp);

	    hdrNdTree->uiCnt++;
	    return (hdrNdTree->uiCnt > RMAXENTRIES) ? TRUE : FALSE;
	}


	return FALSE;
}

RHdrNd RbuildRTree(DataHdr dataHdrLst)
{
	RHdrNd hdrNdTree = RinitHdrNd();
	hdrNdTree->ptrFirstNd = RinitLstNd(RinitIntNd(NULL, NULL));
	hdrNdTree->uiCnt++;
	hdrNdTree->ptrFirstNd->ptrChildLst = RinitHdrNd();

	int cnt = 0,i;
	for(i=0;i<dataHdrLst->uiCnt;i++)
	{
		RinsertTree(hdrNdTree, RinitExtNd((dataHdrLst->dataClstElem)+i));
		if(hdrNdTree->uiCnt > 1)
			hdrNdTree = RcreateRoot(hdrNdTree);
	}

	return	hdrNdTree;
}

void RprintTree(RHdrNd hdrNdTree)
{   

	RLstNd lstNdTemp = hdrNdTree->ptrFirstNd;
	int iCnt = 0;
	static int iIndent = 0;
	iIndent++;

	while(lstNdTemp != NULL)
    {   
    	for(iCnt = 0; iCnt < iIndent; iCnt++)
			printf("---");

		if(lstNdTemp->tnInfo->ndType == INTNODE)
        {   
        	printf("i hav %d children..\n",lstNdTemp->ptrChildLst->uiCnt);
			printf(" BottomLeft: ");

			for(iCnt = 0; iCnt < DIMENSION; iCnt++)
				printf("%lf ", lstNdTemp->tnInfo->tdInfo->rgnRect->iBottomLeft[iCnt]);

            printf(" TopRight: ");

			for(iCnt = 0; iCnt < DIMENSION; iCnt++)
				printf("%lf ", lstNdTemp->tnInfo->tdInfo->rgnRect->iTopRight[iCnt]);

            printf("\n");
			printf("i hav %d children..\n",lstNdTemp->ptrChildLst->uiCnt);
			RprintTree(lstNdTemp->ptrChildLst);
		}
		else
        {   printf(" DataPoint: ");
    		printData(lstNdTemp->tnInfo->tdInfo->dataClstElem);
			printf("\n");
		}
		lstNdTemp = lstNdTemp->ptrNextNd;
	}
	iIndent--;
	return;
}

double RfindDist(DataPoint iDataOne, DataPoint iDataTwo)
{   
	double dDist = 0;
	int iCnt = 0;
	int k=0;
	for(iCnt = 0; iCnt < DIMENSION; iCnt++)
	{    
		dDist = dDist + pow(iDataOne[iCnt] - iDataTwo[iCnt], 2);
	}

	return sqrt(dDist);
}

Boolean RisContains(Region rgnRect, DataPoint iData)
{
	int iCnt = 0;
	Boolean bIsContains = TRUE;
	for(iCnt = 0; iCnt < DIMENSION; iCnt++)
    {   
    	if((rgnRect->iBottomLeft[iCnt] > iData[iCnt]) || (rgnRect->iTopRight[iCnt] < iData[iCnt]))
        {   
        	bIsContains = FALSE;
			break;
		}
	}
	return bIsContains;
}

Boolean RisOverLap(Region rgnRectOne, Region rgnRectTwo)
{
	int iDim = 0;
	for(iDim = 0; iDim < DIMENSION; iDim++)
		if(rgnRectOne->iTopRight[iDim] < rgnRectTwo->iBottomLeft[iDim] || rgnRectTwo->iTopRight[iDim] < rgnRectOne->iBottomLeft[iDim])
			return FALSE;
	return TRUE;
}

void RappendRTree(RHdrNd hdrNdTree, DataHdr dataHdrLst)
{
	int cnt = 0,i;
    for(i=0;i<dataHdrLst->uiCnt;i++)
	{
		RinsertTree(hdrNdTree, RinitExtNd(dataHdrLst->dataClstElem+i));
	}

	return;	
}

void freeRTree(RHdrNd hdrNdTree)
{

	if(hdrNdTree == NULL) 
		return;

	if(hdrNdTree->uiCnt == 0)
	{
		free(hdrNdTree);
		return;
	}
	
	RLstNd lstNdTemp = hdrNdTree->ptrFirstNd;
	RLstNd lstNdNextTemp;
	
	if(lstNdTemp!=NULL)
	{
		while(lstNdTemp != NULL)
    	{
			switch(lstNdTemp->tnInfo->ndType)
        	{   
        		case INTNODE:   
				    			free(lstNdTemp->tnInfo->tdInfo->rgnRect->iBottomLeft);
				    			free(lstNdTemp->tnInfo->tdInfo->rgnRect->iTopRight);
				    			free(lstNdTemp->tnInfo->tdInfo->rgnRect);
				    			free(lstNdTemp->tnInfo->tdInfo);
				    			free(lstNdTemp->tnInfo);				    			
				    			freeRTree(lstNdTemp->ptrChildLst);                            	
                            	lstNdNextTemp = lstNdTemp->ptrNextNd;
                            	free(lstNdTemp);
                            	break;
            	case EXTNODE:   					            
					            free(lstNdTemp->tnInfo->tdInfo);
					            free(lstNdTemp->tnInfo);
					            freeRTree(lstNdTemp->ptrChildLst);
					            lstNdNextTemp = lstNdTemp->ptrNextNd;
                            	free(lstNdTemp);	                            
		                        break;

			}
			
			lstNdTemp = lstNdNextTemp;
		}
	}	
	
	free(hdrNdTree);	
	return;

}

void isCorrectRTree(RHdrNd hdrNdTree)
{
    RLstNd lstNdTemp = hdrNdTree->ptrFirstNd,temp;
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
		                    if(temp->tnInfo->tdInfo->rgnRect->iBottomLeft[iCnt]<lstNdTemp->tnInfo->tdInfo->rgnRect->iBottomLeft[iCnt])
		                        flag=1;
		                }
		                for(iCnt=0;iCnt<DIMENSION;iCnt++)
		                {
		                    if(temp->tnInfo->tdInfo->rgnRect->iTopRight[iCnt]>lstNdTemp->tnInfo->tdInfo->rgnRect->iTopRight[iCnt])
		                        flag=1;
		                }
		                if(flag==1)
		                {
		                    printf("WRONG!!!\n");
		                    for(iCnt=0;iCnt<DIMENSION;iCnt++)
		                    {
		                        printf("%lf %lf %lf %lf\n",temp->tnInfo->tdInfo->rgnRect->iBottomLeft[iCnt],temp->tnInfo->tdInfo->rgnRect->iTopRight[iCnt],lstNdTemp->tnInfo->tdInfo->rgnRect->iBottomLeft[iCnt],lstNdTemp->tnInfo->tdInfo->rgnRect->iTopRight[iCnt]);
		                    }
		                }
		                temp=temp->ptrNextNd;
            		break;

            		case EXTNODE:
            			return;
            		break;
            	}
                
            }
            isCorrectRTree(lstNdTemp->ptrChildLst);
        }
        lstNdTemp = lstNdTemp->ptrNextNd;
    }
    return;
}	
