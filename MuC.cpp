/*

Algorithm: Micro-cluster based DBSCAN
Author: Aditya Sarma
email: asaditya1195@gmail.com
*/

#include "MuC.h"
#include "RList.h"
#include "MuC_RTree.h"
#include "vectorc.h"
#include "Data.h"


void processGroupSequential(DataHdr dataList, Group g)
{
    int myrank;
    MPI_Comm_rank(MPI_COMM_WORLD, &myrank);

    if(VECTOR_TOTAL(g->inner_points) >= MINPOINTS + 1)
    {
        int i;
        Data master = dataList->dataClstElem + g->master_id; // CORE POINT
        int root = master->id;

        for(i=0; i<VECTOR_TOTAL(g->inner_points); i++)
        {
            Data data = dataList->dataClstElem + g->inner_points->intItems[i];

            if(data->core_tag == FALSE) totalCore++;

            data->core_tag = TRUE; // ALL INNER POINTS ARE CORE POINTS
            data->isProcessed = TRUE;
            data->ClusterID = 1;
            if(visited[data->id] != 1)
            {
                visited[data->id] = 1;
                addHelper[0] = data->id;
                VECTOR_ADD(unprocessedCore, addHelper);
                VECTOR_ADD(g->corepoints, addHelper);
            }
        }
        i = 0;
        Data datalocal;
        Data dataroot1;
        Data dataroot2;
        for(i=0; i<VECTOR_TOTAL(g->total_points); i++)
        {
            datalocal = dataList->dataClstElem + g->total_points->intItems[i];

            int npid = datalocal->id;

            int root1 = npid;
            int root2 = root;

            datalocal->ClusterID = 1;

             dataroot1 = dataList->dataClstElem + root1;
             dataroot2 = dataList->dataClstElem + root2; //MASTER - CORE POINT

            while(dataroot1->parentId != dataroot2->parentId)
            {
                if(dataroot1->parentId < dataroot2->parentId)
                {
                    if(dataroot1->parentId == root1)
                    {
                        dataroot1->parentId = dataroot2->parentId;
                        root = dataroot2->parentId;
                        break;
                    }

                    // splicing comression technique
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
            datalocal->ClusterID = 1;
        }
    }
    else if(VECTOR_TOTAL(g->total_points) >= MINPOINTS + 1)
    {
        int i;

        g->type = COREGROUP;
        Data data = dataList->dataClstElem + g->master_id;

        if(data->core_tag == FALSE) totalCore++;

        data->core_tag = TRUE;

        data->isProcessed = TRUE; //Later in the dbscan() method this point will not undergo neighbourhood query.
        data->ClusterID = 1; //These are core points. They are inherently members of a cluster.
       
        if(visited[data->id] != 1)
        {
            visited[data->id] = 1;
            addHelper[0] = data->id;
            VECTOR_ADD(unprocessedCore, addHelper);
            VECTOR_ADD(g->corepoints, addHelper);
        }

        Data master = dataList->dataClstElem + g->master_id;
        int root = master->id;
        Data datalocal;
        Data dataroot1;
        Data dataroot2;
        for(i=0; i<VECTOR_TOTAL(g->total_points); i++)
        {
            datalocal = dataList->dataClstElem + g->total_points->intItems[i];

            int npid = datalocal->id;

            int root1 = npid;
            int root2 = root;

            datalocal->ClusterID = 1;

            dataroot1 = dataList->dataClstElem + root1;
            dataroot2 = dataList->dataClstElem + root2;

            // REMS algorithm to (union) merge the trees
            // UNION OPERATION
            while(dataroot1->parentId != dataroot2->parentId)
            {
                if(dataroot1->parentId < dataroot2->parentId)
                {
                    if(dataroot1->parentId == root1)
                    {
                        dataroot1->parentId = dataroot2->parentId;
                        root = dataroot2->parentId;
                        break;
                    }

                    // splicing comression technique
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

            datalocal->ClusterID = 1;
        }
    }
    else
    {
        g->type = SPARSEGROUP;
    }
}


void processGroup(DataHdr dataList, int i, vector < vector <int > >* p_cur_insert)
{
    int myrank;
    MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
    Group g = groupList->groupItems[i];

    if(VECTOR_TOTAL(g->inner_points) >= MINPOINTS + 1)
    {
        int i;
        Data master = dataList->dataClstElem + g->master_id; // CORE POINT
        int root = g->master_id;

        Data data;
        for(i=0; i<VECTOR_TOTAL(g->inner_points); i++)
        {
            data = dataList->dataClstElem + g->inner_points->intItems[i];

            if(data->haloPoint == FALSE && data->core_tag == FALSE) totalCore++;

            data->core_tag = TRUE; // ALL INNER POINTS ARE CORE POINTS
            data->isProcessed = TRUE;
            data->ClusterID = 1;
            
            if(visited[data->id] != 1)
            {
                visited[data->id] = 1;
                addHelper[0] = data->id;
                VECTOR_ADD(unprocessedCore, addHelper);
                VECTOR_ADD(g->corepoints, addHelper);
            } 
        }

        i = 0;

        //If master if non halo do merging right now itself
        //If master is halo then don't do merging now, because you don't know how to do merging. Do merging when you are doing postProcessing of core points. For every non halo core point check if
        //its master point is halo and core, if yes then do merging there

        if(master->haloPoint == FALSE) //master is local point
        {
            Data datalocal;
            Data dataroot1;
            Data dataroot2;
            for(i=0; i<VECTOR_TOTAL(g->total_points); i++)
            {
                datalocal = dataList->dataClstElem + g->total_points->intItems[i];
                datalocal->ClusterID = 1;

                if(datalocal->haloPoint == TRUE)
                {
                    Data dataremote = datalocal;
                    (*p_cur_insert)[dataremote->actualProcessId].push_back(master->id);
                    (*p_cur_insert)[dataremote->actualProcessId].push_back(dataremote->remoteIndex);
                }
                else
                {
                    int npid = datalocal->id;

                    int root1 = npid;
                    int root2 = root;

                     dataroot1 = dataList->dataClstElem + root1;
                     dataroot2 = dataList->dataClstElem + root2; //MASTER - CORE POINT

                    while(dataroot1->parentId != dataroot2->parentId)
                    {
                        if(dataroot1->parentId < dataroot2->parentId)
                        {
                            if(dataroot1->parentId == root1)
                            {
                                dataroot1->parentId = dataroot2->parentId;
                                root = dataroot2->parentId;
                                break;
                            }

                            // splicing comression technique
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

                            int z = dataroot2->parentId;
                            dataroot2->parentId = dataroot1->parentId;                
                            root2 = z;
                            dataroot2 = dataList->dataClstElem + root2;
                        }
                    }
                }
            }
        }
    }
    else if(VECTOR_TOTAL(g->total_points) >= MINPOINTS + 1)
    {
        int i;

        Data data = dataList->dataClstElem + g->master_id;
        if(data->haloPoint == FALSE && data->core_tag == FALSE) totalCore++;

        data->core_tag = TRUE;
        data->isProcessed = TRUE;
        data->ClusterID = 1;

        if(visited[data->id] != 1)
        {
            visited[data->id] = 1;
            addHelper[0] = data->id;
            VECTOR_ADD(unprocessedCore, addHelper);
            VECTOR_ADD(g->corepoints, addHelper);
        }

        Data master = dataList->dataClstElem + g->master_id;
        int root = master->id;
        //If master if non halo do merging right now itself
        //If master is halo then don't do merging now, because you don't know how to do merging. Do merging when you are doing postProcessing of core points. For every non halo core point check if
        //its master point is halo and core, if yes then do merging there
        if(master->haloPoint == FALSE)
        {
            Data datalocal;
            Data dataroot1;
            Data dataroot2;
            for(i=0; i<VECTOR_TOTAL(g->total_points); i++)
            {
                datalocal = dataList->dataClstElem + g->total_points->intItems[i];
                datalocal->ClusterID = 1;

                if(datalocal->haloPoint == TRUE)
                {
                        Data dataremote = datalocal;
                        (*p_cur_insert)[dataremote->actualProcessId].push_back(master->id);
                        (*p_cur_insert)[dataremote->actualProcessId].push_back(dataremote->remoteIndex);
                }
                else
                {
                    int npid = datalocal->id;

                    int root1 = npid;
                    int root2 = root;


                     dataroot1 = dataList->dataClstElem + root1;
                     dataroot2 = dataList->dataClstElem + root2;

                    // REMS algorithm to (union) merge the trees
                    // UNION OPERATION
                    while(dataroot1->parentId != dataroot2->parentId)
                    {
                        if(dataroot1->parentId < dataroot2->parentId)
                        {
                            if(dataroot1->parentId == root1)
                            {
                                dataroot1->parentId = dataroot2->parentId;
                                root = dataroot2->parentId;
                                break;
                            }

                            // splicing comression technique
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

                            int z = dataroot2->parentId;
                            dataroot2->parentId = dataroot1->parentId;                
                            root2 = z;
                            dataroot2 = dataList->dataClstElem + root2;
                        }
                    }
                }
            }
        }   
    }

}


void printGroup(Group g, FILE* f)
{
    fprintf(f, "ID: %d\tTotalPoints: %d\tInnerPoints: %d\tCorepoints: %d\n", g->id, g->total_count, g->inner_count, VECTOR_TOTAL(g->corepoints));
    fprintf(f, "ID: %d\tTotalPoints: %p\tInnerPoints: %p\tCorepoints: %p\n", g->id, g->total_points, g->inner_points, g->corepoints);
    fprintf(f, "ID: %d\tReachableGroups: %p\tBCell: %p\t\n", g->id, g->reachable_groups, g->bcell);

    fflush(f);
}

Group initGroup(Data datapoint)
{
    noOfGroups++;
    Group newGroup = (Group)calloc(1, sizeof(struct group)); 
    // assert(newGroup != NULL);

    newGroup->type = NONE;
    newGroup->id = GROUPID++;
    newGroup->bcell = NULL;
    newGroup->master_id = datapoint->id;
    newGroup->threshold = 0;

    newGroup->reachable_groups = NULL;
    
    newGroup->master = datapoint->iData;

    newGroup->inner_count = 0;
    newGroup->total_count = 0;

    newGroup->total_points = (vectorc*)malloc(sizeof(struct vectorc));
    // assert(newGroup->total_points != NULL);
    VECTOR_INIT(newGroup->total_points, INTEGER);

    newGroup->inner_points = (vectorc*)malloc(sizeof(struct vectorc));
    // assert(newGroup->inner_points != NULL);
    VECTOR_INIT(newGroup->inner_points, INTEGER);

    newGroup->corepoints = (vectorc*)malloc(sizeof(struct vectorc));
    // assert(newGroup->corepoints != NULL);
    VECTOR_INIT(newGroup->corepoints, INTEGER);

    newGroup->reachable_groups = (vectorc*)malloc(sizeof(struct vectorc));
    // assert(newGroup->reachable_groups != NULL);
    VECTOR_INIT(newGroup->reachable_groups, INTEGER);
    
    return newGroup;
}


BCell initBCell(Region rectTemp)
{
    BCell newBCell = (struct bcell*) calloc(1, sizeof(struct bcell));
    // assert(newBCell != NULL);

    newBCell->minOriginalBoundary = (double*)calloc(DIMENSION, sizeof(double));
    // assert(newBCell->minOriginalBoundary != NULL);

    newBCell->maxOriginalBoundary = (double*)calloc(DIMENSION, sizeof(double));
    // assert(newBCell->maxOriginalBoundary != NULL);

    newBCell->minActualBoundary = (double*)calloc(DIMENSION, sizeof(double));
    // assert(newBCell->minActualBoundary != NULL);

    newBCell->maxActualBoundary = (double*)calloc(DIMENSION, sizeof(double));
    // assert(newBCell->maxActualBoundary != NULL);

    newBCell->id= BCELLID++;
    newBCell->auxCellRTree = NULL;
    int i =0;
    for(i = 0; i < DIMENSION; i++)
    {
        newBCell->minOriginalBoundary[i] = rectTemp->iBottomLeft[i];
        newBCell->maxOriginalBoundary[i] = rectTemp->iTopRight[i];
    }

    return newBCell;
}

void freeGroup(Group group)
{       
    free(group->bcell->minOriginalBoundary);
    free(group->bcell->maxOriginalBoundary);
    free(group->bcell->minActualBoundary);
    free(group->bcell->maxActualBoundary);
    freeRTree(group->bcell->auxCellRTree);
    free(group->bcell);

    free(group);
    return;
}