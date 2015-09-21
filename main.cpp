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
int main(int argc,char** argv) {
    const int family = AF_INET;
    const unsigned short port = 50000;
    struct sockaddr_storage listenAddr;
    FD_SOCKET listenfd;
    // db start
    if(db_InitEnv(DB_TYPE_MYSQL) != DB_RESULT_SUCCESS)
        dumpFatalError("db_InitEnv");
    // socket init
    if(sock_SetupEnv() != EXEC_SUCCESS)
        dumpFatalError("sock_SetupEnv");
    if((listenfd = socket(family,SOCK_DGRAM,0)) == INVALID_SOCKET)
        dumpFatalError("socket");
    if(sock_Text2Addr((struct sockaddr*)&listenAddr,family,NULL,port) != EXEC_SUCCESS)
        dumpFatalError("sock_Text2Addr");
    if(sock_BindLocalAddr(listenfd,(struct sockaddr*)&listenAddr) != EXEC_SUCCESS)
        dumpFatalError("sock_BindLocalAddr");
    //accept user

    //cleanup
    if(sock_Close(listenfd) != EXEC_SUCCESS)
        dumpFatalError("sock_Shut");
    db_CleanEnv(DB_TYPE_MYSQL);
    if(sock_CleanEnv() != EXEC_SUCCESS)
        dumpFatalError("sock_CleanEnv");
    return 0;
}