//
// Created by hujianzhe on 15/9/14.
//

#ifndef SERVER_USER_H
#define SERVER_USER_H

#include <string>
#include <map>
#include "../Util/libc/db.h"
#include "../Util/DBUtil.h"

using std::string;
using std::map;
using std::pair;

class User: public DBTable{
protected:
    char userId[32];
    char password[32];
    char userName[64];
    char sessionId[32];
    DB_SQL_DATETIME_STRUCT loginTime;
    unsigned long long id;
public:
    static const char* TABLE_NAME;
    User(void);
    ~User(void);
    const char* getUserId(void) const;
    const char* getUserName(void) const;
    const char* getPassword(void) const;
};

#endif //SERVER_USER_H
