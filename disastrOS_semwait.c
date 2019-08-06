#define _DISASTROS_DEBUG_

#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include "disastrOS.h"
#include "disastrOS_syscalls.h"
#include "disastrOS_semaphore.h"
#include "disastrOS_semdescriptor.h"


/*decrements (locks) the semaphore pointed to by sem_d.
 *   
 * Se il valore del semaforo è maggiore di zero,
 * quindi il decremento procede,
 * la funzione ritorna immediatamente.
 * Se il semaforo ha attualmente un valore inferiore a zero,
 * il processo va in waiting, finché non diventa possibile
 * eseguire il decremento (ovvero, il valore del semaforo sale sopra lo zero)
 */

void internal_semWait(){
	
	printf("\n\n Inizio semWait \n\n");
	
	int sem_fd = running->syscall_args[0];

	SemDescriptor* sem_d = SemDescriptorList_byFd(&running->sem_descriptors,sem_fd);
	
	if(!sem_d){
		disastrOS_debug("Errore sul descrittore\n");
		running->syscall_retvalue =  DSOS_ESEMFD;
		return;
	}
		
	printf("Descrittore trovato, prendo il semaforo\n\n");
	
	Semaphore* sem = sem_d->semaphore;
	
	if(!sem){
		disastrOS_debug("Semaforo non esistente \n");
		running->syscall_retvalue =  DSOS_ESEMERR;
		return;
	}

	SemDescriptorPtr* sem_ptr = sem_d -> ptr;
	
	if(!sem_ptr){
		disastrOS_debug("Errore nel puntatore \n");
		running->syscall_retvalue =  DSOS_ESEMDESC;
		return;
	}
	
	if(sem -> count <= 0){
		printf("Contatore minore di 0, mando un nuovo processo in esecuzione\n\n\n");
		// rimuovo il descrittore del processo dalla lista dei descrittori(sem->descriptors)
		List_detach(&sem->descriptors, (ListItem*)sem_ptr); 
		// inserisco il descrittore in fondo alla lista di waiting
		List_insert(&sem->waiting_descriptors, sem->waiting_descriptors.last, (ListItem*) sem_d->ptr);
		
		running->status = Waiting; // nuovo stato del processo 
		
		// inserisco il processo in esecuzione nella waiting_list
		List_insert(&waiting_list, waiting_list.last, (ListItem*) running);
			
		// prendo e metto in esecuzione il primo elemento della lista di ready
		PCB* p = (PCB*)List_detach(&ready_list,ready_list.first);
		running = p;
		running->status = Running; 
	}
		
	printf("count: %d  ", sem -> count);
	sem -> count--;
	printf("count: %d\n\n", sem -> count);
	printf("La chiamata alla semWait è terminata\n\n");
	disastrOS_printStatus();
	running->syscall_retvalue = 0;
}
