#include "server.h"

Server::Server(int port) {
    // setup variables
    port_ = port;
    buflen_ = 1024;
    buf_ = new char[buflen_+1];
}

Server::~Server() {
    delete buf_;
}

void
Server::run() {
    // create and run the server
    create();
    serve();
}

void
Server::create() {
    struct sockaddr_in server_addr;

    // setup socket address structure
    memset(&server_addr,0,sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port_);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    // create socket
    server_ = socket(PF_INET,SOCK_STREAM,0);
    if (!server_) {
        perror("socket");
        exit(-1);
    }

    // set socket to immediately reuse port when the application closes
    int reuse = 1;
    if (setsockopt(server_, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
        perror("setsockopt");
        exit(-1);
    }

    // call bind to associate the socket with our local address and
    // port
    if (bind(server_,(const struct sockaddr *)&server_addr,sizeof(server_addr)) < 0) {
        perror("bind");
        exit(-1);
    }

    // convert the socket to listen for incoming connections
    if (listen(server_,SOMAXCONN) < 0) {
        perror("listen");
        exit(-1);
    }
}

void
Server::close_socket() {
    close(server_);
}

void
Server::serve() {
    // setup client
    int client;
    struct sockaddr_in client_addr;
    socklen_t clientlen = sizeof(client_addr);

      // accept clients
    while ((client = accept(server_,(struct sockaddr *)&client_addr,&clientlen)) > 0) {

        handle(client);
    }
    close_socket();
}

void
Server::handle(int client) {
    // loop to handle all requests
    while (1) {
        // get a request
        string request = get_request(client);
        // break if client is done or an error occurred
        if (request.empty())
            break;
        // send response
        bool success = send_response(client,request);
        // break if an error occurred
        if (not success)
            break;
    }
    close(client);
}

string
Server::get_request(int client) {
    string request = "";
    // read until we get a newline
    while (request.find("\n") == string::npos) {
        int nread = recv(client,buf_,1024,0);
        if (nread < 0) {
            if (errno == EINTR)
                // the socket call was interrupted -- try again
                continue;
            else
                // an error occurred, so break out
                return "";
        } else if (nread == 0) {
            // the socket is closed
            return "";
        }
        // be sure to use append in case we have binary data
        request.append(buf_,nread);
    }
    // a better server would cut off anything after the newline and
    // save it in a cache

    int msgLen = lengthOfMsg(request);
    int headLen = lengthOfHeader(request);
    int dif = 0;

    if(msgLen > 0) {
      if(msgLen + headLen > 1024) {
	dif = (msgLen + headLen) - nread;

	while (dif > 0) {
	  nread = recv(client, buf_, 1024, 0);
	  if (nread < 0) {
	    if (errno == EINTR) {
	      continue;
	    }
	    else {
	      //error!
	      return "";
	    }
	  }
	   else if (nread == 0) {
	     return "";
           }
	   request.append(buf_, nread);
	   dif -= nread;
	}
      }
    }
    return request;
}

bool
Server::send_response(int client, string response) {
    // prepare to send response
    const char* ptr = response.c_str();
    int nleft = response.length();
    int nwritten;
    // loop to be sure it is all sent
    while (nleft) {
        if ((nwritten = send(client, ptr, nleft, 0)) < 0) {
            if (errno == EINTR) {
                // the socket call was interrupted -- try again
                continue;
            } else {
                // an error occurred, so break out
                perror("write");
                return false;
            }
        } else if (nwritten == 0) {
            // the socket is closed
            return false;
        }
        nleft -= nwritten;
        ptr += nwritten;
    }
    return true;
}

string Server::parseReq(string line) {
  int index = readToSent(' ', line);
  string cmd = line.substr(0, index);

  if (line.size() > index + 1) {
    line = line.substr(index + 1);
  }

  string temp = getCmd(cmd, line);
  return temp;
}

string Server::readToSent(char sent, string line) {
  string temp = "";
  int rtn = 0;
  for (int i = 0; i < line.size(); i++) {
    rtn = i;
    if (line.at(i) == sent) {
      break;
    }

    temp += line.at(i);
    
  }

  return rtn;
}

string Server::getCmd(string value, string val) {
  if (value == "put") {
    return put(val);
  } else if (value == "get") {
    return get(val);
  } else if (value == "list") {
    return list(val);
  } else if (value == "reset") {
    return reset();
  }

  return "invalid command\n"; 
}

string Server::put(string line) {
  int index = "";

  string usr = "";
  string sub = "";
  string msgLength = "";
  string error = "error, not enough parameters";
  int p = 3;
  for (int i = 0; i < p; i++) {
    if (i != p - 1) {
      index = readToSent(' ', line);
      if (i == 1) {
	usr = line.substr(0, index);
      } else {
	sub = line.substr(0, index);
      }
    } else {
      index = readToSent('\n', line);
      msgLength = line.substr(0, index);
    }
    
    if (line.size() > index + 1) {
      line = line.substr(index + 1);
    } else {

      return error;
    }

    string theMessage = line;

    message mess(usr, sub, theMessage, theMessage.size());
    string rtn = addToMap(mess);

    return rtn;
}

  string Server::get(string line) {
    int index = getCmd(' ', line);
    string usr = line.substr(0, index);
    string error = "error, not enough params\n";

    if(line.size() > index + 1) {
      line = line.substr(index + 1);
    } else {
      return error;
    }

    index = readToSent('\n', line);
    string numMsg = line.substr(0, index);
    int i = atoi(numMsg.c_str());
    error = "error, not a user\n";
    
    if (contains(usr)) {
      string rtn = getResponse(usr, i);
      return rtn;
    } else {
      return error;
    }
  }

  string Server::list(string line) {
    int index = readToSent('\n', line);
    string usr = line.substr(0, index);
    string error = "error, not a user\n";

    if (contains(usr)) {
      string rtn = listResponse(usr);
      return rtn;
    } else {
      return error;
    }
  }

  string Server::reset() {
    messageMap.clear();
    string rtn = "OK, cleared\n";
    return rtn;
  }

  string Server::listResponse(string userName) {
    map<string, vector<message>>::iterator iter;
    string rtn = "list \n";

    for(iter = messageMap.begin(); iter != messageMap.end(); iter++) {
      if (iter->first == userName) {
        rtn =+ intToString(iter->second.size()) + "\n";

	for (int i = 0; i < iter->second.size(); i++) {
	  rtn += intToString(i + 1) + " ";
	  rtn += it->second.at(i).getSubject() + "\n";
	}
	
      }
    }
    return rtn;
  }

  string Server::getResponse(stirng, int) {
    map<string, vector<message>>::iterator iter;
    string rtn = "";
    string error = "error, no messages there\n";

    for(iter = messageMap.begin(); iter != messageMap.end(); iter++) {
      if (iter->first == name) {
	if (iter->second.size() > index - 1) {
	  rtn = it->second.at(index - 1).toString();
	} else {
	  return error;
	}
	
      }
    }

    return rtn;
  }

  string Server::print() {
     map<string, vector<message>>::iterator iter;
     string rtn = intToString(messageMap.size()) + "\n";

     for(iter = messageMap.begin(); iter != messageMap.end(); iter++) {
      rtn += iter->first + "\n";
      rtn += "\t";

      for (int i = 0; i < it->second.size(); i++) {
	rtn += iter->second.at(i).toString();
      }
    }

    return rtn;
  }

  bool Server::contains(string userName) {
    map<string, vector<message>>::iterator iter;
    for (iter = messageMap.begin(); iter != messageMap.end(); iter++) {
      if (iter->first == userName) {
	return true;
      }
    }
    return false;
  }
