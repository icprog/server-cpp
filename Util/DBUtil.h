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
using std::pair;

class DBUtil{
private:
    DBUtil(void){}
protected:
    DB_HANDLE dbHandle;
public:
    explicit DBUtil(DB_TYPE type);
    ~DBUtil(void);
    template <typename OBJECT>
    bool get(OBJECT* object,const char* key,const char* value);
    template <typename OBJECT>
    void insert(OBJECT* object);
};

class DBTable{
protected:
    map<string,pair<void*,size_t> > SQLTableFields;
    map<string,pair<void*,size_t> >& getTableFields(void);
};

#endif //SERVER_DBUTIL_H
