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

#ifdef DB_ENABLE_MYSQL
    #include <mysql/mysql.h>
#else
#endif

typedef struct{
    DB_TYPE type;
#ifdef DB_ENABLE_MYSQL
    MYSQL Connect;
    MYSQL_STMT* hStmt;
    MYSQL_RES* stmt_res_fields;
    MYSQL_BIND* stmt_res_bind;
    my_bool stmt_res_has_bind;
#else
#endif
}DB_CONNECT;

#ifdef __cplusplus
extern "C"{
#endif

/* 句柄操作 */
DB_RETURN db_InitEnv(DB_TYPE type);
void db_CleanEnv(DB_TYPE type);
DB_CONNECT *db_CreateConnectHandle(DB_CONNECT *dbConnect, DB_TYPE type);
void db_CloseConnectHandle(DB_CONNECT *dbConnect);
DB_RETURN db_SetConnectTimeout(DB_CONNECT* dbConnect,unsigned int sec);
DB_CONNECT *db_SetupConnect(DB_CONNECT *dbConnect, const char *ip, unsigned short port, const char *user, const char *pwd, const char *database);
/* 事务 */
DB_CONNECT *db_EnableAutoCommit(DB_CONNECT *dbConnect, int bool_val);
DB_CONNECT *db_Commit(DB_CONNECT *dbConnect);
DB_CONNECT *db_Rollback(DB_CONNECT *dbConnect);
/* SQL执行 */
DB_RETURN db_AllocStmt(DB_CONNECT *dbConnect);
DB_RETURN db_CloseStmt(DB_CONNECT *dbConnect);
DB_RETURN db_SQLPrepare(DB_CONNECT *dbConnect, const char *sql, size_t length);
DB_RETURN db_SQLParamCount(DB_CONNECT *dbConnect, unsigned int *count);
DB_RETURN db_SQLExecute(DB_CONNECT *dbConnect);
/* 结果集 */
DB_RETURN db_GetAutoIncrementValue(DB_CONNECT *dbConnect, unsigned long long *value);
DB_RETURN db_GetAffectedRows(DB_CONNECT *dbConnect, unsigned long long *rowCount);
DB_RETURN db_LoadResultFieldMetaData(DB_CONNECT* dbConnect);
DB_RETURN db_GetResultFieldCount(DB_CONNECT *dbConnect, unsigned int *count);
const char* db_GetResultFieldName(DB_CONNECT* dbConnect,unsigned int fieldIndex);
unsigned long db_GetResultFieldLength(DB_CONNECT* dbConnect,unsigned int fieldIndex);
DB_RETURN db_BindResultFieldBuffer(DB_CONNECT* dbConnect,unsigned int fieldIndex,void* buffer,size_t nbytes,unsigned long* realbytes);
DB_RETURN db_GetFirstResult(DB_CONNECT* dbConnect);
DB_RETURN db_GetNextResult(DB_CONNECT* dbConnect);
DB_RETURN db_FreeResult(DB_CONNECT* dbConnect);
DB_RETURN db_FetchResult(DB_CONNECT* dbConnect);

#ifdef __cplusplus
}
#endif

#endif //SERVER_DB_H
