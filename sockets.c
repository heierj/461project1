// Jordan Heier, Will McNamara, Cameron Hardin

#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sockets.h"
#include "client.h"

#define STUDENT_NUMBER 980
#define HOSTNAME "amlia.cs.washington.edu"
#define P_TIMEOUT 500000

char *stage_a();
char *stage_b(char *server_response);
char *stage_c(char *server_response, uint32_t *sock_fd);
char *stage_d(char *server_response, uint32_t *sock_fd);

int main(int argc, char **argv) {
  char *server_response;
  uint32_t sock_fd;

  server_response = stage_a();
  if(server_response == NULL) {
    printf("Error in stage A");
    return 1;
  }
  server_response = stage_b(server_response);
  if(server_response == NULL) {
    printf("Error in stage B");
    return 1;
  }
  server_response = stage_c(server_response, &sock_fd);
  if(server_response == NULL) {
    printf("Error in stage C");
    return 1;
  }
  server_response = stage_d(server_response, &sock_fd);
  if(server_response == NULL) {
    printf("Error in stage D");
    return 1;
  }
}

// This function handles the sending and receiving of
// packets for stage a. The server response is
// returned or NULL on error.
char *stage_a() {
  int sock_fd, res, buf_length;
  char *buf;

  char payload[] = "hello world";

  // Connect to server via UDP
  if (connect_to_hostname(HOSTNAME, 12235, SOCK_DGRAM, &sock_fd) != 0) {
    perror("Couldn't connect to host\n");
    return NULL;
  }
  
  //Create packet header 
  char *packet = create_header(sizeof(payload), 0, 1);
  if(packet == NULL) {
    printf("Error creating packet for stage A\n");
    return NULL;
  }

  // Set payload of packet
  memcpy((packet + sizeof(packet_header)), payload, sizeof(payload));
  
  if (write_to_socket(sock_fd, packet,
      sizeof(packet_header) + sizeof(payload))!=0) { 
    perror("Error writing to socket");
    return NULL;
  }  

  if (read_from_socket(sock_fd, &buf, &buf_length) != 0) {
    perror("Error reading from socket");
    return NULL;
  }
  
  free(packet);
  return buf;
}


// This function handles the sending and receiving of packets for
// stage b. Returns the server response for stage b or NULL on error.
char *stage_b(char *prev_packet) {
  uint32_t num, len, udp_port, secretA, sock_fd;
 
  // Extract all the values from the response packet from stage a
  num = 
    ntohl(*(int *) (prev_packet + sizeof(packet_header))); 
  len = 
    ntohl(*(int *) (prev_packet + sizeof(packet_header) + sizeof(int))); 
  udp_port = 
    ntohl(*(int *) (prev_packet + sizeof(packet_header) + 2 * sizeof(int))); 
  secretA =
    ntohl(*(int *) (prev_packet + sizeof(packet_header) + 3 * sizeof(int)));

  printf("Secret A: %d\n", secretA);

  // Free server response for A, its no longer needed
  free(prev_packet);
  
  // Connect to the host again on the provided port via UDP
  if (connect_to_hostname(HOSTNAME, udp_port, SOCK_DGRAM, &sock_fd) != 0) {
    printf("Couldn't connect to host\n");
    return NULL;
  }

  // Add 4 to len for the packet id and then byte align to 4 bytes
  uint32_t payload_len = (((len + 4) + 3) / 4) * 4;

  // Create the packet and set the header
  char *packet = create_header(len+4, secretA, 1);
  
  // Zero out the portion of the payload following the packet id
  memset(packet + sizeof(packet_header) + 4, 0, (payload_len - 4));

  //Begin write/read loop
  uint32_t i;
  for (i = 0; i < num; i++) {

    // Set packet number
    uint32_t packet_id = htonl(i);
    *((uint32_t *)(packet + sizeof(packet_header))) = packet_id;

    if (write_to_socket(sock_fd, packet, 
        sizeof(packet_header) + payload_len)!=0) { 
      perror("Error writing to socket");
      return NULL;
    }

    // Check that server received the packet
    // If we don't get an acknowledgement the resend the packet
    char *buf;
    int buf_length;
    if (read_from_socket(sock_fd, &buf, &buf_length) != 0) {
      printf("Resending packet %d for stage D, no ACK received\n", i);
      i--;
      continue;
    }
  }
  free(packet);

  // All packets sent, get the server's response
  char *buf;
  int buf_length;
  if (read_from_socket(sock_fd, &buf, &buf_length) != 0) {
      printf("Error reading from socket.\n");
  }
  return buf;
}

// This function handles the sending and receiving of packets for
// stage c. Returns the server response for stage c or NULL on error.
char *stage_c(char *prev_packet, uint32_t *sock_fd) {
  uint32_t tcp_port, secretB;
 
  // Extract all the values from the response packet from stage a
  tcp_port = 
    ntohl(*(int *) (prev_packet + sizeof(packet_header))); 
  secretB = 
    ntohl(*(int *) (prev_packet + sizeof(packet_header) + sizeof(int))); 

  printf("Secret B: %d\n", secretB);

  // Free server response for B, its no longer needed
  free(prev_packet);
  
  // Connect to the host again on the provided port via TCP 
  if (connect_to_hostname(HOSTNAME, tcp_port, SOCK_STREAM, sock_fd) != 0) {
    printf("Couldn't connect to host\n");
    return NULL;
  }
  
  //Read server response (No need to loop on TCP)
  char *buf;
  int buf_length;
  if (read_from_socket(*sock_fd, &buf, &buf_length) != 0) {
    printf("Error reading from socket");
  }
  return buf;
}

// This function handles the sending and receiving of packets for
// stage d. Returns the server response for stage d or NULL on error.
char *stage_d(char *prev_packet, uint32_t *sock_fd) {
  uint32_t num2, len2, secretC; char c;

  // Extract all the values from the response packet from stage c
  num2 = 
    ntohl(*(int *) (prev_packet + sizeof(packet_header))); 
  len2 = 
    ntohl(*(int *) (prev_packet + sizeof(packet_header) + sizeof(int))); 
  secretC = 
    ntohl(*(int *) (prev_packet + sizeof(packet_header) + 2 * sizeof(int))); 
  c = 
    *(char *) (prev_packet + sizeof(packet_header) + 3 * sizeof(int)); 

  printf("Secret C: %d\n", secretC);

  // Free server response for C, its no longer needed
  free(prev_packet);

  // byte align to 4 bytes
  uint32_t payload_len = len2 + 4 - len2 % 4;

  // Create the packet and set the header
  char *packet = create_header(len2, secretC, 1);
  
  // Set payload and zero out padding 
  memset(packet + sizeof(packet_header), (uint32_t) c, payload_len);
  memset(packet + sizeof(packet_header), 0, payload_len - len2);
  
  //write to server for d1
  uint32_t i;
  for (i = 0; i < num2; i++) {
    if (write_to_socket(*sock_fd, packet, 
        sizeof(packet_header) + payload_len)!=0) { 
      perror("Error writing to socket");
      return NULL;
    }
  }
  free(packet);

  //Read from socket
  char *buf;
  int buf_length;
  if (read_from_socket(*sock_fd, &buf, &buf_length) != 0) {
    printf("Error reading from socket");
  }
  uint32_t secretD = 
    ntohl(*(int *) (buf + sizeof(packet_header))); 
  printf("Secret D: %d\n", secretD);
  return buf;
}

// Allocates space for the packet including the payload. Fills in the header
// and returns a pointer to the start of the packet.
char *create_header(uint32_t payload_len, uint32_t psecret, uint16_t step) {
  
  packet_header *header = 
    (packet_header *) malloc(sizeof(packet_header) + payload_len);
  if(header == NULL) {
    return NULL;
  }

  // Add data into the header converting from
  // host order to network order
  header->payload_len = htonl(payload_len);
  header->psecret = htonl(psecret);
  header->step = htons(step);
  header->student_number = htons(STUDENT_NUMBER);

  return (char *) header;
}
