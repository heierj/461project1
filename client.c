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

  if (lookup_hostname(hostname, port, &addr, &addr_len))
    // Failed to find an IP address associated with hostname, 
    // return false.
    return -1;

  //////
  // Create the socket.
  int socket_fd = socket(addr.ss_family, socket_type, 0);
  if (socket_fd == -1) {
    perror("Failed to create socket");
    return -1;
  }

  // connect the socket to the remote host.
  int res = connect(socket_fd,
                   (struct sockaddr *) &addr,
                   addr_len);
  if (res == -1) {
    perror("connect() failed");
    return -1;
  }

  *ret_sock_fd = socket_fd;
  return 0;
}



int write_to_socket(const int socket_fd, char *buf, int buf_size) {
  int bytes_written = 0;

  while (bytes_written < buf_size) {
    int wres = write(socket_fd, buf + bytes_written, buf_size - bytes_written);
    if (wres == 0) {
      fprintf(stderr, "socket closed prematurely");
      return -1;
    }
    if (wres == -1) {
      if (errno == EINTR) 
        continue;
      // We have a legitimate error so we return false.
      perror("socket write failure");
      return -1;
    }
    bytes_written += wres;
  }
 
  // Successfully wrote the entire buffer over the network.
  return 0;
}

// Read bytes from the client file descriptor until EOF or a network
// interruption occurs. Prints bytes to stdout. Returns true on
// success and false otherwise.
char* read_from_client(const int client_fd, int* buf_size) {
  // Prepare a buffer to be used to read bytes from the client.
  char buf[256];
  int bytes_read = 0;
  int result = 1;

  // While we are not at the end of file continue reading into the buffer.
  while ( (result != 0) ) {
    result = read(client_fd, buf + bytes_read, 256 - bytes_read);
    if (result == -1) {
      if (errno != EINTR) {
        // Some error occured trying to read the file so return false.
        return -1;
      }
      // EINTR happened, just try again
      continue;
    }
    bytes_read += result;

    // If buf is full or we have read the entire file, flush the buffer
    // by writing to stdout.
    if (bytes_read == 256 || result == 0) {
      fwrite(buf, 1, bytes_read, stdout);
      bytes_read = 0;
    }
  }

  // Success.
  return 0;
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
    return -1;
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
    return -1;
  }

  // Return the first result.
  memcpy(ret_addr, results->ai_addr, results->ai_addrlen);
  *ret_len = results->ai_addrlen;

  // Clean up.
  freeaddrinfo(results);
  return 0;
}

