#ifndef _TCPKERNEL_H
#define _TCPKERNEL_H



#include "TCPNet.h"
#include "Mysql.h"
#include <packdef.h>
class TcpKernel;
typedef void (TcpKernel::*PFUN)(int,char*,int nlen);

typedef struct
{
    PackType m_type;
    PFUN m_pfun;
} ProtocolMap;

typedef struct UserInfo_S
{
    UserInfo_S()
    {
        sock_fd = 0;
        memset(m_szName,0,MAX_SIZE);
        memset(m_szfelling,0,MAX_SIZE);
        status = 0;
    }
    int sock_fd;
    int iocnid;
    char m_szName[MAX_SIZE];
    char m_szfelling[MAX_SIZE];
    int status;
}UserInfo_S;

class TcpKernel:public IKernel
{
public:
    int Open();
    void Close();
    void DealData(int clientfd ,char* szbuf,int nlen);
    STRU_USER_INFO *GetUserInfo(int );
    //注册
    void Register(int,char*,int);
    //登录
    void Login(int,char*,int);
    //查找好友
    void SearchFriend(int,char*,int);

    //转发好友请求
    void RepeatFriendRq(int,char*,int);
    //添加好友
    void AddFriend(int,char*,int);
    //发送好友列表
    void PostFriList(int,int);


    void Test(int clientfd ,char* szbuf,int nlen);
 private:
    CMysql * m_sql;
    TcpNet * m_tcp;
    map<int,UserInfo_S*> m_mapIdtoUserInfo;
};






////读取上传的视频流信息
//void UpLoadVideoInfoRq(int,char*);
//void UpLoadVideoStreamRq(int,char*);
//void GetVideoRq(int,char*);
//char* GetVideoPath(char*);
//void QuitRq(int,char*);
//void PraiseVideoRq(int,char*);
//void GetAuthorInfoRq(int,char*);
#endif
