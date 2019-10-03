#include "includes.h"

using namespace std;



void *servicethread(void *sock);

int main(int argc,char *argv[])
{
    if(argc<2) 
    {
        cout<<"Enter the port number next time!\nExiting...\n";
        return 0;
    }
    int sock = 0;
    sockin_t serv_addr;
    pthread_t thread_id;

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket creation error in client side\n");
        return -1;
    }
    memset(&serv_addr, '0', sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(atoi(argv[1]));
    serv_addr.sin_addr.s_addr=inet_addr("127.0.0.1");

    int addrlen=sizeof(serv_addr);

    bind(sock,(sock_t*)&serv_addr,sizeof(sock_t));
    
    int status=listen(sock,5);

    //one thread per client
    while(1)
    {
        int client_sockfd=accept(sock,(sock_t*)&serv_addr,(socklen_t*)&addrlen);
        //cout<<"incoming!!\n";
        if(client_sockfd<0)
        {
            perror("accept");
        }
        if (pthread_create(&thread_id, NULL, servicethread, (void *)&client_sockfd) < 0)
        {
            perror("\ncould not create thread\n");
        }
       
    }
    // close(client_sockfd);
    // close(sock);
    // fclose(f);
    return 0;
}

void *servicethread(void *sockNum)
{
    cout<<"new thread created.. "<<endl;
    // int filesize;
    // recv(client_sockfd,&filesize,sizeof(filesize),0);
    // FILE *f=fopen("recvFile","wb");
    int sockfd=*((int*)sockNum);
    //vector<string> hashes;
    //cout<<sockfd<<endl;


    char buffer[100]={0};
    int n=0;
    while (( n = recv(sockfd , buffer ,100, 0) ) > 0 ){
        //hashes.push_back(buffer);
        cout<<buffer<<endl;
        //fwrite (buffer , sizeof (char), n, f);
        memset (buffer, '\0', 100);
    }
    cout<<"Thread Exiting..\n";
    return NULL;
}