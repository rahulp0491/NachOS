// exception.cc 
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.  
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include <stdlib.h>
#include "copyright.h"
#include "system.h"
#include "syscall.h"
#include "machine.h"
#include <sys/stat.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#define BUF_SIZE 100

//----------------------------------------------------------------------
// ExceptionHandler
// 	Entry point into the Nachos kernel.  Called when a user program
//	is executing, and either does a syscall, or generates an addressing
//	or arithmetic exception.
//
// 	For system calls, the following is the calling convention:
//
// 	system call code -- r2
//		arg1 -- r4
//		arg2 -- r5
//		arg3 -- r6
//		arg4 -- r7
//
//	The result of the system call, if any, must be put back into r2. 
//
// And don't forget to increment the pc before returning. (Or else you'll
// loop making the same system call forever!
//
//	"which" is the kind of exception.  The list of possible exceptions 
//	are in machine.h.
//----------------------------------------------------------------------

void updatePC(){

		// Note that we have to maintain three PC registers, 
		// namely : PCReg, NextPCReg, PrevPCReg. 
		// (See machine/machine.cc, machine/machine.h) for more details.
		int pc, nextpc, prevpc;

		// Read PCs
		prevpc = machine->ReadRegister(PrevPCReg);
		pc = machine->ReadRegister(PCReg);
		nextpc = machine->ReadRegister(NextPCReg);

		// Update PCs
		prevpc = pc;
		pc = nextpc;
		nextpc = nextpc + 4;	// PC incremented by 4 in MIPS

		// Write back PCs
		machine->WriteRegister(PrevPCReg, prevpc);
		machine->WriteRegister(PCReg, pc);
		machine->WriteRegister(NextPCReg, nextpc);
	}

void ExceptionHandler(ExceptionType which) {
	int type = machine->ReadRegister(2);

	switch(which) {
	case SyscallException:
		switch(type) {
			case SC_Halt:
				DEBUG('a', "Shutdown, initiated by user program.\n");
				interrupt->Halt();
				break;

			case SC_Print: {
				DEBUG('a', "Print() system call invoked \n");
				int vaddr = machine->ReadRegister(4);
				// This address (pointer to the string to be printed) is 
				// the address that pointes to the user address space.
				// Simply trying printf("%s", (char*)addr) will not work
				// as we are now in kernel space.

				// Get the string from user space.

				int size = 0;
				char buf[BUF_SIZE];
				buf[BUF_SIZE - 1] = '\0';               // For safety.

				do {
					// Invoke ReadMem to read the contents from user space

					machine->ReadMem(vaddr, sizeof(char), (int*)(buf+size));

					// Compute next address
					vaddr+=sizeof(char);
					size++;

				} while( size < (BUF_SIZE - 1) && buf[size-1] != '\0');

				size--;
				DEBUG('a', "Size of string = %d", size);

				printf("%s", buf);
				bzero(buf, sizeof(char)*BUF_SIZE);  // Zeroing the buffer.
				updatePC();
				DEBUG('a', "PC updated \n");
			}
			break;

			case SC_Open:{
				DEBUG('a', "Open() system call invoked \n");
				OpenFileId id;
				int vaddr = machine->ReadRegister(4);

				int size = 0;
				char buf[BUF_SIZE];
				buf[BUF_SIZE - 1] = '\0';               // For safety.

				do{
					// Invoke ReadMem to read the contents from user space

					machine->ReadMem(vaddr, sizeof(char), (int*)(buf+size));

					// Compute next address
					vaddr+=sizeof(char);
					size++;

				} while( size < (BUF_SIZE - 1) && buf[size-1] != '\0');

				size--;
				DEBUG('a', "Size of string = %d", size);

				id = open(buf, O_CREAT | O_RDWR, S_IRWXU | S_IRWXG | S_IRWXO);
				machine->WriteRegister(2, id);
				bzero(buf, sizeof(char)*BUF_SIZE);  // Zeroing the buffer.
				updatePC();
				DEBUG('a', "PC updated \n");
			}
			break;

			case SC_Write:{
				DEBUG('a', "Write() system call invoked \n");
				OpenFileId id;
				id = machine->ReadRegister(6);
				int size = machine->ReadRegister(5);
				int vaddr = machine->ReadRegister(4);
				
				int sizeread = 0;
				char buf[BUF_SIZE];
				buf[BUF_SIZE - 1] = '\0';               // For safety.

				do{
					// Invoke ReadMem to read the contents from user space

					machine->ReadMem(vaddr, sizeof(char), (int*)(buf+sizeread)); 

					// Compute next address
					vaddr+=sizeof(char);    
					sizeread++;

				} while( size < (BUF_SIZE - 1) && sizeread < size);

				size--;
				DEBUG('a', "Size of string = %d", size);
				write(id, buf, size);
				bzero(buf, sizeof(char)*BUF_SIZE);  // Zeroing the buffer.
				updatePC();
				DEBUG('a', "PC updated \n");
			}
			break;
			
			case SC_Read: {
				DEBUG('a', "Read() system call invoked \n");
				OpenFileId id;
				id = machine->ReadRegister(6);
				int size = machine->ReadRegister(5);
				int vaddr = machine->ReadRegister(4);
					
				char buf[BUF_SIZE];
				read(id, buf, size);
					
				int sizewritten = 0;

				do{
					// Invoke ReadMem to read the contents from user space

					machine->WriteMem(vaddr, sizeof(char),*(buf + sizewritten));

					// Compute next address
					vaddr+=sizeof(char);    
					sizewritten++;

				} while( size < (BUF_SIZE - 1) && sizewritten < size);
				DEBUG('a', "Size of string = %d", size);
				bzero(buf, sizeof(char)*BUF_SIZE);  // Zeroing the buffer.
				updatePC();
				DEBUG('a', "PC updated \n");
			}
			break;
			
			case SC_Close:{
				DEBUG('a', "Close() system call invoked \n");
				OpenFileId id;
				id = machine->ReadRegister(4);
				close(id);
				updatePC();
				DEBUG('a', "PC updated \n");
			}
			break;
			
			default:
				printf("Unknown/Unimplemented system call %d!", type);
				ASSERT(FALSE); // Should never happen
				break;
			} // End switch(type)
	break; // End case SyscallException

	default:
		printf("Unexpected user mode exception %d %d\n", which, type);
		ASSERT(FALSE);
		break;
	}
}
