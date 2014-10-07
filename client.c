// Jordan Heier, Will McNamara, Cameron Hardin

#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <fcntl.h>  
#include <unistd.h>

// Used to look up a hostname and port number and return via 
// parameter a struct sockaddr_storage and the address length.
static int lookup_hostname(char *hostname,
                           unsigned short port_num,
                           struct sockaddr_storage *ret_addr,
                           size_t *ret_len); 

 int connect_to_hostname(char *hostname,
                       unsigned short port,
                       int socket_type,
                       int *ret_sock_fd) {
  
  struct sockaddr_storage addr;
  size_t addr_len;

  if (!lookup_hostname(hostname, port, &addr, &addr_len))
    // Failed to find an IP address associated with hostname, 
    // return false.
    return 0;

  //////
  // Create the socket.
  int socket_fd = socket(addr.ss_family, socket_type, 0);
  if (socket_fd == -1) {
    perror("Failed to create socket");
    return 0;
  }

  // connect the socket to the remote host.
  int res = connect(socket_fd,
                   (struct sockaddr *) &addr,
                   addr_len);
  if (res == -1) {
    perror("connect() failed");
    return 0;
  }

  *ret_sock_fd = socket_fd;
  return 1;
}



int write_to_socket(const int socket_fd, char *buf, int buf_size) {
  int bytes_written = 0;

  while (bytes_written < buf_size) {
    int wres = write(socket_fd, buf + bytes_written, buf_size - bytes_written);
    if (wres == 0) {
      fprintf(stderr, "socket closed prematurely");
      return 0;
    }
    if (wres == -1) {
      if (errno == EINTR) 
        continue;
      // We have a legitimate error so we return false.
      perror("socket write failure");
      return 0;
    }
    bytes_written += wres;
  }
 
  // Successfully wrote the entire buffer over the network.
  return 1;
}

static int lookup_hostname(char *hostname,
                    unsigned short port_num,
                    struct sockaddr_storage *ret_addr,
                    size_t *ret_len) {

  struct addrinfo hints, *results;
  int res;

  memset(&hints, 0, sizeof(hints));  
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = 0;

  res = getaddrinfo(hostname, NULL, &hints, &results);
  if (res != 0) {
    perror("getaddrinfo failed");
    return 0;
  }

  // Set the port in the first result.
  if (results->ai_family == AF_INET) {
    struct sockaddr_in *v4addr = (struct sockaddr_in *) results->ai_addr;
    v4addr->sin_port = htons(port_num);
  } else if (results->ai_family == AF_INET6) {
    struct sockaddr_in6 *v6addr = (struct sockaddr_in6 *) results->ai_addr;
    v6addr->sin6_port = htons(port_num);
  } else {
    perror("getaddrinfo failed to provide an IPv4 or IPv6 address\n");
    freeaddrinfo(results);
    return 0;
  }

  // Return the first result.
  memcpy(ret_addr, results->ai_addr, results->ai_addrlen);
  *ret_len = results->ai_addrlen;

  // Clean up.
  freeaddrinfo(results);
  return 1;
}

