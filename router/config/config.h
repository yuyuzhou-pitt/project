#ifndef __CONFIG_H__
#define __CONFIG_H__

#define ETHX 128 // actual number of interface defined by num_of_interface

//#include "../packet/libqueue.h"

char i[80];
char j[80];

typedef struct Router_Ethernet{
    char eth_id[32];// = 10.0.0.1
    char netmask[32];
    char direct_link_addr[32]; // = 10.0.0.2
    char direct_link_eth_id[32]; // = 10.0.0.2
    int direct_link_type; //0 is router, 1 is end system, default is 0
    int direct_link_port;
    int link_availability; //
    char link_cost_method[32]; // = manual // auto - calculated by  ping delay, manual - manual setting
    struct timeval link_cost; // = 9999 // infinit
    int link_failure_time; // = 60 // seconds
    int packet_error_rate; // = 0
}Ethernet;

typedef struct LSRP_Router{
    /* [Global config] */
    char router_id[32]; // = 127.0.0.1
    char protocol_version[4]; // = 1.0
    char acquisition_authorization[128]; //= password in md5sum
    int hello_interval; // = 40 //seconds
    int ping_interval; // = 60 seconds
    int ping_alpha; // to calculate the average ping cost
    int ping_timeout; // = 1 //seconds
    int ls_updated_interval; // = 120 //seconds
    int ls_age_limit; // = 60 //seconds
    int hold_down_timer; // = 60 //seconds
    int num_of_interface;// = 2
    
    /* [interface config] */
    Ethernet ethx[ETHX]; // there will be more than one interfaces, it will be an interfaces array
}Router;

void cfgread(char filename[], char parameter[], char viarable[]);
void cfgwrite(char filename[], char parameter[], char content[]);

int writeRouter(char *filename, Router *router);
Router *getRouter(char *filename);

#endif
