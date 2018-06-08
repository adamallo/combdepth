//
//  main.c
//  combdepth
//
//  Created by Diego Mallo on 6/5/18.
//  Copyright © 2018 diegoM. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <zlib.h>
#include <getopt.h>
#include <math.h>

#define LIST 0
#define DICT 1

//By Hallvard B. Furuseth. Found here https://stackoverflow.com/questions/3957252/is-there-any-way-to-compute-the-width-of-an-integer-type-at-compile-time
#define IMAX_BITS(m) ((m) /((m)%0x3fffffffL+1) /0x3fffffffL %0x3fffffffL *30 \
+ (m)%0x3fffffffL /((m)%31+1)/31%31*5 + 4-12/((m)%31+3))

#undef DEBUG

//Function declarations
void printBits(size_t const size, void const * const ptr);
int countsetbits(unsigned long long v);
FILE *smartfopen(const char *path, const char *mode);

//Main
int main(int argc, const char * argv[]) {
    
    //Constants
    const int WBITS=IMAX_BITS(ULONG_LONG_MAX);

    //Conf variables
    long MAX_IT=4000000000;
    const char usage[542]="Usage: combdepth (-f|--ref reference_genome.fa | -d|--dict reference_genome.dict | -c|--list chrlist.tsv) -o|--output outputfile [options] inputsample1.tsv [inputsample2.tsv ... inputtsamplen.tsv]\n\nOptions: \n\t-l|--min min \n\t-r|--max max \n\t-b|--by by\n\t-c|--list list of chromosmes sorted in the same order as the input tsv files (to use instead of ref or dict)\n\nThis script takes as input the output of a number of \"samtools depth\" runs and calculates the number of common nucleotide positions at n= (max-min)/by depths in a range [min,max].\n";
    
    //General-usage variables
    long i=0;
    long j=0;
    char *line=NULL;
    size_t lengthlineptr=0;
    char *string=malloc(sizeof(char)*2);
    strcmp(string," ");
    size_t sizestring=1;
    size_t auxsize=0;
    
    //Main variables
    FILE ** filehandles;
    FILE *chr_files;
    
    //ARGV-parsing
    int c=0; // getopt_long case variable
    int option_index = 0;// getopt_long stores the option index here
    int help=0;
    int error=0;
    char **files;
    files=malloc(sizeof(char*)*WBITS);
    int n_allocatedfiles=WBITS;
    char *outputfile="combdepth.csv"; //Default
    char *chrsfile=NULL;
    int chrtype=-1;
    int nfiles=0;
    int maxfilt=40;
    int minfilt=10;
    int byfilt=9;

    while (1)
    {
        static struct option long_options[] =
        {
            {"help", no_argument,       NULL, 'h'},
            {"input",     required_argument,       NULL, 'i'},
            {"output",  required_argument,       NULL, 'o'},
            {"min",  required_argument, NULL, 'l'},
            {"max",  required_argument, NULL, 'r'},
            {"by",    required_argument, NULL, 'b'},
            {"ref", required_argument,  NULL,  'f'},
            {"dict", required_argument, NULL,  'd'},
            {"list", required_argument, NULL,  'c'},
            //{"traces",  no_argument,    NULL, 't'},//TODO: pending to implement
            {0, 0, 0, 0}
        };

        c = getopt_long (argc, argv, "hi:o:l:r:b:f:d:c:",
                         long_options, &option_index);
        
        /* Detect the end of the options. */
        if (c == -1)
            break;
        
        //If parsing problems are detected in the switch I generate warnings. Errors are only generated if the resulting parsed options are not valid
        
        switch (c)
        {
            case 'h':
                help=1;
                break;

            case 'o':
                outputfile=malloc(sizeof(char)*(strlen(optarg)+1));
                strcpy(outputfile,optarg);
                break;
                
            case 'l':
                error=sscanf(optarg,"%d",&minfilt);
                if(error==EOF || error==0)
                {
                    fprintf(stderr,"WARNING: the option -l|--min was not parsed correctly\n");
                }
                break;
                
            case 'r':
                error=sscanf(optarg,"%d",&maxfilt);
                if(error==EOF || error==0)
                {
                    fprintf(stderr,"WARNING: the option -r|--max was not parsed correctly\n");
                }
                break;
                
            case 'b':
                error=sscanf(optarg,"%d",&byfilt);
                if(error==EOF || error==0)
                {
                    fprintf(stderr,"WARNING: the option -b|--by was not parsed correctly\n");
                }
                break;
                
            case 'f':
                auxsize=strlen(optarg)+1;
                for(i=auxsize-1;i>=0;--i)
                    if(optarg[i]=='.')
                        break;
                
                if(i==0 || strcmp(optarg+i,".fa")!=0)
                {
                    fprintf(stderr,"WARNING: the file %s, specified in the option -r|--ref does not seem to be a fasta .fa file\n",optarg);
                }
                
                chrsfile=malloc(sizeof(char)*(auxsize+5)); //Max size, overstimation
                strcpy(chrsfile,optarg);
                strcpy(chrsfile+i,".dict");
                chrtype=DICT;
                break;
                
            case 'd':
                chrsfile=malloc(sizeof(char)*(strlen(optarg)+1));
                strcpy(chrsfile,optarg);
                chrtype=DICT;
                break;
                
            case 'c':
                chrsfile=malloc(sizeof(char)*(strlen(optarg)+1));
                strcpy(chrsfile,optarg);
                chrtype=LIST;
                break;
                
//            case 't': //TODO pending to implement
//                break;
                
            case ':':   /* missing option argument */
                fprintf(stderr, "%s: option `-%c' requires an argument\n",
                        argv[0], optopt);
                help=1;
                break;
                
            case '?':
                /* getopt_long already printed an error message. */
            default:
                fprintf(stderr, "%s: option `-%c' is invalid: ignored\n",
                        argv[0], optopt);
                break;
        }
    }
    
    // Remaining command line arguments (not options)
    if (optind < argc)
    {
        while (optind < argc)
        {
            if(nfiles==n_allocatedfiles)
            {
                files=realloc(files, sizeof(char*)*++n_allocatedfiles);
            }
            files[nfiles]=malloc(sizeof(char)*strlen(argv[optind]));
            strcpy(files[nfiles++], argv[optind++]);
        }
    }
    
    if(help==1)
    {
        fprintf(stderr,"%s\n",usage);
        return 0;
    }
    else
    {
        //TODO Check that required arguments are specified
        if(nfiles>WBITS)
        {
            fprintf(stderr, "ERROR: The current implementation of this program does only allow to work with %d files in this specific machine\n",WBITS);
            return -1;
        }
        if(nfiles<1)
        {
            fprintf(stderr,"ERROR: No input files detected\n\n");
            fprintf(stderr,"%s\n",usage);
            return -1;
        }
        if(maxfilt<=minfilt)
        {
            fprintf(stderr,"ERROR: Filters are not specified correctly, please revisit your -l|--min, -r|--max, and -b|--by options\n\n");
            fprintf(stderr,"%s\n",usage);
            return -1;
        }
        if(chrtype==-1)
        {
            fprintf(stderr,"ERROR: File with chromosome order (-r|--ref, -d|--dict , or -c|--list is required\n");
            fprintf(stderr, "%s\n",usage);
            return -1;
        }
        
        filehandles=malloc(sizeof(FILE *)*nfiles);
        for(i=0; i<nfiles; ++i)
        {
            filehandles[i]=smartfopen(files[i],"r");
            if (filehandles[i]==NULL)
            {
                fprintf(stderr,"ERROR: Error opening input file %s\n",files[i]);
                return -1;
            }
        }
        
        chr_files=smartfopen(chrsfile,"r");
        if (chr_files==NULL)
        {
            fprintf(stderr,"ERROR: Error opening chromosome input file input file %s\n",chrsfile);
            return -1;
        }
    }
    
    //Making filters
    int nfilters=ceil((maxfilt-minfilt)/(float)byfilt) + 1;
    int *filters = malloc(sizeof(int)*nfilters);
    j=0;
    
    for(i=minfilt;i<=maxfilt;i=i+byfilt)
    {
        filters[j++]=(int)i;
    }
    
    if(filters[j-1]!=maxfilt)
    {
        filters[j]=maxfilt;
    }
    
    
    //Output input options
    printf("CombDepth v0.1\n-----------------\nInput files:\n\t%s",files[0]);
    for(i=1; i<nfiles;++i)
    {
        printf("\n\t%s",files[i]);
    }
    printf("\nChromosome order extracted from file: %s\nFilters:\n\t>=%d",chrsfile,filters[0]);
    for(i=1;i<nfilters;++i)
    {
        printf("\n\t>=%d",filters[i]);
    }
    printf("\nOutput file: %s\n\n",outputfile);

    //Memory allocation and initialization
    
    //Tracking variables
    char **cchr;//For the future
    cchr=malloc(sizeof(char*)*nfiles);
    
    long *cpos;
    cpos=calloc((size_t) nfiles, sizeof(long));

    int *cdepth;
    cdepth=calloc((size_t) nfiles, sizeof(int));
    
    //Output variables
    long ** results=NULL;
    
    results=malloc(sizeof(long *)*(nfiles+1));
    
    for(i=0; i<=nfiles; ++i)
    {
        results[i]=calloc(nfilters,sizeof(long));
    }

    //Input data
    
    //Loading list of chromosomes
    char **chrs;
    int nkchrs=0;
    int alloc_chrs=100;
    int alloc_chrs_step=10;
    int MAX_CHR_SIZE=0;
    char *format_sscanf=NULL;
    ssize_t nchar=0;
    char * linep=NULL;

    chrs=malloc(sizeof(char*)*alloc_chrs);
    
    do
    {
        if (nchar>0) //skips empty lines
        {
            if(nkchrs==alloc_chrs)
            {
                chrs=realloc(chrs,sizeof(char*)*(alloc_chrs+alloc_chrs_step));
                alloc_chrs+=alloc_chrs_step;
            }
            if(sizestring<strlen(line))
            {
                string=realloc(string, strlen(line)+1);
                sizestring=strlen(line)+1;
            }
            
            if(chrtype==LIST)
            {
                strncpy(string,line,strlen(line)-1);
            }
            else
            {
                linep=NULL;
                for(i=0;i<=strlen(line);++i)
                {
                    if(line[i]==':' && line[i-1]=='N' && line[i-2]=='S')
                    {
                        linep=line+(++i);
                        break;
                    }
                }
                for(j=1;j+i<=strlen(line);++j)
                {
                    if(linep[j]=='\t' || linep[j]==' ' || linep[j]=='\n' || linep[j]=='\0')
                    {
                        break;
                    }
                }
                if(linep!=NULL)
                {
                    strncpy(string,linep,j);
                    string[j]='\0';
                }
                else
                {
                    nchar=getline(&line,&lengthlineptr,chr_files);
                    continue;
                }
            }
            if(strlen(string)+1>MAX_CHR_SIZE)
            {
                MAX_CHR_SIZE=(int)strlen(string)+1;
            }
            chrs[nkchrs]=malloc(sizeof(char)*(strlen(string)+1));
            strcpy(chrs[nkchrs], string);
            ++nkchrs;
        }
        nchar=getline(&line,&lengthlineptr,chr_files);

    }while(nchar!=-1);
    
    //Count number of digits of max_chr_size-1
    int ndig=0;
    j=MAX_CHR_SIZE-1;
    while(j != 0)
    {
        j /=10;
        ++ndig;
    }
    
    format_sscanf=malloc(sizeof(char)*(ndig+12+1));
    snprintf(format_sscanf, 10+ndig, "%%%ds\t%%ld\t%%d",MAX_CHR_SIZE-1);//This will be used multiple times to scan the data from the tsv files. We limit the size of the chr strings with this.
    
    //Initializing memory where the current chr for each file will be stored, now that we know the maximum chr size
    for(i=0;i<nfiles;++i)
    {
        cchr[i]=malloc(sizeof(char)*MAX_CHR_SIZE);
    }
    
    fclose(chr_files);
    
    // Reading first line of TSV inputs
    
    for(i=0; i<nfiles; ++i)
    {
        do{
            nchar=getline(&line,&lengthlineptr, filehandles[i]); //Returns the number of characters, I may want to use this
        }while(nchar==0); //Eliminating empty lines
        
        if (nchar!=-1)
        {
            sscanf(line,format_sscanf,cchr[i],&cpos[i],&cdepth[i]);
        }
        else
        {
            fprintf(stderr,"ERROR: Error reading input file %s\n\n",files[i]);
            return -1;
        }
    }
    
    //Masks
    //Masks indicate the state on the file nbit starting from the right for nfiles. The rest bits should be 0
    unsigned long long chrmask;
    unsigned long long posmask;
    unsigned long long posvalidfilesmask;
    unsigned long long validfilesmask;
    unsigned long long unfinishedmask=0;
    for(i=0; i<nfiles;++i)
    {
        unfinishedmask |= (1<<i);
    }

    // Control variables
    int stop=0;
    i=0;
    j=0;
    int nchr=0;
    int nvchrs=0;
    char *chr=chrs[nchr]; //
    long pos=0;
    int ifilt=0;
    int nvalidfiles=0;
    int npassthisfilter=0;
    long min=LONG_MAX;
    
    while(stop < nfiles && i<MAX_IT)
    {
        do
        {
            chrmask=0;
            nvchrs=0;
            for(j=0;j<nfiles;++j)
            {
                if((unfinishedmask & (1<<j))!= 0 && strcmp(cchr[j],chr)==0)
                {
                    chrmask |= 1<<j;
                    ++nvchrs;
                }
            }
            if(nvchrs==0)
            {
                ++nchr;
                chr=chrs[nchr];
            }
            if(nchr>=nkchrs)
            {
                printf("Unknown chromosomes present in the input files. Make sure that the input reference is the same you used to generate the bams you run with samtools depth\n");
                return -1;
            }
            
        }while(nvchrs==0);
        
        min=LONG_MAX;
        for(j=0; j<nfiles; ++j)
        {
#ifdef DEBUG
            printf("Sample %ld, chr %s, is chr %s and pos < %ld?",j, cchr[j],chr,min);
#endif
            if(((chrmask & (1<<j)) != 0) && cpos[j]<min)
            {
                min=cpos[j];
#ifdef DEBUG
                printf("\n\tYes, pos %ld, minpos %ld\n",cpos[j],min);
            }
            else
            {
                printf("\n\tNo, pos %ld, minpos %ld\n",cpos[j],min);
#endif
            }
        }
        pos=min;
        posmask=0;
        for(j=0;j<nfiles;++j)
        {
            if(((chrmask & (1<<j)) != 0) && cpos[j]==pos)
            {
                posmask |= 1<<j;
            }
        }
        
        validfilesmask=chrmask & posmask;
        posvalidfilesmask=validfilesmask;
        nvalidfiles=countsetbits(validfilesmask);
        
        for (ifilt=0; ifilt< nfilters; ++ifilt) //desired depth
        {
            npassthisfilter=0; //number of samples that pass this filter
            
            for(j=0; j<nfiles; ++j)
            {
                if((validfilesmask & (1<<j)) != 0 )//Sample that is not masked out
                {
                    if(cdepth[j]>=filters[ifilt]) //Depth comparison
                    {
                        ++npassthisfilter;
                    }
                    else
                    {
                        validfilesmask &= ~(1<<j);//this sample does not have enough coverage at this level and therefore it does not need to be checked again for this position
                        --nvalidfiles;
                    }
                }
            }
            for (j=1; j<=npassthisfilter; ++j)
            {
                ++results[j][ifilt]; //Results for this position
            }
        }

        for(j=0;j<nfiles;++j)
        {
            if((posvalidfilesmask & (1<<j)) != 0 ) //If this position was valid
            {
                do
                {
                    nchar=getline(&line,&lengthlineptr, filehandles[j]);
                }while(nchar==0);
                
                if (nchar!=-1)
                {
                    sscanf(line,format_sscanf,cchr[j],&cpos[j],&cdepth[j]);
                }
                else
                {
                    unfinishedmask &= ~ (1<<j);
                    ++stop;
                }
                    
            }
        }
        if (i%1000000==0)
        {
            printf("Iteration number %ld: chr %s, pos %ld\n",i,chr,pos);
        }
        ++i;
    }
    if (i>=MAX_IT)
    {
        printf("ERROR: Maximum number of iteractions reached. This is not expected to happen and the output will be truncated. You can modify the variable MAX_IT in the source code to increase the number of iteractions, but it has been set so that a whole human genome can be parsed without any problems. Please, check that the input format is correct before performing the suggested modification in the source code\n");
    }
    
    //Print results

    //Matrix output
    char *header; //Getting
    char *filtname;
    j=0;
    ndig=3;
    int newndig;
    int lenheader=1;
    int alloc_head_step=10;
    int maxsizeheader=(ndig+2+1)*nfilters;
    
    filtname=malloc(sizeof(char)*(ndig+1));
    header=malloc(sizeof(char)*(ndig+2+1)*nfilters); //approximation
    snprintf(header,8+1,"Sharedby");
    lenheader+=8;
    
    for(i=0;i<nfilters;++i)
    {
        newndig=0;
        j=filters[i];
        while(j != 0)
        {
            j /=10;
            ++newndig;
        }
        
        if(newndig>ndig)
        {
            filtname=realloc(filtname, sizeof(char)*(newndig+3));
            ndig=newndig;
        }
        snprintf(filtname,ndig+2+1,">=%d",filters[i]);
        if(lenheader+strlen(filtname)+1>=maxsizeheader)
        {
            header=realloc(header,sizeof(char)*(maxsizeheader+alloc_head_step));
            maxsizeheader+=alloc_head_step;
        }
        snprintf(header,ndig+2+1+strlen(header),"%s,%s",header,filtname);
        lenheader+=strlen(filtname)+1;
    }
    printf("%s\n",header);
    for (i=1; i <= nfiles; ++i)
    {
        printf("%ld",i);
        for (j=0; j<nfilters; ++j)
        {
            printf(",%ld",results[i][j]);
        }
        printf("\n");
    }
    
    //Tidy output (i.e., R-style)
    FILE * ofilehand;
    
    ofilehand=fopen(outputfile,"w");
    fprintf(ofilehand, "%s,%s,%s\n","bp","mindepth","nsamples");
    for (i=1; i<= nfiles; ++i)
    {
        for(j=0; j<nfilters; ++j){
            fprintf(ofilehand,"%ld,%d,%ld\n",results[i][j],filters[j],i);
        }
    }
    fclose(ofilehand);
    
    return 0;
}

//Functions

//By criptych. Found at https://stackoverflow.com/a/3974138/9761847
void printBits(size_t const size, void const * const ptr)
{
    unsigned char *b = (unsigned char*) ptr;
    unsigned char byte;
    long i, j;
    
    for (i=size-1;i>=0;i--)
    {
        for (j=7;j>=0;j--)
        {
            byte = (b[i] >> j) & 1;
            printf("%u", byte);
        }
    }
    puts("");
}

//Found at http://graphics.stanford.edu/~seander/bithacks.html#CountBitsSetTable by several people and modifications
int countsetbits(unsigned long long v)
{
    v = v - ((v >> 1) & (unsigned long long)~(unsigned long long)0/3);                           // temp
    v = (v & (unsigned long long)~(unsigned long long)0/15*3) + ((v >> 2) & (unsigned long long)~(unsigned long long)0/15*3);      // temp
    v = (v + (v >> 4)) & (unsigned long long)~(unsigned long long)0/255*15;                      // temp
    int c = (unsigned long long)(v * ((unsigned long long)~(unsigned long long)0/255)) >> (sizeof(unsigned long long) - 1) * CHAR_BIT; // count
    return c;
}

//Based on Fernando Mut's solution at https://stackoverflow.com/a/21755701/9761847
FILE *smartfopen(const char *path, const char *mode)
{
    char gzipid[4]=".gz";
    gzFile zfp;
    
    if(strcmp(path+strlen(path)-3,gzipid)==0)//If gzip
    {
        zfp = gzopen(path,mode);
        if (zfp == NULL)
        {
            printf("ERROR: file %s, detected as gzipped cannot be oppened\n",path);
            return NULL;
        }
        
        /* open file pointer */
        return funopen(zfp,
                       (int(*)(void*,char*,int))gzread,
                       (int(*)(void*,const char*,int))gzwrite,
                       (fpos_t(*)(void*,fpos_t,int))gzseek,
                       (int(*)(void*))gzclose);
    }
    else //otherwise
    {
        return fopen(path,mode);
    }


}

