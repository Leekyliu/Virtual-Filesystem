#include <stdio.h>
#include <string.h>
#define TEST(x) test(x, #x)
#include "myfilesystem.h"

/* You are free to modify any part of this file. The only requirement is that when it is run, all your tests are automatically executed */

/* Some example unit test functions */
int success() {
    return 0;
}

int failure() {
    return 1;
}

/* Test init_fs*/

int no_operation() {
    void * helper = init_fs("test_data.txt", "test_dir.txt", "test_hash.txt", 1); // Remember you need to provide your own test files and also check their contents as part of testing
    char * filename = ((file *)helper)->filename;
    char * directory = ((file *)helper)->directory;
    char * hash = ((file *)helper)->hash;
    int core = ((file *)helper)->core;
    if(strcmp(filename,"test_data.txt")!=0 || strcmp(directory,"test_dir.txt")!=0 || strcmp(hash,"test_hash.txt")!=0 || core != 1)
        return 1;
    close_fs(helper);
    return 0;
}

/*Test create_file simple case*/

int create_file_test() {
    void * helper = init_fs("test_data.txt", "test_dir.txt", "test_hash.txt", 1);

    create_file("1.doc",100,helper);
    ssize_t size = file_size("1.doc",helper);
    
     if(size == 100)
     {
         close_fs(helper);
          return 0;
     } 

         close_fs(helper);

         return 1;
}

/*Test create an existsed file*/

int create_existsfile_test() {
    void * helper = init_fs("test_data.txt", "test_dir.txt", "test_hash.txt", 1);

   int result =  create_file("1.doc",100,helper);
    
    
     if(result == 1)
     {
         close_fs(helper);
          return 0;
     } 

         close_fs(helper);

         return 1;
}

/*Test create_file but insufficient space*/

int create_file_insufficient_test(){
 void * helper = init_fs("test_data.txt", "test_dir.txt", "test_hash.txt", 1);
 int result = create_file("2.txt",1000000000000000,helper);
 if(result == 2)
 {
     
  close_fs(helper);
  return 0;
     
 }
    
 close_fs(helper);
 return 1;
}

/*Test create_file reapck case*/

int create_file_repack_test(){
  void * helper = init_fs("test_data.txt", "test_dir.txt", "test_hash.txt", 1);
 create_file("14.txt",2000,helper);
 create_file("12.txt",2000,helper);
 create_file("13.txt",10,helper);
 delete_file("12.txt", helper);
 int result = create_file("3.txt",2010,helper);
 if(result==0){
  close_fs(helper);
  return 0;
 }
 close_fs(helper);
 return 1;

}

/*Test decress resize*/

int resize_file_test(){
void * helper = init_fs("test_data_resize.txt", "test_dir_resize.txt", "test_hash_resize.txt", 1);
 create_file("1.doc",1000,helper);
 resize_file("1.doc",500,helper);
 int size = file_size("1.doc",helper);
 if(size == 500){
  close_fs(helper);
  return 0;
 }
 close_fs(helper);
 return 1;
}

/*Test resize a not exists file*/

int resize_nofile_test(){
 void * helper = init_fs("test_data_resize.txt", "test_dir_resize.txt", "test_hash_resize.txt", 1);
 int result = resize_file("77.txt",5,helper);
 if(result == 1)
 {
  close_fs(helper);
  return 0;
 }
 close_fs(helper);
 return 1;
}

/*Test resize a file which cause insufficient space*/

int resize_insufficient_test(){
   
    void * helper = init_fs("test_data_resize.txt", "test_dir_resize.txt", "test_hash_resize.txt", 1);
    create_file("11.doc",1000,helper);
    int result = resize_file("11.doc",4000000000,helper);
 
   if(result == 2 )
   {
      close_fs(helper);
      return 0;
   }
  
   close_fs(helper);
   return 1;
}

/*Test simple increase size repack*/

int resize_repack1_test(){
   void * helper = init_fs("test_data_resize.txt", "test_dir_resize.txt", "test_hash_resize.txt", 1);
    create_file("44.doc",50,helper);
    resize_file("44.doc",1000,helper);
    size_t size = file_size("44.doc",helper);
    if(size == 1000)
    {
        close_fs(helper);
        return 0;
    }
  
    close_fs(helper);
    return 1;
}

/* Test increase size that affect next file offset*/

int resize_repack2_test(){
   void * helper = init_fs("test_data_resize.txt", "test_dir_resize.txt", "test_hash_resize.txt", 1);
    resize_file("11.doc",1010,helper);
    size_t size = file_size("11.doc",helper);
    if(size == 1010)
    {
        close_fs(helper);
        return 0;
    }
  
    close_fs(helper);
    return 1;
}

/*Test decrease size that doesn't affect next file offset*/

int resize_repack3_test(){
   void * helper = init_fs("test_data_resize.txt", "test_dir_resize.txt", "test_hash_resize.txt", 1);
    resize_file("11.doc",10,helper);
    size_t size = file_size("11.doc",helper);
    if(size == 10)
    {
        close_fs(helper);
        return 0;
    }
  
    close_fs(helper);
    return 1;
}

/*Test delete file simple case*/

int delete_test(){
   void * helper = init_fs("test_data_resize.txt", "test_dir_resize.txt", "test_hash_resize.txt", 1);
    delete_file("11.doc",helper);
    ssize_t size = file_size("11.doc",helper);
    if(size < 0)
    {
        close_fs(helper);
        return 0;
    }
  
    close_fs(helper);
    return 1;
}

/*Test delete a file twice*/

int delete_filetwice_test(){
   void * helper = init_fs("test_data_resize.txt", "test_dir_resize.txt", "test_hash_resize.txt", 1);
     delete_file("11.doc",helper);
    ssize_t size = file_size("11.doc",helper);
    if(size < 0)
    {
        close_fs(helper);
        return 0;
    }
  
    close_fs(helper);
    return 1;
}

/*Test delete a not existsed file*/

int delete_nofile_test(){
   void * helper = init_fs("test_data_resize.txt", "test_dir_resize.txt", "test_hash_resize.txt", 1);
    int result = delete_file("111.doc",helper);
    if(result == 1)
    {
        close_fs(helper);
        return 0;
    }
  
    close_fs(helper);
    return 1;
}

/*Test rename a not existsed file*/

int rename_nofile_test(){
   void * helper = init_fs("test_data.txt", "test_dir.txt", "test_hash.txt", 1);
  int result = rename_file("123.txt","456.txt",helper);
  if(result == 1)
  {
        close_fs(helper);
        return 0;
    }
    close_fs(helper);
    return 1;
    
}

/*Test rename a file but its new name existsed*/

int rename_new_exists_test(){
   void * helper = init_fs("test_data.txt", "test_dir.txt", "test_hash.txt", 1);
  int result = rename_file("14.txt","3.txt",helper);
  if(result == 1)
  {
        close_fs(helper);
        return 0;
    }
    close_fs(helper);
    return 1;
    
}

/*Test rename file simple case*/

int rename_test(){
   void * helper = init_fs("test_data.txt", "test_dir.txt", "test_hash.txt", 1);
  int result = rename_file("1.doc","31.txt",helper);
  if(result == 0)
  {
        close_fs(helper);
        return 0;
    }
    close_fs(helper);
    return 1;
    
}

/*Test read_file file simple case*/

int read_file_test(){
    void * helper = init_fs("test_data_read.txt", "test_dir_read.txt", "test_hash_read.txt", 1);
   
    create_file("00.doc",50,helper);
    char input[50];
    memset(input,'0',50);
    write_file("00.doc",0,50,input,helper);
    char buf[50];
   int result =  read_file("00.doc",0,50,buf,helper);
    if(result == 3)
    {
        close_fs(helper);
        return 0;
    }
    close_fs(helper);
    return 1;
}

/*Test read a not exists file*/

int read_file_nofile_test(){
     void * helper = init_fs("test_data_read.txt", "test_dir_read.txt", "test_hash_read.txt", 1);
     char buf[1];
    int result =read_file("2.doc",0,1,buf,helper);
    if(result == 1){
        close_fs(helper);
        return 0;
    }
    close_fs(helper);
    return 1;
}

/*Test read a file from an impossible offset*/

int read_file_offsetover_test(){
    void * helper = init_fs("test_data_read.txt", "test_dir_read.txt", "test_hash_read.txt", 1);
     char buf[1];
   create_file("1.doc",100,helper);
    int result = read_file("1.doc",1000,1,buf,helper);
    if(result == 2){
        close_fs(helper);
        return 0;
    }
    close_fs(helper);
    return 1;
}


/*Test read a file for an impossible length*/

int read_file_lenover_test(){
    void * helper = init_fs("test_data_read.txt", "test_dir_read.txt", "test_hash_read.txt", 1);
     char buf[1];
    int result =read_file("1.doc",1,1000,buf,helper);
    if(result == 2){
        close_fs(helper);
        return 0;
    }
    close_fs(helper);
    return 1;
}

/*Test write a not exists file*/

int write_nofile_test(){
     void * helper = init_fs("test_data_write.txt", "test_dir_write.txt", "test_hash_write.txt", 1);
     char input[10];
    memset(input,'0',10);
    int result = write_file("p.bin",2,12,input,helper);
    if(result == 1)
    {
        close_fs(helper);
        return 0;
    }
    close_fs(helper);
    return 1;
    
}

/*Test write a file which cause insufficient space*/

int write_file_insufficient_test(){
    void * helper = init_fs("test_data.txt", "test_dir.txt", "test_hash.txt", 1);
       char input[100000];
     memset(input,'0',100000);
    create_file("19.doc",100,helper);
    int result = write_file("19.doc",1,100000,input,helper);
    
    if(result == 3)
    {
        close_fs(helper);
        return 0;
    }
    close_fs(helper);
    return 1;
}

/*Test write a file simple case*/

int write_file_test(){
    void * helper = init_fs("test_data.txt", "test_dir.txt", "test_hash.txt", 1);
    char* input = "helloC";
    int result = write_file("19.doc",1,strlen(input),input,helper);
    if(result == 0)
    {
        close_fs(helper);
        return 0;
    }
    close_fs(helper);
    return 1;
    
}

/*Test write and increase size of file but no need repack*/

int write_file_increase1_test(){
  void * helper = init_fs("test_data_.txt", "test_dir.txt", "test_hash.txt", 1);
  char * input = "world123";
 int result = write_file("19.doc",6,strlen(input),input,helper);
     if(result == 0)
     {
        close_fs(helper);
        return 0;
    }
    close_fs(helper);
    return 1;
}

/*Test write and increase size of file need to repack*/

int write_file_test_increase2_test(){
   void * helper = init_fs("test_data_write.txt", "test_dir_write.txt", "test_hash_write.txt", 1);
    char input[150];
    memset(input,'0',150);
  create_file("1.doc",1000,helper);
  create_file("2.doc",1000,helper);
  create_file("3.doc",2000,helper);
  create_file("4.doc",50,helper);
    int result = write_file("1.doc",1000,10,input,helper);
    if(result == 0)
    {
        close_fs(helper);
        return 0;
    }
    close_fs(helper);
    return 1;
    
}

/*Test get a file size simple case*/

int file_size_test(){
   void * helper = init_fs("test_data.txt", "test_dir.txt", "test_hash.txt", 1);
   create_file("size.txt",5,helper);
   ssize_t size = file_size("size.txt",helper);
    if(size == 5)
    {
        close_fs(helper);
        return 0;
    }
    close_fs(helper);
    return 1;
}

/*Test hash_bock*/

int hash_block_test(){
   void * helper = init_fs("test_data_h.txt", "test_dir_h.txt", "test_hash_h.txt", 1);
    create_file("ppx.doc",1000,helper);
    char buf [496];
    FILE* fpt = fopen("test_hash_h.txt","rb+");
    fread(buf,1,496,fpt);
    fclose(fpt);
    compute_hash_tree(helper);
    char b [496];
    fpt = fopen("test_hash_h.txt","rb+");
    fread(b,1,496,fpt);
    fclose(fpt);
    if(strcmp(buf,b) == 0){
        close_fs(helper);
        return 0;
    }
    close_fs(helper);
    return 1;
    
}

/*Test get the size of a not exists file*/

int file_size_nofile_test(){
   void * helper = init_fs("test_data.txt", "test_dir.txt", "test_hash.txt", 1);
   ssize_t size = file_size("999.txt",helper);
    if(size < 0)
    {
        close_fs(helper);
        return 0;
    }
    close_fs(helper);
    return 1;
}
/****************************/

/* Helper function */
void test(int (*test_function) (), char * function_name) {
    int ret = test_function();
    if (ret == 0) {
        printf("Passed %s\n", function_name);
    } else {
        printf("Failed %s returned %d\n", function_name, ret);
    }
}


/************************/

int main(int argc, char * argv[]) {
    
    // You can use the TEST macro as TEST(x) to run a test function named "x"
    TEST(no_operation);
    TEST(success);
    TEST(failure);
    printf("Test cases\n");
  
    TEST(create_file_test);
    TEST(create_existsfile_test);
    TEST(create_file_insufficient_test);
    TEST(create_file_repack_test);
    TEST(resize_file_test);
    TEST(resize_nofile_test);
    TEST(resize_insufficient_test);
    TEST(resize_repack1_test);
    TEST(resize_repack2_test);
    TEST(resize_repack3_test);
    TEST(delete_test);
    TEST(delete_filetwice_test);
    TEST(delete_nofile_test);
    TEST(rename_nofile_test);
    TEST(rename_new_exists_test);
    TEST(rename_test);
    TEST(read_file_test);
    TEST(read_file_nofile_test);
    TEST(read_file_offsetover_test);
    TEST(read_file_lenover_test);
    TEST(write_nofile_test);
    TEST(write_file_insufficient_test);
    TEST(write_file_test);
    TEST(write_file_test_increase2_test);
    TEST(file_size_test);
    TEST(file_size_nofile_test);
    TEST(hash_block_test);
  
    // Add more tests here

    return 0;
}
