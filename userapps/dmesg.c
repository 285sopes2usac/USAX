#include <stdio.h>
#include <unistd.h>
#include <sys/syscall.h>

int main(int argc, char **argv) {
   // Use syslog to get printk ring buffer contents
   const int buffer_size = 4096; // Define the size of the buffer
   char buffer[buffer_size];

   int bytes_read = syscall(103, buffer, buffer_size - 1); // Leave space for null terminator

   if (bytes_read < 0) {
      printf("syslog syscall failed with error: %d\n", bytes_read);
      // Handle error
      return 1;
   }

    return 0;
}
