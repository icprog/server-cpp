//
// Created by hujianzhe on 15/9/14.
//

#ifndef SERVER_USER_H
#define SERVER_USER_H

#include <string>
#include <map>
#include "../Util/libc/db.h"

using std::string;
using std::map;
using std::pair;

class User {
private:
    map<string,pair<void*,size_t> > SQLTableFields;
protected:
    char userId[32];
    char userName[64];
    char sessionId[32];
    DB_SQL_DATETIME_STRUCT loginTime;
    unsigned long long id;
public:
    static const char* TABLE_NAME;
    map<string,pair<void*,size_t> >& getTableFields(void);

    User(void);
    ~User(void);
};

#endif //SERVER_USER_H
