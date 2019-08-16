/*

Algorithm: Micro-cluster based DBSCAN
Author: Aditya Sarma
email: asaditya1195@gmail.com

*/

#include "Def.h"

#include <fstream>
#include <string>
#include <iostream>


int fileReadSingle(char* filename, int* numObjs, int* numCoords, vector< vector<double> >& objects) ;
int fileReadMulti(char* infilename, int* numObjs, int* numCoords, vector< vector<double> >& objects) ;
void compute_local_bounding_box(vector< vector<double> >& objects, int* num_points, interval* box);


void compute_global_bounding_box(Interval box, Interval gbox, int nproc);
void copy_global_box_to_each_node(Interval* nodes_gbox, Interval gbox, int internal_nodes);
double findKMedian(vector<double>& A,int K);
double get_median(vector< vector< double> >& objects, int* num_points, int d, MPI_Comm& new_comm);
int get_points_to_send(vector< vector< double> >& objects, int* num_points, vector<double>& send_buf, vector<int>& invalid_pos_as, double median, int d, int rank, int partner_rank);
void copy_box(interval* target_box, interval* source_box);
void update_points(vector< vector< double> >& objects, int* num_points, int s_count, vector <int>& invalid_pos_as, vector <double>& recv_buf);
void start_partitioning(vector< vector<double> >& objects, int* num_points);
bool addPoints(int source, int buf_size, int dims, vector<double>& raw_data, vector< vector<double> >& remote_objects, vector<int>& remote_PrIDs, int* remote_number);
bool updatePoints(vector< vector<int> >& raw_ind, vectorc* remote_Indices, vectorc* remote_PrIDs, int remote_number);
void get_extra_points(vector< vector<double> >& objects, int* num_points, vector< vector<double> >& remote_objects, vectorc* remote_PrIDs, vectorc* remote_Indices, int* remote_number);