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
#define SILEAD_FP_DEV "/dev/silead_fp"
#define TEST_CMD_RST 101
#define TEST_CMD_SEND_SCREEN_STATUS 102
#define TEST_CMD_DISABLE_IRQ 103
#define TEST_CMD_ENABLE_IRQ 104
#define TEST_CMD_POWER_OFF_AND_ON 105

int process_ioctl_cmd(int acq_cmd,char* screen_status)
{
	int ret = -1;
	switch(acq_cmd)
	{
		case TEST_CMD_RST: {
		    ret = ioctl(fd, SIFP_IOC_RESET, 0);
		    if(ret >= 0){
		        printf("******rst success************\n");
		    }
		    else{
			  printf("******rst operate failed*******ret=%d\n",ret);
		    }
		    break;
		}
		case TEST_CMD_SEND_SCREEN_STATUS: {
		    ret = ioctl(fd, SIFP_IOC_SCR_STATUS, screen_status);
		    if(ret >= 0){
		        printf("******get screen_status success,screen_status=%d*******\n",*screen_status);
		    }
		    else{
			  printf("******get screen status operate failed*******ret=%d\n",ret);
		    }			
		    break;
		}
		case TEST_CMD_DISABLE_IRQ: {
		    ret = ioctl(fd, SIFP_IOC_DISABLE_IRQ, 0);
		    if(ret >= 0){
		        printf("******disable irq success************\n");
		    }
		    else{
			  printf("******disable irq operate failed*******ret=%d\n",ret);
		    }
		    break;
		}
		case TEST_CMD_ENABLE_IRQ: {
		    ret = ioctl(fd, SIFP_IOC_ENABLE_IRQ, 0);
		    if(ret >= 0){
		         printf("******enable irq success************\n");
		    }
		    else{
			  printf("******enable irq operate failed*******ret=%d\n",ret);
		    }
		    break;
		}
		case TEST_CMD_POWER_OFF_AND_ON: {
		    ret = ioctl(fd, SIFP_IOC_PWDN, SIFP_PWDN_FLASH);
		    if(ret >= 0){
    		         printf("******power off & on success************\n");
		    }
		    else{
			  printf("******power off & on operate failed*******ret=%d\n",ret);
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
		printf("*********silead driver test***********\n");
		printf("**pls select  your operation with enter**\n");
		printf("******reset		101************\n");
		printf("******send scr		102************\n");		
		printf("******disable irq	103************\n");
		printf("******enable irq	104************\n");
		printf("******power off on	104************\n");
		printf("******quit			1000***********\n");	
		printf("*********************************\n");

		printf("******eg: ./spi_test 101*********\n");
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
		printf("*********silead driver test***********\n");
		printf("**pls select  your operation with enter**\n");
		printf("******reset		101************\n");
		printf("******send scr		102************\n");		
		printf("******disable irq	103************\n");
		printf("******enable irq	104************\n");
		printf("******power off on	104************\n");	
		printf("******quit			1000***********\n");		
		printf("******pls enter your cmd:\n");
		printf("******cmd_str=%s\n",cmd_str);
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
		}else{
			printf("unknow cmd=%d!!\n",acquire_cmd);
		}
		acquire_cmd = 0;//reset cmd to 0
		
	}while(1000!=acquire_cmd);
	return ret;
}

