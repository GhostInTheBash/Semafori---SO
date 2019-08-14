#include <stdio.h>
#include <unistd.h>
#include <poll.h>
#include <stdlib.h>

#include "disastrOS.h"

// we need this to handle the sleep state

// termina quando il processo non ha più figli?? (Vedere disastrOS_wait)

void sleeperFunction(void* args){
  printf("Hello, I am the sleeper, and I sleep %d\n",disastrOS_getpid());
  while(1) {
    getc(stdin);
    //disastrOS_printStatus();
  }
}

void accedi_risorsa(){
	printf("Semaforo del processo %d ha accesso alla risorsa\n",(disastrOS_getpid()));
	disastrOS_printStatus();
	disastrOS_sleep((20-disastrOS_getpid())*2);
	disastrOS_printStatus();
	printf("Semaforo del processo %d lascia la risorsa\n",(disastrOS_getpid()));
}

// testo apertura MAX_NUM_SEMDES e varie aperture/chiusure non possibili
void childFunction2(void* args){
  // apro semafori
  printf("\nApro MAX_NUM semafori per il processo\n");
  for(int id = 0; id < MAX_NUM_SEMDESCRIPTORS_PER_PROCESS; id++){
	  int sem = disastrOS_semopen(id, 1);
	  printf("Sem_fd: %d\n",sem);
	  if(sem < 0){
		printf("ERRORE, %d\n\n",sem);
		disastrOS_exit(EXIT_FAILURE);
	}
  }
  printf("\n%d semafori aperti\n\n",MAX_NUM_SEMDESCRIPTORS_PER_PROCESS );
  
  //apro altro semaforo, errore!
  printf("Apro semaforo di troppo\n\n");
  disastrOS_semopen(MAX_NUM_SEMDESCRIPTORS_PER_PROCESS,1);
  
  printf("Chiudo semafori aperti\n\n");
  disastrOS_printStatus();
  for(int id = 0; id < MAX_NUM_SEMDESCRIPTORS_PER_PROCESS; id++){
	int sem = disastrOS_semclose(id);
	if(sem){
		printf("Semaforo non chiuso\n");
	}
  }
  disastrOS_printStatus();
  
  printf("Chiudo sem mai aperto nel processo\n");
  int sem_close = disastrOS_semclose(MAX_NUM_SEMDESCRIPTORS_PER_PROCESS);
  printf("Errore: %d\n",sem_close);
  
  printf("\nWait e Post su sem mai aperto nel processo:");
  int sem_wait = disastrOS_semwait(1);
  printf("Errore: %d\n", sem_wait);
  
  int sem_post = disastrOS_sempost(1);
  printf("Errore: %d\n\n", sem_post);
  
  
  printf("Apro nuovi semafori\n");
  int sem1 = disastrOS_semopen(1,1);
  int sem2 = disastrOS_semopen(2,3);
  int sem3 = disastrOS_semopen(1,1); //stesso id, fd diverso
  
  if(sem1 < 0 || sem2 < 0 || sem3 < 0){
	  printf("Errore\n\n");
	disastrOS_exit(EXIT_FAILURE);
  }
  
  printf("\nPID: %d \n\n", disastrOS_getpid());
  if(disastrOS_semwait(sem1) || disastrOS_semwait(sem2))
	disastrOS_exit(EXIT_FAILURE);
	  
	  // critical section
     accedi_risorsa();
      
   if(disastrOS_sempost(sem2) ||  disastrOS_sempost(sem3))
     disastrOS_exit(EXIT_FAILURE);
  
  
  if(disastrOS_semclose(sem1) || disastrOS_semclose(sem2) || disastrOS_semclose(sem3))
	disastrOS_exit(EXIT_FAILURE);


  disastrOS_exit(disastrOS_getpid()+1);

}

// testo semaforo con contatore negativo, più processi accedono con lo stesso
// semaforo alla risorsa. 
// Un solo processo per volta può accedere alla risorsa poichè ho contatore pari a 1

void childFunction(void* args){
  disastrOS_printStatus();
  
  int sem = disastrOS_semopen(1, 1);
  
  if(sem)
	exit(EXIT_FAILURE);
  
  if(disastrOS_semwait(sem))
	exit(EXIT_FAILURE);
  
  accedi_risorsa();
  
  if(disastrOS_sempost(sem))
	exit(EXIT_FAILURE);
  
  disastrOS_printStatus();
  
  
  if(disastrOS_semclose(sem) == 0){
	  printf("Semaforo chiuso \n\n");
  }
  else{
	  printf("Il semaforo non è chiuso\n\n");
  }
  
  disastrOS_exit(disastrOS_getpid()+1);
  
}


void initFunction(void* args) {
  disastrOS_printStatus();
  printf("hello, I am init and I just started con pid %d\n",disastrOS_getpid());
  disastrOS_spawn(sleeperFunction, 0);
  

  printf("I feel like to spawn 10 nice threads\n");
  int alive_children=0;
  for (int i=0; i<4; ++i) {
    int type=0;
    int mode=DSOS_CREATE;
    printf("mode: %d\n", mode);
    printf("opening resource (and creating if necessary)\n");
    int fd=disastrOS_openResource(i,type,mode);
    printf("fd=%d\n", fd);
    disastrOS_spawn(childFunction, 0);
    alive_children++;
  }

  //disastrOS_printStatus();
  int retval;
  int pid;

  while(alive_children>0 && (pid=disastrOS_wait(0, &retval))>=0){ 
    disastrOS_printStatus();
    printf("initFunction, child: %d terminated, retval:%d, alive: %d \n",
	   pid, retval, alive_children);
    --alive_children;
  }
  printf("shutdown!\n");
  disastrOS_shutdown();
}

int main(int argc, char** argv){
  char* logfilename=0;
  if (argc>1) {
    logfilename=argv[1];
  }
  // we create the init process processes
  // the first is in the running variable
  // the others are in the ready queue
  printf("the function pointer is: %p\n", childFunction);
  // spawn an init process
  printf("start \n");
  disastrOS_start(initFunction, 0, logfilename);
  return 0;
}
