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

DB_HANDLE* db_CreateConnectHandle(DB_HANDLE* dbHandle,DB_TYPE type){
    dbHandle->type = type;
    switch(type){
        case DB_TYPE_MYSQL:{
#ifdef DB_ENABLE_MYSQL
            dbHandle->handle.mysql.hStmt = NULL;
            dbHandle->handle.mysql.stmt_res_fields = NULL;
            dbHandle->handle.mysql.stmt_res_bind = NULL;
            dbHandle->handle.mysql.stmt_param_bind = NULL;
            dbHandle->handle.mysql.stmt_res_has_bind = 0;
            if(mysql_init(&dbHandle->handle.mysql.mysql) == NULL)
                dbHandle = NULL;
#endif
            break;
        }
        default:
            dbHandle = NULL;
    }
    return dbHandle;
}

void db_CloseConnectHandle(DB_HANDLE* dbHandle){
    switch(dbHandle->type){
        case DB_TYPE_MYSQL:{
#ifdef DB_ENABLE_MYSQL
            mysql_close(&dbHandle->handle.mysql.mysql);
            mysql_thread_end();
#endif
            break;
        }
    }
}

DB_RETURN db_SetConnectTimeout(DB_HANDLE* dbHandle,unsigned int sec){
    DB_RETURN res = DB_RESULT_ERROR;
    switch(dbHandle->type){
        case DB_TYPE_MYSQL:{
#ifdef DB_ENABLE_MYSQL
            if(mysql_options(&dbHandle->handle.mysql.mysql,MYSQL_OPT_CONNECT_TIMEOUT,(const char *)&sec) == 0)
                res = DB_RESULT_SUCCESS;
#endif
            break;
        }
    }
    return res;
}

DB_HANDLE* db_SetupConnect(DB_HANDLE* dbHandle,const char* ip,unsigned short port,const char* user,const char* pwd,const char* database){
    switch(dbHandle->type){
        case DB_TYPE_MYSQL:{
#ifdef DB_ENABLE_MYSQL
            if(mysql_real_connect(&dbHandle->handle.mysql.mysql,ip,user,pwd,database,port,NULL,CLIENT_MULTI_STATEMENTS) == NULL) {
                dbHandle = NULL;
                break;
            }
            // mysql_query(env->hEnv,"set names utf8");
            mysql_set_character_set(&dbHandle->handle.mysql.mysql,"utf8");
#endif
            break;
        }
        default:
            dbHandle = NULL;
    }
    return dbHandle;
}

DB_RETURN db_TestConnectAlive(DB_HANDLE *dbHandle){
    DB_RETURN res = DB_RESULT_ERROR;
    switch(dbHandle->type){
        case DB_TYPE_MYSQL:{
#ifdef DB_ENABLE_MYSQL
            if(mysql_ping(&dbHandle->handle.mysql.mysql) == 0)
                res = DB_RESULT_SUCCESS;
#endif
            break;
        }
    }
    return res;
}

/* 事务 */
DB_HANDLE* db_EnableAutoCommit(DB_HANDLE* dbHandle,int bool_val){
    switch(dbHandle->type){
        case DB_TYPE_MYSQL:{
#ifdef DB_ENABLE_MYSQL
            if(mysql_autocommit(&dbHandle->handle.mysql.mysql,bool_val != 0))
                dbHandle = NULL;
#endif
            break;
        }
        default:
            dbHandle = NULL;
    }
    return dbHandle;
}

DB_HANDLE* db_Commit(DB_HANDLE* dbHandle){
    switch(dbHandle->type){
        case DB_TYPE_MYSQL:{
#ifdef DB_ENABLE_MYSQL
            if(mysql_commit(&dbHandle->handle.mysql.mysql))
                dbHandle = NULL;
#endif
            break;
        }
        default:
            dbHandle = NULL;
    }
    return dbHandle;
}

DB_HANDLE* db_Rollback(DB_HANDLE* dbHandle){
    switch(dbHandle->type){
        case DB_TYPE_MYSQL:{
#ifdef DB_ENABLE_MYSQL
            if(mysql_rollback(&dbHandle->handle.mysql.mysql))
                dbHandle = NULL;
#endif
            break;
        }
        default:
            dbHandle = NULL;
    }
    return dbHandle;
}

/* SQL执行 */
DB_RETURN db_AllocStmt(DB_HANDLE* dbHandle){
    DB_RETURN res = DB_RESULT_ERROR;
    switch(dbHandle->type){
        case DB_TYPE_MYSQL:{
#ifdef DB_ENABLE_MYSQL
            dbHandle->handle.mysql.hStmt = mysql_stmt_init(&dbHandle->handle.mysql.mysql);
            if(dbHandle->handle.mysql.hStmt)
                res = DB_RESULT_SUCCESS;
#endif
            break;
        }
    }
    return res;
}

DB_RETURN db_CloseStmt(DB_HANDLE* dbHandle){
    DB_RETURN res = DB_RESULT_ERROR;
    switch(dbHandle->type){
        case DB_TYPE_MYSQL:{
#ifdef DB_ENABLE_MYSQL
            if(mysql_stmt_close(dbHandle->handle.mysql.hStmt) == 0) {
                dbHandle->handle.mysql.hStmt = NULL;
                res = DB_RESULT_SUCCESS;
            }
#endif
            break;
        }
    }
    return res;
}

DB_RETURN db_SQLPrepare(DB_HANDLE* dbHandle,const char* sql,size_t length){
    DB_RETURN res = DB_RESULT_ERROR;
    switch(dbHandle->type){
        case DB_TYPE_MYSQL:{
#ifdef DB_ENABLE_MYSQL
            if (mysql_stmt_prepare(dbHandle->handle.mysql.hStmt, sql, length) == 0) {
                unsigned int paramCount = (unsigned int)mysql_stmt_param_count(dbHandle->handle.mysql.hStmt);
                if(paramCount){
                    dbHandle->handle.mysql.stmt_param_bind = (MYSQL_BIND*)malloc(sizeof(MYSQL_BIND) * paramCount);
                    if(dbHandle->handle.mysql.stmt_param_bind){
                        memset(dbHandle->handle.mysql.stmt_param_bind,0,sizeof(MYSQL_BIND) * paramCount);
                        res = DB_RESULT_SUCCESS;
                    }
                }else
                    res = DB_RESULT_SUCCESS;
            }
#endif
            break;
        }
    }
    return res;
}

DB_RETURN db_SQLParamCount(DB_HANDLE* dbHandle,unsigned int* count){
    DB_RETURN res = DB_RESULT_ERROR;
    switch(dbHandle->type) {
        case DB_TYPE_MYSQL: {
#ifdef DB_ENABLE_MYSQL
            *count = (unsigned int)mysql_stmt_param_count(dbHandle->handle.mysql.hStmt);
            res = DB_RESULT_SUCCESS;
#endif
            break;
        }
    }
    return res;
}

DB_RETURN db_SQLBindParam(DB_HANDLE *dbHandle,unsigned int paramIndex,int type,void* buffer,size_t nbytes){
    DB_RETURN res = DB_RESULT_ERROR;
    switch(dbHandle->type){
        case DB_TYPE_MYSQL:{
#ifdef DB_ENABLE_MYSQL
            MYSQL_BIND* bind = dbHandle->handle.mysql.stmt_param_bind + paramIndex;
            bind->buffer = buffer;
            bind->buffer_length = nbytes;
            bind->buffer_type = (enum enum_field_types)type;
            res = DB_RESULT_SUCCESS;
#endif
            break;
        }
    }
    return res;
}

DB_RETURN db_SQLExecute(DB_HANDLE* dbHandle){
    DB_RETURN res = DB_RESULT_ERROR;
    switch(dbHandle->type){
        case DB_TYPE_MYSQL:{
#ifdef DB_ENABLE_MYSQL
            if(dbHandle->handle.mysql.stmt_param_bind){
                if(mysql_stmt_bind_param(dbHandle->handle.mysql.hStmt,dbHandle->handle.mysql.stmt_param_bind))
                    break;
            }
            if(mysql_stmt_execute(dbHandle->handle.mysql.hStmt) == 0) {
                free(dbHandle->handle.mysql.stmt_param_bind);
                dbHandle->handle.mysql.stmt_param_bind = NULL;
                res = DB_RESULT_SUCCESS;
            }
#endif
            break;
        }
    }
    return res;
}

/* 结果集 */
DB_RETURN db_GetAutoIncrementValue(DB_HANDLE* dbHandle,unsigned long long* value){
    DB_RETURN res = DB_RESULT_ERROR;
    switch(dbHandle->type) {
        case DB_TYPE_MYSQL: {
#ifdef DB_ENABLE_MYSQL
            *value = mysql_stmt_insert_id(dbHandle->handle.mysql.hStmt);
            res = DB_RESULT_SUCCESS;
#endif
            break;
        }
    }
    return res;
}

DB_RETURN db_GetAffectedRows(DB_HANDLE* dbHandle,unsigned long long* rowCount){
    DB_RETURN res = DB_RESULT_ERROR;
    switch(dbHandle->type){
        case DB_TYPE_MYSQL:{
#ifdef DB_ENABLE_MYSQL
            my_ulonglong affectRows = mysql_stmt_affected_rows(dbHandle->handle.mysql.hStmt);
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

DB_RETURN db_LoadResultFieldMetaData(DB_HANDLE* dbHandle){
    DB_RETURN res = DB_RESULT_ERROR;
    switch(dbHandle->type){
        case DB_TYPE_MYSQL:{
#ifdef DB_ENABLE_MYSQL
            unsigned int fieldCount = mysql_stmt_field_count(dbHandle->handle.mysql.hStmt);
            if(fieldCount > 0) {/* produce a result set */
                dbHandle->handle.mysql.stmt_res_has_bind = 0;
                dbHandle->handle.mysql.stmt_res_bind = (MYSQL_BIND *) malloc(sizeof(MYSQL_BIND) * fieldCount);
                if (dbHandle->handle.mysql.stmt_res_bind == NULL)
                    break;
                memset(dbHandle->handle.mysql.stmt_res_bind,0,sizeof(MYSQL_BIND) * fieldCount);
                dbHandle->handle.mysql.stmt_res_fields = mysql_stmt_result_metadata(dbHandle->handle.mysql.hStmt);
                if (dbHandle->handle.mysql.stmt_res_fields)
                    res = DB_RESULT_SUCCESS;
            }
#endif
            break;
        }
    }
    return res;
}

DB_RETURN db_GetResultFieldCount(DB_HANDLE* dbHandle,unsigned int* count){
    DB_RETURN res = DB_RESULT_ERROR;
    switch(dbHandle->type){
        case DB_TYPE_MYSQL:{
#ifdef DB_ENABLE_MYSQL
            *count = mysql_stmt_field_count(dbHandle->handle.mysql.hStmt);
            res = DB_RESULT_SUCCESS;
#endif
            break;
        }
    }
    return res;
}

const char* db_GetResultFieldName(DB_HANDLE* dbHandle,unsigned int fieldIndex){
    const char* name = NULL;
    switch(dbHandle->type){
        case DB_TYPE_MYSQL:{
#ifdef DB_ENABLE_MYSQL
            name = mysql_fetch_field_direct(dbHandle->handle.mysql.stmt_res_fields,fieldIndex)->org_name;
#endif
            break;
        }
    }
    return name;
}

unsigned long db_GetResultFieldLength(DB_HANDLE* dbHandle,unsigned int fieldIndex){
    unsigned long length = 0;
    switch(dbHandle->type){
        case DB_TYPE_MYSQL:{
#ifdef DB_ENABLE_MYSQL
            length = mysql_fetch_field_direct(dbHandle->handle.mysql.stmt_res_fields,fieldIndex)->length;
#endif
            break;
        }
    }
    return length;
}

DB_RETURN db_BindResultFieldBuffer(DB_HANDLE* dbHandle,unsigned int fieldIndex,void* buffer,size_t nbytes,unsigned long* realbytes){
    DB_RETURN res = DB_RESULT_ERROR;
    switch(dbHandle->type){
        case DB_TYPE_MYSQL:{
#ifdef DB_ENABLE_MYSQL
            MYSQL_BIND* bind = dbHandle->handle.mysql.stmt_res_bind + fieldIndex;
            bind->buffer_type = mysql_fetch_field_direct(dbHandle->handle.mysql.stmt_res_fields,fieldIndex)->type;
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

DB_RETURN db_GetFirstResult(DB_HANDLE* dbHandle){
    DB_RETURN res = DB_RESULT_ERROR;
    switch(dbHandle->type){
        case DB_TYPE_MYSQL:{
#ifdef DB_ENABLE_MYSQL
            if(mysql_stmt_store_result(dbHandle->handle.mysql.hStmt) == 0)
                res = DB_RESULT_MORE_RESULT;
#endif
            break;
        }
    }
    return res;
}

DB_RETURN db_GetNextResult(DB_HANDLE* dbHandle){
    DB_RETURN res = DB_RESULT_ERROR;
    switch(dbHandle->type){
        case DB_TYPE_MYSQL:{
#ifdef DB_ENABLE_MYSQL
            dbHandle->handle.mysql.stmt_res_fields = NULL;
            dbHandle->handle.mysql.stmt_res_bind = NULL;
            dbHandle->handle.mysql.stmt_res_has_bind = 0;
            int ret = mysql_stmt_next_result(dbHandle->handle.mysql.hStmt);
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

DB_RETURN db_FreeResult(DB_HANDLE* dbHandle){
    DB_RETURN res = DB_RESULT_ERROR;
    switch(dbHandle->type) {
        case DB_TYPE_MYSQL: {
#ifdef DB_ENABLE_MYSQL
            if(dbHandle->handle.mysql.stmt_res_fields) {
                mysql_free_result(dbHandle->handle.mysql.stmt_res_fields);
                dbHandle->handle.mysql.stmt_res_fields = NULL;
            }
            if(dbHandle->handle.mysql.stmt_res_bind){
                free(dbHandle->handle.mysql.stmt_res_bind);
                dbHandle->handle.mysql.stmt_res_bind = NULL;
                dbHandle->handle.mysql.stmt_res_has_bind = 0;
            }
            if(mysql_stmt_free_result(dbHandle->handle.mysql.hStmt) == 0)
                res = DB_RESULT_SUCCESS;
#endif
            break;
        }
    }
    return res;
}

DB_RETURN db_FetchResult(DB_HANDLE* dbHandle){
    DB_RETURN res = DB_RESULT_ERROR;
    switch(dbHandle->type){
        case DB_TYPE_MYSQL:{
#ifdef DB_ENABLE_MYSQL
            if(dbHandle->handle.mysql.stmt_res_has_bind == 0) {
                if(mysql_stmt_bind_result(dbHandle->handle.mysql.hStmt,dbHandle->handle.mysql.stmt_res_bind) == 0)
                    dbHandle->handle.mysql.stmt_res_has_bind = 1;
                else
                    break;
            }
            int ret = mysql_stmt_fetch(dbHandle->handle.mysql.hStmt);
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