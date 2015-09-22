//
// Created by hujianzhe on 15/9/14.
//

#include "User.h"
#include <exception>
using std::exception;

const char* User::TABLE_NAME = "user";

User::User(void) {
    setSQLFields("user_id",userId,sizeof(userId));
    setSQLFields("password",password,sizeof(password));
    setSQLFields("username",userName,sizeof(userName));
    setSQLFields("session_id",sessionId,sizeof(sessionId));
    setSQLFields("login_time",&loginTime,sizeof(loginTime));
    setSQLFields("id",&id,sizeof(id),false);
}
User::~User(void) { }