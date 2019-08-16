/*

Algorithm: Micro-cluster based DBSCAN
Author: Aditya Sarma
email: asaditya1195@gmail.com

*/

#ifndef GLIST_H
#define GLIST_H
#include "Def.h"

GHdrNd GinitHdrNd();
GLstNd GinitLstNd(GTreeNode tnInfo);
void GinsertLstElem(GHdrNd HdrNdLst, GTreeNode tnInfo);
void GinsertLstNd(GHdrNd HdrNdLst, GLstNd LstNdElem);
Boolean GisLstEmpty(GHdrNd HdrNdLst);
GLstNd GdeleteLstElem(GHdrNd HdrNdLst, GTreeNode tnInfo);
GLstNd GdeleteLstFirst(GHdrNd HdrNdLst);
GLstNd GdeleteNextNd(GHdrNd HdrNdLst, GLstNd LstNdElem);

#endif
