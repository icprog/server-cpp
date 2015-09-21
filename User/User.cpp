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

User::User(void) {
    FIELDS_BUFFER[0] = userId;
    FIELDS_BUFFER[1] = userName;
    FIELDS_BUFFER[2] = sessionId;
    FIELDS_BUFFER[3] = &loginTime;
    FIELDS_BUFFER[4] = &id;

    FIELDS_LENGTH[0] = sizeof(userId);
    FIELDS_LENGTH[1] = sizeof(userName);
    FIELDS_LENGTH[2] = sizeof(sessionId);
    FIELDS_LENGTH[3] = sizeof(loginTime);
    FIELDS_LENGTH[4] = sizeof(id);
}
User::~User(void) { }