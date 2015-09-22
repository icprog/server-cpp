//
// Created by hujianzhe on 15/9/14.
//

#include "User.h"
#include <exception>
using std::exception;

const char* User::TABLE_NAME = "user";

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

const char* User::getUserName(void) const{
    return this->userName;
}

const char* User::getPassword(void) const{
    return this->password;
}