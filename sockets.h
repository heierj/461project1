// Header file for sockets.c

#ifndef SOCKETS_H
#define SOCKETS_H

typedef struct packet_header {
  uint32_t payload_len;
  uint32_t psecret;
  uint16_t step;
  uint16_t student_number;
} packet_header;

char *create_header(uint32_t payload_len, uint32_t psecret, uint16_t step);

#endif /* ifndef SOCKETS_H */
