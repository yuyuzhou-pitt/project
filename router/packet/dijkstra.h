#ifndef __DIJKSTRA_H__
#define __DIJKSTRA_H__

#define MAX_CONN 10
#define MAX_NODES 50
#define INFINITE INT_MAX

struct node /*data structure to store information of each node*/
{
    int id; 
    struct node *l[MAX_CONN+1]; /*array that points to all other nodes connected with this node*/
    int cost[MAX_CONN+1];  /*array that stores cost of all connections with this node*/
    struct node **next; /*address of the pointer of the next node to traverse, for the ith destination node in the array of records datastructure*/
    int *mincost;/*minimum cost to traverse the ith destination node in the array of records datastructure*/
};
typedef struct node nodes; 
struct record /* data structure to map node id and corresponding address of nodes structure.*/
{
    int nid;
    struct node *add;
};
typedef struct record records;

void view(records nlist[]);
void djkstra(records nlist[], int count);
int min_route(int sid, int did, int *gateway, int *metric, records nlist[]);
int scanfile(char *filename, records nlist[]);
#endif
