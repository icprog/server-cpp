//
// Created by hujianzhe on 15/9/15.
//

#ifndef SERVER_DB_H
#define SERVER_DB_H

#define DB_ENABLE_MYSQL
typedef enum{
    DB_TYPE_MYSQL,
}DB_TYPE;
typedef enum{
    DB_RESULT_ERROR,
    DB_RESULT_SUCCESS,
    DB_RESULT_MORE_RESULT,
    DB_RESULT_NO_RESULT,
    DB_RESULT_NO_DATA,
}DB_RETURN;
typedef enum{
    DB_FIELD_TYPE_TINY,
    DB_FIELD_TYPE_SMALLINT,
    DB_FIELD_TYPE_INT,
    DB_FIELD_TYPE_BIGINT,
    DB_FIELD_TYPE_FLOAT,
    DB_FIELD_TYPE_DOUBLE,
    DB_FIELD_TYPE_DATETIME,
    DB_FIELD_TYPE_TIMESTAMP,
    DB_FIELD_TYPE_CHAR,
    DB_FIELD_TYPE_BINARY,
    DB_FIELD_TYPE_VARCHAR,
    DB_FIELD_TYPE_VARBINARY,
}DB_FIELD_TYPE;

#ifdef DB_ENABLE_MYSQL
    #include <mysql/mysql.h>
#else
#endif

/* 句柄 */
typedef struct DB_HANDLE{
    DB_TYPE type;
    union {
        char reserved[0];
#ifdef DB_ENABLE_MYSQL
        struct {
            MYSQL mysql;
            MYSQL_STMT *hStmt;
            MYSQL_RES *stmt_res_fields;
            MYSQL_BIND *stmt_res_bind,*stmt_param_bind;
            my_bool stmt_res_has_bind;
        } mysql;
#else
#endif
    }handle;
}DB_HANDLE;
/* 类型 */
typedef struct DB_SQL_DATETIME_STRUCT{
    union {
        char reserved[0];
#ifdef DB_ENABLE_MYSQL
        MYSQL_TIME mysql_time;
#else
#endif
    }st;
    DB_TYPE type;
}DB_SQL_DATETIME_STRUCT;

#ifdef __cplusplus
extern "C"{
#endif

/* 句柄操作 */
DB_RETURN db_InitEnv(DB_TYPE type);
void db_CleanEnv(DB_TYPE type);
DB_HANDLE *db_CreateConnectHandle(DB_HANDLE *dbHandle, DB_TYPE type);
void db_CloseConnectHandle(DB_HANDLE *dbHandle);
DB_RETURN db_SetConnectTimeout(DB_HANDLE* dbHandle,unsigned int sec);
DB_HANDLE *db_SetupConnect(DB_HANDLE *dbHandle, const char *ip, unsigned short port, const char *user, const char *pwd, const char *database);
DB_RETURN db_TestConnectAlive(DB_HANDLE *dbHandle);
/* 事务 */
DB_HANDLE *db_EnableAutoCommit(DB_HANDLE *dbHandle, int bool_val);
DB_HANDLE *db_Commit(DB_HANDLE *dbHandle);
DB_HANDLE *db_Rollback(DB_HANDLE *dbHandle);
/* SQL执行 */
DB_RETURN db_AllocStmt(DB_HANDLE *dbHandle);
DB_RETURN db_CloseStmt(DB_HANDLE *dbHandle);
DB_RETURN db_SQLPrepare(DB_HANDLE *dbHandle, const char *sql, size_t length);
DB_RETURN db_SQLParamCount(DB_HANDLE *dbHandle, unsigned int *count);
DB_RETURN db_SQLBindParam(DB_HANDLE *dbHandle,unsigned int paramIndex,DB_FIELD_TYPE type,void* buffer,size_t nbytes);
DB_RETURN db_SQLExecute(DB_HANDLE *dbHandle);
/* 结果集 */
DB_RETURN db_GetAutoIncrementValue(DB_HANDLE *dbHandle, unsigned long long *value);
DB_RETURN db_GetAffectedRows(DB_HANDLE *dbHandle, unsigned long long *rowCount);
DB_RETURN db_LoadResultFieldMetaData(DB_HANDLE* dbHandle);
DB_RETURN db_GetResultFieldCount(DB_HANDLE *dbHandle, unsigned int *count);
const char* db_GetResultFieldName(DB_HANDLE* dbHandle,unsigned int fieldIndex);
unsigned long db_GetResultFieldLength(DB_HANDLE* dbHandle,unsigned int fieldIndex);
DB_RETURN db_BindResultFieldBuffer(DB_HANDLE* dbHandle,unsigned int fieldIndex,void* buffer,size_t nbytes,unsigned long* realbytes);
DB_RETURN db_GetFirstResult(DB_HANDLE* dbHandle);
DB_RETURN db_GetNextResult(DB_HANDLE* dbHandle);
DB_RETURN db_FreeResult(DB_HANDLE* dbHandle);
DB_RETURN db_FetchResult(DB_HANDLE* dbHandle);

#ifdef __cplusplus
}
#endif

#endif //SERVER_DB_H
