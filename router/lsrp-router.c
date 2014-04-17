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

int vread(char *fmt, ...){
    int rc;
    va_list arg_ptr;
    va_start(arg_ptr, fmt);
    rc = vscanf(fmt, arg_ptr);
    va_end(arg_ptr);

    return(rc);
}

/* three parts in the main:
 * 1) a terminal for router configuration
 * 2) a thread for sockserver
 * 3) multiple threads for sockclient */
int main(int argc, char *argv[]){

    /* use default cfg file if not given */
    char *lsrpcfg = "config/lsrp-router.cfg";

    /* read cfg file first */
    Router *router;

    if(argc > 2){
        fprintf(stderr, "USAGE: ./lsrp-router <lsrp-router.cfg>\n");
        exit(1);
    }
    else if(argc == 2) {
        fprintf(stdout, "lsrp-router: use router cfg file: %s\n", argv[1]);
        router = getRouter(argv[1]);
    }
    else {
        fprintf(stdout, "lsrp-router: use default router cfg file: %s\n", lsrpcfg);
        router = getRouter(lsrpcfg);
    }

    ThreadParam *threadParam;
    threadParam = (ThreadParam *)malloc(sizeof(ThreadParam));
    threadParam->ls_db_size = 0;
    threadParam->router = router;
    int i;
    for(i=0;i < router->num_of_interface;i++){
        threadParam->buffer[i].buffsize = 0; 
        threadParam->buffer[i].packet_q = initlist();
    }

    /* Thread attribute */
    pthread_attr_t attr;
    pthread_attr_init(&attr); // Creating thread attributes
    pthread_attr_setschedpolicy(&attr, SCHED_RR); // Round Robin scheduling for threads 
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED); // Don't want threads (particualrly main)

    /* for server thread */
    pthread_t sockserverid; 
    fprintf(stdout, "lsrp-router: please track log file for detail: %s\n", LOGFILE);
    pthread_create(&sockserverid, NULL, &sockserver, (void *)threadParam);

    /* for client threads*/
    pthread_t threadid[NTHREADS]; // Thread pool
    int counter = 0;

    int iThread; // Thread iterator
    
    /* start num_of_interface threads in busy wait */
    for(iThread=0; iThread < router->num_of_interface; iThread++){
        pthread_create(&threadid[iThread], &attr, &sockclient, (void *)threadParam);
        //sleep(1);
    }
    
    int rc;
    char command[64], arguments[64];
    int terminal = 1;
    while(terminal == 1){
        fprintf(stdout, "\nEnter the commands ('help' for usage): \n");
	fflush(stdin);
	if (fgets(command, sizeof(command), stdin) != NULL ){
            if(strcmp(command, "quit\n") == 0){ // there is a \n in the stdin
                /* TODO: clean threads before quit terminal */
                if((terminal = quitRouter()) == 0){
                    break;
                }
            }
            if(strcmp(command, "help\n") == 0){ // there is a \n in the stdin
                showHelp();
            }
            if(strcmp(command, "showlsdb\n") == 0){ // there is a \n in the stdin
                showLSDB(threadParam);
            }

        }
        else{
            fprintf(stderr, "Not all fields are assigned\n");
        }

    }

    return 0;
}
