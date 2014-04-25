/* Three parts for the project:
 *  - a terminal for router configuration, communicated through the stdio
 *  - a thread for lsrp-server, which can accept multiple clients connection
 *  - multiple clients for lsrp-clent, depends on the interfaces number configured for the router
 *
 * Steps to build and run:
 * $ make
 * $ ./lsrp-router
*/

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>
#include <string.h>
#include <sys/time.h>
#include <pthread.h>
#include "socket/libsocket.h"
#include "socket/server.h"
#include "socket/client.h"
#include "packet/hello.h"
#include "packet/libqueue.h"
#include "config/config.h"
#include "config/liblog.h"

#define PORT 0 //0 means assign a port randomly
#define BUFFER_SIZE  1024
#define NTHREADS 128
#define LOGFILE "lsrp-router.log"
//#define LSRPCFG "config/lsrp-router.cfg"

/* three parts in the main:
 * 1) a terminal for router configuration
 * 2) a thread for sockserver
 * 3) multiple threads for sockclient */
int main(int argc, char *argv[]){

    /* use default cfg file if not given */
    char lsrpcfg[128];
    memset(lsrpcfg, 0, sizeof(lsrpcfg));

    /* read cfg file first */
    Router *router;

    if(argc > 2){
        fprintf(stderr, "USAGE: ./lsrp-router <lsrp-router.cfg>\n");
        exit(1);
    }
    else if(argc == 2) {
        fprintf(stdout, "(lsrp-router): use router cfg file: %s\n", argv[1]);
        strncpy(lsrpcfg, argv[1], strlen(argv[1]));
    }
    else {
        strncpy(lsrpcfg, "config/lsrp-router.cfg", sizeof(lsrpcfg));
        fprintf(stdout, "(lsrp-router): use default router cfg file: %s\n", lsrpcfg);
    }

    router = getRouter(lsrpcfg);

    ThreadParam *threadParam;
    threadParam = (ThreadParam *)calloc(1, sizeof(ThreadParam));
    threadParam->ls_db_size = 0;
    threadParam->routing_size = 0;
    threadParam->router = router;
    int i;
    for(i=0;i < router->num_of_interface;i++){
        threadParam->lsa_buffer[i].buffsize = 0; 
        threadParam->data_buffer[i].buffsize = 0; 
        threadParam->lsa_buffer[i].packet_q = initlist();
        threadParam->data_buffer[i].packet_q = initlist();
    }

    /* Thread attribute */
    pthread_attr_t attr;
    pthread_attr_init(&attr); // Creating thread attributes
    pthread_attr_setschedpolicy(&attr, SCHED_RR); // Round Robin scheduling for threads 
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED); // Don't want threads (particualrly main)

    /* for server thread */
    pthread_t sockserverid; 
    pthread_create(&sockserverid, NULL, &sockserver, (void *)threadParam);
    fprintf(stdout, "(lsrp-router): socket server start up.\n");

    /* for client threads*/
    pthread_t threadid[NTHREADS]; // Thread pool
    int counter = 0;

    int iThread; // Thread iterator
    
    /* start num_of_interface threads in busy wait */
    for(iThread=0; iThread < router->num_of_interface; iThread++){
        pthread_create(&threadid[iThread], &attr, &sockclient, (void *)threadParam);
        fprintf(stdout, "(lsrp-router): socket client %d start up.\n", iThread);
        //sleep(1);
    }
    fprintf(stdout, "(lsrp-router): please track log file for detail: %s\n", LOGFILE);
    fprintf(stdout, "\n== WELCOME TO LSRP ROUTER CONFIGURATION TERMINAL! ==\n");
    fprintf(stdout, "\nEnter the commands 'help' for usage.\n\n");

    
    int rc;
    char command[64], arguments[64];
    int terminal = 1;
    char *split1, *split2, *strsplit;
    while(terminal == 1){
        fprintf(stdout, "(lsrp-router: %s)# ", router->router_id);
	fflush(stdin);
	if (fgets(command, sizeof(command), stdin) != NULL ){
            strsplit = command;
            split1 = strtok_r(strsplit, " ", &split2);

            if(strcmp(split1, "quit\n") == 0){ // there is a \n in the stdin
                /* TODO: clean threads before quit terminal */
                if((terminal = quitRouter()) == 0){
                    break;
                }
            }
            else if(strcmp(split1, "help\n") == 0){ // there is a \n in the stdin
                showHelp();
            }
            else if(strcmp(split1, "showlsdb\n") == 0){ // there is a \n in the stdin
                showLSDB(threadParam);
            }
            else if(strcmp(split1, "showrt\n") == 0){ // there is a \n in the stdin
                showRouting(threadParam);
            }
            else if(strcmp(split1, "showcfg\n") == 0){ // there is a \n in the stdin
                showCFG(router, lsrpcfg);
            }
            else if(strcmp(split1, "enablelink") == 0){ // there is a \n in the stdin
                if(isdigit(split2[0]) == 0){
                    fprintf(stderr, "ERROR: input is not integer: %s\n", split2);
                }
                else{
                    enableLink(router, atoi(split2));
                    writeRouter(lsrpcfg, router);
                }
            }
            else if(strcmp(split1, "disablelink") == 0){ // there is a \n in the stdin
                if(isdigit(split2[0]) == 0){
                    fprintf(stderr, "ERROR: input is not integer: %s\n", split2);
                }
                else{
                    disableLink(router, atoi(split2));
                    writeRouter(lsrpcfg, router);
                }
            }
            else if(strcmp(split1, "\n") != 0){ // there is a \n in the stdin
                fprintf(stderr, "ERROR: command not found: %s\n", split1);
                showHelp();
            }

        }
        else{
            fprintf(stderr, "Not all fields are assigned\n");
        }

    }

    return 0;
}
