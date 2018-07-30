#include "types.h"
#include "user.h"
#include "fcntl.h"

void checkgettag(int fd,char* key)
{
    char buf[50];
    if (gettag(fd, key,buf) > 0)
        printf(1, "gettag succeded, key =  %s, val = %s\n",key,buf);
    else
        printf(2, "gettag failed, key = %s \n",key);
}


void checkuntag(int fd,char* key)
{
    if (funtag(fd, key) == 0)
        printf(1, "funtag succeded key = %s \n",key);
    else
        printf(2, "funtag failed key = %s \n",key);
}

int get_tag_test(int fd)
{   
    printf(1,"\nTest for get tag\n");
    ftag(fd, "os", "182");
    checkgettag(fd,"os");
    ftag(fd, "x", "y");
    checkgettag(fd,"x");
    printf(1,"\n\n");
    return 0;
}


int value_override_test(int fd)
{   
    printf(1,"Test for value override\n");
    
    ftag(fd, "key3", "value3");
    checkgettag(fd,"key3");

    ftag(fd, "key3", "c3");
    checkgettag(fd,"key3");

    printf(1,"\n\n");
    return 0;
}

int test_remove_key_get_failed(int fd)
{
    printf(1,"Test for gettag after untag\n"); 
    printf(1,"# tagging\n");
    ftag(fd, "123456789", "somevalue");
    checkgettag(fd, "123456789");
    printf(1,"# untagging\n");
    checkuntag(fd, "123456789");
    printf(1,"# gettaging the key which was untagged\n");
    char buf[50];
    if (gettag(fd,"123456789",buf) < 0)
    {
        printf(2,"succeded - key is really doen't exists\n");
    }
    else
    {
        printf(2,"failed\n");
    }

    return 0;
}

int
main (int argc, char *argv[])
	{
	int fd;

	if ((fd = open("hellooo", O_CREATE | O_RDWR)) < 0) 
	{
        printf(2,"ftag: open failed\n");
        exit();
	}
    get_tag_test(fd);
    value_override_test(fd);
    test_remove_key_get_failed(fd);
    
	close(fd);
	exit();
}