#include "types.h"
#include "stat.h"
#include "user.h"
#include "fs.h"
#include "fcntl.h"


#define _BUFSIZE 512
//Options
int flag_follow = 1;

//Tests
int got_name = 0; //False
char file_name[512]={0};

int file_size = -1;
int sign = 0;

char file_type = 'n';
int got_tag = 0; //False
char key[10] = {0};
char value[30] ={0};
int q_mark_flag = 1; //False



//function from ls.c
char*
fmtname(char *path)
{
  static char buf[DIRSIZ+1];
  char *p;

  // Find first character after last slash.
  for(p=path+strlen(path); p >= path && *p != '/'; p--)
    ;
  p++;

  // Return blank-padded name.
  if(strlen(p) >= DIRSIZ)
    return p;
  memmove(buf, p, strlen(p));
  memset(buf+strlen(p),0, DIRSIZ-strlen(p));
  return buf;
}

int 
filter_it(int fd,struct stat st, char *name) 
{
    int type = st.type;
    int size = st.size;
    char value_data[30];
    // if we got name
    if(got_name==1 && (strcmp(file_name, name) != 0) )
    {
       // printf(1,"GOT NAME\n");
        return -1; 
    }
    // if we got size
    if(file_size != -1)
    {
     //   printf(1,"GOT SIZE\n");
        switch(sign)
        {
            case 1: if(file_size  >= size) return -1; break;
            case -1: if(file_size  < size) return -1; break;
            default: if(file_size  != size) return -1; break;
        }
    }

    // if we got type
    if (file_type != 'n')
    {
      //  printf(1,"GOT TYPE %d \n",type);
        switch(file_type)
        {

            case 'd': if (type != T_DIR) return -1; break;
            case 'f': if (type != T_FILE) return -1; break; 
            case 's': if ((type != T_SYM)) return -1; break;

        }
    }

    // if we got tag value=tag
    if(got_tag ==1)
    {
      //  printf(1,"GOT TAG\n");
        if (gettag(fd,key,value_data) > 0) //if success
        {
            // if got NO "?" must check the val correctness
            if(q_mark_flag==1) 
            {
                //if the actual value not equal to the given value - byebye
                if(strcmp(value,value_data)!=0)
                {
                    return -1;
                }
            }
        }

        //if gettag failed - byebye
        else return -1;
    }

    //file is OK for the filter
    return 0;


}


void 
find(char* path, char *name) 
{
    char *p;
    char* buf=(char*)(malloc(sizeof(char)*_BUFSIZE));
    memset(buf,0,_BUFSIZE);
    int fd;
    struct dirent de;
    struct stat st;
    // in case for symlink
    char* sym_path=(char*)(malloc(sizeof(char)*_BUFSIZE));
    memset(sym_path,0,_BUFSIZE);


    if (flag_follow==0)//True
    {
      if((fd = open(path, 0)) < 0)
      {
              printf(2, "find: cannot open in O_NO_DERFRENCE%s\n", path);
              free(buf);
              free(sym_path);
              return;
      }
    }
    else
    {
      if((fd = open(path, O_NO_DERFRENCE)) < 0)
      {
              printf(2, "find: cannot open %s\n", path);
              free(buf);
              free(sym_path);
              return;
      }
    }

    if(fstat(fd, &st) < 0)
    {
        printf(2, "find: cannot stat %s\n", path);
        close(fd);

        free(buf);
        free(sym_path);
        return;
    }

    switch(st.type)
    {
        case T_FILE:
            if(filter_it(fd, st, name) == 0)
            {
                printf(1, "%s\n", path);
            }
            break;

        case T_SYM:
            if(filter_it(fd, st, name) == 0)
            {
                printf(1, "%s\n", path);
            }
        
            break;     
        case T_DIR:
            if(strlen(path) + 1 + DIRSIZ + 1 >  _BUFSIZE)
            {
                printf(1, "find: path too long\n");
                break;
            }
            //symlink or not - buf it
            strcpy(buf, path);
            if(filter_it(fd, st, fmtname(path))==0)
            {
                printf(1, "%s\n", buf);
            }
            p = buf+strlen(buf);
            if(strcmp(path,"/"))
            *p++ = '/';
            while(read(fd, &de, sizeof(de)) == sizeof(de))
            {
                if(de.inum == 0)
                    continue;
                memmove(p, de.name, DIRSIZ);
                p[DIRSIZ] = 0;
                //prevet inifinte loop
                if (de.name[0] == '.')
                    continue;
                {
                    find(buf, de.name);
                }

            }
            break;

    } //close switch

    close(fd);

     free(buf);
     free(sym_path);
    return;
}
int main(int argc, char *argv[])
{   
    int i;
    char tmp;
    //path is the ONLY mandatory argument
    if(argc < 2 || strcmp(argv[1], "-follow") ==0 || strcmp(argv[1], "-name") ==0
        || strcmp(argv[1], "-size") ==0 || strcmp(argv[1], "-type") ==0 || strcmp(argv[1], "-tag") ==0)
    {
        printf(1,"find: error less args\n");
        exit();
    }

    //start from 2 because argv[0]=find, argv[1]=pathONLY 
    for(i=2; i<argc; i++)
    {
        if(strcmp(argv[i], "-follow") == 0 )
        {   
            //printf(1,"got follow flag\n");

            flag_follow = 0;
        }
        else if(strcmp(argv[i], "-name") == 0 )
        {
            if(i+1==argc)
            {
                printf(2,"find: missing argument to `-name'\n");
                exit();
            }
            got_name = 1; //True
            memset(file_name,0,512);
            strcpy(file_name,argv[i+1]);
        }

        else if(strcmp(argv[i], "-size") ==0 )
        {   

            if(i+1==argc)
            {
                printf(2,"find: missing argument to `-size'\n");
                exit();
            }
            tmp = argv[i+1][0];
            switch(tmp)
            {
                case '+': sign = 1; file_size = atoi(&argv[i+1][1]); break;
                case '-': sign = -1; file_size = atoi(&argv[i+1][1]); break;
                default: sign = 0; file_size = atoi(&argv[i+1][0]); break;
            }
        }

        else if(strcmp(argv[i], "-type") ==0 )
        {
             if(i+1==argc)
            {
                printf(2,"find: missing argument to `-type'\n");
                exit();
            }

            tmp = argv[i+1][0];
            switch(tmp)
            {
                case 'd': file_type = 'd'; break;
                case 'f': file_type = 'f'; break;
                case 's': file_type = 's'; break;
                default: printf(2, "find: error invalid type: %c\n", tmp); exit();
            }
        }

        // -tag <key>=<value>
        else if(strcmp(argv[i], "-tag") ==0 )
        {
              if(i+1==argc)
            {
                printf(2,"find: missing argument to `-tag'\n");
                exit();
            }
            got_tag = 1; //True
            char* eq = strchr(argv[i+1],'=');
            *eq = 0;

            memset(key,0,10);
            memset(value,0,30);
            if (strcmp(eq+1, "?")==0)
            {   
                q_mark_flag = 0; //True
            }
            strcpy(key,argv[i+1]);
            strcpy(value,eq+1);


        }


    }

    find(argv[1], fmtname(argv[1]));
    exit();
}
