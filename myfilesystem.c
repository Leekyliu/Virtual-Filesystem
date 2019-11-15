#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include<pthread.h>
#include "myfilesystem.h"




pthread_mutex_t hash_update_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lock_repack = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t write_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lock_resize = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t hash_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t compute_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lock_delete = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lock_read = PTHREAD_MUTEX_INITIALIZER;








void update_hash_file(size_t start, size_t length, void * helper){
    
    
    
    pthread_mutex_lock(&hash_update_lock);
    int index = (int)(start/256);
    int end = (int)(start + length)/256; // get the block that the file end up
    
    for(int i = index; i <= end; i++)
    {
        
        compute_hash_block(i, helper); // update the hash block
        
    }
    pthread_mutex_unlock(&hash_update_lock);
    return;
    
}




void * init_fs(char * f1, char * f2, char * f3, int n_processors) {
    
    file* ptr=(file*)malloc(sizeof(file));
    
    strcpy(ptr->filename,f1);
    strcpy(ptr->directory,f2);
    strcpy(ptr->hash,f3);
    ptr -> core = n_processors;
    
    return (void*)ptr;
}



void close_fs(void * helper) {
    free(helper);
    return;
}




int create_file(char * filename, size_t length, void * helper) {
    
    
    
    pthread_mutex_lock(&lock);
    
    
    
    
    FILE* dp;
    file* help = (file*)(helper);
    char* directory = help->directory;
    dp = fopen(directory,"rb+");
    
    
    FILE* fp;
    char* data = help->filename;
    fp = fopen(data,"rb+");
    
    char buf[72]; //  used to traverse the directory file
    
    
    size_t f_size = 0;
    size_t len = 0;
    size_t sum_len = 0;
    
    fseek(dp,0,SEEK_END);
    size_t d_size = ftell(dp); // get the size of directory file
    rewind(dp);
    int count = d_size/72; // number of blocks
    
    
    fseek(fp, 0, SEEK_END);
    f_size = ftell(fp); // size of file data file
    rewind(fp);
    if(f_size < length)
    {
        pthread_mutex_unlock(&lock);
        return 2;
    }
    
    char filec[length];
    memset(filec,'\0',length); // create the space write in file data
    
    
    file_content content[count];
    for(int k = 0; k < count; k++)
    {
        content[k].file_length = 0;
        content[k].file_offset = 0;
    }
    int i = 0;
    int flag = 0;
    int time = 0;
    int postion = 0;
    
    
    while(fread(buf,1,72,dp)!=0) // traverse
    {
        
        
        if(flag == 0 && buf[0] == '\0') // find the first empty slot
        {
            flag = 1;
            postion = time;
            
        }
        
        
        if(buf[0] != '\0') //record the file information
        {
            
            memcpy(content[i].filename,buf,64);
            if(strcmp(content[i].filename,filename) == 0) // file exists
            {
                pthread_mutex_unlock(&lock);
                return 1;
            }
            
            memcpy(&content[i].file_offset,buf+64,4);
            
            memcpy(&content[i].file_length,buf+68,4);
            
            fseek(fp,content[i].file_offset,SEEK_SET);
            fread(content[i].file_data,1,content[i].file_length,fp);
            
            content[i].position = time;
            len = len + content[i].file_length;
            
            ++i;
        }
        time++;
    }
    
    sum_len = f_size - len; // find rest space
    if(sum_len<length)
    {
        pthread_mutex_unlock(&lock);
        return 2; // overall insufficient space
        
    }
    size_t a = 0;
    if(i<2) // only 1 file or 0 file in directory file
    {
        
        fseek(dp,postion*72,SEEK_SET);
        char name_f[64];
        strcpy(name_f,filename);
        fwrite(name_f, sizeof(char), 64,dp);
        
        if(i==1)
        {
            if(content[0].file_offset < length)
                a=content[0].file_offset+content[0].file_length;
        }
        
        fwrite(&a, sizeof(int),1,dp);
        fwrite(&length, sizeof(int),1,dp);    //write in directory
        fclose(dp);
        
        fseek(fp,a,SEEK_SET);    //write in data
        fwrite(filec, sizeof(char), length,fp);
        
        fclose(fp);
        
        compute_hash_tree(helper); // update hash tree
        pthread_mutex_unlock(&lock);
        return 0; // created
    }
    
    
    else // not only 1 file in directory file
    {
        for(int j = 0; j< i-1; j++) //Bubble Sort through file offeset
        {
            
            for(int t = j+1; t < i; t++)
            {
                if(content[j].file_offset > content[t].file_offset)
                {
                    size_t tmp_offset = content[j].file_offset;
                    size_t tmp_len = content[j].file_length;
                    char tmp_name[64];
                    memcpy(tmp_name,content[j].filename,64);
                    char tmp_c[BUFSIZ];
                    memcpy(tmp_c,content[j].file_data,strlen(content[j].file_data));
                    
                    content[j].file_offset = content[t].file_offset;
                    content[j].file_length = content[t].file_length;
                    memcpy(content[j].filename,content[t].filename,64);
                    memcpy(content[j].file_data,content[t].file_data,strlen(content[t].file_data));
                    
                    content[t].file_offset = tmp_offset;
                    content[t].file_length = tmp_len;
                    memcpy(content[t].filename,tmp_name,64);
                    memcpy(content[t].file_data,tmp_c,strlen(tmp_c));
                }
            }
            
        }
        if(content[0].file_offset >= length) // insert before the first file
        {
            fseek(dp,postion*72,SEEK_SET);
            char name_f[64];
            strcpy(name_f,filename);
            fwrite(name_f, sizeof(char), 64,dp);
            
            fwrite(&a, sizeof(int),1,dp);
            fwrite(&length, sizeof(int),1,dp);    //write in directory
            fclose(dp);
            
            fseek(fp,a,SEEK_SET);    //write in data
            fwrite(filec, sizeof(char), length,fp);
            fclose(fp);
            
            compute_hash_tree(helper); // update hash tree
            pthread_mutex_unlock(&lock);
            return 0; //created
        }
        
        int result = 0;
        size_t space = 0;
        
        for(int j=0; j<i-1; j++) //find the space to insert the file
        {
            space =content[j+1].file_offset - content[j].file_length - content[j].file_offset;
            if(space >= length)
            {
                result = j;
                fseek(dp,postion*72,SEEK_SET);
                char name_f[64];
                strcpy(name_f,filename);
                fwrite(name_f, sizeof(char), 64,dp);
                a = content[result].file_length + content[result].file_offset;
                fwrite(&a, sizeof(int),1,dp);
                fwrite(&length, sizeof(int),1,dp);    //write in directory
                fclose(dp);
                
                fseek(fp,a,SEEK_SET);    //write in data
                fwrite(filec, sizeof(char), length,fp);
                fclose(fp);
                
                compute_hash_tree(helper); // update hash tree
                pthread_mutex_unlock(&lock);
                return 0;
                
            }
        }
        
        
        repack(helper); // no space
        
        fseek(dp,postion*72,SEEK_SET);
        char name_f[64];
        strcpy(name_f,filename);
        fwrite(name_f, sizeof(char), 64,dp);
        a = len;
        fwrite(&a, sizeof(int),1,dp);
        fwrite(&length, sizeof(int),1,dp);    //write in directory
        fclose(dp);
        
        fseek(fp,a,SEEK_SET);    //write in data
        fwrite(filec, sizeof(char), length,fp);
        fclose(fp);
        
        compute_hash_tree(helper); // update hash tree
        pthread_mutex_unlock(&lock);
        return 0; //create
        
    }
}





int resize_file(char * filename, size_t length, void * helper) {
    
    
    
    
    FILE* dp;
    char buf[72];
    file* help = (file*)(helper);
    char* directory = help->directory;
    dp = fopen(directory,"rb+");
    
    FILE* fp;
    
    char* data = help->filename;
    fp = fopen(data,"rb+");
    
    if(!dp||!fp)
    {
        fclose(dp);
        fclose(fp);
        
        return 1; // if files not exists
    }
    
    
    size_t sum_len = 0;
    size_t f_size = 0;
    size_t len = 0;
    size_t file_len = 0;
    int flag = -1;
    
    
    fseek(dp,0,SEEK_END);
    size_t d_size = ftell(dp); // the size of directory file
    int count = d_size/72;
    file_content content[count];
    for(int k = 0; k < count; k++)
    {
        content[k].file_length = 0;
        content[k].file_offset = 0;
    }
    int i = 0;
    int tp = 0;
    rewind(dp);
    
    fseek(fp, 0, SEEK_END);
    
    f_size = ftell(fp); // the size of file data file
    rewind(fp);
    
    pthread_mutex_lock(&lock_resize);
    
    while (fread(buf,1,72,dp)!=0) // store the information of file in struct file_content
    {
        char name[64];
        memcpy(name,buf,64);
        
        if(buf[0]!='\0')
        {
            memcpy(content[i].filename,buf,64);
            memcpy(&content[i].file_offset,buf+64,4);
            memcpy(&content[i].file_length,buf+68,4);
            fseek(fp,content[i].file_offset,SEEK_SET);
            fread(content[i].file_data,1,content[i].file_length,fp);
            content[i].position = tp;
            len = len + content[i].file_length;
            if(strcmp(content[i].filename, filename)==0) // check if the file exists
            {
                memcpy(&file_len,buf+68,4);
                
                flag = i;
            }
            i++;
        }
        
        tp++;
    }
    
    rewind(dp);
    
    sum_len = f_size - len + file_len;
    
    
    if( sum_len < length) //insufficient space overall
        
    {
        fclose(dp);
        fclose(fp);
        pthread_mutex_unlock(&lock_resize);
        return 2;
    }
    
    
    
    if(flag == -1) // file not exists
    {
        fclose(dp);
        fclose(fp);
        pthread_mutex_unlock(&lock_resize);
        return 1;
    }
    
    
    
    
    if(i==1) // only one file in the directory file
    {
        
        if( length <= content[0].file_length ) // new size less that old size
        {
            
            fseek(dp,(int)(content[0].position * 72 + 68),SEEK_SET);
            
            //fwrite(&content[0].file_offset,4,1,dp);
            fwrite(&length,4,1,dp);
            
            fseek(fp,(content[0].file_offset+length),SEEK_SET); //find the end of the new fi
            
            char end = '\0';
            fwrite(&end,1,1,fp);
            
            
            fclose(dp);
            fclose(fp);
            
            
            
            
            
            compute_hash_tree(helper);
            
            
            
            pthread_mutex_unlock(&lock_resize);
            
            
            return 0;
            
            
        }
        
        else if( length > content[0].file_length && content[0].file_offset + length <= sum_len ) //new length bigger than old one and no need to repack
        {
            
            fseek(dp,content[0].position * 72 + 68,SEEK_SET);
            fwrite(&length,4,1,dp);
            fseek(fp,content[0].file_length + content[0].file_offset,SEEK_SET);
            char input[length - content[0].file_length];
            memset(input,'\0',length - content[0].file_length);
            fwrite(input,1,length - content[0].file_length,fp);
            
            fclose(dp);
            fclose(fp);
            
            compute_hash_tree(helper);
            pthread_mutex_unlock(&lock_resize);
            return 0;
            
            
        }
        
        else //need to repack, but it is simple when only one file there
        {
            
            int s = 0;
            fseek(dp,content[0].position * 72 + 64,SEEK_SET);
            fwrite(&s,4,1,dp);
            fwrite(&length,4,1,dp);
            fwrite(content[0].file_data,1,content[0].file_length,fp);
            char input[length - content[0].file_length];
            memset(input,'\0',length - content[0].file_length);
            fwrite(input,1,length - content[0].file_length,fp);
            fclose(dp);
            fclose(fp);
            
            compute_hash_tree(helper);
            pthread_mutex_unlock(&lock_resize);
            return 0;
            
        }
        
        
    }
    
    else // more than 1 file
    {
        
        for(int j = 0; j< i-1; j++) //Bubble Sort through file offeset
        {
            
            for(int t = j+1; t < i; t++)
            {
                if(content[j].file_offset > content[t].file_offset)
                {
                    size_t tmp_offset = content[j].file_offset;
                    size_t tmp_len = content[j].file_length;
                    char tmp_name[64];
                    memcpy(tmp_name,content[j].filename,64);
                    char tmp_c[BUFSIZ];
                    memcpy(tmp_c,content[j].file_data,strlen(content[j].file_data));
                    
                    int tmp = content[j].position;
                    
                    content[j].file_offset = content[t].file_offset;
                    content[j].file_length = content[t].file_length;
                    memcpy(content[j].filename,content[t].filename,64);
                    memcpy(content[j].file_data,content[t].file_data,strlen(content[t].file_data));
                    content[j].position = content[t].position;
                    
                    content[t].file_offset = tmp_offset;
                    content[t].file_length = tmp_len;
                    memcpy(content[t].filename,tmp_name,64);
                    memcpy(content[t].file_data,tmp_c,strlen(tmp_c));
                    content[t].position = tmp;
                }
            }
            
        }
        
        int result = 0;
        for(int j = 0; j < i; j++) // find the file need to update in the content array
        {
            if(strcmp(content[j].filename,filename)==0)
            {
                result = j;
                break;
            }
        }
        
        if(length <= content[result].file_length) // new length less than the old one
        {
            
            
            
            fseek(dp,content[result].position * 72 + 64,SEEK_SET);
            fwrite(&content[result].file_offset,1,4,dp);
            fwrite(&length,1,4,dp);
            fseek(fp,content[result].file_offset+length,SEEK_SET);
            char end = '\0';
            fwrite(&end,1,1,fp);
            
            fclose(dp);
            fclose(fp);
            compute_hash_tree(helper);
         
            pthread_mutex_unlock(&lock_resize);
            return 0;
            
            
        }
        
        
        
        
        else // new length bigger than the old one
            
        {
            
            if( result!= i-1 && (length + content[result].file_offset) <= content[result+1].file_offset ) // no need to repack
            {
                
                fseek(dp,content[result].position * 72 + 68,SEEK_SET);
                fwrite(&length,4,1,dp);
                
                fseek(fp,content[result].file_length + content[result].file_offset,SEEK_SET);
                char input[length - content[result].file_length];
                memset(input,'\0',length - content[result].file_length);
                fwrite(input,1,length - content[result].file_length,fp);
                
                fclose(dp);
                fclose(fp);
                compute_hash_tree(helper);
                //update_hash_file(content[result].file_offset, length + content[result].file_offset, helper);
                pthread_mutex_unlock(&lock_resize);
                return 0;
                
                
            }
            
            
            else // need to repack
            {
                
                fclose(dp);
                fclose(fp);
                delete_file(content[result].filename,helper); // delete the file first to repack
                
                
                
                repack(helper);
                
                FILE* dpf = fopen(directory,"rb+");
                FILE* fpf = fopen(data,"rb+");
                
                
                fseek(fpf,len-file_len,SEEK_SET);
                fwrite(content[result].file_data,1,content[result].file_length,fpf);
                char input[length - content[result].file_length];
                memset(input,'\0',length - content[result].file_length);
                fwrite(input,1,length - content[result].file_length,fpf);
                
                
                fseek(dpf,content[result].position*72,SEEK_SET);
                fwrite(content[result].filename,1,64,dpf);
                int r_off = len - file_len;
                fwrite(&r_off,4,1,dpf);
                fwrite(&length,4,1,dpf);
                
                fclose(dpf);
                fclose(fpf);
                
                compute_hash_tree(helper);
                pthread_mutex_unlock(&lock_resize);
                return 0;
            }
            
        }
        
        
    }
    
    pthread_mutex_unlock(&lock_resize);
    return 1;
}








void repack(void * helper) {
    
    pthread_mutex_lock(&lock_repack);
    FILE* dp;
    file* help = (file*)(helper);
    char* directory = help->directory;
    dp = fopen(directory,"rb+");
    
    
    FILE* fp;
    char* data = help->filename;
    fp = fopen(data,"rb+");
    
    fseek(dp,0,SEEK_END);
    size_t d_size = ftell(dp);
    rewind(dp);
    
    int count = d_size/72;
    file_content content[count];
    for(int k = 0; k < count; k++)
    {
        content[k].file_length = 0;
        content[k].file_offset = 0;
    }
    int i = 0;
    char buf[72];
    int tp = 0;
    
    
    while(fread(buf,1,72,dp)!=0) //record data in file_content
    {
        if(buf[0] != '\0')
        {
            memcpy(content[i].filename,buf,64);
            memcpy(&content[i].file_offset,buf+64,4);
            memcpy(&content[i].file_length,buf+68,4);
            fseek(fp,content[i].file_offset,SEEK_SET);
            fread(content[i].file_data,1,content[i].file_length,fp);
            content[i].position = tp;
            i++;
        }
        tp++;
    }
    
    
    
    
    if(i == 1) // only 1 file in the directory file
    {    rewind(dp);
        
        while(fread(buf,1,72,dp)!=0)
        {
            char name[64];
            memcpy(name,buf,64);
            int a = 0;
            if(strcmp(name,content[0].filename)==0)
            {   fseek(dp,-8l,SEEK_CUR);
                fwrite(&a,4,1,dp);
                
                fseek(fp,0,SEEK_SET);
                fwrite(content[0].file_data,1,content[0].file_length,fp);
                
                fclose(dp);
                fclose(fp);
                compute_hash_tree(helper);
                
                pthread_mutex_unlock(&lock_repack);
                return;
            }
        }
    }
    
    
    else // more than one file
        
    {
        fclose(fp);
        for(int j = 0; j< i-1; j++) //Bubble Sort through file offeset
        {
            
            for(int t = j+1; t < i; t++)
            {
                if(content[j].file_offset > content[t].file_offset)
                {
                    size_t tmp_offset = content[j].file_offset;
                    size_t tmp_len = content[j].file_length;
                    char tmp_name[64];
                    memcpy(tmp_name,content[j].filename,64);
                    char tmp_c[BUFSIZ];
                    memcpy(tmp_c,content[j].file_data,strlen(content[j].file_data));
                    int tmp = content[j].position;
                    
                    
                    content[j].file_offset = content[t].file_offset;
                    content[j].file_length = content[t].file_length;
                    memcpy(content[j].filename,content[t].filename,64);
                    memcpy(content[j].file_data,content[t].file_data,strlen(content[t].file_data));
                    content[j].position = content[t].position;
                    
                    
                    content[t].file_offset = tmp_offset;
                    content[t].file_length = tmp_len;
                    memcpy(content[t].filename,tmp_name,64);
                    memcpy(content[t].file_data,tmp_c,strlen(tmp_c));
                    content[t].position = tmp;
                }
            }
            
        }
        size_t offset_f = 0;
        
        
        FILE* fpn;
        fpn = fopen(data,"rb+");
        rewind(dp);
        
        for(int j = 0; j< i; j++) // write new content in file data file
        {
            char tmp[content[j].file_length];
            memcpy(tmp,content[j].file_data,content[j].file_length);
            fwrite(tmp,1,content[j].file_length,fpn);
            content[j].file_offset = offset_f;
            
            offset_f = offset_f + content[j].file_length;
        }
        
        while(fread(buf,1,72,dp)!=0) // change the offset in directory file
        {
            char name[64];
            memcpy(name,buf,64);
            for(int j = 0; j < i; j++)
            {
                if(strcmp(name,content[j].filename)==0)
                {   fseek(dp,-8l,SEEK_CUR);
                    fwrite(&content[j].file_offset,4,1,dp);
                    fseek(dp,4,SEEK_CUR);
                }
            }
        }
        fclose(dp);
        fclose(fpn);
        compute_hash_tree(helper);
        pthread_mutex_unlock(&lock_repack);
        return;
    }
    
}








int delete_file(char * filename, void * helper) {
    
    FILE* fp;
    char buf[72];
    file* help = (file*)(helper);
    char* directory = help->directory;
    fp = fopen(directory,"rb+");
    size_t offset = 0;
    size_t len = 0;
    if(!fp)
    {
        fclose(fp);
        pthread_mutex_unlock(&lock_delete);
        return -1;
    }
    
    
    ssize_t file_length = file_size(filename,helper);
    if(file_length == -1)
    {
        fclose(fp);
        
        return 1;
    }
    
    while (fread(buf,1,72,fp)!=0)// find the file
    {
        
        char name[64];
        memcpy(name,buf,64);
        
        
        if(strcmp(name, filename)==0)
            
        {
            memcpy(&offset,buf+64,4);
            memcpy(&len,buf+68,4);
            fseek(fp,-72l,SEEK_CUR);
            char k = '\0';
            fwrite(&k,1,1,fp); //delete
            fclose(fp);
            
            return 0;
        }
        
    }
    
    
    fclose(fp);
    
    
    return 1;
}


int rename_file(char * oldname, char * newname, void * helper) {
    
    pthread_mutex_lock(&lock);
    FILE* fp;
    char buf[72];
    memset(buf,'\0',72);
    file* help = (file*)(helper);
    char* directory = help->directory;
    fp = fopen(directory,"rb+");
    
    if(!fp)
    {
        fclose(fp);
        pthread_mutex_unlock(&lock);
        return 1;
    }
    while (fread(buf,1,72,fp)!=0) // check if newname in the directory file
    {
        
        char name[64];
        memcpy(name,buf,64);
        
        
        if(strcmp(name, newname)==0)
            
        {
            fclose(fp);
            pthread_mutex_unlock(&lock);
            return 1;
        }
    }
    
    rewind(fp);
    
    while (fread(buf,1,72,fp)!=0) // check if oldname in the directory file
    {
        
        char name[64];
        memcpy(name,buf,64);
        
        
        if(strcmp(name, oldname)==0)
            
        {
            fseek(fp,-72,SEEK_CUR);
            memcpy(name,newname,strlen(newname)); //change name
            fwrite(name,1,64,fp);
            fclose(fp);
            pthread_mutex_unlock(&lock);
            return 0;
        }
        
    }
    pthread_mutex_unlock(&lock);
    return 1;
    
    
}


int read_file(char * filename, size_t offset, size_t count, void * buf, void * helper) {
    
    ssize_t file_length = file_size(filename,helper);
    if(file_length == -1) // check if the file name exists
        return 1;
    
    if(count+offset>file_length||count<0||offset<0) // cannot read
        return 2;
    
    pthread_mutex_lock(&lock_read);
    FILE* fp;
    file* help = (file*)(helper);
    char* file_data = help->filename;
    fp=fopen(file_data,"rb+");
    
    FILE* p;
    char b[72];
    memset(b,'\0',72);
    char* directory = help->directory;
    p=fopen(directory,"rb+");
    ssize_t off = 0;
    
    FILE* fh;
    char* hash = help->hash;
    fh = fopen(hash,"rb+");
    
    
    
    
    if(!fp||!p||!fh)
    {
        fclose(fp);
        fclose(p);
        fclose(fh);
        
        pthread_mutex_unlock(&lock_read);
        
        return 1;
    }
    
    
    fseek(fh,0,SEEK_END);
    size_t size = ftell(fh);
    rewind(fh);
    uint8_t hash_file[size];
    fread(hash_file,1,size,fh);
    fclose(fh);
    
    
    
    
    while (fread(b,1,72,p)!=0) // find the filename
    {
        char name[64];
        memcpy(name,b,64);
        
        if(strcmp(name, filename)==0)
        {
            memcpy(&off,b+64,4);
            
            break;
        }
    }
    fclose(p);
    
    
    update_hash_file(off, off+ offset + count, helper); // get the hash data
    fh = fopen(hash,"rb+");
    uint8_t hash_file_update[size];
    fread(hash_file_update,1,size,fh);
    fclose(fh);
    
    if(memcmp(hash_file,hash_file_update,size) != 0) // check if the hash data correct
    {
        fclose(fp);
        pthread_mutex_unlock(&lock_read);
        return 3;
        
    }
    
    
    fseek(fp,offset+off,SEEK_SET);
    char bu[count];
    
    fread(bu,1,count,fp);
    memcpy(buf,bu,count); //copy to buf
    
    fclose(fp);
    pthread_mutex_unlock(&lock_read);
    return 0;
    
    
    
    
}


int write_file(char * filename, size_t offset, size_t count, void * buf, void * helper) {
    
    pthread_mutex_lock(&write_lock);
    
    ssize_t file_length = file_size(filename,helper);
    if(file_length == -1) // check if the file name exists
    {
        pthread_mutex_unlock(&write_lock);
        return 1;
    }
    
    if(offset>file_length||count<=0||offset<0) //cannot write
    {
        pthread_mutex_unlock(&write_lock);
        return 2;
    }
    
    
    FILE* fp;
    file* help = (file*)(helper);
    char* file_data = help->filename;
    fp=fopen(file_data,"rb+");
    
    FILE* dp;
    char b[72];
    memset(b,'\0',72);
    char* directory = help->directory;
    
    dp=fopen(directory,"rb+");
    
    size_t file_offset = 0;
    
    
    if(!fp||!dp)
    {
        fclose(fp);
        fclose(dp);
        pthread_mutex_unlock(&write_lock);
        return 1;
    }
    
    
    
    
    
    
    while (fread(b,1,72,dp)!=0) // find the file name
    {
        
        char name[64];
        memcpy(name,b,64);
        
        if(strcmp(name, filename)==0)
        {
            memcpy(&file_offset,b+64,4);
        }
        
    }
    
    if(offset > file_length) // cannot write
    {
        pthread_mutex_unlock(&write_lock);
        return 2;
        
    }
    
    if((offset + count) <= file_length) // the changed content inside the file
    {
        
        fseek(fp,(offset + file_offset),SEEK_SET);
        
        char input[BUFSIZ];
        
        memcpy(input,buf,count);
        fwrite(input,1,count,fp);
        
        
        fclose(fp);
        fclose(dp);
        
        compute_hash_tree( helper);
        pthread_mutex_unlock(&write_lock);
        
        return 0;
    }
    else // need to write outside the content
    {
        
        fclose(dp);
        fclose(fp);
        
        int result = resize_file(filename, (offset + count), helper); // resize the file first
        
        if(result == 1)
        {
            pthread_mutex_unlock(&write_lock);
            return 1;
        }
        
        if(result == 2)
        {
            pthread_mutex_unlock(&write_lock);
            return 3;
        }
        
        else
        {    dp = fopen(directory,"rb+");
            fp = fopen(file_data,"rb+");
            int tp = 0;
            while (fread(b,1,72,dp)!=0) // find the file
            {
                
                char name[64];
                memcpy(name,b,64);
                
                if(strcmp(name, filename)==0)
                {
                    memcpy(&file_offset,b+64,4);
                    break;
                }
                tp++;
                
            }
            
            fseek(fp,offset + file_offset,SEEK_SET);
            
            

            fwrite(buf,1,count,fp); // write to the file
            
            
            fclose(fp);
            fclose(dp);
            
            
            compute_hash_tree( helper);
            pthread_mutex_unlock(&write_lock);
            
            return 0;
        }
    }
    
    
}





ssize_t file_size(char * filename, void * helper) {
    
    FILE* fp;
    char buf[72];
    memset(buf,'\0',72);
    file* help = (file*)(helper);
    char* directory = help->directory;
    fp=fopen(directory,"rb+");
    if(!fp)
    {
        fclose(fp);
        return -1;
    }
    while (fread(buf,1,72,fp)!=0) // find the file and its length
    {
        char name[64];
        memcpy(name,buf,64);
        ssize_t file_length = 0;
        if(buf[0] != '\0')
        {if(strcmp(name, filename)==0)
        {
            memcpy(&file_length,buf+68,4);
            fclose(fp);
            return file_length;
        }}
        
    }
    
    fclose(fp);
    return -1;
}

void fletcher(uint8_t * buf, size_t length, uint8_t * output) {
    
    uint32_t* new_buf;
    size_t tmp = length;
    
    if(length % 4 !=0) //check if the length can mode by 4
        length = length + (4 - length % 4); // if it cannot, chage the length.
    
    new_buf = (uint32_t*)calloc(length/4,sizeof(uint32_t));// create a new buff to store the content of buf, add zeros at the end of new buff if length cannot mode by 4
    
    memcpy(new_buf,buf,tmp);
    
    
    uint64_t a = 0;
    uint64_t b = 0;
    uint64_t c = 0;
    uint64_t d = 0;
    
    for(size_t i = 0; i< length/4 ; i++)
    {
        
        
        a = (a + new_buf[i]) % (uint64_t)(pow(2,32)-1);
        b = (b + a) % (uint64_t)(pow(2,32)-1);
        c = (c + b) % (uint64_t)(pow(2,32)-1);
        d = (d + c) % (uint64_t)(pow(2,32)-1);
        
        
    }
    
    free(new_buf);
    uint32_t re[4]={a,b,c,d}; //create an array to cast a,b,c,d
    
    memcpy(output,re,16);//copy to output
    
    return;
}


void compute_hash_tree(void * helper) {
    
    pthread_mutex_lock(&compute_lock);
    FILE* fp;
    file* help = (file*)(helper);
    char* data = help->filename;
    fp = fopen(data,"rb");
    
    FILE* fh;
    char* hash = help->hash;
    fh = fopen(hash,"rb+");
    
    fseek(fp,0,SEEK_END);
    size_t flen = ftell(fp); // find the size of file data
    rewind(fp);
    size_t n = flen/256; // find the number of blocks
    double dep = log(n)/log(2); // compute the depth of the tree
    uint8_t buf[256];
    
    
    
    if(n==1) // only one block in file data file
    {
        fread(buf,1,256,fp);
        uint8_t* result;
        result = (uint8_t*)malloc(16);
        fletcher(buf,256,result); //hash the content
        fwrite(result,16,1,fh); // write to the hash data file
        
        fclose(fp);
        fclose(fh);
        free(result);
        pthread_mutex_unlock(&compute_lock);
        return;
    }
    
    
    
    uint8_t f_hash[(int)(pow(2,dep+1)-1)*16];
    
    
    int start = (int)(n-1);
    int offset = 0;
    while(fread(buf,1,256,fp)) // hash all blocks in file data file
    {
        uint8_t* result;
        result = (uint8_t*)malloc(16);
        fletcher(buf,256,result);
        memcpy(f_hash+(start+offset)*16,result,16);
        offset++;
        free(result);
    }
    
    
    size_t tmp = n/2;
    
    for(int i = 0; i <(int) dep; i++)
    {
        uint8_t* result;
        result = (uint8_t*)malloc(16);
        
        uint8_t* input;
        input = (uint8_t*)malloc(32);
        
        
        for(int j = 0; j < tmp; j++ ) // hash the node and store in parent node
        {
            
            
            int index_low = (int)(start+pow(2,i+1)*j+1)/pow(2,i)-1;
            int index = (int)((index_low+1)/2-1);
            memcpy(input,f_hash+index_low*16,16);
            memcpy(input+16,f_hash+(index_low+1)*16,16);
            
            fletcher(input,32,result);
            
            memcpy(f_hash+index*16,result,16);
            
        }
        
        tmp = tmp/2;
        free(result);
        free(input);
    }
    
    fwrite(f_hash,(int)(pow(2,dep+1)-1)*16,1,fh); //write to hash data file
    
    fclose(fp);
    fclose(fh);
    pthread_mutex_unlock(&compute_lock);
    return;
}


void compute_hash_block(size_t block_offset, void * helper) {
    
    
    
    pthread_mutex_lock(&hash_lock);
    FILE* fp;
    file* help = (file*)(helper);
    char* data = help->filename;
    fp = fopen(data,"rb+");
    
    FILE* fh;
    char* hash = help->hash;
    fh = fopen(hash,"rb+");
    
    fseek(fp,0,SEEK_END);
    size_t flen = ftell(fp); // find the size of file data file
    rewind(fp);
    size_t n = flen/256;  // find the number of blocks
    
    
    
    double dep = log(n)/log(2); // compute the depth of the tree
    uint8_t buf[256];
    
    uint8_t* result;
    
    
    
    result = (uint8_t*)malloc(16);
    fseek(fp,block_offset*256,SEEK_SET); // find the block need to re-hash
    fread(buf,1,256,fp);
    fletcher(buf,256,result); //re-hash
    
    int index = (int)((pow(2,dep+1)-1) - n + block_offset);// the index of the node in the hash tree
    fclose(fp);
    
    
    fseek(fh,index*16,SEEK_SET);
    fwrite(result,1,16,fh);
    free(result);
    
    fflush(fh); // save to hash data file
    
    
    
    
    for(int i = 0; i <(int) dep; i++)
    {
        
        
        
        
        
        uint8_t input[32];
        uint8_t re[16];
        
        
        if(index % 2 == 0) //get the node need to compute the parent hash
        {
            fseek(fh,(index-1)*16,SEEK_SET);
            fread(input,1,32,fh);
            index = index / 2 - 1; // find the index of parent node
            
            
        }
        else //get the node need to compute the parent hash
        {
            
            fseek(fh,index*16,SEEK_SET);
            fread(input,1,32,fh);
            
            index = (index + 1) / 2 - 1; // find the index of parent node
            
            
            
        }
        
        fletcher(input,32,re); // re-hash the parent
        
        
        fseek(fh,index*16,SEEK_SET);
        
        
        fwrite(re,1,16,fh); // write to hash data file
        fflush(fh);
        
        
    }
    
    
    
    fclose(fh);
    
    
    pthread_mutex_unlock(&hash_lock);
    return;
    
    //only leaft node can be changed
    
}

