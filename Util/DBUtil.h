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
    DB_HANDLE dbHandle;
public:
    explicit DBUtil(DB_TYPE type);
    ~DBUtil(void);
    template <typename OBJECT>
    bool get(OBJECT* object,const char* key);
    template <typename OBJECT>
    void insert(OBJECT* object);
};

#endif //SERVER_DBUTIL_H
