#define _GNU_SOURCE
#include <stdio.h>
#include <sys/socket.h>
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
int main(int argc, char *argv[]){
    FILE *inp,*outp;
    if(argc != 3){
        perror("Usage: extract <input file> <ouptut file>");
        return 1;
    }
    if( (inp=fopen(argv[1], "rb")) == 0){
        perror("Cannot find input file");
        return 2;
    }
    if( (outp=fopen(argv[2], "wb")) == 0){
        perror("Cannot create/open output file");

        return 3;
    }

    struct ip *header=malloc(sizeof(struct ip));
    struct tcphdr *TCPheader=malloc(sizeof(struct ip));
    int buffSize=1024;
    char *data=malloc(buffSize*sizeof(char));
    int currPos=0;
    while(!feof(inp)){
        int rd=fread(header,sizeof(struct ip),1,inp);
        if(rd){//read can be 0, so needs an explicit check
            //ip_len is for both data and header
            //so substract header length (as it's already read), to leave out just data
            //header length is measured in 32 parts, so neet to do conversion to bytes
            //*32/8 = just *4 = <<2

            if(header->ip_hl == 6){
                fseek(inp, 4, SEEK_CUR);
            }
            rd=fread(TCPheader,sizeof(struct tcphdr),1,inp);
            if(!rd){
                perror("Read was corrupted: no TCP header after IP header");
                return 3;
            }
            //data measured in bytes
            int total=ntohs(header->ip_len);
            int headers=((header->ip_hl+TCPheader->th_off)<<2);
            int dataSize=total-headers;


            //realloc data if the buffer is too small
            if(dataSize>buffSize){
                char *newdata=realloc(data,dataSize*sizeof(char));
                data=newdata;
                buffSize=dataSize;

            }


            //read and write data
            currPos+=headers;
            fseek(inp,currPos,SEEK_SET);
            int n=fread(data,sizeof(char),dataSize,inp);
            fwrite(data,sizeof(char),n,outp);
            currPos+=dataSize;
        }
    }
    fclose(inp);
    fclose(outp);
}
