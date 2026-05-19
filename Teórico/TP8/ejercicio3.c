#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>

int main(int argc, char** argv) {
  int fd_p2c[2]; // Parent to child
  int fd_c2p[2]; // Child to parent
  
  if (pipe(fd_p2c) == -1 || pipe(fd_c2p) == -1) {
      perror("Error al crear las tuberías");
      exit(1);
  }

  int childpid = fork();
  if (childpid == -1) {
    fprintf(stderr, "fork error!\n");
    exit(1);
  }
  
  if (childpid == 0) {
    // Child closes unused ends
    close(fd_p2c[1]); // Close write end of parent-to-child
    close(fd_c2p[0]); // Close read end of child-to-parent

    // Child writes to parent
    char str_to_parent[] = "Hola Papa!";
    fprintf(stdout, "HIJO: Esperando 2 segundos ...\n");
    sleep(2);
    fprintf(stdout, "HIJO: Escribiendo a papa ...\n");
    write(fd_c2p[1], str_to_parent, strlen(str_to_parent) + 1);

    // Child reads from parent
    char buff[64];
    read(fd_p2c[0], buff, 64);
    fprintf(stdout, "HIJO: Recibido de papa: %s\n", buff);

    close(fd_p2c[0]);
    close(fd_c2p[1]);
  } else {
    // Parent closes unused ends
    close(fd_p2c[0]); // Close read end of parent-to-child
    close(fd_c2p[1]); // Close write end of child-to-parent

    // Parent reads from child
    char buff[64];
    fprintf(stdout, "PADRE: Leyendo del hijo ...\n");
    read(fd_c2p[0], buff, 64);
    fprintf(stdout, "PADRE: Recibido del hijo: %s\n", buff);

    // Parent writes to child
    char str_to_child[] = "Hola Hijo!";
    fprintf(stdout, "PADRE: Escribiendo al hijo ...\n");
    write(fd_p2c[1], str_to_child, strlen(str_to_child) + 1);

    close(fd_p2c[1]);
    close(fd_c2p[0]);
  }
  
  return 0;
}
