//
// Created by hujianzhe on 15/9/14.
//

#include "User.h"
#include <exception>
using std::exception;

const char* User::getTableName(void){return "user";}

User::User(void) {
    setSQLFields("user_id",DB_FIELD_TYPE_VARCHAR,userId,sizeof(userId));
    setSQLFields("password",DB_FIELD_TYPE_VARCHAR,password,sizeof(password));
    setSQLFields("username",DB_FIELD_TYPE_VARCHAR,userName,sizeof(userName));
    setSQLFields("session_id",DB_FIELD_TYPE_VARCHAR,sessionId,sizeof(sessionId));
    setSQLFields("login_time",DB_FIELD_TYPE_DATETIME,&loginTime,sizeof(loginTime));
    setSQLFields("id",DB_FIELD_TYPE_BIGINT,&id,sizeof(id));
}
User::~User(void) { }

const char* User::getUserId(void) const{
    return this->userId;
}
void User::setUserId(const char* userId){
    if(this->userId != userId){
        memset(this->userId,0,sizeof(this->userId));
        strcpy(this->userId,userId);
        setAlterFlag("user_id");
    }
}

const char* User::getPassword(void) const{
    return this->password;
}
void User::setPassword(const char* password){
    if(this->password != password){
        memset(this->password,0,sizeof(this->password));
        strcpy(this->password,password);
        setAlterFlag("password");
    }
}

const char* User::getUserName(void) const{
    return this->userName;
}
void User::setUserName(const char* userName){
    if(this->userName != userName){
        memset(this->userName,0,sizeof(this->userName));
        strcpy(this->userName,userName);
        setAlterFlag("username");
    }
}

const char* User::getSessionId() const{
    return this->sessionId;
}
void User::setSessionId(const char* sessionId){
    if(this->sessionId != sessionId){
        memset(this->sessionId,0,sizeof(this->sessionId));
        strcpy(this->sessionId,sessionId);
        setAlterFlag("session_id");
    }
}

const DB_SQL_DATETIME_STRUCT& User::getLoginTime(void) const{
    return this->loginTime;
}
void User::setLoginTime(const DB_SQL_DATETIME_STRUCT& loginTime){
    if(&this->loginTime != &loginTime){
        this->loginTime = loginTime;
        setAlterFlag("login_time");
    }
}