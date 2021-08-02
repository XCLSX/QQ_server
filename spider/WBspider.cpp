#include "WBspider.h"



WBSpider::WBSpider()
{
    webssl = NULL;

}



int WBSpider::spider_nat_init()
{
    struct sockaddr_in myaddr;
    memset(&myaddr,0,sizeof(myaddr));
    myaddr.sin_family = AF_INET;
    myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    myaddr.sin_port = htons(8001);
    if((webfd = socket(AF_INET,SOCK_STREAM,0)) == -1)
    {
        printf("spider_net_init failed\n");
        return -1;
    }
    if(bind(webfd,(struct sockaddr*)&myaddr,sizeof(myaddr)) == -1)
    {
        printf("spider_net_bind failed\n");
        return -2;
    }

    printf("[1] Net Init success\n");
    return 1;
}

int WBSpider::spider_url_analytical(url_t *url)
{
    //获取http类型
    char *arr[] = {"http://","https://",NULL};
    int start = 0;
    //获得端口号
    if(strncmp(url->srcul.c_str(),arr[0],strlen(arr[0])) == 0)
    {
        url->httptype = 0;
        url->port = 80;
        start = strlen(arr[0]);
    }
    else
    {
        url->httptype = 1;
        url->port = 443;
        start = strlen(arr[1]);
    }
    //获得域名
    for(int i = start;url->srcul[i]!='/'&&url->srcul[i]!='\0';i++)
        url->domain.push_back(url->srcul[i]);

    //获取文件名
    int nameSize = 0;
    for(int i=url->srcul.length();url->srcul[i]!='/';i--)
        nameSize++;
    for(int i = url->srcul.length() - nameSize + 1;url->srcul[i]!='\0';i++)
        url->filenames.push_back(url->srcul[i]);

    //获取路径
    for(int i = start + url->domain.length();i<url->srcul.length() - nameSize +1;i++)
        url->path.push_back(url->srcul[i]);

    //通过DNS获取ip
    struct  hostent *ent;
    ent = gethostbyname(url->domain.c_str());
    if(!ent)
    {
        printf("get ip failed\n");
        return -1;
    }
    memset(url->ip,0,sizeof(url->ip));
    inet_ntop(AF_INET,ent->h_addr_list[0],url->ip,sizeof(url->ip));
   cout<<"[2] 解析成功"<<endl;
}

int WBSpider::spider_connect(url_t* url)
{
    struct sockaddr_in webaddr;
    memset(&webaddr,0,sizeof(webaddr));
    webaddr.sin_family = AF_INET;
    webaddr.sin_port = htons(url->port);
    inet_pton(AF_INET,url->ip,&webaddr.sin_addr.s_addr);
    if(connect(webfd,(struct sockaddr*)&webaddr,sizeof(webaddr)) == -1)
    {
        printf("connect failed\n");
        return -1;
    }
    printf("[3] connect success\n");
    return 1;

}

int WBSpider::create_request(url_t*url)
{

    sprintf(request,"GET %s HTTP/1.1\r\n"\
                 "Accept:*/*\r\n"\
                 "User-Agent:Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/74.0.3729.131 Safari/537.36\r\n"\
                 "Host:%s\r\n"\
                 "Connection:close\r\n\r\n",url->srcul.c_str(),url->domain.c_str());
    printf("%s",request);

    if(url->httptype == 1)
    {
        webssl = create_ssl();
        printf("[4] Create HTTPS Request success\n");
        return 2;
    }
    printf("[4] Create HTTP Request success\n");

    return 1;

}

int WBSpider::get_statecode(char *str)
{
    regex reg(" \\d* ");
    string szbuf = string(str);
    smatch result;
    if(!regex_search(szbuf,result,reg))
    {
        return 0;
    }

    int num = atoi(result.str().c_str());
    return num;
}

ssl_t* WBSpider::create_ssl()
{
    ssl_t *ssl = new ssl_t;
    //初始化openssl库
    SSL_library_init();
    //加载openssl错误处理
    SSL_load_error_strings();
    //加载加密功能和散列函数
    OPENSSL_add_all_algorithms_conf();
    //创建CTX
    ssl->m_ctx = SSL_CTX_new(SSLv23_method());
    //创建ssl安全套接字
    ssl->sock_ssl = SSL_new(ssl->m_ctx);
    //通过socket设置ssl
    SSL_set_fd(ssl->sock_ssl,webfd);
    //SSl认证链接
    SSL_connect(ssl->sock_ssl);
    return ssl;
}

int WBSpider::work(char *u)
{
       url_t *url = new url_t;
       memset(request,0,sizeof (request));
       url->filenames+="/home/hush/qtProject/";
       url->srcul = u;
       que_url.push(url);

       while(que_url.size()>0)
       {
           url_t* temp = que_url.front();
           que_url.pop();
           spider_nat_init();
           spider_url_analytical(url);
           spider_connect(temp);
           create_request(temp);
           download(temp);
           data_analytical(temp);
       }

}

int WBSpider::data_analytical(url_t *u)
{
    std::ifstream t;
    t.open(u->filenames);
    t.seekg(0, std::ios::end);
    int bufferLen = t.tellg();
    t.seekg(0, std::ios::beg);
    char *buffer = new char[bufferLen];
    //html 源码
    t.read(buffer,bufferLen);
    t.close();
    regex reg("<a href=[^ ]* target=\"_blank\">([^<]*)");
    string szbuf = string(buffer);
    smatch result;
    string::const_iterator iteStart = szbuf.begin();
    string::const_iterator iteEnd = szbuf.end();
    string wb_begin = "https://s.weibo.com/weibo?q=%23";
    string wb_end = "%23&Refer=top";
    int i = 1;
    cout<<"微博热搜"<<endl;
    while(regex_search(iteStart,iteEnd,result,reg))
    {
        string szbuf = result[0];
        string title = result[1].str();
        vec_title.push_back(title);
        string url_temp = wb_begin+result[1].str()+wb_end;
        vec_url.push_back(url_temp);
        cout<<i<<"  "<<title<<"  "<<url_temp<<endl;
        iteStart = result[0].second;
        i++;
    }


}

int WBSpider::download(url_t *url)
{
    char buf[8192] = {0};
    char res[4096] = {0};
    char * pos = NULL;
    int len;
    int fd;
    if(!webssl)
    {

        send(webfd,request,strlen(request),0);
        len = recv(webfd,buf,sizeof(buf),0);

        if((pos = strstr(buf,"\r\n\r\n")) == NULL)
        {
            printf("download failed\n");
            return -1;
        }
        snprintf(res,pos-buf+4,"%s",buf); //pos - buf 地址的差值
        printf("[5] Get RequestHead\n%s",res);
        if(get_statecode(res) == 200)
        {
            fd = open(url->filenames.c_str(),O_RDWR|O_CREAT,0775);
            write(fd,pos+4,len - (pos - buf +4));
            while((len = recv(webfd,buf,sizeof(buf),0))>0)
            {
                write(fd,buf,len);
            }
            bzero(buf,sizeof(buf));
        }
        else
            return -1;
    }
    else
    {

        SSL_write(webssl->sock_ssl,request,strlen(request));
        len = SSL_read(webssl->sock_ssl,buf,sizeof(buf));

        if((pos = strstr(buf,"\r\n\r\n")) == NULL)
        {
            printf("ssl download failed\n");
            return -1;
        }
        snprintf(res,pos-buf+4,"%s",buf); //pos - buf 地址的差值
        printf("[5]Get RequestHead\n%s",res);
        if(get_statecode(res) == 200)
        {
            fd = open(url->filenames.c_str(),O_RDWR|O_CREAT,0775);
            write(fd,pos+4,len - (pos - buf +4));
            while((len = SSL_read(webssl->sock_ssl,buf,sizeof(buf)))>0)
            {
                write(fd,buf,len);
            }
            bzero(buf,sizeof(buf));
        }
        else
            return -1;
    }
    printf("[6] download success\n");
    close(fd);
    return 0;
}
