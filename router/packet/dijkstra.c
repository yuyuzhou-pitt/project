/* 
 * Author: Mufaddal Makati
   Official Post: http://www.rawbytes.com
 Copyright [2010] [Author: Mufaddal Makati]

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
 *
 * Created on September, 2010.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "dijkstra.h"
#include "../config/liblog.h"

//records nlist[MAX_NODES+1]={0}; /*create array of records and mark the end of the array with zero.*/
//int count; /*stores number of nodes*/

int scanfile(char *filename, records nlist[])
{
    FILE *f;
    int d;
    int i=0,j=0,n_id,n_cost;
    nodes *temp=0,*temp1=0;

    if((f=fopen(filename,"r"))== NULL)
    {
        printf("Error opening file.\n");
        exit(1);
    }
    memset(nlist, 0, sizeof(struct record) * MAX_NODES);
    int count=0;
    do /*first get the id and address of all nodes*/
    {
        fscanf(f,"%d",&n_id);
        for(i=0;nlist[i].nid!=0;i++)
        {
            if(n_id==nlist[i].nid)
            {
                printf("Id already exists: %d.", n_id);
                return;
            }
        }
        temp=(nodes *)malloc(sizeof(nodes));
        if (temp == 0)
        {
           printf("ERROR: Out of memory\n");
           return;
        }
        memset(temp, 0, sizeof(struct node));
        temp->id=n_id;
        temp->l[MAX_CONN+1]=0;
        temp->cost[MAX_CONN+1]=0;
        for(i=0;nlist[i].nid!=0;i++)
        {}
        nlist[i].nid=n_id;
        nlist[i].add=temp;
        count++;
        while((d=fgetc(f)!=';'))
        {}
    }while((d=fgetc(f))!=EOF);

    rewind(f);

    for(i=0;i<count;i++) /*now get the information of all nodes connections.*/
    {
        fscanf(f,"%*d");
        temp=nlist[i].add;
        while((d=fgetc(f)!=';'))
        {
            fscanf(f,"%d-%d",&n_id,&n_cost);
            for(j=0;nlist[j].nid!=0;j++)
            {
                if(nlist[j].nid==n_id)
                {
                    temp1=nlist[j].add;
                    break;
                }
            }
            for(j=0;temp->cost[j]!=0;j++)
            {}
            temp->cost[j]=n_cost;
            temp->l[j]=temp1;
        }
    }
    fclose(f);

    return count;
}

void view(records nlist[])
{
    int i,j;
    nodes *temp=0,*temp1=0;

    printf("\nID\tConnceted to- ID:cost");
    for(i=0;nlist[i].nid!=0;i++)
    {
        printf("\n%d",nlist[i].nid);
        temp=nlist[i].add;
        for(j=0;temp->l[j]!=0;j++)
        {
            temp1=temp->l[j];
            printf("\t%d:%d",temp1->id,temp->cost[j]);
        }
    }
    printf("\n \n \n");
}

void djkstra(records nlist[], int count)
{
    int i,j,k,num,num1=0,min=INFINITE;
    int *tcost=0,*done=0;
    nodes *temp=0,*temp1=0,**tent=0;

    tcost=(int*)calloc(count, sizeof(int));
    if (tcost == 0)
    {
	printf("ERROR: Out of memory\n");
	return;
    }
    done=(int*)calloc(count, sizeof(int));
    if (done == 0)
    {
	printf("ERROR: Out of memory\n");
	return;
    }
    tent=(nodes**)calloc(count, sizeof(nodes));
    if (tent == 0)
    {
	printf("ERROR: Out of memory\n");
	return;
    }

    for(i=0;nlist[i].nid!=0;i++)
    {
         for(j=0;j<count;j++)
         {
            tcost[j]=INFINITE;
            done[j]=0;
         }
        temp=nlist[i].add;
        temp->next=(nodes**)calloc(count, sizeof(nodes));
        temp->mincost=(int*)calloc(count, sizeof(int));
        tcost[i]=0;
        done[i]=1;
        temp->mincost[i]=0;
        temp1=temp;
        for(;;)
        {
            for(num1=0;nlist[num1].nid!=0;num1++)
            {
                if(nlist[num1].add==temp1)
                    break;
            }
            for(k=0;temp1->l[k]!=0;k++)
            {
                for(num=0;nlist[num].nid!=0;num++)
                {
                    if(nlist[num].add==temp1->l[k])
                        break;
                }

                if(tcost[num] > (tcost[num1]+temp1->cost[k]))
                {
                    tcost[num]= tcost[num1] + temp1->cost[k];
                    if(temp1==temp)
                        tent[num]=temp1->l[k];
                    else
                        tent[num]=tent[num1];
                }
            }
            min=INFINITE;num1=0;
            for(j=0;j<count;j++)
            {
                if(tcost[j]<min && done[j]!=1 && tcost[j]!=0)
                {
                    min=tcost[j];
                    num1=j;
                }
            }
            if(min==INFINITE)
                break;

            temp1=nlist[num1].add;
            temp->mincost[num1]=tcost[num1];
            temp->next[num1]=tent[num1];
            done[num1]=1;
        }
    }
}

int min_route(int sid, int did, int *gateway, int *metric, records nlist[])
{
    char logmsg[128], tmpmsg[32], endmsg[16];
    memset(logmsg, 0, sizeof(logmsg));
    memset(tmpmsg, 0, sizeof(tmpmsg));
    memset(endmsg, 0, sizeof(endmsg));
    int i,num,chk=0;
    nodes *temp=0,*temp1=0;

    for(i=0;nlist[i].nid!=0;i++)
    {
        temp=nlist[i].add;
        if(temp->id==sid)
        {
            chk=1;
            break;
        }
    }
    if(chk==0)
    {
        printf("\nSource Id not found.\n");
        return -1;
    }
    chk=0;
    for(num=0;nlist[num].nid!=0;num++)
    {
        temp1=nlist[num].add;
        if(temp1->id==did)
        {
            chk=1;
            break;
        }
    }
    if(chk==0)
    {
        printf("\nDestination Id not found.\n");
        return -1;
    }

    //printf("%d-",temp->id);
    snprintf(logmsg, sizeof(logmsg), "dijkstra: Routing from %d to %d: %d->", sid, did, temp->id);

    temp1=temp;
    if(temp1->next[num]!=0)
    {
        *gateway = temp1->next[num]->id;
    }

    for(;;)
    {
        if(temp1->id==did)
            break;
        if(temp1->next[num]!=0)
        {
            temp1=temp1->next[num];

            snprintf(tmpmsg, sizeof(tmpmsg), "%d->", temp1->id);
            strncat(logmsg, tmpmsg, strlen(tmpmsg));
            //printf("-%d-",temp1->id);
        }
        else
        {
            snprintf(endmsg, sizeof(endmsg), "No Route\n");
            strncat(logmsg, endmsg, strlen(endmsg));
            logging(LOGFILE, logmsg);
            return -1;
            //break;
        }
    }
    snprintf(endmsg, sizeof(endmsg), "end\n");
    strncat(logmsg, endmsg, strlen(endmsg));
    logging(LOGFILE, logmsg);

    memset(logmsg, 0, sizeof(logmsg));
    snprintf(logmsg, sizeof(logmsg), "dijkstra: Total cost from %d to %d: %d\n", sid, did, temp->mincost[num]);
    logging(LOGFILE, logmsg);

    *metric = temp->mincost[num];
}

/*
int main(){
    int gateway, metric;
    
    records nlist[MAX_NODES+1]={0}; //create array of records and mark the end of the array with zero.
    nlist[MAX_NODES+1].nid=0;

    int count; //stores number of nodes

    count = scanfile("graph.ospf", nlist);
    djkstra(nlist, count);
    view(nlist);
    //min_route(3,4);
    min_route(45762, 40293, &gateway, &metric, nlist);
    printf("gateway=%d, metric=%d\n", gateway, metric);
    min_route(55022, 37050, &gateway, &metric, nlist);
    printf("gateway=%d, metric=%d\n", gateway, metric);
}
*/
