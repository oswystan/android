/*
 **************************************************************************************
 *       Filename:  jpeg_enc.h
 *    Description:   header file
 *
 *        Version:  1.0
 *        Created:  2017-09-06 10:43:13
 *
 *       Revision:  initial draft;
 **************************************************************************************
 */

#ifndef JPEG_ENC_H_INCLUDED
#define JPEG_ENC_H_INCLUDED

int jpegenc_init(int w, int h);
int jpegenc_encode(void* addr, int in_fd, void** out, int* size);
int jpegenc_deinit();

#endif /*JPEG_ENC_H_INCLUDED*/
/********************************** END **********************************************/
