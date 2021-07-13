# QQ_server

#创建用户表
create table t_user (user_id  int unsigned auto_increment,user_account varchar(11) not null,user_pwd varchar(20) not null,primary key(user_id));

#创建用户信息表
create table t_userInfo (user_id int unsigned not null,icon_id int unsigned not null,user_name varchar(12)
 not null,felling varchar(25) ,status int unsigned not null,primary key(user_id));


