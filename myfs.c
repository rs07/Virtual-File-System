#include<stdio.h>
#include<string.h>
#include<sys/types.h>			/*open(2) */
#include<sys/stat.h>			/*open(2) */
#include<fcntl.h>				/*open(2) */
#include<unistd.h>				/*write(2) close(2) */		
#include <error.h>				/*perror()*/
#include<errno.h>
#include<math.h>

#define FILE_SIZE_VALUE 20
#define INODE_COUNT 12
#define MAX_INODE 15
#define MAX_DATA_BLOCK 15
#define MAX_FS 10

int global_count=0;

typedef struct
{
	long int file_size;
	int block_size;
	int root_inode_no;
	int inode_size;
	int inode_start_location;
	int num_inode;
	int num_free_inode;
	int data_block_start_location;
	int num_data_block;
	int num_free_data_block;
	char inode_bitmap[MAX_INODE];
	char data_block_bitmap[MAX_DATA_BLOCK];
}super_block;


// typedef struct
// {
// 	char file_name[FILE_SIZE_VALUE];			/* File System Name */
// 	super_block sb;								/* The information about the file system in superblock */
// 	int num_file;
// }fs;

typedef struct
{
	char type;									/* INODE contains file--- 'f' or directories--- 'd' */
	//int size;
	int num_block_count;
	//inode *link;
	char file_name[FILE_SIZE_VALUE];
	int inode_num;
	int size;									/* IF type= 'f' else -1 if type= 'd' */
	int starting_data_block;					/* IF type= 'f' else -1 if type= 'd' */
	int block_info[MAX_DATA_BLOCK];
}inode;


typedef struct
{
	char drive_name;
	char file_name[FILE_SIZE_VALUE];
}file2dir;

file2dir maping[MAX_FS];

typedef struct
{
	char fs_name[FILE_SIZE_VALUE];
	int count;
	int inode_num[10000];
}present_dir;

present_dir pwd;
char current_drive;
pwd_path [20][FILE_SIZE_VALUE];

/**********************************************************************************************************************************

                                                      BREAK INTO WORDS

***********************************************************************************************************************************/

char **words(char *buffer)
{
	int num_space=0;
	for(int i=0; i<100; i++)
	{
		if(buffer[i]=='\0')
			break;
		if(buffer[i]==' ')
			num_space++;
	}
	char **argv;
	(argv)= (char **)malloc((num_space+1)*sizeof(char *));
	int k=0,c=0;
	char cache[FILE_SIZE_VALUE];
	//printf("%s\n",buffer);
	for(int i=0; i<=num_space; i++)
	{
		(argv[i])= (char *)malloc(20*sizeof(char));
	}
	for(int i=0; i<100; i++)
	{
		//printf("hi");
		if(buffer[i]=='\0')
		{
			argv[k][c]='\0';
			break;
		}
		if(buffer[i]==' ')
		{
			argv[k][c]='\0';
			//strcpy(argv[k],cache);
			k++;
			c=0;
		}
		else
		{
			argv[k][c]=buffer[i];
			c++;
		}
	}
	/*printf("%s\n",argv[0]);
	printf("%s\n",argv[1]);
	printf("%s\n",argv[2]);*/
	return argv;
}






/**********************************************************************************************************************************

                                                      MKFS SYSTEM CALL

***********************************************************************************************************************************/


void mkfs(char *buffer)
{
	//printf("%s\n",buffer);
	char **para= words(buffer);
	char filename[FILE_SIZE_VALUE], blk_size[FILE_SIZE_VALUE];
	int mem_size;
	strcpy(filename,para[1]);
	strcpy(blk_size,para[2]);
	sscanf(para[3],"%d",&mem_size);
	// printf("filename: %s\n",filename);
	// printf("blk_size: %s\n",blk_size);
	// printf("mem_size: %d\n",mem_size);
	int block_size= atoi(blk_size);
	//printf("%d\n",block_size);
	int file_id= open(filename, O_CREAT|O_EXCL|O_RDWR);
	if(file_id==-1)
	{
		fprintf(stderr, "Error: File already exists.\n");
		return;
	}
	long int mem_size_bytes= mem_size*1048576;
	int status= ftruncate(file_id,mem_size_bytes);
	if(status==-1)
	{
		fprintf(stderr, "Error: Unable to allocate memory: %s====>\n",strerror(errno));
		//perror(errno);
		if(errno==EACCES)
		{
			printf("EACCES\n");
		}
		else if(errno==EFAULT)
		{
			printf("EFAULT\n");
		}
		else if(errno== EFBIG)
		{
			printf("EFBIG\n");
		}
		else if(errno==EINTR)
		{
			printf("EINTR\n");
		}
		else if(errno== EBADF)
		{
			printf("EBADF\n");
		}
		else if(errno== EINVAL)
		{
			printf("EINVAL\n");
		}
		else
		{
			printf("Other\n");
		}
	}
	else
	{
		//printf("Success\n");
	}
	super_block sb;
	sb.file_size= mem_size_bytes;
	sb.block_size= block_size;
	sb.root_inode_no= 0;
	sb.inode_size= sizeof(inode);
	sb.inode_start_location= sizeof(super_block);
	sb.num_inode= INODE_COUNT;
	sb.num_free_inode= INODE_COUNT;
	sb.data_block_start_location= sizeof(super_block) + INODE_COUNT*sizeof(inode);
	sb.num_data_block= (mem_size_bytes - (sizeof(super_block) + INODE_COUNT * sizeof(inode)))/block_size;
	sb.num_free_data_block= sb.num_data_block;
	for(int i=0; i<MAX_INODE; i++)
		sb.inode_bitmap[i]='0';
	for(int i=0; i<MAX_DATA_BLOCK; i++)
		sb.data_block_bitmap[i]='0';

	inode in;
	in.type= 'd';
	in.num_block_count=0;
	strcpy(in.file_name,"root");
	in.inode_num= 0;
	in.size= -1;
	in.starting_data_block= -1;

	for(int i=0; i<MAX_DATA_BLOCK; i++)
		in.block_info[i]=-1;

	sb.inode_bitmap[0]='1';
	sb.num_free_inode= sb.num_free_inode-1;
	int write_status;

	write_status= write(file_id, (void *)&sb, sizeof(sb));
	if(write_status== sizeof(sb))
	{
		//printf("Superblock writing Successfull\n");
	}
	else
	{
		printf("\x1B[31mError: Writing superblock failed=== %s ====\n",strerror(errno));
		return;
	}
	//fseek(file_id, 0, SEEK_SET);
	lseek(file_id,0,SEEK_SET);
	lseek(file_id,sizeof(sb),SEEK_SET);
	write_status = write(file_id, (void *)&in, sizeof(in));
	if(write_status== sizeof(in))
	{
		//printf("Inode writing Successfull\n");
	}
	else
	{
		fprintf(stderr, "\x1B[31mError: Writing inode failed\n");
		return;
	}
	close(file_id);
}



/**********************************************************************************************************************************

                                                      CD SYSTEM CALL

***********************************************************************************************************************************/


void cd(char *buffer)
{
	char **para= words(buffer);
	if(strcmp(para[1],"..")==0)
	{
		pwd.count--;
		return;
	}
	int file_id= open(pwd.fs_name,O_RDONLY,S_IRWXU);
	if(file_id==-1)
	{
		fprintf(stderr, "\x1B[31mError: Unable to open file\n");
	}
	super_block sb;
	int num_in;
	num_in= pwd.inode_num[pwd.count-1];
	//printf("Inode num: %d\n",num_in);
	lseek(file_id,0,SEEK_SET);
	lseek(file_id,sizeof(sb)+sizeof(inode)*num_in,SEEK_SET);
	inode in,in_cache;
	int read_status;
	read_status= read(file_id,(void *)&in, sizeof(in));
	if(read_status==-1)
		fprintf(stderr, "\x1B[31mError: Unable to read inode\n");
	for(int i=0; i<MAX_DATA_BLOCK; i++)
	{
		lseek(file_id,0,SEEK_SET);
		lseek(file_id,sizeof(sb)+sizeof(inode)*in.block_info[i],SEEK_SET);
		read_status= read(file_id,(void *)&in_cache, sizeof(in_cache));
		if(read_status==-1)
		{
			fprintf(stderr,"\x1B[31mError: Unable to read inode_cache\n");
		}
		if(strcmp(in_cache.file_name,para[1])==0)
		{
			pwd.inode_num[pwd.count]=in_cache.inode_num;
			pwd.count++;
			strcpy(pwd_path[pwd.count],para[1]);
			break;
		}
	}
	close(file_id);
}





/**********************************************************************************************************************************

                                                      MKDIR SYSTEM CALL

***********************************************************************************************************************************/


void nwdir(char *buffer)
{
	char **para= words(buffer);
	//printf("folder name: %s\n",para[1]);
	super_block sb;
	int file_id= open(pwd.fs_name,O_RDWR);
	if(file_id==-1)
	{
		fprintf(stderr, "\x1B[31mError: Unable to open filesystem\n");
	}
	lseek(file_id,0,SEEK_SET);
	int read_status= read(file_id,(void *)&sb, sizeof(sb));
	if(read_status==-1)
	{
		fprintf(stderr, "\x1B[31mError: Unable to read file\n");
	}
	int num_in= pwd.inode_num[pwd.count-1];
	inode in;
	lseek(file_id,0,SEEK_SET);
	lseek(file_id,sizeof(sb)+sizeof(inode)*num_in,SEEK_SET);
	read_status= read(file_id,(void *)&in, sizeof(in));
	if(read_status==-1)
	{
		fprintf(stderr, "\x1B[31mError: Unable to read file\n");
	}
	inode in_cache;
	int i;
	for(i=0; i<MAX_INODE; i++)
	{
		if(sb.inode_bitmap[i]=='0')
		{
			sb.inode_bitmap[i]='1';
			sb.num_free_inode--;
			//printf("Empty Slot found: %d\n",i);
			break;
		}
	}
	in.block_info[in.num_block_count]=i;
	in.num_block_count++;
	in_cache.type='d';
	in_cache.num_block_count=0;
	strcpy(in_cache.file_name,para[1]);
	in_cache.inode_num=i;
	in_cache.size=-1;
	in_cache.starting_data_block=-1;
	for(int i=0; i<MAX_DATA_BLOCK; i++)
		in_cache.block_info[i]=-1;
	lseek(file_id,0,SEEK_SET);
	lseek(file_id,sizeof(sb)+sizeof(inode)*num_in,SEEK_SET);
	int write_status= write(file_id, (void *)&in, sizeof(in));
	if(write_status==-1)
	{
		fprintf(stderr, "\x1B[31mError: Unable to write back into File System\n");
	}
	lseek(file_id,0,SEEK_SET);
	write_status= write(file_id, (void *)&sb, sizeof(sb));
	if(write_status==-1)
	{
		fprintf(stderr, "\x1B[31mError: Unable to write back into File System\n");
	}
	lseek(file_id,0,SEEK_SET);
	lseek(file_id,sizeof(sb)+sizeof(inode)*i,SEEK_SET);
	write_status= write(file_id, (void *)&in_cache, sizeof(in_cache));
	if(write_status==-1)
	{
		fprintf(stderr, "\x1B[31mError: Unable to write back into File System\n");
	}
	//printf("%s:\\",pwd_path[0]);
	//printf("%s\\",pwd_path[1]);
	close(file_id);
}





/**********************************************************************************************************************************

                                                      USE SYSTEM CALL

***********************************************************************************************************************************/

void use(char *buffer)
{
	//printf("%s\n",buffer);
	char **para= words(buffer);
	//printf("filename1: %s\n",para[1]);
	//printf("oper: %s\n",para[2]);
	//printf("filename2: %s\n",para[3]);
	maping[global_count].drive_name= para[3][0];
	//printf("Hi1\n");
	strcpy(maping[global_count].file_name,para[1]);
	//printf("Hi2\n");
	//printf("%s\n",maping[global_count].file_name);
	/*for(int i=0; i<FILE_SIZE_VALUE; i++)
	{
		if(para[1][i]=='\0')
			break;
		file2dir[global_count].file_name[i]=para[1][i];
	}*/
	global_count++;
	current_drive=para[3][0];
	char fs_name[FILE_SIZE_VALUE];
	strcpy(fs_name,para[1]);
	/*for(int i=0; i<global_count; i++)
	{
		if(current_drive==maping[i].drive_name)
		{
			strcpy(fs_name,maping[i].file_name);
			break;
		}
	}*/
	/*int file_id= open(fs_name,O_RDONLY,S_IRWXU);
	if(file_id==-1)
	{
		fprintf(stderr, "Error: Unable to open file\n");
	}*/
	pwd_path[0][0]= current_drive;
	pwd_path[0][1]= '\0';
	strcpy(pwd.fs_name,fs_name);
	pwd.inode_num[0]=0;
	strcpy(pwd_path[1],"root");
	pwd.count=1;
	//printf("%s:\\",pwd_path[0]);
	//printf("%s\\",pwd_path[1]);
}

/**********************************************************************************************************************************

                                                      CP SYSTEM CALL

***********************************************************************************************************************************/

void cp(char *buffer)   /* JUST FOR CHECKING PURPOSE ----NOT CORRECT */
{
	char **para= words(buffer);
	//printf("source: %s\n",para[1]);
	//printf("destination: %s\n",para[2]);
	//int file_id= open(para[1],O_RDONLY);
	int len1= strlen(para[1]);
	int len2= strlen(para[2]);
	int num_slash1=0, num_slash2=0;
	for(int i=0; i<len1; i++)
	{
		if(para[1][i]=='\\')
			num_slash1++;
	}
	for(int i=0; i<len2; i++)
	{
		if(para[2][i]=='\\')
			num_slash2++;
	}
	//printf("Slash1: %d\n",num_slash1);
	//printf("Slash2: %d\n",num_slash2);
	char src_add[num_slash1+1][FILE_SIZE_VALUE];
	char des_add[num_slash2][FILE_SIZE_VALUE];

	int k1=0, k2=0, c1=0, c2=0;

	for(int i=0; i<len1; i++)
	{
		if(para[1][i]!='\\')
		{
			src_add[k1][c1++]=para[1][i];
		}
		else
		{
			if(para[1][i]=='\\')
			{
				src_add[k1][c1]='\0';
				k1++;
				c1=0;
			}
			else if(para[1][i]=='\0')
			{
				src_add[k1][c1]='\0';
				//printf("Hi\n");
				break;
			}
		}
	}

	for(int i=0; i<len2; i++)
	{
		if(para[2][i]!='\\')
		{
			des_add[k2][c2++]=para[2][i];
		}
		else
		{
			if(para[2][i]=='\\')
			{
				des_add[k2][c2]='\0';
				k2++;
				c2=0;
			}
			else if(para[2][i]=='\0')
			{
				des_add[k2][c2]='\0';
				break;
			}
		}
	}
	/*printf("%s\n",src_add[0]);
	printf("%s\n",src_add[1]);
	printf("%s\n",src_add[2]);
	printf("%s\n",src_add[3]);

	printf("%s\n",des_add[0]);
	printf("%s\n",des_add[1]);
	printf("%s\n",des_add[2]);
	printf("%s\n",des_add[3]);*/

	/*for(int i=0; i<=num_slash1; i++)
	{
		printf("%s\n",src_add[i]);
	}
	for(int i=0; i<num_slash2; i++)
	{
		printf("%s\n",des_add[i]);
	}*/

	char fs_name1[FILE_SIZE_VALUE],fs_name2[FILE_SIZE_VALUE];
	for(int i=0; i<global_count; i++)
	{
		if(maping[i].drive_name==src_add[0][0])
		{
			strcpy(fs_name1,maping[i].file_name);
			break;
		}
	}

	for(int i=0; i<global_count; i++)
	{
		if(maping[i].drive_name==des_add[0][0])
		{
			strcpy(fs_name2,maping[i].file_name);
			break;
		}
	}

	int file_id1,file_id2;
	file_id1= open(fs_name1,O_RDWR);
	if(file_id1==-1)
	{
		fprintf(stderr, "\x1B[31mError: Unable to open file\n");
	}
	file_id2= open(fs_name2,O_RDWR);
	if(file_id2==-1)
	{
		fprintf(stderr, "\x1B[31mError: Unable to open file\n");
	}
	lseek(file_id1,0,SEEK_SET);
	super_block sb1, sb2;
	int read_status= read(file_id1,(void *)&sb1, sizeof(sb1));
	if(read_status==-1)
	{
		fprintf(stderr, "\x1B[31mError: Unable to read file sb1\n");
	}
	read_status= read(file_id2,(void *)&sb2, sizeof(sb2));
	if(read_status==-1)
	{
		fprintf(stderr, "\x1B[31mError: Unable to read file sb2\n");
	}
	inode in1, in2, in_cache1, in_cache2;
	lseek(file_id1,0,SEEK_SET);
	lseek(file_id1,sizeof(sb1)+sizeof(inode)*0,SEEK_SET);
	read_status= read(file_id1,(void *)&in1, sizeof(in1));
	if(read_status==-1)
	{
		fprintf(stderr, "\x1B[31mError: Unable to read file inode1\n");
	}
	lseek(file_id2,0,SEEK_SET);
	lseek(file_id2,sizeof(sb2)+sizeof(inode)*0,SEEK_SET);
	read_status= read(file_id2,(void *)&in2, sizeof(in2));
	if(read_status==-1)
	{
		fprintf(stderr, "\x1B[31mError: Unable to read file inode2\n");
	}
	for(int j=0; j<num_slash1; j++)
	{
		if(src_add[0][1]==':')
		{
			if(j==0 || j==1)
				continue;
		}
		for(int i=0; i<in1.num_block_count; i++)
		{
			lseek(file_id1,0,SEEK_SET);
			lseek(file_id1,sizeof(sb1)+sizeof(inode)*in1.block_info[i],SEEK_SET);
			read_status= read(file_id1,(void *)&in_cache1, sizeof(in_cache1));
			if(read_status==-1)
			{
				fprintf(stderr, "\x1B[31mError: Unable to read file nested_inode\n");
			}
			if(strcmp(in_cache1.file_name,src_add[j])==0)
			{
				in1=in_cache1;
				break;
			}
		}
	}
	//printf("Folder name src: %s\n",in1.file_name);
	for(int j=0; j<num_slash2; j++)
	{
		if(des_add[0][1]==':')
		{
			if(j==0 || j==1)
				continue;
		}
		for(int i=0; i<in2.num_block_count; i++)
		{
			lseek(file_id2,0,SEEK_SET);
			lseek(file_id2,sizeof(sb2)+sizeof(inode)*in2.block_info[i],SEEK_SET);
			read_status= read(file_id2,(void *)&in_cache2, sizeof(in_cache2));
			if(read_status==-1)
			{
				fprintf(stderr, "\x1B[31mError: Unable to read file nested_inode\n");
			}
			if(strcmp(in_cache2.file_name,des_add[j])==0)
			{
				in2=in_cache2;
				break;
			}
		}
	}
	//printf("Folder name des: %s\n",in2.file_name);
	int num_inode;
	for(int i=0; i<in1.num_block_count; i++)
	{
		lseek(file_id1,0,SEEK_SET);
		lseek(file_id1,sizeof(sb1)+sizeof(inode)*in1.block_info[i],SEEK_SET);
		read_status= read(file_id1,(void *)&in_cache1, sizeof(in_cache1));
		if(read_status==-1)
		{
			fprintf(stderr, "\x1B[31mError: Unable to read file nested_inode\n");
		}
		if(strcmp(in_cache1.file_name,src_add[num_slash1])==0)
		{
			num_inode= in_cache1.inode_num;
			break;
		}
	}
	int fr_i, fr_d;
	for(int i=0; i<MAX_INODE; i++)
	{
		if(sb2.inode_bitmap[i]=='0')
		{
			sb2.inode_bitmap[i]='1';
			sb2.num_free_inode--;
			fr_i=i;
			//printf("Empty Slot found: %d\n",fr_i);
			for(int j=0; j<MAX_DATA_BLOCK; j++)
			{
				if(sb2.data_block_bitmap[j]=='0')
				{
					sb2.data_block_bitmap[j]='1';
					fr_d=j;
					//printf("Empty Slot Data_Block found: %d\n",fr_d);
					break;
				}
			}
			break;
		}
	}
	in2.block_info[in2.num_block_count]=fr_i;
	in2.num_block_count++;
	lseek(file_id2,0,SEEK_SET);
	lseek(file_id2,sizeof(sb2)+sizeof(inode)*in2.inode_num, SEEK_SET);
	int write_status= write(file_id2, (void *)&in2, sizeof(in2));
	if(write_status==-1)
	{
		fprintf(stderr, "\x1B[31mError: Unable to write back into File System\n");
	}
	inode in_new;
	in_new = in_cache1;
	in_new.inode_num= fr_i;
	in_new.starting_data_block=fr_d;
	for(int k=0; k<in_new.num_block_count; k++)
	{
		sb2.data_block_bitmap[fr_d+k]='1';
	}
	lseek(file_id2,0,SEEK_SET);
	write_status= write(file_id2, (void *)&sb2, sizeof(sb2));
	if(write_status==-1)
	{
		fprintf(stderr, "\x1B[31mError: Unable to write back into File System\n");
	}
	char buff[1000];
	lseek(file_id1,0,SEEK_SET);
	lseek(file_id1,sizeof(sb1)+sizeof(inode)*sb1.num_inode+sb1.block_size*in_cache1.starting_data_block,SEEK_SET);
	read_status= read(file_id1, (char *)&buff, in_cache1.size);
	if(read_status==-1)
	{
		fprintf(stderr, "\x1B[31mError: Unable to read back into File System_file\n");
	}
	//printf("This is copying: %s\n",buff);
	lseek(file_id2,0,SEEK_SET);
	lseek(file_id2,sizeof(sb2)+sizeof(inode)*sb2.num_inode+sb2.block_size*fr_d,SEEK_SET);
	write_status= write(file_id2, (char *)&buff, in_new.size);
	if(write_status==-1)
	{
		fprintf(stderr, "\x1B[31mError: Unable to write back into File System_file\n");
	}
	lseek(file_id2,0,SEEK_SET);
	lseek(file_id2,sizeof(sb2)+sizeof(inode)*fr_i,SEEK_SET);
	write_status= write(file_id2, (void *)&in_new, sizeof(in_new));
	if(write_status==-1)
	{
		fprintf(stderr, "\x1B[31mError: Unable to write back into File System_file\n");
	}
	close(file_id1);
	close(file_id2);
}



/**********************************************************************************************************************************

                                                      LS SYSTEM CALL

***********************************************************************************************************************************/


void ls(char *buffer)
{
	char **para= words(buffer);
	super_block sb;
	int file_id= open(pwd.fs_name,O_RDWR);
	if(file_id==-1)
	{
		fprintf(stderr, "\x1B[31mError: Unable to open filesystem\n");
	}
	lseek(file_id,0,SEEK_SET);
	int read_status= read(file_id,(void *)&sb, sizeof(sb));
	if(read_status==-1)
	{
		fprintf(stderr, "\x1B[31mError: Unable to read file\n");
	}
	inode in, in_cache;
	lseek(file_id,0,SEEK_SET);
	lseek(file_id,sizeof(sb)+sizeof(inode)*pwd.inode_num[pwd.count-1],SEEK_SET);
	read_status= read(file_id, (void *)&in, sizeof(in));
	if(read_status==-1)
	{
		fprintf(stderr, "\x1B[31mError: Unable to read File\n");
	}
	for(int i=0; i<in.num_block_count; i++)
	{
		lseek(file_id,0,SEEK_SET);
		lseek(file_id,sizeof(sb)+sizeof(inode)*in.block_info[i],SEEK_SET);
		read_status= read(file_id,(void *)&in_cache, sizeof(in_cache));
		if(read_status==-1)
		{
			fprintf(stderr,"\x1B[31mError: Unable to read inode\n");
		}
		printf("\033[1m\033[32m%s\n",in_cache.file_name);
		/*if(strcmp(in_cache.file_name,para[1])==0)
		{
			pwd.inode_num[pwd.count]=in_cache.inode_num;
			pwd.count++;
			strcpy(pwd_path[pwd.count],para[1]);
			break;
		}*/
	}
	close(file_id);
}



/**********************************************************************************************************************************

                                                      NWFILE SYSTEM CALL

***********************************************************************************************************************************/

void nwfile(char *buffer)
{
	char **para= words(buffer);
	//printf("command: %s\n",para[0]);
	//printf("file name: %s\n",para[1]);
	//printf("Size: %s\n",para[2]);
	int file_size;
	super_block sb;
	sscanf(para[2],"%d",&file_size);
	//printf("File Size: %d\n",file_size);
	long int file_size_bytes= file_size*1024;
	int file_id= open(pwd.fs_name,O_RDWR);
	if(file_id==-1)
	{
		fprintf(stderr, "\x1B[31mError: Unable to open filesystem\n");
	}
	lseek(file_id,0,SEEK_SET);
	int read_status= read(file_id,(void *)&sb, sizeof(sb));
	if(read_status==-1)
	{
		fprintf(stderr, "\x1B[31mError: Unable to read file\n");
	}
	inode in;
	int num_in= pwd.inode_num[pwd.count-1];
	lseek(file_id,0,SEEK_SET);
	lseek(file_id,sizeof(sb)+sizeof(inode)*num_in,SEEK_SET);
	read_status= read(file_id, (void *)&in, sizeof(in));
	if(read_status==-1)
	{
		fprintf(stderr, "\x1B[31mError: Unable to read from File System\n");
	}
	inode in_cache;
	int i,j;
	for(i=0; i<MAX_INODE; i++)
	{
		if(sb.inode_bitmap[i]=='0')
		{
			sb.inode_bitmap[i]='1';
			sb.num_free_inode--;
			//printf("Empty Slot Inode found: %d\n",i);
			for(j=0; j<MAX_DATA_BLOCK; j++)
			{
				if(sb.data_block_bitmap[j]=='0')
				{
					sb.data_block_bitmap[j]='1';
					//printf("Empty Slot Data_Block found: %d\n",j);
					break;
				}
			}
			break;
		}
	}
	int num_dblock= ceil(file_size_bytes/sb.block_size);
	lseek(file_id,0,SEEK_SET);
	lseek(file_id,sizeof(sb)+sizeof(inode)*num_in, SEEK_SET);
	in.block_info[in.num_block_count]=i;
	in.num_block_count++;
	int write_status= write(file_id, (void *)&in, sizeof(in));
	if(write_status==-1)
	{
		fprintf(stderr, "Error: Unable to write back into File System\n");
	}
	in_cache.type='f';
	in_cache.num_block_count= num_dblock;
	strcpy(in_cache.file_name,para[1]);
	in_cache.inode_num=i;
	//printf("Inode writing at: %d\n",i);
	in_cache.size= file_size_bytes;
	in_cache.starting_data_block= j;
	for(int k=0; k<MAX_DATA_BLOCK; k++)
		in_cache.block_info[k]=-1;
	for(int k=0; k<num_dblock; k++)
	{
		sb.data_block_bitmap[j+k]='1';
	}
	sb.num_free_data_block-=num_dblock;    /////////////////////////////////ERROR CHANCES
	lseek(file_id,0,SEEK_SET);
	write_status= write(file_id, (void *)&sb, sizeof(sb));
	if(write_status==-1)
	{
		fprintf(stderr, "Error: Unable to write back into File System\n");
	}
	lseek(file_id,0,SEEK_SET);
	lseek(file_id,sizeof(sb)+sizeof(inode)*sb.num_inode+sb.block_size*j,SEEK_SET);
	char buff[1000];
	printf("\033[1m\033[33mEnter things want to write in file: \x1B[31m");
	gets(buff);
	int len= strlen(buff);
	//printf("Length: %d\n",len);
	in_cache.size= sizeof(char)*len;

	/* INODE_CACHE not saved till now not complete */

	write_status= write(file_id, (char *)&buff, sizeof(char)*len);
	if(write_status==-1)
	{
		fprintf(stderr, "\x1B[31mError: Unable to write back into File System_file\n");
	}
	char blk[1000];
	lseek(file_id,0,SEEK_SET);
	lseek(file_id,sizeof(sb)+sizeof(inode)*sb.num_inode+sb.block_size*j,SEEK_SET);
	read_status= read(file_id, (char *)&blk, sizeof(char)*len);
	if(read_status==-1)
	{
		fprintf(stderr, "\x1B[31mError: Unable to read back into File System_file\n");
	}
	//printf("This Line: %s\n",blk);
	lseek(file_id,0,SEEK_SET);
	lseek(file_id,sizeof(sb)+sizeof(inode)*i,SEEK_SET);
	write_status= write(file_id, (void *)&in_cache, sizeof(in_cache));
	if(write_status==-1)
	{
		fprintf(stderr, "\x1B[31mError: Unable to write back into File System_file\n");
	}
	close(file_id);
}



/**********************************************************************************************************************************

                                                      RDFILE SYSTEM CALL

***********************************************************************************************************************************/


void rdfile(char *buffer)
{
	char **para= words(buffer);
	int flag=0;
	int file_id= open(pwd.fs_name,O_RDWR);
	if(file_id==-1)
	{
		fprintf(stderr, "Error: Unable to open filesystem\n");
	}
	super_block sb;
	lseek(file_id,0,SEEK_SET);
	int read_status= read(file_id, (void *)&sb, sizeof(sb));
	if(read_status==-1)
	{
		fprintf(stderr, "Error: Unable to read File\n");
	}
	inode in, in_cache;
	lseek(file_id,0,SEEK_SET);
	lseek(file_id,sizeof(sb)+sizeof(inode)*pwd.inode_num[pwd.count-1],SEEK_SET);
	read_status= read(file_id, (void *)&in, sizeof(in));
	if(read_status==-1)
	{
		fprintf(stderr, "Error: Unable to read File\n");
	}
	for(int i=0; i<in.num_block_count; i++)
	{
		lseek(file_id,0,SEEK_SET);
		lseek(file_id,sizeof(sb)+sizeof(inode)*in.block_info[i],SEEK_SET);
		read_status= read(file_id,(void *)&in_cache, sizeof(in_cache));
		if(read_status==-1)
		{
			fprintf(stderr,"Error: Unable to read inode\n");
		}
		//printf("%s\n",in_cache.file_name);
		if(strcmp(in_cache.file_name,para[1])==0)
		{
			flag=1;
			break;
		}
	}
	if(flag==0)
	{
		fprintf(stderr, "Error: Unable to find file\n");
		close(file_id);
		return;
	}
	char buff[1000];
	lseek(file_id,0,SEEK_SET);
	lseek(file_id,sizeof(sb)+sizeof(inode)*sb.num_inode+sb.block_size*in_cache.starting_data_block,SEEK_SET);
	read_status= read(file_id, (char *)&buff, in_cache.size);
	if(read_status==-1)
	{
		fprintf(stderr, "Error: Unable to read back into File System_file\n");
	}
	printf("\033[1m\033[33mStored in file: \x1B[36m%s\n",buff);
	close(file_id);
}



/**********************************************************************************************************************************

                                                      RM SYSTEM CALL

***********************************************************************************************************************************/


void rm(char *buffer)
{
	char **para=words(buffer);
	int file_id= open(pwd.fs_name,O_RDWR);
	if(file_id==-1)
	{
		fprintf(stderr, "Error: Unable to open filesystem\n");
	}
	super_block sb;
	lseek(file_id,0,SEEK_SET);
	int read_status= read(file_id, (void *)&sb, sizeof(sb));
	if(read_status==-1)
	{
		fprintf(stderr, "Error: Unable to read File\n");
	}
	inode in,in_cache;
	int num_in;
	lseek(file_id,0,SEEK_SET);
	lseek(file_id,sizeof(sb)+sizeof(inode)*pwd.inode_num[pwd.count-1],SEEK_SET);
	read_status= read(file_id, (void *)&in, sizeof(in));
	for(int i=0; i<in.num_block_count; i++)
	{
		lseek(file_id,0,SEEK_SET);
		lseek(file_id,sizeof(sb)+sizeof(inode)*in.block_info[i],SEEK_SET);
		read_status= read(file_id,(void *)&in_cache, sizeof(in_cache));
		if(read_status==-1)
		{
			fprintf(stderr, "Error: Unable to read File\n");
		}
		if(strcmp(para[1],in_cache.file_name)==0)
		{
			num_in=i;
			break;
		}
	}
	int db_start= in_cache.starting_data_block;
	int num_db= in_cache.num_block_count;
	for(int i=db_start; i<db_start+num_db; i++)
	{
		if(sb.data_block_bitmap[i]=='1')
			sb.data_block_bitmap[i]='0';
	}
	sb.num_free_data_block+=num_db;
	sb.num_free_inode+=1;
	sb.inode_bitmap[in_cache.inode_num]='0';
	for(int i=num_in; i<in.num_block_count-1; i++)
	{
		in.block_info[i]=in.block_info[i+1];
	}
	in.num_block_count--;
	lseek(file_id,0,SEEK_SET);
	int write_status= write(file_id, (void *)&sb, sizeof(sb));
	if(write_status==-1)
	{
		fprintf(stderr, "Error: Unable to write back into File System\n");
	}
	lseek(file_id,0,SEEK_SET);
	lseek(file_id,sizeof(sb)+sizeof(inode)*pwd.inode_num[pwd.count-1],SEEK_SET);
	write_status= write(file_id, (void *)&in, sizeof(in));
	if(write_status==-1)
	{
		fprintf(stderr, "Error: Unable to write back into File System\n");
	}
	close(file_id);
}






/**********************************************************************************************************************************

                                                      MAIN FUNCTION

***********************************************************************************************************************************/



int main()
{
	char buffer[100];
	char *prompt="myfs:~";
	while(1)
	{
		printf("\033[1m\033[33m%s \033[1m\033[34m",prompt);
		for(int i=0; i<=pwd.count; i++)
		{
			if(i==0)
				printf("%s:\\",pwd_path[i]);
			else
				printf("%s\\",pwd_path[i]);
		}
		printf("\033[1m\033[37m# ");
		fflush(stdin);
		//scanf("%[^\n]%*c",buffer);
		gets(buffer);
		//printf("====> %s\n",buffer);
		if(strcmp(buffer,"exit()")==0)
			break;
		char operation[10];
		int i,num_space=0;
		for(i=0; i<100; i++)
		{
			if(buffer[i]==' '|| buffer[i]=='\0')
				break;
			operation[i]=buffer[i];
		}
		operation[i]='\0';
		for(i=0; i<100; i++)
		{
			if(buffer[i]==' ')
				num_space++;
		}
		//printf("Spaces: %d\n",num_space);
		if(strcmp(operation,"mkfs")==0)
		{
			//printf("MKFS command entered\n");
			//if(num_space==3)
				mkfs(buffer);
			//else
			//	fprintf(stderr, "Usage: mkfs [filename] [block_size] [mem_size]\n");
		}
		else if(strcmp(operation,"use")==0)
		{
			//printf("USE command entered\n");
			//if(num_space==3)
				use(buffer);
			//else
			//	fprintf(stderr, "Usage: use [filename] as [drive_name]\n");
		}
		else if(strcmp(operation,"nwdir")==0)
		{
			//printf("NWDIR command entered\n");
			//if(num_space==1)
				nwdir(buffer);
			//else
			//	fprintf(stderr, "Usage: nwdir [dir_name]\n");
		}
		else if(strcmp(operation,"cd")==0)
		{
			//printf("CD command entered\n");
			//if(num_space==1)
				cd(buffer);
			//else
			//	fprintf(stderr, "Usage: cd [dir_name]\n");
		}
		else if(strcmp(operation,"cp")==0)
		{
			//printf("CP command entered\n");
			//**words(buffer);
			//if(num_space==2)
				cp(buffer);
			//else
			//	fprintf(stderr, "Usage: cp [source] [destination]\n");
			/*char **para= words(buffer);
			printf("%s\n",para[0]);
			printf("%s\n",para[1]);
			printf("%s\n",para[2]);*/
		}
		else if(strcmp(operation,"ls")==0)
		{
			//printf("LS command entered\n");
			//if(num_space==0)
				ls(buffer);
			//else
			//	fprintf(stderr, "Usage: ls\n");
		}
		else if(strcmp(operation,"nwfile")==0)
		{
			//printf("LS command entered\n");
			//if(num_space==2)
				nwfile(buffer);
			//else
			//	fprintf(stderr, "Usage: nwfile [filename] [mem_size in KB]\n");
		}
		else if(strcmp(operation,"rdfile")==0)
		{
			//printf("LS command entered\n");
			//if(num_space==1)
				rdfile(buffer);
			//else
			//	fprintf(stderr, "Usage: rdfile [filename]\n");
		}
		else if(strcmp(operation,"rm")==0)
		{
			//printf("MV command entered\n");
			//if(num_space==1)
				rm(buffer);
			//else
			//	fprintf(stderr, "Usage: rm [filename]\n");
		}
	}
}