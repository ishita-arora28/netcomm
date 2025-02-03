#include <netinet/in.h>
#include "include/sans.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void get_response(int sock) {
  char buffer[1024];
  int totalbytes=0;
  int contentlength=0;
  int headerFlag=0;
  while(1){
      int bytesreceived= sans_recv_pkt(sock,buffer,sizeof(buffer)-1);
      if(bytesreceived<=0)
        break;
      buffer[bytesreceived] = '\0';
      printf("%s", buffer);
      if(!headerFlag){
        char *headerend= NULL;
        for(int i=0;i<bytesreceived;i++){
          if(buffer[i-1]=='\r' && buffer[i]=='\n'){
              headerend=&buffer[i+1];

           if (buffer[i-2] == '\r' && buffer[i-1] == '\n') 
            {
              headerFlag = 1;
              break;

            }

        }
      }
      if(headerend){
        char *contentheader="Content-Length: ";
        char *ptr=headerend;
        while(ptr-buffer<bytesreceived){
          if(strncmp(ptr,contentheader,strlen(contentheader))){
            ptr+=strlen(contentheader);
            contentlength=strtol(ptr,&ptr,10);

          }
           while (ptr < buffer + bytesreceived && *ptr != '\n') {
                        ptr++;
            }
            if (*ptr == '\n') {
                        ptr++;
                    }
        }
      }
  }
  totalbytes+=bytesreceived;
  if (contentlength > 0 && totalbytes  >= contentlength) {
            break; 
  }
  }

}
int http_client(const char* host, int port){
  char method[3];

  scanf("%s", method);

  if(strcmp("GET",method) !=0)
  {  
      
      return 0; 
  }
  char path[1024];
 
  scanf("%s",path);


  int socketID=sans_connect(host,port,IPPROTO_TCP);
  char request[1024];
  
  snprintf(request,sizeof(request),
  "%s /%s HTTP/1.1\r\n"
  "Host: %s:%d\r\n"
  "User-Agent: sans/1.0\r\n"
  "Cache-Control: no-cache\r\n"
  "Connection: close\r\n"
  "Accept: */*\r\n"
  "\r\n",method,path, host,port);
  
  sans_send_pkt(socketID, request,strlen(request));
  get_response(socketID);
  sans_disconnect(socketID);
  return 0;
 
}
