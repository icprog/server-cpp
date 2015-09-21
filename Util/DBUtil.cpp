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
bool DBUtil::get(OBJECT* object,const char* key){
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