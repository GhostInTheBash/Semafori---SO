#define _DISASTROS_DEBUG_

#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include "disastrOS.h"
#include "disastrOS_syscalls.h"
#include "disastrOS_semaphore.h"
#include "disastrOS_semdescriptor.h"

/*increments  (unlocks)  the semaphore pointed to by sem.  
 * If the semaphore's value consequently  becomes  greater  than  zero,  
 * then another  process  or thread blocked in a sem_wait call 
 * will be woken up and proceed to lock the semaphore.
*/


void internal_semPost(){
	
	printf("\nInizio semPost \n\n");
	
	int sem_fd = running->syscall_args[0];
	SemDescriptor* sem_d = SemDescriptorList_byFd(&running->sem_descriptors,sem_fd);
	
	if(!sem_d){
		disastrOS_debug("Errore sul descrittore \n");
		running->syscall_retvalue =  DSOS_ESEMFD;
		return;
	}
	
	Semaphore* sem = sem_d->semaphore;
	
	if(!sem){
		disastrOS_debug("Semaforo non esistente \n");
		running->syscall_retvalue =  DSOS_ESEMERR;
		return;
	}
	    
	sem -> count++;
	
	// inserisco il processo in running nella lista dei ready
	
	if(sem->count <= 0){ 
        disastrOS_printStatus();
        
        List_insert(&ready_list, ready_list.last, (ListItem*) running);
    
        // elimino il primo elemento di waiting_descriptors dalla lista sem->waiting_descriptors
        
        SemDescriptorPtr* sem_descr_ptr = (SemDescriptorPtr*) List_detach(&sem->waiting_descriptors, (ListItem*) sem->waiting_descriptors.first); 
         
        List_insert(&sem->descriptors, sem->descriptors.last, (ListItem*) sem_descr_ptr); 

        
        // rimuovo il processo dalla lista di waiting, attravesro il descrittore preso in precedenza
        // questo sarà il prossimo processo ad andare in esecuzione. (preso tra i descrittori in waiting)
        
        List_insert(&ready_list,ready_list.last,(List_detach(&waiting_list, (ListItem*) sem_descr_ptr->descriptor->pcb))); 
        
        // qui lo metto in running
     
        running -> status = Ready;
      
        running = sem_descr_ptr->descriptor->pcb;
      
        running -> status = Running;
        
        disastrOS_printStatus();
        
    }

	printf("Fine semPost \n\n");
    running->syscall_retvalue = 0;
}
