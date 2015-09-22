//
// Created by hujianzhe on 15/9/18.
//

#include "DBUtil.h"

//DBTable
map<string,DBTable::DBTableFieldsAttr>& DBTable::getSQLTableFields(void){
    return SQLTableFields;
}
void DBTable::setSQLFields(const char* key,DB_FIELD_TYPE type,void* buffer,size_t nbytes){
    DBTable::DBTableFieldsAttr attr = {buffer,nbytes,false,type};
    SQLTableFields[key] = attr;
}

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