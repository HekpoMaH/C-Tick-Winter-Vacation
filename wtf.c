#include <stdio.h>
//#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
//#include <unistd.h>
#define _GNU_SOURCE
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
typedef unsigned char byte;
 
int main(int argc, char *argv[]){
  //Check for correct usage
  if(argc != 2){
    puts("Usage: summary <file>");
    return 1;
  }
 
  //Open file
  FILE *fp;
  if ( (fp=fopen(argv[1], "rb")) == 0){
    perror("Cannot find log file");
    return 2;
  }
 
  //Read data
  struct ip *firstIP = malloc(sizeof(struct ip)); 
  struct tcphdr *firstTCP = malloc(sizeof(struct tcphdr));
 
  fseek(fp, 0, SEEK_SET);
  fread(firstIP, sizeof(struct ip), 1, fp);
 
  //Skip IP options if they exist (conform to spec)
  if(firstIP->ip_hl == 6){
    fseek(fp, 1, SEEK_CUR);
  }
  fread(firstTCP, sizeof(struct tcphdr), 1, fp);
 
  //Skip to end of this IP packet
  fseek(fp, ntohs(firstIP->ip_len), SEEK_SET);
 
  //Count number of IP packets
  int n = 1; //We already have the first
  int filePos = ntohs(firstIP->ip_len); //Position in file
 
  struct ip *packet = malloc(sizeof(struct ip)); //Current packet
 
  while(!feof(fp)){
    int r = fread(packet, sizeof(struct ip), 1, fp); //Read in packet
 
    if(r == 1){ //If 1 IP header was read
      n++; //Increment counter
 
      //Seek forward through data to next packet
      filePos += ntohs(packet->ip_len);
      fseek(fp, filePos, SEEK_SET);
    }
  }
   
  free(packet);
 
  printf("%s %s %d %d %d %d\n", 
      inet_ntoa(firstIP->ip_src), 
      inet_ntoa(firstIP->ip_dst), 
      firstIP->ip_hl,
      ntohs(firstIP->ip_len),
      firstTCP->th_off,
      n
  );
 
  return 0;
}
