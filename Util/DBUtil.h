//
// Created by hujianzhe on 15/9/18.
//

#ifndef SERVER_DBUTIL_H
#define SERVER_DBUTIL_H

#include "libc/db.h"
#include <map>
#include <string>
#include <sstream>
#include <stdexcept>

using std::string;
using std::stringstream;
using std::map;
using std::pair;
using std::runtime_error;

class DBTable{
public:
    typedef struct DBTableFieldsAttr{
        void* buffer;
        size_t nbytes;
        bool alterable;
        DB_FIELD_TYPE type;
    }DBTableFieldsAttr;
    map<string,DBTable::DBTableFieldsAttr>& getSQLTableFields(void);
protected:
    map<string,DBTable::DBTableFieldsAttr> SQLTableFields;
    void setSQLFields(const char* key,DB_FIELD_TYPE type,void* buffer,size_t length);
};

class DBUtil{
private:
    DBUtil(void){}
    DBUtil(const DBUtil&){}
    DBUtil& operator=(const DBUtil&){return *this;}
protected:
    DB_HANDLE dbHandle;
public:
    explicit DBUtil(DB_TYPE type);
    ~DBUtil(void);

    template <typename OBJECT>
    bool get(OBJECT* object,const char* key,const char* value){
        map<string,DBTable::DBTableFieldsAttr>& SQLTableFields = object->getSQLTableFields();
        //
        stringstream ss("");
        ss << "select * from " << OBJECT::TABLE_NAME << " where " << key << " = " << value;
        string sql = ss.str();
        ss.str("");
        //
        if(db_SQLPrepare(&this->dbHandle,sql.c_str(),sql.length()) == DB_RESULT_ERROR)
            throw runtime_error("DBUtil get(db_SQLPrepare)...");
        if(db_SQLExecute(&this->dbHandle) == DB_RESULT_ERROR)
            throw runtime_error("DBUtil get(db_SQLExecute)...");
        if(db_GetFirstResult(&this->dbHandle) == DB_RESULT_ERROR)
            throw runtime_error("DBUtil get(db_GetFirstResult)...");
        if(db_LoadResultFieldMetaData(&this->dbHandle) == DB_RESULT_ERROR)
            throw runtime_error("DBUtil get(db_LoadResultFieldMetaData)...");
        //
        unsigned int fieldsCount;
        if(db_GetResultFieldCount(&this->dbHandle,&fieldsCount) == DB_RESULT_ERROR)
            throw runtime_error("DBUtil get(db_GetResultFieldCount)...");
        for(unsigned int i = 0; i < fieldsCount; ++i){
            map<string,DBTable::DBTableFieldsAttr>::iterator kv = SQLTableFields.find(db_GetResultFieldName(&this->dbHandle,i));
            if(db_BindResultFieldBuffer(&this->dbHandle,i,kv->second.buffer,kv->second.nbytes,NULL) == DB_RESULT_ERROR)
                throw runtime_error("DBUtil get(db_BindResultFieldBuffer)...");
        }
        //
        bool exist = false;
        int res = db_FetchResult(&this->dbHandle);
        if(res == DB_RESULT_ERROR)
            throw runtime_error("DBUtil get(db_FetchResult)...");
        else if(res == DB_RESULT_SUCCESS)
            exist = true;
        if(db_FreeResult(&this->dbHandle) == DB_RESULT_ERROR)
            throw runtime_error("DBUtil get(db_FreeResult)...");
        return exist;
    }

    template <typename OBJECT>
    void insert(OBJECT* object){
        map<string,DBTable::DBTableFieldsAttr>& SQLTableFields = object->getSQLTableFields();
        //
        string fields,args;
        for(map<string,DBTable::DBTableFieldsAttr>::iterator iter = SQLTableFields.begin(); iter != SQLTableFields.end(); ++iter) {
            if(iter->second.alterable){
                if(!fields.empty())
                    fields += ",";
                fields += iter->first;
                if(!args.empty())
                    args += ",";
                args += "?";
            }
        }
        //
        stringstream ss("");
        ss << "insert into " << OBJECT::TABLE_NAME << " (" << fields << ") values (" << args << ")";
        string sql = ss.str();
        ss.str("");
        //
        if(db_SQLPrepare(&this->dbHandle,sql.c_str(),sql.length()) == DB_RESULT_ERROR)
            throw runtime_error("DBUtil insert(db_SQLPrepare)...");
        unsigned int fieldIndex = 0;
        for(map<string,DBTable::DBTableFieldsAttr>::iterator iter = SQLTableFields.begin(); iter != SQLTableFields.end(); ++iter) {
            if(iter->second.alterable) {
                if (db_SQLBindParam(&dbHandle, fieldIndex++, iter->second.type, iter->second.buffer, iter->second.nbytes) == DB_RESULT_ERROR)
                    throw ("DBUtil insert(db_SQLBindParam)...");
            }
        }
        if(db_SQLExecute(&this->dbHandle) == DB_RESULT_ERROR)
            throw runtime_error("DBUtil insert(db_SQLExecute)...");
    }
};

#endif //SERVER_DBUTIL_H
