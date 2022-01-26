# NITW-Code-Judge

Check out the below file for details:
Server.cpp : https://github.com/pkc232/NITW-Code-Judge/blob/master/CODE/SERVER.cpp

The Judge works on a Server - Client architecture and has the following features:

1. User uploads the code file on the judge
2. The judge compiles the code file to generate a binary.
3. The judge returns any compilation errors if it occurs.
4. The generated binary is then executed in a sandbox environment against a set of input files.
5. We change the file descriptors to map the console I/O to file I/O.
6. The binary generates an output file which is checked against a pre-generated output file to return the number of test cases passed.




Server Architecture (Using a Super Server approach):
Its functionalities:
1. Every client first makes a connection with the super 
	server.
2.	The super server forks out a different server process to cater the needs of the client.




