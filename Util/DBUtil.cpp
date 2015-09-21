//
// Created by hujianzhe on 15/9/18.
//

#include "DBUtil.h"
#include <sstream>
#include <stdexcept>
using std::stringstream;
using std::runtime_error;

DBUtil::DBUtil(DB_TYPE type) {
    if(db_CreateConnectHandle(&this->dbHandle,type) == NULL)
        throw runtime_error("new DBUtil(db_CreateConnectHandle)...");
    if(db_SetupConnect(&this->dbHandle,"127.0.0.1",MYSQL_PORT,"root","123456","chat") == NULL)
        throw runtime_error("new DBUtil(db_SetupConnect)...");
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
    stringstream ss("");
    ss << "select * from " << OBJECT::TABLE_NAME << " where " << key << " = " << value;
    string sql = ss.str();
    ss.str("");
    if(db_SQLPrepare(&this->dbHandle,sql.c_str(),sql.length()) == DB_RESULT_ERROR)
        throw runtime_error("DBUtil get(db_SQLPrepare)...");
    if(db_SQLExecute(&this->dbHandle) == DB_RESULT_ERROR)
        throw runtime_error("DBUtil get(db_SQLExecute)...");
    if(db_GetFirstResult(&this->dbHandle) == DB_RESULT_ERROR)
        throw new runtime_error("DBUtil get(db_GetFirstResult)...");
    if(db_LoadResultFieldMetaData(&this->dbHandle) == DB_RESULT_ERROR)
        throw  runtime_error("DBUtil get(db_LoadResultFieldMetaData)...");
    unsigned int fieldCount;
    if(db_GetResultFieldCount(&this->dbHandle,&fieldCount) == DB_RESULT_ERROR)
        throw new runtime_error("DBUtil get(db_GetResultFieldCount)...");
    for(unsigned int i = 0; i < fieldCount; ++i) {
        if(db_BindResultFieldBuffer(&this->dbHandle, i, object->FIELDS_BUFFER, object->FIELDS_LENGTH, NULL) == DB_RESULT_ERROR)
            throw new runtime_error("DBUtil get(db_BindResultFieldBuffer)...");
    }
    int res = db_FetchResult(&this->dbHandle);
    if(res == DB_RESULT_ERROR)
        throw new runtime_error("DBUtil get(db_FetchResult)...");
    if(res == DB_RESULT_SUCCESS)
        return true;
    return false;
}

template <typename OBJECT>
void DBUtil::insert(OBJECT* object){
    string fields,values;
    map<string,string>& objectFields = object->getSQLFields();
    for(map<string,string>::iterator iter = objectFields.begin(); iter != objectFields.end(); ++iter){
        if(!fields.empty())
            fields += ",";
        fields += iter->first;
        if(!values.empty())
            values += ",";
        values += iter->second;
    }
    stringstream ss("");
    ss << "insert into " << OBJECT::TABLE_NAME << " (" << fields << ") " << "values (" << values << ")";
    string sql = ss.str();
    ss.str("");
    if(db_SQLPrepare(&this->dbHandle,sql.c_str(),sql.length()) == DB_RESULT_ERROR)
        throw runtime_error("DBUtil insert(db_SQLPrepare)...");
    if(db_SQLExecute(&this->dbHandle) == DB_RESULT_ERROR)
        throw runtime_error("DBUtil insert(db_SQLExecute)...");
}