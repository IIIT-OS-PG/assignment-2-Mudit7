#include "includes.h"
#include <unordered_map>
using namespace std;
void *servicethread(void *sock);

//  filename,list of ports
unordered_map<string,vector<string> > filePortMap;
unordered_map<string,int > sizeMap;
unordered_map<string,User> portUserMap;


// ./tracker 4000(tracker port)
int main(int argc,char *argv[])
{
  
    int serverPortNum;
    if(argc<2) 
    {
        cout<<"Using defaut port number "<<serverPortNum<<endl;   
        serverPortNum=4000;
    }
    else
    {
        serverPortNum=atoi(argv[1]);
    }

    // make the tracker socket (primary)
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
    serv_addr.sin_port = htons(serverPortNum);
    serv_addr.sin_addr.s_addr=inet_addr("127.0.0.1");
    int addrlen=sizeof(serv_addr);

    bind(sock,(sock_t*)&serv_addr,sizeof(sock_t));  
    int status=listen(sock,50);
    cout<<"tracker initialised\n";
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
   
    return 0;
}

string lookupPorts(string filename)
{
    string portlist;
    vector<string> ports;
    //lookup for ports in stored data.. hardcoding here
    ports=filePortMap[filename];
    portlist=makemsg(ports);
    return portlist;
}
int lookupFileSize(string filename)
{
    int filesize=sizeMap[filename];
    return filesize;
}

void trackerProcessReq(string buffer,int sockfd)
{
    // cases for all kinds of requests
    vector<string> req;
    req=splitStringOnHash(buffer);
    string msg;
    cout<<"command recieved="<<req[1]<<endl;
    if(req[1]=="login")
    {
        //check userid and passwd
        if((portUserMap[req[0]].user_id==req[2])&&((portUserMap[req[0]].passwd==req[3])))
        {
            portUserMap[req[0]].islogged=true;
            cout<<req[2]<<" login successful\n";
            msg="login#success";
        }
        else
        {
            cout<<"Invalid credentials, Couldn't login\n";
            msg="login#failed";
        }
        if(send (sockfd , (void*)msg.c_str(), (size_t)msg.size(), 0 )<0){
            cout<<"Not sent\n";
            perror("send");
            return;
        }

    }
    else if(req[1]=="create_user")
    {
        User u;
        u.user_id=req[2];
        u.passwd=req[3];
        u.islogged=false;
        portUserMap[req[0]]=u;
        cout<<"User added\n";
        string msg="create_user#success";
        if(send (sockfd , (void*)msg.c_str(), (size_t)msg.size(), 0 )<0){
            cout<<"Not sent\n";
            perror("send");
            return;
        }
    }
    else if(!portUserMap[req[0]].islogged){
        //cout<<"Login First!\n";
        string msg="login#incomplete";
        if(send (sockfd , (void*)msg.c_str(), (size_t)msg.size(), 0 )<0){
            cout<<"Not sent\n";
            perror("send");
        }
        return;
    } 
    else if(req[1]=="download")
    {
        int senderPort=atoi(req[0].c_str());
        string filename=req[2];
        
        //get list of peer ports containing that file
        string portList=lookupPorts(filename);
        int filesize=lookupFileSize(filename);
        vector<string> msg_v;
        msg_v.push_back(filename);
        msg_v.push_back(to_string(filesize));
        msg_v.push_back("portList");
        msg_v.push_back(portList);

        string msg=makemsg(msg_v);
        msg=msg.substr(0,msg.length()-2);

        if(send (sockfd , (void*)msg.c_str(), (size_t)msg.size(), 0 )<0){
            cout<<"Not sent\n";
            perror("send");
            return;
        }
    }
    else if(req[1]=="upload")
    {
        cout<<"upload request recieved\n";
        string filename=req[2];
        string filesize=req[3];
        if(req.size()>4)
        {
            string SHA=req[4];
            cout<<"SHA to be uploaded\n";
        }
        string portNum=req[0];
        cout<<"adding "<<filename<<" to the maps\n"<<endl;
        filePortMap[filename].push_back(portNum);
        sizeMap[filename]=atoi(filesize.c_str());
    }
    
    
    else if(req[1]=="logout")
    {
        portUserMap[req[0]].islogged=false;
        cout<<"logout successful";
    }
}

void *servicethread(void *sockNum)
{
    cout<<"new thread created.. "<<endl;
    
    int sockfd=*((int*)sockNum);

    while(1)
    {
        char buffer[100]={0};
        int n=0;
        while (( n = recv(sockfd , buffer ,100, 0) ) > 0 ){
          
            cout<<"Command Recieved:";
            cout<<buffer<<endl;
            trackerProcessReq(string((char*)buffer),sockfd);
           // processReq("download#dsf");        
            memset (buffer, '\0', 100);
        }
    }
    cout<<"Thread Exiting..\n";
    return NULL;
}