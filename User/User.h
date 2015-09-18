//
// Created by hujianzhe on 15/9/14.
//

#ifndef SERVER_USER_H
#define SERVER_USER_H

#include <string>
#include <map>

using std::string;
using std::map;

class User {
private:
    void* FIELDS[5];
    map<string,string> SQLFields;
protected:
    char userId[32];
    char userName[64];
    char sessionId[32];
    unsigned long long id;
public:
    static const char* TABLE_NAME;
    map<string,string>& getSQLFields(void);

    User(void);
    ~User(void);
};

#endif //SERVER_USER_H
