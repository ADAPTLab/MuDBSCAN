/*

Algorithm: Micro-cluster based DBSCAN
Author: Aditya Sarma
email: asaditya1195@gmail.com

*/

#include "Def.h"
#include "vectorc.h"
#include <mpi.h>
#include "Data.h"
#include "MuC_RTree.h"
#include "MuC.h"
#include "Data.h"
#include <fstream>
#include <string>
#include <iostream>
#include <sstream>
#include <typeinfo>
#include <vector>
#include <map>
#include<string.h> 

#include "partition.h"
#include "clustering.h"

using namespace std;

char debugFileName[50];
char neighbourFileName[50];

double EPS;
int MINPOINTS;
int DIMENSION;    

int GMINENTRIES;  
int GMAXENTRIES;
int GAUXMINENTRIES;
int GAUXMAXENTRIES; 
int RMINENTRIES;        
int RMAXENTRIES;  

int CLUSTERID;
int GROUPID;
int BCELLID;

int* addHelper;
double* addHelperDouble;

vectorc* groupList;
int** noiseNeighbours;
int* visited;

int noOfGroups;
vectorc* reachableGroupCount;

int maxReachableGroupCount;
int findNeighbourCount;
int avgReachableGroupCount;
int savedQueries;
int totalCore;

vectorc* unprocessedCore;
double * MINGRIDSIZE;
double * MAXGRIDSIZE;
double * MINGRIDSIZEglobal, *MAXGRIDSIZEglobal;
FILE* output;

double start, end, end_time, filereadstart, filereadend, treestart, treeend, reachablegroupstart, reachablegroupend, clusteringstart, clusteringend, postprocessingend, postprocessingstart;
double partitionstart, partitionend, dataliststart, datalistend, mergestart, mergeend;


namespace patch
{
	template < typename T > std::string to_string( const T& n )
	{
		std::ostringstream stm ;
		stm << n ;
		return stm.str() ;
	}
}

int populateSingle(DataHdr* dataList, char* argv, int* totalSize, int* dims)
{

	vector< vector<double> > objects;

	int localSize = fileReadSingle(argv, totalSize, dims, objects);

	*dataList = initDataHdr(objects.size());

	populateDataListLocal(*dataList, objects);

	return localSize;

}

void createDebugCSV(DataHdr dataList, int rank)
{
	FILE* debug1;
	string currFile1("");

	currFile1 = debugFileName + patch::to_string(rank);

	debug1 = fopen(currFile1.c_str(), "w");

	printf("Before Remote Points Procedure %d \n",rank);
	for(int i = 0; i < dataList->localCnt; i++)
	{
		Data datai = dataList->dataClstElem + i;
		if(debug1!=NULL){
			fprintf(debug1, "%lf\t%lf\t%lf\t%d\n",datai->iData[0],datai->iData[1],datai->iData[2],(int)datai->core_tag); 

		}
	}
}

int populateMulti(DataHdr* dataList, char* argv, int* totalSize, int* dims)
{

	int myrank, numprocs;
 	MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &myrank);	vector< vector<double> > objects;
    
    filereadstart = MPI_Wtime();
	int localSize = fileReadMulti(argv, totalSize, dims, objects);
	filereadend = MPI_Wtime();

	MPI_Barrier(MPI_COMM_WORLD);
	vector< vector<double> > remote_objects;

	vectorc* remote_PrIDs;
	remote_PrIDs = (vectorc*)malloc(sizeof(struct vectorc));
	// assert(remote_PrIDs != NULL);
	VECTOR_INIT(remote_PrIDs, INTEGER);

	vectorc* remote_Indices;
	remote_Indices = (vectorc*)malloc(sizeof(struct vectorc));
	// assert(remote_Indices != NULL);
	VECTOR_INIT(remote_Indices, INTEGER);

	int remote_number = 0;

	int i;

	get_extra_points(objects, &localSize, remote_objects, remote_PrIDs, remote_Indices, &remote_number);

	MPI_Barrier(MPI_COMM_WORLD);

	int rs_size = remote_objects.size();

	dataliststart = MPI_Wtime();
	*dataList = initDataHdr(localSize + remote_number);
	populateDataListLocal(*dataList, objects);
	int old = (*dataList)->uiCnt;
	populateDataListRemote(*dataList, remote_objects, remote_PrIDs, remote_Indices);
	datalistend = MPI_Wtime();

    VECTOR_FREE(remote_PrIDs);
    free(remote_PrIDs);

    VECTOR_FREE(remote_Indices);
    free(remote_Indices);

	return localSize + remote_number;
}

int main(int argc, char **argv)
{

	if(argc != 7)
	{
		fprintf(stderr, "Usage: ./<output> <InputPath> <Epsilon> <MINPOINTS> <MinEntries> <MaxEntries> <OutputFile>\n");
        return -1;
	}

	//strcpy(debugFileName,argv[7]);
	//strcpy(neighbourFileName,argv[8]);

	MPI_Init(&argc, &argv);
	start = MPI_Wtime();
	
	int myrank, numprocs, i, j;

 	MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &myrank);

   	if(myrank == proc_of_interest) fprintf(stderr, "Number of processes %d\n", numprocs);

   	unsigned int proc_count = numprocs;

 	while (((proc_count % 2) == 0) && proc_count > 1)
   		proc_count /= 2;
 	
	if(proc_count != 1)
	{
		if(myrank == proc_of_interest) fprintf(stderr, "\n\nPlease use the number of process cores as a multiple of TWO\n");
		MPI_Finalize();
		return 0;
	}

	struct stat st;
	int fileExists = stat(argv[1], &st);

	if(fileExists != 0)
    {
        fprintf(stderr, "Input file/folder doesn't exist.\n");
		exit(-1);
	}

	EPS = strtod(argv[2], NULL);
	MINPOINTS = atoi(argv[3]);

   	if(myrank == proc_of_interest) fprintf(stderr, "Epsilon: %lf  MinPts: %d\n", EPS, MINPOINTS);

	GMINENTRIES = atoi(argv[4]);  
	GMAXENTRIES = atoi(argv[5]);

	GAUXMINENTRIES = GMINENTRIES;
	GAUXMAXENTRIES = GMAXENTRIES; 

	RMINENTRIES = GMINENTRIES;        
	RMAXENTRIES = GMAXENTRIES;

	addHelper = NULL;
	addHelper = (int*)calloc(1, sizeof(int));
	// assert(addHelper != NULL);

    addHelperDouble = (double*)calloc(1, sizeof(double));
    // assert(addHelperDouble != NULL);

	#ifdef DEBUG
	if(myrank == 0)
	{
		printf("Input File: \t%s\n", argv[1]);
		printf("Epsilon: \t%lf\n", EPS);
		printf("Minpts: \t%d\n", MINPOINTS);
		printf("Number of processes: \t%d\n", numprocs);
	}
	#endif

	int totalSize, dims;
	int localSize;

	totalCore = 0;
    findNeighbourCount = 0;
    savedQueries = 0;

    maxReachableGroupCount = 0;
	avgReachableGroupCount = 0;

	GHdrNd MuCRTree = NULL;
    DataHdr dataList = NULL;
    int sequentialflag = 0;

    if(numprocs == 1)sequentialflag = 1;
	output = fopen(argv[6], "w");
	//cout<<"Debug After populateMulti\n";
	if(sequentialflag)
	{
		filereadstart = MPI_Wtime();
		localSize = populateSingle(&dataList, argv[1], &totalSize, &dims);
		filereadend = MPI_Wtime();

		visited = NULL;
		visited = (int*) calloc(dataList->uiCnt, sizeof(int));

		// assert(visited != NULL);

		for(i=0;i<dataList->uiCnt;i++)
		{
			visited[i] = 0;
		}

		unprocessedCore = (vectorc*)malloc(sizeof(struct vectorc));
		// assert(unprocessedCore != NULL);
		VECTOR_INIT(unprocessedCore, INTEGER);

		groupList = NULL;
	    groupList = (vectorc*)malloc(sizeof(struct vectorc));
	    // assert(groupList != NULL);
	    VECTOR_INIT(groupList, GROUP);

		treestart = MPI_Wtime();		
		MuCRTree = populateMuCRTree(dataList, GMINENTRIES, GMAXENTRIES);

		populateAuxRTrees(dataList, groupList);
		treeend = MPI_Wtime();

		int total = 0;

	    reachablegroupstart =  MPI_Wtime();

		for(i = 0;i<VECTOR_TOTAL(groupList);i++)
		{
			Group g = groupList->groupItems[i];
			findReachableGroupsofGroupG(dataList, MuCRTree, i);
			processGroupSequential(dataList, g);
			maxReachableGroupCount = max(maxReachableGroupCount, VECTOR_TOTAL(g->reachable_groups));
			total = total + VECTOR_TOTAL(g->reachable_groups);
		}
		reachablegroupend = MPI_Wtime();

		avgReachableGroupCount = total*1.0/VECTOR_TOTAL(groupList);
		fflush(output);
		sequentialClustering(dataList);

		savedQueries = dataList->uiCnt - findNeighbourCount;
		double end = MPI_Wtime();
		
		struct result* r = get_clusters_sequential(dataList);
		
		fprintf(output, "Filename: %s\t Eps: %lf\t Minpts: %d\n\n", argv[1], EPS, MINPOINTS);
		fprintf(output, "Clusters: %d\n", r->clusters); 
		fprintf(output, "Noise : %d\n", r->noise); 
		fprintf(output, "TotaoCore: %d\n", r->corepoints); 
		fprintf(output, "\n\n"); 

		fprintf(output, "[MPI_Wtime] File Read: %lf s \n", filereadend - filereadstart);
		fprintf(output, "[MPI_Wtime] Tree Construction: %lf s \n", treeend - treestart);
		fprintf(output, "[MPI_Wtime] Reachable Groups: %lf s \n", reachablegroupend - reachablegroupstart);
		fprintf(output, "[MPI_Wtime] Clustering [Without Post Processing]: %lf s \n", clusteringend - clusteringstart);
		fprintf(output, "[MPI_Wtime] Clustering [Post Processing only]: %lf s \n", postprocessingend - postprocessingstart);
		fprintf(output, "[MPI_Wtime] Total [Including File IO]: %lf s \n", end - start);
		fprintf(output, "[MPI_Wtime] Total [Wihtout File IO]: %lf s \n", end - filereadend);

		fprintf(output, "\n\nSTATS:\n");
		fprintf(output, "GroupList Size: %d\n", VECTOR_TOTAL(groupList)); 
		fprintf(output, "Post Process Queries: %d\n", VECTOR_TOTAL(unprocessedCore)); 
		fprintf(output, "findNeighbourQueries: %d\n", findNeighbourCount);

		fprintf(output, "savedQueries(findNeighbours): %lf\n", (savedQueries*100.0)/totalSize);
		fprintf(output, "savedQueries(TotalSaves: %lf\n", (findNeighbourCount+VECTOR_TOTAL(groupList)+VECTOR_TOTAL(unprocessedCore))*100.0/totalSize);
		fprintf(output, "avgReachableGroupCount: %d\n", avgReachableGroupCount); 
		fflush(output);

		fprintf(output, "Noise : %d\n", r->noise); 
		fprintf(output, "Clusters: %d\n", r->clusters); 
		fprintf(output, "totalCore: %d\n\n", r->corepoints);
		
		fprintf(output, "findNeighbourQueries: %d\n", findNeighbourCount);
		fprintf(output, "Reachable group queries: %d\n", VECTOR_TOTAL(groupList));
		fprintf(output, "Post Process Queries: %d\n", VECTOR_TOTAL(unprocessedCore)); 
		fprintf(output, "savedQueries(findNeighbours): %lf\n", (savedQueries*100.0)/totalSize);
		fprintf(output, "savedQueries(TotalSaves: %lf\n", (findNeighbourCount+VECTOR_TOTAL(groupList)+VECTOR_TOTAL(unprocessedCore))*100.0/totalSize);
		fprintf(output, "avgReachableGroupCount: %d\n", avgReachableGroupCount); 

		totalCore = 0;
		int noise = 0;
		free(r);
		Data data;
		for(i=0;i<dataList->uiCnt;i++)
		{
			data = dataList->dataClstElem + i;
			free(data->iData);
		}

		free(dataList->dataClstElem);
		free(dataList);
		int groupcore = 0;
		Group g;
		for(i=0;i<VECTOR_TOTAL(groupList);i++)
		{

			g = groupList->groupItems[i];
			groupcore = groupcore + VECTOR_TOTAL(g->corepoints);
			VECTOR_FREE(g->inner_points);
			VECTOR_FREE(g->total_points);
			VECTOR_FREE(g->reachable_groups);
			VECTOR_FREE(g->corepoints);

			free(g->inner_points);
			free(g->total_points);
			free(g->reachable_groups);
			free(g->corepoints);

			freeRTree(g->bcell->auxCellRTree);
			free(g->bcell->minOriginalBoundary);
		  	free(g->bcell->maxOriginalBoundary);
		  	free(g->bcell->minActualBoundary);
		  	free(g->bcell->maxActualBoundary);
		  	free(g->bcell);
			free(g);
		}

		freeGRTree(MuCRTree);

		free(visited);

	    VECTOR_FREE(unprocessedCore);
	    free(unprocessedCore);

	    VECTOR_FREE(groupList);
	    free(groupList);

	    free(addHelper);
	    free(addHelperDouble);

	    free(MINGRIDSIZEglobal);
	    free(MINGRIDSIZE);
	    free(MAXGRIDSIZEglobal);
	    free(MAXGRIDSIZE);
	}
	else
	{
		int xx = populateMulti(&dataList, argv[1], &totalSize, &dims);

		if(dataList->uiCnt != xx)
		{
			fprintf(stderr, "Unexpected situation! Issue in datastructure transformation!\n");
			exit(-1);
		}
		
		visited = (int*)calloc(dataList->uiCnt, sizeof(int));
		// assert(visited != NULL);
		for(i=0; i<dataList->uiCnt; i++) visited[i] = 0;

		unprocessedCore = NULL;
		unprocessedCore =(vectorc*)malloc(sizeof(struct vectorc)); //vector of integers

		// assert(unprocessedCore != NULL);
		VECTOR_INIT(unprocessedCore, INTEGER);

		groupList = NULL;
	    groupList = (vectorc*)malloc(sizeof(struct vectorc)); // vector of groups

	    // assert(groupList != NULL);
	    VECTOR_INIT(groupList, GROUP);

		treestart = MPI_Wtime();		
		MuCRTree = populateMuCRTree(dataList, GMINENTRIES, GMAXENTRIES);

		totalCore = 0;

		populateAuxRTrees(dataList, groupList);

		treeend = MPI_Wtime();

		vector < vector <int> > merge_received;
		vector < vector <int> > merge_send1;
		vector < vector <int> > merge_send2;
		vector <int> init;

		vector < vector <int> >* pswap;
		vector < vector <int> >* p_cur_send;
		vector < vector <int> >* p_cur_insert;

		merge_received.resize(numprocs, init);
		merge_send1.resize(numprocs, init);
		merge_send2.resize(numprocs, init);

		p_cur_send = &merge_send1;
		p_cur_insert = &merge_send2;

		//cout<<dataList->localCnt<<"\n";
		for(i = 0; i < numprocs; i++)
		{
			if(i != myrank)
			{
				merge_received[i].reserve(dataList->localCnt);
				merge_send1[i].reserve(dataList->localCnt);
				merge_send2[i].reserve(dataList->localCnt);
			}
		}	

		reachablegroupstart = MPI_Wtime();
		totalCore = 0;

		for(i = 0;i<VECTOR_TOTAL(groupList);i++)
		{
			findReachableGroupsofGroupG(dataList, MuCRTree, i);
		}

		Group g;
		for(i = 0;i<VECTOR_TOTAL(groupList);i++)
		{
			g = groupList->groupItems[i];
			processGroup(dataList, i, p_cur_insert);
		}

		reachablegroupend = MPI_Wtime();
		//I think it is Local Union Find

		run_dbscan_algo_uf_mpi_interleaved(dataList, merge_received, merge_send1, merge_send2, init, pswap, p_cur_send, p_cur_insert);

		double collectClusters =  MPI_Wtime();

		//I think it is global Union Find
		struct result* r = get_clusters_distributed(dataList);

		savedQueries = dataList->localCnt - findNeighbourCount;

		int global_sum = 0;
		MPI_Reduce(&findNeighbourCount, &global_sum, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
		
		int globalCore = 0;
		MPI_Reduce(&totalCore, &globalCore, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

		int totalSavedQueries = 0;
		MPI_Reduce(&savedQueries, &totalSavedQueries, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
		int totalNoise = 0;

		MPI_Reduce(&(r->noise), &totalNoise, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

		int groupSize = VECTOR_TOTAL(groupList);
		int totalGroupSize = 0;
		MPI_Reduce(&groupSize, &totalGroupSize, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

		int postCore = VECTOR_TOTAL(unprocessedCore);
		int totalPostCore = 0;
		MPI_Reduce(&postCore, &totalPostCore, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
		
		int totalPoints = 0;
		int localPoints = dataList->localCnt;
		MPI_Reduce(&localPoints, &totalPoints, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

		MPI_Barrier(MPI_COMM_WORLD);
		end_time = MPI_Wtime();

		if(myrank == 0)
		{
			fprintf(output, "Filename: %s\t Eps: %lf\t Minpts: %d\t Nodes: %d\n\n", argv[1], EPS, MINPOINTS, numprocs);

            fprintf(output, "Clusters: %d\n", r->clusters - totalNoise);
            fprintf(output, "Noise : %d\n", totalNoise);
            fprintf(output, "totalCore: %d\n\n", globalCore);

            fprintf(output, "RANK: %d\t [MPI_Wtime] File Read: %lf s\n", myrank, filereadend - filereadstart);

			fprintf(output, "RANK: %d\t [MPI_Wtime] Tree Construction: %lf s \n", myrank, treeend - treestart);
            fprintf(output, "RANK: %d\t [MPI_Wtime] Reachable Groups: %lf s \n", myrank, reachablegroupend - reachablegroupstart);
            fprintf(output, "RANK: %d\t [MPI_Wtime] Clustering [Without Post Processing]: %lf s \n", myrank, clusteringend - clusteringstart);

            fprintf(output, "RANK: %d\t [MPI_Wtime] Clustering [Post Processing only]: %lf s \n", myrank, postprocessingend - postprocessingstart);
            fprintf(output, "RANK: %d\t [MPI_Wtime] Clustering [Merging only]: %lf s \n", myrank, mergeend - mergestart);

            fprintf(output, "RANK: %d\t [MPI_Wtime] Total [Including File IO]: %lf s \n", myrank, end_time - start);
            fprintf(output, "[MPI_Wtime] Total [Wihtout File IO]: %lf s \n", end_time - filereadend - (datalistend - dataliststart));

			fprintf(output, "\n\nSTATS:\n");
			fprintf(output, "findNeighbourQueries: %d\n", global_sum);
			fprintf(output, "Reachable group queries: %d\n", totalGroupSize);
			fprintf(output, "Post Process Queries: %d\n", totalPostCore); 
			fprintf(output, "Total Saved Queries: %d\n", totalSavedQueries); 
			fprintf(output, "Total Points: %d\n", totalPoints); 

			fprintf(output, "savedQueries(findNeighbours): %lf percent\n",(double) (totalSavedQueries*100.0)/(double)totalPoints);
			fprintf(output, "savedQueries(TotalSaves: %lf percent\n", (global_sum+totalGroupSize+totalPostCore)*100.0/totalPoints);

			fflush(stderr);
		
			/*fprintf(stderr, "Clusters: %d\n", r->clusters); 
			fprintf(stderr, "Noise : %d\n", totalNoise); 
			fprintf(stderr, "totalCore: %d\n\n", globalCore);
			fprintf(stderr, "findNeighbourQueries: %d\n", global_sum);
			fprintf(stderr, "Reachable group queries: %d\n", totalGroupSize);
			fprintf(stderr, "Post Process Queries: %d\n", totalPostCore); 
			fprintf(stderr, "savedQueries(findNeighbours): %lf\n", ((double)totalSavedQueries*100.0)/(double)totalSize);
			fprintf(stderr, "savedQueries(TotalSaves: %lf\n", (global_sum+totalGroupSize+totalPostCore)*100.0/totalSize);
			fflush(stderr);*/
		}
		
		/*
		FILE* debug1;
		string currFile1("");
		currFile1 = debugFileName + patch::to_string(myrank);
		debug1 = fopen(currFile1.c_str(), "w");
		for(i = 0; i < dataList->localCnt; i++)
		{
			Data datai = dataList->dataClstElem + i;
			int parent = datai->parentId;
			Data parenti = dataList->dataClstElem + parent;
			if(debug1!=NULL){
				//fprintf(debug1,"%lf\t%lf\t%lf\t%d\t%lf\t%lf\t%lf\n",datai->iData[0],datai->iData[1],datai->iData[2],(int)datai->core_tag,parenti->iData[0],parenti->iData[1],parenti->iData[2]);
			}
		}
		*/
			

		for(i=0;i<dataList->localCnt;i++)
		{
			if(noiseNeighbours[i] != NULL)free(noiseNeighbours[i]);
		}
		free(noiseNeighbours);

		Data data;
		for(i=0;i<dataList->uiCnt;i++)
		{
			data = dataList->dataClstElem + i;
			free(data->iData);
		}
		//cout<<"i = "<<i<<"\n";

		free(dataList->dataClstElem);
		free(dataList);

		//Group g;
		for(i=0;i<VECTOR_TOTAL(groupList);i++)
		{
			g = groupList->groupItems[i];
			VECTOR_FREE(g->inner_points);
			VECTOR_FREE(g->total_points);
			VECTOR_FREE(g->reachable_groups);
			VECTOR_FREE(g->corepoints);
			free(g->inner_points);
			free(g->total_points);
			free(g->reachable_groups);
			free(g->corepoints);

			free(g->bcell->minOriginalBoundary);
		  	free(g->bcell->maxOriginalBoundary);
		  	free(g->bcell->minActualBoundary);
		  	free(g->bcell->maxActualBoundary);

			free(g);
		}

		freeGRTree(MuCRTree);

		free(visited);

	    VECTOR_FREE(unprocessedCore);
	    free(unprocessedCore);


	    free(addHelper);
	    free(addHelperDouble);

	    //fclose(output_file);
	}

	fclose(output);
	
	MPI_Finalize();
	return 0;

}

