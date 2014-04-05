#ifndef __CONFIG_H__
#define __CONFIG_H__

#define LSRPCFG "lsrp-router.cfg"

char i[80];
char j[80];

void cfgread(FILE *fp, char parameter[], char viarable[]);
void cfgwrite(FILE *fp, FILE *tempfile, char parameter[], char content[]);
#endif
