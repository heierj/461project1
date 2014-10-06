// Jordan Heier, Will McNamara, Cameron Hardin

#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include "sockets.h"

#define STUDENT_NUMBER 980
#define HEADER_SIZE 12

int main(int argc, char **argv) {


}

void *create_header(uint32_t payload_len, uint32_t psecret, uint16_t step) {
  
  void *header = (char *) malloc(HEADER_SIZE);
  if(header == NULL) {
    return NULL;
  }

  // Add data into the header converting from
  // host order to network order
  char *current_pos = header;
  *(uint32_t *)current_pos = htonl(payload_len);
  current_pos += 4;
  *(uint32_t *)current_pos = htonl(psecret);
  current_pos += 4;
  *(uint16_t *)current_pos = htons(step);
  current_pos += 2;

  uint16_t s_number = STUDENT_NUMBER;
  *(uint16_t *)current_pos = htons(s_number);

  return header;
}
