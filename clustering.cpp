/*
Algorithm: Micro-cluster based DBSCAN
Author: Aditya Sarma
email: asaditya1195@gmail.com
*/

#include "Def.h"
#include "clustering.h"
#include <sstream>
#include <vector>
#include <iostream>
#include "MuC_RTree.h"
#include "vectorc.h"
#include <unordered_map>
#include <map>
#include <string> 
#include "Data.h"

using namespace std;

extern double postprocessingstart;
extern double postprocessingend;
extern double clusteringstart;
extern double clusteringend;
extern double mergestart;
extern double mergeend;
extern char debugFileName[50];
extern char neighbourFileName[50];

namespace patch
{
	template < typename T > std::string to_string( const T& n )
	{
		std::ostringstream stm ;
		stm << n ;
		return stm.str() ;
	}
}

void trivial_decompression(vector<int>* data, int nproc, int rank, int round, double& dcomtime)
{
	double start = MPI_Wtime();

	vector <int> parser;
	parser.reserve((*data).size());
	parser = (*data);
	(*data).clear();

	int pid_count = parser[0], pos, i, j, pid, npid, npid_count;
	
	pos++;

	while(pid_count > 0)
	{
		pid = parser[pos++];
		npid_count = parser[pos++];

		for(j = 0; j < npid_count; j++)
		{
			(*data).push_back(pid);
			(*data).push_back(parser[pos++]);
		}

		pid_count--;
	}

	parser.clear();

	double stop = MPI_Wtime();

	dcomtime += (stop - start);
}

void trivial_compression(DataHdr dataList, vector <int>* data, vector < vector <int> >* parser, int nproc, int rank, int round, double& comtime, double& sum_comp_rate)
{
	double start = MPI_Wtime();
	double org = 0, comp = 0;
	int pairs, pid, npid, i, j, pid_count, npid_count;

	pairs = (*data).size()/2;

	org = (*data).size();		

	for(i = 0; i < pairs; i++)
	{
		pid = (*data)[2 * i];
		npid = (*data)[2 * i + 1];

		(*parser)[pid].push_back(npid);
	}

	(*data).clear();

	pid_count = 0;
	(*data).push_back(pid_count); // uniques pids, should update later
		
	for(i = 0; i < dataList->localCnt; i++)
	{
		npid_count = (*parser)[i].size();
		if(npid_count > 0)
		{
			(*data).push_back(i);
			(*data).push_back(npid_count);

			for(j = 0; j < npid_count; j++)
				(*data).push_back((*parser)[i][j]);						 
			
			pid_count++;

			(*parser)[i].clear();
		}
	}
		
	(*data)[0] = pid_count;

	comp = (*data).size();
	
	double stop = MPI_Wtime();
	comtime += (stop - start);

	sum_comp_rate += (comp / org);
	
}

void run_dbscan_algo_uf_mpi_interleaved(DataHdr dataList, vector < vector <int > >& merge_received,	vector < vector <int > >& merge_send1,
	vector < vector <int > >& merge_send2, vector <int>& init, vector < vector <int > >* pswap, vector < vector <int > >* p_cur_send, vector < vector <int > >* p_cur_insert)
{

	clusteringstart = MPI_Wtime();
	int i;

	int j, k, npid, prID;
	int rank, nproc, mpi_namelen;

	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &nproc);

	merge_received.resize(nproc, init);
	merge_send1.resize(nproc, init);
	merge_send2.resize(nproc, init);

	int pid;

    noiseNeighbours = (int**)calloc(dataList->localCnt, sizeof(int*));
    // assert(noiseNeighbours != NULL);
    
	int total_points = 0, points_per_pr[nproc], start_pos[nproc];
	MPI_Allgather(&(dataList->localCnt), 1, MPI_INT, &points_per_pr[0], 1, MPI_INT, MPI_COMM_WORLD);
	
	for(i = 0; i < nproc; i++)
	{
		start_pos[i] = total_points;
		total_points += points_per_pr[i];
	}

	vector <int> vec_prID;
	vec_prID.resize(total_points, -1);

	k = 0;
	for(i = 0; i < nproc; i++)
	{
		for(j = 0; j < points_per_pr[i]; j++)
		{
			vec_prID[k++] = i;
		}
	}

	int rtag, rsource, tag = 0, pos = 0, scount, rcount, isend[nproc], irecv[nproc];
	int root, root1, root2, tid;

	MPI_Barrier(MPI_COMM_WORLD);

	Neighbours neigh = NULL;
	Data data;
	Data dataremote;
	Data datalocal;
	for(i = 0; i < dataList->localCnt; i++)
	{
		noiseNeighbours[i] = NULL;
		pid = i;
		data = dataList->dataClstElem + i;
		
		if(data->haloPoint == TRUE || data->isProcessed == TRUE || data->core_tag == TRUE)
		{
			continue;   
		}

		neigh = findNeighbours3(dataList, i, EPS, p_cur_insert);
		data->isProcessed = TRUE;

		int total_neighbours = VECTOR_TOTAL(neigh->neighbours_local_inner) + VECTOR_TOTAL(neigh->neighbours_remote_inner) + VECTOR_TOTAL(neigh->neighbours_local_outer) + VECTOR_TOTAL(neigh->neighbours_remote_outer);

		if(total_neighbours >= MINPOINTS)
		{
			root = pid;
			data->core_tag = TRUE;
			data->ClusterID = 1;
			for(j = 0; j < VECTOR_TOTAL(neigh->neighbours_remote_outer); j++)
			{
				dataremote = dataList->dataClstElem + neigh->neighbours_remote_outer->intItems[j];
				npid = dataremote->id;
				dataremote->ClusterID = 1;
				(*p_cur_insert)[dataremote->actualProcessId].push_back(pid);
    			(*p_cur_insert)[dataremote->actualProcessId].push_back(dataremote->remoteIndex);
			}

			for(j = 0; j < VECTOR_TOTAL(neigh->neighbours_remote_inner); j++)
			{
				dataremote = dataList->dataClstElem + neigh->neighbours_remote_inner->intItems[j];
				npid = dataremote->id;
				dataremote->ClusterID = 1;
				(*p_cur_insert)[dataremote->actualProcessId].push_back(pid);
    			(*p_cur_insert)[dataremote->actualProcessId].push_back(dataremote->remoteIndex);
			}

			Data dataroot1,dataroot2;

			for (j = 0; j < VECTOR_TOTAL(neigh->neighbours_local_inner); j++)
    		{
				datalocal = dataList->dataClstElem + neigh->neighbours_local_inner->intItems[j];
				npid = datalocal->id;
				root1 = npid;
				root2 = root;

				if(datalocal->core_tag == TRUE || datalocal->ClusterID == 0)
				{
					datalocal->ClusterID = 1;

					dataroot1 = dataList->dataClstElem + root1;
					dataroot2 = dataList->dataClstElem + root2;

					// REMS algorithm to (union) merge the trees
					unionFindMerge(dataList, root1, root2, dataroot1, dataroot2);

				}
			}

			for (j = 0; j < VECTOR_TOTAL(neigh->neighbours_local_outer); j++)
    		{
				datalocal = dataList->dataClstElem + neigh->neighbours_local_outer->intItems[j];
				npid = datalocal->id;
				root1 = npid;
				root2 = root;

				if(datalocal->core_tag == TRUE || datalocal->ClusterID == 0)
				{
					datalocal->ClusterID = 1;

					Data dataroot1 = dataList->dataClstElem + root1;
					Data dataroot2 = dataList->dataClstElem + root2;

					// REMS algorithm to (union) merge the trees
					unionFindMerge(dataList, root1, root2, dataroot1, dataroot2);

				}
			}
		}
		else
		{
			data->core_tag = FALSE;
			if(data->ClusterID == 1)
			{
				;
			}
			else
			{
				data->ClusterID = 0;
				Data neighbour;
	            for(j=0; j<VECTOR_TOTAL(neigh->neighbours_local_inner); j++)
	            {
	                neighbour = dataList->dataClstElem + neigh->neighbours_local_inner->intItems[j];
	                if(neighbour->core_tag == TRUE && neighbour->haloPoint == FALSE) //Border point scenario.
	                {
	                	root1 = data->id;
						root2 = neighbour->id;

						data->ClusterID = 1;

						Data dataroot1 = dataList->dataClstElem + root1;
						Data dataroot2 = dataList->dataClstElem + root2;

						unionFindMerge(dataList, root1, root2, dataroot1, dataroot2);

	                    neighbour->ClusterID = 1;
	                    break;
	                }
	            }
	            for(j=0; j<VECTOR_TOTAL(neigh->neighbours_local_outer); j++)
	            {
	                neighbour = dataList->dataClstElem + neigh->neighbours_local_outer->intItems[j];
	                if(neighbour->core_tag == TRUE && neighbour->haloPoint == FALSE) //Border point scenario.
	                {
	                	root1 = data->id;
						root2 = neighbour->id;
						data->ClusterID = 1;
						Data dataroot1 = dataList->dataClstElem + root1;
						Data dataroot2 = dataList->dataClstElem + root2;

						unionFindMerge(dataList, root1, root2, dataroot1, dataroot2);
	                    neighbour->ClusterID = 1;
	                    break;
	                }
	            }

            	if(data->ClusterID == 0)
            	{
	                noiseNeighbours[data->id] = (int*)malloc(sizeof(int)*MINPOINTS);

	                int k = 0;
	            	for(k=0;k<MINPOINTS;k++) noiseNeighbours[data->id][k] = -1;
	            	k=0;

	                for(j=0; j<VECTOR_TOTAL(neigh->neighbours_local_inner); j++)
	                {
                       	noiseNeighbours[data->id][k++] = neigh->neighbours_local_inner->intItems[j];
	                }
	                for(j=0; j<VECTOR_TOTAL(neigh->neighbours_local_outer); j++)
	                {
                       	noiseNeighbours[data->id][k++] = neigh->neighbours_local_outer->intItems[j];
	                }
	            }
			}
		}

		VECTOR_FREE(neigh->neighbours_local_outer);
		free(neigh->neighbours_local_outer);

		VECTOR_FREE(neigh->neighbours_remote_outer);
		free(neigh->neighbours_remote_outer);

		VECTOR_FREE(neigh->neighbours_local_inner);
		free(neigh->neighbours_local_inner);

		VECTOR_FREE(neigh->neighbours_remote_inner);
		free(neigh->neighbours_remote_inner);

		free(neigh);
		neigh = NULL;
	}



	//Remote core local core merging happens here only
	//Post Process Remote points

	for(i=dataList->localCnt-1;i<dataList->uiCnt;i++)
	{
		pid = i;
		Data data = dataList->dataClstElem + i;
		if(data->haloPoint == TRUE)
		{
			Group g = groupList->groupItems[data->group_id];
			Region region = createCellRegOfPoint(data, EPS);
			RHdrNd hdrNdTree = g->bcell->auxCellRTree;
			
			// if(data->ClusterID == 0)
			// {
			for(k=0; k<VECTOR_TOTAL(g->corepoints); k++)
			{
				Data data2 = dataList->dataClstElem + g->corepoints->intItems[k];
				if(data2->haloPoint == FALSE)
				{
					if(RisContains(region, data2->iData))
				    {
					    if(distance(data->iData, data2->iData) <= EPS)
					    {
					    	Data dataremote = data;
					    	data->ClusterID = 1;
							(*p_cur_insert)[dataremote->actualProcessId].push_back(data2->id);
			    			(*p_cur_insert)[dataremote->actualProcessId].push_back(dataremote->remoteIndex);
			    			break;
					    }
					}
				}
			}
			
			Group rg;
			DataPoint d;
    		for(j=0; j<VECTOR_TOTAL(g->reachable_groups); j++)
			{
				rg = groupList->groupItems[g->reachable_groups->intItems[j]];
				d = (dataList->dataClstElem + rg->master_id)->iData;

				Region potentialRegion = createCellRegOfDataPoint(d, EPS);
				if(GisOverLap(region, potentialRegion))
				{
					RHdrNd hdrNdTree = rg->bcell->auxCellRTree;
					for(k=0; k<VECTOR_TOTAL(rg->corepoints); k++)
					{
					    Data data2 = dataList->dataClstElem + rg->corepoints->intItems[k];
					    if(data2->haloPoint == FALSE)
					    {
					    	if(RisContains(region, data2->iData))
					    	{
					    		if(distance(data->iData, data2->iData) <= EPS)
					            {
					            	Data dataremote = data;
							    	data->ClusterID = 1;
									(*p_cur_insert)[dataremote->actualProcessId].push_back(data2->id);
					    			(*p_cur_insert)[dataremote->actualProcessId].push_back(dataremote->remoteIndex);
					    			break;
					            }
					    	}
					    }
					}
				}
				free(potentialRegion->iBottomLeft);
				free(potentialRegion->iTopRight);
				free(potentialRegion);

	    	}    		
			free(region->iBottomLeft);
			free(region->iTopRight);
			free(region);
		}
	}

	clusteringend = MPI_Wtime();
	postprocessingstart = MPI_Wtime();

	Data potentialNoise;
	for(i=0; i<dataList->localCnt; i++)
	{
	    potentialNoise = dataList->dataClstElem + i;
	    if(potentialNoise->ClusterID == 0 && noiseNeighbours[i] != NULL)
	    {
	    	int v;
	    	Data data;
		    for(v=0; v<MINPOINTS; v++)
		    {
		        if(noiseNeighbours[i][v] >= 0)
		        {
		            data = dataList->dataClstElem + noiseNeighbours[i][v];
		            if(data->core_tag == TRUE && data->haloPoint == FALSE)
		            {
		            	root1 = data->id;
						root2 = potentialNoise->id;

						Data dataroot1 = dataList->dataClstElem + root1;
						Data dataroot2 = dataList->dataClstElem + root2;
						unionFindMerge(dataList, root1, root2, dataroot1, dataroot2);

		                potentialNoise->ClusterID = 1;
		                break;
		            }
		        }
		    }
		}
	}

	// path compression.
	int temp = 0;
	Data dataroot;
	for(temp = 0 ; temp < dataList->localCnt; temp++)
	{
	 	dataroot = dataList->dataClstElem + temp;
	 	Data datastart = dataroot;

	 	while(dataroot->id != dataroot->parentId)
	 	{
	 		dataroot = dataList->dataClstElem + dataroot->parentId;
	 	}

	 	int finalroot = dataroot->parentId;

	 	while(datastart->id != finalroot)
	 	{
	 		int xx = datastart->parentId;
	 		datastart->parentId = finalroot;
	 		datastart = dataList->dataClstElem + xx;
	 	}
	}

	// Only local core local core merging
	Group g;
	//Data data;
	for(i=0; i<VECTOR_TOTAL(unprocessedCore); i++)
	{
		data = dataList->dataClstElem + unprocessedCore->intItems[i];

		if(data->haloPoint == FALSE)
		{
			g = groupList->groupItems[data->group_id];
			Region region = createCellRegOfPoint(data, EPS);
		
			vectorc* neighbours_local_outer = (vectorc*)malloc(sizeof(struct vectorc));
			VECTOR_INIT(neighbours_local_outer, INTEGER);

			vectorc* neighbours_remote_outer = (vectorc*)malloc(sizeof(struct vectorc));
			VECTOR_INIT(neighbours_remote_outer, INTEGER);

			Neighbours n = (Neighbours)malloc(sizeof(struct neighbours));

			n->neighbours_local_outer = neighbours_local_outer;
			n->neighbours_remote_outer = neighbours_remote_outer;

			RHdrNd hdrNdTree = g->bcell->auxCellRTree;
			
			Data master = dataList->dataClstElem + g->master_id;

			// if(master->haloPoint == TRUE && (master->core_tag == TRUE || master->ClusterID == 0))
			if(master->haloPoint == TRUE && (master->core_tag == TRUE))
			{
		    	Data dataremote = master;
		    	data->ClusterID = 1;
				(*p_cur_insert)[dataremote->actualProcessId].push_back(data->id);
    			(*p_cur_insert)[dataremote->actualProcessId].push_back(dataremote->remoteIndex);
			}

			Data data2;	
			for(k=0; k<VECTOR_TOTAL(g->corepoints); k++)
			{
				data2 = dataList->dataClstElem + g->corepoints->intItems[k];
				if(data2->haloPoint == FALSE && data2->id != data->id && (data2->parentId != data->parentId || data2->parentProcessId != data->parentProcessId))
				{
				    if(RisContains(region, data2->iData))
				    {
				    	if(distance(data->iData, data2->iData) <= EPS)
				    	{
					    	addHelper[0] = data2->id;
							VECTOR_ADD(n->neighbours_local_outer, addHelper);
				    	}
				    }
				}
			}
	    	Group rg;
			for(j=0; j<VECTOR_TOTAL(g->reachable_groups); j++)
			{
				rg = groupList->groupItems[g->reachable_groups->intItems[j]];
				DataPoint d = (dataList->dataClstElem + rg->master_id)->iData;

				Region potentialRegion = createCellRegOfDataPoint(d, EPS);
				
				if(GisOverLap(region, potentialRegion))
				{
					RHdrNd hdrNdTree = rg->bcell->auxCellRTree;
					for(k=0; k<VECTOR_TOTAL(rg->corepoints); k++)
					{
					    Data data2 = dataList->dataClstElem + rg->corepoints->intItems[k];
					    if(data2->haloPoint == FALSE && (data2->parentId != data->parentId || data2->parentProcessId != data->parentProcessId))
					    {
					        if(RisContains(region, data2->iData))
				            {
				                if(distance(data->iData, data2->iData) <= EPS)
						    	{
							    	addHelper[0] = data2->id;
									VECTOR_ADD(n->neighbours_local_outer, addHelper);
						    	}
				            }
					    }
					}
				}

				free(potentialRegion->iBottomLeft);
				free(potentialRegion->iTopRight);
				free(potentialRegion);
			}
			Data temp;
			for(j=0;j<VECTOR_TOTAL(n->neighbours_local_outer);j++)
			{
				temp = dataList->dataClstElem + n->neighbours_local_outer->intItems[j];

				int root1 = temp->id;
				int root2 = data->id;
				int root;
				Data dataroot1 = dataList->dataClstElem + root1;
				Data dataroot2 = dataList->dataClstElem + root2;

				unionFindMerge(dataList, root1, root2, dataroot1, dataroot2);
			}

			VECTOR_FREE(n->neighbours_remote_outer);
			VECTOR_FREE(n->neighbours_local_outer);
			free(n->neighbours_local_outer);
			free(n->neighbours_remote_outer);
			free(n);

			free(region->iBottomLeft);
			free(region->iTopRight);
			free(region);
		}
	}

	// Only local core local core merging
	MPI_Barrier(MPI_COMM_WORLD);
	int v1, v2, par_proc, triples, local_count, global_count;
	double temp_inter_med, inter_med, stop = MPI_Wtime();

	inter_med = MPI_Wtime();

	i = 0;
		
	MPI_Request s_req_recv[nproc], s_req_send[nproc], d_req_send[nproc], d_req_recv[nproc]; // better to malloc the memory
	MPI_Status  s_stat, d_stat_send[nproc], d_stat;

	local_count = 0;

	// path compression
	for(tid = 0; tid < nproc; tid++)
	{
		triples = (*p_cur_insert)[tid].size()/2;
		local_count += triples;
		int root1, v1;

		for(pid = 0; pid < triples; pid++)
		{
			v1 = (*p_cur_insert)[tid][2 * pid];
			root1 = v1;
			Data dataroot1 = dataList->dataClstElem + root1;
			Data datav1 = dataList->dataClstElem + v1;

			while(dataroot1->parentId != root1)
			{
       			root1 = dataroot1->parentId;
       			dataroot1 = dataList->dataClstElem + root1;
			}

			while(datav1->parentId != root1)
	    	{
				int tmp = datav1->parentId;
				datav1->parentId = root1;
	        	v1 = tmp;
	        	Data datav1 = dataList->dataClstElem + v1;
			}
			(*p_cur_insert)[tid][2 * pid] = root1;
		}
	}

	local_count = local_count/nproc;
    	
	global_count = 0;
    MPI_Allreduce(&local_count, &global_count, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);	

	int uv, uf, um, ul, ucount;
	int local_continue_to_run, global_continue_to_run;
	double dcomtime = 0, comtime = 0, sum_comp_rate = 0;
	vector <vector <int> > parser;
	vector <int> init_ex;
	parser.resize(dataList->localCnt, init_ex);

	postprocessingend = MPI_Wtime();

	mergestart = MPI_Wtime();
	while(1)
	{
		pswap = p_cur_insert;
		p_cur_insert = p_cur_send;  
		p_cur_send = pswap;

		for(tid = 0; tid < nproc; tid++)
    		(*p_cur_insert)[tid].clear();

		scount = 0;
		for(tid = 0; tid < nproc; tid++)
		{
			isend[tid] = (*p_cur_send)[tid].size();

			if(isend[tid] > 0)
	    	{
        		MPI_Isend(&(*p_cur_send)[tid][0], isend[tid], MPI_INT, tid, tag + 1, MPI_COMM_WORLD, &d_req_send[scount]);
        		scount++;
	    	}
		}

		MPI_Alltoall(&isend[0], 1, MPI_INT, &irecv[0], 1, MPI_INT, MPI_COMM_WORLD);

		rcount = 0;

		for(tid = 0; tid < nproc; tid++)
		{
			if(irecv[tid] > 0)
			{
				merge_received[tid].clear();
				merge_received[tid].assign(irecv[tid], -1);
				MPI_Irecv(&merge_received[tid][0], irecv[tid], MPI_INT, tid, tag + 1, MPI_COMM_WORLD, &d_req_recv[rcount]);
					rcount++;
			}
		}

		local_count = 0;
		
		//get the data and process them
		for(tid = 0; tid < rcount; tid++)
		{
			MPI_Waitany(rcount, &d_req_recv[0], &pos, &d_stat);
		
			rtag = d_stat.MPI_TAG;
			rsource = d_stat.MPI_SOURCE;

			if(rtag == tag + 1)	
			{
				// process received the data now
				if(i == 0)
				{
					
					triples = merge_received[rsource].size()/2;
					par_proc = rsource;
					
				}						
				else
					triples = merge_received[rsource].size()/3;

				for(pid = 0; pid < triples; pid++)
				{
					// get the pair
					v1 = merge_received[rsource].back(); //local index of the point
					merge_received[rsource].pop_back();

					Data datav1;
					Data datav2;
					if((i > 0))
					{
						par_proc = merge_received[rsource].back();  //parent process id
						merge_received[rsource].pop_back();
					}

					v2 = merge_received[rsource].back(); //id of parent which belongs to another node
					datav2 = dataList->dataClstElem + v2;

					merge_received[rsource].pop_back();
					datav1 = dataList->dataClstElem + v1;

					int con = 0;
					
					if(i > 0)
						con = 1;
					else if (i == 0 && (datav1->core_tag == TRUE || datav1->ClusterID == 0))
					{	
						datav1->ClusterID = 1;
						con = 1;
					}
					if(con == 1)
					{
						root1 = v1;
						Data dataroot1 = dataList->dataClstElem + root1;

						// this will find the boundary vertex or the root if the root is in this processor
						while(dataroot1->parentProcessId == rank)
						{
							if(dataroot1->parentId == root1)
								break;

							root1 = dataroot1->parentId;
							dataroot1 = dataList->dataClstElem + root1;
						}
			
						// compress the local path
						while(v1 != root1 && vec_prID[v1] == rank)
						{
							int tmp = datav1->parentId;
							datav1->parentId = root1;
							v1 = tmp;
							datav1 = dataList->dataClstElem + v1;
						}

						if(dataroot1->parentId == v2 && dataroot1->parentProcessId == par_proc)
						{
							continue;
						}						
							
						if(par_proc == rank)
						{
							if(dataroot1->parentId == datav2->parentId)
								continue;
						}
							
						if(dataroot1->parentId == root1 && dataroot1->parentProcessId == rank) // root1 is a local root
						{
							if(start_pos[rank] + root1 < start_pos[par_proc] + v2)
							{
								dataroot1->parentId = v2;
								dataroot1->parentProcessId = par_proc;
								continue;
							}
							else
							{
								(*p_cur_insert)[par_proc].push_back(root1);
								(*p_cur_insert)[par_proc].push_back(dataroot1->parentProcessId);
								(*p_cur_insert)[par_proc].push_back(v2);

								local_count++;
							}
						}
						else
						{
							// root1 is not local
							if(start_pos[dataroot1->parentProcessId] + root1 < start_pos[par_proc] + v2)
							{
   	    						(*p_cur_insert)[dataroot1->parentProcessId].push_back(v2);
								(*p_cur_insert)[dataroot1->parentProcessId].push_back(par_proc);
								(*p_cur_insert)[dataroot1->parentProcessId].push_back(dataroot1->parentId);
								local_count++;
							}
							else
							{
								// ask the parent of v2
	            				(*p_cur_insert)[par_proc].push_back(dataroot1->parentId);
                				(*p_cur_insert)[par_proc].push_back(dataroot1->parentProcessId);
                				(*p_cur_insert)[par_proc].push_back(v2);

								local_count++;
							}
						}
					}
				}
				merge_received[rsource].clear();
			}
			else
			{
				cout << "rank " << rank << " SOMETHING IS WRONG" << endl;
			}
		}
			
		if(scount > 0)
			MPI_Waitall(scount, &d_req_send[0], &d_stat_send[0]);	

		tag += 2; // change the tag value although not important
		
		local_continue_to_run = 0;
		local_count = 0;
		for(tid = 0; tid < nproc; tid++)
		{
			local_count += (*p_cur_insert)[tid].size()/3;

			if((*p_cur_insert)[tid].size() > 0)
				local_continue_to_run = 1;
		}
		
		local_count = local_count / nproc;

		global_count = 0;
		global_continue_to_run = 0;
        
		MPI_Allreduce(&local_continue_to_run, &global_continue_to_run, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
		
		if(global_continue_to_run == 0)
        	break;
		
		i++;
	
	}

	mergeend = MPI_Wtime();

	stop = MPI_Wtime(); 

	pswap = NULL;
	p_cur_insert = NULL;
	p_cur_send = NULL;

	for(tid = 0; tid < nproc; tid++)
	{
		merge_received[tid].clear();
		merge_send1[tid].clear();
		merge_send2[tid].clear();
	}

	merge_received.clear();
	merge_send1.clear();
	merge_send2.clear();

	vec_prID.clear();
	parser.clear();
	init_ex.clear();
	init.clear();
}

struct result* get_clusters_distributed(DataHdr dataList)
{
	int rank, nproc, i;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &nproc);

	vector < vector <int > > merge_received;
	vector < vector <int > > merge_send1;
	vector < vector <int > > merge_send2;
	vector <int> init;

	merge_received.resize(nproc, init);
	merge_send1.resize(nproc, init);
	merge_send2.resize(nproc, init);

	int pid;

    for(pid = 0; pid < nproc; pid++)
	{
    	merge_received[pid].reserve(dataList->localCnt);
    	merge_send1[pid].reserve(dataList->localCnt);
		merge_send2[pid].reserve(dataList->localCnt);
	}

	vector < vector <int > >* pswap;
	vector < vector <int > >* p_cur_send;
	vector < vector <int > >* p_cur_insert;

	p_cur_send = &merge_send1;
	p_cur_insert = &merge_send2;


	int root, local_continue_to_run = 0, global_continue_to_run;
	for(i = 0; i < dataList->localCnt; i++)
	{
		// find the point containing i
		root = i;
		Data dataroot = dataList->dataClstElem + root;
		// Find highest ancestor in this node
		while(dataroot->parentProcessId == rank)
		{
			if(dataroot->parentId == root)
				break;
			root = dataroot->parentId;
			dataroot = dataList->dataClstElem + root;
		}
		
		// if the highest ancestor from the previous loop is local point
		if(dataroot->parentId == root && dataroot->parentProcessId == rank) // root is a local root
		{
			Data datai = dataList->dataClstElem + i;
			datai->parentId = root;
			dataroot->childCount = dataroot->childCount + 1;
		}
		else
		{
			(*p_cur_insert)[dataroot->parentProcessId].push_back(0); // flag: 0 means query and 1 means a reply
			(*p_cur_insert)[dataroot->parentProcessId].push_back(dataroot->parentId);
   			(*p_cur_insert)[dataroot->parentProcessId].push_back(i);
			(*p_cur_insert)[dataroot->parentProcessId].push_back(rank);
			local_continue_to_run++;
		}			
	}

	int pos, round = 0, quadraples, scount, tid, tag = 0, rtag, rsource, rcount, isend[nproc], irecv[nproc], flag;

	MPI_Request s_req_recv[nproc], s_req_send[nproc], d_req_send[nproc], d_req_recv[nproc]; // better to malloc the memory
	MPI_Status  s_stat, d_stat_send[nproc], d_stat;
	int target_point, source_point, source_pr;

	while(1)
	{
		global_continue_to_run = 0;

		MPI_Allreduce(&local_continue_to_run, &global_continue_to_run, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
		
		if(global_continue_to_run == 0)
    		break;

		pswap = p_cur_insert;
   		p_cur_insert = p_cur_send;
   		p_cur_send = pswap;

		for(tid = 0; tid < nproc; tid++)
   			(*p_cur_insert)[tid].clear();

		scount = 0;
		for(tid = 0; tid < nproc; tid++)
		{
			if(tid != rank)
			{
				isend[tid] = (*p_cur_send)[tid].size();
				if(isend[tid] > 0)
	    		{
	    			MPI_Isend(&(*p_cur_send)[tid][0], isend[tid], MPI_INT, tid, tag + 1, MPI_COMM_WORLD, &d_req_send[scount]);
	        		scount++;
	    		}	
			}
		}

		MPI_Alltoall(&isend[0], 1, MPI_INT, &irecv[0], 1, MPI_INT, MPI_COMM_WORLD);

		rcount = 0;
		for(tid = 0; tid < nproc; tid++)
		{
			if(tid != rank)
			{
				if(irecv[tid] > 0)
				{
					merge_received[tid].clear();
					merge_received[tid].assign(irecv[tid], -1);
					MPI_Irecv(&merge_received[tid][0], irecv[tid], MPI_INT, tid, tag + 1, MPI_COMM_WORLD, &d_req_recv[rcount]);
					rcount++;
				}
			}
		}

		local_continue_to_run = 0;
		Data dataroot;
		Data datatarget;
		for(tid = 0; tid < rcount; tid++)
		{
			MPI_Waitany(rcount, &d_req_recv[0], &pos, &d_stat);
		
			rtag = d_stat.MPI_TAG;
			rsource = d_stat.MPI_SOURCE;

			if(rtag == tag + 1)
			{
				quadraples = merge_received[rsource].size()/4;

				for(pid = 0; pid < quadraples; pid++)
				{
					source_pr = merge_received[rsource].back();
					merge_received[rsource].pop_back();

        			source_point = merge_received[rsource].back();
        			merge_received[rsource].pop_back();

        			target_point = merge_received[rsource].back();
        			merge_received[rsource].pop_back();
		
					flag = merge_received[rsource].back();
        			merge_received[rsource].pop_back();

					if(flag == 0)
					{					
						root = target_point;
						dataroot = dataList->dataClstElem + root;
						while(dataroot->parentProcessId == rank)
						{
							if(dataroot->parentId == root)
								break;
							root = dataroot->parentId;
							dataroot = dataList->dataClstElem + root;
						}
						if(dataroot->parentId == root && dataroot->parentProcessId == rank) // root is a local root
						{
							dataroot->childCount = dataroot->childCount + 1; // increase the child count by one
							// have to return the child about root
							
            				(*p_cur_insert)[source_pr].push_back(1);
            				(*p_cur_insert)[source_pr].push_back(source_point);
            				(*p_cur_insert)[source_pr].push_back(dataroot->parentId);
            				(*p_cur_insert)[source_pr].push_back(dataroot->parentProcessId);
							local_continue_to_run++;
						}
						else
						{
							(*p_cur_insert)[dataroot->parentProcessId].push_back(0);
							(*p_cur_insert)[dataroot->parentProcessId].push_back(dataroot->parentId);
		         			(*p_cur_insert)[dataroot->parentProcessId].push_back(source_point);
        	    			(*p_cur_insert)[dataroot->parentProcessId].push_back(source_pr);
							local_continue_to_run++;
						}
					}
					else
					{
						datatarget = dataList->dataClstElem + target_point;
						datatarget->parentId = source_point;
						datatarget->parentProcessId = source_pr;
					}
				}
			}
		}
		tag++;
		round++;

		if(scount > 0)
			MPI_Waitall(scount, &d_req_send[0], &d_stat_send[0]); // wait for all the sending operation
	}

	int final_cluster_root = 0, total_final_cluster_root = 0;
	int points_in_cluster_final = 0, total_points_in_cluster_final = 0;
	int noise = 0;

	/*FILE* debug;
	string currFile("");
	currFile = neighbourFileName + patch::to_string(rank);
	debug = fopen(currFile.c_str(), "w");
*/
	Data datai;
	for(i = 0; i < dataList->localCnt; i++)
	{

		datai = dataList->dataClstElem + i;

		if(datai->ClusterID == 0 && datai->haloPoint == FALSE) noise++;

		//fprintf(debug, "%d_%d\t%d_%d\t%d\t%d\n", datai->id, datai->actualProcessId, datai->parentId, datai->parentProcessId, datai->ClusterID, datai->core_tag);
		
		if(datai->parentId == i && datai->parentProcessId == rank && datai->childCount >= 1)
		{
			points_in_cluster_final += datai->childCount;
			final_cluster_root++;
		}

	}

	MPI_Allreduce(&final_cluster_root, &total_final_cluster_root, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);

	struct result* result = (struct result*)calloc(1, sizeof(struct result));

	result->noise = noise;
	result->clusters = total_final_cluster_root;
	result->corepoints = -1;

	merge_received.clear();
	merge_send1.clear();
	merge_send2.clear();
	init.clear();

	return result;
}

void sequentialClustering(DataHdr dataList)
{
	int i;
	int j, k, npid, prID;
	int rank, nproc, mpi_namelen;

	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &nproc);

	int pid;
	int root, root1, root2, tid;

    noiseNeighbours = (int**)calloc(dataList->localCnt, sizeof(int*));
    // assert(noiseNeighbours != NULL);
    
	Neighbours neigh = NULL,neigh1 = NULL;
	
	clusteringstart = MPI_Wtime();
	Data data;
	for(i = 0; i < dataList->localCnt; i++)
	{
		noiseNeighbours[i] = NULL;
		pid = i;
		data = dataList->dataClstElem + i;

		if(data->isProcessed == TRUE || data->core_tag == TRUE)
        {
            continue;   
        }

		neigh = findNeighbours3(dataList, i, EPS, NULL);
		data->isProcessed = TRUE;

		if(neigh->inner_neighbours >= MINPOINTS)
		{
			//Merging has already happened in the findNeighbours3 method. No need to duplicate merging
			continue;
		}


		if(VECTOR_TOTAL(neigh->neighbours_local_inner) + VECTOR_TOTAL(neigh->neighbours_remote_inner) + VECTOR_TOTAL(neigh->neighbours_local_outer) + VECTOR_TOTAL(neigh->neighbours_remote_outer) >= MINPOINTS)
		{
			// Current point is a core point
			// pid is a core point
			root = pid;

			data->core_tag = TRUE;
			data->ClusterID = 1;
			
			//NEEDS MODIFICATION
			Data dataroot1;
			Data dataroot2;
			for (j = 0; j < VECTOR_TOTAL(neigh->neighbours_local_inner); j++)
    		{

				Data datalocal = dataList->dataClstElem + neigh->neighbours_local_inner->intItems[j];
				npid = datalocal->id;

				root1 = npid;
				root2 = root;

				if(datalocal->core_tag == TRUE || datalocal->ClusterID == 0)
				{
					datalocal->ClusterID = 1;

					 dataroot1 = dataList->dataClstElem + root1;
					 dataroot2 = dataList->dataClstElem + root2;

					unionFindMerge(dataList, root1, root2, dataroot1, dataroot2);
					// REMS algorithm to (union) merge the trees
					
				}
			}


			//Data dataroot1;
			//Data dataroot2;
			for (j = 0; j < VECTOR_TOTAL(neigh->neighbours_local_outer); j++)
    		{

				Data datalocal = dataList->dataClstElem + neigh->neighbours_local_outer->intItems[j];
				npid = datalocal->id;

				root1 = npid;
				root2 = root;

				if(datalocal->core_tag == TRUE || datalocal->ClusterID == 0)
				{
					datalocal->ClusterID = 1;

					 dataroot1 = dataList->dataClstElem + root1;
					 dataroot2 = dataList->dataClstElem + root2;

					unionFindMerge(dataList, root1, root2, dataroot1, dataroot2);
					// REMS algorithm to (union) merge the trees
					
				}
			}
		}
		else
		{
			data->core_tag = FALSE;
			if(data->ClusterID == 1)
			{
				;
			}
			else
			{
				data->ClusterID = 0;
				Data neighbour;
				Data dataroot1;
				Data dataroot2;
	            for(j=0;j<VECTOR_TOTAL(neigh->neighbours_local_inner);j++)
	            {
	                neighbour = dataList->dataClstElem + neigh->neighbours_local_inner->intItems[j];
	                if(neighbour->core_tag == TRUE) //Border point scenario.
	                {
	                	root1 = data->id;
						root2 = neighbour->id;

						data->ClusterID = 1;

						Data dataroot1 = dataList->dataClstElem + root1;
						Data dataroot2 = dataList->dataClstElem + root2;
						unionFindMerge(dataList, root1, root2, dataroot1, dataroot2);
						// REMS algorithm to (union) merge the trees
	                    data->ClusterID = 1;
	                    neighbour->ClusterID = 1;
	                    break;
	                }
	            }

	            //Data dataroot1;
	            //Data dataroot2;
	            for(j=0;j<VECTOR_TOTAL(neigh->neighbours_local_outer);j++)
	            {
	                neighbour = dataList->dataClstElem + neigh->neighbours_local_outer->intItems[j];
	                if(neighbour->core_tag == TRUE) //Border point scenario.
	                {
	                	root1 = data->id;
						root2 = neighbour->id;

						data->ClusterID = 1;

						dataroot1 = dataList->dataClstElem + root1;
						dataroot2 = dataList->dataClstElem + root2;
						unionFindMerge(dataList, root1, root2, dataroot1, dataroot2);
						// REMS algorithm to (union) merge the trees
	                    data->ClusterID = 1;
	                    neighbour->ClusterID = 1;
	                    break;
	                }
	            }

	            if(data->ClusterID == 0)
	            {
	                noiseNeighbours[data->id] = (int*)malloc(sizeof(int)*MINPOINTS);

	                int k = 0;
	            	for(k=0;k<MINPOINTS;k++) noiseNeighbours[data->id][k] = -1;
	            	k=0;

	                for(j=0; j<VECTOR_TOTAL(neigh->neighbours_local_inner); j++)
	                {
                       	noiseNeighbours[data->id][k++] = neigh->neighbours_local_inner->intItems[j];
	                }

	                for(j=0; j<VECTOR_TOTAL(neigh->neighbours_local_outer); j++)
	                {
                       	noiseNeighbours[data->id][k++] = neigh->neighbours_local_outer->intItems[j];
	                }

	                for(j=0; j<VECTOR_TOTAL(neigh->neighbours_remote_inner); j++)
	                {
                       	noiseNeighbours[data->id][k++] = neigh->neighbours_remote_inner->intItems[j];
	                }

	                for(j=0; j<VECTOR_TOTAL(neigh->neighbours_remote_outer); j++)
	                {
                       	noiseNeighbours[data->id][k++] = neigh->neighbours_remote_outer->intItems[j];
	                }
	            }
			}
		}

		VECTOR_FREE(neigh->neighbours_local_outer);
		free(neigh->neighbours_local_outer);

		VECTOR_FREE(neigh->neighbours_remote_outer);
		free(neigh->neighbours_remote_outer);

		VECTOR_FREE(neigh->neighbours_local_inner);
		free(neigh->neighbours_local_inner);

		VECTOR_FREE(neigh->neighbours_remote_inner);
		free(neigh->neighbours_remote_inner);

		free(neigh);
		neigh = NULL;
	}

	clusteringend = MPI_Wtime();
	postprocessingstart = MPI_Wtime();
	postprocessingstart = MPI_Wtime();


	Data dataroot1;
	Data dataroot2;
	for(i=0; i<dataList->localCnt; i++)
	{
	    Data potentialNoise = dataList->dataClstElem + i;
	    
	    if(potentialNoise->ClusterID == 0 && noiseNeighbours[i] != NULL)
	    {
	    	int v;
		    for(v=0; v<MINPOINTS; v++)
		    {
		        if(noiseNeighbours[i][v] >= 0)
		        {
		            Data data = dataList->dataClstElem + noiseNeighbours[i][v];
		            if(data->core_tag == TRUE)
		            {
		            	root1 = data->id;
						root2 = potentialNoise->id;

						dataroot1 = dataList->dataClstElem + root1;
						dataroot2 = dataList->dataClstElem + root2;
						unionFindMerge(dataList, root1, root2, dataroot1, dataroot2);
						
						// REMS algorithm to (union) merge the trees
	                    data->ClusterID = 1;
		                potentialNoise->ClusterID = 1;
		                break;
		            }
		        }
		    }
		    free(noiseNeighbours[i]);
		}
	}

	free(noiseNeighbours);
	//Data data;
	Neighbours n;
	RHdrNd hdrNdTree;
	Data data2;
	Group g;
	for(i=0; i<VECTOR_TOTAL(unprocessedCore); i++)
	{
		data = dataList->dataClstElem + unprocessedCore->intItems[i];
		g = groupList->groupItems[data->group_id];
		Region region = createCellRegOfPoint(data, EPS);

		vectorc* neighbours_local_outer = (vectorc*)malloc(sizeof(struct vectorc));
		VECTOR_INIT(neighbours_local_outer, INTEGER);

		vectorc* neighbours_remote_outer = (vectorc*)malloc(sizeof(struct vectorc));
		VECTOR_INIT(neighbours_remote_outer, INTEGER);

		n = (Neighbours)malloc(sizeof(struct neighbours));

		n->neighbours_local_outer = neighbours_local_outer;
		n->neighbours_remote_outer = neighbours_remote_outer;

		hdrNdTree = g->bcell->auxCellRTree;
		for(k=0; k<VECTOR_TOTAL(g->corepoints); k++)
		{
			data2 = dataList->dataClstElem + g->corepoints->intItems[k];
			if(data2->haloPoint == FALSE)
			{
			    if(RisContains(region, data2->iData))
			    {
			    	if(distance(data->iData, data2->iData) <= EPS)
			    	{
				    	addHelper[0] = data2->id;
						VECTOR_ADD(n->neighbours_local_outer, addHelper);
			    	}
			    }
			}
		}
		Group rg;
		DataPoint d;
		Data data2;
		//RHdrNd hdrNdTree;
		for(j=0; j<VECTOR_TOTAL(g->reachable_groups); j++)
		{
			rg = groupList->groupItems[g->reachable_groups->intItems[j]];
			d = (dataList->dataClstElem + rg->master_id)->iData;
			Region potentialRegion = createCellRegOfDataPoint(d, EPS);

			if(GisOverLap(region, potentialRegion))
			{
				hdrNdTree = rg->bcell->auxCellRTree;
				for(k=0; k<VECTOR_TOTAL(rg->corepoints); k++)
				{
				    data2 = dataList->dataClstElem + rg->corepoints->intItems[k];
				    if(data2->haloPoint == FALSE)
				    {
				        if(RisContains(region, data2->iData))
			            {
			                if(distance(data->iData, data2->iData) <= EPS)
					    	{
						    	addHelper[0] = data2->id;
								VECTOR_ADD(n->neighbours_local_outer, addHelper);
					    	}
			            }
				    }
				}
			}
			free(potentialRegion->iBottomLeft);
			free(potentialRegion->iTopRight);
			free(potentialRegion);
		}
		Data temp;
		Data dataroot1;
		Data dataroot2;
		for(j=0;j<VECTOR_TOTAL(n->neighbours_local_outer);j++)
		{
			temp = dataList->dataClstElem + n->neighbours_local_outer->intItems[j];

			int root1 = temp->id;
			int root2 = data->id;
			int root;
			dataroot1 = dataList->dataClstElem + root1;
			dataroot2 = dataList->dataClstElem + root2;
			unionFindMerge(dataList, root1, root2, dataroot1, dataroot2);
	     	data->ClusterID = 1;
		}

		VECTOR_FREE(n->neighbours_remote_outer);
		VECTOR_FREE(n->neighbours_local_outer);
		free(n->neighbours_local_outer);
		free(n->neighbours_remote_outer);
		free(n);

		free(region->iBottomLeft);
		free(region->iTopRight);
		free(region);
	}
	 postprocessingend =  MPI_Wtime();

}

struct result* get_clusters_sequential(DataHdr dataList)
{
	int rank, nproc, i;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	int root;
	int clusters = 0;
	int noise = 0;
	int totalcore = 0;

	FILE* debug;
	string currFile("");
	currFile = debugFileName + patch::to_string(rank);
	debug = fopen(currFile.c_str(), "w");
	Data dataroot;
	Data datai;
	for(i = 0; i < dataList->localCnt; i++)
	{
		root = i;
		dataroot = dataList->dataClstElem + root;
		datai = dataroot;

		if(datai->core_tag == TRUE)totalcore++;
		if(datai->ClusterID == 0) noise++;
		
		if(dataroot->parentId == dataroot->id && dataroot->parentProcessId == rank && dataroot->ClusterID==1) // root is a local root
		{
			datai = dataList->dataClstElem + i;
			datai->parentId = root;
			clusters++;
			dataroot->childCount = dataroot->childCount + 1;
		}

		// if(debug!=NULL){
		// 	fprintf(debug,"%lf\t%lf\t%lf\t%d\n",datai->iData[0],datai->iData[1],datai->iData[2],(int)datai->core_tag);
		// }

	}

	struct result* r;
	r = (struct result*)calloc(1, sizeof(struct result));
	r->noise = noise;
	r->clusters = clusters;
	r->corepoints = totalcore;
	
	return r;
}
