#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include "h324m.h"



unsigned char env[] =
  {
   0x87, 0xb2, 0x00, 0x00, 0x00, 0x87, 0xb2, 0x00, 0x00, 0x00, 0x87, 0xb2, 0x00, 0x00, 0x00,
   0x87, 0xb2, 0x00, 0x00, 0x00, 0x87, 0xb2, 0x00, 0x00, 0x00, 0x87, 0xb2, 0x00, 0x00, 0x00,
   0x87, 0xb2, 0x00, 0x00, 0x00, 0x87, 0xb2, 0x00, 0x00, 0x00, 0x87, 0xb2, 0x00, 0x00, 0x00,
   0x87, 0xb2, 0x00, 0x00, 0x00, 0x87, 0xb2, 0x00, 0x00, 0x00, 0x87, 0xb2, 0x00, 0x00, 0x00,
   0x87, 0xb2, 0x00, 0x00, 0x00, 0x87, 0xb2, 0x00, 0x00, 0x00, 0x87, 0xb2, 0x00, 0x00, 0x00,
   0x87, 0xb2, 0x00, 0x00, 0x00, 0x87, 0xb2, 0x00, 0x00, 0x00, 0x87, 0xb2, 0x00, 0x00, 0x00,
   0x87, 0xb2, 0x00, 0x00, 0x00, 0x87, 0xb2, 0x00, 0x00, 0x00, 0x87, 0xb2, 0x00, 0x00, 0x00,
   0x87, 0xb2, 0x00, 0x00, 0x00, 0x87, 0xb2, 0x00, 0x00, 0x00, 0x87, 0xb2, 0x00, 0x00, 0x00,
   0x87, 0xb2, 0x00, 0x00, 0x00, 0x87, 0xb2, 0x00, 0x00, 0x00, 0x87, 0xb2, 0x00, 0x00, 0x00,
   0x87, 0xb2, 0x00, 0x00, 0x00, 0x87, 0xb2, 0x00, 0x00, 0x00, 0x87, 0xb2, 0x00, 0x00, 0x00,
   0x87, 0xb2, 0x00, 0x00, 0x00, 0x87, 0xb2, 0x00, 0x00, 0x00};


/*
static int myaffiche(unsigned char *buf, int len)
{
  int i,m=0;
  for(i = 0; i <len; i++){
    //printf("%02x ",reverse(buf[i]));
    printf("%02x ",buf[i]);
    if (m == 15){
      printf("\n");
      m = 0;
    }
    else m++;
  }
  if(len)
    printf("\n");
  return 0;
}
*/

const unsigned char tmplate[16][20] = {
"AT S7=45 S0=0 L1 V1 X4 &c1 E1 Q0 \r\n"
"ATZ \r\n",
"AT&F \r\n",
"AT&C1 \r\n",
"ATE1 \r\n",
"ATQ0 \r\n",
"ATV1 \r\n",
"AT+CMEE=1 \r\n",
"AT+CPBS=\"SM\" \r\n",
"AT+CBST=134,1,0 \r\n",
"AT+CLIP=1 \r\n",
"AT+CLIR=1 \r\n",
"ATS10=10 \r\n",
"ATS0=0 \r\n",
"ATS7=60 \r\n",
"ATS0=1 \r\n",
"ATD0634065160 \r\n", //Compose le numero ici
};




int main(int argc, char *argv[])
{
  h324m_session *session;
  session = new_session();
  FILE *out, *in;
  unsigned char buffer[4096];
  int *fd, *fdb;
  unsigned int m,n,i,k=0;
  
  out =fopen("output.amr","wb");
  fprintf(out,"#!AMR\n");
  fclose(out);
  out = fopen("Received.bin","wb");
  in = fopen("Sent.bin","wb");

  printf("AMIN RAMTIN, TUNNEL \n");
  fd = open("/dev/ttyUSB2",O_RDWR);

  for(i = 0; i < 15; i++){
    n =  write(fd,tmplate[i],strlen(tmplate[i]));
    printf("%d: Ecrit %d %s\n",i,n,tmplate[i]);
    usleep(50000);
    bzero(buffer,256);
    n = read(fd,buffer,256);
    printf("Lu %d %s\n",n,buffer);
    bzero(buffer,256);
  }
  printf("READY\n");
  
  fdb = open("/dev/ttyUSB0",O_RDWR);  

  while(1){
    n = read(fd,buffer,4096);
    k+= n;
    h324m_OnReceived(session,buffer,n);
    //myaffiche(buffer,n);
    fwrite(buffer,1,n,out);
    //    h324m_TreatPackets(session);
    if(session->audiolen)
      myaffiche(session->audiobuf, session->audiolen);
    
    if (k >= 160){
      k = 0;
      h324m_MakePdu(session);
      m = write(fdb,session->Output,160);
      fwrite(session->Output,1,160,in);
    }
  }
}



