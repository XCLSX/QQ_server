#include<TCPKernel.h>
#include "packdef.h"
#include<stdio.h>

using namespace std;


////注册
//#define  DEF_PACK_REGISTER_RQ    (DEF_PACK_BASE + 0)
//#define  DEF_PACK_REGISTER_RS    (DEF_PACK_BASE + 1)
////登录
//#define  DEF_PACK_LOGIN_RQ    (DEF_PACK_BASE + 2)
//#define  DEF_PACK_LOGIN_RS    (DEF_PACK_BASE + 3)

static const ProtocolMap m_ProtocolMapEntries[] =
{
    {DEF_PACK_REGISTER_RQ , &TcpKernel::Register},
    {DEF_PACK_LOGIN_RQ , &TcpKernel::Login},
    {DEF_PACK_SEARCHFRIEND_RQ,&TcpKernel::SearchFriend},
    {DEF_PACK_ADDFRIEND_RQ,&TcpKernel::RepeatFriendRq},
    {DEF_PACK_ADDFRIEND_RS,&TcpKernel::AddFriend},
    {DEF_PACK_TEST,&TcpKernel::Test},
    {0,0}
};
#define RootPath   "/home/colin/Video/"

int TcpKernel::Open()
{
    m_sql = new CMysql;
    m_tcp = new TcpNet(this);
    m_tcp->SetpThis(m_tcp);
    pthread_mutex_init(&m_tcp->alock,NULL);
    pthread_mutex_init(&m_tcp->rlock,NULL);
    if(  !m_sql->ConnectMysql("localhost","root","root","qqdb")  )
    {
        printf("Conncet Mysql Failed...\n");
        return FALSE;
    }
    else
    {
        printf("MySql Connect Success...\n");
    }
    if( !m_tcp->InitNetWork()  )
    {
        printf("InitNetWork Failed...\n");
        return FALSE;
    }
    else
    {
        printf("Init Net Success...\n");
    }

    return TRUE;
}

void TcpKernel::Close()
{
    m_sql->DisConnect();
    m_tcp->UnInitNetWork();
}


void TcpKernel::DealData(int clientfd,char *szbuf,int nlen)
{
    PackType *pType = (PackType*)szbuf;
    int i = 0;
    while(1)
    {
        if(*pType == m_ProtocolMapEntries[i].m_type)
        {
            PFUN fun= m_ProtocolMapEntries[i].m_pfun;
            (this->*fun)(clientfd,szbuf,nlen);
        }
        else if(m_ProtocolMapEntries[i].m_type == 0 &&
                m_ProtocolMapEntries[i].m_pfun == 0)
            return;
        ++i;
    }
    return;
}

STRU_USER_INFO *TcpKernel::GetUserInfo(int m_userid)
{
    char szsql[MAX_SIZE] = {0};
    list<string> ls;
    sprintf(szsql,"select icon_id,user_name,felling,status from t_user where user_id = %d;",m_userid);
    if(!m_sql->SelectMysql(szsql,4,ls))
    {
        printf("sql error:%s\n",szsql);
        return NULL;
    }
    STRU_USER_INFO *uInfo = new STRU_USER_INFO;
    uInfo->m_icon_id = atoi(ls.front().c_str());   ls.pop_front();
    strcpy(uInfo->m_userName,ls.front().c_str());       ls.pop_front();
    strcpy(uInfo->sz_feeling ,ls.front().c_str());    ls.pop_front();
    uInfo->m_status = atoi(ls.front().c_str());           ls.pop_front();
    return  uInfo;
}




//注册
void TcpKernel::Register(int clientfd,char* szbuf,int nlen)
{
    printf("clientfd:%d RegisterRq\n", clientfd);

    STRU_REGISTER_RQ * rq = (STRU_REGISTER_RQ *)szbuf;
    STRU_REGISTER_RS rs;
    list<string> ls;
    char szsql[_DEF_SQLIEN] = {0};
    snprintf(szsql,sizeof(szsql),"select user_account from t_user where user_account = '%s';",rq->m_szAccount);
    if(!m_sql->SelectMysql(szsql,1,ls))
    {
        printf("sql error:%s\n",szsql);
        return ;
    }
    //用户已存在
    if(ls.size()!=0)
    {
        rs.m_lResult = userid_is_exist;
    }
    else
    {
        rs.m_lResult = register_sucess;
        bzero(szsql,sizeof(szsql));
        sprintf(szsql,"insert into t_user values(null,'%s','%s','%s',0,0,'');",rq->m_szAccount,rq->m_szPwd,rq->m_szName);
        if(!m_sql->UpdataMysql(szsql))
        {
            printf("sql error:%s\n",szsql);
            return ;
        }


    }
    m_tcp->SendData( clientfd , (char*)&rs , sizeof(rs) );
}
//登录
void TcpKernel::Login(int clientfd ,char* szbuf,int nlen)
{
    printf("clientfd:%d LoginRq\n", clientfd);

    STRU_LOGIN_RQ * rq = (STRU_LOGIN_RQ *)szbuf;
    STRU_LOGIN_RS rs;
    list<string> ls;
    char szsql[_DEF_SQLIEN] = {0};
    sprintf(szsql,"select user_id,user_pwd from t_user where user_account = '%s';",rq->m_szAccount);
    if(!m_sql->SelectMysql(szsql,2,ls))
    {
        printf("sql error:%s\n",szsql);
    }
    if(ls.size()==0)    //用户不存在
    {
        rs.m_lResult = userid_no_exist;
        m_tcp->SendData( clientfd , (char*)&rs , sizeof(rs) );
        return ;
    }
    int user_id = atoi(ls.front().c_str());     ls.pop_front();
    if(strcmp(rq->m_szPwd,ls.front().c_str())!=0)           //密码错误
    {
        rs.m_lResult = password_error;
    }
    else
    {
        //修改登录状态
        memset(szsql,0,sizeof(szsql));
        sprintf(szsql,"update  t_user set status = 1 where user_id = %d;",user_id);
        if(!m_sql->UpdataMysql(szsql))
        {
            printf("sql error:%s\n",szsql);
        }
        //录入映射
        UserInfo_S *userInfo = new UserInfo_S;
        userInfo->sock_fd = clientfd;
        ls.clear();
        sprintf(szsql,"select icon_id,user_name,felling,status from t_user where user_id = %d;",user_id);
        if(!m_sql->SelectMysql(szsql,4,ls))
        {
            printf("sql error:%s\n",szsql);
            return ;
        }
        userInfo->iocnid = atoi(ls.front().c_str());            ls.pop_front();
        strcpy(userInfo->m_szName,ls.front().c_str());          ls.pop_front();
        strcpy(userInfo->m_szfelling,ls.front().c_str());       ls.pop_front();
        userInfo->status = atoi(ls.front().c_str());
        m_mapIdtoUserInfo[user_id] = userInfo;
        //编写回复包
        rs.m_lResult = login_sucess;
        rs.m_userInfo.m_icon_id = userInfo->iocnid;
        strcpy(rs.m_userInfo.m_userName,userInfo->m_szName);
        strcpy(rs.m_userInfo.sz_feeling,userInfo->m_szfelling);
        rs.m_userInfo.m_status = userInfo->status;
        rs.m_userInfo.m_user_id = user_id;

    }

    m_tcp->SendData( clientfd , (char*)&rs , sizeof(rs) );

    //发送好友列表
    PostFriList(clientfd,rs.m_userInfo.m_user_id);
}
//查找好友
void TcpKernel::SearchFriend(int clientfd ,char* szbuf,int nlen)
{
    STRU_SEARCHFRIEND_RQ *rq = (STRU_SEARCHFRIEND_RQ*)szbuf;
    STRU_SEARCHFRIEND_RS rs;
    char szsql[_DEF_SQLIEN] = {0};
    list<string> ls;

    sprintf(szsql,"select * from t_user where user_name like  '%%%s%';",rq->m_szBuf);
   // cout<<szsql<<endl;
    if(!m_sql->SelectMysql(szsql,7,ls))
    {
        printf("sql error:%s\n",szsql);
        return ;
    }

    for(int i=0;i<ls.size();i++)
    {
        rs.m_userInfoArr[i].m_user_id = atoi(ls.front().c_str());           ls.pop_front();
        strcpy(rs.m_userInfoArr[i].m_userAccount,ls.front().c_str());       ls.pop_front(); ls.pop_front();
        strcpy(rs.m_userInfoArr[i].m_userName,ls.front().c_str());          ls.pop_front();
        rs.m_userInfoArr[i].m_icon_id = atoi(ls.front().c_str());           ls.pop_front();
        rs.m_userInfoArr[i].m_status = atoi(ls.front().c_str());            ls.pop_front();
        strcpy(rs.m_userInfoArr[i].sz_feeling,ls.front().c_str());          ls.pop_front();
    }
    m_tcp->SendData(clientfd,(char *)&rs,sizeof(rs));
}
//转发好友请求
void TcpKernel::RepeatFriendRq(int clientfd ,char* szbuf,int nlen)
{
    STRU_ADDFRIEND_RQ *rq = (STRU_ADDFRIEND_RQ*)szbuf;
    //在线
    if(m_mapIdtoUserInfo.find(rq->m_frid)!=m_mapIdtoUserInfo.end())
    {
        int sockfd = m_mapIdtoUserInfo[rq->m_frid]->sock_fd;
        m_tcp->SendData(sockfd,szbuf,nlen);
    }
    //离线
    else
    {
        char szsql[_DEF_SQLIEN] = {0};
        list<string> ls;
        sprintf(szsql,"insert into t_offAddfriend values(%d,%d,'%s','%s',%d,'%s',null);"
                ,rq->m_frid,rq->m_UserInfo.m_user_id,rq->m_UserInfo.m_userAccount,
                rq->m_UserInfo.m_userName,rq->m_UserInfo.m_icon_id,rq->m_UserInfo.sz_feeling);
    }
}
//添加好友
void TcpKernel::AddFriend(int clientfd ,char* szbuf,int nlen)
{
    STRU_ADDFRIEND_RS *rs = (STRU_ADDFRIEND_RS*)szbuf;
    char szsql[_DEF_SQLIEN] = {0};
    list<string> ls;
    sprintf(szsql,"insert into t_friend values(%d,%d);",rs->m_frid,rs->m_userid);
    if(!m_sql->UpdataMysql(szsql))
    {
        printf("sql error:%s\n");
        return ;
    }
    bzero(szsql,sizeof(szsql));
    sprintf(szsql,"insert into t_friend values(%d,%d);",rs->m_userid,rs->m_frid);
    if(!m_sql->UpdataMysql(szsql))
    {
        printf("sql error:%s\n");
        return ;
    }
    //更新双方好友信息
    STRU_UPDATE_STATUS sus;

    sus.m_UserInfo = *GetUserInfo(rs->m_frid);
    m_tcp->SendData(m_mapIdtoUserInfo[rs->m_userid]->sock_fd,(char *)&sus,sizeof(sus));

    bzero(&sus,sizeof(sus));

    sus.m_UserInfo = *GetUserInfo(rs->m_userid);
    m_tcp->SendData(m_mapIdtoUserInfo[rs->m_frid]->sock_fd,(char *)&sus,sizeof(sus));
}

void TcpKernel::PostFriList(int clientfd,int userid)
{
    STRU_GetFriList_Rs rs;
   //rs.m_FriInfo->
    char szsql[_DEF_SQLIEN] = {0};
    sprintf(szsql,"select t_user.user_id,user_account,user_name,icon_id,status,felling "
                  "from t_user inner join t_friend on t_user.user_id = t_friend.user_id "
                  "where t_friend.friend_id = %d;",userid);
    list<string> ls;
    if(!m_sql->SelectMysql(szsql,5,ls))
    {
        printf("sql error:%s\n",szsql);
        return ;
    }
    int i=0;
    while(ls.size()>0)
    {
        rs.m_FriInfo[i].m_user_id = atoi(ls.front().c_str());           ls.pop_front();
        strcpy(rs.m_FriInfo[i].m_userAccount,ls.front().c_str());       ls.pop_front();
        strcpy(rs.m_FriInfo[i].m_userName,ls.front().c_str());          ls.pop_front();
        rs.m_FriInfo[i].m_icon_id = atoi(ls.front().c_str());                   ls.pop_front();
        strcpy(rs.m_FriInfo[i].sz_feeling,ls.front().c_str());          ls.pop_front();
        i++;
    }
    m_tcp->SendData(clientfd,(char *)&rs,sizeof(rs));
}

void TcpKernel::Test(int clientfd, char *szbuf, int nlen)
{
    cout<<"sockfd:\t"<<clientfd<<"Net success\n";
}



