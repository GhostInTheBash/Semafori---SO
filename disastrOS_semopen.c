#define _DISASTROS_DEBUG_

#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include "disastrOS.h"
#include "disastrOS_syscalls.h"
#include "disastrOS_semaphore.h"
#include "disastrOS_semdescriptor.h"

// creates a new semaphore or opens an existing semaphore.
// oflag not defined, il primo argomento è il nome(id)

/*
On  success,  sem_open() returns the address of the new semaphore; this
       address is used when calling  other  semaphore-related  functions.   On
       error,  sem_open()  returns  SEM_FAILED, with errno set to indicate the
       error.
*/

void internal_semOpen(){
	
	printf("Start sem_open\n\n\n");
	
	if(running -> sem_descriptors.size >= MAX_NUM_SEMDESCRIPTORS_PER_PROCESS){
		printf("Troppi descrittori per processo \n\n");
		running->syscall_retvalue = DSOS_ESEM_MAX_NUMBER_OF_SEMDESCRIPTORS; 
		return;
    }
    
	int id = running -> syscall_args[0];
	int count = running -> syscall_args[1]; //numero di processi che possono accedere al semaforo
	if(count <= 0){
		printf("\nContatore del semaforo non può essere <= 0, impostato a 1\n\n");
		count = 1;
	}
	
	//if sem not open yet, alloc it
	
	Semaphore* sem = SemaphoreList_byId(&semaphores_list, id); // check for existing semaphore
	if(!sem){
		printf("Sem non in lista...\n");
		sem = Semaphore_alloc(id, count);
		if(!sem){
			disastrOS_debug("Errore allocazione semaforo\n");
			running -> syscall_retvalue = DSOS_ESEMALL;
			return;
		}
		printf("Sem allocato\n");
		List_insert(&semaphores_list, semaphores_list.last, (ListItem*) sem);
		printf("Sem inserito in lista\n");
	}
	
	// alloc sem descriptor
	
	printf("Alloco il descrittore\n");
	
	SemDescriptor* des = SemDescriptor_alloc(running -> last_sem_fd, sem, running);
	
	// if error, return with code error (Defined in constant)
	if(!des){
		disastrOS_debug("Errore allocazione descrittore semaforo\n");
		running -> syscall_retvalue = DSOS_ESEMFD;
		return;
	}
	
	printf("Descrittore allocato\n");
	
	List_insert(&running->sem_descriptors, running->sem_descriptors.last, (ListItem*) des);
	
	running -> last_sem_fd++;
	
	// alloc ptr sem descriptor
	
	SemDescriptorPtr* desptr = SemDescriptorPtr_alloc(des);
	
	// if error, return with code error (Defined in constant)
	if(!desptr){
		disastrOS_debug("Errore allocazione puntatore al descrittore del semaforo\n");
		running -> syscall_retvalue = DSOS_ESEMDESC;
		return;
	}
	
	printf("Puntatore allocato\n");
	des -> ptr = desptr;
	List_insert(&sem->descriptors, sem->descriptors.last, (ListItem*) desptr);
	printf("Semaforo aperto \n\n");
	running -> syscall_retvalue = des -> fd; // ritorno il file descriptor del semaforo aperto
}
