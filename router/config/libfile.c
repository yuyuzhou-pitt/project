#include<stdio.h>
#include<string.h>

char *strstrip(char *s)
{
    size_t size;
    char *end;

    size = strlen(s);

    if (!size)
    	return s;

    end = s + size - 1;
    while (end >= s && isspace(*end))
    	end--;
    *(end + 1) = '\0';

    while (*s && isspace(*s))
    	s++;

    return s;
}

int copyFile(char *input, char *output)
{
    FILE *fp1,*fp2;
    char ch;
    //clrscr();

    if((fp1 = fopen(input,"r")) < 0){
        printf("copyFile: Failed to open file: %s\n", input);
        return -1;
    }
    if((fp2 = fopen(output,"w")) < 0){
        printf("copyFile: Failed to open file: %s\n", output);
        return -1;
    }

    while(1)
    {
       ch = fgetc(fp1);

       if(ch==EOF)
          break;
       else
          putc(ch,fp2);
    }

    printf("File %s backup as: %s\n", input, output);
    fclose(fp1);
    fclose(fp2);

    return 0;
}
