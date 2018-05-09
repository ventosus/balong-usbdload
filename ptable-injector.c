//   Das Programm für die Ersatz Tabellen Abschnitte in der Bootloader usbloader
// 
// 
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "parts.h"

  
//############################################################################################################3

void main(int argc, char* argv[]) {
  
  
int opt;  
int mflag=0;
char ptfile[100];
int rflag=0,xflag=0;

uint32_t ptaddr;
struct ptable_t ptable;

FILE* ldr;
FILE* out;
FILE* in;

while ((opt = getopt(argc, argv, "mr:hx")) != -1) {
  switch (opt) {
   case 'h': 
     
printf("\n Dienstprogramm für die Ersatz Tabellen Abschnitte in der Lader usbloader\
\n Modem sollte sei in der Modus fastboot\
\n\n\
%s [Schlüssel] <Name Datei usbloader>\n\n\
 Akzeptabel das Folgende Schlüssel:\n\n\
-m       - zeigen aktuell Karte Abschnitte in der usbloader\n\
-x       - auswerfen aktuell Karte in der Datei ptable.bin\n\
-r <file>- ersetzen Karte Abschnitte von Karte von der die Datei\n\
\n",argv[0]);
    return;
    
   case 'm':
    mflag=1;
    break;
    
   case 'x':
    xflag=1;
    break;
    
   case 'r':
     rflag=1;
     strcpy (ptfile,optarg);
     break;
     
   case '?':
   case ':':  
     return;
  
  }  
}  
if (optind>=argc) {
    printf("\n - Nicht angezeigt Name Datei Bootloader\n");
    return;
}  

ldr=fopen(argv[optind],"r+");
if (ldr == 0) {
  printf("\n Fehler Entdeckungen Datei %s\n",argv[optind]);
  return;
}

 
// Auf der Suche nach Diagramm Abschnitte in der Datei Bootloader  

ptaddr=find_ptable(ldr);
if (ptaddr == 0) {
  printf("\n Tabelle Abschnitte in der Bootloader nicht gefunden\n");
  return ;
}
// lesen aktuell Diagramm
fread(&ptable,sizeof(ptable),1,ldr);

if (xflag) {
   out=fopen("ptable.bin","w");
   fwrite(&ptable,sizeof(ptable),1,out);
   fclose(out);
}   

if (mflag) {
  show_map(ptable);
}

if (mflag | xflag) return;

  
if (rflag) { 
  in=fopen(ptfile,"r");
  if (in == 0) {
    printf("\n Fehler Entdeckungen Datei %s",ptfile);
    return;
  }
  fread(&ptable,sizeof(ptable),1,in);
  fclose(in);
  
  // überprüfen Datei
  if (memcmp(ptable.head,headmagic,16) != 0) {
    printf("\n Eingabe Datei nicht ist das Tabelle Abschnitte\n");
    return;
  }
  fseek(ldr,ptaddr,SEEK_SET);
  fwrite(&ptable,sizeof(ptable),1,ldr);
  fclose(ldr);
  
}  
}
