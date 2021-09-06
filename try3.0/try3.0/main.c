#define _GNU_SOURCE 

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pthread.h>
#include <wait.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "datamgr.h"
#include "sensor_db.h"
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
#define LOG_FILE_NAME "gateway.log"
#define MAX     120

sbuffer_t* shared_buffer=NULL;  
pthread_mutex_t thread_lock;
int connmgr_finished=0;

static int print_sequency_number=0;
void print_to_file(FILE * fp,char * print_buf);
void run_log(void);

int callbackf(void *NotUsed, int argc, char ** argv, char **azColName){ 
  NotUsed=0;
  for(int i=0;i<argc;i++){// argc is the number of columns in the result
    printf("%s=%s ",azColName[i],argv[i]?argv[i]:"NULL");//argv is an array of string representing the fileds in the row
                                          //azCOlName is an array of string representing column names
  }
  printf("\n");
  return 0;
} 

//below are the three thread functions 

//this function waiting for TCP connection, write the data to the shared buffer
void* thread_function_connmgr(void *arg)
{
  int server_port=*((int*)arg);
  char * message_to_send="Hello World! Bonjore!\n";
  send_to_log( message_to_send);
   connmgr_listen(server_port,&shared_buffer);
   printf("We got here!");

   pthread_mutex_lock(&thread_lock);
   connmgr_finished=1;
   pthread_mutex_unlock(&thread_lock);

   connmgr_free();
   sbuffer_print(&shared_buffer);
   //pthread_exit( NULL );
   return NULL;
 // exit(EXIT_SUCCESS);
}
//this function fetch sensor data, 
void* thread_function_datamgr(void *arg)
{//we must also remove it in the databasemgr, or the node always exsit and then program never goes out!
    FILE* fpmap=fopen("room_sensor.map","r");

      pthread_mutex_lock(&thread_lock);
    do{
      pthread_mutex_unlock(&thread_lock);
      sleep(1);
      datamgr_parse_sensor_data(fpmap, &shared_buffer);
      pthread_mutex_lock(&thread_lock);
    }while(connmgr_finished==0);
      pthread_mutex_unlock(&thread_lock);

    printf("now datamgr goes out\n");
    fclose(fpmap);
    datamgr_free();
  return NULL;
}

//
void* thread_function_databasemgr(void *arg)
{
  DBCONN* db= init_connection(1);
  if(db==NULL)
  {
     int connectiontry=0;
    do{
      sleep(3);
      db= init_connection(0);
      connectiontry++;
      }while( connectiontry<3&&db==NULL);
      if(db==NULL)//after 3 tries it still can not connect
      {
        return NULL;//terminate the whole proram........
      }
  }
//here the connection has established!
      pthread_mutex_lock(&thread_lock);
    do{
      pthread_mutex_unlock(&thread_lock);
      storagemgr_parse_sensor_data(db, &shared_buffer);
      pthread_mutex_lock(&thread_lock);
    }while(connmgr_finished==0);
      pthread_mutex_unlock(&thread_lock);

  printf("now databasemgr goes out\n");
    
   int sqlresult=0;
   printf("Now print everything we put in the database!!!!\n");
   sqlresult= find_sensor_all(db, callbackf);
   if(sqlresult==-1){exit(EXIT_FAILURE);}
    disconnect(db);
  return NULL;
}


int main(int argc, char *argv[])
{
  pid_t child_pid;//this is for the log process creaion
  
  printf("Start to fork!\n");
  child_pid=fork();
  SYSCALL_ERROR(child_pid);
  if(child_pid==0){
    //this is the log process
     run_log();
  }
  else
  { //below is for the main process
    int server_port;
    int result;//an interger for error handling

    result=mkfifo(FIFO_NAME,0666);
    CHECK_MKFIFO(result);
    printf("FIFO creaion succeeds!\n");

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
    

   // fp points to FIFO is initialize in sbuffer_init
   if(sbuffer_init(&shared_buffer)!=0)
   {
      printf("Couldn't initialize the shared buffer!\n"); 
      exit(EXIT_FAILURE);
    }     
    result=pthread_mutex_init(&thread_lock,NULL);
    MAIN_ERR_HANDLER(result!=0);

    pthread_mutex_lock(&thread_lock);
    connmgr_finished=0;
    pthread_mutex_unlock(&thread_lock);

    presult=pthread_create(&thread_connmgr, NULL,thread_function_connmgr,&server_port);
    MAIN_ERR_HANDLER(presult!=0);

    presult=pthread_create(&thread_datamgr, NULL,thread_function_datamgr,NULL);
    MAIN_ERR_HANDLER(presult!=0);

    presult=pthread_create(&thread_databasemgr, NULL,thread_function_databasemgr,NULL);
    MAIN_ERR_HANDLER(presult!=0);
    
    presult=pthread_join(thread_connmgr,NULL);
    MAIN_ERR_HANDLER(presult!=0);
 
    presult=pthread_join(thread_datamgr,NULL);
    MAIN_ERR_HANDLER(presult!=0);    

    presult=pthread_join(thread_databasemgr,NULL);
    MAIN_ERR_HANDLER(presult!=0);

    printf("We got here!\n");
    kill(child_pid,SIGCONT);

   if(sbuffer_free(&shared_buffer)!=0)
   {
      printf("Couldn't free the shared buffer!\n"); 
      exit(EXIT_FAILURE);
   }   
   SYSCALL_ERROR(waitpid(child_pid,NULL,0) );
     //fp is closed in sbuffer_free
    result=pthread_mutex_destroy(&thread_lock);
    MAIN_ERR_HANDLER(result!=0);
    printf("Well Done! Good Boy!\n");
   
  }
  return 0;
}  

void run_log(void){
  FILE * fp; 
  FILE * fplog;
  int result;//an interger for error handling
  print_sequency_number=0;
  char *str_result;
  char recv_buf[MAX]; 
  printf("Now the log process begins!\n");

  //pid_t my_pid;
  //pid_t parent_pid; 
  //my_pid = getpid();
  //parent_pid = getppid();

  fp = fopen(FIFO_NAME, "r"); 
  printf("synchronizing with main process ok\n");
  FILE_OPEN_ERROR(fp);
  fplog=fopen("gateway.log","w");
  FILE_OPEN_ERROR(fplog);
  printf("We get here1\n");

  printf("The first print finished!\n");
  printf("Now the log process is waiting for main process to end!\n");
  raise(SIGSTOP);
   do 
  {
    str_result = fgets(recv_buf, MAX, fp);
    if ( str_result != NULL )
    { 
      printf("Message received in log process: %s\n", recv_buf);
      print_to_file(fplog,recv_buf);  

      // print_sequency_number++;
      // fprintf(fplog, "%d ",print_sequency_number);
      // fprintf(fplog, "%s\n",recv_buf);
    }
  }while( str_result != NULL ); 
  printf("Now Log process goes out !!!!!!!!!!!!!!!!!!!!!!!\n");

  result = fclose( fp );
  FILE_CLOSE_ERROR(result);
  result = fclose( fplog );
  FILE_CLOSE_ERROR(result);
  result=remove(FIFO_NAME);
  REMOVE_ERROR(result);

  exit(EXIT_SUCCESS);
}


/*this function print the char* to the fp with
the formet <sequence number><timestamp><log message>
*/
void print_to_file(FILE * fp, char* print_buf){
  if(fp==NULL){
    perror("fplog is NULL");
    exit(EXIT_FAILURE);
  }
  print_sequency_number++;

  char sequence_number[10];
  sprintf(sequence_number,"%d ",print_sequency_number);

  char  message_to_print[120];
  strcpy(message_to_print,sequence_number);
  strcat(message_to_print,print_buf);
  fprintf(fp, "%s\n",message_to_print);
}



