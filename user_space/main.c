/*
 * Devon Mickels
 * 4/20/2020
 * ECE 373
 *
 * Char Driver
 */
 
 #include <stdio.h>
 #include <stdlib.h>
 #include <errno.h>
 #include <fcntl.h>
 #include <string.h>
 #include <unistd.h>
 
 #define BUF_LEN 256 //General buffer size
 
 int main(){
	 
	 int ret, fd;
	 char buf[BUF_LEN];
	 char *end;
	 
	 fd = open ("/dev/testCDriver", O_RDWR);
	 
	 if(fd < 0){
		 printf("Cannot open device! \t");
		 printf("fd = %d \n", fd);
		 return 0;
	 }
	 
	 //Read from the device
	 ret = read(fd, buf, BUF_LEN);
	 if(ret < 0){
		perror("Failed to read\n");
		return errno;
	 }
	 printf("Value: %d \n", *(unsigned int*)buf);
	 
	 //Write to the device
	 int valueToWrite = 97;
	 strcpy(buf, (char*)&valueToWrite);
	 ret = write(fd, buf, strlen(buf));
	 if(ret < 0){
		 perror("Failed to write\n");
		return errno;
	 }
	 
	 //Read from the device
	 ret = read(fd, buf, BUF_LEN);
	 if(ret < 0){
		perror("Failed to read\n");
		return errno;
	 }
	 printf("Value: %d \n", *(unsigned int*)buf);
	 
	 if( 0 != close(fd)){
		 printf("Failed to close device!\n");
	 }
	 
	 return 0;
 }