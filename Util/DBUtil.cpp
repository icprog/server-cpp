//
// Created by hujianzhe on 15/9/18.
//

#include "DBUtil.h"

//SQL_OBJECT
map<string,SQL_OBJECT::DBTableFieldsAttr>& SQL_OBJECT::getSQLTableFields(void){
    return SQLTableFields;
}
void SQL_OBJECT::setSQLFields(const char* key,DB_FIELD_TYPE type,void* buffer,size_t nbytes){
    SQL_OBJECT::DBTableFieldsAttr attr = {buffer,nbytes,false,type};
    SQLTableFields[key] = attr;
}
void SQL_OBJECT::setAlterFlag(const char* key){
    SQLTableFields[key].alterable = true;
}

//DBUtil
DBUtil::DBUtil(DB_TYPE type) {
    if(db_CreateConnectHandle(&this->dbHandle,type) == NULL)
        throw runtime_error("new DBUtil(db_CreateConnectHandle)...");
    if(db_SetupConnect(&this->dbHandle,"127.0.0.1",MYSQL_PORT,"root","123456","chat") == NULL)
        throw runtime_error("new DBUtil(db_SetupConnect)...");
    if(db_EnableAutoCommit(&this->dbHandle,false) == NULL)
        throw runtime_error("new DBUtil(db_EnableAutoCommit)...");
    if(db_AllocStmt(&this->dbHandle) == DB_RESULT_ERROR)
        throw runtime_error("new DBUtil(db_AllocStmt)...");
}

DBUtil::~DBUtil(void) {
    db_FreeResult(&this->dbHandle);
    db_CloseStmt(&this->dbHandle);
    db_CloseConnectHandle(&this->dbHandle);
}

DB_HANDLE* DBUtil::getHandle(void) const{
    return &this->dbHandle;
}

bool DBUtil::get(SQL_OBJECT* object,const char* key,void* value){
    map<string,SQL_OBJECT::DBTableFieldsAttr>& SQLTableFields = object->getSQLTableFields();
    //
    stringstream ss("");
    ss << "select * from " << object->getTableName() << " where " << key << " = ?";
    string sql = ss.str();
    ss.str("");
    //
    if(db_SQLPrepare(&this->dbHandle,sql.c_str(),sql.length()) == DB_RESULT_ERROR)
        throw runtime_error("DBUtil get(db_SQLPrepare)...");
    SQL_OBJECT::DBTableFieldsAttr& v = SQLTableFields[key];
    if(db_SQLBindParam(&this->dbHandle,0,v.type,value,v.nbytes) == DB_RESULT_ERROR)
        throw runtime_error("DBUtil get(db_SQLBindParam)...");
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
        map<string,SQL_OBJECT::DBTableFieldsAttr>::iterator kv = SQLTableFields.find(db_GetResultFieldName(&this->dbHandle,i));
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
    //
    if(db_FreeResult(&this->dbHandle) == DB_RESULT_ERROR)
        throw runtime_error("DBUtil get(db_FreeResult)...");
    return exist;
}

void DBUtil::insert(SQL_OBJECT* object){
    map<string,SQL_OBJECT::DBTableFieldsAttr>& SQLTableFields = object->getSQLTableFields();
    //
    string fields,args;
    for(map<string,SQL_OBJECT::DBTableFieldsAttr>::iterator iter = SQLTableFields.begin(); iter != SQLTableFields.end(); ++iter) {
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
    ss << "insert into " << object->getTableName() << " (" << fields << ") values (" << args << ")";
    string sql = ss.str();
    ss.str("");
    //
    if(db_SQLPrepare(&this->dbHandle,sql.c_str(),sql.length()) == DB_RESULT_ERROR)
        throw runtime_error("DBUtil insert(db_SQLPrepare)...");
    unsigned int fieldIndex = 0;
    for(map<string,SQL_OBJECT::DBTableFieldsAttr>::iterator iter = SQLTableFields.begin(); iter != SQLTableFields.end(); ++iter) {
        if(iter->second.alterable) {
            if (db_SQLBindParam(&this->dbHandle, fieldIndex++, iter->second.type, iter->second.buffer, iter->second.nbytes) == DB_RESULT_ERROR)
                throw runtime_error("DBUtil insert(db_SQLBindParam)...");
        }
    }
    if(db_SQLExecute(&this->dbHandle) == DB_RESULT_ERROR)
        throw runtime_error("DBUtil insert(db_SQLExecute)...");
}

void DBUtil::update(SQL_OBJECT* object,const char* key){
    map<string,SQL_OBJECT::DBTableFieldsAttr>& SQLTableFields = object->getSQLTableFields();
    //
    unsigned int fieldIndex = 0;
    stringstream ss("");
    ss << "update " << object->getTableName() << " set ";
    for(map<string,SQL_OBJECT::DBTableFieldsAttr>::iterator iter = SQLTableFields.begin(); iter != SQLTableFields.end(); ++iter){
        if(iter->second.alterable){
            if(fieldIndex++)
                ss << ",";
            ss << iter->first << " = ?";
        }
    }
    ss << " where " << key << " = ?";
    string sql = ss.str();
    ss.str("");
    //
    if(db_SQLPrepare(&this->dbHandle,sql.c_str(),sql.length()) == DB_RESULT_ERROR)
        throw runtime_error("DBUtil update(db_SQLPrepare)...");
    fieldIndex = 0;
    for(map<string,SQL_OBJECT::DBTableFieldsAttr>::iterator iter = SQLTableFields.begin(); iter != SQLTableFields.end(); ++iter){
        if(iter->second.alterable){
            if(db_SQLBindParam(&this->dbHandle,fieldIndex++,iter->second.type,iter->second.buffer,iter->second.nbytes) == DB_RESULT_ERROR)
                throw runtime_error("DBUtil update(db_SQLBindParam)...");
        }
    }
    SQL_OBJECT::DBTableFieldsAttr& keyAttr = SQLTableFields[key];
    if(db_SQLBindParam(&this->dbHandle,fieldIndex,keyAttr.type,keyAttr.buffer,keyAttr.nbytes) == DB_RESULT_ERROR)
        throw runtime_error("DBUtil update(db_SQLBindParam)...");
    if(db_SQLExecute(&this->dbHandle) == DB_RESULT_ERROR)
        throw runtime_error("DBUtil update(db_SQLExecute)...");
}

void DBUtil::remove(SQL_OBJECT* object,const char* key){
    //
    stringstream ss("");
    ss << "delete from " << object->getTableName() << " where " << key << " = ?";
    string sql = ss.str();
    ss.str("");
    //
    if(db_SQLPrepare(&this->dbHandle,sql.c_str(),sql.length()) == DB_RESULT_ERROR)
        throw runtime_error("DBUtil remove(db_SQLPrepare)...");
    SQL_OBJECT::DBTableFieldsAttr& keyAttr = object->getSQLTableFields()[key];
    if (db_SQLBindParam(&this->dbHandle, 0, keyAttr.type, keyAttr.buffer, keyAttr.nbytes) == DB_RESULT_ERROR)
        throw runtime_error("DBUtil remove(db_SQLBindParam)...");
    if(db_SQLExecute(&this->dbHandle) == DB_RESULT_ERROR)
        throw runtime_error("DBUtil remove(db_SQLExecute)...");
}