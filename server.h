#pragma once

#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#include <string>

using namespace std;

class Server {
public:
    Server(int port);
    ~Server();

    void run();
    
private:
    void create();
    void close_socket();
    void serve();
    void handle(int);
    string get_request(int);
    bool send_response(int, string);	
	string parseReq(string);
	int readToSent(char, string);
	string getCmd(string, string);
	string put(string);
	string get(string);
	string list(string);
	string reset();
	string listResponse(string);
	string getResponse(string, int);
	string print();
	string getReq(int);
	bool contains(string);
	int lengthOfMsg(string);
	int lengthOfHeader(string);
	bool sendResponse(int, string);

	//map of the message with the subject as keya
	map<string, vector<message> > messageMap;
        string addToMap(message);



    int port_;
    int server_;
    int buflen_;
    char* buf_;
};
