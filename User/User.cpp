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
    FIELDS[0] = userId;
    FIELDS[1] = userName;
    FIELDS[2] = sessionId;
    FIELDS[3] = NULL;
    FIELDS[4] = &id;
}
User::~User(void) { }