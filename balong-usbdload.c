// Uploader usbloader.bin durch die Notfall Hafen für die Modems von Plattform Balong V7R2.
//
//
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#ifndef WIN32
//%%%%
#include <termios.h>
#include <unistd.h>
#include <arpa/inet.h>
#else
//%%%%
#include <windows.h>
#include <setupapi.h>
#include "getopt.h"
#include "printf.h"
#endif

#include "parts.h"
#include "patcher.h"


#ifndef WIN32
int siofd;
struct termios sioparm;
#else
static HANDLE hSerial;
#endif
FILE* ldr;


//*************************************************
//* HEX-ablegen Bereich von Speicher                       *
//*************************************************

void dump(unsigned char buffer[],int len) {
int i,j;
unsigned char ch;

printf("\n");
for (i=0;i<len;i+=16) {
  printf("%04x: ",i);
  for (j=0;j<16;j++){
   if ((i+j) < len) printf("%02x ",buffer[i+j]&0xff);
   else printf("   ");}
  printf(" *");
  for (j=0;j<16;j++) {
   if ((i+j) < len) {
    // Transformation Byte für die Charakter anzeigen
    ch=buffer[i+j];
    if ((ch < 0x20)||((ch > 0x7e)&&(ch<0xc0))) putchar('.');
    else putchar(ch);
   } 
   // ausfüllen Räume für die unvollständig der Begriff 
   else printf(" ");
  }
  printf("*\n");
 }
}


//*************************************************
//* Расчет Kontrolle Summen Befehl Paket
//*************************************************
void csum(unsigned char* buf, int len) {

unsigned  int i,c,csum=0;

unsigned int cconst[]={0,0x1021, 0x2042, 0x3063, 0x4084, 0x50A5, 0x60C6, 0x70E7, 0x8108, 0x9129, 0xA14A, 0xB16B, 0xC18C, 0xD1AD, 0xE1CE, 0xF1EF};

for (i=0;i<(len-2);i++) {
  c=(buf[i]&0xff);
  csum=((csum<<4)&0xffff)^cconst[(c>>4)^(csum>>12)];
  csum=((csum<<4)&0xffff)^cconst[(c&0xf)^(csum>>12)];
}  
buf[len-2]=(csum>>8)&0xff;
buf[len-1]=csum&0xff;
  
}

//*************************************************
//*   Senden Befehl Paket Modem
//*************************************************
int sendcmd(unsigned char* cmdbuf, int len) {

unsigned char replybuf[1024];
unsigned int replylen;

#ifndef WIN32
csum(cmdbuf,len);
write(siofd,cmdbuf,len);  // Referenz Teams
tcdrain(siofd);
replylen=read(siofd,replybuf,1024);
#else
    DWORD bytes_written = 0;
    DWORD t;

    csum(cmdbuf, len);
    WriteFile(hSerial, cmdbuf, len, &bytes_written, NULL);
    FlushFileBuffers(hSerial);

    t = GetTickCount();
    do {
        ReadFile(hSerial, replybuf, 1024, (LPDWORD)&replylen, NULL);
    } while (replylen == 0 && GetTickCount() - t < 1000);
#endif
if (replylen == 0) return 0;    
if (replybuf[0] == 0xaa) return 1;
return 0;
}

//*************************************
// Eröffnung und tunen konsistent der Hafen
//*************************************

int open_port(char* devname) {

//============= Linux ========================  
#ifndef WIN32

int i,dflag=1;
char devstr[200]={0};

// Stattdessen voll Name Geräte ist erlaubt übertragen nur Nummer ttyUSB-der Hafen

// Überprüfung Name Geräte von Verfügbarkeit nicht digital Symbole
for(i=0;i<strlen(devname);i++) {
  if ((devname[i]<'0') || (devname[i]>'9')) dflag=0;
}
// Wenn die in der Linie - nur Ziffern, hinzufügen Präfix /dev/ttyUSB
if (dflag) strcpy(devstr,"/dev/ttyUSB");
// kopieren Name Geräte
strcat(devstr,devname);

siofd = open(devstr, O_RDWR | O_NOCTTY |O_SYNC);
if (siofd == -1) return 0;

bzero(&sioparm, sizeof(sioparm)); // vorbereiten Block Attribute termios
sioparm.c_cflag = B115200 | CS8 | CLOCAL | CREAD ;
sioparm.c_iflag = 0;  // INPCK;
sioparm.c_oflag = 0;
sioparm.c_lflag = 0;
sioparm.c_cc[VTIME]=30; // timeout  
sioparm.c_cc[VMIN]=0;  
tcsetattr(siofd, TCSANOW, &sioparm);
return 1;

//============= Win32 ========================  
#else
    char device[20] = "\\\\.\\COM";
    DCB dcbSerialParams = {0};
    COMMTIMEOUTS CommTimeouts;

    strcat(device, devname);
    
    hSerial = CreateFileA(device, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    if (hSerial == INVALID_HANDLE_VALUE)
        return 0;

    ZeroMemory(&dcbSerialParams, sizeof(dcbSerialParams));
    dcbSerialParams.DCBlength=sizeof(dcbSerialParams);
    dcbSerialParams.BaudRate=CBR_115200;
    dcbSerialParams.ByteSize=8;
    dcbSerialParams.StopBits=ONESTOPBIT;
    dcbSerialParams.Parity=NOPARITY;
    dcbSerialParams.fBinary = TRUE;
    dcbSerialParams.fDtrControl = DTR_CONTROL_ENABLE;
    dcbSerialParams.fRtsControl = RTS_CONTROL_ENABLE;
    if(!SetCommState(hSerial, &dcbSerialParams))
    {
        CloseHandle(hSerial);
        return 0;
    }

    CommTimeouts.ReadIntervalTimeout = MAXDWORD;
    CommTimeouts.ReadTotalTimeoutConstant = 0;
    CommTimeouts.ReadTotalTimeoutMultiplier = 0;
    CommTimeouts.WriteTotalTimeoutConstant = 0;
    CommTimeouts.WriteTotalTimeoutMultiplier = 0;
    if (!SetCommTimeouts(hSerial, &CommTimeouts))
    {
        CloseHandle(hSerial);
        return 0;
    }

    return 1;
#endif
}

//*************************************
//* Suche linux-Kern in der Bild Abschnitt
//*************************************
int locate_kernel(char* pbuf, uint32_t size) {
  
int off;

for(off=(size-8);off>0;off--) {
  if (strncmp(pbuf+off,"ANDROID!",8) == 0) return off;
}
return 0;
}

#ifdef WIN32

DEFINE_GUID(GUID_DEVCLASS_PORTS, 0x4D36E978, 0xE325, 0x11CE, 0xBF, 0xC1, 0x08, 0x00, 0x2B, 0xE1, 0x03, 0x18);

static int find_port(int* port_no, char* port_name)
{
  HDEVINFO device_info_set;
  DWORD member_index = 0;
  SP_DEVINFO_DATA device_info_data;
  DWORD reg_data_type;
  char property_buffer[256];
  DWORD required_size;
  char* p;
  int result = 1;

  device_info_set = SetupDiGetClassDevs(&GUID_DEVCLASS_PORTS, NULL, 0, DIGCF_PRESENT);

  if (device_info_set == INVALID_HANDLE_VALUE)
    return result;

  while (TRUE)
  {
    ZeroMemory(&device_info_data, sizeof(SP_DEVINFO_DATA));
    device_info_data.cbSize = sizeof(SP_DEVINFO_DATA);

    if (!SetupDiEnumDeviceInfo(device_info_set, member_index, &device_info_data))
      break;

    member_index++;

    if (!SetupDiGetDeviceRegistryPropertyA(device_info_set, &device_info_data, SPDRP_HARDWAREID,
             &reg_data_type, (PBYTE)property_buffer, sizeof(property_buffer), &required_size))
      continue;

    if (strstr(_strupr(property_buffer), "VID_12D1&PID_1443") != NULL)
    {
      if (SetupDiGetDeviceRegistryPropertyA(device_info_set, &device_info_data, SPDRP_FRIENDLYNAME,
              &reg_data_type, (PBYTE)property_buffer, sizeof(property_buffer), &required_size))
      {
        p = strstr(property_buffer, " (COM");
        if (p != NULL)
        {
          *port_no = atoi(p + 5);
          strcpy(port_name, property_buffer);
          result = 0;
        }
      }
      break;
    }
  }

  SetupDiDestroyDeviceInfoList(device_info_set);

  return result;
}

#endif

//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

void main(int argc, char* argv[]) {

unsigned int i,res,opt,datasize,pktcount,adr;
int bl;    // aktuell Block
unsigned char c;
int fbflag=0, tflag=0, mflag=0, bflag=0, cflag=0;
int koff;  // Vorurteil zu ANDROID-Kopfzeile
char ptfile[100];

FILE* pt;
char ptbuf[2048];
uint32_t ptoff;

struct ptable_t* ptable;

unsigned char cmdhead[14]={0xfe,0, 0xff};
unsigned char cmddata[1040]={0xda,0,0};
unsigned char cmdeod[5]={0xed,0,0,0,0};

// Liste Abschnitte, welche brauche etablieren Datei Flagge
uint8_t fileflag[41];

struct {
  int lmode;  // Modus Downloads: 1 - direkt start, 2 - durch die neu starten A-core
  int size;   // Größe Komponente
  int adr;    // Adresse Downloads Komponente in der Speicher
  int offset; // Vorurteil zu Komponente aus start Datei
  char* pbuf; // Puffer für die Downloads Bild Komponente
} blk[10];


#ifndef WIN32
unsigned char devname[50]="/dev/ttyUSB0";
#else
char devname[50]="";
DWORD bytes_written, bytes_read;
int port_no;
char port_name[256];
#endif

#ifndef WIN32
bzero(fileflag,sizeof(fileflag));
#else
memset(fileflag, 0, sizeof(fileflag));
#endif

while ((opt = getopt(argc, argv, "hp:ft:ms:bc")) != -1) {
  switch (opt) {
   case 'h': 
     
printf("\n Dienstprogramm ist beabsichtigt für die Notfall USB-Downloads Geräte von mach es Balong V7\n\n\
%s [Schlüssel] <Name Datei für die Downloads>\n\n\
 Akzeptabel das Folgende Schlüssel:\n\n"
#ifndef WIN32
"-p <tty> - aufeinanderfolgend Hafen für die Kommunikation mit dem Bootloader (auf dem Standard /dev/ttyUSB0)\n"
#else
"-p <tty> - aufeinanderfolgend Hafen für die Kommunikation mit dem Bootloader\n"
#endif
"-f       - Schiff usbloader nur zu fastboot (ohne Starten Linux)\n\
-b       - ähnlich -f, zusätzlich deaktivieren testen defekt Blöcke an löschen\n\
-t <file>- zu nehmen Diagramm Abschnitte von der die Datei\n\
-m       - zeigen Diagramm Abschnitte Bootloader und fertig die Arbeit\n\
-s n     - etablieren Datei Flagge für die Abschnitt n (Hinweis du kannst angeben mehrere mal)\n\
-c       - nicht produzieren automatisch patch Auslöschungen Abschnitte\n\
\n",argv[0]);
    return;

   case 'p':
    strcpy(devname,optarg);
    break;

   case 'f':
     fbflag=1;
     break;

   case 'c':
     cflag=1;
     break;

   case 'b':
     fbflag=1;
     bflag=1;
     break;

   case 'm':
     mflag=1;
     break;

   case 't':
     tflag=1;
     strcpy(ptfile,optarg);
     break;

   case 's':
     i=atoi(optarg);
     if (i>41) {
       printf("\n Abschnitt #%i nicht da ist\n",i);
       return;
     }
     fileflag[i]=1;
     break;
     
   case '?':
   case ':':  
     return;
    
  }
}  

printf("\n Notfall USB-Bootloader Balong-Chipsatz, Version 2.03, (c) forth32, 2015");
#ifdef WIN32
printf("\n Hafen für die Windows 32bit  (c) rust3028, 2016");
#endif


if (optind>=argc) {
    printf("\n - Nicht angezeigt Name Datei für die Downloads\n");
    return;
}  

ldr=fopen(argv[optind],"rb");
if (ldr == 0) {
  printf("\n Fehler Entdeckungen %s",argv[optind]);
  return;
}

// Bestätigen Unterschrift usloader
fread(&i,1,4,ldr);
if (i != 0x20000) {
  printf("\n Datei %s nicht ist das Bootloader usbloader\n",argv[optind]);
  return;
}  

fseek(ldr,36,SEEK_SET); // Zuhause Deskriptoren Blöcke für die Downloads

// Verstehen Beschriftung

fread(&blk[0],1,16,ldr);  // raminit
fread(&blk[1],1,16,ldr);  // usbldr

//---------------------------------------------------------------------
// Lesen Komponenten in der Speicher
for(bl=0;bl<2;bl++) {

  // hervorheben Speicher unter dem fertig Bild Abschnitt
  blk[bl].pbuf=(char*)malloc(blk[bl].size);

  // lesen Bild Abschnitt in der Speicher
  fseek(ldr,blk[bl].offset,SEEK_SET);
  res=fread(blk[bl].pbuf,1,blk[bl].size,ldr);
  if (res != blk[bl].size) {
      printf("\n Unerwartet das Ende Datei: lesen %i wurde erwartet %i\n",res,blk[bl].size);
      return;
  }
  if (bl == 0) continue; // für die raminit mehr als nichts machen nicht brauche

  //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%    
  // fastboot-patch
  if (fbflag) {
    koff=locate_kernel(blk[bl].pbuf,blk[bl].size);
    if (koff != 0) {
      blk[bl].pbuf[koff]=0x55; // patch Signaturen
      blk[bl].size=koff+8; // schneiden Abschnitt zu start Kern
    }
    else {
        printf("\n In der Bootloader Nein ANDROID-Komponente - fastboot-Laden ist unmöglich\n");
        exit(0);
    }    
  }  

  //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%    
  // Auf der Suche nach Diagramm Abschnitte in der Bootloader
  ptoff=find_ptable_ram(blk[bl].pbuf,blk[bl].size);
  
  //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%    
  // patch Tabellen Abschnitte
  if (tflag) {
    pt=fopen(ptfile,"rb");
    if (pt == 0) { 
      printf("\n Nicht gefunden Datei %s - Substitution Tabellen Abschnitte ist unmöglich\n",ptfile);
      return;
    }  
    fread(ptbuf,1,2048,pt);
    fclose(pt);
    if (memcmp(headmagic,ptbuf,sizeof(headmagic)) != 0) {
      printf("\n Datei %s nicht ist das Tabelle Abschnitte\n",ptfile);
      return;
    }  
    if (ptoff == 0) {
          printf("\n In der Bootloader nicht gefunden Diagramm Abschnitte - Substitution ist unmöglich");
	  return;
    }
    memcpy(blk[bl].pbuf+ptoff,ptbuf,2048);
  }
  ptable=(struct ptable_t*)(blk[bl].pbuf+ptoff);
  
  //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%    
  // Patch Datei Flaggen
  for(i=0;i<41;i++) {
    if (fileflag[i]) {
      ptable->part[i].nproperty |= 1;
    }  
  }  

  //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%    
  // Fazit Tabellen Abschnitte
  if (mflag) {
    if (ptoff == 0) {
      printf("\n Tabelle Abschnitte nicht gefunden - Schlussfolgerung Karten unmöglich\n");
      return;
    }
    show_map(*ptable);
    return;
  }

  // Patch erase-Verfahren von Objekt zu ignorieren Rückschläge
  if (bflag) {
    res=perasebad(blk[bl].pbuf, blk[bl].size);
    if (res == 0) { 
      printf("\n! Nicht gefunden Signatur isbad - Laden ist unmöglich\n");  
      return;
    }  
  }
  // Löschen Verfahren flash_eraseall
  if (!cflag) {
    res=pv7r2(blk[bl].pbuf, blk[bl].size)+ pv7r11(blk[bl].pbuf, blk[bl].size) + pv7r1(blk[bl].pbuf, blk[bl].size) + pv7r22(blk[bl].pbuf, blk[bl].size) + pv7r22_2(blk[bl].pbuf, blk[bl].size);
   if (res != 0)  printf("\n\n * Entfernt Verfahren flash_eraseal auf dem Vorurteil %08x",res);
   else {
       printf("\n Verfahren eraseall nicht gefunden in der Bootloader - verwenden Hinweis -mit dem für die Downloads ohne patch!\n");
       return;
   }    
  }   
     
  
}

//---------------------------------------------------------------------

#ifdef WIN32
if (*devname == '\0')
{
  printf("\n\nSuche der Hafen Notfall Downloads...\n");
  
  if (find_port(&port_no, port_name) == 0)
  {
    sprintf(devname, "%d", port_no);
    printf("Hafen: \"%s\"\n", port_name);
  }
  else
  {
    printf("Hafen nicht erkannt!\n");
    return;
  }
}
#endif

if (!open_port(devname)) {
  printf("\n Seriell Hafen nicht öffnet sich\n");
  return;
}  


// Überprüfung bootfähig Hafen
c=0;
#ifndef WIN32
write(siofd,"A",1);
res=read(siofd,&c,1);
#else
    WriteFile(hSerial, "A", 1, &bytes_written, NULL);
    FlushFileBuffers(hSerial);
    Sleep(100);
    ReadFile(hSerial, &c, 1, &bytes_read, NULL);
#endif
if (c != 0x55) {
  printf("\n ! Hafen nicht befindet sich in der Modus USB Boot\n");
  return;
}  

//----------------------------------
// Prinzipal Zyklus Downloads - herunterladen all das Blöcke, gefunden in der Titel
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%    

printf("\n\n Komponente    Adresse    Größe   %%Downloads\n------------------------------------------\n");

for(bl=0;bl<2;bl++) {

  datasize=1024;
  pktcount=1;


  // wir bilden Charge start Block  
  *((unsigned int*)&cmdhead[4])=htonl(blk[bl].size);
  *((unsigned int*)&cmdhead[8])=htonl(blk[bl].adr);
  cmdhead[3]=blk[bl].lmode;
  
  // senden Charge start Block
  res=sendcmd(cmdhead,14);
  if (!res) {
    printf("\nModem abgelehnt Charge Kopfzeile\n");
    return;
  }  

  
  // ---------- Zyklus blockieren Downloads von Daten ---------------------
  for(adr=0;adr<blk[bl].size;adr+=1024) {

    // Form Größe das letzte herunterladbar Paket
    if ((adr+1024)>=blk[bl].size) datasize=blk[bl].size-adr;  

    printf("\r %s    %08x %8i   %i%%",bl?"usbboot":"raminit",blk[bl].adr,blk[bl].size,(adr+datasize)*100/blk[bl].size); 
  
    // vorbereiten Charge von Daten
    cmddata[1]=pktcount;
    cmddata[2]=(~pktcount)&0xff;
    memcpy(cmddata+3,blk[bl].pbuf+adr,datasize);
    
    pktcount++;
    if (!sendcmd(cmddata,datasize+5)) {
      printf("\nModem abgelehnt Charge von Daten");
      return;
    }  
  }
  free(blk[bl].pbuf);

  // Wir bilden Charge das Ende von Daten
  cmdeod[1]=pktcount;
  cmdeod[2]=(~pktcount)&0xff;

  if (!sendcmd(cmdeod,5)) {
    printf("\nModem abgelehnt Charge das Ende von Daten");
  }
printf("\n");  
} 
printf("\n Herunterladen ist vorbei\n");  
}


