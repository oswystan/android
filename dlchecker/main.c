/*
 **************************************************************************************
 *       Filename:  main.c
 *    Description:   source file
 *
 *        Version:  1.0
 *        Created:  2017-08-12 16:53:31
 *
 *       Revision:  initial draft;
 **************************************************************************************
 */


#include <stdio.h>
#include <dlfcn.h>
#include <libgen.h>

int main(int argc, const char *argv[]) {
    if (argc != 2) {
        printf("usage: %s <file_name>\n", basename(argv[0]));
        return -1;
    }

    void* handle = dlopen(argv[1], RTLD_NOW);
    if (!handle) {
        printf("error: %s\n", dlerror());
        return -1;
    }

    dlclose(handle);
    printf("load %s OK\n", argv[1]);
    return 0;
}

/********************************** END **********************************************/

