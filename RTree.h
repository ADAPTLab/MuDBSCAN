/*

Algorithm: Micro-cluster based DBSCAN
Author: Aditya Sarma
email: asaditya1195@gmail.com

*/

#ifndef RTREE_H
#define RTREE_H

#include "RList.h"
#include "Data.h"

Region RinitRgnRect(Dimension iBottomLeft, Dimension iTopRight);
void RsetRect(RLstNd lstNd, RTreeNode tnInfo);

RTreeNode RinitExtNd(Data dataClstElem);
RTreeNode RinitIntNd(Dimension iBottomLeft, Dimension iTopRight);
RHdrNd RcreateRoot(RHdrNd hdrNdTree);
Boolean RexpansionArea(Region rgnRect, RTreeNode tnInfo, Double ptrDMinExp, Region rgnNewRect);
double Rarea(Region rgnRect);

RLstNd RpickChild(RHdrNd ptrChildLst, RTreeNode tnInfo);
void RpickSeeds(RHdrNd ptrChildList, RLstNd *lstNdChildOne, RLstNd *lstNdChildTwo);
void RsplitNode(RLstNd ptrChild);

Boolean RinsertTree(RHdrNd hdrNdTree, RTreeNode tnInfo);
RHdrNd RbuildRTree(DataHdr dataHdrLst);

RHdrNd RbuildRTreeFromCells(BCellListHd cellsList, Region epsExtendedRgn);



void RprintTree(RHdrNd hdrNdTree);
double RfindDist(DataPoint iDataOne, DataPoint iDataTwo);
unsigned int RgetNeighborHood(RHdrNd hdrNdTree, Data dataNdTemp,int size);

unsigned int RfindRecords(RHdrNd hdrNdTree, Region rgnRect, Data dataNdTemp);

Boolean RisContains(Region rgnRect, DataPoint iData);
Boolean RisOverLap(Region rgnRectOne, Region rgnRectTwo);

void RappendRTree(RHdrNd hdrNdTree, DataHdr dataHdrLst);

void freeRTree(RHdrNd auxRTree);
void isCorrectRTree(RHdrNd hdrNdTree);

#endif
