/*
 **************************************************************************************
 *       Filename:  camera_st.cpp
 *    Description:   source file
 *
 *        Version:  1.0
 *        Created:  2016-06-14 09:46:19
 *
 *       Revision:  initial draft;
 **************************************************************************************
 */

#define LOG_TAG "CAMERA_ST"
#include <stdio.h>
#include <stdlib.h>
#include "camera_st.h"


extern cmd_handler_t g_handlers[];

static void str_split(char* str, char d, std::vector<std::string> &cmd) {
    char* p = str;
    char* s = str;
    int count = 0;
    while(*p)
    {
        if(*p == d && s < p)
        {
            *p = '\0';
            cmd.push_back(s);
            s = p + 1;
        }
        p++;
    }
    if(s < p)
    {
        cmd.push_back(s);
    }
}
static int is_space(char c)
{
    if(c == ' ' || c == '\t' || c == '\r' || c == '\n')
    {
        return 1;
    }
    else
    {
        return 0;
    }
}
static char* str_trim(char* str)
{
    char* p = str;
    char* e = p + strlen(str) - 1;
    while(is_space(*p))
    {
        p++;
    }
    while(is_space(*e))
    {
        e--;
    }
    e[1] = '\0';
    return p;
}

static st_file_t* parse_st(const char* f) {
    FILE* fp = fopen(f, "r");
    if (NULL == fp) {
        loge("can not open file %s", f);
        return NULL;
    }

    char* str_cmd = NULL;
    std::string cmd;
    size_t read = 1;
    size_t len = 0;
    int line = 0;
    st_file_t* st = new st_file_t();
    st->fname = f;
    stc_t* stc = NULL; 

    std::vector<std::string> opt;
    while ((read = getline(&str_cmd, &len, fp)) != size_t(-1)) {
        line++;

        //skip empty line and comments
        char* p = str_trim(str_cmd);
        if ('#' == *p || '\0' == *p) {
            continue;
        }
        opt.clear();
        str_split(p, ' ', opt);
        if (opt.size() == 0) {
            continue;
        }
        
        // start a new stc
        if (opt[0] == "BEGIN") {
            //should not be in here
            if (stc) {
                delete stc;
                stc = NULL;
            }
            stc = new stc_t();
            stc->priv = NULL;
            if (opt.size() >= 2) {
                stc->name = opt[1];
            }
        } else if (opt[0] == "END") {
            // end of the current stc
            if (stc != NULL) {
                //logd("get stc %s", stc->name.c_str());
                st->stcs.push_back(stc);
                stc = NULL;
            }
        } else {
            if (NULL == stc) {
                logw("no stc found in %s@%d, skip it", st->fname.c_str(), line);
                continue;
            }
            command_t* cmd = new command_t();
            cmd->line = line;
            cmd->cmd = opt[0];
            for (unsigned i = 1; i < opt.size(); i++) {
                cmd->arg.push_back(opt[i]);
            }
            stc->cmds.push_back(cmd);
            cmd = NULL;
        }
    }

    free(str_cmd);
    return st;
}

static cmd_handler_t*  get_handler(const char* cmd) {
    for (unsigned i = 0; g_handlers[i].cmd != NULL; i++) {
        if (strcmp(cmd, g_handlers[i].cmd) == 0) {
            return &g_handlers[i];
        }
    }
    return NULL;
}
static int run_stc(stc_t* c) {
    int ret = 0;
    for (unsigned i = 0; i < c->cmds.size(); i++) {
        command_t* cmd = c->cmds[i];
        cmd_handler_t* handler = get_handler(cmd->cmd.c_str());
        //logd("run %s=>%s", c->name.c_str(), cmd->cmd.c_str());
        if (NULL == handler) {
            loge("invalid cmd: %s=>%s", c->name.c_str(), cmd->cmd.c_str());
            return -1;
        }
        ret = handler->fn(c, cmd->arg);
        if (ret != 0) {
            loge("failed to do cmd: %s=>%s", c->name.c_str(), cmd->cmd.c_str());
            return ret;
        }
    }
    return 0;
}

static int run(st_file_t* st) {
    for (unsigned i = 0; i < st->stcs.size(); i++) {
        int ret = run_stc(st->stcs[i]);
        if (0 == ret) {
            log("[" GREEN "SUCC" NONE "]%s", st->stcs[i]->name.c_str());
        }else {
            log("[" RED   "FAIL" NONE "]%s", st->stcs[i]->name.c_str());
            return ret;
        }
    }
    return 0;
}

int main(int argc, const char *argv[]) {
    if (argc != 2) {
        log("usage: %s <cmdfile>", argv[0]);
        return -1;
    }

    st_file_t* st = parse_st(argv[1]);
    if (NULL == st) {
        return -1;
    }

    int ret = run(st);
    delete st;

    return ret;
}

/********************************** END **********************************************/

