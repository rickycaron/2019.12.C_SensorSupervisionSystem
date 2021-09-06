//#include <config.h>
// I don't know why, but the <config.h> won't be compired but the down 
//formet is ok
#include "config.h"
//#include <sensor_db.c>
#include "sensor_db.h"
int callbackf(void *NotUsed, int argc, char ** argv, char **azColName){	
	NotUsed=0;
	for(int i=0;i<argc;i++){// argc is the number of columns in the result
		printf("%s=%s ",azColName[i],argv[i]?argv[i]:"NULL");//argv is an array of string representing the fileds in the row
		                                      //azCOlName is an array of string representing column names
	}
	printf("\n");
	return 0;
} 

int main(int argc, char const *argv[])
{
	/* code */
	FILE* sensor_db_ptr=fopen("sensor_data","rb");
	DBCONN* db= init_connection(1);
	printf("The version for the SQLite library is %s\n", sqlite3_libversion());
	
	int insertrs=insert_sensor_from_file(db, sensor_db_ptr);
	if(insertrs==0){
		printf("Sucessful insert in main!\n");
	}else{
		printf("Failed insertion in main! stop proceeding!\n");
	}
	 int sqlresult=0;
	 printf("We got here!\n");
	 //sqlresult= find_sensor_all(db, callbackf);

	 sensor_value_t senvalue=25;

	 sqlresult= find_sensor_by_value(db, senvalue, callbackf);

	 sqlresult= find_sensor_exceed_value(db, senvalue, callbackf);

	 sensor_ts_t sents=1575820542;

	 sqlresult= find_sensor_by_timestamp(db, sents, callbackf);

	 sqlresult= find_sensor_after_timestamp(db,sents, callbackf);

	disconnect(db);
	return 0;
}