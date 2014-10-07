// Jordan Heier, Will McNamara, Cameron Hardin

#ifndef CLIENT_H
#define CLIENT_H

/*
  Connects to a 'hostname' on the specified 'port'. Returns -1 if fails or
  a 0 if successful. Also sets a valid file descriptor in 'ret_sock_fd'
  upon success.
*/  
int connect_to_hostname(char *hostname,
                        unsigned short port,
                        int socket_type,
                        int *ret_sock_fd);

/*
  Writes 'buf_size' number of bytes in 'buf' to the specified socket. 
  Returns 0 upon success and a -1 if an error is encountered.
*/
int write_to_socket(const int socket_fd, 
                    char *buf, 
                    int buf_size);

/*
int read_from_socket(const int socket_fd,
                     char *data);
*/

#endif
