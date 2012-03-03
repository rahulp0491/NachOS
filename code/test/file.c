#include "syscall.h"

void main(){
	char buf[100];
	OpenFileId fd = Open("testing_file.txt");
	Write("Hello!", sizeof("Hello!"), fd);
	Close(fd);
	fd = Open("testing_file.txt");
	Read(buf, (sizeof("Hello!")-1), fd);
	buf[99] = '\0';
	Print(buf);
	Print("\n");
	Halt();
}
