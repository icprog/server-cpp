//
// Created by hujianzhe on 15/9/14.
//

#include "Util/libc/socket.h"
#include "Util/libc/statistics.h"
#include "Util/DBUtil.h"

#include <string>
#include <sstream>
#include <exception>
#include <stdexcept>
#include <iostream>

using std::string;
using std::stringstream;
using std::exception;
using std::runtime_error;
using std::cerr;
//用户行为循环
void userActivityLoop(FD_SOCKET sockfd,struct sockaddr* userAddr) {
    int res;
    try{
        DBUtil dbUtil(DB_TYPE_MYSQL);
        // 登陆校验
        // generate a new SessionId
        stringstream loginSessionId("");
        UUID uuid;
        res = uuid_Create(&uuid);
        if(res != EXEC_SUCCESS)
            throw runtime_error("generate a login session failure...");
        loginSessionId << uuid.id;
        // save SessionId and user IP

        // loop
        while(1){

        }
    }catch (const exception& e){
        cerr << "Exception: " <<  e.what() << std::endl;
    }
}
