/*

Algorithm: Micro-cluster based DBSCAN
Author: Aditya Sarma
email: asaditya1195@gmail.com

*/

#include "Def.h"


void trivial_decompression(vector<int>* data, int nproc, int rank, int round, double& dcomtime);
void trivial_compression(DataHdr dataList, vector <int>* data, vector < vector <int> >* parser, int nproc, int rank, int round, double& comtime, double& sum_comp_rate);
void run_dbscan_algo_uf_mpi_interleaved(DataHdr dataList, vector < vector <int > >& merge_received,	vector < vector <int > >& merge_send1,
	vector < vector <int > >& merge_send2, vector <int>& init, vector < vector <int > >* pswap, vector < vector <int > >* p_cur_send, vector < vector <int > >* p_cur_insert);
struct result* get_clusters_distributed(DataHdr dataList);
void sequentialClustering(DataHdr dataList);
struct result* get_clusters_sequential(DataHdr dataList);