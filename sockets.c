// Jordan Heier, Will McNamara, Cameron Hardin

#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sockets.h"
#include "client.h"

#define STUDENT_NUMBER 980


int main(int argc, char **argv) {
  struct sockaddr_storage ret_addr;
  int sock_fd;

  //int ret = connect_to_hostname("amlia.cs.washington.edu", 12235, SOCK_DGRAM,&sock_fd);
  //printf("Success = %d, ret_sock = %d\n", ret, sock_fd);

  // Create the packet for part a1
  packet_header *header = create_header(12, 0, 1);
  int a1_payload_size = 12;

  // Allocate enough space for the packet
  char *packet = (char *) malloc(sizeof(packet_header) + a1_payload_size);

  // Copy header to packet
  memcpy(packet, header, sizeof(packet_header));
  // Set payload of packet
  strncpy((packet + sizeof(packet_header)), "hello world", a1_payload_size);


}


// Creates a packet header. The header's field will be in network order.
packet_header *create_header(uint32_t payload_len, uint32_t psecret, uint16_t step) {
  
  packet_header *header = (packet_header *) malloc(sizeof(packet_header));
  if(header == NULL) {
    return NULL;
  }

  // Add data into the header converting from
  // host order to network order
  header->payload_len = htonl(payload_len);
  header->psecret = htonl(psecret);
  header->step = htons(step);
  header->student_number = htons(STUDENT_NUMBER);

  return header;
}
