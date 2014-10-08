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


int main(int argc, char **argv) {
  char *next_packet;
  char *prev_packet;
  int next_len, prev_len;  

  stage_a(&prev_packet, &prev_len);
  stage_b(prev_packet, prev_len, &next_packet, &next_len);
}


int stage_a(char **next_packet, int *packet_len) {
  int sock_fd, res, buf_length;
  char *buf;

  char payload[] = "hello world";

  if (connect_to_hostname(HOSTNAME, 12235, SOCK_DGRAM, &sock_fd) != 0) {
    perror("Couldn't connect to host\n");
    return -1;
  }
 
  char *packet = create_header(sizeof(payload), 0, 1);
  if(packet == NULL) {
    printf("Error creating packet\n");
    return -1;
  }

  // Set payload of packet
  memcpy((packet + sizeof(packet_header)), payload, sizeof(payload));
  
  if (write_to_socket(sock_fd, packet, sizeof(packet_header) + sizeof(payload))!=0) { 
    perror("Error writing to socket");
    return -1;
  }  

  if (read_from_socket(sock_fd, &buf, &buf_length) != 0) {
    perror("Error reading from socket");
    return -1;
  }
  
  // Free a1 packet
  free(packet);
  
  char *result = (char *) malloc(buf_length); 
  memcpy(result, buf, buf_length);

  *next_packet = result;
  *packet_len = buf_length;

  return 0;
}

int stage_b(char *prev_packet, int prev_len, char **next_packet, int *next_len) {
  uint32_t num, len, udp_port, secretA;

  num = ntohl(*((int *) (prev_packet + sizeof(packet_header)))); 
  len = ntohl(*((int *) (prev_packet + sizeof(packet_header) + sizeof(int)))); 
  udp_port = ntohl(*((int *) (prev_packet + sizeof(packet_header) + 2 * sizeof(int)))); 
  secretA = ntohl(*((int *) (prev_packet + sizeof(packet_header) + 3 * sizeof(int)))); 


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
