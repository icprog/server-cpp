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
    map<string,string> SQLFields;
protected:
    string userId;
    string userName;
    string sessionId;
    string loginTime;
public:
    static const char* TABLE_NAME;
    map<string,string>& getSQLFields(void);

    User(void);
    ~User(void);

    const string& getUserId(void) const;
    void setUserId(const string& userId);

    const string& getUserName(void) const;
    void setUserName(const string& userName);

    const string& getSessionId(void) const;
    void setSessionId(const string& sessionId);

    const string& getLoginTime(void) const;
    void setLoginTime(const string& loginTime);
};

#endif //SERVER_USER_H
