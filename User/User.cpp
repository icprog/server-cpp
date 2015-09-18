//
// Created by hujianzhe on 15/9/14.
//

#include "User.h"
#include <exception>
using std::exception;

const char* User::TABLE_NAME = "user";
map<string,string>& User::getSQLFields(void){
    return this->SQLFields;
}

User::User(void) { }
User::~User(void) { }

const string& User::getUserId(void) const{
    return this->userId;
}
void User::setUserId(const string& userId){
    this->userId = userId;
    this->SQLFields["user_id"] = "'" + userId + "'";
}

const string& User::getUserName(void) const{
    return this->userName;
}
void User::setUserName(const string& userName){
    this->userName = userName;
    this->SQLFields["username"] = "'" + userName + "'";
}

const string& User::getSessionId(void) const{
    return this->sessionId;
}
void User::setSessionId(const string& sessionId){
    this->sessionId = sessionId;
    this->SQLFields["session_id"] = "'" + sessionId + "'";
}

const string& User::getLoginTime(void) const{
    return this->loginTime;
}
void User::setLoginTime(const string& loginTime){
    this->loginTime = loginTime;
    this->SQLFields["login_time"] = "'" + loginTime + "'";
}