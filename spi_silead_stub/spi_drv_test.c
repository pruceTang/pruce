#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include "spi_drv_test.h"
#include <string.h>
#include <time.h>
#include <sys/stat.h>

static int fd = 0;
#define SILEAD_FP_DEV "/dev/silead_stub"
#define TEST_CMD_PRINT 101
#define TEST_CMD_INIT 102
#define TEST_CMD_DEINIT 103
#define TEST_CMD_GETDATA 104
#define TEST_CMD_SETDATA 105

#define SIL_STUB_IOCPRINT   _IO(SIFP_IOC_MAGIC, 1)
#define SIL_STUB_IOCINIT    _IO(SIFP_IOC_MAGIC, 2)
#define SIL_STUB_IOCDEINIT  _IO(SIFP_IOC_MAGIC, 3)

#define SIL_STUB_IOCGETDATA  _IOR(SIFP_IOC_MAGIC, 4, int)
#define SIL_STUB_IOCSETDATA  _IOW(SIFP_IOC_MAGIC, 5, int)
int process_ioctl_cmd(int acq_cmd,char* screen_status)
{
	int ret = -1;
	switch(acq_cmd)
	{
		case TEST_CMD_PRINT: {
		    ret = ioctl(fd, SIL_STUB_IOCPRINT, 0);
		    if(ret >= 0){
		        printf("******print success************\n");
		    }
		    else{
			  printf("******print operate failed*******ret=%d\n",ret);
		    }
		    break;
		}
		case TEST_CMD_INIT: {
		    ret = ioctl(fd, SIL_STUB_IOCINIT, 0);
		    if(ret >= 0){
		        printf("******init*******\n");
		    }
		    else{
			  printf("******init silead_fp operate failed*******ret=%d\n",ret);
		    }			
		    break;
		}
		case TEST_CMD_DEINIT: {
		    ret = ioctl(fd, SIL_STUB_IOCDEINIT, 0);
		    if(ret >= 0){
		        printf("******deinit silead_fp success************\n");
		    }
		    else{
			  printf("******deinit silead_fp operate failed*******ret=%d\n",ret);
		    }
		    break;
		}
		case TEST_CMD_GETDATA: {
		    ret = ioctl(fd, SIL_STUB_IOCGETDATA, 0);
		    if(ret >= 0){
		         printf("******get data success************\n");
		    }
		    else{
			  printf("******get data operate failed*******ret=%d\n",ret);
		    }
		    break;
		}
		case TEST_CMD_SETDATA: {
		    ret = ioctl(fd, SIL_STUB_IOCSETDATA, SIFP_PWDN_FLASH);
		    if(ret >= 0){
    		         printf("******set data success************\n");
		    }
		    else{
			  printf("******set data operate failed*******ret=%d\n",ret);
		    }
		    break;
		}
		default: {
		    printf("******unknow cmd,or just not implement!!***\n");
		    break;
		}
	}
	return ret;
}

void string_to_num(char *arg,int *cmd)
{
	*cmd = 0;
	if (!strcmp(arg,"101"))
	{	
		*cmd = 101;
	}
	else if (!strcmp(arg,"102"))
	{	
		*cmd = 102;
	}	
	else if (!strcmp(arg,"103"))
	{  
		*cmd = 103;
  	}
 	else if (!strcmp(arg,"104"))
	{  
		*cmd = 104;
  	}
 	else if (!strcmp(arg,"105"))
	{  
		*cmd = 105;
  	}
	if(0 != *cmd){
		printf("********cmd=%d********\n",*cmd);
	}
  	else
	{
		printf("unkonw cmd, or just not implement!!\n");
		*cmd = 0;
  	}
}

int main(int argc, char** argv)
{
	int ret = -1;
	char cur_screen_status = 0;
	char cmd_str[64] = {0};
	int acquire_cmd = 0;
	if (argc < 2){
		printf("*********silead stub test***********\n");
		printf("**pls select  your operation with enter**\n");
		printf("******print		101************\n");
		printf("******init		102************\n");		
		printf("******deinit	103************\n");
		printf("******get data	104************\n");
		printf("******set data	105************\n");
		printf("******quit			1000***********\n");	
		printf("*********************************\n");

		printf("******eg: ./silead_stub_test 101*********\n");
	}
	if (0==fd){
		fd = open(SILEAD_FP_DEV,O_RDWR);
	}
	if (fd < 0){
		printf("open device fail!\n");
		return 0;
	}

	printf("************you selcet operation is: %s************\n",argv[1]);
	string_to_num(argv[1], &acquire_cmd);//get first cmd by args
	if(acquire_cmd>0){
		ret = process_ioctl_cmd(acquire_cmd, &cur_screen_status);
	}
	

	do{
		printf("*********silead stub test***********\n");
		printf("**pls select  your operation with enter**\n");
		printf("******print		101************\n");
		printf("******init		102************\n");		
		printf("******deinit	103************\n");
		printf("******get data	104************\n");
		printf("******set data	105************\n");
		printf("******quit			1000***********\n");	
		printf("*********************************\n");		
		printf("******pls enter your cmd:\n");
		//printf("******cmd_str=%s\n",cmd_str);
		if(!scanf("%d",&acquire_cmd))
		{
			scanf("%s",cmd_str);
			printf("******get cmd error,%s is not a cmd!\n",cmd_str);
			memset(cmd_str, 0, sizeof(cmd_str));
			continue;
		}
		if(acquire_cmd>0&&acquire_cmd<1000)
		{
			printf("acquire_cmd=%d\n",acquire_cmd);
			ret = process_ioctl_cmd(acquire_cmd, &cur_screen_status);
		}else if (1000==acquire_cmd){
			printf("acquire_cmd=%d,need to quit test!\n",acquire_cmd);
			break;
		}else{
			printf("unknow cmd=%d!!\n",acquire_cmd);
		}
		acquire_cmd = 0;//reset cmd to 0
		
	}while(1000!=acquire_cmd);
	return ret;
}

