// Header file for sockets.c

#ifndef SOCKETS_H
#define SOCKETS_H

char *create_header(uint32_t payload_len, uint32_t psecret, uint16_t step);

int lookup_hostname(char *hostname,
                    unsigned short port_num,
                    struct sockaddr_storage *ret_addr,
                    size_t *ret_len);

#endif /* ifndef SOCKETS_H */
