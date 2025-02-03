#include <stdio.h>
#include <string.h>
#include "include/sans.h"
#include <stdlib.h>
#include <netinet/in.h>

int smtp_agent(const char* host, int port) {
    char filePath[1024];
    char buffer[1024];
    char sender_receiver[256];

    // Input the file path
    scanf("%s", sender_receiver);
    scanf("%s", filePath);
    
    // Connect to the SMTP server
    int sID = sans_connect(host, port, IPPROTO_TCP);
    if (sID < 0) {
        return -1;  // Connection failed
    }

    // Receive server greeting
    sans_recv_pkt(sID, buffer, sizeof(buffer));

    // Send HELO command
    snprintf(buffer, sizeof(buffer), "HELO %s\r\n", host);
    sans_send_pkt(sID, buffer, strlen(buffer));
    sans_recv_pkt(sID, buffer, sizeof(buffer)); 

    // Send MAIL FROM command
      // Read sender email
    snprintf(buffer, sizeof(buffer), "MAIL FROM: <%s>\r\n", sender_receiver);
    sans_send_pkt(sID, buffer, strlen(buffer));
    sans_recv_pkt(sID, buffer, sizeof(buffer)); 
    if (strncmp(buffer, "250", 3) != 0) {
        sans_disconnect(sID);
        return -1;  // Failed to set from address
    }

    // Send RCPT TO command
    snprintf(buffer, sizeof(buffer), "RCPT TO: <%s>\r\n", sender_receiver);
    sans_send_pkt(sID, buffer, strlen(buffer));
    sans_recv_pkt(sID, buffer, sizeof(buffer)); 
    if (strncmp(buffer, "250", 3) != 0) {
        sans_disconnect(sID);
        return -1;  // Failed to set to address
    }

    // Send DATA command
    sans_send_pkt(sID, "DATA\r\n", strlen("DATA\r\n"));
    sans_recv_pkt(sID, buffer, sizeof(buffer)); 
    if (strncmp(buffer, "354", 3) != 0) {
        sans_disconnect(sID);
        return -1;  // DATA command failed
    }

    FILE *fp = fopen(filePath, "rb");
    if (fp == NULL) {
        sans_disconnect(sID);
        return -1;  // File open failed
    }

    // Send file contents with termination
    char filecontent[1024];
    size_t bytes_read;

    while ((bytes_read = fread(filecontent, 1, sizeof(filecontent), fp)) > 0) {
        sans_send_pkt(sID, filecontent, bytes_read);
    }

    // Send message termination
    sans_send_pkt(sID, "\r\n.\r\n", strlen("\r\n.\r\n")); 
    sans_recv_pkt(sID, buffer, sizeof(buffer)); 
    if (strncmp(buffer, "250", 3) != 0) {
        fclose(fp);
        sans_disconnect(sID);
        return -1;  // Failed to terminate message body
    }
    printf("%s\n",buffer);
    // Send QUIT command
    sans_send_pkt(sID, "QUIT\r\n", strlen("QUIT\r\n"));
    sans_recv_pkt(sID, buffer, sizeof(buffer)); 

    fclose(fp);
    sans_disconnect(sID);
    return 0;  // Success
}
