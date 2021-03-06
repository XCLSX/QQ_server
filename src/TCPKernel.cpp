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
    {DEF_PACK_SENDMSG_RQ,&TcpKernel::RepeatMsg},
    {DEF_PACK_OFFLINE_RQ,&TcpKernel::OffLine},
    {DEF_PACK_UPLOAD_RQ,&TcpKernel::GetFileMSG},
    {DEF_PACK_FILEBLOCK_RQ,&TcpKernel::GetFileBlock},
    {DEF_PACK_UPLOAD_RS,&TcpKernel::SendFileBlock},
    {DEF_PACK_DEL_FRIEND_RQ,&TcpKernel::DelFriendRq},
    {DEF_PACK_ALTER_USERINFO_RQ,&TcpKernel::AlterUserInfo},
    {DEF_PACK_CREATEGROUP_RQ,&TcpKernel::CreateGroup},
    {DEF_PACK_SENDGROUPMSG_RS,&TcpKernel::SendGroupMsg},
    {DEF_PACK_GETHOT_POINT_RQ,&TcpKernel::GetHotLine},
    {DEF_PACK_TEST,&TcpKernel::Test},
    {0,0}
};

int TcpKernel::Open()
{
    pthread_mutex_init(&lock,NULL);
    m_sql = new CMysql;
    m_tcp = new TcpNet(this);
    m_redis = new RedisTool;
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
    m_redis->ExitRedis();
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
            if(fun == 0)
                return ;
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


            ls.clear();
            sprintf(szsql,"select icon_id,user_name,felling,status from t_user where user_id = %d;",user_id);
            if(!m_sql->SelectMysql(szsql,4,ls))
            {
                printf("sql error:%s\n",szsql);
                return ;
            }
            //编写回复包

            rs.m_lResult = login_sucess;
            rs.m_userInfo.m_icon_id = atoi(ls.front().c_str());     ls.pop_front();
            strcpy(rs.m_userInfo.m_userName,ls.front().c_str());    ls.pop_front();
            strcpy(rs.m_userInfo.sz_feeling,ls.front().c_str());    ls.pop_front();
            rs.m_userInfo.m_status = 1;
            rs.m_userInfo.m_user_id = user_id;

            //录入映射
            if(rs.m_userInfo.sz_feeling[0] == 0)
                sprintf(rs.m_userInfo.sz_feeling,"\"11\"");
            string key_redis = "qq-user-"+to_string(user_id);
            string fields_redis = "user_id "+to_string(user_id)+" sockfd "+to_string(clientfd)+
                    " icon_id "+to_string(rs.m_userInfo.m_icon_id)+" user_name "+rs.m_userInfo.m_userName
                    +" feeling "+rs.m_userInfo.sz_feeling;

            if(!m_redis->SetmHashValue(key_redis,fields_redis))
                return ;
        }

        m_tcp->SendData( clientfd , (char*)&rs , sizeof(rs) );

        //发送好友列表
        PostFriList(clientfd,rs.m_userInfo.m_user_id);
        //获取离线信息
        GetOffMsg(clientfd,rs.m_userInfo.m_user_id);

        //通知好友上线
        bzero(szsql,sizeof (szsql));    ls.clear();
        sprintf(szsql,"select friend_id from t_friend where user_id = %d;",user_id);
        if(!m_sql->SelectMysql(szsql,1,ls))
        {
            printf("sql error:%s\n",szsql);
            return;
        }
        STRU_UPDATE_STATUS sus;
        sus.m_UserInfo = rs.m_userInfo;
        sus.m_UserInfo.m_status = 1;
        for(int i=0;i<ls.size();i++)
        {

            string redis_key = "qq-user-"+ls.front();                    ls.pop_front();
            string str;
            str = m_redis->GetHashValue(redis_key,string("sockfd"));
            if(str.size()==0)
                continue;
            int sockfd = atoi(str.c_str());
            m_tcp->SendData(sockfd,(char *)&sus,sizeof(sus));

        }
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
    string key_str = "qq-user-"+to_string(rq->m_frid);
    string sockstr = m_redis->GetHashValue(key_str,string("sockfd"));
    if(sockstr.size()>0)
    {
        int sockfd = atoi(sockstr.c_str());
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
        if(!m_sql->UpdataMysql(szsql))
        {
            printf("sql error:%s\n",szsql);
            return ;
        }
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
    string key_str = "qq-user-"+to_string(rs->m_userid);
    string sockstr = m_redis->GetHashValue(key_str,string("sockfd"));
    if(sockstr.size()>0)
    {
        int sockfd = atoi(sockstr.c_str());
        m_tcp->SendData(sockfd,(char *)&sus,sizeof(sus));
    }
    sus.m_UserInfo = *GetUserInfo(rs->m_userid);
    key_str = "qq-user-"+to_string(rs->m_frid);
    sockstr = m_redis->GetHashValue(key_str,string("sockfd"));
    if(sockstr.size()>0)
    {
        int sockfd = atoi(sockstr.c_str());
        m_tcp->SendData(sockfd,(char *)&sus,sizeof(sus));
    }
}
//发送好友列表
void TcpKernel::PostFriList(int clientfd,int userid)
{
    STRU_GetFriList_Rs rs;

    char szsql[_DEF_SQLIEN] = {0};
    sprintf(szsql,"select t_user.user_id,user_account,user_name,icon_id,status,felling "
                  "from t_user inner join t_friend on t_user.user_id = t_friend.user_id "
                  "where t_friend.friend_id = %d;",userid);
    list<string> ls;
    if(!m_sql->SelectMysql(szsql,6,ls))
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
        rs.m_FriInfo[i].m_icon_id = atoi(ls.front().c_str());           ls.pop_front();
        rs.m_FriInfo[i].m_status = atoi(ls.front().c_str());            ls.pop_front();
        strcpy(rs.m_FriInfo[i].sz_feeling,ls.front().c_str());          ls.pop_front();
        i++;
    }
    m_tcp->SendData(clientfd,(char *)&rs,sizeof(rs));
}
//转发信息
void TcpKernel::RepeatMsg(int clientfd, char *szbuf, int nlen)
{
    STRU_SENDMSG_RQ *rq = (STRU_SENDMSG_RQ*)szbuf;
    string key_str = "qq-user-"+to_string(rq->m_Touserid);
    string sockstr = m_redis->GetHashValue(key_str,string("sockfd"));
    if(sockstr.size()>0)
    {
        int sockfd = atoi(sockstr.c_str());
        m_tcp->SendData(sockfd,szbuf,nlen);

    }
    else
    {
        char szsql[_DEF_SQLIEN] = {0};
        list<string> ls;
        sprintf(szsql,"insert into t_offMsg values(%d,%d,'%s',now());",rq->m_Touserid,rq->m_userid,rq->msg);
        if(!m_sql->UpdataMysql(szsql))
        {
          printf("sql error:%s\n");
          return ;
        }
    }
}

void TcpKernel::DelFriendRq(int clientfd, char *szbuf, int nlen)
{
    STRU_DELFRIEND_RQ *rq = (STRU_DELFRIEND_RQ *)szbuf;
    STRU_DELFRIEND_RS rs;
    rs.del_userid = rq->m_frid;
    char szsql[_DEF_SQLIEN] = {0};
    list<string> ls;
    sprintf(szsql,"select * from t_friend where user_id = %d and friend_id = %d;",rq->m_userid,rq->m_frid);
    if(!m_sql->SelectMysql(szsql,2,ls))
    {
        printf("szsql error:%S\n",szsql);
        return ;
    }
    if(ls.size()==0)
        return ;
    bzero(szsql,sizeof (szsql));
    sprintf(szsql,"delete from t_friend where user_id = %d and friend_id = %d;",rq->m_frid,rq->m_userid);
    if(!m_sql->UpdataMysql(szsql))
    {
        printf("szsql error:%S\n",szsql);
        return ;
    }
    m_tcp->SendData(clientfd,(char *)&rs,sizeof(rs));
    string key_str = "qq-user-"+to_string(rq->m_frid);
    string sockstr = m_redis->GetHashValue(key_str,string("sockfd"));
    if(sockstr.size()>0)
    {
        int sockfd = atoi(sockstr.c_str());
        m_tcp->SendData(sockfd,(char*)&rs,sizeof (rs));
    }
}
//获取文件信息
void TcpKernel::GetFileMSG(int clientfd, char *szbuf, int nlen)
{
    STRU_UPLOAD_RQ *rq = (STRU_UPLOAD_RQ*)szbuf;
    STRU_UPLOAD_RS rs;
    STRU_FILE_INFO * info = new STRU_FILE_INFO;
    strcpy(info->fileMd5,rq->m_szFileMD5);
    strcpy(info->fileName,rq->m_szFileName);
    info->filePos = 0;
    info->fileSize = rq->m_nFileSize;
    sprintf(info->filePath,"/home/hush/tmpFile/%s",rq->m_szFileName);
    info->pFile = fopen(info->filePath,"a");

    string Md5str = info->fileMd5;
    map_Md5ToFileinfo[Md5str] = info;

    rs.m_UserId = rq->m_UserId;
    rs.m_nResult = 1;
    rs.m_friendId = rq->m_friendId;
    strcpy(rs.m_szFileMD5,info->fileMd5);
    if(info->pFile == NULL)
        rs.m_nResult = 0;
    m_tcp->SendData(clientfd,(char *)&rs,sizeof(rs));

}

void TcpKernel::GetFileBlock(int clientfd, char *szbuf, int nlen)
{
    STRU_FILEBLOCK_RQ *rq = (STRU_FILEBLOCK_RQ*)szbuf;
    string Md5 = rq->m_szFileMD5;
    if(map_Md5ToFileinfo.find(Md5) != map_Md5ToFileinfo.end())
    {
        STRU_FILE_INFO * info = map_Md5ToFileinfo[Md5];

        int res = fwrite(rq->m_szFileContent,1,rq->m_nBlockLen,info->pFile);
        info->filePos += res;
        if(info->filePos>=info->fileSize)
        {
            fclose(info->pFile);
            //开始向目标发送文件头
            printf("finsh\n");
            STRU_UPLOAD_RQ urq;
            urq.m_UserId = rq->m_nUserId;
            urq.m_friendId = rq->m_friendId;
            strcpy(urq.m_szFileMD5,rq->m_szFileMD5);
            strcpy(urq.m_szFileName,info->fileName);
            urq.m_nFileSize = info->fileSize;
            string key_str = "qq-user-"+to_string(urq.m_friendId);
            string sockstr = m_redis->GetHashValue(key_str,string("sockfd"));
            if(sockstr.size()>0)
            {
                int sockfd = atoi(sockstr.c_str());
                m_tcp->SendData(sockfd,(char *)&urq,sizeof(urq));
            }
            else
            {
                char szsql[_DEF_SQLIEN] = {0};
                sprintf(szsql,"insert into t_OffFile values(%d,%d,'%s','%s',%ld,'%s');",urq.m_friendId,urq.m_UserId,info->filePath,info->fileName,info->fileSize,info->fileMd5);
                if(!m_sql->UpdataMysql(szsql))
                {
                    printf("szsql error:%s",szsql);
                    return ;
                }
                pthread_mutex_lock(&lock);
                auto ite = map_Md5ToFileinfo.find(Md5);
                map_Md5ToFileinfo.erase(ite);
                delete info;
                info = NULL;
                pthread_mutex_unlock(&lock);
            }

        }
    }
}
//发送文件快
void TcpKernel::SendFileBlock(int clientfd, char *szbuf, int nlen)
{
    STRU_UPLOAD_RS *rs = (STRU_UPLOAD_RS*)szbuf;
    STRU_FILEBLOCK_RQ rq;
    STRU_FILE_INFO *info = NULL;
    rq.m_nUserId = rs->m_UserId;
    rq.m_friendId = rs->m_friendId;
    string md5 = rs->m_szFileMD5;
    if(map_Md5ToFileinfo.find(md5)!=map_Md5ToFileinfo.end())
       info = map_Md5ToFileinfo[md5];
       if(rs->m_nResult == 0)
       {
           fclose(info->pFile);
           pthread_mutex_lock(&lock);
           string md5 = rs->m_szFileMD5;
           auto ite = map_Md5ToFileinfo.find(md5);
           if(ite != map_Md5ToFileinfo.end())
           {
               delete (*ite).second;
               map_Md5ToFileinfo.erase(ite);

           }
           pthread_mutex_unlock(&lock);
           return ;
       }
       else
       {

           info->pFile = fopen(info->filePath,"r");
           info->filePos = 0;
           int pos = 0;
           int times = 0;

              while(1)
              {
                  pos++;
                  if(pos%500==0)
                  {
                      usleep(10000);
                  }

                  int res  = fread( rq.m_szFileContent , 1 , MAX_CONTENT_LEN ,info->pFile);
                  strcpy( rq.m_szFileMD5  , info->fileMd5 );

                  info->filePos += res ;
                  rq.m_nBlockLen = res;
                  while( m_tcp->SendData(clientfd, (char*)&rq , sizeof(rq) )== false)
                     {
                          usleep(100000);
                          //判断时间
                          times++;
                          if(times == 20)
                              return;
                     }


                  if( info->filePos >= info->fileSize )
                  {
                      fclose(info->pFile);
                      pthread_mutex_lock(&lock);
                      string md5 = rq.m_szFileMD5;
                      auto ite = map_Md5ToFileinfo.find(md5);
                      if(ite != map_Md5ToFileinfo.end())
                      {
                          delete (*ite).second;
                          map_Md5ToFileinfo.erase(ite);

                      }
                      pthread_mutex_unlock(&lock);

                          return ;
                  }
              }


       }
}
//获取离线信息
void TcpKernel::GetOffMsg(int clientfd, int user_id)
{
    //获取离线好友聊天信息
    char szsql[_DEF_SQLIEN] = {0};
    list<string> ls;
    sprintf(szsql,"select user_id,from_id,msg from t_offMsg where user_id = %d;",user_id);
    if(!m_sql->SelectMysql(szsql,3,ls))
    {
        printf("sql error:%s\n",szsql);
        return;
    }
    for(int i=0;i<ls.size();i++)
    {
        STRU_SENDMSG_RQ rq;
        rq.m_Touserid = atoi(ls.front().c_str());       ls.pop_front();
        rq.m_userid = atoi(ls.front().c_str());         ls.pop_front();
        strcpy(rq.msg,ls.front().c_str());              ls.pop_front();
        m_tcp->SendData(clientfd,(char *)&rq,sizeof(rq));
    }
    bzero(szsql,sizeof(szsql));
    sprintf(szsql,"delete from t_offMsg where user_id = %d;",user_id);
    if(!m_sql->UpdataMysql(szsql))
    {
        printf("sql error:%s\n",szsql);
        return;
    }
    //获取离线好友申请
    ls.clear();
    bzero(szsql,sizeof(szsql));
    sprintf(szsql,"select friend_id,fri_account,fri_name,fri_iconid,fri_feeling from t_offAddfriend where user_id = %d;",user_id);
    if(!m_sql->SelectMysql(szsql,5,ls))
    {
        printf("sql error:%s\n",szsql);
        return;
    }
    for(int i=0;i<ls.size();i++)
    {
        STRU_ADDFRIEND_RQ rq;
        rq.m_frid = user_id;
        rq.m_UserInfo.m_user_id = atoi(ls.front().c_str());         ls.pop_front();
        strcpy(rq.m_UserInfo.m_userAccount,ls.front().c_str());     ls.pop_front();
        strcpy(rq.m_UserInfo.m_userName,ls.front().c_str());        ls.pop_front();
        rq.m_UserInfo.m_icon_id = atoi(ls.front().c_str());         ls.pop_front();
        strcpy(rq.m_UserInfo.sz_feeling,ls.front().c_str());        ls.pop_front();
        m_tcp->SendData(clientfd,(char *)&rq,sizeof(rq));
    }
    bzero(szsql,sizeof(szsql));
    sprintf(szsql,"delete from t_offAddfriend where user_id = %d;",user_id);
    if(!m_sql->UpdataMysql(szsql))
    {
        printf("sql error:%s\n",szsql);
        return;
    }
    //获取群信息
    ls.clear();
    STRU_GET_GROUP_INFO_RQ ggiq;
    bzero(szsql,sizeof(szsql));
    sprintf(szsql,"select group_id from t_group where user_id = %d;",user_id);
    if(!m_sql->SelectMysql(szsql,1,ls))
    {
        printf("sql error:%s\n",szsql);
        return;
    }
    auto ite = ls.begin();
    while(ite!=ls.end())
    {
        //获取群信息
        int group_id = atoi((*ite).c_str());
        list<string> templs;
        bzero(szsql,sizeof(szsql));
        sprintf(szsql,"select group_name,group_iconid from t_groupInfo where group_id = %d;",group_id);
        if(!m_sql->SelectMysql(szsql,2,templs))
        {
            printf("sql error:%s\n",szsql);
            return;
        }
        ggiq.m_groupid = group_id;
        strcpy(ggiq.m_szGroupName,templs.front().c_str());           templs.pop_front();
        ggiq.m_iconid = atoi(templs.front().c_str());
        //获取群成员信息
        bzero(szsql,sizeof(szsql));
        templs.clear();
        sprintf(szsql,"select * from t_group inner join t_user on t_group.user_id = t_user.user_id where group_id = %d;",ggiq.m_groupid);
        if(!m_sql->SelectMysql(szsql,10,templs))
        {
            printf("sql error:%s\n",szsql);
            return;
        }
        STRU_USER_INFO info;
        for(int i=0;i<templs.size();i++)
        {
            templs.pop_front();
            info.m_user_id = atoi(templs.front().c_str());      templs.pop_front();
            ggiq.rightarr[i] = atoi(templs.front().c_str());    templs.pop_front();     templs.pop_front();
            strcpy(info.m_userAccount,templs.front().c_str());  templs.pop_front();     templs.pop_front();
            strcpy(info.m_userName,templs.front().c_str());     templs.pop_front();
            info.m_icon_id = atoi(templs.front().c_str());      templs.pop_front();
            info.m_status = atoi(templs.front().c_str());       templs.pop_front();
            strcpy(info.sz_feeling,templs.front().c_str());     templs.pop_front();
            ggiq.m_userInfo[i] = info;
        }
        m_tcp->SendData(clientfd,(char *)&ggiq,sizeof(ggiq));
        memset(&ggiq,0,sizeof(ggiq));
        ggiq.m_nType = DEF_PACK_GETGROUPINFO_RQ;
        ++ite;
    }

}
//修改个人信息
void TcpKernel::AlterUserInfo(int clientfd, char *szbuf, int nlen)
{
    STRU_ALTER_USERINFO_RQ *rq = (STRU_ALTER_USERINFO_RQ*)szbuf;
    STRU_ALTER_USERINFO_RS rs;
    rs.m_iconid = rq->m_iconid;
    strcpy(rs.m_szName,rq->m_szName);
    strcpy(rs.m_szFeeling,rq->m_szFeeling);
    char szsql[_DEF_SQLIEN] = {0};
    sprintf(szsql,"update t_user set user_name =  '%s', felling = '%s', icon_id = %d where user_id = %d;",rq->m_szName,rq->m_szFeeling,rq->m_iconid,rq->m_userid);
    if(!m_sql->UpdataMysql(szsql))
    {
        printf("sql error:%s\n");
        return ;
    }
    m_tcp->SendData(clientfd,(char *)&rs,sizeof(rs));
    //为所有在线好友更新信息
    STRU_UPDATE_STATUS sus;
    sus.m_UserInfo.m_status = 1;
    sus.m_UserInfo.m_icon_id = rq->m_iconid;
    sus.m_UserInfo.m_user_id = rq->m_userid;
    strcpy(sus.m_UserInfo.m_userName,rq->m_szName);
    strcpy(sus.m_UserInfo.sz_feeling,rq->m_szFeeling);
    sus.m_UserInfo.m_userAccount[0] = 0;

    bzero(szsql,sizeof(szsql));
    sprintf(szsql,"select friend_id from t_friend where user_id = %d;",rq->m_userid);
    list<string> ls;
    if(!m_sql->SelectMysql(szsql,1,ls))
    {
        printf("sql error:%s\n",szsql);
        return;
    }
    for(int i=0;i<ls.size();i++)
    {
        string key_str = "qq-user-"+ls.front();         ls.pop_front();
        string sockstr = m_redis->GetHashValue(key_str,string("sockfd"));
        if(sockstr.size()>0)
        {
            int sockfd = atoi(sockstr.c_str());
            m_tcp->SendData(sockfd,(char *)&sus,sizeof (sus));
        }
    }

}
//群聊
void TcpKernel::CreateGroup(int clientfd, char *szbuf, int nlen)
{
    STRU_CREATE_GROUP_RQ *rq = (STRU_CREATE_GROUP_RQ*)szbuf;
    STRU_CREATE_GROUP_RS rs;
    char szsql[_DEF_SQLIEN] = {0};
    sprintf(szsql,"insert into t_groupInfo values(null,'%s',1,%d,0);",rq->m_szGroupName,rq->m_userid);
    if(!m_sql->UpdataMysql(szsql))
    {
        printf("sql error:%s\n",szsql);
        return;
    }
    bzero(szsql,sizeof(szsql));
    sprintf(szsql,"select group_id from t_groupInfo where group_name = '%s' and leader_id = %d;",rq->m_szGroupName,rq->m_userid);
    list<string> ls;
    if(!m_sql->SelectMysql(szsql,1,ls))
    {
        printf("sql error:%s\n",szsql);
        return;
    }
    int group_id = atoi(ls.front().c_str());
    bzero(szsql,sizeof(szsql));
    sprintf(szsql,"insert into t_group values(%d,%d,0);",group_id,rq->m_userid);
    if(!m_sql->UpdataMysql(szsql))
    {
        printf("sql error:%s\n",szsql);
        return;
    }

    rs.m_groupid = group_id;
    strcpy(rs.m_szGroupName,rq->m_szGroupName);
    m_tcp->SendData(clientfd,(char *)&rs,sizeof(rs));
}
//转发群聊消息
void TcpKernel::SendGroupMsg(int clientfd, char *szbuf, int nlen)
{
    STRU_SENDGROUPMSG_RQ *rq = (STRU_SENDGROUPMSG_RQ*)szbuf;
    char szsql[_DEF_SQLIEN] = {0};
    sprintf(szsql,"select user_id from t_group where group_id = %d;",rq->m_groupid);
    list<string> ls;
    if(!m_sql->SelectMysql(szsql,1,ls))
    {
        printf("sql error:%s\n",szsql);
        return;
    }
    auto ite = ls.begin();
    while(ite!=ls.end())
    {

        string key_str = "qq-user-"+*ite;
        string sockstr = m_redis->GetHashValue(key_str,string("sockfd"));
        if(sockstr.size()>0)
        {
            int sockfd = atoi(sockstr.c_str());
            m_tcp->SendData(sockfd,szbuf,nlen);
        }
        ++ite;
    }
}

void TcpKernel::GetHotLine(int clientfd, char *szbuf, int nlen)
{
    STRU_GETHOTPOINT_RS rs;
    WBSpider sp;
    sp.work();
    for(int i=0;i<50;i++)
    {
        strcpy(rs.m_hotres[i].sz_title,sp.vec_title[i].c_str());
        strcpy(rs.m_hotres[i].sz_url,sp.vec_url[i].c_str());
    }
    m_tcp->SendData(clientfd,(char *)&rs,sizeof(rs));
}
//下线
void TcpKernel::OffLine(int clientfd, char *szbuf, int nlen)
{
    STRU_OFFLINE_RQ *rq = (STRU_OFFLINE_RQ*)szbuf;
    char szsql[_DEF_SQLIEN] = {0};
    sprintf(szsql,"update t_user set status = 0 where user_id = %d;",rq->m_userid);
    if(!m_sql->UpdataMysql(szsql))
    {
        printf("sql error:%s\n");
        return ;
    }

    STRU_USER_INFO *info = GetOnlineUserInfo(rq->m_userid);
    string key = "qq-user-"+to_string(rq->m_userid);
    m_redis->RemoveKey(key);

    //通知好友下线
    bzero(szsql,sizeof(szsql));
    sprintf(szsql,"select friend_id from t_friend where user_id = %d;",rq->m_userid);
    list<string> ls;
    if(!m_sql->SelectMysql(szsql,1,ls))
    {
        printf("sql error:%s\n",szsql);
        return;
    }
    STRU_UPDATE_STATUS sus;
    sus.m_UserInfo = *info;
    sus.m_UserInfo.m_status = 0;
    for(int i=0;i<ls.size();i++)
    {
        string key_str = "qq-user-"+ls.front();         ls.pop_front();
        string sockstr = m_redis->GetHashValue(key_str,string("sockfd"));
        if(sockstr.size()>0)
        {
            int sockfd = atoi(sockstr.c_str());
            m_tcp->SendData(sockfd,(char *)&sus,sizeof (sus));
        }
    }
    char id[5] = {0};
    sprintf(id,"%d",rq->m_userid);
    string m_key = "qq-user-"+string(id);
    m_redis->RemoveKey(m_key);

}

void TcpKernel::Test(int clientfd, char *szbuf, int nlen)
{
    cout<<"sockfd:\t"<<clientfd<<"Net success\n";
}

STRU_USER_INFO *TcpKernel::Info_sToInfo(UserInfo_S *info_s)
{
    STRU_USER_INFO * info = new STRU_USER_INFO;
    info->m_status = info_s->status;
    info->m_icon_id = info_s->icon_id;
    info->m_user_id = info_s->m_user_id;
    strcpy(info->m_userName,info_s->m_szName);
    strcpy(info->sz_feeling,info_s->m_szfelling);
    return info;
}

STRU_USER_INFO *TcpKernel::GetOnlineUserInfo(int user_id)
{
    string key_str = "qq-user-"+to_string(user_id);
    list<string> ls;
    ls = m_redis->GetHashAllValue(key_str);
    if(ls.size() == 0)
        return NULL;
    STRU_USER_INFO *info = new STRU_USER_INFO;
    info->m_user_id = user_id; ls.pop_front();      ls.pop_front();    //跳过user_id 和 sockfd
    info->m_icon_id = atoi(ls.front().c_str());     ls.pop_front();
    strcpy(info->m_userName,ls.front().c_str());    ls.pop_front();
    strcpy(info->sz_feeling,ls.front().c_str());    ls.pop_front();
    return info;
}



