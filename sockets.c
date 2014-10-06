// Jordan Heier, Will McNamara, Cameron Hardin

#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <iostream.h>
#include <fcntl.h>  
#include <unistd.h>

#include "sockets.h"

#define STUDENT_NUMBER 980
#define HEADER_SIZE 12

int main(int argc, char **argv) {
  
}
/*
// connects to a socket with specifications given by 'addr'
// and uses a return parameter to send back the new socket
// file descriptor
static bool connect(const struct sockaddr_storage &addr,
                     const size_t addrlen,
                     int *ret_fd);

// Used to look up a hostname and port number and return via 
// parameter a struct sockaddr_storage and the address length.
static bool lookup_hostname(char *hostname,
                           unsigned short port_num,
                           struct sockaddr_storage *ret_addr,
                           size_t *ret_len); 

bool connect_to_hostname(char *hostname,
                       unsigned short port,
                       int *ret_sock_fd) {
  
  struct sockaddr_storage addr;
  size_t addr_len;

  if (!lookup_hostname(hostname, port, &addr, &addr_len))
    // Failed to find an IP address associated with hostname, 
    // return false.
    return false;

  if (!connect(addr, addr_len, ret_sock_fd))
    // We failed to connect to the socket so fail and return false.
    return false;

  return true;
}

bool write_to_socket(const int socket_fd, char *buf, int buf_size) {
  int bytes_read = 0;

  while (bytes_read < buf_size) {
    int wres = write(socket_fd, buf + bytes_read, buf_size - bytes_read);
    if (wres == 0) {
      std::cerr << "socket closed prematurely" << std::endl;
      return false;
    }
    if (wres == -1) {
      if (errno == EINTR) 
        continue;
      // We have a legitimate error so we return false.
      std::cerr << "socket write failure: " << strerror(errno) << std::endl;
      return false;
    }
    bytes_read += wres;
  }

  
  
  // Successfully wrote the entire buffer over the network.
  return 1;
}

bool connect(const struct sockaddr_storage *addr,
             const size_t addrlen,
             int socket_type,
             int *ret_fd) {

  // Create the socket.
  int socket_fd = socket(addr->ss_family, socket_type, 0);
  if (socket_fd == -1) {
    perror("Failed to create socket");
    return false;
  }

  // connect the socket to the remote host.
  int res = connect(socket_fd,
                    reinterpret_cast<const sockaddr *>(&addr),
                    addrlen);
  if (res == -1) {
    std::cerr << "connect() failed: " << strerror(errno) << std::endl;
    return false;
  }

  *ret_fd = socket_fd;
  return true;
}*/

bool lookup_hostname(char *hostname,
                     unsigned short port_num,
                     struct sockaddr_storage *ret_addr,
                     size_t *ret_len) {

  struct addrinfo hints, *results;
  int res;

  memset(&hints, 0, sizeof(hints));  
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;

  res = getaddrinfo(hostname, NULL, &hints, &results);
  if (res != 0) {
    perror("getaddrinfo failed");
    return false;
  }

  // Set the port in the first result.
  if (results->ai_family == AF_INET) {
    struct sockaddr_in *v4addr = (struct sockaddr_in *) results->ai_addr;
    v4addr->sin_port = htons(port_num);
  } else if (results->ai_family == AF_INET6) {
    struct sockaddr_in6 *v6addr = (struct sockaddr_in6 *) results->ai_addr;
    v6addr->sin6_port = htons(port_num);
  } else {
    std::cerr << "getaddrinfo failed to provide an IPv4 or IPv6 address";
    std::cerr << std::endl;
    freeaddrinfo(results);
    return false;
  }

  // Return the first result.
  assert(results != NULL);
  memcpy(ret_addr, results->ai_addr, results->ai_addrlen);
  *ret_len = results->ai_addrlen;

  // Clean up.
  freeaddrinfo(results);
  return true;
}

char *create_header(uint32_t payload_len, uint32_t psecret, uint16_t step) {
  
  void *header = (char *) malloc(HEADER_SIZE);
  if(header == NULL) {
    return NULL;
  }

  // Add data into the header converting from
  // host order to network order
  char *current_pos = header;
  *current_pos = htonl(payload_len);
  current_pos += 4;
  *current_pos = htonl(psecret);
  current_pos += 4;
  *current_pos = htons(step);
  current_pos += 2;

  uint16_t s_number = STUDENT_NUMBER;
  *current_pos = htons(s_number);

  return header;
}
