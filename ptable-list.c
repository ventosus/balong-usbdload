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
  
  

struct ptable_t ptable;
FILE* in;

if (argc != 2) {
    printf("\n - Nicht angezeigt Name Datei mit dem Tabelle Abschnitte\n");
    return;
}  

in=fopen(argv[optind],"r+");
if (in == 0) {
  printf("\n Fehler Entdeckungen Datei %s\n",argv[optind]);
  return;
}

 
// lesen aktuell Diagramm
fread(&ptable,sizeof(ptable),1,in);

if (strncmp(ptable.head, "pTableHead", 16) != 0) {
  printf("\n Datei nicht ist das Tabelle Abschnitte\n");
  return ;
}
  
show_map(ptable);
}
