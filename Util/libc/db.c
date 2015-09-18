//
// Created by hujianzhe on 15/9/15.
//

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "db.h"

#ifdef __cplusplus
extern "C"{
#endif

/* 句柄操作 */
DB_RETURN db_InitEnv(DB_TYPE type){
    DB_RETURN res = DB_RESULT_ERROR;
    switch(type){
        case DB_TYPE_MYSQL:{
#ifdef DB_ENABLE_MYSQL
            if(mysql_library_init(0,NULL,NULL) == 0){
                if(mysql_thread_safe() == 1)
                    res = DB_RESULT_SUCCESS;
                else
                    mysql_library_end();
            }
#endif
            break;
        }
    }
    return res;
}

void db_CleanEnv(DB_TYPE type){
    switch(type){
        case DB_TYPE_MYSQL:{
#ifdef DB_ENABLE_MYSQL
            mysql_library_end();
#endif
            break;
        }
    }
}

DB_CONNECT* db_CreateConnectHandle(DB_CONNECT* dbConnect,DB_TYPE type){
    dbConnect->type = type;
    switch(type){
        case DB_TYPE_MYSQL:{
#ifdef DB_ENABLE_MYSQL
            dbConnect->hStmt = NULL;
            dbConnect->stmt_res_fields = NULL;
            dbConnect->stmt_res_bind = NULL;
            dbConnect->stmt_res_has_bind = 0;
            if(mysql_init(&dbConnect->Connect) == NULL)
                dbConnect = NULL;
#endif
            break;
        }
        default:
            dbConnect = NULL;
    }
    return dbConnect;
}

void db_CloseConnectHandle(DB_CONNECT* dbConnect){
    switch(dbConnect->type){
        case DB_TYPE_MYSQL:{
#ifdef DB_ENABLE_MYSQL
            mysql_close(&dbConnect->Connect);
            mysql_thread_end();
#endif
            break;
        }
    }
}

DB_RETURN db_SetConnectTimeout(DB_CONNECT* dbConnect,unsigned int sec){
    DB_RETURN res = DB_RESULT_ERROR;
    switch(dbConnect->type){
        case DB_TYPE_MYSQL:{
#ifdef DB_ENABLE_MYSQL
            if(mysql_options(&dbConnect->Connect,MYSQL_OPT_CONNECT_TIMEOUT,(const char *)&sec) == 0)
                res = DB_RESULT_SUCCESS;
#endif
            break;
        }
    }
    return res;
}

DB_CONNECT* db_SetupConnect(DB_CONNECT* dbConnect,const char* ip,unsigned short port,const char* user,const char* pwd,const char* database){
    switch(dbConnect->type){
        case DB_TYPE_MYSQL:{
#ifdef DB_ENABLE_MYSQL
            if(mysql_real_connect(&dbConnect->Connect,ip,user,pwd,database,port,NULL,CLIENT_MULTI_STATEMENTS) == NULL) {
                dbConnect = NULL;
                break;
            }
            // mysql_query(env->hEnv,"set names utf8");
            mysql_set_character_set(&dbConnect->Connect,"utf8");
#endif
            break;
        }
        default:
            dbConnect = NULL;
    }
    return dbConnect;
}

/* 事务 */
DB_CONNECT* db_EnableAutoCommit(DB_CONNECT* dbConnect,int bool_val){
    switch(dbConnect->type){
        case DB_TYPE_MYSQL:{
#ifdef DB_ENABLE_MYSQL
            if(mysql_autocommit(&dbConnect->Connect,bool_val != 0))
                dbConnect = NULL;
#endif
            break;
        }
        default:
            dbConnect = NULL;
    }
    return dbConnect;
}

DB_CONNECT* db_Commit(DB_CONNECT* dbConnect){
    switch(dbConnect->type){
        case DB_TYPE_MYSQL:{
#ifdef DB_ENABLE_MYSQL
            if(mysql_commit(&dbConnect->Connect))
                dbConnect = NULL;
#endif
            break;
        }
        default:
            dbConnect = NULL;
    }
    return dbConnect;
}

DB_CONNECT* db_Rollback(DB_CONNECT* dbConnect){
    switch(dbConnect->type){
        case DB_TYPE_MYSQL:{
#ifdef DB_ENABLE_MYSQL
            if(mysql_rollback(&dbConnect->Connect))
                dbConnect = NULL;
#endif
            break;
        }
        default:
            dbConnect = NULL;
    }
    return dbConnect;
}

/* SQL执行 */
DB_RETURN db_AllocStmt(DB_CONNECT* dbConnect){
    DB_RETURN res = DB_RESULT_ERROR;
    switch(dbConnect->type){
        case DB_TYPE_MYSQL:{
#ifdef DB_ENABLE_MYSQL
            dbConnect->hStmt = mysql_stmt_init(&dbConnect->Connect);
            if(dbConnect->hStmt)
                res = DB_RESULT_SUCCESS;
#endif
            break;
        }
    }
    return res;
}

DB_RETURN db_CloseStmt(DB_CONNECT* dbConnect){
    DB_RETURN res = DB_RESULT_ERROR;
    switch(dbConnect->type){
        case DB_TYPE_MYSQL:{
#ifdef DB_ENABLE_MYSQL
            if(mysql_stmt_close(dbConnect->hStmt) == 0) {
                dbConnect->hStmt = NULL;
                res = DB_RESULT_SUCCESS;
            }
#endif
            break;
        }
    }
    return res;
}

DB_RETURN db_SQLPrepare(DB_CONNECT* dbConnect,const char* sql,size_t length){
    DB_RETURN res = DB_RESULT_ERROR;
    switch(dbConnect->type){
        case DB_TYPE_MYSQL:{
#ifdef DB_ENABLE_MYSQL
            if (mysql_stmt_prepare(dbConnect->hStmt, sql, length) == 0)
                res = DB_RESULT_SUCCESS;
#endif
            break;
        }
    }
    return res;
}

DB_RETURN db_SQLParamCount(DB_CONNECT* dbConnect,unsigned int* count){
    DB_RETURN res = DB_RESULT_ERROR;
    switch(dbConnect->type) {
        case DB_TYPE_MYSQL: {
#ifdef DB_ENABLE_MYSQL
            *count = (unsigned int)mysql_stmt_param_count(dbConnect->hStmt);
            res = DB_RESULT_SUCCESS;
#endif
            break;
        }
    }
    return res;
}

DB_RETURN db_SQLExecute(DB_CONNECT* dbConnect){
    DB_RETURN res = DB_RESULT_ERROR;
    switch(dbConnect->type){
        case DB_TYPE_MYSQL:{
#ifdef DB_ENABLE_MYSQL
            if(mysql_stmt_execute(dbConnect->hStmt) == 0)
                res = DB_RESULT_SUCCESS;
#endif
            break;
        }
    }
    return res;
}

/* 结果集 */
DB_RETURN db_GetAutoIncrementValue(DB_CONNECT* dbConnect,unsigned long long* value){
    DB_RETURN res = DB_RESULT_ERROR;
    switch(dbConnect->type) {
        case DB_TYPE_MYSQL: {
#ifdef DB_ENABLE_MYSQL
            *value = mysql_stmt_insert_id(dbConnect->hStmt);
            res = DB_RESULT_SUCCESS;
#endif
            break;
        }
    }
    return res;
}

DB_RETURN db_GetAffectedRows(DB_CONNECT* dbConnect,unsigned long long* rowCount){
    DB_RETURN res = DB_RESULT_ERROR;
    switch(dbConnect->type){
        case DB_TYPE_MYSQL:{
#ifdef DB_ENABLE_MYSQL
            my_ulonglong affectRows = mysql_stmt_affected_rows(dbConnect->hStmt);
            if(affectRows != (my_ulonglong)~0) {
                *rowCount = affectRows;
                res = DB_RESULT_SUCCESS;
            }
#endif
            break;
        }
    }
    return res;
}

DB_RETURN db_LoadResultFieldMetaData(DB_CONNECT* dbConnect){
    DB_RETURN res = DB_RESULT_ERROR;
    switch(dbConnect->type){
        case DB_TYPE_MYSQL:{
#ifdef DB_ENABLE_MYSQL
            unsigned int fieldCount = mysql_stmt_field_count(dbConnect->hStmt);
            if(fieldCount > 0) {/* produce a result set */
                dbConnect->stmt_res_has_bind = 0;
                dbConnect->stmt_res_bind = (MYSQL_BIND *) malloc(sizeof(MYSQL_BIND) * fieldCount);
                if (dbConnect->stmt_res_bind == NULL)
                    break;
                memset(dbConnect->stmt_res_bind,0,sizeof(MYSQL_BIND) * fieldCount);
                dbConnect->stmt_res_fields = mysql_stmt_result_metadata(dbConnect->hStmt);
                if (dbConnect->stmt_res_fields)
                    res = DB_RESULT_SUCCESS;
            }
#endif
            break;
        }
    }
    return res;
}

DB_RETURN db_GetResultFieldCount(DB_CONNECT* dbConnect,unsigned int* count){
    DB_RETURN res = DB_RESULT_ERROR;
    switch(dbConnect->type){
        case DB_TYPE_MYSQL:{
#ifdef DB_ENABLE_MYSQL
            *count = mysql_stmt_field_count(dbConnect->hStmt);
            res = DB_RESULT_SUCCESS;
#endif
            break;
        }
    }
    return res;
}

const char* db_GetResultFieldName(DB_CONNECT* dbConnect,unsigned int fieldIndex){
    const char* name = NULL;
    switch(dbConnect->type){
        case DB_TYPE_MYSQL:{
#ifdef DB_ENABLE_MYSQL
            name = mysql_fetch_field_direct(dbConnect->stmt_res_fields,fieldIndex)->org_name;
#endif
            break;
        }
    }
    return name;
}

unsigned long db_GetResultFieldLength(DB_CONNECT* dbConnect,unsigned int fieldIndex){
    unsigned long length = 0;
    switch(dbConnect->type){
        case DB_TYPE_MYSQL:{
#ifdef DB_ENABLE_MYSQL
            length = mysql_fetch_field_direct(dbConnect->stmt_res_fields,fieldIndex)->length;
#endif
            break;
        }
    }
    return length;
}

DB_RETURN db_BindResultFieldBuffer(DB_CONNECT* dbConnect,unsigned int fieldIndex,void* buffer,size_t nbytes,unsigned long* realbytes){
    DB_RETURN res = DB_RESULT_ERROR;
    switch(dbConnect->type){
        case DB_TYPE_MYSQL:{
#ifdef DB_ENABLE_MYSQL
            MYSQL_BIND* bind = dbConnect->stmt_res_bind + fieldIndex;
            bind->buffer_type = mysql_fetch_field_direct(dbConnect->stmt_res_fields,fieldIndex)->type;
            bind->buffer = buffer;
            bind->buffer_length = nbytes;
            bind->length = realbytes;
            res = DB_RESULT_SUCCESS;
#endif
            break;
        }
    }
    return res;
}

DB_RETURN db_GetFirstResult(DB_CONNECT* dbConnect){
    DB_RETURN res = DB_RESULT_ERROR;
    switch(dbConnect->type){
        case DB_TYPE_MYSQL:{
#ifdef DB_ENABLE_MYSQL
            if(mysql_stmt_store_result(dbConnect->hStmt) == 0)
                res = DB_RESULT_MORE_RESULT;
#endif
            break;
        }
    }
    return res;
}

DB_RETURN db_GetNextResult(DB_CONNECT* dbConnect){
    DB_RETURN res = DB_RESULT_ERROR;
    switch(dbConnect->type){
        case DB_TYPE_MYSQL:{
#ifdef DB_ENABLE_MYSQL
            dbConnect->stmt_res_fields = NULL;
            dbConnect->stmt_res_bind = NULL;
            dbConnect->stmt_res_has_bind = 0;
            int ret = mysql_stmt_next_result(dbConnect->hStmt);
            if(ret == 0)
                res = DB_RESULT_MORE_RESULT;
            else if(ret == -1)
                res = DB_RESULT_NO_RESULT;
#endif
            break;
        }
    }
    return res;
}

DB_RETURN db_FreeResult(DB_CONNECT* dbConnect){
    DB_RETURN res = DB_RESULT_ERROR;
    switch(dbConnect->type) {
        case DB_TYPE_MYSQL: {
#ifdef DB_ENABLE_MYSQL
            if(dbConnect->stmt_res_fields) {
                mysql_free_result(dbConnect->stmt_res_fields);
                dbConnect->stmt_res_fields = NULL;
            }
            if(dbConnect->stmt_res_bind){
                free(dbConnect->stmt_res_bind);
                dbConnect->stmt_res_bind = NULL;
                dbConnect->stmt_res_has_bind = 0;
            }
            if(mysql_stmt_free_result(dbConnect->hStmt) == 0)
                res = DB_RESULT_SUCCESS;
#endif
            break;
        }
    }
    return res;
}

DB_RETURN db_FetchResult(DB_CONNECT* dbConnect){
    DB_RETURN res = DB_RESULT_ERROR;
    switch(dbConnect->type){
        case DB_TYPE_MYSQL:{
#ifdef DB_ENABLE_MYSQL
            if(dbConnect->stmt_res_has_bind == 0) {
                if(mysql_stmt_bind_result(dbConnect->hStmt,dbConnect->stmt_res_bind) == 0)
                    dbConnect->stmt_res_has_bind = 1;
                else
                    break;
            }
            int ret = mysql_stmt_fetch(dbConnect->hStmt);
            if(ret == 0 || ret == MYSQL_DATA_TRUNCATED)
                res = DB_RESULT_SUCCESS;
            else if(ret == MYSQL_NO_DATA)
                res = DB_RESULT_NO_DATA;
#endif
            break;
        }
    }
    return res;
}

#ifdef __cplusplus
}
#endif