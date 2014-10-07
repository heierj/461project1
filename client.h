// Jordan Heier, Will McNamara, Cameron Hardin

#ifndef CLIENT_H
#define CLIENT_H

/*
 * Connects to a 'hostname' on the specified 'port'. Returns -1 if fails or
 * a 0 if successful. Also sets a valid file descriptor in 'ret_sock_fd'
 * upon success.
 */  
int connect_to_hostname(char *hostname,
                        unsigned short port,
                        int socket_type,
                        int *ret_sock_fd);

/*
 * Writes 'buf_size' number of bytes in 'buf' to the specified socket. 
 * Returns 0 upon success and a -1 if an error is encountered.
 */
int write_to_socket(const int socket_fd, 
                    char *buf, 
                    int buf_size);

/*
 * Reads from the client. returns -1 on error, 0 if successful, and 1 if successful but the buffer limit
 * was filled (meaning all the data might not have been read in). The buffer 'data' will be set to the 
 * data received and 'buf_size' will be set to the number of bytes read into the buffer.
 */
int read_from_socket(const int client_fd, 
                     char** data,
                     int* buf_size);

#endif
