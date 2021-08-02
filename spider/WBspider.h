#ifndef SPIDER_H
#define SPIDER_H
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cstring>
#include <string>
#include <fcntl.h>
#include <sys/mman.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <regex>
#include <queue>
#include <list>
#include <fstream>
using namespace std;

typedef struct ssl_t
{
    SSL* sock_ssl;
    SSL_CTX*m_ctx;
}ssl_t;

typedef struct url_t
{
    string srcul;
    string domain;
    char ip[16];
    string path;
    string filenames;
    int port;
    int httptype; //0:http 1:https
}url_t;
using namespace std;
class WBSpider
{
public:
    WBSpider();
    int work();
    vector<string> vec_url;
    vector<string> vec_title;
private:

    char request[4096];
    int webfd;
    ssl_t *webssl;


private:
    int spider_nat_init();                          //网络初始化
    int spider_url_analytical(url_t*);              //解析模块
    int spider_connect(url_t*);                     //链接服务器
    int create_request(url_t*);                     //构建网络头
    int get_statecode(char *);                      //获取状态码
    ssl_t* create_ssl();
    int data_analytical(url_t*);
    int download(url_t *);                          //请求与下载

};

#endif // SPIDER_H
