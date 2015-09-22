//
// Created by hujianzhe on 15/9/18.
//

#include "DBUtil.h"
#include <sstream>
#include <stdexcept>

using std::stringstream;
using std::pair;
using std::runtime_error;

//DBUtil
DBUtil::DBUtil(DB_TYPE type) {
    if(db_CreateConnectHandle(&this->dbHandle,type) == NULL)
        throw runtime_error("new DBUtil(db_CreateConnectHandle)...");
    if(db_SetupConnect(&this->dbHandle,"127.0.0.1",MYSQL_PORT,"root","123456","chat") == NULL)
        throw runtime_error("new DBUtil(db_SetupConnect)...");
    if(db_EnableAutoCommit(&this->dbHandle,true) == NULL)
        throw runtime_error("new DBUtil(db_EnableAutoCommit)...");
    if(db_AllocStmt(&this->dbHandle) == DB_RESULT_ERROR)
        throw runtime_error("new DBUtil(db_AllocStmt)...");
}

DBUtil::~DBUtil(void) {
    db_FreeResult(&this->dbHandle);
    db_CloseStmt(&this->dbHandle);
    db_CloseConnectHandle(&this->dbHandle);
}

template <typename OBJECT>
bool DBUtil::get(OBJECT* object,const char* key,const char* value){
    map<string,DBTable::DBTableFieldsAttr>& SQLTableFields = object->getSQLTableFields();
    stringstream ss("");
    ss << "select * from " << OBJECT::TABLE_NAME << " where " << key << " = " << value;
    string sql = ss.str();
    ss.str("");
    if(db_SQLPrepare(&this->dbHandle,sql.c_str(),sql.length()) == DB_RESULT_ERROR)
        throw runtime_error("DBUtil get(db_SQLPrepare)...");
    if(db_SQLExecute(&this->dbHandle) == DB_RESULT_ERROR)
        throw runtime_error("DBUtil get(db_SQLExecute)...");
    if(db_GetFirstResult(&this->dbHandle) == DB_RESULT_ERROR)
        throw runtime_error("DBUtil get(db_GetFirstResult)...");
    if(db_LoadResultFieldMetaData(&this->dbHandle) == DB_RESULT_ERROR)
        throw runtime_error("DBUtil get(db_LoadResultFieldMetaData)...");
    unsigned int fieldsCount;
    if(db_GetResultFieldCount(&this->dbHandle,&fieldsCount) == DB_RESULT_ERROR)
        throw runtime_error("DBUtil get(db_GetResultFieldCount)...");
    for(unsigned int i = 0; i < fieldsCount; ++i){
        map<string,DBTable::DBTableFieldsAttr>::iterator kv = SQLTableFields.find(db_GetResultFieldName(&this->dbHandle,i));
        if(db_BindResultFieldBuffer(&this->dbHandle,i,kv->second.buffer,kv->second.nbytes,NULL) == DB_RESULT_ERROR)
            throw runtime_error("DBUtil get(db_BindResultFieldBuffer)...");
    }
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
void DBUtil::insert(OBJECT* object){
    map<string,DBTable::DBTableFieldsAttr>& SQLTableFields = object->getSQLTableFields();
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
    stringstream ss("");
    ss << "insert into " << OBJECT::TABLE_NAME << " (" << fields << ") values (" << args << ")";
    string sql = ss.str();
    ss.str("");
    if(db_SQLPrepare(&this->dbHandle,sql.c_str(),sql.length()) == DB_RESULT_ERROR)
        throw runtime_error("DBUtil insert(db_SQLPrepare)...");

    if(db_SQLExecute(&this->dbHandle) == DB_RESULT_ERROR)
        throw runtime_error("DBUtil insert(db_SQLExecute)...");
}

//DBTable
map<string,DBTable::DBTableFieldsAttr>& DBTable::getSQLTableFields(void){
    return SQLTableFields;
}
void DBTable::setSQLFields(const char* key,void* buffer,size_t nbytes){
    DBTable::DBTableFieldsAttr attr = {buffer,nbytes,false};
    SQLTableFields[key] = attr;
}