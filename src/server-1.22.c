#include <netinet/in.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#define SERVER_BUFFER_SIZE      1024

static int const maxConnectionBacklog = 5;

struct msg {
  int32_t type;
  int32_t length;
  char * payload;
};

int main()
{
    int socketId = socket(PF_INET, SOCK_STREAM, 0);
    struct sockaddr_in serverAddr;
    bzero((char*)&serverAddr, sizeof(serverAddr));
    serverAddr.sin_family       = AF_INET;
    serverAddr.sin_port         = htons(8080);
    serverAddr.sin_addr.s_addr  = INADDR_ANY;
    bind(socketId, (struct sockaddr *) &serverAddr, sizeof(serverAddr));
    listen(socketId, maxConnectionBacklog);
    int finished    = 0;
    while(!finished)
    {
        struct sockaddr_storage serverStorage;
        socklen_t addr_size   = sizeof serverStorage;
        int  newSocket = accept(socketId, (struct sockaddr*)&serverStorage, &addr_size);
        char buffer[SERVER_BUFFER_SIZE];
        int  get = read(newSocket, buffer, SERVER_BUFFER_SIZE - 1);        
        if(get>7) {
          get=0;
          int32_t length=*(int32_t*)&buffer[4];
          int32_t type=*(int32_t*)&buffer[0];
          struct msg *request = (void*) malloc(8+length);
          request->type=type;
          request->length=length;
          memcpy(&request->payload,(void*)&buffer[8],request->length);
          
          // echo command (will echo back 4 bytes of the request):
          if(request->type==0) {
            struct msg *response = (struct msg*) malloc(8+4);
            response->length=4;
            response->type=0;
            memcpy(&response->payload,(void*)&request->payload,request->length);
            write(newSocket,(void *)response,8+response->length);
            free(response);
          }

          // random int command:
          else if(request->type==1) {
            struct msg *response = (struct msg*) malloc(3+4);
            response->length=4;
            response->type=1;
            srand(time(NULL));   // Initialization, should only be called once.
            int32_t r = rand(); 
            memcpy(&response->payload,(void*)&r,4);
            write(newSocket,(void *)response,8+response->length);
            free(response);
          }

          // pong command:
          else if(request->type==2) {
            struct msg *response = (struct msg*) malloc(8+4);
            response->length=4;
            response->type=2;
            memcpy(&response->payload,(void*)"pong",4);
            write(newSocket,(void *)response,8+response->length);
            free(response);
          }

          else {
            write(newSocket,(void *)"*** unk command type ***\n",16);
          } 
          free(request);     
        }
        else if (get>0) {
          write(newSocket,(void *)"unknown message format\n",22);
        }  
        close(newSocket);
    }
    close(socketId);
}
