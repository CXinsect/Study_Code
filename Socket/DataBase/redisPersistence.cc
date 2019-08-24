#include "redisPersistence.h"

void Persistence::rdbSave (const std::string filename) {
    struct sigaction act , act1;
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set,SIGUSR2);
    sigprocmask(SIG_BLOCK,&set,NULL);
    pid_t pid = fork();
    son_pid_ = pid;
    if(pid > 0) {
        //接收来自于子进程的信号
    } else {
        char buf[1024] = {0};
        std::string tmp = getcwd(buf,sizeof(buf));
        assert(tmp.c_str() != NULL);
        tmp += "/1.rdb";
        std::cout << "tmp: The File Path: " << tmp << std::endl;
        std::ofstream out;
        //首先打开文件
        out.open(tmp,std::ios::app | std::ios::out);

        //如果字符串对象不为空便开始写入过程
        if(database_->getKeySpaceListObject().size() != 0) {
           //首先判断字符串对象，实现对字符串对象的保存。
            StringIterator_ it = database_->getKeySpaceStringObject().begin();
            while(it != database_->getKeySpaceStringObject().end()) {   
                snprintf(buf,sizeof(buf),"%s%s%ld%s%lld%hd%d%s%d%s%d%ld\r\n",FixedStructure.c_str(),DataBase.c_str(),databaseNum_,
                                ExpireTime.c_str(),it->first.second,RedisRdbTypeString,(int)it->first.first.size(),
                                it->first.first.c_str(),(int)it->second.size(),it->second.c_str(),Eof,CheckSum);
                std::cout << "Rdb Structure: " << buf << std::endl;
                out.write(buf,strlen(buf)+1);
                memset(buf,0,sizeof(buf));
                it++;
            }
        }
        else if(database_->getKeySpaceHashObject().size() != 0){
            //保存哈希对象
            HashIterator_ it = database_->getKeySpaceHashObject().begin();
            memset(buf,0,sizeof(buf));
            while(it != database_->getKeySpaceHashObject().end()) {
                std::map<std::string, std::string>::iterator iter = it->second.begin();
                while(iter != it->second.end()) {
                    snprintf(buf,sizeof(buf),"%s%s%ld%s%lld%hd%d%d%s%d%s%d%ld\r\n",FixedStructure.c_str(),DataBase.c_str(),databaseNum_,
                                ExpireTime.c_str(),it->first.second,RedisRdbTypeHash,(int)database_->getKeySpaceHashObject().size(),
                                (int)iter->first.size(),iter->first.c_str(),(int)iter->second.size(),iter->second.c_str(),Eof,CheckSum);

                    std::cout << "Hash Object: " << buf << std::endl;
                    out.write(buf,strlen(buf)+1);
                    memset(buf,0,sizeof(buf));
                    iter++;
                }
                it++;
            }
        }
        else if(database_->getKeySpaceListObject().size() != 0) {
            memset(buf,0,sizeof(buf));
            ListIteator_ it = database_->getKeySpaceListObject().begin();
            while(it != database_->getKeySpaceListObject().end()) {
                std::list<std::string>::iterator iter = it->second.begin();
                char tmp[4*1024] = {0};
                char *ptr = tmp;
                int nwrite = 0,sum = 0;
                while(iter != it->second.end()) {
                    nwrite = snprintf(ptr+sum,sizeof(tmp)-nwrite,"%d%s",(int)((*iter).size()),(*iter).c_str());
                    sum += nwrite;
                    iter++;
                }
                snprintf(buf,sizeof(buf),"%s%s%ld%s%lld%hd%s%d%s%d%ld\r\n",FixedStructure.c_str(),DataBase.c_str(),
                                         databaseNum_,ExpireTime.c_str(),it->first.second,RedisRdbTypeList,
                                         it->first.first.c_str(),(int)it->second.size(),tmp,Eof,CheckSum);
                memset(buf,0,sizeof(buf));
                it++;
            }
        }
        else {
            std::cout << "Rdb saved Error" << std::endl;
            return;
        }
    }
}