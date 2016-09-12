#include "message.h"

message::message() {
  user = "";
  subject = "";
  msg = "";
  len = 0;
}

message::message(string usr) {
  user = usr;
  subject = "";
  msg = "";
  len = 0;
}

message::message(string usr, string sub) {
  user = usr;
  subject = sub;
  msg = "";
  len = 0;
}

message::message(string usr, string sub, string mess) {
  user = usr;
  subject = sub;
  msg = mess;
  len = 0;
}

message::message(string usr, string sub, string mess, int leng) {
  user = usr;
  subject = sub;
  msg = mess;
  len = leng;  
}

message::~message() {
}

string message::getUser() {
  return user;
}

string message::getSubject() {
  return subject;
}

string message::getMessage() {
  return msg;
}

int message::getMessageLen() {
  return len;
}

