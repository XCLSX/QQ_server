#ifndef _PACKDEF_H
#define _PACKDEF_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <sys/epoll.h>
#include <sys/stat.h>
#include <pthread.h>
#include <signal.h>
#include <errno.h>
#include "err_str.h"
#include <malloc.h>

#include<iostream>
#include<map>
#include <unordered_map>
#include<list>

using namespace std;

//边界值
#define _DEF_SIZE 45
#define _DEF_BUFFERSIZE 1000
#define _DEF_PORT 8000
#define _DEF_SERVERIP "0.0.0.0"  //TODO


#define TRUE true
#define FALSE false


#define _DEF_LISTEN 128
#define _DEF_EPOLLSIZE 4096
#define _DEF_IPSIZE 16
#define _DEF_COUNT 10
#define _DEF_TIMEOUT 10
#define _DEF_SQLIEN 400

typedef enum Net_PACK
{
    DEF_PACK_REGISTER_RQ = 10000,               //注册请求
    DEF_PACK_REGISTER_RS,

    DEF_PACK_LOGIN_RQ,                          //登录请求
    DEF_PACK_LOGIN_RS,

    DEF_PACK_FRIENDLIST_FRESH_RS,               //刷新好友回复

    DEF_PACK_SEARCHFRIEND_RQ,                   //查找好友请求
    DEF_PACK_SEARCHFRIEND_RS,

    DEF_PACK_ADDFRIEND_RQ,                      //添加好友请求
    DEF_PACK_ADDFRIEND_RS,

    DEF_PACK_UPDATESTATUS,                      //更新用户状态

    DEF_PACK_SENDMSG_RQ,                        //发送聊天信息请求

    DEF_PACK_OFFLINE_RQ,                        //下线请求

    DEF_PACK_UPLOAD_RQ,                         //上传文件请求
    DEF_PACK_UPLOAD_RS,
    DEF_PACK_FILEBLOCK_RQ,                      //发送文件块请求

    DEF_PACK_DEL_FRIEND_RQ,                     //删除好友请求
    DEF_PACK_DEL_FRIEND_RS,                     //删除好友回复

    DEF_PACK_ALTER_USERINFO_RQ,                 //修改个人信息请求
    DEF_PACK_ALTER_USERINFO_RS,

    DEF_PACK_CREATEGROUP_RQ,                    //创建群聊请求
    DEF_PACK_CREATEGROUP_RS,

    DEF_PACK_GETGROUPINFO_RQ,                   //获取群聊信息

    DEF_PACK_SENDGROUPMSG_RS,

    DEF_PACK_GETHOT_POINT_RQ,                    //获取热搜请求
    DEF_PACK_GETHOT_POINT_RS,
    DEF_PACK_TEST,
}Net_PACK;


//注册请求结果
#define userid_is_exist      0
#define register_sucess      1

//登录请求结果
#define userid_no_exist      0
#define password_error       1
#define login_sucess         2
#define user_online          3



#define DEF_PACK_COUNT (100)

#define MAX_PATH            (280 )
#define MAX_SIZE            (60  )
#define DEF_HOBBY_COUNT     (8  )
#define MAX_MGS_SIZE        (2048)
#define MAX_CONTENT_LEN     (4096 )


/////////////////////网络//////////////////////////////////////


#define DEF_MAX_BUF	  1024
#define DEF_BUFF_SIZE	  4096


typedef int PackType;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        //用户信息
        typedef struct STRU_USER_INFO
        {
            STRU_USER_INFO()
            {
                m_user_id = 0;
                m_icon_id = 0;
                memset(m_userName,0,MAX_SIZE);
                memset(sz_feeling,0,MAX_SIZE);
                memset(m_userAccount,0,MAX_SIZE);
            }
            int m_user_id;
            char m_userAccount[MAX_SIZE];
            char m_userName[MAX_SIZE];
            int m_icon_id;
            int m_status;
            char sz_feeling[MAX_SIZE];
        }STRU_USER_INFO;

        //文件信息
        typedef struct STRU_FILE_INFO
        {
            STRU_FILE_INFO()
            {
                memset(fileMd5,0,sizeof(fileMd5));
                filePos = 0;
                fileSize = 0;
                memset(fileName,0,sizeof (fileName));
                memset(filePath,0,sizeof(filePath));
                pFile = NULL;
            }
            char fileMd5[MAX_SIZE];
            int64_t filePos;
            int64_t fileSize;
            char filePath[MAX_PATH];
            char fileName[MAX_PATH];
            FILE * pFile;

        }STRU_FILE_INFO;
//注册
typedef struct STRU_REGISTER_RQ
{
    STRU_REGISTER_RQ()
    {
        m_nType = DEF_PACK_REGISTER_RQ;
        memset(m_szAccount,0,MAX_SIZE);
        memset(m_szName,0,MAX_SIZE);
        memset(m_szPwd,0,MAX_SIZE);
    }
    PackType m_nType;
    char m_szAccount[MAX_SIZE];
    char m_szName[MAX_SIZE];
    char m_szPwd[MAX_SIZE];
}STRU_REGISTER_RQ;
typedef struct STRU_REGISTER_RS
{
    STRU_REGISTER_RS()
    {
        m_nType = DEF_PACK_REGISTER_RS;
        m_lResult = 0;
    }
    PackType m_nType;
    int m_lResult;
}STRU_REGISTER_RS;

//登录
typedef struct STRU_LOGIN_RQ
{
    STRU_LOGIN_RQ()
    {
        m_nType = DEF_PACK_LOGIN_RQ;
        memset(m_szAccount,0,MAX_SIZE);
        memset(m_szPwd,0,MAX_SIZE);
    }
    PackType m_nType;
    char m_szAccount[MAX_SIZE];
    char m_szPwd[MAX_SIZE];
}STRU_LOGIN_RQ;
typedef struct STRU_LOGIN_RS
{
    STRU_LOGIN_RS()
    {
        m_Type = DEF_PACK_LOGIN_RS;
        m_lResult = 0;
    }
    PackType m_Type;
    int m_lResult;
    STRU_USER_INFO m_userInfo;
}STRU_LOGIN_RS;

//获取好友列表
typedef struct STRU_GetFriList_Rs
{
    STRU_GetFriList_Rs()
    {
        m_nType = DEF_PACK_FRIENDLIST_FRESH_RS;
    }
    PackType m_nType;
    STRU_USER_INFO m_FriInfo[100];
}STRU_GetFriList_Rs;

//查找好友
typedef struct STRU_SEARCHFRIEND_RQ
{
    STRU_SEARCHFRIEND_RQ()
    {
        m_nType = DEF_PACK_SEARCHFRIEND_RQ;
        memset(m_szBuf,0,MAX_SIZE);
    }
    PackType m_nType;
    char m_szBuf[MAX_SIZE];
}STRU_SEARCHFRIEND_RQ;

typedef struct STRU_SEARCHFRIEND_RS
{
    STRU_SEARCHFRIEND_RS()
    {
        m_nType = DEF_PACK_SEARCHFRIEND_RS;
        memset(m_userid,0,MAX_SIZE);

    }
    PackType m_nType;
    STRU_USER_INFO m_userInfoArr[MAX_SIZE];
    int m_userid[MAX_SIZE];
}STRU_SEARCHFRIEND_RS;

//添加好友
typedef struct STRU_ADDFRIEND_RQ
{
    STRU_ADDFRIEND_RQ()
    {
        m_nType = DEF_PACK_ADDFRIEND_RQ;
        m_frid = 0;
    }
    PackType m_nType;
    int m_frid;
    STRU_USER_INFO m_UserInfo;
}STRU_ADDFRIEND_RQ;
typedef struct STRU_ADDFRIEND_RS
{
    STRU_ADDFRIEND_RS()
    {
        m_nType = DEF_PACK_ADDFRIEND_RS;
    }
    PackType m_nType;
    int m_userid;
    int m_frid;
}STRU_ADDFRIEND_RS;

//更新在线状态
typedef struct STRU_UPDATE_STATUS
{
    STRU_UPDATE_STATUS()
    {
        m_nType = DEF_PACK_UPDATESTATUS;
    }
    PackType m_nType;
    STRU_USER_INFO m_UserInfo;
}STRU_UPDATE_STATUS;

typedef struct STRU_TEST
{
    STRU_TEST()
    {
        m_nType = DEF_PACK_TEST;
    }
    PackType m_nType;
}STRU_TEST;

//发送信息请求
typedef struct STRU_SENDMSG_RQ
{
    STRU_SENDMSG_RQ()
    {
        m_nType = DEF_PACK_SENDMSG_RQ;
        m_userid = 0;
        m_Touserid = 0;
        memset(msg,0,sizeof(msg));
    }
    PackType m_nType;
    int m_userid;
    int m_Touserid;
    char msg[MAX_MGS_SIZE];
}STRU_SENDMSG_RQ;

typedef struct STRU_OFFLINE_RQ
{
    STRU_OFFLINE_RQ()
    {
        m_nType = DEF_PACK_OFFLINE_RQ;
        m_userid = 0;
    }
    PackType m_nType;
    int m_userid;
}STRU_OFFLINE_RQ;

//上传文件请求
typedef struct STRU_UPLOAD_RQ
{
    STRU_UPLOAD_RQ()
    {
        m_nType = DEF_PACK_UPLOAD_RQ;
        m_UserId = 0;
        m_friendId = 0;
        m_nFileSize = 0;
        memset( m_szFileMD5 , 0 , MAX_SIZE);
        memset(m_szFileName , 0 ,MAX_PATH);
    }
    PackType m_nType; //包类型
    int m_UserId; //用于查数据库, 获取用户名字, 拼接路径
    int m_friendId; //对方的id

    int64_t m_nFileSize; //文件大小, 用于文件传输结束
    char m_szFileMD5[MAX_SIZE];
    char m_szFileName[MAX_PATH]; //文件名, 用于存储文件

}STRU_UPLOAD_RQ;

//上传文件请求回复
typedef struct STRU_UPLOAD_RS
{
    STRU_UPLOAD_RS()
    {
        m_nType = DEF_PACK_UPLOAD_RS;
        m_UserId= 0 ;
        m_friendId = 0;
        memset( m_szFileMD5 , 0 , MAX_SIZE);
        m_nResult = 0;
    }
    PackType m_nType; //包类型
    int m_nResult;
    int m_UserId; //用于查数据库, 获取用户名字, 拼接路径
    int m_friendId; //对方的id
    char m_szFileMD5[MAX_SIZE];
}STRU_UPLOAD_RS;

//文件块请求
typedef struct STRU_FILEBLOCK_RQ
{
    STRU_FILEBLOCK_RQ()
    {
        m_nType = DEF_PACK_FILEBLOCK_RQ;
        m_nUserId = 0;
        m_friendId = 0;
        memset( m_szFileMD5 , 0 , MAX_SIZE);
        m_nBlockLen =0;
        memset(m_szFileContent, 0 , MAX_CONTENT_LEN);
    }
    PackType m_nType; //包类型
    int m_nUserId; //用户 ID
    int m_friendId; //对方的id
    char m_szFileMD5[MAX_SIZE]; // 文件块身份标识
    int m_nBlockLen; //文件写入大小
    char m_szFileContent[MAX_CONTENT_LEN];

}STRU_FILEBLOCK_RQ;
//删除好友请求
typedef struct STRU_DELFRIEND_RQ
{
    STRU_DELFRIEND_RQ()
    {
        m_nType = DEF_PACK_DEL_FRIEND_RQ;
        m_userid = 0;
        m_frid = 0;
    }
    PackType m_nType;
    int m_userid;
    int m_frid;
}STRU_DELFRIEND_RQ;

typedef struct STRU_DELFRIEND_RS
{
    STRU_DELFRIEND_RS()
    {
        m_nType = DEF_PACK_DEL_FRIEND_RS;
        del_userid = 0;
    }
    PackType m_nType;
    int del_userid;
}STRU_DELFRIEND_Rs;

typedef struct STRU_ALTER_USERINFO_RQ
{
    STRU_ALTER_USERINFO_RQ()
    {
        m_nType = DEF_PACK_ALTER_USERINFO_RQ;
        m_userid = 0;
        m_iconid = 0;
        memset(m_szName,0,MAX_SIZE);
        memset(m_szFeeling,0,MAX_SIZE);
    }
    PackType m_nType;
    int m_userid;
    int m_iconid;
    char m_szName[MAX_SIZE];
    char m_szFeeling[MAX_SIZE];

}STRU_ALTER_USERINFO_RQ;

typedef struct STRU_ALTER_USERINFO_RS
{
    STRU_ALTER_USERINFO_RS()
    {
        m_nType = DEF_PACK_ALTER_USERINFO_RS;
        m_iconid = 0;
        memset(m_szName,0,MAX_SIZE);
        memset(m_szFeeling,0,MAX_SIZE);
    }
    PackType m_nType;
    int m_iconid;
    char m_szName[MAX_SIZE];
    char m_szFeeling[MAX_SIZE];
}STRU_ALTER_USERINFO_RS;

//创建群聊请求
typedef struct STRU_CREATE_GROUP_RQ
{
    STRU_CREATE_GROUP_RQ()
    {
        m_nType = DEF_PACK_CREATEGROUP_RQ;
        m_userid = 0;
        memset(m_szGroupName,0,sizeof(m_szGroupName));
    }
    PackType m_nType;
    int m_userid;
    char m_szGroupName[MAX_SIZE];
}STRU_CREATE_GROUP_RQ;
typedef struct STRU_CREATE_GROUP_RS
{
    STRU_CREATE_GROUP_RS()
    {
        m_nType = DEF_PACK_CREATEGROUP_RS;
        m_groupid = 0;
        memset(m_szGroupName,0,sizeof (MAX_SIZE));
    }
    PackType m_nType;
    int m_groupid;
    char m_szGroupName[MAX_SIZE];
}STRU_CREATE_GROUP_RS;

typedef struct STRU_GET_GROUP_INFO_RQ
{
    STRU_GET_GROUP_INFO_RQ()
    {
        m_nType = DEF_PACK_GETGROUPINFO_RQ;
        m_groupid = 0;
        m_iconid = 0;
        memset(m_szGroupName,0,MAX_SIZE);
        memset(rightarr,2,sizeof(rightarr));
    }
    PackType m_nType;
    int m_groupid;
    char m_szGroupName[MAX_SIZE];
    int m_iconid;
    STRU_USER_INFO m_userInfo[100];
    int rightarr[100];
}STRU_GET_GROUP_INFO_RQ;

typedef struct STRU_SENDGROUPMSG_RQ
{
    STRU_SENDGROUPMSG_RQ()
    {
        m_nType = DEF_PACK_SENDGROUPMSG_RS;
        m_groupid = 0;
        m_userid = 0;
        memset(m_szMsg,0,sizeof(m_szMsg));
    }
    PackType m_nType;
    int m_groupid;
    int m_userid;
    char m_szMsg[MAX_SIZE];
}STRU_SENDGROUPMSG_RQ;
//获取热搜请求
typedef struct STRU_GETHOTPOINT_RQ
{
    STRU_GETHOTPOINT_RQ()
    {
        m_nType = DEF_PACK_GETHOT_POINT_RQ;
    }
    PackType m_nType;

}STRU_GETHOTPOINT_RQ;
typedef struct hotP_t
{
    hotP_t()
    {
        memset(sz_title,0,MAX_SIZE);
        memset(sz_url,0,MAX_PATH);
    }
    char sz_title[MAX_SIZE];
    char sz_url[MAX_PATH];
}hotP_t;
typedef struct STRU_GETHOTPOINT_RS
{
    STRU_GETHOTPOINT_RS()
    {
        m_nType = DEF_PACK_GETHOT_POINT_RS;
    }
    PackType m_nType;
    hotP_t m_hotres[50];

}STRU_GETHOTPOINT_RS;
#endif
