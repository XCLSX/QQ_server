//void TcpKernel::Login(int clientfd ,char* szbuf,int nlen)
//{
//    printf("clientfd:%d LoginRq\n", clientfd);

//    STRU_LOGIN_RQ * rq = (STRU_LOGIN_RQ *)szbuf;
//    STRU_LOGIN_RS rs;
//    list<string> ls;
//    char szsql[_DEF_SQLIEN] = {0};
//    sprintf(szsql,"select user_id,user_pwd from t_user where user_account = '%s';",rq->m_szAccount);
//    if(!m_sql->SelectMysql(szsql,2,ls))
//    {
//        printf("sql error:%s\n",szsql);
//    }
//    if(ls.size()==0)    //用户不存在
//    {
//        rs.m_lResult = userid_no_exist;
//        m_tcp->SendData( clientfd , (char*)&rs , sizeof(rs) );
//        return ;
//    }
//    int user_id = atoi(ls.front().c_str());     ls.pop_front();
//    if(strcmp(rq->m_szPwd,ls.front().c_str())!=0)           //密码错误
//    {
//        rs.m_lResult = password_error;
//    }
//    else
//    {
//        //修改登录状态
//        memset(szsql,0,sizeof(szsql));
//        sprintf(szsql,"update  t_user set status = 1 where user_id = %d;",user_id);
//        if(!m_sql->UpdataMysql(szsql))
//        {
//            printf("sql error:%s\n",szsql);
//        }
//        //录入映射
//        string str_userInfo;
//        UserInfo_S *userInfo = new UserInfo_S;
//        userInfo->sock_fd = clientfd;
//        ls.clear();
//        sprintf(szsql,"select icon_id,user_name,felling,status from t_user where user_id = %d;",user_id);
//        if(!m_sql->SelectMysql(szsql,4,ls))
//        {
//            printf("sql error:%s\n",szsql);
//            return ;
//        }
//        userInfo->m_user_id = user_id;


//        userInfo->icon_id = atoi(ls.front().c_str());            ls.pop_front();
//        strcpy(userInfo->m_szName,ls.front().c_str());           ls.pop_front();
//        strcpy(userInfo->m_szfelling,ls.front().c_str());        ls.pop_front();
//        userInfo->status = atoi(ls.front().c_str());
//        m_mapIdtoUserInfo[user_id] = userInfo;
//        编写回复包
//        rs.m_lResult = login_sucess;
//        rs.m_userInfo.m_icon_id = userInfo->icon_id;
//        strcpy(rs.m_userInfo.m_userName,userInfo->m_szName);
//        strcpy(rs.m_userInfo.sz_feeling,userInfo->m_szfelling);
//        rs.m_userInfo.m_status = userInfo->status;
//        rs.m_userInfo.m_user_id = user_id;
//        rs.m_lResult = login_sucess;
//        rs.m_userInfo.m_icon_id = atoi(ls.front().c_str());     ls.pop_front();
//        strcpy(rs.m_userInfo.m_userName,)

//    }

//    m_tcp->SendData( clientfd , (char*)&rs , sizeof(rs) );

//    //发送好友列表
//    PostFriList(clientfd,rs.m_userInfo.m_user_id);

//    GetOffMsg(clientfd,rs.m_userInfo.m_user_id);

//    //通知好友上线
//    bzero(szsql,sizeof (szsql));    ls.clear();
//    sprintf(szsql,"select friend_id from t_friend where user_id = %d;",user_id);
//    if(!m_sql->SelectMysql(szsql,1,ls))
//    {
//        printf("sql error:%s\n",szsql);
//        return;
//    }
//    STRU_UPDATE_STATUS sus;
//    sus.m_UserInfo = rs.m_userInfo;
//    sus.m_UserInfo.m_status = 1;
//    for(int i=0;i<ls.size();i++)
//    {
//        auto ite = m_mapIdtoUserInfo.find(atoi(ls.front().c_str()));
//        if(ite == m_mapIdtoUserInfo.end())
//        {
//            ls.pop_front();
//            continue;
//        }
//        int sockfd = (*ite).second->sock_fd;
//        m_tcp->SendData(sockfd,(char *)&sus,sizeof (sus));
//        ls.pop_front();
//    }
//}

