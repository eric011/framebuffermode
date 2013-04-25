/*
 * Copyright (C) 2009 Google Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

//#define LOG_NDEBUG 0
#define LOG_TAG "srclib.huyanwei.FrameBufferMode"

#define TAG	LOG_TAG

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <errno.h>
#include <string.h>

#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/mount.h>
#include <sys/statfs.h>

#include <linux/fb.h>
#include <linux/input.h>

#include <dlfcn.h>
#include <dirent.h>
#include <fcntl.h>
#include <pthread.h>
#include <math.h>

#if defined(ANDROID)
#include <utils/Log.h>
#endif

#if 1
#if defined(ALOGD)
#undef ALOGD
#endif
#define ALOGD printf
#endif

#ifdef __cplusplus 
extern "C" { 
#endif 

#ifdef __cplusplus 
}
#endif 

static int    fd_framebuffer;
static void * framebuffer_mem;
static struct fb_var_screeninfo vi;
static struct fb_fix_screeninfo fi;

void usage(const char * name)
{
	printf("\r\n%s usage :\r\n",name);
	printf("\r\nAuthor:huyanwei\r\n");
	printf("Email :srclib@hotmail.com\r\n");
	return ;
}

int open_framebuffer_device()
{
    int fd;

#if defined(ANDROID)
    fd = open("/dev/graphics/fb0", O_RDWR);
#else
    fd = open("/dev/fb0", O_RDWR);
#endif

    if (fd < 0) {
        perror("cannot open framebuffer\n");
        return -1;
    }

    if (ioctl(fd, FBIOGET_VSCREENINFO, &vi) < 0) {
        perror("failed to get framebuffer var screen info\n");
        close(fd);
        return -1;
    }

	printf("\nhuyanwei debug fb_var_screeninfo:\n");
	printf("==================================================\n");
	printf("vi.xres\t\t\t=0x%08x(%d)\n", vi.xres, vi.xres);
	printf("vi.yres\t\t\t=0x%08x(%d)\n", vi.yres, vi.yres);
	printf("vi.xres_virtual\t\t=0x%08x(%d)\n", vi.xres_virtual, vi.xres_virtual);
	printf("vi.yres_virtual\t\t=0x%08x(%d)\n", vi.yres_virtual,vi.yres_virtual);
	printf("vi.xoffset\t\t=0x%08x(%d)\n", vi.xoffset,vi.xoffset);
	printf("vi.yoffset\t\t=0x%08x(%d)\n", vi.yoffset,vi.yoffset);

	printf("vi.bits_per_pixel\t=0x%08x(%d)\n", vi.bits_per_pixel, vi.bits_per_pixel);
	printf("vi.red.offset\t\t=0x%08x(%d)\n", vi.red.offset, vi.red.offset);
	printf("vi.red.length\t\t=0x%08x(%d)\n", vi.red.length, vi.red.length);
	printf("vi.green.offset\t\t=0x%08x(%d)\n", vi.green.offset,vi.green.offset);
	printf("vi.green.length\t\t=0x%08x(%d)\n", vi.green.length,vi.green.length);
	printf("vi.blue.offset\t\t=0x%08x(%d)\n", vi.blue.offset,vi.blue.offset);
	printf("vi.blue.length\t\t=0x%08x(%d)\n", vi.blue.length,vi.blue.length);
	printf("vi.transp.offset\t=0x%08x(%d)\n", vi.transp.offset,vi.transp.offset);
	printf("vi.transp.length\t=0x%08x(%d)\n", vi.transp.length,vi.transp.length);
	printf("vi.activate\t\t=0x%08x(%d)\n", vi.activate,vi.activate);
	printf("vi.left_margin\t\t=0x%08x(%d)\n", vi.left_margin,vi.left_margin);
	printf("vi.right_margin\t\t=0x%08x(%d)\n", vi.right_margin,vi.right_margin);
	printf("vi.upper_margin\t\t=0x%08x(%d)\n", vi.upper_margin,vi.upper_margin);
	printf("vi.lower_margin\t\t=0x%08x(%d)\n", vi.lower_margin,vi.lower_margin);
	printf("vi.hsync_len\t\t=0x%08x(%d)\n", vi.hsync_len,vi.hsync_len);
	printf("vi.vsync_len\t\t=0x%08x(%d)\n", vi.vsync_len,vi.vsync_len);
	printf("vi.sync\t\t\t=0x%08x(%d)\n", vi.sync,vi.sync);
	printf("vi.height\t\t=0x%08x(%d)\n", vi.height,vi.height);
	printf("vi.width\t\t=0x%08x(%d)\n", vi.width,vi.width);
#if defined(ANDROID)
	printf("vi.colorspace\t\t=0x%08x(%d)\n", vi.colorspace,vi.colorspace);
#endif

	printf("==================================================\n\n");

    if (ioctl(fd, FBIOGET_FSCREENINFO, &fi) < 0) {
        perror("failed to get framebuffer fix screen info\n");
        close(fd);
        return -1;
    }

	printf("huyanwei debug fb_var_screeninfo:\n");
	printf("==================================================\n");
	printf("fi.id\t\t\t=%s\n",fi.id);
	printf("fi.smem_start\t\t=0x%08x(%d)\n",fi.smem_start,fi.smem_start);
	printf("fi.smem_len\t\t=0x%08x(%d)\n",fi.smem_len,fi.smem_len);
	printf("fi.type\t\t\t=0x%08x(%d)\n",fi.type,fi.type);
	printf("fi.type_aux\t\t=0x%08x(%d)\n",fi.type_aux,fi.type_aux);
	printf("fi.line_length\t\t=0x%08x(%d)\n",fi.line_length,fi.line_length);
	printf("==================================================\n\n");

    framebuffer_mem = mmap(0, fi.smem_len, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (framebuffer_mem == MAP_FAILED) {
        perror("failed to mmap framebuffer \n");
        close(fd);
        return -1;
    }

	fd_framebuffer = fd ;

	return fd;
}

int close_framebuffer()
{
	if(framebuffer_mem != MAP_FAILED)
	{
		munmap(framebuffer_mem,fi.smem_len);
	}
	close(fd_framebuffer);
	fd_framebuffer = -1;

	return 0;
}

#define MIN(A,B) ((A<B)?(A):(B))
#define MAX(A,B) ((A>B)?(A):(B))

int fill_framebuffer( int id ,int x , int y , int w,int h , int color)
{
	int width  = vi.xres_virtual;
	int height = vi.yres;

	unsigned char A = (color >> 24) & 0xff ;
	unsigned char R = (color >> 16) & 0xff ;
	unsigned char G = (color >> 8)  & 0xff ;
	unsigned char B = (color )  	& 0xff ;

	int start_x = MAX(x,0);
	int end_x   = MIN((x+w),width);
	int start_y = MAX(y,0);
	int end_y   = MIN((y+h),height);

	// printf("start_x=%d,start_y=%d,end_x=%d,end_y=%d\n",start_x,start_y,end_x,end_y);

	unsigned short u16_color = ((R & 0xf8) << 8) | ((G & 0xf8) << 2) | ((B & 0xf8) >> 3 ) ;

	char * fb_mem = (char *)framebuffer_mem ;
	char * fb_start_row = (char *)framebuffer_mem;


	if(id = 0)
	{
			fb_mem = (char *)framebuffer_mem ;
	}
	else
	{
			fb_mem = (char *)framebuffer_mem + (vi.yres * fi.line_length);
	}


	int col_no = 0 ;
	int row_no = 0 ;

	while(row_no < ( end_y -start_y))
	{
#if defined(ANDROID)
		fb_start_row = ((char *) fb_mem) + ((start_y+row_no) * (width*2));
		col_no = 0 ;
		while(col_no < (end_x - start_x))
		{
		    memset(fb_start_row + ((start_x + col_no)* 2)+0,(u16_color & 0xff),1);
		    memset(fb_start_row + ((start_x + col_no)* 2)+1,(u16_color & 0xff00) >> 8,1);
			col_no ++;
		}
		row_no ++;
#else
		fb_start_row = ((char *) fb_mem) + ((start_y+row_no) * (width*4));
		col_no = 0 ;
		while(col_no < (end_x - start_x))
		{
		    memset(fb_start_row + ((start_x + col_no)* 4)  ,B,1);		
		    memset(fb_start_row + ((start_x + col_no)* 4)+1,G,1);		
		    memset(fb_start_row + ((start_x + col_no)* 4)+2,R,1);		
		    memset(fb_start_row + ((start_x + col_no)* 4)+3,A,1);		
			col_no ++;
		}
		row_no ++;
#endif
	}

	return 0;
}


int active_framebuffer(int id)
{
#if defined(ANDROID)
    //vi.yres_virtual = vi.yres * 2;
	vi.yoffset      = vi.yres * id ;
#endif
	vi.activate     = FB_ACTIVATE_VBL ; //vi.activate     = FB_ACTIVATE_NOW ;

	if (ioctl(fd_framebuffer, FBIOPUT_VSCREENINFO, &vi) < 0) 
	{
        perror("active fb swap failed");
    }
	return 0;
}


int main(int argc, char **argv)
{
	int i = 0 ;
	open_framebuffer_device();


	while( i < 300 )
	{
#if defined(ANDROID)
		fill_framebuffer(0,0,0,vi.xres,vi.yres,0x00000000);
		fill_framebuffer(0,00+i,100,160,120,0x00FF0000);
		fill_framebuffer(0,20+i,150,160,120,0x0000FF00);
		fill_framebuffer(0,40+i,200,160,120,0x000000FF);
		fill_framebuffer(0,60+i,250,160,120,0x00FFFF00);
		fill_framebuffer(0,80+i,300,160,120,0x0000FFFF);
		fill_framebuffer(0,99+i,350,160,120,0x00FF00FF);

		active_framebuffer(0);  // must switch 1 times. 

		sleep(1);

		fill_framebuffer(1,0,0,vi.xres,vi.yres,0x00000000);
		fill_framebuffer(1,00+i,350,160,120,0x00FF0000);
		fill_framebuffer(1,20+i,300,160,120,0x0000FF00);
		fill_framebuffer(1,40+i,250,160,120,0x000000FF);
		fill_framebuffer(1,60+i,200,160,120,0x00FFFF00);
		fill_framebuffer(1,80+i,150,160,120,0x0000FFFF);
		fill_framebuffer(1,99+i,100,160,120,0x00FF00FF);

		active_framebuffer(1);  // must switch 1 times. 

		sleep(1);
#else
		fill_framebuffer(0,0,0,vi.xres,vi.yres,0x00000000);
		fill_framebuffer(0,100+i,100,600,400,0x00FF0000);
		fill_framebuffer(0,150+i,150,600,400,0x0000FF00);
		fill_framebuffer(0,200+i,200,600,400,0x000000FF);
		fill_framebuffer(0,250+i,250,600,400,0x00FFFF00);
		fill_framebuffer(0,300+i,300,600,400,0x0000FFFF);
		fill_framebuffer(0,350+i,350,600,400,0x00FF00FF);

		active_framebuffer(0);  // must switch 1 times. 

		sleep(1);
#endif
		i = i+3 ;
	}
	close_framebuffer();
	return 0;
}




