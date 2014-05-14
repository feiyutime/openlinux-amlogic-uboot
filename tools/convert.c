#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <getopt.h>

#include "romfuns.h"
struct romboot_s support_socs[]={
    {
        .cpu="m1",
        .write=m1_write,
    },
    {
        .cpu="m2",
        .write=m1_write,
    },
    {
        .cpu="a3",
        .write=a3_write,
    },
    {
        .cpu="m3",
        .write=m3_write,
        .write_ex=m3_write_ex,
    },
    {
        .cpu=NULL,
        .write=NULL,
    }
};
#define MAX_SOCS    (sizeof(support_socs)/sizeof(support_socs[0]))
static struct option longopts[] =
{
    { "spl",     1,  NULL,    's'  },
    { "input",   1,  NULL,    'i'  },
    { "addr",    1,  NULL,    'a'  },
    { "output",  1,  NULL,    'o'  },
    { "soc",     1,  NULL,    'c'  },
    { "help",    1,  NULL,    'h'  },
    [MAX_SOCS-1+5]=
    {     0,    	0,     0,    0 },
};


#define for_each_soc(entry) for(entry=&support_socs[0]; \
                                entry->cpu;entry++)
static void printUsage(char * program,unsigned line)
{
    struct romboot_s *entry;
    fprintf("\nLine==%d\n",line);
    fprintf(stderr ,"\n%s usage:\t%s -i input <-s spl> -o output <-soc <cpuid>|-<cpuid>\n"
                            ,program,program);
    fprintf(stderr,"\t available cpuid: ");
    for_each_soc(entry)
    {
        fprintf(stderr,"%s ",entry->cpu);
    }
    fprintf(stderr,"\n");
}
#define ERROR_DUPLICATE_PARAMETER -1
int main(int argc,char * argv[])
{

	FILE * fp_in=NULL;
	FILE * fp_out=NULL;
	FILE * fp_spl=NULL;
	unsigned addr=0;
    char * spl_file=NULL;
    char * in_file=NULL;
    char * out_file=NULL;
    struct romboot_s *entry;
    int cpu = -1;
	int ret=1;
	int i=0;
	int c;
	for_each_soc(entry)
	{
	    longopts[i+5].name=entry->cpu;
	    longopts[i+5].has_arg=0;
	    longopts[i+5].flag=&cpu;
	    longopts[i+5].val=i;
	    i++;
	}
	entry=NULL;
	
	while ((c = getopt_long(argc, argv, ":s:i:o:c:h:a:", longopts, NULL)) != -1)
	{
	    switch(c)
	    {
	    case 'a':
	        addr=strtoul(optarg,NULL,0);
	        break;
	    case 's':
	        if(spl_file)
	        {
	            ret=ERROR_DUPLICATE_PARAMETER;
	            goto exit_prog;
	        }
	        spl_file=optarg;
	        if(fp_in&&strcmp(in_file,spl_file)==0)
	        {
	            fp_spl=fp_in;
	        }else{
	            fp_spl=fopen(optarg,"rb");
	        }
	        
	        break;
	    case 'i':
	        if(in_file)
	        {
	            ret=ERROR_DUPLICATE_PARAMETER-1;
	            goto exit_prog;
	        }
	        in_file=optarg;
	        if(fp_spl&&strcmp(in_file,spl_file)==0)
	        {
	            fp_in=fp_spl;
	        }else{
	            fp_in=fopen(optarg,"rb");
	        }
	        
	        
	        break;
	    case 'o':
	        if(out_file)
	        {
	            ret=ERROR_DUPLICATE_PARAMETER-2;
	            goto exit_prog;
	        }
	        out_file=optarg;
	        fp_out=fopen(optarg,"wb");
	        break;
	    case 'c':
	        if(cpu!=-1)
	        {
	            ret=ERROR_DUPLICATE_PARAMETER-3;
	            goto exit_prog;
	        }
	        if(!optarg)
	            goto exit_prog;
	        for_each_soc(entry)
	        {
	            if(strcasecmp(entry->cpu,optarg)==0)
	                break;
	        }
	        if(entry->cpu==NULL)
	            entry=NULL;
	        break;
        case 'h':
            printUsage(argv[0],__LINE__);	   
            goto exit_prog;     
	    }
	}
	ret=ERROR_DUPLICATE_PARAMETER-4;
	if(cpu==-1 && entry==NULL)
	{
	    goto exit_prog;
	}
//	printf("%d,%d,%d,%d\n",(cpu<-1),(-1>(int)(MAX_SOCS-2)),cpu,MAX_SOCS-2);
	if(cpu<-1||cpu>(int)(MAX_SOCS-2))
	    goto exit_prog;
	
	if(entry==NULL)
	    entry=&support_socs[cpu];
	
	if(fp_spl==NULL && fp_in==NULL)
	{
        //printf("ERROR LINE==%d\n",__LINE__);
	    
        goto exit_prog;     
	}
	if(fp_spl==NULL || fp_in==NULL)
	{
	    if(fp_spl)
	        fp_in=fp_spl;
	    else
	        fp_spl=fp_in;
	    
	}
	
	if(fp_out==NULL)
	{
//printf("ERROR LINE==%d\n",__LINE__);
	    goto exit_prog;
	}
	if(addr&&entry->write_ex)
	    ret=entry->write_ex(fp_spl,fp_in,fp_out,addr);
	else
	    ret=entry->write(fp_spl,fp_in,fp_out);

	
/*	
	
*/
    if(ret)
        fprintf(stderr,"File I/O error\n");
exit_prog:
//    printf("%d\n",ret);
	if(fp_spl)
	{
		fclose(fp_spl);
	}else{
		fprintf(stderr,"File open error %s",spl_file?spl_file:in_file);
	}
	if(fp_in)
	{
	    if(fp_in!=fp_spl)
		    fclose(fp_in);
	}else{
		fprintf(stderr,"File open error A %s",in_file?in_file:spl_file);
	}
	if(fp_out)
	{
		fclose(fp_out);
	}else{
		fprintf(stderr,"File open error %s",out_file);
	}
	if(ret)
	    printUsage(argv[0],__LINE__);
	return ret;
}
