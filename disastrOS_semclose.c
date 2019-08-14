#define _DISASTROS_DEBUG_

#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include "disastrOS.h"
#include "disastrOS_syscalls.h"
#include "disastrOS_semaphore.h"
#include "disastrOS_semdescriptor.h"

void internal_semClose(){
	// fd semaforo da chiudere
	
    int sem_fd = running -> syscall_args[0];
    
    printf("Cerco il descrittore nella lista\n");
    
    SemDescriptor* sem_des = SemDescriptorList_byFd(&running -> sem_descriptors, sem_fd);
    
    if(!sem_des){
		disastrOS_debug("Descrittore semaforo non trovato \n");
		running -> syscall_retvalue = DSOS_ESEMFD;
		return;
	}
	
	printf("Ho trovato il descrittore, lo elimino\n");
	sem_des = (SemDescriptor*)List_detach(&running->sem_descriptors,(ListItem*)sem_des);
	 
	Semaphore* sem = sem_des -> semaphore;
	
	SemDescriptorPtr* desptr = (SemDescriptorPtr*)List_detach(&sem->descriptors,(ListItem*)(sem_des->ptr));
	
	SemDescriptor_free(sem_des);
	SemDescriptorPtr_free(desptr);
	
	if(sem -> descriptors.size == 0){
		printf("Libero semaforo\n\n");
		sem = (Semaphore*) List_detach(&semaphores_list, (ListItem*) sem);
		Semaphore_free(sem);
		running -> syscall_retvalue = 0;
		return;
	}
	
	running -> syscall_retvalue = -1;
}
