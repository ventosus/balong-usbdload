#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include "patcher.h"


//#######################################################################################################
void main(int argc, char* argv[]) {
  
FILE* in;
FILE* out;
uint8_t* buf;
uint32_t fsize;
int opt;
uint8_t outfilename[100];
int oflag=0,bflag=0;
uint32_t res;


// Parsing der Befehl Linien

while ((opt = getopt(argc, argv, "o:bh")) != -1) {
  switch (opt) {
   case 'h': 
     
printf("\n Das Programm für die automatisch patch Downloader Plattformen Balong V7\n\n\
%s [Schlüssel] <Datei Bootloader usbloader>\n\n\
 Akzeptabel das Folgende Schlüssel:\n\n\
-o file  - Name Ausgabe Datei. Von Standard produziert nur testen Möglichkeiten patch\n\
-b       - hinzufügen patch, abgeschnitten testen defekt Blöcke\n\
\n",argv[0]);
    return;

   case 'o':
    strcpy(outfilename,optarg);
    oflag=1;
    break;

   case 'b':
     bflag=1;
     break;
     
   case '?':
   case ':':  
     return;
    
  }
}  

printf("\n Das Programm automatisch Änderungen Downloader Balong V7, (c) forth32");

 if (optind>=argc) {
    printf("\n - Nicht angezeigt Name Datei für die Downloads\n - Für Tipps angeben Hinweis -h\n");
    return;
}  
    
in=fopen(argv[optind],"r");
if (in == 0) {
  printf("\n Fehler Entdeckungen Datei %s",argv[optind]);
  return;
}

// bestimmen Größe Datei
fseek(in,0,SEEK_END);
fsize=ftell(in);
rewind(in);

// hervorheben Puffer und lesen dort das Ganze Datei
buf=malloc(fsize);
fread(buf,1,fsize,in);
fclose(in);

//==================================================================================

res=pv7r22_2(buf, fsize);
if (res != 0)  {
  printf("\n* Gefunden Signatur eingeben V7R22_2 auf dem Vorurteil %08x",res);
  goto endpatch;
}

res=pv7r22(buf, fsize);
if (res != 0)  {
  printf("\n* Gefunden Signatur eingeben V7R22 auf dem Vorurteil %08x",res);
  goto endpatch;
}  

res=pv7r1(buf, fsize);
if (res != 0)  {
  printf("\n* Gefunden Signatur eingeben V7R1 auf dem Vorurteil %08x",res);
  goto endpatch;
}  

res=pv7r2(buf, fsize);
if (res != 0)  {
  printf("\n* Gefunden Signatur eingeben V7R2 auf dem Vorurteil %08x",res);
  goto endpatch;
}  

res=pv7r11(buf, fsize);
if (res != 0)  {
  printf("\n* Gefunden Signatur eingeben V7R11 auf dem Vorurteil %08x",res);
  goto endpatch;
}   

printf("\n! Unterschrift eraseall-patch nicht gefunden");

//==================================================================================
endpatch:

if (bflag) {
   res=perasebad(buf, fsize);
   if (res != 0) printf("\n* Gefunden Signatur isbad auf dem Vorurteil %08x",res);  
   else  printf("\n! Unterschrift isbad nicht gefunden");  
}

if (oflag) {
  out=fopen(outfilename,"w");
  if (out != 0) {
    fwrite(buf,1,fsize,out);
    fclose(out);
  }
  else printf("\n Fehler Entdeckungen Ausgabe Datei %s",outfilename);
}
free(buf);
printf("\n");
}

   