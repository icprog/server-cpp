//
// Created by hujianzhe on 15/9/14.
//

#include "User.h"
#include <exception>
using std::exception;

const char* User::TABLE_NAME = "user";

User::User(void) {
    this->SQLTableFields["user_id"] = pair<void*,size_t>(userId,sizeof(userId));
    this->SQLTableFields["username"] = pair<void*,size_t>(userName,sizeof(userName));
    this->SQLTableFields["session_id"] = pair<void*,size_t>(sessionId,sizeof(sessionId));
    this->SQLTableFields["login_time"] = pair<void*,size_t>(&loginTime,sizeof(loginTime));
    this->SQLTableFields["id"] = pair<void*,size_t>(&id,sizeof(id));
}
User::~User(void) { }