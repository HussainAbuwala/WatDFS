//
// Starter code for CS 454/654
// You SHOULD change this file
//

#include "rpc.h"
#include "debug.h"
INIT_LOG

#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <cstring>
#include <cstdlib>
#include <iostream>
#include <fcntl.h>
#include <fuse.h>
#include <map>
#include <vector>


using namespace std;
#include "rw_lock.h"
#include "rw_lock.cpp"

rw_lock_t file_lock;


struct File_metadata
{
    int is_writer_open;
    int writer_fh;
};

vector <pair<std::string,int>> vec;
map<const char*,struct File_metadata> file_meta;


// Global state server_persist_dir.
char *server_persist_dir = nullptr;

// Global state server_metadata
//struct Server_metadata sm;

// Important: the server needs to handle multiple concurrent client requests.
// You have to be carefuly in handling global variables, esp. for updating them.
// Hint: use locks before you update any global variable.

// We need to operate on the path relative to the the server_persist_dir.
// This function returns a path that appends the given short path to the
// server_persist_dir. The character array is allocated on the heap, therefore
// it should be freed after use.
// Tip: update this function to return a unique_ptr for automatic memory management.
char *get_full_path(char *short_path) {
    int short_path_len = strlen(short_path);
    int dir_len = strlen(server_persist_dir);
    int full_len = dir_len + short_path_len + 1;

    char *full_path = (char *)malloc(full_len);

    // First fill in the directory.
    strcpy(full_path, server_persist_dir);
    // Then append the path.
    strcat(full_path, short_path);
    DLOG("Full path: %s\n", full_path);

    return full_path;
}

// The server implementation of getattr.
int watdfs_getattr(int *argTypes, void **args) {
    // Get the arguments.
    // The first argument is the path relative to the mountpoint.
    char *short_path = (char *)args[0];
    // The second argument is the stat structure, which should be filled in
    // by this function.
    struct stat *statbuf = (struct stat *)args[1];
    // The third argument is the return code, which should be set be 0 or -errno.
    int *ret = (int *)args[2];

    // Get the local file name, so we call our helper function which appends
    // the server_persist_dir to the given path.
    char *full_path = get_full_path(short_path);

    // Initially we set set the return code to be 0.
    *ret = 0;
    
    //(void)statbuf;  ORIGINAL CODE LINE

    // Let sys_ret be the return code from the stat system call.

    int sys_ret = 0;

    // TODO: Make the stat system call, which is the corresponding system call needed
    // to support getattr. You should use the statbuf as an argument to the stat system call.
    
    //------------------------------------------------------------------
    //------------------------------------------------------------------
    
    sys_ret = stat(full_path,statbuf);


    //------------------------------------------------------------------
    //------------------------------------------------------------------

    if (sys_ret < 0) {
        // If there is an error on the system call, then the return code should
        // be -errno.
        *ret = -errno;
    }

    // Clean up the full path, it was allocated on the heap.
    free(full_path);

    //DLOG("Returning code: %d", *ret);
    // The RPC call succeeded, so return 0.
    return 0;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
//------------------------------------------------------------------
//------------------------------------------------------------------
//------------------------------------------------------------------
    

int watdfs_mknod(int *argTypes, void **args){


    // Get the arguments.
    // The first argument is the path relative to the mountpoint.
    char *short_path = (char *)args[0];

    // The second argument is the mode datatype, 
    mode_t mode = *(mode_t *)args[1];

    // The third argument is the dev datatype.
    dev_t dev = *(dev_t *)args[2];

    // The fourth argument is the returncode.
    int *ret = (int *)args[3];


    // Get the local file name, so we call our helper function which appends
    // the server_persist_dir to the given path.
    char *full_path = get_full_path(short_path);

    // Initially we set set the return code to be 0.
    *ret = 0;

    // Let sys_ret be the return code from the mknod system call.

    int sys_ret = 0;

    sys_ret = mknod(full_path,mode,dev);

    if (sys_ret < 0) {
        // If there is an error on the system call, then the return code should
        // be -errno.
        *ret = -errno;
    }

    // Clean up the full path, it was allocated on the heap.
    free(full_path);

    //DLOG("Returning code: %d", *ret);
    // The RPC call succeeded, so return 0.
   
    return 0;
}


//------------------------------------------------------------------
//------------------------------------------------------------------
//------------------------------------------------------------------
//------------------------------------------------------------------
//------------------------------------------------------------------

int check_file_exist(char *short_path){
    for(int i = 0; i < vec.size(); i++){ 
        if(vec[i].first == short_path){
            return 0;
        }
    } 
    return -1;
}

int add_value(char *short_path, int fh){
    int r = check_file_exist(short_path);
    if(r == 0){
        for(int i = 0; i < vec.size(); i++){ 
            if(vec[i].first == short_path){
                vec[i].second = fh;
            }
        } 
    }
    else{
        vec.push_back({short_path,fh});
    }

}

int get_value(char *short_path){
    for(int i = 0; i < vec.size(); i++){ 
        if(vec[i].first == short_path){
            return vec[i].second;
        }
    }
    return -1; 
}


int check_if_writer_open(char *short_path){
    for(int i = 0; i < vec.size(); i++){ 
        if(vec[i].first == short_path){
            if(vec[i].second < 0){
                return -1;
            }
            return 0;
        }
    } 
    return -1;
}

int watdfs_open(int *argTypes, void **args){

    DLOG("IN watdfs_open [SERVER SIDE]: STARTING ---------------------------------------------------------");

    struct File_metadata f_m;

    // The first argument is the path relative to the mountpoint.
    char *short_path = (char *)args[0];

    // The second argument is the fuse_file_info structure, fh should be filled in
    // by this function.
    struct fuse_file_info *fi = (struct fuse_file_info *)args[1];

    int open_mode = (fi->flags) & (O_ACCMODE);
    DLOG("IN watdfs_open [SERVER SIDE]: open_mode, '%d'", open_mode);


    // The third argument is the return code, which should be set be 0 or -errno.
    int *ret = (int *)args[2];

    // Get the local file name, so we call our helper function which appends
    // the server_persist_dir to the given path.
    char *full_path = get_full_path(short_path);

    // Initially we set set the return code to be 0.
    *ret = 0;

    int sys_ret = 0;

    if(open_mode == O_RDWR || open_mode == O_WRONLY){
        DLOG("IN watdfs_open [SERVER SIDE]: open_mode Is either O_RDWR or O_WRONLY");
        
        if((check_file_exist(short_path) == -1) || (check_file_exist(short_path) == 0 && check_if_writer_open(short_path) == -1)){
            DLOG("IN watdfs_open [SERVER SIDE]: NO EXISTING WRITER PRESENT");

            sys_ret = open(full_path, fi->flags);
            
            if(sys_ret < 0){
                *ret = -errno;
            }else{
                DLOG("IN watdfs_open [SERVER SIDE]: SAVING NEW WRITER INFO");
                fi->fh = sys_ret;
                add_value(short_path,sys_ret);
            }
        }

        else{ 
            DLOG("IN watdfs_open [SERVER SIDE]: EXISTING WRITER PRESENT, set ret=-EACCES");
            *ret = -EACCES;
        }
    }
    else{

        DLOG("IN watdfs_open [SERVER SIDE]:OPENING FILE IN READONLY");

        sys_ret = open(full_path, fi->flags);
        if(sys_ret < 0){
            *ret = -errno;
        }else{
            fi->fh = sys_ret;
        }

    }

    // Let sys_ret be the return code from the open system call.

    free(full_path);

    //DLOG("IN watdfs_open [SERVER SIDE]:: short_path '%s'", short_path);

    //DLOG("IN watdfs_open [SERVER SIDE]:: is_writer_open '%d'", file_meta[short_path].is_writer_open);
    DLOG("IN watdfs_open [SERVER SIDE]:: writer_fh '%d'", get_value(short_path));

   

    DLOG("watdfs_open [SERVER SIDE]: ENDING ---------------------------------------------------------");

    return 0;

}



int watdfs_release(int *argTypes, void **args){

    DLOG("IN watdfs_release [SERVER SIDE]: STARTING ---------------------------------------------------------");


    // The first argument is the path relative to the mountpoint.
    char *short_path = (char *)args[0];

    // The second argument is the fuse_file_info structure, fh should be filled in
    // by this function.
    struct fuse_file_info *fi = (struct fuse_file_info *)args[1];
    
    // The third argument is the return code, which should be set be 0 or -errno.
    int *ret = (int *)args[2];

    // Get the local file name, so we call our helper function which appends
    // the server_persist_dir to the given path.
    char *full_path = get_full_path(short_path);

    if(check_file_exist(short_path) == 0 && get_value(short_path) == int(fi->fh)){
        DLOG("IN watdfs_release [SERVER SIDE]: delete writer from file meta data");
        //file_meta[short_path].is_writer_open = 0;
        //file_meta[short_path].writer_fh = -1;
        add_value(short_path,-1);
        DLOG("IN watdfs_release [SERVER SIDE]: writer_fh after deleting '%d'",get_value(short_path));

    }


    // Initially we set set the return code to be 0.
    *ret = 0;

    // Let sys_ret be the return code from the open system call.

    int sys_ret = 0;

    //make open system call

    sys_ret = close(fi->fh);

    if(sys_ret < 0){
        *ret = -errno;
    }
    
    free(full_path);
    
    DLOG("IN watdfs_release [SERVER SIDE]: ENDING ---------------------------------------------------------");

    return 0;

}


int watdfs_utimens(int *argTypes, void **args){

    // The first argument is the path relative to the mountpoint.
    char *short_path = (char *)args[0];

    // The second argument is the array of struct timespec structure, which should be filled in
    // by this function.
    struct timespec *ts = (struct timespec *)(args[1]);

     // The third argument is the return code, which should be set be 0 or -errno.
    int *ret = (int *)args[2];

    // Get the local file name, so we call our helper function which appends
    // the server_persist_dir to the given path.
    char *full_path = get_full_path(short_path);

    // Initially we set set the return code to be 0.
    *ret = 0;
    
    //(void)statbuf;  ORIGINAL CODE LINE

    // Let sys_ret be the return code from the stat system call.

    int sys_ret = 0;

    sys_ret = utimensat(0,full_path,ts,0);


    //------------------------------------------------------------------
    //------------------------------------------------------------------

    if (sys_ret < 0) {
        // If there is an error on the system call, then the return code should
        // be -errno.
        *ret = -errno;
    }

    // Clean up the full path, it was allocated on the heap.
    free(full_path);

    //DLOG("Returning code: %d", *ret);
    // The RPC call succeeded, so return 0.
    return 0;

}

int watdfs_read(int *argTypes, void **args){

    // Get the arguments.
    // The first argument is the path relative to the mountpoint.
    char *short_path = (char *)args[0];

    //second argument is the buffer
    void *buf = args[1];

    // The third argument is the size_t datatype
    size_t size = *(size_t *)args[2];

    // The fourth argument is the off_t datatype.
    off_t offset = *(off_t *)args[3];

    // The fifth argument is the fuse_file_info structure.
    struct fuse_file_info *fi = (struct fuse_file_info *)args[4];

    // The 6th argument is the return code, which should be set be 0 or -errno.
    int *ret = (int *)args[5];

    char *full_path = get_full_path(short_path);

    // Initially we set set the return code to be 0.
    *ret = 0;

    // Let sys_ret be the return code from the pread system call.

    int sys_ret = 0;

    sys_ret = pread(fi->fh, buf, size, offset);

    if (sys_ret < 0) {
        // If there is an error on the system call, then the return code should
        // be -errno.
        *ret = -errno;
    }
    else{
        *ret = sys_ret;
    }

    // Clean up the full path, it was allocated on the heap.
    free(full_path);

    //DLOG("Returning code: %d", *ret);
    // The RPC call succeeded, so return 0.
    return 0;

}

int watdfs_write(int *argTypes, void **args){

    // Get the arguments.
    // The first argument is the path relative to the mountpoint.
    char *short_path = (char *)args[0];

    //second argument is the buffer
    void *buf = args[1];

    // The third argument is the size_t datatype
    size_t size = *(size_t *)args[2];

    // The fourth argument is the off_t datatype.
    off_t offset = *(off_t *)args[3];

    // The fifth argument is the fuse_file_info structure.
    struct fuse_file_info *fi = (struct fuse_file_info *)args[4];

    // The 6th argument is the return code, which should be set be 0 or -errno.
    int *ret = (int *)args[5];

    char *full_path = get_full_path(short_path);

    // Initially we set set the return code to be 0.
    *ret = 0;

    // Let sys_ret be the return code from the pread system call.

    int sys_ret = 0;

    sys_ret = pwrite(fi->fh, buf, size, offset);

    if (sys_ret < 0) {
        // If there is an error on the system call, then the return code should
        // be -errno.
        *ret = -errno;
    }
    else{
        *ret = sys_ret;
    }

    // Clean up the full path, it was allocated on the heap.
    free(full_path);

    //DLOG("Returning code: %d", *ret);
    // The RPC call succeeded, so return 0.
    return 0;

}


int watdfs_truncate(int *argTypes, void **args){

    // Get the arguments.
    // The first argument is the path relative to the mountpoint.
    char *short_path = (char *)args[0];

    // The second argument is the off_t datatype.
    off_t newsize = *(off_t *)args[1];

    // The 3rd argument is the return code, which should be set be 0 or -errno.
    int *ret = (int *)args[2];

    char *full_path = get_full_path(short_path);

    // Initially we set set the return code to be 0.
    *ret = 0;

    // Let sys_ret be the return code from the pread system call.

    int sys_ret = 0;

    sys_ret = truncate(full_path, newsize);

    if (sys_ret < 0) {
        // If there is an error on the system call, then the return code should
        // be -errno.
        *ret = -errno;
    }

    // Clean up the full path, it was allocated on the heap.
    free(full_path);

    //DLOG("Returning code: %d", *ret);
    // The RPC call succeeded, so return 0.
    return 0;

}





int watdfs_fsync(int *argTypes, void **args){
    // The first argument is the path relative to the mountpoint.
    char *short_path = (char *)args[0];

    // The second argument is the fuse_file_info structure, 
    struct fuse_file_info *fi = (struct fuse_file_info *)args[1];
    
    // The third argument is the return code, which should be set be 0 or -errno.
    int *ret = (int *)args[2];

    // Get the local file name, so we call our helper function which appends
    // the server_persist_dir to the given path.
    char *full_path = get_full_path(short_path);

    // Initially we set set the return code to be 0.
    *ret = 0;

    // Let sys_ret be the return code from the open system call.

    int sys_ret = 0;

    //make open system call

    sys_ret = fsync(fi->fh);

    if(sys_ret < 0){
        *ret = -errno;
    }

    free(full_path);

    return 0;
}

int watdfs_lock(int *argTypes, void **args){
    // Get the arguments.
    // The first argument is the path relative to the mountpoint.
    char *short_path = (char *)args[0];

    // The second argument is the mode datatype, 
    rw_lock_mode_t mode = *(rw_lock_mode_t *)args[1];

    // The fourth argument is the returncode.
    int *ret = (int *)args[2];


    // Get the local file name, so we call our helper function which appends
    // the server_persist_dir to the given path.
    char *full_path = get_full_path(short_path);

    // Initially we set set the return code to be 0.
    *ret = 0;

    // Let sys_ret be the return code from the mknod system call.

    int sys_ret = 0;

    sys_ret = rw_lock_lock(&file_lock, mode);

    if (sys_ret < 0) {
        // If there is an error on the system call, then the return code should
        // be -errno.
        *ret = -errno;
    }

    // Clean up the full path, it was allocated on the heap.
    free(full_path);

    //DLOG("Returning code: %d", *ret);
    // The RPC call succeeded, so return 0.
   
    return 0;
}

int watdfs_unlock(int *argTypes, void **args){
    // Get the arguments.
    // The first argument is the path relative to the mountpoint.
    char *short_path = (char *)args[0];

    // The second argument is the mode datatype, 
    rw_lock_mode_t mode = *(rw_lock_mode_t *)args[1];

    // The fourth argument is the returncode.
    int *ret = (int *)args[2];


    // Get the local file name, so we call our helper function which appends
    // the server_persist_dir to the given path.
    char *full_path = get_full_path(short_path);

    // Initially we set set the return code to be 0.
    *ret = 0;

    // Let sys_ret be the return code from the mknod system call.

    int sys_ret = 0;

    sys_ret = rw_lock_unlock(&file_lock, mode);

    if (sys_ret < 0) {
        // If there is an error on the system call, then the return code should
        // be -errno.
        *ret = -errno;
    }

    // Clean up the full path, it was allocated on the heap.
    free(full_path);

    //DLOG("Returning code: %d", *ret);
    // The RPC call succeeded, so return 0.
   
    return 0;
}



// The main function of the server.
int main(int argc, char *argv[]) {
    // argv[1] should contain the directory where you should store data on the
    // server. If it is not present it is an error, that we cannot recover from.
    if (argc != 2) {
        // In general you shouldn't print to stderr or stdout, but it may be
        // helpful here for debugging. Important: Make sure you turn off logging
        // prior to submission!
        // See watdfs_client.c for more details
        // # ifdef PRINT_ERR
        // std::cerr << "Usaage:" << argv[0] << " server_persist_dir";
        // #endif
        return -1;
    }
    // Store the directory in a global variable.
    server_persist_dir = argv[1];

    //------------------------------------------------------------------
    //------------------------------------------------------------------

    int ret = 0;    
    ret = rpcServerInit();

    if (ret < 0){
        cout << "Server Initialisation failed";
        return ret;
    }

    ret = rw_lock_init(&file_lock);

    if(ret < 0){
        DLOG("Lock initialization failed");
    }

    //------------------------------------------------------------------
    //------------------------------------------------------------------



    // TODO: Initialize the rpc library by calling `rpcServerInit`.
    // Important: `rpcServerInit` prints the 'export SERVER_ADDRESS' and
    // 'export SERVER_PORT' lines. Make sure you *do not* print anything
    // to *stdout* before calling `rpcServerInit`.
    //DLOG("Initializing server...");

   

    // TODO: If there is an error with `rpcServerInit`, it maybe useful to have
    // debug-printing here, and then you should return.

    // TODO: Register your functions with the RPC library.
    // Note: The braces are used to limit the scope of `argTypes`, so that you can
    // reuse the variable for multiple registrations. Another way could be to
    // remove the braces and use `argTypes0`, `argTypes1`, etc.
    {
        // There are 3 args for the function (see watdfs_client.c for more
        // detail).
        int argTypes[4];
        // First is the path.
        argTypes[0] =
            (1u << ARG_INPUT) | (1u << ARG_ARRAY) | (ARG_CHAR << 16u) | 1u;
        // The second argument is the statbuf.
        argTypes[1] =
            (1u << ARG_OUTPUT) | (1u << ARG_ARRAY) | (ARG_CHAR << 16u) | 1u;
        // The third argument is the retcode.
        argTypes[2] = (1u << ARG_OUTPUT) | (ARG_INT << 16u);
        // Finally we fill in the null terminator.
        argTypes[3] = 0;

        // We need to register the function with the types and the name.
        ret = rpcRegister((char *)"getattr", argTypes, watdfs_getattr);
        if (ret < 0) {
            // It may be useful to have debug-printing here.
            cout << "rpcRegister failed";
            return ret;
        }
    }

    //------------------------------------------------------------------
    //------------------------------------------------------------------
    //------------------------------------------------------------------
    //------------------------------------------------------------------
    //------------------------------------------------------------------

    {

        // There are 4 args for the function (see watdfs_client.c for more
        // detail).


        int argTypes[5];

        // First is the path.
        argTypes[0] =
            (1u << ARG_INPUT) | (1u << ARG_ARRAY) | (ARG_CHAR << 16u) | 1u;

        // Second argument is mode datatype
        argTypes[1] = (1u << ARG_INPUT) | (ARG_INT << 16u);

        // The third argument is the dev datatype.
        argTypes[2] = (1u << ARG_INPUT) | (ARG_LONG << 16u);

        // The fourth argument is the returncode
        argTypes[3] = (1u << ARG_OUTPUT) | (ARG_INT << 16u);

        // Finally we fill in the null terminator.
        argTypes[4] = 0;

        // We need to register the function with the types and the name.
        ret = rpcRegister((char *)"mknod", argTypes, watdfs_mknod);
        if (ret < 0) {
            // It may be useful to have debug-printing here.
            cout << "rpcRegister failed";

            return ret;
        }


    }

    {


        // There are 4 args for the function (see watdfs_client.c for more
        // detail).
        int argTypes[4];

        // First is the path.
        argTypes[0] = (1u << ARG_INPUT) | (1u << ARG_ARRAY) | (ARG_CHAR << 16u) | 1u;

        // The second argument is the fuse_file_info structure.
        argTypes[1] = (1u << ARG_INPUT) | (1u << ARG_OUTPUT) | (1u << ARG_ARRAY) | (ARG_CHAR << 16u) | 1u;

        // The third argument is the retcode.
        argTypes[2] = (1u << ARG_OUTPUT) | (ARG_INT << 16u);

        // Finally we fill in the null terminator.
        argTypes[3] = 0;

        ret = rpcRegister((char *)"open", argTypes, watdfs_open);
        if (ret < 0) {
            // It may be useful to have debug-printing here.
            cout << "rpcRegister failed";
            return ret;
        }

    }

    {


        // There are 4 args for the function (see watdfs_client.c for more
        // detail).
        int argTypes[4];

        // First is the path.
        argTypes[0] = (1u << ARG_INPUT) | (1u << ARG_ARRAY) | (ARG_CHAR << 16u) | 1u;

        // The second argument is the fuse_file_info structure.
        argTypes[1] = (1u << ARG_INPUT) | (1u << ARG_ARRAY) | (ARG_CHAR << 16u) | 1u;

        // The third argument is the retcode.
        argTypes[2] = (1u << ARG_OUTPUT) | (ARG_INT << 16u);

        // Finally we fill in the null terminator.
        argTypes[3] = 0;

        ret = rpcRegister((char *)"release", argTypes, watdfs_release);
        if (ret < 0) {
            // It may be useful to have debug-printing here.
            cout << "rpcRegister failed";
            return ret;
        }

    }


    {


        // There are 4 args for the function (see watdfs_client.c for more
        // detail).
        int argTypes[4];

        // First is the path.
        argTypes[0] = (1u << ARG_INPUT) | (1u << ARG_ARRAY) | (ARG_CHAR << 16u) | 1u;

        // The second argument is the array of struct timespec structure.
        argTypes[1] = (1u << ARG_INPUT) | (1u << ARG_ARRAY) | (ARG_CHAR << 16u) | 1u;

        // The third argument is the retcode.
        argTypes[2] = (1u << ARG_OUTPUT) | (ARG_INT << 16u);

        // Finally we fill in the null terminator.
        argTypes[3] = 0;

        ret = rpcRegister((char *)"utimens", argTypes, watdfs_utimens);
        if (ret < 0) {
            // It may be useful to have debug-printing here.
            cout << "rpcRegister failed";
            return ret;
        }

    }

    {


        // There are 7 args for the function (see watdfs_client.c for more
        // detail).
        int argTypes[7];

        // First is the path.
        argTypes[0] = (1u << ARG_INPUT) | (1u << ARG_ARRAY) | (ARG_CHAR << 16u) | 1u;

        // The second argument is the buffer.
        argTypes[1] = (1u << ARG_OUTPUT) | (1u << ARG_ARRAY) | (ARG_CHAR << 16u) | 1u;

        // The third argument is the size_t data type.
        argTypes[2] = (1u << ARG_INPUT) | (ARG_LONG << 16u);

        // The fourth argument is the off_t data type.
        argTypes[3] = (1u << ARG_INPUT) | (ARG_LONG << 16u);

        // The fifth argument is the fuse_file_info data type.
        argTypes[4] = (1u << ARG_INPUT) | (1u << ARG_ARRAY) | (ARG_CHAR << 16u) | 1u;

        // The sixth argument is the return data type.
        argTypes[5] = (1u << ARG_OUTPUT) | (ARG_INT << 16u);

        // Finally we fill in the null terminator.
        argTypes[6] = 0;

        ret = rpcRegister((char *)"read", argTypes, watdfs_read);
        if (ret < 0) {
            // It may be useful to have debug-printing here.
            cout << "rpcRegister failed";
            return ret;
        }


    }

    {


        // There are 7 args for the function (see watdfs_client.c for more
        // detail).
        int argTypes[7];

        // First is the path.
        argTypes[0] = (1u << ARG_INPUT) | (1u << ARG_ARRAY) | (ARG_CHAR << 16u) | 1u;

        // The second argument is the buffer.
        argTypes[1] = (1u << ARG_INPUT) | (1u << ARG_ARRAY) | (ARG_CHAR << 16u) | 1u;

        // The third argument is the size_t data type.
        argTypes[2] = (1u << ARG_INPUT) | (ARG_LONG << 16u);

        // The fourth argument is the off_t data type.
        argTypes[3] = (1u << ARG_INPUT) | (ARG_LONG << 16u);

        // The fifth argument is the fuse_file_info data type.
        argTypes[4] = (1u << ARG_INPUT) | (1u << ARG_ARRAY) | (ARG_CHAR << 16u) | 1u;

        // The sixth argument is the return data type.
        argTypes[5] = (1u << ARG_OUTPUT) | (ARG_INT << 16u);

        // Finally we fill in the null terminator.
        argTypes[6] = 0;

        ret = rpcRegister((char *)"write", argTypes, watdfs_write);
        if (ret < 0) {
            // It may be useful to have debug-printing here.
            cout << "rpcRegister failed";
            return ret;
        }


    }

    {


        // There are 4 args for the function (see watdfs_client.c for more
        // detail).
        int argTypes[4];

        // First is the path.
        argTypes[0] = (1u << ARG_INPUT) | (1u << ARG_ARRAY) | (ARG_CHAR << 16u) | 1u;

        // The second argument is the off_t data type..
        argTypes[1] = (1u << ARG_INPUT) | (ARG_LONG << 16u);

        // The third argument is the retcode.
        argTypes[2] = (1u << ARG_OUTPUT) | (ARG_INT << 16u);

        // Finally we fill in the null terminator.
        argTypes[3] = 0;

        ret = rpcRegister((char *)"truncate", argTypes, watdfs_truncate);
        if (ret < 0) {
            // It may be useful to have debug-printing here.
            cout << "rpcRegister failed";
            return ret;
        }

    }










    {

        // There are 4 args for the function (see watdfs_client.c for more
        // detail).
        int argTypes[4];

        // First is the path.
        argTypes[0] = (1u << ARG_INPUT) | (1u << ARG_ARRAY) | (ARG_CHAR << 16u) | 1u;

        // The second argument is the fuse_file_info structure.
        argTypes[1] = (1u << ARG_INPUT) | (1u << ARG_ARRAY) | (ARG_CHAR << 16u) | 1u;

        // The third argument is the retcode.
        argTypes[2] = (1u << ARG_OUTPUT) | (ARG_INT << 16u);

        // Finally we fill in the null terminator.
        argTypes[3] = 0;

        ret = rpcRegister((char *)"fsync", argTypes, watdfs_fsync);
        if (ret < 0) {
            // It may be useful to have debug-printing here.
            cout << "rpcRegister failed";
            return ret;
        }

    }

    {


        // There are 4 args for the function (see watdfs_client.c for more
        // detail).
        int argTypes[4];

        // First is the path.
        argTypes[0] = (1u << ARG_INPUT) | (1u << ARG_ARRAY) | (ARG_CHAR << 16u) | 1u;

        // Second argument is mode datatype
        argTypes[1] = (1u << ARG_INPUT) | (ARG_INT << 16u);

        // The third argument is the retcode.
        argTypes[2] = (1u << ARG_OUTPUT) | (ARG_INT << 16u);

        // Finally we fill in the null terminator.
        argTypes[3] = 0;

        ret = rpcRegister((char *)"lock", argTypes, watdfs_lock);
        if (ret < 0) {
            // It may be useful to have debug-printing here.
            cout << "rpcRegister failed";
            return ret;
        }
    }

    {


        // There are 4 args for the function (see watdfs_client.c for more
        // detail).
        int argTypes[4];

        // First is the path.
        argTypes[0] = (1u << ARG_INPUT) | (1u << ARG_ARRAY) | (ARG_CHAR << 16u) | 1u;

        // Second argument is mode datatype
        argTypes[1] = (1u << ARG_INPUT) | (ARG_INT << 16u);

        // The third argument is the retcode.
        argTypes[2] = (1u << ARG_OUTPUT) | (ARG_INT << 16u);

        // Finally we fill in the null terminator.
        argTypes[3] = 0;

        ret = rpcRegister((char *)"unlock", argTypes, watdfs_unlock);
        if (ret < 0) {
            // It may be useful to have debug-printing here.
            cout << "rpcRegister failed";
            return ret;
        }

    }



    //------------------------------------------------------------------
    //------------------------------------------------------------------
    //------------------------------------------------------------------
    //------------------------------------------------------------------
    //------------------------------------------------------------------
    

    // TODO: Hand over control to the RPC library by calling `rpcExecute`.

    // rpcExecute could fail so you may want to have debug-printing here, and
    // then you should return.


    //------------------------------------------------------------------
    //------------------------------------------------------------------

    ret = rpcExecute();
    if (ret < 0){
        cout << "rpcExecute failed";
    }

    //------------------------------------------------------------------
    //------------------------------------------------------------------
    



    return ret;
}
