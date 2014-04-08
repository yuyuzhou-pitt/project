/*********************
rule of config file
1. all config should follow the rule:
parameter = value
there should be one blank before the =, and one blank after =.
2. name of one parameter should not be the prefix of another parameter.
for example
eth0_ip and eth0_ip_direct can not be used at the same time.
**************/

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include"libfile.h"
#include"config.h"

Router *getRouter(char *filename){
    FILE *fp;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    if(access(filename, F_OK) < 0) {
        printf("getRouter: File not found: %s\n", filename);
        return -1;
    }

    if ((fp = fopen(filename,"rt"))<0){
        printf("getRouter: Faile to open file: %s \n", filename);
        return;
    }

    Router *router;
    //Ethernet ethx[ETHX];
    router = (Router *)malloc(sizeof(Router));

    char *tmp_param, *tmp_value, *saveptr;

    int interface = -1; // the index to router->ethx[ETHX]
    while ((read = getline(&line, &len, fp)) != -1) {
        tmp_param = strtok_r(line, "=", &saveptr); // got the param before "="
        tmp_value = strtok(saveptr, "#"); // ignore the comments

        if(strstrip(tmp_param) == ""){
            continue; // if blank line
        }
        if(strcmp(strstrip(tmp_param), "[EOF]") == 0){
            break; // if end of file
        }

        if(strcmp(strstrip(tmp_param), "router_id") == 0){
            snprintf(router->router_id, sizeof(router->router_id), "%s", strstrip(tmp_value));
        }
        if(strcmp(strstrip(tmp_param), "protocol_version") == 0){
            snprintf(router->protocol_version, sizeof(router->protocol_version), "%s", strstrip(tmp_value));
        }
        if(strcmp(strstrip(tmp_param), "acquisition_authorization") == 0){
            snprintf(router->acquisition_authorization, sizeof(router->acquisition_authorization), "%s", strstrip(tmp_value));
        }
        if(strcmp(strstrip(tmp_param), "hello_interval") == 0){
            router->hello_interval = atoi(strstrip(tmp_value));
        }
        if(strcmp(strstrip(tmp_param), "ping_timeout") == 0){
            router->ping_timeout = atoi(strstrip(tmp_value));
        }
        if(strcmp(strstrip(tmp_param), "ls_updated_interval") == 0){
            router->ls_updated_interval = atoi(strstrip(tmp_value));
        }
        if(strcmp(strstrip(tmp_param), "ls_age_limit") == 0){
            router->ls_age_limit = atoi(strstrip(tmp_value));
        }
        if(strcmp(strstrip(tmp_param), "hold_down_timer") == 0){
            router->hold_down_timer = atoi(strstrip(tmp_value));
        }
        if(strcmp(strstrip(tmp_param), "num_of_interface") == 0){
            router->num_of_interface = atoi(strstrip(tmp_value));
        }
        if(strcmp(strstrip(tmp_param), "eth_id") == 0){
            interface++; // index start from 0
            snprintf(router->ethx[interface].eth_id, sizeof(router->ethx[interface].eth_id), "%s", strstrip(tmp_value));
        }
        if(strcmp(strstrip(tmp_param), "direct_link_addr") == 0){
            snprintf(router->ethx[interface].direct_link_addr, sizeof(router->ethx[interface].direct_link_addr), "%s", strstrip(tmp_value));
        }
        if(strcmp(strstrip(tmp_param), "link_cost_method") == 0){
            snprintf(router->ethx[interface].link_cost_method, sizeof(router->ethx[interface].link_cost_method), "%s", strstrip(tmp_value));
        }
        if(strcmp(strstrip(tmp_param), "link_cost") == 0){
            router->ethx[interface].link_cost = atoi(strstrip(tmp_value));
        }
        if(strcmp(strstrip(tmp_param), "link_failure_time") == 0){
            router->ethx[interface].link_failure_time = atoi(strstrip(tmp_value));
        }
        if(strcmp(strstrip(tmp_param), "packet_error_rate") == 0){
            router->ethx[interface].packet_error_rate = atoi(strstrip(tmp_value));
        }

    }
    
    return router;
}

int writeRouter(char *filename, Router *router){
    copyFile(filename, "lsrp-router.cfg.bk");

    FILE *fp;

    if ((fp = fopen(filename,"wt")) < 0){
        printf("writeRouter: Failed to open file: %s\n", filename);
        return -1;
    }

    char routerStr[2048];
    memset(routerStr, 0, sizeof(routerStr));
    snprintf(routerStr, sizeof(routerStr), "\
# NOTE: 1) update ETHX in config.h if more interfaces defined here \n\
#       2) value does NOT have \"\" even it's a string \n\
\n\
[Global config]\n\
router_id = %s\n\
protocol_version = %s\n\
acquisition_authorization = %s\n\
hello_interval = %d # in seconds\n\
ping_timeout = %d # in seconds\n\
ls_updated_interval = %d # in seconds\n\
ls_age_limit = %d #in seconds\n\
hold_down_timer = %d #in seconds\n\
num_of_interface = %d\n\
\n\
[interface config]\n\
", router->router_id, router->protocol_version, router->acquisition_authorization, \
    router->hello_interval, router->ping_timeout, router->ls_updated_interval, \
    router->ls_age_limit, router->hold_down_timer, router->num_of_interface);


    /* generate the Ethernet part */
    int i;
    for(i=0;i<ETHX;i++){
        char ethxStr[512];
        memset(ethxStr, 0, sizeof(ethxStr));
        snprintf(ethxStr, sizeof(ethxStr), "\
[eth%d]\n\
eth_id = %s\n\
direct_link_addr = %s # remote host ip (router_id)\n\
link_cost_method = %s # auto - calculated by  ping delay, manual - manual setting\n\
link_cost = %d # infinit\n\
link_failure_time = %d # seconds\n\
packet_error_rate = %d\n\
\n\
", i, router->ethx[i].eth_id, router->ethx[i].direct_link_addr, router->ethx[i].link_cost_method, \
    router->ethx[i].link_cost, router->ethx[i].link_failure_time, router->ethx[i].packet_error_rate);

       strcat(routerStr, ethxStr); // add ethx information to router
    }

    fwrite(strstrip(routerStr), 1 , strlen(strstrip(routerStr)) , fp );

    fclose(fp);
    
    return 0;
}

void cfgread(char filename[], char parameter[], char viarable[])
{
    FILE *fp;
    char temp[80];
    char result[80];
    int len, index;

    if ((fp = fopen(filename,"rt"))==NULL){
        printf("\nCannot open file!");
        return;
    }
    
    len = strlen(parameter);
    index = 0;
    temp[index] = fgetc(fp);
    while (temp[index]!=EOF){
        if (index == len) {
            printf("\nParameter %s is found!", parameter);
            index = 0;
            temp[index]=fgetc(fp);
            temp[index]=fgetc(fp);//get rid of =
            result[index] = fgetc(fp);
            while ((result[index]!='\n')&&(result[index]!=EOF)){
                index=index+1;
                result[index]=fgetc(fp);
            }
            strcpy(viarable, result);
            fclose(fp);
            return;
        }
        else if (temp[index]!=parameter[index]) {
            index = 0;
            temp[index] =fgetc(fp);
        }
        else {
            index = index + 1;
            temp[index] = fgetc(fp);
        }
    }
    printf("\nError: parameter %s is not found!",parameter);
    fclose(fp); 
}

void cfgwrite(char filename[], char parameter[], char content[])
{
    FILE *fp, *tempfile;
    char temp[80];
    char ch;
    int par_len, con_len, index;

    if ((fp = fopen(filename,"rt+"))==NULL){
        printf("\nCannot open file!");
        return;
    }
    
    if ((tempfile = fopen("tempconfig","wt"))==NULL){
       printf("\nCreate temp configure file fail!");
       return;
    }
    
    ch = fgetc(fp);
    while (ch!=EOF){
        fputc(ch,tempfile);
        ch = fgetc(fp);
    }
    fclose(fp);
    fclose(tempfile); // back up old configure file

   if ((fp = fopen(filename,"wt"))==NULL){
        printf("\nCannot reopen file!");
        return;
    }

    if ((tempfile = fopen("tempconfig","rt"))==NULL){
       printf("\nOpen temp configure file fail!");
       return;
    }

    par_len = strlen(parameter);
    con_len = strlen(content);
    index = 0;
    temp[index] = fgetc(tempfile);
    while (temp[index]!=EOF){
        if (index == par_len) { // replace the parameter value
            printf("\nParameter %s is found!", parameter);
            fputc(temp[index],fp);
            index = 0;
            ch=fgetc(tempfile);fputc(ch,fp);
            ch=fgetc(tempfile);fputc(ch,fp);//get rid of =
            ch=fgetc(tempfile);
            while ((ch!='\n') && (ch!=EOF)){
                ch=fgetc(tempfile);
            }
            while (index!=con_len){
                fputc(content[index],fp);
                index = index + 1;
            }
            while (ch!=EOF){
                fputc(ch,fp);
                ch = fgetc(tempfile);
            }
            fclose(tempfile);
            fclose(fp);
            printf("\nParameter %s is updated as %s!", parameter, content);
            return;
        }
        else if (temp[index]!=parameter[index]) {
            fputc(temp[index],fp);
            index = 0;
            temp[index] =fgetc(tempfile);
        }
        else {
            fputc(temp[index],fp);
            index = index + 1;
            temp[index] = fgetc(tempfile);
        }
    }
    printf("\nError: parameter %s is not found!",parameter);
    fclose(fp);
    fclose(tempfile);
}

/*
int main()
{
//  cfgread("lsrp-router.cfg","link_cost_method",j);
//  printf("%s",j);
//  cfgwrite("lsrp-router.cfg","eth_0_id","10.66.10.3");

    Router *router;
    router = getRouter();
    writeRouter(router);
    return 0;
}
*/
