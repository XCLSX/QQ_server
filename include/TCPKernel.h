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
        icon_id = 0;
        memset(m_szName,0,MAX_SIZE);
        memset(m_szfelling,0,MAX_SIZE);
        status = 0;
        m_user_id = 0;
    }
    int sock_fd;
    int icon_id;
    char m_szName[MAX_SIZE];
    char m_szfelling[MAX_SIZE];
    int status;
    int m_user_id;
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
    //转发消息
    void RepeatMsg(int clientfd ,char* szbuf,int nlen);

    //接受文件头
    void GetFileMSG(int clientfd ,char* szbuf,int nlen);
    //接受文件块
    void GetFileBlock(int clientfd ,char* szbuf,int nlen);
    //发送文件块
    void SendFileBlock(int clientfd ,char* szbuf,int nlen);
    //获取离线信息
    void GetOffMsg(int clientfd,int user_id);

    //下线
    void OffLine(int clientfd ,char* szbuf,int nlen);
    void Test(int clientfd ,char* szbuf,int nlen);

    STRU_USER_INFO* Info_sToInfo(UserInfo_S*);
 private:
    CMysql * m_sql;
    TcpNet * m_tcp;
    unordered_map<int,UserInfo_S*> m_mapIdtoUserInfo;
    unordered_map<string,STRU_FILE_INFO*> map_Md5ToFileinfo;
    pthread_mutex_t lock;
};




#endif
