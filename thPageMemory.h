#ifndef THSTDINCLUDE
 #include <stdlib.h>
 #include <stdio.h>
 #include <errno.h>
 #include <string.h>
#endif

#define DEBUG_ENABLED 1
#define PRINT_DEBUG(on,str) ( (on!=0) ? printf("\n%s\n",str) : (0) )
#define DEBUG
#ifdef DEBUG
 #include <assert.h>
#endif
#define THSTDINCLUDE

#define BOOK_SIZE 0x100
#define PAGE_SIZE 0x100
#define ALLOCATE  0x10
#define READ 0x20
#define WRITE 0x30
#define DEALLOCATE 0x40

#define FAIL_ALLOCATE 0x1000
#define FAIL_ALLOCATE_LIMIT 0x1100
#define FAIL_ALLOCATE_MEMORY 0x1200
#define FAIL_ALLOCATE_NOPAGE 0x1300
const char s1_fail[4][65]={"FAIL_ALLOCATE:Page allocation failed\0","FAIL_ALLOCATE_LIMIT:Requested page length exceed internal limits\0","FAIL_ALLOCATE_MEMORY:System returned null pointer\0","FAIL_ALLOCATE_NOPAGE:There are no more blank pages\0"};

#define FAIL_READ 0x2000
#define FAIL_READ_TOOSHORT 0x2100
#define FAIL_READ_NOTALLOCATED 0x2200
#define FAIL_READ_MEMCPYUDEF 0x2300
const char s2_fail[4][65]={"FAIL_READ:Failed to read page\0","FAIL_READ_TOOSHORT:Page data would overrun destination buffer.\0","FAIL_READ_NOTALLOCATED:The page has no buffer allocated\0","FAIL_READ_MEMCPYUDEF:Page data is in undefined state.\0"};

#define FAIL_WRITE 0x3000		//General WRITE error
#define FAIL_WRITE_TOOLONG 0x3100	//WRITE was given data longer than allocated page
#define FAIL_WRITE_NOTALLOCATED 0x3200	//Page not Allocated to the ID
#define FAIL_WRITE_MEMCPYUDEF 0x3300	//library memcpy result was undefined
const char s3_fail[4][65]={"FAIL_WRITE:Failed to write page\0","FAIL_WRITE_TOOLONG:Input data would overrun destination page.\0","FAIL_WRITE_NOTALLOCATED:The page has no buffer allocated\0","FAIL_WRITE_MEMCPYUDEF:Page data is in undefined state.\0"};

#define FAIL_DEALLOCATE 0x4000
const char s4_fail[1][65]={"FAIL_DEALLOCATE:The page does not exist\0"};

#define FAIL_PGNE 0x5000 //Page not exist error
const char s5_fail[1][65]={"FAIL_PGNE:The Page does not exist\0"};

#define UNKNOWN 0xF000

#define PG_LIMIT(id) ( ( (id<0)||(id>=BOOK_SIZE) ) ? (FAIL_PGNE) : (0) )


