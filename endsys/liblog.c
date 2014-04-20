/* Write my own logging
 * */
#include <stdio.h>
#include <string.h>
#include <time.h>

     
int logging(char *filename, char *message){

    FILE *fp;

    if ((fp = fopen(filename,"a+")) < 0){
        fprintf(stderr, "openlogging: Failed to open file: %s\n", filename);
        return -1;
    }

    char buf[80];
    time_t now = time(0);
    struct tm  tstruct;
    tstruct = *localtime(&now);
    strftime(buf, sizeof(buf), "%Y-%m-%d %X", &tstruct);
    char newmsg[256]; // one line lengh
    snprintf(newmsg, sizeof(newmsg), "[%s]: %s", buf, message);
    fwrite(newmsg, 1 , strlen(newmsg) , fp );

    fclose(fp);
    return 0;
}
