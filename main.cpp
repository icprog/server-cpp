#include "Util/libc/code.h"
#include "Util/libc/socket.h"
#include "Util/libc/process.h"
#include "Util/libc/db.h"

// 输出错误日志
void dumpFatalError(const char* API){
    sock_CleanEnv();
    fprintf(stderr,"fatal error: %s(%d)",API,GetSocketError());
    exit(EXIT_FAILURE);
}
void dumpNormalError(const char* desc){
    fprintf(stderr,"failure: %s\n",desc);
}
//void db_test(void){
//    DB_RETURN res;
//    DB_CONNECT hdb;
//    db_CreateConnectHandle(&hdb,DB_TYPE_MYSQL);
//    db_SetupConnect(&hdb,"127.0.0.1",MYSQL_PORT,"root","123456","chat");
//    db_AllocStmt(&hdb);
//    const char* sql = "call abc(5)";
//    db_SQLPrepare(&hdb,sql,strlen(sql));
//    db_SQLExecute(&hdb);
//    for(res = db_GetFirstResult(&hdb); res == DB_RESULT_MORE_RESULT; res = db_GetNextResult(&hdb)) {
//        if(db_LoadResultFieldMetaData(&hdb) != DB_RESULT_SUCCESS)
//            continue;
//        char buffer[64] = {0};
//        db_BindResultFieldBuffer(&hdb,0,buffer,sizeof(buffer),NULL);
//        while(db_FetchResult(&hdb) == DB_RESULT_SUCCESS){
//            puts(buffer);
//        }
//        db_FreeResult(&hdb);
//    }
//    if(res == DB_RESULT_NO_RESULT) {
//        puts("scan all result...");
//    }else if(res == DB_RESULT_ERROR)
//        puts(mysql_stmt_error(hdb.hStmt));
//    db_CloseStmt(&hdb);
//    db_CloseConnectHandle(&hdb);
//    exit(0);
//}
int main(int argc,char** argv) {
    int res;
    const int family = AF_INET;
    const unsigned short port = 50000;
    struct sockaddr_storage listenAddr,userAddr;
    FD_SOCKET listenfd;
    DB_RETURN dbres;
    // db start
    dbres = db_InitEnv(DB_TYPE_MYSQL);
    if(dbres != DB_RESULT_SUCCESS)
        dumpFatalError("db_InitEnv");
    // socket init
    res = sock_SetupEnv();
    if(res != EXEC_SUCCESS)
        dumpFatalError("sock_SetupEnv");
    listenfd = socket(family,SOCK_DGRAM,0);
    if(listenfd == INVALID_SOCKET)
        dumpFatalError("socket");
    res = sock_Text2Addr((struct sockaddr*)&listenAddr,family,NULL,port);
    if(res != EXEC_SUCCESS)
        dumpFatalError("sock_Text2Addr");
    res = sock_BindLocalAddr(listenfd,(struct sockaddr*)&listenAddr);
    if(res != EXEC_SUCCESS)
        dumpFatalError("sock_BindLocalAddr");
    //accept user

    //clean
    db_CleanEnv(DB_TYPE_MYSQL);
    res = sock_CleanEnv();
    if(res != EXEC_SUCCESS)
        dumpFatalError("sock_CleanEnv");
    return 0;
}