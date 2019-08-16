/*

Algorithm: Micro-cluster based DBSCAN
Author: Aditya Sarma
email: asaditya1195@gmail.com

*/

#ifndef __DATA_H
#define __DATA_H
#include "Def.h"

DataHdr initDataHdr(int size_data);
void printData(Data dataPoint);
void freeDataList(DataHdr dataList1);

void unionFindMerge(DataHdr dataList, int root1, int root2, Data dataroot1, Data dataroot2);
void insertDataLstElemRemote(DataHdr dataHdrInfo, DataPoint iData, int parentId, int remoteIndex);
void populateDataListRemote(DataHdr dataList, vector< vector<double> >& remote_objects, vectorc* remote_PrIDs, vectorc* remote_Indices);
void insertDataLstElemLocal(DataHdr dataHdrInfo, DataPoint iData);
void populateDataListLocal(DataHdr dataList, vector< vector< double > >& objects);



#endif