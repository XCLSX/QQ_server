#include "RedisTools.h"
#include "RedisConfig.h"
#include <stdio.h>
#include <string.h>
#include <sstream>
#include <stdlib.h>

RedisTool::RedisTool()
{
    m_redis = NULL;
    init();
}
RedisTool::~RedisTool()
{
    if(m_redis != NULL)
    {
        redisFree(m_redis);//析构函数释放资源
        cout << "~RedisTool :: free redis connection " << endl;
    }
}

void RedisTool::init()
{
    struct timeval timeout = { 1, 500000 }; // 1.5 seconds 设置连接等待时间
    char ip[255];
    strcpy(ip, m_config.getRedisIP().c_str());
    cout << "init : ip = " << ip << endl;
    m_redis = redisConnectWithTimeout(ip, m_config.getRedisPort(), timeout);//建立连接
    if (m_redis->err)
    {
        printf("RedisTool : Connection error: %s\n", m_redis->errstr);
    }
    else
    {
        cout << "init redis tool success " << endl;
        //REDIS_REPLY响应的类型type
        cout << "#define REDIS_REPLY_STRING 1"<< endl;
        cout << "#define REDIS_REPLY_ARRAY 2"<< endl;
        cout << "#define REDIS_REPLY_INTEGER 3"<< endl;
        cout << "#define REDIS_REPLY_NIL 4"<< endl;
        cout << "#define REDIS_REPLY_STATUS 5"<< endl;
        cout << "#define REDIS_REPLY_ERROR 6"<< endl;
    }
}

//向数据库写入string类型数据
int RedisTool::setString(string key, string value)
{
    if(m_redis == NULL || m_redis->err)//int err; /* Error flags, 错误标识，0表示无错误 */
    {
        cout << "Redis init Error !!!" << endl;
        init();
        return -1;
    }
    redisReply *reply;
    reply = (redisReply *)redisCommand(m_redis,"SET %s %s", key.c_str(), value.c_str());//执行写入命令
   // cout<<"set string type = "<<reply->type<<endl;//获取响应的枚举类型
    int result = 0;
    if(reply == NULL)
    {
        redisFree(m_redis);
        m_redis = NULL;
        result = -1;
        cout << "set string fail : reply->str = NULL " << endl;
        //pthread_spin_unlock(&m_redis_flock);
        return -1;
    }
    else if(strcmp(reply->str, "OK") == 0)//根据不同的响应类型进行判断获取成功与否
    {
        result = 1;
    }
    else
    {
        result = -1;
        cout << "set string fail :" << reply->str << endl;
    }
    freeReplyObject(reply);//释放响应信息

    return result;
}

//从数据库读出string类型数据
string RedisTool::getString(string key)
{
    if(m_redis == NULL || m_redis->err)
    {
        cout << "Redis init Error !!!" << endl;
        init();
        return NULL;
    }
    redisReply *reply;
    reply = (redisReply *)redisCommand(m_redis,"GET %s", key.c_str());
   // cout<<"get string type = "<<reply->type<<endl;

    if(reply == NULL)
    {
        redisFree(m_redis);
        m_redis = NULL;
        cout << "ERROR getString: reply = NULL!!!!!!!!!!!! maybe redis server is down" << endl;
        return NULL;
    }
    else if(reply->len <= 0)
    {
        freeReplyObject(reply);
        return NULL;
    }
    else
    {
        stringstream ss;
        ss << reply->str;
        freeReplyObject(reply);
        return ss.str();
    }
}
void RedisTool::RemoveKey(string key)
{
    if(m_redis == NULL || m_redis->err)
    {
        cout << "Redis init Error !!!" << endl;
        init();
        return;
    }
    redisReply *reply;

    reply = (redisReply*)redisCommand(m_redis,"DEL %s", key.c_str());

    freeReplyObject(reply);
}

//向数据库写入vector（list）类型数据
int RedisTool::AddOneItemToList(string key,string value)
{
    if(m_redis == NULL || m_redis->err)
    {
        cout << "Redis init Error !!!" << endl;
        init();
        return -1;
    }

    redisReply *reply;

    int result = 0;

    {
        reply = (redisReply*)redisCommand(m_redis,"RPUSH %s %s", key.c_str(),value.c_str());
    //    cout<<"set list type = "<<reply->type<<" ";
        int old = reply->integer;
        if(reply == NULL)
        {
            redisFree(m_redis);
            m_redis = NULL;
            result = -1;
            cout << "set list fail : reply->str = NULL " << endl;
            //pthread_spin_unlock(&m_redis_flock);
        }
        else if(reply->integer == old++)
        {
            result = 1;
            cout<<"add list ok"<<endl;
        }
        else
        {
            result = -1;
            cout << "set list fail ,reply->integer = " << reply->integer << endl;
        }

    }

    freeReplyObject(reply);
    cout<<"add List  success"<<endl;
    return result;

}

//向数据库写入vector（list）类型数据
int RedisTool::setList(string key,vector<string> value)
{
    if(m_redis == NULL || m_redis->err)
    {
        cout << "Redis init Error !!!" << endl;
        init();
        return -1;
    }

    redisReply *reply;

    int valueSize = value.size();
    int result = 0;

    redisCommand(m_redis,"DEL %s", key.c_str());
    for(int i=0; i<valueSize; i++)
    {
        reply = (redisReply*)redisCommand(m_redis,"RPUSH %s %s", key.c_str(),value.at(i).c_str());
      //  cout<<"set list type = "<<reply->type<<" ";
        int old = reply->integer;
        if(reply == NULL)
        {
            redisFree(m_redis);
            m_redis = NULL;
            result = -1;
            cout << "set list fail : reply->str = NULL " << endl;
            //pthread_spin_unlock(&m_redis_flock);
            return -1;
        }
        else if(reply->integer == old++)
        {
            result = 1;
            cout<<"rpush list ok"<<endl;
            continue;
        }
        else
        {
            result = -1;
            cout << "set list fail ,reply->integer = " << reply->integer << endl;
            return -1;
        }

    }

    freeReplyObject(reply);
    cout<<"set List  success"<<endl;
    return result;

}

//从数据库读出vector（list）类型数据
vector<string> RedisTool::getList(string key)
{

    if(m_redis == NULL || m_redis->err)
    {
        cout << "Redis init Error !!!" << endl;
        init();
        return vector<string> {}; //返回空的向量
    }

    redisReply *reply;
    reply = (redisReply*)redisCommand(m_redis,"LLEN %s", key.c_str());
    int valueSize = reply->integer;
  //  cout<<"List size is :"<<reply->integer<<endl;

    reply = (redisReply*)redisCommand(m_redis,"LRANGE %s %d %d", key.c_str(),0,valueSize-1);
    cout<<"get list type = "<<reply->type<<endl;
    cout<<"get list size = "<<reply->elements<<endl;//对于数组类型可以用elements元素获取数组长度

    redisReply** replyVector = reply->element;//获取数组指针
    vector<string> result;
    for(int i=0; i<valueSize; i++)
    {
        stringstream ss;
        ss << (*replyVector)->str;

//       string temp =(*replyVector)->str;//遍历redisReply*数组,存入vector向量
//        int a =atoi(temp.c_str());
        result.push_back(ss.str());
        replyVector++;
    }

    cout<<"result size:"<<result.size()<<endl;
    return result;

}

void RedisTool::SetExpire(string key, int time)
{
    if(m_redis == NULL || m_redis->err)
    {
        cout << "Redis init Error !!!" << endl;
        init();
    }
    redisCommand(m_redis,"expire %s %d", key.c_str() , time );

}


void RedisTool::SetHashValue(string key,string field , string value)
{
    if(m_redis == NULL || m_redis->err)
    {
        cout << "Redis init Error !!!" << endl;
        init();
        return;
    }

    redisReply *reply;

    {
        reply = (redisReply*)redisCommand(m_redis,"hset %s %s %s", key.c_str(),field.c_str() ,value.c_str());
//        cout<<"Set Hash type = "<<reply->type<<" ";
    }

    freeReplyObject(reply);
    cout<<"Set Hash success"<<endl;
    return;
}

string  RedisTool::GetHashValue(string key,string field )
{

    if(m_redis == NULL || m_redis->err)
    {
        cout << "Redis init Error !!!" << endl;
        init();
        return "";
    }
    redisReply *reply;
    reply = (redisReply *)redisCommand(m_redis,"hget %s %s", key.c_str() , field.c_str() );
    //cout<<"get GetHashValue type = "<<reply->type<<endl;

    if(reply == NULL)
    {
        redisFree(m_redis);
        m_redis = NULL;
        cout << "ERROR getString: reply = NULL!!!!!!!!!!!! maybe redis server is down" << endl;

    }
    else if(reply->len <= 0)
    {
        freeReplyObject(reply);
    }
    else
    {
        stringstream ss;
        ss << reply->str;
        freeReplyObject(reply);

        return ss.str();
    }
    return "";
}

bool RedisTool::SetmHashValue(string Key, string fields)
{
    if(m_redis == NULL || m_redis->err)
    {
        cout << "Redis init Error !!!" << endl;
        init();
        return false;
    }

    redisReply *reply;

    string temp = "hmset "+Key+" "+fields;
    reply = (redisReply*)redisCommand(m_redis,temp.c_str());
    if(strcmp(reply->str,"OK") !=0 )
        return false;

    freeReplyObject(reply);
    return true;
}

list<string> RedisTool::GetmHashValue(string key,string fields,int field_len)
{
    if(m_redis == NULL || m_redis->err)
    {
        cout << "Redis init Error !!!" << endl;
        init();
        return {};
    }
    redisReply *reply;
    string temp = "hmget "+key+" "+fields;
    reply = (redisReply *)redisCommand(m_redis,temp.c_str());
    //reply = (redisReply *)redisCommand(m_redis,temp.c_str());



    if(reply == NULL)
    {
        redisFree(m_redis);
        m_redis = NULL;
        cout << "ERROR getString: reply = NULL!!!!!!!!!!!! maybe redis server is down" << endl;

    }
    else if(reply->elements <= 0)
    {
        freeReplyObject(reply);
    }
    else
    {
        list<string> res;
        redisReply** replyVector = reply->element;
        for(int i=0;i<field_len;i++)
        {
            stringstream ss;
            ss << (*replyVector)->str;
            res.push_back(ss.str());
            replyVector++;
        }

        freeReplyObject(reply);

        return res;
    }
    return {};
}

list<string> RedisTool::GetHashAllValue(string key)
{
    if(m_redis == NULL || m_redis->err)
    {
        cout << "Redis init Error !!!" << endl;
        init();
        return {};
    }
    redisReply *reply;
    int times = 0;
    string temp = "hlen "+key;
    reply = (redisReply *)redisCommand(m_redis,temp.c_str());
    times = reply->integer;
    temp = "hvals "+key;
    reply = (redisReply *)redisCommand(m_redis,temp.c_str());

    if(reply == NULL)
    {
        redisFree(m_redis);
        m_redis = NULL;
        cout << "ERROR getString: reply = NULL!!!!!!!!!!!! maybe redis server is down" << endl;

    }
    else if(times<=0)
    {
        freeReplyObject(reply);
    }
    else
    {
        list<string> res;
        redisReply** replyVector = reply->element;
        for(int i=0;i<times;i++)
        {
            stringstream ss;
            ss << (*replyVector)->str;
            res.push_back(ss.str());
            replyVector++;
        }

        freeReplyObject(reply);

        return res;
    }
    return {};
}
int RedisTool::isHashKeyExists(string key )
{
    if(m_redis == NULL || m_redis->err)
    {
        cout << "Redis init Error !!!" << endl;
        init();
        return NULL;
    }
    redisReply *reply;
    reply = (redisReply *)redisCommand(m_redis,"hlen %s", key.c_str() );
  //  cout<<"get GetHashValue type = "<<reply->type<<endl;

    if(reply == NULL)
    {
        redisFree(m_redis);
        m_redis = NULL;
        cout << "ERROR getString: reply = NULL!!!!!!!!!!!! maybe redis server is down" << endl;
        return NULL;
    }
    else if(reply->integer <= 0)
    {
        freeReplyObject(reply);
        return NULL;
    }
    else
    {
        return 1;
    }
}

void RedisTool::ExitRedis()
{
    if(m_redis == NULL || m_redis->err)
    {
        cout << "Redis init Error !!!" << endl;
        init();
        return ;
    }
    redisReply *reply;
    reply = (redisReply *)redisCommand(m_redis,"SHUTDOWN NOSAVE");
    redisFree(m_redis);
    m_redis = NULL;
    freeReplyObject(reply);

    return ;

}
