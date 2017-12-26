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
    struct ip *IP_1=malloc(sizeof(struct ip));
    struct tcphdr *TCP_1=malloc(sizeof(struct tcphdr));

    fread(IP_1,sizeof(struct ip),1,fp);
    //Options may be present if HL is 6 32bitwords, but fread may not read them
    if(IP_1->ip_hl == 6){
        fseek(fp, 4, SEEK_CUR);
    }
    fread(TCP_1,sizeof(struct tcphdr),1,fp);
    fseek(fp,ntohs(IP_1->ip_len),SEEK_SET);//skip to end of data for the next part

    int cnt=1;//count starts from 1 as we already had one
    struct ip *next=malloc(sizeof(struct ip));
    while(!feof(fp)){
        int rd=fread(next,sizeof(struct ip),1,fp);
        //same as above
        if(next->ip_hl == 6){
            fseek(fp, 4, SEEK_CUR);
        }
        if(rd){//read can be 0, so needs an explicit check
            //ip_len is for both data and header
            //so substract header length (as it's already read), to leave out just data
            //header length is measured in 32 parts, so neet to do conversion to bytes
            //*32/8 = just *4 = <<2
            fseek(fp,ntohs(next->ip_len)-((next->ip_hl)<<2),SEEK_CUR);//skip the data
            cnt++;
        }
        //printf("%d\n",sum);
    }
    //IP_1 and TCP_1 are the first 2 packets
    printf("%s %s %d %d %d %d\n",inet_ntoa(IP_1->ip_src),inet_ntoa(IP_1->ip_dst),
            IP_1->ip_hl,
            ntohs(IP_1->ip_len),
            TCP_1->th_off,
            cnt);
    fclose(fp);
}
