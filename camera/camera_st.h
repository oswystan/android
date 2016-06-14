/*
 **************************************************************************************
 *       Filename:  camera_st.h
 *    Description:   header file
 *
 *        Version:  1.0
 *        Created:  2016-06-14 09:59:38
 *
 *       Revision:  initial draft;
 **************************************************************************************
 */

#ifndef CAMERA_ST_H_INCLUDED
#define CAMERA_ST_H_INCLUDED

#include <stdio.h>
#include <string>
#include <vector>

#define log(fmt, ...)  printf("" fmt "\n", ##__VA_ARGS__);
#define logd(fmt, ...) printf("[D/" LOG_TAG "]" fmt "\n", ##__VA_ARGS__)
#define logi(fmt, ...) printf("[I/" LOG_TAG "]" fmt "\n", ##__VA_ARGS__)
#define logw(fmt, ...) printf("[W/" LOG_TAG "]" fmt "\n", ##__VA_ARGS__)
#define loge(fmt, ...) printf("[E/" LOG_TAG "]" fmt "\n", ##__VA_ARGS__)
#define logfunc()      printf("[D/%s,%d]enter\n", __FUNCTION__, __LINE__)

struct stc_t;
typedef int (*cmd_func)(stc_t* stc, std::vector<std::string>& arg);

struct cmd_handler_t {
    const char* cmd;
    cmd_func    fn;
};

struct command_t {
    std::string              cmd;
    std::vector<std::string> arg;
    int                      line;
};

struct stc_t {
    ~stc_t() {
        for (unsigned int i = 0; i <cmds.size(); i++) {
            delete cmds[i];
        }
        cmds.clear();
    }
    std::string             name;
    int                     cnt;
    std::vector<command_t*> cmds;
    void*                   priv;
};

struct st_file_t {
    ~st_file_t() {
        for (unsigned i = 0; i < stcs.size(); i++) {
            delete stcs[i];
        }
        stcs.clear();
    }
    std::string             fname;
    std::vector<stc_t*>     stcs;
};

#endif /*CAMERA_ST_H_INCLUDED*/

/********************************** END **********************************************/

