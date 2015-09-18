//
// Created by hujianzhe on 15/9/18.
//

#ifndef SERVER_DBUTIL_H
#define SERVER_DBUTIL_H

#include "libc/db.h"
#include <map>
#include <string>

using std::string;
using std::map;

class DBUtil{
private:
    DBUtil(){}
protected:
    DB_CONNECT dbConnect;
public:
    explicit DBUtil(DB_TYPE type);
    ~DBUtil(void);
    template <typename OBJECT>
    void Insert(OBJECT* object);
};

#endif //SERVER_DBUTIL_H
