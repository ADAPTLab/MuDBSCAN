/*

Algorithm: Micro-cluster based DBSCAN
Author: Aditya Sarma
email: asaditya1195@gmail.com

*/

#ifndef BCELL_H
#define BCELL_H

#include "Def.h"

CellDataHd initCellDataHd();
BCell initBCell(Region rectTemp);
CellData initCellData(Data dataClstElem);
BCellListHd initBCellListHd();
void freeCellsList(BCellListHd cellsListNbh);
void freeAllBCells(BCellListHd cellsList);
void freeBCell(BCell bCellElem);
void freeCellDataList(CellDataHd cellDataList);
void processGroup(DataHdr dataList, int i, vector < vector <int > >* p_cur_insert);
Group initGroup(Data datapoint);

void processGroupSequential(DataHdr dataList, Group g);
void freeGroup(Group group);
void printGroup(Group g, FILE* f);
#endif
