#include "thPageMemory.h"
//#define BOOK_SIZE 256 //define a maximum book size
//#define LAST_PAGE (BOOK_SIZE-1)
//#define PAGE_SIZE 256 //

//due to data allignment, struct should be packed without leavig gaps, assuming int=4 bytes & pointer=8bytes on 64bit machine

//The page struct - expected size of struct thPage is (4x4)+8 = 40 bytes
typedef struct {
 int pgn;	//page number
 int siz;	//page size
 int nxtdp;	//next data page (page containing  data) (This is the id or element number of the 'book' array of pages)
 int prvdp;	//previos data page (page containing data) (This is the id or element number of the 'book' array of pages)
 void*pg;	//page with data
} thPage;


//The book (array of pages) - expected size is (24 x BOOK_SIZE) + 4
typedef struct {
 thPage * pages;
 int pgcnt;
} thBook;

/********************************************************
*		Page Allocate - thPAlloc()		*
*********************************************************
* Page Allocate looks through the book to find a page   *
* which is blank i.e. not allocated. It will allocate   *
* memory according to the 'lenb' parameter returning    *
* via parameter 'pgid' id number for the allocated page *
*							*
* Unallocated pages have a zero length in the struct    *
* and a NULL pointer.					*
* Internally (see thPageMemory.h) there is a maximum    *
* page length  'PAGE_SIZE' and page count 'BOOK_SIZE'   *
*							*
* Once an unallocated page is found the loop will quit  *
* immediately.						*
* If after all pages have been checked, no more pages   *
* are available 'pgid' is set to (-1) and the an error  *
* is returned.						*
* Pages should be deallocated and reallocated as needed *
*							*
* Note: the length to allocate is specified in bytes    *
*********************************************************/
int thPAlloc(thBook*book, int*pgid, int lenb){	//lenb is the amount of bytes to be allocated to the page as requested by calling function
  int err=0;
  int page_num=0;
  if(book->pgcnt<BOOK_SIZE){
   if( (lenb<=0) || (lenb>PAGE_SIZE) ){
    err=FAIL_ALLOCATE_LIMIT;
   } else {
    while(page_num<=book->pgcnt){
     if(book->pages[page_num].siz==0){
      //allocate memory and quit the loop
      book->pages[page_num].pg=calloc(1,lenb);
      if(book->pages[page_num].pg!=NULL){
       book->pgcnt+=1;
       book->pages[page_num].siz=lenb;
       *pgid=page_num;
      } else {
       err=FAIL_ALLOCATE_MEMORY;
      }
      break; //quit for loop
     } else {
      //go to next empty page
      page_num=book->pages[page_num].nxtdp;
     }
    }
   }
  } else {
   err=FAIL_ALLOCATE_NOPAGE;
  }
 return err;
}

//Deallocate needs to set the page size to zero
//condition of pgid>pgcnt is wrong because
// page id  could legitimately be larger than the count of pages
int thDAlloc(thBook*book, int pgid){
 int err=0;
 if( (pgid<0)||(pgid>=BOOK_SIZE) ){
  err=FAIL_PGNE;
 } else {
  if( (book->pages[pgid].pg!=NULL) && (book->pages[pgid].siz>0) ){
   free(book->pages[pgid].pg);
   book->pages[pgid].siz=0;
   book->pgcnt-=1;
  } else if( (book->pages[pgid].pg!=NULL) != (book->pages[pgid].siz>0) ){
   printf("Help! I don't know what to do!");
  }
 }
 return err;
}

/****************************************************************
*			Page Read - thRead()			*
*****************************************************************
* Page Read copies the entire page into the supplied buffer	*
* The insuffient space will result in an error.			*
* Attempting to read from an unallocated page give an errror	*
*****************************************************************/
int thRead(thBook*book, int pgid, int word_len, void*words){
 void*pRet;
 int err=0;
 if( (book->pages[pgid].siz>0) && (book->pages[pgid].pg!=NULL) ){
  if(word_len>=book->pages[pgid].siz){
   pRet=memcpy(words, book->pages[pgid].pg, book->pages[pgid].siz);
   if(pRet!=words){
    err=FAIL_READ_MEMCPYUDEF;
   }
  } else {
   //error too short
   err=FAIL_READ_TOOSHORT;
  }
 } else {
  //error not allocated
  err=FAIL_READ_NOTALLOCATED;
 }
 return err;
}

/********************************************************
*		Page Write - thWrite()			*
*********************************************************
* Page Write copies data from the supplied buffer into  *
* the page. If the supplied buffer is too long it will	*
* give an error. 					*
* A buffer shorter than the page will be written, but	*
* the page will not truncate. If a later read happens	*
* the whole page will be read not just the last write.	*
* Thefore the page should be allocated correctly before *
* writing. To resize the page, first call DEALLOCATE	*
* then ALLOCATE						*
*********************************************************/
int thWrite(thBook*book, int pgid, int word_len, void*words){
 void*pRet;
 int err=0;
 
 if( (book->pages[pgid].siz>0) && (book->pages[pgid].pg!=NULL) ){
  if( word_len <= book->pages[pgid].siz ){
   pRet=memcpy(book->pages[pgid].pg, words, word_len);
   if(pRet!=book->pages[pgid].pg){
    err=FAIL_WRITE_MEMCPYUDEF;
   }
  } else {
   //error too long
   err=FAIL_WRITE_TOOLONG;
  }
 } else {
  //error not allocated
  err=FAIL_WRITE_NOTALLOCATED;
 }
 return err;
} 

//Add read and write words to a page
// thWRead() and thWWrite()

/****************************************************************
*		The Page Manager - thPgMgr()			*
*****************************************************************
* The page manager function is used to initialise the 'book'	*
* memory and to manage access to the pages within the book. All *
* calls to the management functions ALLOCATE, READ, WRITE, and  *
* DEALLOCATE should originate from the Page Manager.		*
*								*
* Page Manager requires a pointer to a memory location to into  *
* which it will copy the desired page.
* Page Manager wil copy the lesser of 'len' or 'thPage.siz'	*
* however the actual copy will be performed by READ.		*
* Likewise for ALLOCATE, WRITE and DEALLOCATE.			*
* 								*
* The memory location and action is by 'command' and 'pg_id'	*
* In the case of DEALLOCATE the length parameter is ignored.	*
* In the case of ALLOCATE the pg_id paramter is ignored.	*
*								*
* Page Manager error codes are in the range 0x1000 to 0xF000 	*
* When ALLOCATE is invoked Page Manager will return the id of	*
* the allocated page, which is used for future READ, WRITE and	*
* DEALLOCATE requests. 						*
****************************************************************/
int thPgMgr(int command, int pg_id, int len, void*words){

 //book1 of type thBook is an array of structs type thPage
 static thBook book1;
 static int initState=0;
 int new_pg=0;
 int err=0;



 //Dont realocate memory if it already been done
 if(book1.pages==NULL){

  #ifdef DEBUG
  PRINT_DEBUG(DEBUG_ENABLED,"Assign List Memory");
  assert(book1.pages==NULL); 
  #endif
  book1.pages=(thPage *)calloc(BOOK_SIZE , sizeof(thPage)); 
  book1.pgcnt=0;
 }

 //Only initial the book once
 if(initState!=1){
  for(int i=0;i<BOOK_SIZE;i++){
   book1.pages[i].pgn=i;
   book1.pages[i].siz=0;
   book1.pages[i].nxtdp=( i==(BOOK_SIZE-1) ? 0 : i+1 );
   book1.pages[i].prvdp=( i==0 ? BOOK_SIZE-1 : i-1 );
   book1.pages[i].pg=NULL;
  }
  initState=1;
 }

//Call appropriate function based on command
 switch(command){
  case ALLOCATE:
   PRINT_DEBUG(DEBUG_ENABLED,"Allocate Invoked");
   err = thPAlloc(&book1, &new_pg, len);
  break;
  case READ:
   err=PG_LIMIT(pg_id);
   if(!err){
    err=thRead(&book1, pg_id, len, words);
   }
  break;
  case WRITE:
   PRINT_DEBUG(DEBUG_ENABLED,"Write Invoked");
   err=PG_LIMIT(pg_id);
   if(!err){
    err=thWrite(&book1, pg_id, len, words);
   }
  break;
  case DEALLOCATE:
   PRINT_DEBUG(DEBUG_ENABLED,"Deallocate Invoked");
   err=PG_LIMIT(pg_id);
   if(!err){
    err=thDAlloc(&book1, pg_id);
   }
  break;
  default:
   printf("\nNo such command.\n");
   err=UNKNOWN;
  break;
 }
 return ( err==0 ) ? new_pg : err ;
}

/****************************************
*	Page Error - thPgError()	*
*****************************************
* Page Error is provided to convert the *
* error number into a text description.	*
*****************************************/
void thPgError(int err){
 int err_array=(err&0xF000);
 int err_num=(err>>8)&0xF;

 switch(err_array){
 case 0:
  printf("\nNo Error\n");
  break;
 case 0x1000:
  printf("\n%s\n",s1_fail[err_num]);
  break;
 case 0x2000:
  printf("\n%s\n",s2_fail[err_num]);
  break;
 case 0x3000:
  printf("\n%s\n",s3_fail[err_num]);
  break;
 case 0x4000:
  printf("\n%s\n",s4_fail[err_num]);
  break;
 case 0x5000:
  printf("\n%s\n",s5_fail[err_num]);
  break;
 default:
  printf("\nNo description available.\n");
  break;
 }
}

int main(){
 void*w;
 int pg1, pg2, pg3, pg4;
 
long int64_32=4294967296;
 
 int int32_16=65536;
 int int32_16_out=0;

 int int32_24=16777216;
 char*str="abc123\0";
 int err;
/*
 PRINT_DEBUG(DEBUG_ENABLED,"1. Assign int64");
 pg1=thPgMgr(ALLOCATE, 20, sizeof(int), &int32_16);
 printf("\nAllocate Result: %x\n",pg1);
 thPgError(pg1);

 PRINT_DEBUG(DEBUG_ENABLED,"2a. Valid Writing to Page");
 err=thPgMgr(WRITE,pg1, sizeof(int), &int32_16);
 printf("\nresult 2a: %x\n",err);
 thPgError(err);
 PRINT_DEBUG(DEBUG_ENABLED,"2b. Valid Read from Page");
 err=thPgMgr(READ,pg1, sizeof(int), &int32_16_out);
 printf("\nresult 2b: %d\n",int32_16_out);
 thPgError(err);

 int32_16_out=0;
 PRINT_DEBUG(DEBUG_ENABLED,"3a. Write to Unallocated Page");
 err=thPgMgr(WRITE, 6, sizeof(int), &int32_16);
 printf("\nResult 3b: %x\n",err);
 thPgError(err);
 PRINT_DEBUG(DEBUG_ENABLED,"3b. Read From Unallocated Page");
 err=thPgMgr(READ, 6, sizeof(int), &int32_16_out);
 printf("\nResult 3b: %d\n",int32_16_out);
 thPgError(err);

 int32_16_out=0;
 PRINT_DEBUG(DEBUG_ENABLED,"4a. Write too many bytes to Page");
 err=thPgMgr(WRITE, pg1,sizeof(long), &int32_16);
 printf("\nResult 4a: %x\n",err);
 thPgError(err);
 PRINT_DEBUG(DEBUG_ENABLED,"4b. Read too many bytes to Page");
 err=thPgMgr(READ, pg1,sizeof(long), &int32_16_out);
 printf("\nResult 4b: %d\n",int32_16_out);
 thPgError(err);

 int32_16_out=0;
 PRINT_DEBUG(DEBUG_ENABLED,"5a. Write too few bytes to Page");
 err=thPgMgr(WRITE, pg1,sizeof(char), &int32_16);
 printf("\nResult 5a: %x\n",err);
 thPgError(err);
 PRINT_DEBUG(DEBUG_ENABLED,"5b. Read too few bytes from Page");
 err=thPgMgr(READ, pg1,sizeof(char), &int32_16_out);
 printf("\nResult 5b: %d\n",int32_16_out);
 thPgError(err);

 int32_16_out=0;
 PRINT_DEBUG(DEBUG_ENABLED,"6a. Invalid Page ID");
 err=thPgMgr(WRITE,500,sizeof(int), &int32_16);
 printf("\nResult 6a: %x\n",err);
 thPgError(err);
 PRINT_DEBUG(DEBUG_ENABLED,"6b. Invalid Page ID");
 err=thPgMgr(READ,-500, sizeof(int), &int32_16_out);
 printf("\nResult 6b: %d\n",int32_16_out);
 thPgError(err);

 PRINT_DEBUG(DEBUG_ENABLED,"7. Deallocate");
 err=thPgMgr(DEALLOCATE,pg1,20,&int64_32);
 thPgError(err);
*/
 PRINT_DEBUG(DEBUG_ENABLED,"8. Allocate too many bytes to a page");
 pg2=thPgMgr(ALLOCATE,1000,500,w);
 printf("\nResult  8: %x",pg2);
 thPgError(pg2);


 PRINT_DEBUG(DEBUG_ENABLED,"9. Allocated too many pages");
 for (int i=0;i<(BOOK_SIZE+10);i++){
 printf("Round %d\n",i);
 pg3=thPgMgr(ALLOCATE,100,100,w);
 thPgError(pg3);
 }

 PRINT_DEBUG(DEBUG_ENABLED,"10 Deallocate eights");
 for (int i=0;i<(BOOK_SIZE);i+=8){
 printf("Round %d\n",i);
 pg3=thPgMgr(DEALLOCATE,i,100,w);
 thPgError(pg3);
 }

 PRINT_DEBUG(DEBUG_ENABLED,"11. Allocated fours");
 for (int i=0;i<(BOOK_SIZE);i+=4){
 printf("Round %d\n",i);
 pg3=thPgMgr(ALLOCATE,i,100,w);
 thPgError(pg3);
 }

 PRINT_DEBUG(DEBUG_ENABLED,"12 Deallocate all.");
 for (int i=0;i<(BOOK_SIZE);i++){
 printf("Round %d\n",i);
 pg3=thPgMgr(DEALLOCATE,i,100,w);
 thPgError(pg3);
 }
}
