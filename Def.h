/*

Algorithm: Micro-cluster based DBSCAN
Author: Aditya Sarma
email: asaditya1195@gmail.com

*/

#ifndef _DEF_H
#define _DEF_H

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <assert.h>
#include <math.h>
#include <sched.h>
#include <float.h>
#include <mpi.h>
#include <vector>

using namespace std;

#define LOWER(i) (i<<1)
#define UPPER(i) ((i<<1)+1)

#define DELIM " " //Delimiter in the input file
#define LEFT(x) (2 * (x) + 1)
#define RIGHT(x) (2 * (x) + 2)
#define PARENT(x) ((x-1) / 2)
#define DEBUG 0
#define proc_of_interest 0

#define PRINT 0

#define ASSERT(x) assert(x!=NULL)
#define NOISE 0
#define POW2(x) (1 << (x))

typedef struct interval* Interval;

typedef struct pointelem* PointElem;
typedef struct pointHdr* PointHdr;

typedef struct data* Data;  // Structure holding the data of each element
typedef struct dataHdr* DataHdr;// Head Node for the list of data elements
                                //to store neighbours
typedef double* DataPoint;    // type of pointer to data point
typedef double dataPoint;   // type of data point
typedef struct region* Region;  // type of pointer to structure for rectangle

typedef struct clusterds* ClusterDS; // datatype for pointer to structure for cluster

typedef double *Dimension;  //type of pointer to one corner of rectangle
typedef double dimension; //type of corner of rectangle
typedef double *Double; //type of pointer to double
typedef int RecNum;

typedef struct RhdrNd* RHdrNd;  // Head Node for a list of children of RTree
typedef struct RlstNd* RLstNd;

typedef struct RtreeData* RTreeData; // type of pointer to Data in tree node
typedef struct RtreeNode* RTreeNode;

typedef struct RnbHdr* RNbHdr;
typedef struct Rnb* RNB;

typedef struct group* Group;

typedef struct bcell* BCell;
typedef struct celldatahd* CellDataHd;
typedef struct celldata* CellData;

typedef struct bcelllistnode* BCellListNode;
typedef struct bcelllisthd* BCellListHd;

typedef struct Kdtree* KdTree;

typedef struct KNNdistnode* KNNDistNode;

typedef struct GhdrNd* GHdrNd;  // Head Node for a list of children of RTree
typedef struct GlstNd* GLstNd;  // Nodes in the list of children of RTree node

typedef struct GtreeData* GTreeData; // type of pointer to Data in tree node
typedef struct GtreeNode* GTreeNode;  // type of pointer to tree node

typedef struct vectorc vectorc;

typedef struct assign* Assign;
typedef struct neighbours* Neighbours;

#ifndef EXTERN_H
#define EXTERN_H 

extern double EPS;
extern int MINPOINTS;
extern int DIMENSION;    

extern int GMINENTRIES;  
extern int GMAXENTRIES;
extern int GAUXMINENTRIES;
extern int GAUXMAXENTRIES; 
extern int RMINENTRIES;        
extern int RMAXENTRIES;  

extern int CLUSTERID;
extern int GROUPID;
extern int BCELLID;

extern int* addHelper;
extern double* addHelperDouble;

extern struct vectorc* groupList;
extern int** noiseNeighbours;
extern int* visited;

extern int noOfGroups;
extern struct vectorc* reachableGroupCount;
extern int maxReachableGroupCount;
extern int findNeighbourCount;
extern int avgReachableGroupCount;
extern int savedQueries;
extern int totalCore;
extern struct vectorc* unprocessedCore;

extern double * MINGRIDSIZE;
extern double * MAXGRIDSIZE;
extern double * MINGRIDSIZEglobal, *MAXGRIDSIZEglobal;
extern FILE* output;

#endif

struct interval{
  double lower,upper;
  //float lower, upper;
};

typedef enum{
  INTNODE,
  EXTNODE
}NODETYPE;  // used to mark a node of RTree as internal or external

typedef enum{
  NONE,
  SPARSEGROUP,
  COREGROUP,
  MINICLUSTER  
}GROUPTYPE;

typedef enum{
  FALSE,      // 
  TRUE      // 1
} Boolean;  


struct result
{
  int noise;
  int clusters;
  int corepoints;
};

typedef enum{INTEGER, GROUP, DOUBLE} VECTORTYPE;

struct ilist
{
  int i;
  struct ilist* next;
};
typedef struct ilist* iList;

struct dlist
{
  double d;
  struct dlist* next;
};
typedef struct dlist* dList;

struct neighbours
{
  //Replace with linkedlists
  int id;
  struct vectorc* neighbours_remote_inner;
  struct vectorc* neighbours_local_inner;

  struct vectorc* neighbours_remote_outer;
  struct vectorc* neighbours_local_outer;
  int inner_neighbours;
};

// defs for struct assign. This will contain information if a particular point can be
//assigned a group in the current iteration and if yes, then the pointer to that group
struct assign
{
  int canBeAssigned;
  int groupid;
};
// defs of vector.

struct vectorc
{
    Group* groupItems;
    int* intItems; 
    double* doubleItems;
    BCell* bcellItems;

    int capacity;
    int total;
    VECTORTYPE type;

};

// defs for data
struct data{

  DataPoint iData;

  Boolean haloPoint;
  int actualProcessId;
  int remoteIndex;

  int parentId;
  int parentProcessId;

  int id;
  int group_id;
  int childCount;

  Boolean core_tag; 
  Boolean isProcessed;
  int ClusterID;
  Boolean debugPoint;//Debug Variable
  int cid;

};

struct dataHdr{     //header for data list
  int uiCnt;
  int localCnt;
  Data dataClstElem;
};


struct region{
  Dimension iBottomLeft;  // bottom left corner of rectangle
  Dimension iTopRight;  // top right corner of rectanngle
};

struct clusterds{
  int parent;
  int rank;
};

//defs for Group

struct group{

  int id;
  vectorc* inner_points;
  vectorc* total_points;
  int inner_count;
  int total_count;
  GROUPTYPE type;
  DataPoint master;
  int master_id;
  BCell bcell; //The BCell formed with the master point at the center of the bcell. i.e. (bottomleft + topright) /(float) 2; 
  long double threshold;
  vectorc* reachable_groups;
  vectorc* corepoints;
  
};


// defining bigger cell
struct bcell{

  double * minOriginalBoundary;
  double * maxOriginalBoundary;
  double * minActualBoundary;
  double * maxActualBoundary;
  RHdrNd auxCellRTree;
  int id;

};

struct GtreeData{
    Region rgnRect;   // pointer to rectangle incase of internal node
    int groupElem;
};

struct GtreeNode{

    NODETYPE ndType;  //type of tree node (internal or external)
    GTreeData tdInfo;  //pointer to treedata structure

};

struct GhdrNd{     //node in data list

  unsigned int uiCnt; // Number of elements in the list
  GLstNd ptrFirstNd; // First node of the list
  GLstNd ptrParentNd;  // Parent node of the list

};

struct GlstNd{
     //node of child list
  GTreeNode tnInfo;  // Data MBR Information is stored here.
  GHdrNd ptrChildLst;  // List of child nodes
  GLstNd ptrNextNd;  // Next node in the list

};

// the defs for R Tree
struct Rnb{
       int n;
       int Gindex;
       double dist;
       RNB nbNext;
};

struct RnbHdr{

       int nbhCnt;
       RNB nbFirst;
       RNB nbLast;

};

struct RtreeData{
    Region rgnRect;   // pointer to rectangle incase of internal node
    Data dataClstElem;  //pointer to a Data in case of external node
};

struct RtreeNode{
    NODETYPE ndType;  //type of tree node (internal or external)
    RTreeData tdInfo;  //pointer to treedata structure

};


struct RhdrNd{     //node in data list
  
  unsigned int uiCnt; // Number of elements in the list
  RLstNd ptrFirstNd; // First node of the list
  RLstNd ptrParentNd;  // Parent node of the list

};

struct RlstNd{     //node of child list
  RTreeNode tnInfo;  // Data
  RHdrNd ptrChildLst;  // List of child nodes
  RLstNd ptrNextNd;  // Next node in the list
};


void dbscan(GHdrNd GRTree, DataHdr dataList, double epsilon, int min_pts);
int expandHelp(int index, int cluster_id, DataHdr dataList, double epsilon, int min_pts);

#endif
