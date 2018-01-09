#define _GNU_SOURCE
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
//answer is
//128.232.1.219 128.232.9.6 5 60 10 8
int main(int argc, char *argv[]){
    FILE *fp;
    if(argc != 2){
        perror("Usage: summary <file>");
        return 1;
    }
    if( (fp=fopen(argv[1], "rb")) == 0){
        perror("Cannot find log file");
        return 2;
    }
    struct ip *header=malloc(sizeof(struct ip));
    struct tcphdr *TCPheader=malloc(sizeof(struct tcphdr));

    fread(header,sizeof(struct ip),1,fp);
    //Options may be present if HL is 6 32bitwords, but fread may not read them
    if(header->ip_hl == 6){
        fseek(fp, 4, SEEK_CUR);
    }
    fread(TCPheader,sizeof(struct tcphdr),1,fp);
    fseek(fp,ntohs(header->ip_len),SEEK_SET);//skip to end of data for the next part
 
    char *src = malloc(sizeof(char)*(INET_ADDRSTRLEN));
    inet_ntop(AF_INET, &(header->ip_src), src, INET_ADDRSTRLEN);
    char *dst= malloc(sizeof(char)*(INET_ADDRSTRLEN));
    inet_ntop(AF_INET, &(header->ip_dst), dst, INET_ADDRSTRLEN);
    printf("%s %s %d %d %d",src,dst,header->ip_hl,ntohs(header->ip_len),TCPheader->th_off);

    int cnt=1;//count starts from 1 as we already had one
    while(!feof(fp)){
        int rd=fread(header,sizeof(struct ip),1,fp);
        //same as above
        if(header->ip_hl == 6){
            fseek(fp, 4, SEEK_CUR);
        }
        if(rd){//read can be 0, so needs an explicit check
            //ip_len is for both data and header
            //so substract header length (as it's already read), to leave out just data
            //header length is measured in 32 parts, so neet to do conversion to bytes
            //*32/8 = just *4 = <<2
            fseek(fp,ntohs(header->ip_len)-((header->ip_hl)<<2),SEEK_CUR);//skip the data
            cnt++;
        }
        //printf("%d\n",sum);
    }
    //header and TCPheader are the first 2 packets
    printf(" %d\n",cnt);
    fclose(fp);
}
