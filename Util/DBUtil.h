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

class SQL_OBJECT{
public:
    typedef struct DBTableFieldsAttr{
        void* buffer;
        size_t nbytes;
        bool alterable;
        DB_FIELD_TYPE type;
    }DBTableFieldsAttr;
    virtual const char* getTableName(void) = 0;
    map<string,SQL_OBJECT::DBTableFieldsAttr>& getSQLTableFields(void);
protected:
    map<string,SQL_OBJECT::DBTableFieldsAttr> SQLTableFields;
    void setSQLFields(const char* key,DB_FIELD_TYPE type,void* buffer,size_t length);
    void setAlterFlag(const char* key);
};

class DBUtil{
private:
    DBUtil(void){}
    DBUtil(const DBUtil&){}
    DBUtil& operator=(const DBUtil&){return *this;}
protected:
    mutable DB_HANDLE dbHandle;
public:
    explicit DBUtil(DB_TYPE type);
    ~DBUtil(void);
    DB_HANDLE* getHandle(void) const;
    void commit(void) const;
    void rollback(void) const;

    bool get(SQL_OBJECT* object,const char* key,void* value);
    void insert(SQL_OBJECT* object);
    void update(SQL_OBJECT* object,const char* key);
    void remove(SQL_OBJECT* object,const char* key);
};

#endif //SERVER_DBUTIL_H
