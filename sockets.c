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
  int res;

  printf("Connecting to amlia.cs.washington.edu\n");

  res = connect_to_hostname("amlia.cs.washington.edu", 12235, SOCK_DGRAM, &sock_fd);
  if(res) {
    printf("Couldn't connect to host\n");
  }

  printf("Socket opened with fd %d\n", sock_fd);

  //printf("Success = %d, ret_sock = %d\n", ret, sock_fd);

  // Create the packet for part a1
  printf("Creating packet a1\n");
 
  int a1_payload_size = 12;
  char *packet = create_header(a1_payload_size, 0, 1);
  if(packet == NULL) {
    printf("Error creating packet\n");
  }

  // Set payload of packet
  strncpy((packet + sizeof(packet_header)), "hello world", a1_payload_size);

  printf("Sending packet\n");
  res = write_to_socket(sock_fd, packet, sizeof(packet_header) + a1_payload_size);

  if(res) {
    printf("Error writing to socket\n");
  }  

  // Free a1 packet 
  free(packet);


}


// Allocates space for the packet including the header. Fills in the header
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
