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

int main(int argc, char **argv) {
  char *server_response;

  server_response = stage_a();
  if(server_response == NULL) {
    printf("Error in stage A");
  }

  server_response = stage_b(server_response);
  if(server_response == NULL) {
    printf("Error in stage B");
  }
  // stage_c();
  // stage_d();
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
  
  // Free a1 packet
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
      printf("Error reading from socket. Resending packet %d\n", i);
      i--;
      continue;
    }
  }

  // Free packet sent for stage B
  free(packet);

  // All packets sent, get the server's response
  char *buf;
  int buf_length;
  printf("Done with stage B, reading server response\n");
  if (read_from_socket(sock_fd, &buf, &buf_length) != 0) {
      printf("Error reading from socket.\n");
  }
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
