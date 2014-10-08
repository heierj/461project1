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
  // stage_c();
  // stage_d();
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
  uint32_t num, len, udp_port, secretA, sock_fd;

  // Extract all the values from the response packet from stage a
  num = ntohl(*((int *) (prev_packet + sizeof(packet_header)))); 
  len = ntohl(*((int *) (prev_packet + sizeof(packet_header) + sizeof(int)))); 
  udp_port = ntohl(*((int *) (prev_packet + sizeof(packet_header) + 2 * sizeof(int)))); 
  secretA = ntohl(*((int *) (prev_packet + sizeof(packet_header) + 3 * sizeof(int))));

  free(prev_packet);
  
  // Connect to the host again on the provided port
  if (connect_to_hostname(HOSTNAME, udp_port, SOCK_DGRAM, &sock_fd) != 0) {
    printf("Couldn't connect to host\n");
    return -1;
  }

  // Add 4 to len for the packet id and then byte align to 4 bytes
  uint32_t payload_len = (((len + 4) + 3) / 4) * 4;
  char *packet = create_header(payload_len, secretA, 1);
  
  // Zero out the portion of the payload following the packet id
  memset(packet + sizeof(packet_header) + 4, 0, 
        (payload_len - sizeof(packet_header) - 4) / 4);
  
  uint32_t i;
  for (i = 0; i < num; i++) {
    uint32_t packet_id = htonl(i);
    *((uint32_t *)(packet + sizeof(packet_header))) = packet_id;

    if (write_to_socket(sock_fd, packet, sizeof(packet_header) + payload_len)!=0) { 
      perror("Error writing to socket");
      return -1;
    }

    // TODO: Read response from server, if the server doesn't respond then continue
    // and decrement 'i' so that we try to send the packet again until it an ACK
    // packet is received.
  }

  free(packet);
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
