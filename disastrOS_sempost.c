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
 * then another  process  or thread blocked in a sem_wait(3) call 
 * will be woken up and proceed to lock the semaphore.
*/


void internal_semPost(){
	
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
	
	printf("incremento contatore %d\n\n", sem -> count);
	    
	sem -> count++;
	
	printf("incrementato contatore %d\n\n", sem -> count);
	
	if(sem->count <= 0){ // se il contatore è <=0, inserisco il processo in running nella lista dei ready
        List_insert(&ready_list, ready_list.last, (ListItem*) running);
    
        // elimino il primo elemento di waiting_descriptors dalla lista sem->waiting_descriptors
        SemDescriptorPtr* sem_descr_ptr = (SemDescriptorPtr*) List_detach(&sem->waiting_descriptors, (ListItem*) sem->waiting_descriptors.first); //rimuovo il first descriptor del semaforo dalla lista dei descrittori in attesa
        List_insert(&sem->descriptors, sem->descriptors.last, (ListItem*) sem_descr_ptr); // lista dei descrittori attivi
        List_detach(&waiting_list, (ListItem*) sem_descr_ptr->descriptor->pcb); // rimuovo il processo dalla waiting_list tramite il suo puntatore al descrittore

        running->status = Ready;
        running = sem_descr_ptr->descriptor->pcb; // metto in esecuzione il processo rimosso dalla waiting_list (pcb del semaforo)
    }

    // se tutta l'operazione va a buon fine
    running->syscall_retvalue = 0;
	
	
}
