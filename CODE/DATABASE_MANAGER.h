/****

This is the database manager implementation.
This module contains the methods to manage the student database and
supports the following operations:
1.	initialize the database
2.	create a table
3.	insert entries
4.	update entries

@author:	PKC

****/


#include "DEFINITIONS.h"
#include "CONSTANTS.h"
#define NO_OF_STUDENTS 3

using namespace std;


static int callback(void *NotUsed, int argc, char **argv, char **azColName) {
   	int i;
  	for(i = 0; i<argc; i++) 
   	{
    	printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
   	}
   	printf("\n");
   	return 0;
}

sqlite3* open_database(string db_name){
	/**
		This method opens a database with the given name in SERVER system
		and returns the pointer corresponding to the database.
	**/
	sqlite3 *db;
  	char *sql;
   	string db_path	=	SERVER_FILE_PATH;
   	db_path 		+=	db_name;
   	db_path			+=	".db";
   	int rc = sqlite3_open(db_path.c_str(), &db);
  
	if( rc ) 
	{
		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
	    return NULL;
	} 
	else
	{
		fprintf(stdout, "Opened database successfully\n");
		return db;
	}
}

int execute_sql(string sql, sqlite3 *db){
	/**
		This method simply takes in a sql command as a string 
		and executes it on the database pointed by db.
	**/
	char *zErrMsg = 0;
	int rc;
	rc = sqlite3_exec(db, sql.c_str(), callback, 0, &zErrMsg);
	if( rc != SQLITE_OK )
	{
   		fprintf(stderr, "SQL error: %s\n", zErrMsg);
    	sqlite3_free(zErrMsg);
    	return 0;
	} 
  	else 
   	{
    	fprintf(stdout, "Command executed successfully\n");
      	return 1;
   	}
}

int insert_val(string roll_no, sqlite3 *db){
	/**
		This method inserts a row into the 
		database.
		Input:
		@roll_no:	The roll number of student.
	**/
	string sql 	=	"INSERT INTO STUDENT_DB (ROLL_NO,PROB_1,PROB_2,PROB_3) ";
	sql 		+=	"VALUES (";
	sql 		+=	roll_no;
	sql 		+=  ", 0, 0, 0);";

	execute_sql(sql, db);
}


void initialize_database(sqlite3 *db){
	/**
		This method initializes the database pointed 
		by db.
		It stores the roll numbers and initializes the problem 
		values to 0.
	**/
	string roll_nos[] = {"157148", "157149", "157156"};

	for(int i = 0; i < NO_OF_STUDENTS; i++)
	{
		insert_val(roll_nos[i], db);
	}

}

void create_table(sqlite3 *db){
	/**
		This method creates a table in the database
		pointed by db.
	**/

	string sql = 	"CREATE TABLE IF NOT EXISTS STUDENT_DB("  \
    				"ROLL_NO TEXT PRIMARY KEY     NOT NULL," \
      				"PROB_1         INT    NOT NULL," \
      				"PROB_2         INT     NOT NULL," \
      				"PROB_3         INT	);";

	execute_sql(sql, db);
	initialize_database(db);
}

void update_val(string roll_no, string qno, sqlite3 *db){
	/**
		This method updates the column number corresponding 
		to the Question solved successfully by the student
		with roll number roll_no;
	**/

	string prob 	=	"PROB_";
	prob 			+= 	qno;

	string sql 		=	"UPDATE STUDENT_DB set ";
	sql 		 	+= 	prob;
	sql 			+=  " = 1 where ROLL_NO = ";
	sql 			+=	roll_no;
	sql 			+=	";";

	execute_sql(sql, db);
}



void close_database(sqlite3 *db){
	/**
		This method closes the opened database passed
		as db
	**/
	sqlite3_close(db);
}