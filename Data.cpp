/*

Algorithm: Micro-cluster based DBSCAN
Author: Aditya Sarma

email: asaditya1195@gmail.com

*/

#include "Data.h"
#include "vectorc.h"


void unionFindMerge(DataHdr dataList, int root1, int root2, Data dataroot1, Data dataroot2)
{
    // root1 = dataroot1->id;
    // root2 = dataroot2->id;
    int root;
    while(dataroot1->parentId != dataroot2->parentId)
    {
        if(dataroot1->parentId < dataroot2->parentId)
        {
            // Find top most point of the tree that dataroot1 belongs to. Store that number in the variable root
            if(dataroot1->parentId == root1)
            {
                dataroot1->parentId = dataroot2->parentId;
                root = dataroot2->parentId;
                break;
            }

            // splicing compression technique
            int z = dataroot1->parentId;
            dataroot1->parentId = dataroot2->parentId;
            root1 = z;
            dataroot1 = dataList->dataClstElem + root1;
        }
        else
        {
            if(dataroot2->parentId == root2)
            {
                dataroot2->parentId = dataroot1->parentId;
                root = dataroot1->parentId;
                break;
            }

            // splicing compressio technique
            int z = dataroot2->parentId;
            dataroot2->parentId = dataroot1->parentId;                
            root2 = z;
            dataroot2 = dataList->dataClstElem + root2;
        }
    }

}

DataHdr initDataHdr(int size)
{   
    DataHdr dataHdrInfo = (DataHdr)calloc(1, sizeof(struct dataHdr));
    // assert(dataHdrInfo != NULL);

    dataHdrInfo->uiCnt = 0;
    dataHdrInfo->localCnt = 0;
    dataHdrInfo->dataClstElem=(Data)calloc(size, sizeof(struct data));
    // assert(dataHdrInfo->dataClstElem!=NULL);

    return dataHdrInfo;
}

void insertDataLstElemRemote(DataHdr dataHdrInfo, DataPoint iData, int parentId, int remoteIndex)
{

    Data dataClstElem = dataHdrInfo->dataClstElem + dataHdrInfo->uiCnt;
    dataClstElem->id = dataHdrInfo->uiCnt;
    dataClstElem->iData = iData;
    dataHdrInfo->uiCnt++;

    dataClstElem->haloPoint = TRUE;
    dataClstElem->childCount = 0;
    dataClstElem->cid = -1;
    dataClstElem->actualProcessId = parentId;
    dataClstElem->remoteIndex = remoteIndex;

    dataClstElem->parentId = remoteIndex;
    dataClstElem->parentProcessId = parentId;

    dataClstElem->isProcessed = TRUE;
    dataClstElem->core_tag = FALSE;
    dataClstElem->ClusterID = 0;
    dataClstElem->group_id = -1;

    return;
}

void populateDataListRemote(DataHdr dataList, vector< vector<double> >& remote_objects, vectorc* remote_PrIDs, vectorc* remote_Indices)
{
    
    int i;
    DataPoint iData;
    // if(dataList->uiCnt != dataList->localCnt)
    // {
    //     fprintf(stderr, "Unacceptable state! LocalCnt %d is supposed to be equal to total Count %d here!!\n", dataList->localCnt, dataList->uiCnt);
    //     exit(-1);
    // }

    for(i=0; i<remote_objects.size(); i++)
    {
        iData = (DataPoint) calloc(DIMENSION, sizeof(double));
        // assert(iData != NULL);
        int j;

        for(j=0;j<DIMENSION;j++)
        {
            iData[j] = remote_objects[i][j];
        }
        
        insertDataLstElemRemote(dataList, iData, remote_PrIDs->intItems[i], remote_Indices->intItems[i]);
        vector<double>().swap(remote_objects[i]);
    }
    vector< vector<double> >().swap(remote_objects);
}


void insertDataLstElemLocal(DataHdr dataHdrInfo, DataPoint iData, int rank)
{
    Data dataClstElem = dataHdrInfo->dataClstElem + dataHdrInfo->uiCnt;
    dataClstElem->id = dataHdrInfo->uiCnt; //ID represents local id. ID with respect to the local node
    dataClstElem->iData = iData;
    
    dataHdrInfo->uiCnt++;
    dataHdrInfo->localCnt++;
    
    dataClstElem->haloPoint = FALSE;
    dataClstElem->remoteIndex = -1;
    dataClstElem->actualProcessId = rank;

    dataClstElem->childCount = 0;
    dataClstElem->cid = -1;

    dataClstElem->parentId = dataClstElem->id;
    dataClstElem->parentProcessId = rank;

    dataClstElem->isProcessed = FALSE;
    dataClstElem->core_tag = FALSE;
    dataClstElem->ClusterID = 0;
    dataClstElem->group_id = -1;

    return;
}

void populateDataListLocal(DataHdr dataList, vector< vector< double > >& objects)
{
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    int i;
    DataPoint iData;

    // if(dataList->uiCnt != 0 || dataList->localCnt != 0)
    // {
    //     fprintf(stderr, "Unacceptable state! dataList is supposed to empty here! LSize = %d TSize = %d\n", dataList->uiCnt, dataList->localCnt);
    //     exit(-1);
    // }

    for(i=0;i<objects.size();i++)
    {
        iData = (DataPoint) calloc(DIMENSION, sizeof(double));
        // assert(iData != NULL);
        int j;

        for(j=0;j<DIMENSION;j++)
        {
            iData[j] = objects[i][j];
        }

        insertDataLstElemLocal(dataList, iData, rank);
        vector<double>().swap(objects[i]);
    }
    vector< vector <double> >().swap(objects);
}


void printData(Data dataPoint)
{
    printf("Group %d\t",dataPoint->group_id);
    int iCnt=0;
    for(iCnt = 0; iCnt < DIMENSION; iCnt++)
        printf("%lf ", dataPoint->iData[iCnt] );

    return;
}


void freeDataList(DataHdr dataList1)
{
    int i;
    for(i=0;i<dataList1->uiCnt;i++)
        free((dataList1->dataClstElem+i)->iData);

    free(dataList1->dataClstElem);

    return;
}

