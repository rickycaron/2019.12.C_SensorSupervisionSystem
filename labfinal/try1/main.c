#define _GNU_SOURCE 

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pthread.h>
#include <wait.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "errmacros.h"
//#include "datamgr.h"
//#include "sensor_db.h"
#include "connmgr.h"

#ifdef DEBUG
  #define DEBUG_PRINTF(condition,...)                 \
    do {                        \
       if((condition))                    \
       {                        \
      fprintf(stderr,"\nIn %s - function %s at line %d: ", __FILE__, __func__, __LINE__); \
      fprintf(stderr,__VA_ARGS__);                \
       }                        \
    } while(0)            
#else
  #define DEBUG_PRINTF(...) (void)0
#endif

#define MAIN_ERR_HANDLER(condition,...)  \
  do {            \
    if ((condition))      \
    {         \
      DEBUG_PRINTF(1,"error condition \"" #condition "\" is true\n"); \
      __VA_ARGS__;        \
    }         \
  } while(0)
//below is the important global variable 
#define FIFO_NAME   "logFifo"
pthread_mutex_t sensor_data_mutex; 

int send_to_log(char * send_buf, FILE *fp);

//below are the three thread functions 

//this function waiting for TCP connection, write the data to the shared buffer
void* thread_function_connmgr(void *arg)
{
  int server_port=*((int*)arg);
   connmgr_listen(server_port);
   connmgr_free();

   //pthread_exit( NULL );
   return NULL;

 // exit(EXIT_SUCCESS);
}
//this function fetch sensor data, 
void* thread_function_datamgr(void *arg)
{
  return NULL;
}

//
void* thread_function_databasemgr(void *arg)
{
  return NULL;
}




int main(int argc, char *argv[])
{
  pid_t child_pid;//this is for the log process creaion
  int result;//an interger for error handling
  
  result=mkfifo(FIFO_NAME,0666);
  CHECK_MKFIFO(result);
  printf("FIFO creaion succeeds!\n");

  printf("Start to fork!\n");
  child_pid=fork();
  SYSCALL_ERROR(child_pid);
  if(child_pid==0){
    //this is the log process
    char* program="./log";
    char* arg_list[]={program,NULL};
    execvp(program,arg_list);
    //it should never come here 
    perror("\nAn error occured in execvp!\n");
    exit(EXIT_FAILURE);
  }
  else
  { //below is for the main process
    int server_port;
    FILE *fp;//this is for the FIFO communication
    char *send_buf; //this is for the sending of log information
    fp = fopen(FIFO_NAME, "w");
    FILE_OPEN_ERROR(fp); 
    printf("main process synchronizing with log process ok\n");
    //get the server port number from the argument
    if (argc != 2){
      printf("Use this program with 1 command line options: \n");
      printf("\t%-15s : TCP server port number\n", "\'server port\'");
      exit(EXIT_SUCCESS);
    }else{
      server_port = atoi(argv[1]);
    } 
    printf("The server port number is %d\n",server_port);
    int presult;
    pthread_t thread_connmgr;
    pthread_t thread_datamgr;
    pthread_t thread_databasemgr;
    void *thread_result;
    // send_buf="Hello world!";
    // send_to_log(send_buf, fp); 

   // create the server socket 
 
    result=pthread_mutex_init(&sensor_data_mutex,NULL);
    MAIN_ERR_HANDLER(result!=0);

    presult=pthread_create(&thread_connmgr, NULL,thread_function_connmgr,&server_port);
    MAIN_ERR_HANDLER(presult!=0);

    presult=pthread_create(&thread_datamgr, NULL,thread_function_datamgr,NULL);
    MAIN_ERR_HANDLER(presult!=0);

    presult=pthread_create(&thread_databasemgr, NULL,thread_function_databasemgr,NULL);
    MAIN_ERR_HANDLER(presult!=0);





    
    presult=pthread_join(thread_connmgr,NULL);
    MAIN_ERR_HANDLER(presult!=0);
    printf("We got here!\n");

    presult=pthread_join(thread_datamgr,NULL);
    MAIN_ERR_HANDLER(presult!=0);    

    presult=pthread_join(thread_databasemgr,NULL);
    MAIN_ERR_HANDLER(presult!=0);

    result=fclose(fp);
    FILE_CLOSE_ERROR(result);
    result=pthread_mutex_destroy(&sensor_data_mutex);
    MAIN_ERR_HANDLER(result!=0);
    
    exit(EXIT_SUCCESS);
  }

}  



int send_to_log(char * send_buf, FILE *fp){
  if(fputs(send_buf,fp)==EOF)
  {
    fprintf(stderr, "Error writing data to fifo\n");
    exit(EXIT_FAILURE);
  }
  FFLUSH_ERROR(fflush(fp));
  printf("Message from main: %s\n", send_buf);
  return 0;
}








// int main(int argc, char const *argv[])
// {
//   int server_port;
//    // create the server socket 
//   if (argc != 2){
//     printf("Use this program with 1 command line options: \n");
//     printf("\t%-15s : TCP server port number\n", "\'server port\'");
//     exit(EXIT_SUCCESS);
//   }else{
//     server_port = atoi(argv[1]);
//   }
//    connmgr_listen(server_port);
//    connmgr_free();
// // intialize all the client sd with 0
//   return 0;
// }