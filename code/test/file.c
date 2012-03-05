#include "syscall.h"

void main(){
	char buf[100];
	int size;
	OpenFileId fd = Open("file.txt");
	OpenFileId fd1 = Open("file1.txt");
	Write("Hello!", 7, fd);
	fd = Open("file.txt");
	size = Read(buf, 100, fd);
	Write(buf, size, fd1);
	buf[99] = '\0';
	Print(buf);
	Close(fd);
	Close(fd1);
	Print("\n");
	Halt();
}
