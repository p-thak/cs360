#pragma once
#include <iostream>
#include <string>
#include <sstream>

using namespace std;

class message {
 public:
  message();
  message(string);
  message(string, string);
  message(string, string, string);
  message(string, string, string, int);
  ~message();

  string getUser();
  string getSubject();
  string getMessage();
  int getMessageLen();


 private:
  string user;
  string subject;
  string msg;
  int len;



};
