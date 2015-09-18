//
// Created by hujianzhe on 15/9/18.
//

#include "DBUtil.h"
#include <sstream>
#include <stdexcept>
using std::stringstream;
using std::runtime_error;

DBUtil::DBUtil(DB_TYPE type) {
    if(db_CreateConnectHandle(&this->dbConnect,type) == NULL)
        throw runtime_error("new DBUtil(db_CreateConnectHandle)...");
    if(db_SetupConnect(&this->dbConnect,"127.0.0.1",MYSQL_PORT,"root","123456","chat") == NULL)
        throw runtime_error("new DBUtil(db_SetupConnect)...");
    if(db_AllocStmt(&this->dbConnect) == DB_RESULT_ERROR)
        throw runtime_error("new DBUtil(db_AllocStmt)...");
}
DBUtil::~DBUtil(void) {
    db_FreeResult(&this->dbConnect);
    db_CloseStmt(&this->dbConnect);
    db_CloseConnectHandle(&this->dbConnect);
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
    if(db_SQLPrepare(&this->dbConnect,sql.c_str(),sql.length()) == DB_RESULT_ERROR)
        throw runtime_error("DBUtil insert(db_SQLPrepare)...");
    if(db_SQLExecute(&this->dbConnect) == DB_RESULT_ERROR)
        throw runtime_error("DBUtil insert(db_SQLExecute)...");
}