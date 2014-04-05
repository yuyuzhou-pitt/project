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
#include<string.h>

char i[80];
char j[80];

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
void main()
{
// cfgread("lsrp-router.cfg","link_cost_method",j);
// printf("%s",j);
   cfgwrite("lsrp-router.cfg","eth_0_id","10.66.10.3");
}
*/

