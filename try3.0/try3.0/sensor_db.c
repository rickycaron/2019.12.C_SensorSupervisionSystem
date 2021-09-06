

#include <stdlib.h>
#include <string.h>
#include "config.h"
#include "sensor_db.h"

// stringify preprocessor directives using 2-level preprocessor magic
// this avoids using directives like -DDB_NAME=\"some_db_name\"
#define REAL_TO_STRING(s) #s
#define TO_STRING(s) REAL_TO_STRING(s)    //force macro-expansion on s before stringify s

#ifndef DB_NAME
  #define DB_NAME Sensor.db
#endif

#ifndef TABLE_NAME
  #define TABLE_NAME SensorData
#endif

#define DBCONN sqlite3 


typedef int (*callback_t)(void *, int, char **, char **);


/*
 * Make a connection to the database server
 * Create (open) a database with name DB_NAME having 1 table named TABLE_NAME  
 * If the table existed, clear up the existing data if clear_up_flag is set to 1
 * Return the connection for success, NULL if an error occurs
 */
DBCONN * init_connection(char clear_up_flag){
	sqlite3 *db;// database handler
	//sqlite3_stmt *res;// a single SQL statement
	char *err_msg=0;// a char pointer for error message
	int rc=sqlite3_open(TO_STRING(DB_NAME),&db); //open a new database connection
		time_t timer=time(NULL);
		char print_time[11];//at leatst 11
		sprintf(print_time,"%ld",timer);
		char * message=(char *)malloc(60*sizeof(char));
		strcpy(message, print_time);
	if(rc!=SQLITE_OK){
		strcat(message," Unable to connect to SQL server!\n");
  		send_to_log(message);
  		free(message);
		fprintf(stderr,"Can't open the database: %s \n",sqlite3_errmsg(db)); // return a descriprion of the error, print it on stderr
		sqlite3_close(db);//release the database connection 
		return NULL;
	}else{
		strcat(message," Connection to SQL server established!\n");
  		send_to_log(message);
  		free(message);		
		fprintf(stdout,"Database " TO_STRING(DB_NAME)" is created successfully!\n ");
	}
	//database connection is created successfully
	
	char sqlstate[200];
	strcpy(sqlstate, 	"DROP TABLE IF EXISTS "                             );
	strcat(sqlstate, 	TO_STRING(TABLE_NAME)                               );
	strcat(sqlstate, 	";"    												);
	strcat(sqlstate, 	"CREATE TABLE "  									);
	strcat(sqlstate, 	TO_STRING(TABLE_NAME)												);
	strcat(sqlstate, 	"(Id INTEGER PRIMARY KEY, sensor_id INT, sensor_value DECIMAL(4,2), timestamp TIMESTAMP);" );
	if(clear_up_flag!=1){
	strcpy(sqlstate, 	"CREATE TABLE IF NOT EXISTS "  									);
	strcat(sqlstate, 	TO_STRING(TABLE_NAME)												);
	strcat(sqlstate, 	" (Id INTEGER PRIMARY KEY, sensor_id INT, sensor_value DECIMAL(4,2), timestamp TIMESTAMP);" );
	}
	printf("%s\n",sqlstate);
	rc=sqlite3_exec(db,sqlstate,0,0,&err_msg);

	char print_time2[11];//at leatst 11
	sprintf(print_time2,"%ld",time(NULL));
		char * message2=(char *)malloc(60*sizeof(char));
		strcpy(message2, print_time2);

	if(rc!=SQLITE_OK){
		strcat(message2,"Connection to SQL server lost!\n");
  		send_to_log(message2);
  		free(message2);
		fprintf(stderr,"SQL error %s \n",err_msg); 
		sqlite3_free(err_msg);//free the allocated memory for the err_msg
		sqlite3_close(db);//release the database connection 
		return NULL;
	}else{
		strcat(message2," New table ");
		strcat(message2,TO_STRING(TABLE_NAME) );
		strcat(message2," created!\n");
  		send_to_log(message2);
  		free(message2);
		fprintf(stdout,"Table " TO_STRING(TABLE_NAME)" is ready to use!\n");
	}

	return db;
}


/*
 * Disconnect from the database server
 */
void disconnect(DBCONN *conn){
	//close the database connection
	if(sqlite3_close(conn)==SQLITE_OK){
		char print_time[11];//at leatst 11
	    sprintf(print_time,"%ld",time(NULL));
		char * message=(char *)malloc(60*sizeof(char));
		strcpy(message, print_time);
		strcat(message," Connection to SQL server is disconnectted!\n");
  		send_to_log(message);
  		free(message);
	}
}


/*
 * Write an INSERT query to insert a single sensor measurement
 * Return zero for success, and non-zero if an error occurs here is -1
 */
int insert_sensor(DBCONN * conn, sensor_id_t id, sensor_value_t value, sensor_ts_t ts){
	char *err_msg=0;
	// no one wil know whether id is a uint16_t or just a string
	// we must get the value of id out and store it into a string
	char idstring[10];
	char valuestring[15];
	char tsstring[15];
	//both function is ok, but do not use asprinf or free it 
	snprintf(idstring,sizeof(idstring),"%u",id);
	snprintf(valuestring,sizeof(valuestring),"%g",value);
	snprintf(tsstring,sizeof(tsstring),"%ld",ts);
	// sprintf(idstring,"%u",id);
	// sprintf(valuestring,"%g",value);
	// sprintf(tsstring,"%ld",ts);

	char insertsql[200];
	strcpy(insertsql, 	"INSERT INTO "                                      );
	strcat(insertsql, 	TO_STRING(TABLE_NAME)                               );
	strcat(insertsql, 	" (sensor_id, sensor_value, timestamp) VALUES("     );
	strcat(insertsql, 	idstring											);
	strcat(insertsql, 	", " 												);
	strcat(insertsql, 	valuestring								    		);
	strcat(insertsql, 	", " 												);
	strcat(insertsql, 	tsstring                                      		);
	strcat(insertsql, 	");"				 								);
	//printf("%s\n",insertsql);
	//char*insertsql="INSERT INTO "TO_STRING(TABLE_NAME)" (sensor_id, sensor_value, timestamp) VALUES(("TO_STRING(senid)"), ("TO_STRING(value)"),( "TO_STRING(ts)"));";
	int rc=sqlite3_exec(conn,insertsql,0,0,&err_msg);
	if(rc!=SQLITE_OK){
		fprintf(stderr, "SQL insert meets error! %s\n",err_msg );
		sqlite3_free(err_msg);
		return -1;
	}else{
		//fprintf(stdout, "SQL insert is success!\n" );
		return 0;
	}
}


/*
 * Write an INSERT query to insert all sensor measurements available in the file 'sensor_data'
 * Return zero for success, and non-zero if an error occurs
 */
int insert_sensor_from_file(DBCONN * conn, FILE * sensor_data){
	//char *err_msg=0;

	sensor_id_t idv=0;// these are only variables for manipulating the value in this functions
	sensor_value_t valuev=0;
	sensor_ts_t tsv=0;

	rewind(sensor_data);


	while(feof(sensor_data)==0){
			int insertresult;
			fread(&idv,sizeof(sensor_id_t),1,sensor_data);
			fread(&valuev,sizeof(sensor_value_t),1,sensor_data);
			fread(&tsv,sizeof(sensor_ts_t),1,sensor_data);// read a line value of one sensor
	insertresult=insert_sensor(conn,idv,valuev,tsv);
	if(insertresult!=0){
		fprintf(stderr,"File insert failed!\n");
		return -1;}
}
	fprintf(stdout,"File insert succeed!");
	return 0;
}
/*
 * Reads continiously all data from the shared buffer data structure and stores this into the database
 * When *buffer becomes NULL the method finishes. This method will NOT automatically disconnect from the db
 */
void storagemgr_parse_sensor_data(DBCONN * conn, sbuffer_t ** buffer){
	sensor_id_t idv=0;// these are only variables for manipulating the value in this functions
	sensor_value_t valuev=0;
	sensor_ts_t tsv=0;
	int insertresult=-1;
	int res;
    sensor_data_t  datagot;

   	res=sbuffer_remove_in_databasemgr(*buffer,&datagot);
while(res!=1&&res!=-1)
{
    if(res==0||res==3)
    {
      printf("databasemgr has some data to read in shared buffer!\n");
      sbuffer_print(buffer);
      idv=datagot.id;
      valuev=datagot.value;
      tsv=datagot.ts;
	  insertresult=insert_sensor(conn,idv,valuev,tsv);
	  	if(insertresult!=0)
	  	{
		fprintf(stderr,"File insert failed!\n");
		exit(EXIT_FAILURE);
	  	}
    }
     res=sbuffer_remove_in_databasemgr(*buffer,&datagot);
}

}


/*
  * Write a SELECT query to select all sensor measurements in the table 
  * The callback function is applied to every row in the result
  * Return zero for success, and non-zero if an error occurs
  */
int find_sensor_all(DBCONN * conn, callback_t f){
	char *err_msg=0;
	char sqlselect[100];
	strcpy(sqlselect, 	"SELECT * FROM "                                    );
	strcat(sqlselect, 	TO_STRING(TABLE_NAME)                               );	
	//char *sqlselect="SELECT * FROM" TO_STRING(TABLE_NAME);
	printf("%s\n",sqlselect );
	int rc=sqlite3_exec(conn,sqlselect,f,0,&err_msg);
	if(rc!=SQLITE_OK){
		fprintf(stderr, "Failed to select the file! %s\n",err_msg );
		sqlite3_free(conn);
		return -1;
	}else{
		fprintf(stdout, "Succeed to select the file!\n" );
		return 0;
	}
}


/*
 * Write a SELECT query to return all sensor measurements having a temperature of 'value'
 * The callback function is applied to every row in the result
 * Return zero for success, and non-zero if an error occurs
 */
int find_sensor_by_value(DBCONN * conn, sensor_value_t value, callback_t f){
	char *err_msg=0;
	char valuestring[15];
	sprintf(valuestring," %g",value);
	//sqlite3_stmt *sqlfsbv;
	char sqlselect[100];
	strcpy(sqlselect, 	"SELECT * FROM "                                    );
	strcat(sqlselect, 	TO_STRING(TABLE_NAME)                               );
	strcat(sqlselect, 	" WHERE sensor_value ="                             );
	strcat(sqlselect, 	valuestring                                         );	
    printf("%s\n",sqlselect );
	//char* sql="SELECT * FROM "TO_STRING(TABLE_NAME)" WHERE sensor_value ="TO_STRING(value);
	int rc=sqlite3_exec(conn,sqlselect,f,0,&err_msg);
	if(rc !=SQLITE_OK){
		fprintf(stderr, "Failed to select data by value!%s\n", err_msg);
		sqlite3_free(err_msg);
		return -1;
	}else{
		fprintf(stdout, "Succeed to select by value!\n");
		return 0;
	}
}


/*
 * Write a SELECT query to return all sensor measurements of which the temperature exceeds 'value'
 * The callback function is applied to every row in the result
 * Return zero for success, and non-zero if an error occurs
 */
int find_sensor_exceed_value(DBCONN * conn, sensor_value_t value, callback_t f){
	char *err_msg=0;
	char valuestring[15];
	sprintf(valuestring," %g",value);
	//sqlite3_stmt *sqlfsbv;
	char sqlselect[100];
	strcpy(sqlselect, 	"SELECT * FROM "                                    );
	strcat(sqlselect, 	TO_STRING(TABLE_NAME)                               );
	strcat(sqlselect, 	" WHERE sensor_value >"                             );
	strcat(sqlselect, 	valuestring                                         );	
    printf("%s\n",sqlselect );
	//char *sql="SELECT * FROM " TO_STRING(TABLE_NAME) "WHERE sensor_value >" TO_STRING(value);
	int rc=sqlite3_exec(conn,sqlselect,f,0,&err_msg);
	if(rc!=SQLITE_OK){
		fprintf(stderr, "Failed to select by exceeding value! %s\n",err_msg);
		sqlite3_free(err_msg);
		return -1;
	}else{
		fprintf(stderr, "Succeed to select by exceeding value! \n");
		return 0;
	}

}


/*
 * Write a SELECT query to return all sensor measurements having a timestamp 'ts'
 * The callback function is applied to every row in the result
 * Return zero for success, and non-zero if an error occurs
 */
int find_sensor_by_timestamp(DBCONN * conn, sensor_ts_t ts, callback_t f){
	char* err_msg=0;
	char tsstring[15];
	sprintf(tsstring," %ld",ts);
	//sqlite3_stmt *sqlfsbv;
	char sqlselect[100];
	strcpy(sqlselect, 	"SELECT * FROM "                                    );
	strcat(sqlselect, 	TO_STRING(TABLE_NAME)                               );
	strcat(sqlselect, 	" WHERE timestamp = "                             );
	strcat(sqlselect, 	tsstring                                         );	
    printf("%s\n",sqlselect );
	//char* sql="SELECT * FROM" TO_STRING(TABLE_NAME) "WHERE timestamp =" TO_STRING(ts) ;
	int rc=sqlite3_exec(conn,sqlselect,f,0,&err_msg);
	if(rc!=SQLITE_OK){
		fprintf(stderr, "Failed to select by timestamp! %s\n",err_msg);
		sqlite3_free(err_msg);
		return -1;
	}else{
		fprintf(stderr, "Succeed to select by timestamp! \n");
		return 0;
	}
}


/*
 * Write a SELECT query to return all sensor measurements recorded after timestamp 'ts'
 * The callback function is applied to every row in the result
 * return zero for success, and non-zero if an error occurs
 */
int find_sensor_after_timestamp(DBCONN * conn, sensor_ts_t ts, callback_t f){
	char* err_msg=0;
	char tsstring[15];
	sprintf(tsstring," %ld",ts);
	//sqlite3_stmt *sqlfsbv;
	char sqlselect[100];
	strcpy(sqlselect, 	"SELECT * FROM "                                    );
	strcat(sqlselect, 	TO_STRING(TABLE_NAME)                               );
	strcat(sqlselect, 	" WHERE timestamp > "                               );
	strcat(sqlselect, 	tsstring                                            );	
    printf("%s\n",sqlselect );
	//char* sql="SELECT * FROM" TO_STRING(TABLE_NAME) "WHERE timestamp >" TO_STRING(ts) ;
	int rc=sqlite3_exec(conn,sqlselect,f,0,&err_msg);
	if(rc!=SQLITE_OK){
		fprintf(stderr, "Failed to select after timestamp! %s\n",err_msg);
		sqlite3_free(err_msg);
		return -1;
	}else{
		fprintf(stderr, "Succeed to select after timestamp! \n");
		return 0;
	}
}

