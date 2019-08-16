/*

Algorithm: Micro-cluster based DBSCAN
Author: Aditya Sarma
email: asaditya1195@gmail.com

*/

#ifndef GRIDRTREE_H

#define GRIDRTREE_H

#include "GList.h"
#include "MuC_RTree.h"
#include "RTree.h"
#include <vector>
double distance(DataPoint point1, DataPoint point2);
void insertGroupIntoGroupList(Group newGroup);
void insertPointintoGroup(DataHdr dataList, Group currGroup, Data currDataPoint);
Boolean GisOverLap(Region rgnRectOne, Region rgnRectTwo);
Boolean GisOverLapTotal(Region rgnRectOne, Region rgnRectTwo);
Boolean GisOverLap2(Region rgnRectOne, Region rgnRectTwo);
Region GinitRgnRect(Dimension iBottomLeft, Dimension iTopRight);
GTreeNode GinitIntNd(Dimension iBottomLeft, Dimension iTopRight);
GTreeNode GinitExtNd(Group groupElem);
void GsetRect(GLstNd lstNd, GTreeNode tnInfo);
GLstNd GpickChild(GHdrNd ptrChildLst, GTreeNode tnInfo);
Boolean GexpansionArea(Region rgnRect, GTreeNode tnInfo, Double ptrDMinExp, Region rgnNewRect);
double Garea(Region rgnRect);
Boolean GinsertTree(GHdrNd hdrNdTree, GTreeNode tnInfo, int gMinEntries, int gMaxEntries);
GHdrNd GcreateRoot(GHdrNd hdrNdTree);
void GsplitNode(GLstNd ptrChild, int gminEntries);
void GpickSeeds(GHdrNd ptrChildLst, GLstNd *lstNdChildOne, GLstNd *lstNdChildTwo);
int GisContainsCell(Region rgnRect, Group group);
Group GfindCell(GHdrNd hdrNdTree, Region rgnRect);
void GprintTree(GHdrNd hdrNdTree);
void GprintRegion(Region region);
void GgetNeighborHood(GHdrNd hdrNdTree, Data currDataPoint);
void appendNbh(Data currDataPoint, Data temp);
unsigned int GfindOverlappingCells(GHdrNd hdrNdTree, Region rgnRect, Region cellRgnRect, BCellListHd cellsList);
Boolean GisOverLapTotal_MN(Region rgnRectOne, Region rgnRectTwo, BCell tempCell, int myrank);
unsigned int GfindOverlappingCellsTotalOverlap_MN(GHdrNd hdrNdTree, Region rgnRect, Region cellRgnRect, BCellListHd cellsList, BCell tempCell, int myrank);
unsigned int GfindOverlappingCellsTotalOverlap(GHdrNd hdrNdTree, Region rgnRect, Region cellRgnRect, BCellListHd cellsList);
unsigned int GfindOverlappingCellsOptimized(GHdrNd hdrNdTree, Region rgnRect, Region cellRgnRect, BCellListHd cellsList, Region halfEpsExtended);
unsigned int GfindOverlappingCells2(GHdrNd hdrNdTree, Region rgnRect, Region cellRgnRect, BCellListHd cellsList);
unsigned int GfindTotalOverlappingCells2(GHdrNd hdrNdTree, Region rgnRect, Region cellRgnRect, BCellListHd cellsList);
unsigned int GfindOverlappingCells3(GHdrNd hdrNdTree, Region rgnRect, Region cellRgnRect, BCellListHd cellsList,BCellListHd cellsList2);
void populateAuxRTrees(DataHdr dataList, struct vectorc* groupList);
void findReachableGroupsofGroupG(DataHdr dataList, GHdrNd GRTree, int i);
void findReachableMuCs(DataHdr dataList, GHdrNd GRTree, vectorc* groupList, vector< vector<int> >* p_cur_insert);

GHdrNd populateMuCRTree(DataHdr dataHdrLst, int gMinEntries, int gMaxEntries);
Region createCellRegOfPoint(Data currDataPoint, double eps);
Region createCellRegOfDataPoint(DataPoint currDataPoint, double eps);
GHdrNd insertGroupIntoRTree(GHdrNd hdrNdTree, Group groupNode, int gMinEntries, int gMaxEntries);
void printMinGridSize();
void printMaxGridSize();
Region getEpsExtendedRegion(Region cellRgnRect, double eps);
Region getEpsOptimalExtendedRegion(Region cellRgnRect, double eps);
Region getEpsExtendedRegionPoint(Data dataPoint, double eps);
void freeGRTree(GHdrNd hdrNdTree);
void isCorrectGRTree(GHdrNd hdrNdTree);
// vector<int> findNeighbours(GHdrNd GRTree, DataHdr dataHdrLst, int point_id, double eps);
// vector<int> findNeighbours2(DataHdr dataHdrLst, int point_id, double eps);
// vectorc* findNeighbours3(DataHdr dataHdrLst, int point_id, double eps);
// vectorc* findNeighbours2(DataHdr dataList, int pointid, double eps);
Neighbours findNeighbours3(DataHdr dataList, int pointid, double eps, vector < vector <int > >* p_cur_insert);
void findNeighboursGroup(RHdrNd hdrNdTree, Region rgnRect, Data data, Neighbours n, double eps);

Neighbours findNeighboursGroup_Core(RHdrNd hdrNdTree, Region rgnRect, Data data, Neighbours n, double eps);
//RHdrNd getAuxRTreeofNeighbors(BCell bCellElem, GHdrNd hdrNdTree);

#endif