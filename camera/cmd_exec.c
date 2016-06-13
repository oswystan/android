/*
 **************************************************************************************
 *       Filename:  cmd_exec.c
 *    Description:   source file
 *
 *        Version:  1.0
 *        Created:  2014-11-06 23:17:19
 *
 *       Revision:  initial draft;
 **************************************************************************************
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define log(fmt, ...)  printf(""fmt"\n", ##__VA_ARGS__);
#define logd(fmt, ...) printf("[D/%s,%d]"fmt"\n", __func__, __LINE__, ##__VA_ARGS__);
#define logi(fmt, ...) printf("[I/%s,%d]"fmt"\n", __func__, __LINE__, ##__VA_ARGS__);
#define logw(fmt, ...) printf("[W/%s,%d]"fmt"\n", __func__, __LINE__, ##__VA_ARGS__);
#define loge(fmt, ...) printf("[E/%s,%d]"fmt"\n", __func__, __LINE__, ##__VA_ARGS__);
#define logfunc()      printf("[D/%s,%d]enter\n", __func__, __LINE__);

#define BUILD_CMD(name) {#name, cmd_##name}
#define MAX_ARGC 32

typedef struct _cmd_handler_t 
{
    char* cmd;
    int (*handler)(int argc, char* argv[]);
} cmd_handler_t;

static void str_array_free(int argc, char* argv[])
{
    int i = 0;
    for(; i<argc; i++)
    {
        free(argv[i]);
        argv[i] = 0;
    }
}
static void str_split(char* str, char d, int* argc, char* argv[])
{
    char* p = str;
    char* s = str;
    int count = 0;
    while(*p)
    {
        if(*p == d && s < p)
        {
            argv[count] = strndup(s, p-s);
            s = p + 1;
            count++;
        }
        p++;
    }
    if(s < p)
    {
        argv[count] = strndup(s, p-s);
        count++;
    }
    *argc = count;
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
static dump_options(int argc, char* argv[])
{
    int i=0;
    for(; i<argc; ++i)
    {
        log("argv[%02d]=%s", i, argv[i]);
    }
}

int cmd_acquire_camera(int argc, char* argv[])
{
    logfunc();
    dump_options(argc, argv);
    return 0;
}
int cmd_usecase_config(int argc, char* argv[])
{
    logfunc();
    dump_options(argc, argv);
    return 0;
}
int cmd_request(int argc, char* argv[])
{
    logfunc();
    dump_options(argc, argv);
    return 0;
}

cmd_handler_t g_cmds[] = 
{
    BUILD_CMD(acquire_camera),
    BUILD_CMD(usecase_config),
    BUILD_CMD(request),
};

static cmd_handler_t* find_cmd(char* name)
{
    int i = 0;
    for(i=0; i<sizeof(g_cmds)/sizeof(g_cmds[0]); i++)
    {
        if(strcmp(name, g_cmds[i].cmd) == 0)
        {
            return &g_cmds[i];
        }
    }
    return NULL;
}

static int do_cmd(char* cmd)
{
    int argc = 0;
    int ret  = 0;
    char* argv[MAX_ARGC];
    str_split(cmd, ' ', &argc, argv);    
    
    cmd_handler_t* c = find_cmd(argv[0]);
    if(c)
    {
        ret = c->handler(argc, argv);
    }
    else
    {
        loge("unknown command: %s", argv[0]);
        ret = -1;
    }
    str_array_free(argc, argv);
    return ret;
}

static int parse_cmd_file(char* fname)
{
    char* str_cmd = NULL;
    size_t read = 0;
    size_t len  = 0;
    FILE* fp = fopen(fname, "r");
    if(NULL == fp)
    {
        loge("fail to open [%s]", fname);
        return -1;
    }

    while((read = getline(&str_cmd, &len, fp)) != -1)
    {
        char* p = str_trim(str_cmd);
        if(*p == '#' || *p == '\0')
        {
            continue;
        }
        if (do_cmd(p) != 0)
        {
            break;
        }
    }

    free(str_cmd);
    return 0;
}

int main(int argc, char *argv[])
{
    logfunc();
    return parse_cmd_file(argv[1]);
}

/********************************** END **********************************************/

