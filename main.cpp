#include "mbed.h"
#include "FATFileSystem.h"
#include "HeapBlockDevice.h"
#include <stdio.h>
#include <errno.h>

#define SERIAL_BAUD_RATE (115200)

// Failures using NXP LPC4088 micro, on Embedded Artists' LPC4088 QuickStart Board:
//
// - With mbed online compiler, 2017-12-08 (armcc):
//   - error writing at line 5974
//   - file readback ends early after line 5888
//
// - With NXP MCUXpresso IDE v10.1.0 [Build 589] [2017-11-14]
//   (gcc-arm-none-eabi):
//   - error writing at line 4779
//   - file readback ends early after line 4608

// 6 characters per line (4 digits + "\r\n")
// * 10000 lines = 60 KB, should fit on 128 * 512 = 64 KiB block device
#define NUM_INTS         (10000)
#define FIELD_WIDTH_STR  "4"

HeapBlockDevice bd(128 * 512, 512);
FATFileSystem fs("fs");

Serial s(USBTX, USBRX, SERIAL_BAUD_RATE);

void return_error(int ret_val){
  if (ret_val)
    printf("Failure. %d\r\n", ret_val);
  else
    printf("done.\r\n");
}

void errno_error(void* ret_val){
  if (ret_val == NULL)
    printf(" Failure. %d \r\n", errno);
  else
    printf(" done.\r\n");
}

int main() {
  int error = 0;
  printf("Welcome to the filesystem example.\r\n"
         "Formatting a FAT, RAM-backed filesystem. ");
  error = FATFileSystem::format(&bd);
  return_error(error);

  printf("Mounting the filesystem on \"/fs\". ");
  error = fs.mount(&bd);
  return_error(error);

  printf("Opening a new file, numbers.txt.");
  FILE* fd = fopen("/fs/numbers.txt", "w");
  errno_error(fd);

  bool writeOK = true;

  // write the line number on each line of the file (thus the 1-indexing)
  for (int i = 1; i <= NUM_INTS; i++){
    printf("\rWriting decimal numbers to a file (%d/%d)", i, NUM_INTS);
    int numCharsWritten = fprintf(fd, "%" FIELD_WIDTH_STR "d\r\n", i);

    if (numCharsWritten < 0)
    {
    	printf("\r\n  ERROR writing to file. (fprintf() returned %i.)\r\n",
    			numCharsWritten);
    	writeOK = false;
    	break;
    }
  }

  if (writeOK){
	  printf(". done.\r\n");
  }

  printf("Closing file.");
  fclose(fd);
  printf(" done.\r\n");

  printf("Re-opening file read-only.");
  fd = fopen("/fs/numbers.txt", "r");
  errno_error(fd);

  printf("Dumping file to screen.\r\n");
  char buff[16] = {0};
  while (!feof(fd)){
    int size = fread(&buff[0], 1, 15, fd);
    fwrite(&buff[0], 1, size, stdout);
  }
  printf("EOF.\r\n");

  printf("Closing file.");
  fclose(fd);
  printf(" done.\r\n");

  printf("Opening root directory.");
  DIR* dir = opendir("/fs/");
  errno_error(fd);

  struct dirent* de;
  printf("Printing all filenames:\r\n");
  while((de = readdir(dir)) != NULL){
    printf("  %s\r\n", &(de->d_name)[0]);
  }

  printf("Closing root directory. ");
  error = closedir(dir);
  return_error(error);
  printf("Filesystem Demo complete.\r\n");

  while (true) {}
}

