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

class User: public SQL_OBJECT{
protected:
    char userId[32];
    char password[32];
    char userName[64];
    char sessionId[32];
    DB_SQL_DATETIME_STRUCT loginTime;
    unsigned long long id;
public:
    const char* getTableName(void);
    User(void);
    ~User(void);
    const char* getUserId(void) const;
    void setUserId(const char* userId);

    const char* getPassword(void) const;
    void setPassword(const char* password);

    const char* getUserName(void) const;
    void setUserName(const char* userName);

    const char* getSessionId() const;
    void setSessionId(const char* sessionId);

    const DB_SQL_DATETIME_STRUCT& getLoginTime(void) const;
    void setLoginTime(const DB_SQL_DATETIME_STRUCT& loginTime);
};

#endif //SERVER_USER_H
