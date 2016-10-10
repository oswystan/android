/*
 **************************************************************************************
 *       Filename:  read_id.c
 *    Description:   source file
 *
 *        Version:  1.0
 *        Created:  2016-09-12 11:59:48
 *
 *       Revision:  initial draft;
 **************************************************************************************
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

unsigned char read_reg(int cid, unsigned short addr) {
    unsigned char reg = 0;
    char buf[64];
    char fn[256];
    sprintf(fn, "/sys/devices/soc/a0c000.qcom,cci/a0c000.qcom,cci:qcom,camera@%d/sensor_register_debug", cid);
    chmod(fn, 0666);
    sprintf(buf, "r 0x%x 1", addr);
    int fp = open(fn, O_RDWR);
    if (fp < 0) {
        printf("fail to open file %s\n", fn);
        return -1;
    }
    int ret = write(fp, buf, sizeof(buf));
    if (ret < 0) {
        printf("fail to write cmd when read reg\n");
        reg = -1;
        goto OUT;
    }

    lseek(fp, 0, SEEK_SET);
    ret = read(fp, buf, sizeof(buf));
    if (ret < 0) {
        printf("fail to read reg of addr 0x%04X\n", addr);
        reg = -1;
        goto OUT;
    }
    sscanf(buf, "%x %x", &addr, &reg);

OUT:
    close(fp);
    return reg;
}

void write_reg(int cid, unsigned short addr, unsigned char reg) {
    char buf[64];
    char fn[256];
    sprintf(fn, "/sys/devices/soc/a0c000.qcom,cci/a0c000.qcom,cci:qcom,camera@%d/sensor_register_debug", cid);
    chmod(fn, 0666);
    sprintf(buf, "w 0x%x 0x%x", addr, reg);
    int fd = open(fn, O_RDWR);
    if (fd < 0) {
        printf("fail to open file %s\n", fn);
        return;
    }

    int ret = write(fd, buf, sizeof(buf));
    if (ret < 0) {
        printf("fail to write addr 0x%04X [%s]\n", addr, strerror(errno));
        goto OUT;
    }
OUT:
    close(fd);
}

int main(int argc, const char *argv[]) {
    char fn[256];
    unsigned char production_id[16];
    int cid = 0;
    if (argc == 2) {
        cid = atoi(argv[1]);
    }

    /* set manual mode */
    unsigned char tmp = read_reg(cid, 0x5002);
    tmp = tmp & (~0x02);
    write_reg(cid, 0x5002, tmp);
    tmp = read_reg(cid, 0x5002);

    /* set reg start and end addr */
    write_reg(cid, 0x3d84, 0xC0);
    write_reg(cid, 0x3d88, 0x60); // OTP start address
    write_reg(cid, 0x3d89, 0x00);
    write_reg(cid, 0x3d8A, 0x60); // OTP end address
    write_reg(cid, 0x3d8B, 0x0F);

    /* clear data regs */
    unsigned short base = 0;
    for (base = 0x6000; base <= 0x600F; base++) {
        write_reg(cid, base, 0x00);
    }

    /* load otp data to regs */
    write_reg(cid, 0x3d81, 0x01);
    usleep(10000);

    /* read production id */
    unsigned i = 0;
    for (base = 0x6000; base <= 0x600F; base++) {
        production_id[i] = read_reg(cid, base);
        printf("%02X", production_id[i]);
        i++;
    }
    printf("\n");

    /* clear otp buffer */
    for (base = 0x6000; base <= 0x600F; base++) {
        write_reg(cid, base, 0x00);
    }

    /* switch to non-manual mode */
    tmp = read_reg(cid, 0x5002);
    tmp = tmp | (0x02);
    write_reg(cid, 0x5002, tmp);

    return 0;
}


/********************************** END **********************************************/

