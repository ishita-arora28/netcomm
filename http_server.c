#include <stdio.h>
#include <stdlib.h>
#include "include/sans.h"
#include <netinet/in.h>
#include <string.h>
#include <sys/stat.h>

int http_server(const char* iface, int port) {
  char response[1024];
  //accept connection
  int sID=sans_accept(iface,port,IPPROTO_TCP);

  //receive request 
  char request[1024];
  int data_received=sans_recv_pkt(sID,request,sizeof(request)-1);
  request[data_received] = '\0';
  //fetch file path
  char filePath[1024];
  int res=sscanf(request,"GET /%1023s HTTP/1.1",filePath);
  if(res!=1){
    snprintf(response, sizeof(response),
                 "HTTP/1.1 400 Bad Request\r\n"
                  "Content-Length: 0\r\n"
                 "Content-Type: text/html; charset=utf-8\r\n"
                 "\r\n");
        sans_send_pkt(sID, response, strlen(response));
        sans_disconnect(sID);
    return -1;
  }
  if (filePath[0] == '/') {
        snprintf(response, sizeof(response),
                 "HTTP/1.1 403 Forbidden\r\n"
                  "Content-Length: 0\r\n"
                  "Content-Type: text/html; charset=utf-8\r\n"
                 "\r\n");
        sans_send_pkt(sID, response, strlen(response));
        sans_disconnect(sID);
        return -1;
    }
  for(int i=0;filePath[i]!='\0';i++){
    if((filePath[i]=='.' &&  filePath[i+1]=='.') && (filePath[i+2] =='/' || filePath[i+2]=='\0'))
    {
      snprintf(response, sizeof(response),
                 "HTTP/1.1 403 Forbidden\r\n"
                 "Content-Length: 0\r\n"
                 "Content-Type: text/html; charset=utf-8\r\n"
                 "\r\n");
        sans_send_pkt(sID, response, strlen(response));
        sans_disconnect(sID);
        return -1;
    }
  }
  FILE *fp=fopen(filePath,"r");
  if(fp == NULL) {
     snprintf(response, sizeof(response),
                 "HTTP/1.1 404 Not Found\r\n"
                 "Content-Length: 0\r\n"
                 "Content-Type: text/html; charset=utf-8\r\n"
                 "\r\n");
    sans_send_pkt(sID, response, strlen(response));
    sans_disconnect(sID);
    return -1;  
  }

  //read file and store in buffer
  char filecontent[1024];
  size_t bytes_read=0;
  size_t file_size=0;
  struct stat fileBuffer;
  if(stat(filePath,&fileBuffer)==0){
    file_size=fileBuffer.st_size;
  }
  snprintf(response, sizeof(response),
          "HTTP/1.1 200 OK\r\n"
          "Content-Length: %zu\r\n"
          "Content-Type: text/html; charset=utf-8\r\n"
          "\r\n", file_size);
  sans_send_pkt(sID, response,strlen(response));
  while((bytes_read= fread(filecontent,1,sizeof(filecontent),fp))>0){
   
       sans_send_pkt(sID,filecontent,bytes_read);
    }


  fclose(fp);
  sans_disconnect(sID);
  return 0;

}
