#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <errno.h>
#include <dirent.h>
#include <string.h>
#define mydebug 0
#define FOUND	0
#define NOT_FOUND	1
#define READ_ERROR	2
#define OPEN_ERROR	3

int LoadDirectoryContents(const char* sDir,const char* name);

int breaknow = 0;
char OpenPath[2048];

int main() {
	int bright,dim,mindim,val,iret,red,green;
	unsigned long long int user,nice,system,idle,iowait,irq,softirq,steal,guest,guest_nice,tot,oldidle,oldtot;
	unsigned char rgb[3];
	float percent;
	char strcpu[1000];
	FILE *fptr;
	FILE *fptr2;
	FILE *fptr3;
	oldidle = 0;
	oldtot = 0;

	bright = 255;
	mindim = 32;
	dim = 32;

// Note that the following open statement is based on the usb device address for the pad when installed on my machine
// You will likely need to modify this address to match the location on your machine.
// This approach works on ubuntu 18.04.
	//	fptr = fopen("/sys/devices/pci0000:00/0000:00:14.0/usb3/3-7/3-7:1.0/0003:1532:0C02.0006/matrix_brightness","r+");
	//	fptr = fopen("/sys/devices/pci0000:00/0000:00:14.0/usb3/3-7/3-7:1.0/0003:1532:0C02.000F/matrix_brightness","r+");
	//  fptr = fopen("/sys/devices/pci0000:00/0000:00:14.0/usb3/3-7/3-7:1.0/0003:1532:0C02.0005/matrix_brightness","r+");
#ifdef DEBUG
	fprintf(stderr,"look for matix_brightness" ); // prints look for matix_brightness
#endif
	iret = LoadDirectoryContents("/sys/devices","matrix_brightness");
	fptr = fopen(OpenPath,"r+");
	if(fptr == NULL) {	
		printf("Error 1!");   
		exit(1);
	}
// On ubuntu 18.04, the cpu status is available here:
	fptr2 = fopen("/proc/stat","r");
	if(fptr2 == NULL) {	
		printf("Error 2!");   
		exit(2);
	}
	// 'matrix_effect_static', b'\xFF\x00\x00'
	// (see note above regarding usb addresses)
	//fptr3 = fopen("/sys/devices/pci0000:00/0000:00:14.0/usb3/3-7/3-7:1.0/0003:1532:0C02.0005/matrix_effect_static","wb");
	breaknow = 0;
	iret = LoadDirectoryContents("/sys/devices","matrix_effect_static");
	fptr3 = fopen(OpenPath,"wb");
	if(fptr3 == NULL) {	
		printf("Error 5!");   
		exit(5);
	}
	
	fscanf(fptr,"%i",&val);
	if ( mydebug ) printf("initial val = %i\n",val);
	rewind(fptr);
	while(1) {
		/*
		sleep(1);
		fscanf(fptr,"%i",&val);
		if ( mydebug ) printf("initial val = %i\n",val);
		rewind(fptr);
		fprintf(fptr,"%i",bright);
		rewind(fptr);
		fscanf(fptr,"%i",&val);
		if ( mydebug ) printf("set to bright val = %i\n",val);
*/
		fclose(fptr2);
	fptr2 = fopen("/proc/stat","r");
	if(fptr2 == NULL) {	
		printf("Error 2!");   
		exit(2);
	}
	//	rewind(fptr2);
	// Note: The format of this line has evolved. Linux developers have added fields over time. This worked on ubuntu 18.04 on January 1, 2020. If you are running an older
	// or newer linux, you may need to adjust it.
		iret = fscanf(fptr2,"%s %llu  %llu  %llu  %llu  %llu  %llu  %llu  %llu  %llu  %llu",strcpu,&user,&nice,&system,&idle,&iowait,&irq,&softirq,&steal,&guest,&guest_nice);
		if ( iret == EOF ) {
			printf("EOF on /proc/stat, errno = %i\n",errno);
			clearerr(fptr2);
			exit(3);
		}
		if ( iret != 11 ) {
			printf("failed to read full line on /proc/stat, iret = %i\n",iret);
			clearerr(fptr2);
			exit(4);
		}
//		if ( mydebug ) printf("%s %llu  %llu  %llu  %llu  %llu  %llu  %llu  %llu  %llu  %llu\n",strcpu,user,nice,system,idle,iowait,irq,softirq,steal,guest,guest_nice);
		clearerr(fptr2);

		tot = user+nice+system+idle+iowait+irq+softirq+steal+guest+guest_nice;
//		if ( mydebug ) printf("tot = %llu, idel = %llu\n",tot,idle);
		if ( oldtot ) {
			percent = 1.0 - ((float)(idle-oldidle)/(float)(tot-oldtot));
			dim = mindim + (unsigned int)((bright-mindim) * percent);
			red = (unsigned int)(255 * percent);
			green = (unsigned int)(255 * ( 1.0 - percent) );
			rgb[0] = (unsigned char)red;
			rgb[1] = (unsigned char)green;
			rgb[2] = '\00';
			rewind(fptr3);
			fwrite(rgb,sizeof(rgb),1,fptr3);

			rewind(fptr);
		        fprintf(fptr,"%i",dim);
			rewind(fptr);
			fscanf(fptr,"%i",&val);
			if ( mydebug ) printf("set to dim val = %i\n",val);
		}
		sleep(1);
		oldtot = tot;
		oldidle = idle;
	}
	fclose(fptr);

	return(0);

}


int LoadDirectoryContents(const char* sDir,const char* name){
	struct dirent *dp;
	struct dirent d;
	DIR *dirp;

	dp = &d;
//	const char * typemsg;

	char sPath[2048];

	if (breaknow) return (0);
	dirp = opendir(sDir);

	while (dirp) {
		errno = 0;

		if ((dp = readdir(dirp)) != NULL) {
			if (strcmp(dp->d_name, ".") != 0
				&& strcmp(dp->d_name, "..") != 0)
				{
#ifdef DEBUG
//					fprintf(stderr, "in LoadDirectoryContents, found \"%s\"\n", dp->d_name);
#endif
					//Build up our file path using the passed in
					//  [sDir] and the file/foldername we just found:
					sprintf(sPath, "%s/%s", sDir, dp->d_name);
#ifdef DEBUG
/*					if ( dp->d_type == DT_BLK ) typemsg = "This is a block device.";
					if ( dp->d_type == DT_CHR ) typemsg = "This is a character device.";
					if ( dp->d_type == DT_DIR ) typemsg = "This is a directory.";
					if ( dp->d_type == DT_FIFO ) typemsg = "This is a named pipe (FIFO).";
					if ( dp->d_type == DT_LNK ) typemsg = "This is a symbolic link.";
					if ( dp->d_type == DT_REG ) typemsg = "This is a regular file.";
					if ( dp->d_type == DT_SOCK ) typemsg = "This is a UNIX domain socket.";
					if ( dp->d_type == DT_UNKNOWN ) typemsg = "The file type could not be determined.";
					fprintf(stderr,"%s ... %s\n",sPath,typemsg); */
#endif
					//Is the entity a File or Folder?
					if ( dp->d_type == DT_DIR ) {
						LoadDirectoryContents(sPath,name);
						if(breaknow) {
							closedir(dirp);
							return FOUND;
						}
					} else {
#ifdef DEBUG
//							printf("\"%s\"  ... at %s\n",dp->d_name,sPath);
#endif
						if (strcmp(dp->d_name, name) == 0) {
							breaknow = 1;
#ifdef DEBUG
							printf("%s found! ... at %s\n",name,sPath);
#endif
							strcpy(OpenPath,sPath);
							closedir(dirp);
							return FOUND;
						}
					}
				}
		} else {
			if (errno == 0) {
				closedir(dirp);
				return NOT_FOUND;
			}
			closedir(dirp);
			return READ_ERROR;
		}
	}

return OPEN_ERROR;
}
