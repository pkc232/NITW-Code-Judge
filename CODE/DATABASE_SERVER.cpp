/****
	This module contains the implementation of the database
	server.
	This server continuously runs in the background, 
	takes the results from the client and update the database correspondingly.

	@author:	PKC

****/



#include "DEFINITIONS.h"
#include "CONSTANTS.h"
#include "COMMON_FUNCTIONALITY.h"
#include "DATABASE_MANAGER.h"


using namespace std;

string ROLLNO, QNO;
sqlite3* db;
void update_db(){
	/**
		This method updates the database corresponding to the roll number
		with the question being marked solved. 
	**/
	update_val(ROLLNO, QNO, db);
}

int main(int argc, char const *argv[])
{
	db = open_database("STUDENT_DATABASE");
	create_table(db);
	initialize_database(db);

	while(1){
		int fd = open(INFO_PIPE,O_RDONLY);
		char buff[100];
		read(fd, buff, 100);
		string s(buff);
		cout<<"Message received in the db server "<<s<<endl;
		stringstream ss;
		ss<<s;
		ss>>ROLLNO>>QNO;
		update_db();
	}


	close_database(db);


	return 0;
}
