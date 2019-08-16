#ifndef DATAPARALLEL_H
#define DATAPARALLEL_H
#include "Def.h"
#include "Data.h"

int partitions(DataPoint *objects, int low, int high, int dim, int numCoords, dataPoint *temp) ;
int adjustPartition(DataPoint *objects, int low, int high, int dim, int numCoords, int pivot);
int adjustCellSize(DataPoint *objects, int low, int high, int dim, int numCoords, int pivot);
int quickSelect(DataPoint *objects, int left, int right, int kth, int dim, int numCoords);
int findMaxRange(DataPoint *objects, int numObjs, int numCoords);
void KDtreeGeneric(DataPoint *objects, int numCoords, int nproc, int startPoint, int endPoint, int procId, int * StartArray,int *endIndexNEW, int *split, int *split_dim);
void  file_read1(  char *filename, int  *numObjs, int  *numCoords,double*** objects);     /* no. coordinates */


DataHdr distributePoints(int numprocs, int myrank, int * endIndexNEW, int * startIndex, double *** objects, int totalPoints,int numObjs, int DIMENSION );
DataHdr readDataByObject(double **objects,int Sindex, int noOfPoints, int numObjs  );
void EPSextended(int myrank, int numCoords, double **** ll, double **objects, int numObjs,double ** maxTemp, double ** minTemp, int **epsSize, int numprocs, double EPS);
int AddEpsExtended(int myrank, int numprocs, double ***objects1, int *epsSize,int DIMENSION, double *** ll );
double ** AppendTwoObjectLists(int myrank, int numprocs,int *startIndex,int *endIndexNEW, double **objects, double **objects1,int totalPoints,int totalPoints1);
DataHdr ExpandData(DataHdr dataHdrInfo, double **objects1 , int already, int more, int myrank);
void parallelMerge(int ***SendInfo,DataHdr dataList, int numObjs, int numprocs, int myrank, MPI_Comm comm);
void sequentialMerge(int ***SendInfo, DataHdr dataList, int numObjs);
void bubbleSort(int *** x,int numObjs);
int** getClusters_MN(char * strFileName1, char * strFileName2, int** SendInfo, int numObjs);
#endif