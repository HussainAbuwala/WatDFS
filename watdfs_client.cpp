//
// Starter code for CS 454/654
// You SHOULD change this file
//

#include "watdfs_client.h"
#include "debug.h"
#include <iostream>
#include <cmath>
#include <map>
#include <fstream>
#include <vector>




INIT_LOG

#include "rpc.h"
using namespace std;
#include "rw_lock.h"

struct File_metadata
{
    int is_open;
    int open_mode;
    int fh_client;
    int fh_server;
    time_t Tc; // Tc be the time the cache entry was last validated by the client.

};

vector <pair<std::string,struct File_metadata>> vec;


/*struct Client_metadata
{
    const char *path_to_cache;
    time_t cache_interval;
    map<const char*,struct File_metadata*> fm;
    //map<const char*, int> s_f_e;

};*/

const char *cache_path = nullptr;
time_t interval_cache;

map<const char*,struct File_metadata> file_meta;
FILE *pf;


// SETUP AND TEARDOWN
void *watdfs_cli_init(struct fuse_conn_info *conn, const char *path_to_cache,
                      time_t cache_interval, int *ret_code) {
    // TODO: set up the RPC library by calling `rpcClientInit`.

    // TODO: check the return code of the `rpcClientInit` call
    // `rpcClientInit` may fail, for example, if an incorrect port was exported.

	//------------------------------------------------------------------
	//------------------------------------------------------------------
	//------------------------------------------------------------------
    *ret_code = 0;

    int rpcRet;
	rpcRet = rpcClientInit();
    if (rpcRet < 0){
        *ret_code = rpcRet;
        cout << "rpcClientInit failed";
    }

	//------------------------------------------------------------------
	//------------------------------------------------------------------
	//------------------------------------------------------------------

    // It may be useful to print to stderr or stdout during debugging.
    // Important: Make sure you turn off logging prior to submission!
    // One useful technique is to use pre-processor flags like:
    // # ifdef PRINT_ERR
    // std::cerr << "Failed to initialize RPC Client" << std::endl;
    // #endif
    // Tip: Try using a macro for the above to minimize the debugging code.

    // TODO Initialize any global state that you require for the assignment and return it.
    // The value that you return here will be passed as userdata in other functions.
    // In A1, you might not need it, so you can return `nullptr`.
    //struct Client_metadata cmd;
    DLOG("IN watdfs_init: path");

    cache_path = path_to_cache;
    interval_cache = cache_interval;
    DLOG("IN watdfs_init: path '%s'", cache_path);
    DLOG("IN watdfs_init: path '%ld'", interval_cache);


    pf = fopen("debug_info.txt","w");

    void *userdata = NULL;

    // TODO: save `path_to_cache` and `cache_interval` (for A3).

    // TODO: set `ret_code` to 0 if everything above succeeded else some appropriate
    // non-zero value.

     //------------------------------------------------------------------
	//------------------------------------------------------------------
     //------------------------------------------------------------------
	//------------------------------------------------------------------


    // Return pointer to global state data.
    return userdata;
}

int watdfs_cli_getattr_aux(void *userdata, const char *path, struct stat *statbuf) {
     // SET UP THE RPC CALL
    DLOG("watdfs_cli_getattr called for '%s'", path);
    
    // getattr has 3 arguments.
    int ARG_COUNT = 3;

    // Allocate space for the output arguments.
    void **args = new void*[ARG_COUNT];

    // Allocate the space for arg types, and one extra space for the null
    // array element.
    int arg_types[ARG_COUNT + 1];

    // The path has string length (strlen) + 1 (for the null character).
    int pathlen = strlen(path) + 1;

    // Fill in the arguments
    // The first argument is the path, it is an input only argument, and a char
    // array. The length of the array is the length of the path.
    arg_types[0] =
        (1u << ARG_INPUT) | (1u << ARG_ARRAY) | (ARG_CHAR << 16u) | (uint) pathlen;
    // For arrays the argument is the array pointer, not a pointer to a pointer.
    args[0] = (void *)path;

    // The second argument is the stat structure. This argument is an output
    // only argument, and we treat it as a char array. The length of the array
    // is the size of the stat structure, which we can determine with sizeof.
    arg_types[1] = (1u << ARG_OUTPUT) | (1u << ARG_ARRAY) | (ARG_CHAR << 16u) |
                   (uint) sizeof(struct stat); // statbuf
    args[1] = (void *)statbuf;

    // The third argument is the return code, an output only argument, which is
    // an integer.
    // TODO: fill in this argument type.
    // The return code is not an array, so we need to hand args[2] an int*.
    // The int* could be the address of an integer located on the stack, or use
    // a heap allocated integer, in which case it should be freed.
    // TODO: Fill in the argument


    //------------------------------------------------------------------
	//------------------------------------------------------------------
	//------------------------------------------------------------------

    // The third argument is the returncode. This argument is an output
    // only argument, it is an integer.
    int retcode;
    arg_types[2] = (1u << ARG_OUTPUT) | (ARG_INT << 16u);
    args[2] = (void *)&retcode;


	//------------------------------------------------------------------
	//------------------------------------------------------------------
	//------------------------------------------------------------------




    // Finally, the last position of the arg types is 0. There is no
    // corresponding arg.
    arg_types[3] = 0;

    // MAKE THE RPC CALL
    int rpc_ret = rpcCall((char *)"getattr", arg_types, args);

    // HANDLE THE RETURN
    // The integer value watdfs_cli_getattr will return.
    int fxn_ret = 0;
    if (rpc_ret < 0) {
        DLOG("getattr rpc failed with error '%d'", rpc_ret);
        // Something went wrong with the rpcCall, return a sensible return
        // value. In this case lets return, -EINVAL
        fxn_ret = -EINVAL;
    } else {
        // Our RPC call succeeded. However, it's possible that the return code
        // from the server is not 0, that is it may be -errno. Therefore, we
        // should set our function return value to the retcode from the server.
		// TODO: set the function return value to the return code from the server.

    	//------------------------------------------------------------------
		//------------------------------------------------------------------
		//------------------------------------------------------------------

    	//fxn_ret = *((int *)args[2]);
        fxn_ret = retcode;


    	//------------------------------------------------------------------
		//------------------------------------------------------------------
		//------------------------------------------------------------------


        
    }

    if (fxn_ret < 0) {
        // If the return code of watdfs_cli_getattr is negative (an error), then 
        // we need to make sure that the stat structure is filled with 0s. Otherwise,
        // FUSE will be confused by the contradicting return values.
        memset(statbuf, 0, sizeof(struct stat));
    }

    // Clean up the memory we have allocated.
    delete []args;

    return fxn_ret;
}

int watdfs_cli_open_aux(void *userdata, const char *path, struct fuse_file_info *fi){
    
     // SET UP THE RPC CALL
    DLOG("watdfs_cli_open called for '%s'", path);

    // watdfs_cli_open has 3 arguments.
    int ARG_COUNT = 3;

    // Allocate space for the output arguments.
    void **args = new void*[ARG_COUNT];

    // Allocate the space for arg types, and one extra space for the null
    // array element.
    int arg_types[ARG_COUNT + 1];

    // The path has string length (strlen) + 1 (for the null character).
    int pathlen = strlen(path) + 1;

    // Fill in the arguments
    // The first argument is the path, it is an input only argument, and a char
    // array. The length of the array is the length of the path.
    arg_types[0] = (1u << ARG_INPUT) | (1u << ARG_ARRAY) | (ARG_CHAR << 16u) | (uint) pathlen;
    // For arrays the argument is the array pointer, not a pointer to a pointer.
    args[0] = (void *)path;

    // The second argument is the fuse_file_info structure. This argument is an input and output
    // argument, and we treat it as a char array. The length of the array
    // is the size of the stat structure, which we can determine with sizeof.
    arg_types[1] = (1u << ARG_INPUT) | (1u << ARG_OUTPUT) | (1u << ARG_ARRAY) | (ARG_CHAR << 16u) |
                   (uint) sizeof(struct fuse_file_info); // statbuf
    args[1] = (void *)fi;

    // The third argument is the returncode. This argument is an output
    // only argument, it is an integer.
    int retcode;
    arg_types[2] = (1u << ARG_OUTPUT) | (ARG_INT << 16u);
    args[2] = (void *)&retcode;

    // Finally, the last position of the arg types is 0. There is no
    // corresponding arg.
    arg_types[3] = 0;

    // MAKE THE RPC CALL
    int rpc_ret = rpcCall((char *)"open", arg_types, args);

    // HANDLE THE RETURN
    // The integer value watdfs_cli_open will return.

    int fxn_ret = 0;
    if (rpc_ret < 0) {
        DLOG("getattr rpc failed with error '%d'", rpc_ret);
        // Something went wrong with the rpcCall, return a sensible return
        // value. In this case lets return, -EINVAL
        fxn_ret = -EINVAL;
    } else {
        // Our RPC call succeeded. However, it's possible that the return code
        // from the server is not 0, that is it may be -errno. Therefore, we
        // should set our function return value to the retcode from the server.
		// TODO: set the function return value to the return code from the server.

    	//------------------------------------------------------------------
		//------------------------------------------------------------------
		//------------------------------------------------------------------
        fxn_ret = retcode;

    }

    // Clean up the memory we have allocated.
    delete []args;

    return fxn_ret;

}

int watdfs_cli_read_aux(void *userdata, const char *path, char *buf,
                         size_t size, off_t offset, struct fuse_file_info *fi) {
    
    DLOG("watdfs_cli_read called for '%s'", path);

    //strcpy(buf,"");    

    int ARG_COUNT = 6;

    // Allocate space for the output arguments.
    void **args = new void*[ARG_COUNT];

    // Allocate the space for arg types, and one extra space for the null
    // array element.
    int arg_types[ARG_COUNT + 1];

    int pathlen = strlen(path) + 1;

    // The first argument is the path, it is an input only argument, and a char
    // array. The length of the array is the length of the path.
    arg_types[0] = (1u << ARG_INPUT) | (1u << ARG_ARRAY) | (ARG_CHAR << 16u) | (uint) pathlen;
    // For arrays the argument is the array pointer, not a pointer to a pointer.
    args[0] = (void *)path;

    // The fifth argument is the fuse_file_info structure. This argument is an input
    // argument, and we treat it as a char array. The length of the array
    // is the size of the stat structure, which we can determine with sizeof.
    arg_types[4] = (1u << ARG_INPUT) | (1u << ARG_ARRAY) | (ARG_CHAR << 16u) |
                    (uint) sizeof(struct fuse_file_info); // statbuf
    
    args[4] = (void *)fi;

    // The 6th argument is the returncode. This argument is an output
    // only argument, it is an integer.
    int retcode;
    arg_types[5] = (1u << ARG_OUTPUT) | (ARG_INT << 16u);
    args[5] = (void *)&retcode;

    // Finally, the last position of the arg types is 0. There is no
    // corresponding arg.
    arg_types[6] = 0;


    size_t max_allowed = 65535;

    size_t total_bytes_read = 0;

    size_t temp_bytes_read = 0;

    size_t bytes_left_to_read = size;

    size_t current_number_of_bytes_to_read;

    off_t offset_to_read_from = offset;

    char *temp_buf = buf;


    do {

        if(bytes_left_to_read <= max_allowed){
            current_number_of_bytes_to_read = bytes_left_to_read;
        }
        else{
            current_number_of_bytes_to_read = max_allowed;
        }

        arg_types[1] = (1u << ARG_OUTPUT) | (1u << ARG_ARRAY) | (ARG_CHAR << 16u) | (uint) current_number_of_bytes_to_read;
        args[1] = (void *)temp_buf;

        arg_types[2] = (1u << ARG_INPUT) | (ARG_LONG << 16u);
        args[2] = (void *)&current_number_of_bytes_to_read;

        arg_types[3] = (1u << ARG_INPUT) | (ARG_LONG << 16u);
        args[3] = (void *)&offset_to_read_from;

        // MAKE THE RPC CALL
        int rpc_ret = rpcCall((char *)"read", arg_types, args);

        // HANDLE THE RETURN
        // The integer value watdfs_release will return.

        int fxn_ret = 0;
        if (rpc_ret < 0) {
            DLOG("read rpc failed with error '%d'", rpc_ret);
            fxn_ret = -EINVAL;
        } 
        else {
            fxn_ret = retcode;
        }

        if(fxn_ret < 0){return fxn_ret;}

        temp_bytes_read = fxn_ret;

        total_bytes_read = total_bytes_read + temp_bytes_read;

        offset_to_read_from = offset_to_read_from + temp_bytes_read;

        bytes_left_to_read = bytes_left_to_read - temp_bytes_read;

        temp_buf = temp_buf + temp_bytes_read;


    }
    while (temp_bytes_read != 0 && total_bytes_read != size);

    delete []args;

    return total_bytes_read;


}

int watdfs_cli_write_aux(void *userdata, const char *path, const char *buf,
                     size_t size, off_t offset, struct fuse_file_info *fi){
// Write size amount of data at offset of file from buf.

    // Remember that size may be greater then the maximum array size of the RPC
    // library.


    DLOG("watdfs_cli_write called for '%s'", path);

    int ARG_COUNT = 6;

    // Allocate space for the output arguments.
    void **args = new void*[ARG_COUNT];

    // Allocate the space for arg types, and one extra space for the null
    // array element.
    int arg_types[ARG_COUNT + 1];

    int pathlen = strlen(path) + 1;

    // The first argument is the path, it is an input only argument, and a char
    // array. The length of the array is the length of the path.
    arg_types[0] = (1u << ARG_INPUT) | (1u << ARG_ARRAY) | (ARG_CHAR << 16u) | (uint) pathlen;
    // For arrays the argument is the array pointer, not a pointer to a pointer.
    args[0] = (void *)path;

    // The fifth argument is the fuse_file_info structure. This argument is an input
    // argument, and we treat it as a char array. The length of the array
    // is the size of the stat structure, which we can determine with sizeof.
    arg_types[4] = (1u << ARG_INPUT) | (1u << ARG_ARRAY) | (ARG_CHAR << 16u) |
                    (uint) sizeof(struct fuse_file_info); // statbuf
    
    args[4] = (void *)fi;

    // The 6th argument is the returncode. This argument is an output
    // only argument, it is an integer.
    int retcode;
    arg_types[5] = (1u << ARG_OUTPUT) | (ARG_INT << 16u);
    args[5] = (void *)&retcode;

    // Finally, the last position of the arg types is 0. There is no
    // corresponding arg.
    arg_types[6] = 0;

    size_t max_allowed = 65535;

    size_t total_bytes_written = 0;

    size_t temp_bytes_written = 0;

    size_t bytes_left_to_write = size;

    size_t current_number_of_bytes_to_write;

    off_t offset_to_write_from = offset;

    const char *temp_buf = buf;

    do{

        if(bytes_left_to_write <= max_allowed){
            current_number_of_bytes_to_write = bytes_left_to_write;
        }
        else{
            current_number_of_bytes_to_write = max_allowed;
        }

        arg_types[1] = (1u << ARG_INPUT) | (1u << ARG_ARRAY) | (ARG_CHAR << 16u) | (uint) current_number_of_bytes_to_write;
        args[1] = (void *)temp_buf;

        arg_types[2] = (1u << ARG_INPUT) | (ARG_LONG << 16u);
        args[2] = (void *)&current_number_of_bytes_to_write;

        arg_types[3] = (1u << ARG_INPUT) | (ARG_LONG << 16u);
        args[3] = (void *)&offset_to_write_from;

        // MAKE THE RPC CALL
        int rpc_ret = rpcCall((char *)"write", arg_types, args);

        // HANDLE THE RETURN
        // The integer value watdfs_release will return.

        int fxn_ret = 0;
        if (rpc_ret < 0) {
            DLOG("write rpc failed with error '%d'", rpc_ret);
            fxn_ret = -EINVAL;
        } 
        else {
            fxn_ret = retcode;
        }

        if(fxn_ret < 0){return fxn_ret;}

        temp_bytes_written = fxn_ret;

        total_bytes_written = total_bytes_written + temp_bytes_written;

        offset_to_write_from = offset_to_write_from + temp_bytes_written;

        bytes_left_to_write = bytes_left_to_write - temp_bytes_written;

        temp_buf = temp_buf + temp_bytes_written;


    }while(total_bytes_written != size);

    delete []args;

    return total_bytes_written;
}

int watdfs_cli_truncate_aux(void *userdata, const char *path, off_t newsize){
    // Change the file size to newsize.

    DLOG("watdfs_cli_truncate called for '%s'", path);

    //strcpy(buf,"");    

    int ARG_COUNT = 3;

    // Allocate space for the output arguments.
    void **args = new void*[ARG_COUNT];

    // Allocate the space for arg types, and one extra space for the null
    // array element.
    int arg_types[ARG_COUNT + 1];

    int pathlen = strlen(path) + 1;

    // The first argument is the path, it is an input only argument, and a char
    // array. The length of the array is the length of the path.
    arg_types[0] = (1u << ARG_INPUT) | (1u << ARG_ARRAY) | (ARG_CHAR << 16u) | (uint) pathlen;
    // For arrays the argument is the array pointer, not a pointer to a pointer.
    args[0] = (void *)path;

    // The second argument is the off_t datatype. This argument is an input
    // only argument, it is LONG.
    arg_types[1] = (1u << ARG_INPUT) | (ARG_LONG << 16u);
    args[1] = (void *)&newsize;

    // The 3rd argument is the returncode. This argument is an output
    // only argument, it is an integer.
    int retcode;
    arg_types[2] = (1u << ARG_OUTPUT) | (ARG_INT << 16u);
    args[2] = (void *)&retcode;

    // Finally, the last position of the arg types is 0. There is no
    // corresponding arg.
    arg_types[3] = 0;

    // MAKE THE RPC CALL
    int rpc_ret = rpcCall((char *)"truncate", arg_types, args);

    // HANDLE THE RETURN
    // The integer value watdfs_release will return.

    int fxn_ret = 0;
    if (rpc_ret < 0) {
        DLOG("getattr rpc failed with error '%d'", rpc_ret);
        // Something went wrong with the rpcCall, return a sensible return
        // value. In this case lets return, -EINVAL
        fxn_ret = -EINVAL;
    } else {
        // Our RPC call succeeded. However, it's possible that the return code
        // from the server is not 0, that is it may be -errno. Therefore, we
        // should set our function return value to the retcode from the server.
        // TODO: set the function return value to the return code from the server.

        //------------------------------------------------------------------
        //------------------------------------------------------------------
        //------------------------------------------------------------------

        fxn_ret = retcode;
        //fxn_ret = *((int *)args[5]);


        //------------------------------------------------------------------
        //------------------------------------------------------------------
        //------------------------------------------------------------------


        
    }

     // Clean up the memory we have allocated.
    delete []args;
    
    return fxn_ret;
}

int watdfs_cli_utimens_aux(void *userdata, const char *path,
                       const struct timespec ts[2]){
    // Change file access and modification times.


    DLOG("watdfs_cli_utimens called for '%s'", path);

    // watdfs_cli_open has 3 arguments.
    int ARG_COUNT = 3;

    // Allocate space for the output arguments.
    void **args = new void*[ARG_COUNT];

    // Allocate the space for arg types, and one extra space for the null
    // array element.
    int arg_types[ARG_COUNT + 1];

    // The path has string length (strlen) + 1 (for the null character).
    int pathlen = strlen(path) + 1;

    // Fill in the arguments
    // The first argument is the path, it is an input only argument, and a char
    // array. The length of the array is the length of the path.
    arg_types[0] = (1u << ARG_INPUT) | (1u << ARG_ARRAY) | (ARG_CHAR << 16u) | (uint) pathlen;
    // For arrays the argument is the array pointer, not a pointer to a pointer.
    args[0] = (void *)path;


    // The second argument is the array of struct timespec structure. This argument is an input
    // argument, and we treat it as a char array. The length of the array
    // is the size of the array, which we can determine with sizeof.
    arg_types[1] = (1u << ARG_INPUT) | (1u << ARG_ARRAY) | (ARG_CHAR << 16u) |
                   (uint) (2 * sizeof(struct timespec)); // statbuf
    args[1] = (void *)ts;

    // The third argument is the returncode. This argument is an output
    // only argument, it is an integer.
    int retcode;
    arg_types[2] = (1u << ARG_OUTPUT) | (ARG_INT << 16u);
    args[2] = (void *)&retcode;

    // Finally, the last position of the arg types is 0. There is no
    // corresponding arg.
    arg_types[3] = 0;

    // MAKE THE RPC CALL
    int rpc_ret = rpcCall((char *)"utimens", arg_types, args);

    // HANDLE THE RETURN
    // The integer value watdfs_utimens will return.

    int fxn_ret = 0;
    if (rpc_ret < 0) {
        DLOG("getattr rpc failed with error '%d'", rpc_ret);
        // Something went wrong with the rpcCall, return a sensible return
        // value. In this case lets return, -EINVAL
        fxn_ret = -EINVAL;
    } else {
        // Our RPC call succeeded. However, it's possible that the return code
        // from the server is not 0, that is it may be -errno. Therefore, we
        // should set our function return value to the retcode from the server.
        // TODO: set the function return value to the return code from the server.

        //------------------------------------------------------------------
        //------------------------------------------------------------------
        //------------------------------------------------------------------

        fxn_ret = retcode;
        //fxn_ret = *((int *)args[2]);


        //------------------------------------------------------------------
        //------------------------------------------------------------------
        //------------------------------------------------------------------


        
    }

    // Clean up the memory we have allocated.
    delete []args;

    return fxn_ret;
}

int watdfs_cli_fsync_aux(void *userdata, const char *path, struct fuse_file_info *fi) {
    
    // SET UP THE RPC CALL
    DLOG("watdfs_cli_fsync called for '%s'", path);
    
    // getattr has 3 arguments.
    int ARG_COUNT = 3;

    // Allocate space for the output arguments.
    void **args = new void*[ARG_COUNT];

    // Allocate the space for arg types, and one extra space for the null
    // array element.
    int arg_types[ARG_COUNT + 1];

    // The path has string length (strlen) + 1 (for the null character).
    int pathlen = strlen(path) + 1;

    // Fill in the arguments
    // The first argument is the path, it is an input only argument, and a char
    // array. The length of the array is the length of the path.
    arg_types[0] =
        (1u << ARG_INPUT) | (1u << ARG_ARRAY) | (ARG_CHAR << 16u) | (uint) pathlen;
    // For arrays the argument is the array pointer, not a pointer to a pointer.
    args[0] = (void *)path;

    // The second argument is the fuse_file_info structure. This argument is an input
    // argument, and we treat it as a char array. The length of the array
    // is the size of the stat structure, which we can determine with sizeof.
    arg_types[1] = (1u << ARG_INPUT) | (1u << ARG_ARRAY) | (ARG_CHAR << 16u) |
                   (uint) sizeof(struct fuse_file_info); // statbuf
    args[1] = (void *)fi;

    // The third argument is the returncode. This argument is an output
    // only argument, it is an integer.
    int retcode;
    arg_types[2] = (1u << ARG_OUTPUT) | (ARG_INT << 16u);
    args[2] = (void *)&retcode;

    // Finally, the last position of the arg types is 0. There is no
    // corresponding arg.
    arg_types[3] = 0;

    // MAKE THE RPC CALL
    int rpc_ret = rpcCall((char *)"fsync", arg_types, args);

    // HANDLE THE RETURN
    // The integer value watdfs_cli_fsync will return.
    int fxn_ret = 0;
    if (rpc_ret < 0) {
        DLOG("getattr rpc failed with error '%d'", rpc_ret);
        // Something went wrong with the rpcCall, return a sensible return
        // value. In this case lets return, -EINVAL
        fxn_ret = -EINVAL;
    } else {
        // Our RPC call succeeded. However, it's possible that the return code
        // from the server is not 0, that is it may be -errno. Therefore, we
        // should set our function return value to the retcode from the server.
		// TODO: set the function return value to the return code from the server.

    	//------------------------------------------------------------------
		//------------------------------------------------------------------
		//------------------------------------------------------------------

        fxn_ret = retcode;
    	//fxn_ret = *((int *)args[2]);


    	//------------------------------------------------------------------
		//------------------------------------------------------------------
		//------------------------------------------------------------------


        
    }

    // Clean up the memory we have allocated.
    delete []args;

    // Finally return the value we got from the server.
    return fxn_ret;
    //return -ENOSYS;

}

int watdfs_cli_release_aux(void *userdata, const char *path, struct fuse_file_info *fi) {

    // Called during close, but possibly asynchronously.

    // SET UP THE RPC CALL
    DLOG("watdfs_cli_release called for '%s'", path);

    int ARG_COUNT = 3;

    // Allocate space for the output arguments.
    void **args = new void*[ARG_COUNT];

    // Allocate the space for arg types, and one extra space for the null
    // array element.
    int arg_types[ARG_COUNT + 1];

    // The path has string length (strlen) + 1 (for the null character).
    int pathlen = strlen(path) + 1;

    // Fill in the arguments
    // The first argument is the path, it is an input only argument, and a char
    // array. The length of the array is the length of the path.
    arg_types[0] = (1u << ARG_INPUT) | (1u << ARG_ARRAY) | (ARG_CHAR << 16u) | (uint) pathlen;
    // For arrays the argument is the array pointer, not a pointer to a pointer.
    args[0] = (void *)path;


    // The second argument is the fuse_file_info structure. This argument is an input
    // argument, and we treat it as a char array. The length of the array
    // is the size of the stat structure, which we can determine with sizeof.
    arg_types[1] = (1u << ARG_INPUT) | (1u << ARG_ARRAY) | (ARG_CHAR << 16u) |
                   (uint) sizeof(struct fuse_file_info); // statbuf
    args[1] = (void *)fi;

    // The third argument is the returncode. This argument is an output
    // only argument, it is an integer.
    int retcode;
    arg_types[2] = (1u << ARG_OUTPUT) | (ARG_INT << 16u);
    args[2] = (void *)&retcode;

    // Finally, the last position of the arg types is 0. There is no
    // corresponding arg.
    arg_types[3] = 0;

    // MAKE THE RPC CALL
    int rpc_ret = rpcCall((char *)"release", arg_types, args);

    // HANDLE THE RETURN
    // The integer value watdfs_release will return.

    int fxn_ret = 0;
    if (rpc_ret < 0) {
        DLOG("getattr rpc failed with error '%d'", rpc_ret);
        // Something went wrong with the rpcCall, return a sensible return
        // value. In this case lets return, -EINVAL
        fxn_ret = -EINVAL;
    } else {
        // Our RPC call succeeded. However, it's possible that the return code
        // from the server is not 0, that is it may be -errno. Therefore, we
        // should set our function return value to the retcode from the server.
        // TODO: set the function return value to the return code from the server.

        //------------------------------------------------------------------
        //------------------------------------------------------------------
        //------------------------------------------------------------------

        fxn_ret = retcode;
        //fxn_ret = *((int *)args[2]);


        //------------------------------------------------------------------
        //------------------------------------------------------------------
        //------------------------------------------------------------------


        
    }

    // Clean up the memory we have allocated.
    delete []args;

    return fxn_ret;

}


int watdfs_cli_lock(const char *path, rw_lock_mode_t mode){

    DLOG("watdfs_cli_lock called for '%s'", path);

    // watdfs_cli_open has 3 arguments.
    int ARG_COUNT = 3;

    // Allocate space for the output arguments.
    void **args = new void*[ARG_COUNT];

    // Allocate the space for arg types, and one extra space for the null
    // array element.
    int arg_types[ARG_COUNT + 1];

    // The path has string length (strlen) + 1 (for the null character).
    int pathlen = strlen(path) + 1;

    // Fill in the arguments
    // The first argument is the path, it is an input only argument, and a char
    // array. The length of the array is the length of the path.
    arg_types[0] = (1u << ARG_INPUT) | (1u << ARG_ARRAY) | (ARG_CHAR << 16u) | (uint) pathlen;
    // For arrays the argument is the array pointer, not a pointer to a pointer.
    args[0] = (void *)path;

    // The second argument is the mode datatype. This argument is an input
    // only argument, it is an integer.
    arg_types[1] = (1u << ARG_INPUT) | (ARG_INT << 16u);
    args[1] = (void *)&mode;

    // The third argument is the returncode. This argument is an output
    // only argument, it is an integer.
    int retcode;
    arg_types[2] = (1u << ARG_OUTPUT) | (ARG_INT << 16u);
    args[2] = (void *)&retcode;

    // Finally, the last position of the arg types is 0. There is no
    // corresponding arg.
    arg_types[3] = 0;

    // MAKE THE RPC CALL
    int rpc_ret = rpcCall((char *)"lock", arg_types, args);

    // HANDLE THE RETURN
    // The integer value watdfs_cli_open will return.

    int fxn_ret = 0;
    if (rpc_ret < 0) {
        DLOG("getattr rpc failed with error '%d'", rpc_ret);
        // Something went wrong with the rpcCall, return a sensible return
        // value. In this case lets return, -EINVAL
        fxn_ret = -EINVAL;
    } else {
        // Our RPC call succeeded. However, it's possible that the return code
        // from the server is not 0, that is it may be -errno. Therefore, we
        // should set our function return value to the retcode from the server.
		// TODO: set the function return value to the return code from the server.

    	//------------------------------------------------------------------
		//------------------------------------------------------------------
		//------------------------------------------------------------------
        fxn_ret = retcode;

    }

    // Clean up the memory we have allocated.
    delete []args;

    return fxn_ret;

}

int watdfs_cli_unlock(const char *path, rw_lock_mode_t mode){

    DLOG("watdfs_cli_unlock called for '%s'", path);

    // watdfs_cli_open has 3 arguments.
    int ARG_COUNT = 3;

    // Allocate space for the output arguments.
    void **args = new void*[ARG_COUNT];

    // Allocate the space for arg types, and one extra space for the null
    // array element.
    int arg_types[ARG_COUNT + 1];

    // The path has string length (strlen) + 1 (for the null character).
    int pathlen = strlen(path) + 1;

    // Fill in the arguments
    // The first argument is the path, it is an input only argument, and a char
    // array. The length of the array is the length of the path.
    arg_types[0] = (1u << ARG_INPUT) | (1u << ARG_ARRAY) | (ARG_CHAR << 16u) | (uint) pathlen;
    // For arrays the argument is the array pointer, not a pointer to a pointer.
    args[0] = (void *)path;

    // The second argument is the mode datatype. This argument is an input
    // only argument, it is an integer.
    arg_types[1] = (1u << ARG_INPUT) | (ARG_INT << 16u);
    args[1] = (void *)&mode;

    // The third argument is the returncode. This argument is an output
    // only argument, it is an integer.
    int retcode;
    arg_types[2] = (1u << ARG_OUTPUT) | (ARG_INT << 16u);
    args[2] = (void *)&retcode;

    // Finally, the last position of the arg types is 0. There is no
    // corresponding arg.
    arg_types[3] = 0;

    // MAKE THE RPC CALL
    int rpc_ret = rpcCall((char *)"unlock", arg_types, args);

    // HANDLE THE RETURN
    // The integer value watdfs_cli_open will return.

    int fxn_ret = 0;
    if (rpc_ret < 0) {
        DLOG("getattr rpc failed with error '%d'", rpc_ret);
        // Something went wrong with the rpcCall, return a sensible return
        // value. In this case lets return, -EINVAL
        fxn_ret = -EINVAL;
    } else {
        // Our RPC call succeeded. However, it's possible that the return code
        // from the server is not 0, that is it may be -errno. Therefore, we
        // should set our function return value to the retcode from the server.
		// TODO: set the function return value to the return code from the server.

    	//------------------------------------------------------------------
		//------------------------------------------------------------------
		//------------------------------------------------------------------
        fxn_ret = retcode;

    }

    // Clean up the memory we have allocated.
    delete []args;

    return fxn_ret;

}


int check_file_exist(const char *path){
    for(int i = 0; i < vec.size(); i++){ 
        if(vec[i].first == path){
            return i;
        }
    } 
    return -1;
}


int check_if_file_open(const char *path){
    for(int i = 0; i < vec.size(); i++){ 
        if(vec[i].first == path){
            struct File_metadata f = vec[i].second;
            if(f.is_open == -1){
                return -1;
            }
            return 0;
        }
    } 
    return -1;
}

struct File_metadata get_file_meta(const char *short_path){
    for(int i = 0; i < vec.size(); i++){ 
        if(vec[i].first == short_path){
            return vec[i].second;
        }
    }
    //return -1; 
}

int create_or_update_file_meta(const char *path, struct File_metadata fm){
    int i = check_file_exist(path);
    if(i >= 0){
        vec[i].second = fm;
    }
    else{
        vec.push_back({path,fm});
    }

}

time_t get_server_file_m_time(void *userdata, const char *path){
    struct stat statbuf;
    watdfs_cli_getattr_aux(userdata, path, &statbuf);
    return statbuf.st_mtim.tv_sec;
}



char *get_full_path(const char *short_path) {

    int short_path_len = strlen(short_path);

    int dir_len = strlen(cache_path);

    int full_len = dir_len + short_path_len + 1;

    char *full_path = (char *)malloc(full_len);

    // First fill in the directory.
    strcpy(full_path, cache_path);
    // Then append the path.
    strcat(full_path, short_path);

    DLOG("Full path: %s\n", full_path);

    return full_path;
}


void log_client_server_mtime(const char *path){

    const char *cache_full_path = get_full_path(path);
    time_t server_mtime = get_server_file_m_time(NULL,path);
    struct stat statbuf;
    int ret = stat(cache_full_path,&statbuf);
    int i = check_file_exist(path);

    DLOG("In log_client_server_mtime: T_server  '%ld'",server_mtime);
    DLOG("In log_client_server_mtime: stat m_time  '%ld'",statbuf.st_mtim.tv_sec);


}

int download_file(void *userdata, const char *path){
    
    DLOG("DOWNLOAD FILE STARTING ---------------------------------------------------------");

    DLOG("IN download_file: acquiring read lock");
    int lock_ret = watdfs_cli_lock(path,RW_READ_LOCK);
    DLOG("IN download_file: RETURN VALUE watdfs_cli_lock '%d'", lock_ret);

    
    int ret = 0;

    //open client file in write mode, create also if does not exist

    DLOG("IN download_file: CALLING LOCAL get_full_path");
    const char *cache_full_path = get_full_path(path);
    DLOG("IN download_file: full_path '%s'", cache_full_path);

    //create client file if it does not exist
    if(check_file_exist(path) == -1){

        DLOG("IN download_file: Client file does not exist, so creat it first");
        //ofstream{cache_full_path};
        ret = mknod(cache_full_path,0100640,0);
        DLOG("IN download_file: RETURN VALUE mknod '%d'", ret);


    }

    DLOG("IN download_file: CALLING LOCAL OPEN WITH O_WRONLY");
    int client_fh = open(cache_full_path, O_WRONLY);
    DLOG("IN download_file: RETURN VALUE GET LOCAL OPEN '%d'", client_fh);


    DLOG("IN download_file: CALLING LOCAL TRUNCATE");
    //truncate client file to 0
    ret = truncate(cache_full_path, 0);
    DLOG("IN download_file: RETURN VALUE LOCAL TRUNCATE '%d'", ret);


    // get server file size
    DLOG("IN download_file: CALLING GET ATTR AUX TO GET SERVER FILE SIZE");
    struct stat statbuf;
    ret = watdfs_cli_getattr_aux(userdata, path, &statbuf);
    size_t size = (size_t)statbuf.st_size;
    DLOG("IN download_file: GET ATTR RET VALUE '%d' and file size '%ld'", ret, size);


    DLOG("IN download_file: OPENING SERVER FILE IN READ ONLY MODE");
    struct fuse_file_info fi_server;
    fi_server.flags = O_RDONLY;
    ret = watdfs_cli_open_aux(userdata,path,&fi_server);
    DLOG("IN download_file: RETURN VALUE SERVER OPEN '%d'", ret);


    DLOG("IN download_file: Read SERVER FILE in buffer");
    //read server file to buf
    char buf[size];
    ret = watdfs_cli_read_aux(userdata, path, buf, size, 0, &fi_server);
    DLOG("IN download_file: RETURN VALUE SERVER read '%d'", ret);


    DLOG("IN download_file: local write to client FILE");
    //write buf to client file
    ret = pwrite(client_fh, buf, size, 0);
    DLOG("IN download_file: RETURN VALUE local write '%d'", ret);

    //close files
    ret = close(client_fh);
    ret = watdfs_cli_release_aux(userdata,path,&fi_server);

    DLOG("IN download_file: release read lock");
    int unlock_ret = watdfs_cli_unlock(path,RW_READ_LOCK);
    DLOG("IN download_file: RETURN VALUE watdfs_cli_unlock '%d'", unlock_ret);

    DLOG("DOWNLOAD FILE ENDING ---------------------------------------------------------");

    return ret;
}



int upload_file(void *userdata, const char *path){

    DLOG("upload_file STARTING ---------------------------------------------------------");

    DLOG("IN upload_file: acquiring write lock");
    int lock_ret = watdfs_cli_lock(path,RW_WRITE_LOCK);
    DLOG("IN upload_file: RETURN VALUE watdfs_cli_lock '%d'", lock_ret);

    int i = check_file_exist(path);
    int ret = 0;
    
    DLOG("IN upload_file: CALLING LOCAL get_full_path");
    const char *cache_full_path = get_full_path(path);
    DLOG("IN upload_file: full_path '%s'", cache_full_path);


    DLOG("IN upload_file: CALLING LOCAL OPEN WITH O_RDONLY");
    int client_fh = open(cache_full_path, O_RDONLY);
    DLOG("IN upload_file: RETURN VALUE GET LOCAL OPEN '%d'", client_fh);


    DLOG("IN upload_file: CALLING server side TRUNCATE");
    //truncate client file to 0
    ret = watdfs_cli_truncate_aux(userdata,path,0);
    DLOG("IN upload_file: RETURN VALUE server side TRUNCATE '%d'", ret);


    // get client file size
    DLOG("IN upload_file: CALLING local stat GET client FILE SIZE");
    struct stat statbuf;
    ret = stat(cache_full_path,&statbuf);
    size_t size = (size_t)statbuf.st_size;
    DLOG("IN upload_file: local stat RET VALUE '%d' and file size '%ld'", ret, size);


    DLOG("IN upload_file: use already open server fh in write mode");
    struct fuse_file_info fi_server;
    int server_fh = vec[i].second.fh_server;
    fi_server.fh = server_fh;


    DLOG("IN upload_file: Read client FILE in buffer");
    //read server file to buf
    char buf[size];
    ret = pread(client_fh,buf,size,0);
    //ret = watdfs_cli_read_aux(userdata, path, buf, size, 0, &fi_server);
    DLOG("IN upload_file: RETURN VALUE local client read '%d'", ret);


    DLOG("IN upload_file: rpc write to server FILE");
    //write buf to client file
    ret = watdfs_cli_write_aux(userdata,path,buf,size,0,&fi_server);
    DLOG("IN upload_file: RETURN VALUE server write '%d'", ret);

    //close files
    ret = close(client_fh);
    //ret = watdfs_cli_release_aux(userdata,path,&fi_server);

    DLOG("IN upload_file: release write lock");
    int unlock_ret = watdfs_cli_unlock(path,RW_WRITE_LOCK);
    DLOG("IN upload_file: RETURN VALUE watdfs_cli_unlock '%d'", unlock_ret);

    DLOG("upload_file ENDING ---------------------------------------------------------");

    return ret;
}



int file_freshness_check(void *userdata,const char *path){

    const char *cache_full_path = get_full_path(path);


    time_t current_time = time(NULL);
    struct File_metadata f = get_file_meta(path);

    if((current_time - f.Tc) < interval_cache){
        return 0;
    }

    time_t s_m_time = get_server_file_m_time(userdata,path);
    struct stat buf;
    stat(cache_full_path,&buf);


    if(buf.st_mtim.tv_sec == s_m_time){
        return 0;
    }
    return -1;

}

int interpret_return_value(int ret){
    if(ret < 0){
        return -errno;
    }
    return 0;
}

int client_local_stat(void *userdata, const char *path,struct stat *statbuf){
    const char *cache_full_path = get_full_path(path);
    int ret = stat(cache_full_path,statbuf);
    return interpret_return_value(ret);
}

int local_utimens(const char *path){
    const char *cache_full_path = get_full_path(path);
    int i = check_file_exist(path);

    struct stat buf;
    int ret = watdfs_cli_getattr_aux(NULL,path,&buf);

    struct timespec ts[2];

    struct timespec ts1;
    ts1.tv_nsec = UTIME_OMIT;

    struct timespec ts2;
    ts2 = buf.st_mtim;

    ts[0] = ts1;
    ts[1] = ts2;

    ret = utimensat(AT_FDCWD,cache_full_path,ts,0);
    return ret;

}

int create_client_file_metadata(void *userdata, const char *path, int oc,int fc, int fs,int om){

    struct File_metadata f_m;

    f_m.is_open = oc;
    f_m.fh_client = fc;
    f_m.fh_server = fs;
    f_m.open_mode = om;
    f_m.Tc = time(NULL);

    create_or_update_file_meta(path,f_m);
    local_utimens(path);
   
    return 0;
}


void log_file_meta(const char *path){
    int i = check_file_exist(path);
    DLOG("IN log_file_meta: is_open '%d'", vec[i].second.is_open);
    DLOG("IN log_file_meta: open_mode '%d'", vec[i].second.open_mode);
    DLOG("IN log_file_meta: client file handle '%d'", vec[i].second.fh_client);
    DLOG("IN log_file_meta: server file handle '%d'", vec[i].second.fh_server);
    DLOG("IN log_file_meta: Tc '%ld'", vec[i].second.Tc);
}


void watdfs_cli_destroy(void *userdata) {
    // TODO: clean up your userdata state.
    // TODO: tear down the RPC library by calling `rpcClientDestroy`.

    //------------------------------------------------------------------
	//------------------------------------------------------------------

	rpcClientDestroy();

	//------------------------------------------------------------------
	//------------------------------------------------------------------
}




// GET FILE ATTRIBUTES
int watdfs_cli_getattr(void *userdata, const char *path, struct stat *statbuf) {

    DLOG("IN watdfs_cli_getattr: Starting ---------------------------------- ");

    int check_server_file = watdfs_cli_getattr_aux(userdata,path,statbuf);
    const char *cache_full_path = get_full_path(path);
    int check_client_file_exist = check_file_exist(path);

    if(check_server_file < 0){
        DLOG("IN watdfs_cli_getattr: server file does not exist");
        //memset(statbuf, 0, sizeof(struct stat));
        return check_server_file;
    }
    /*if(check_client_file_exist == -1){
        DLOG("IN watdfs_cli_getattr: client file does not exist");
        //int ret = mknod(cache_full_path,0100640,0);
        //DLOG("IN watdfs_cli_getattr: RETURN VALUE mknod '%d'", ret);

        //ret = create_client_file_metadata(userdata, path, -1,-1, -1,-1);
        return check_server_file;
    }*/

    int ret = 0;

    //server file and client file exists

    int check_client_file_open = check_if_file_open(path);

    if(check_client_file_open == 0){
        DLOG("IN watdfs_cli_getattr: client file is open");
        //client file is open
        int open_mode = vec[check_client_file_exist].second.open_mode;

        if(open_mode == O_RDONLY){
            DLOG("IN watdfs_cli_getattr: client file open in read only");
            //file open in read only mode
            int fresh_check = file_freshness_check(userdata,path);
            
            if(fresh_check == 0){
                DLOG("IN watdfs_cli_getattr: client file is fresh, do local stat, update Tc");
                //file is fresh, do local stat, update Tc
                ret = client_local_stat(userdata,path,statbuf);
                vec[check_client_file_exist].second.Tc = time(NULL);
                return ret;            
            }
            else{
                DLOG("IN watdfs_cli_getattr: client file is not fresh, download file, update file_meta, do local stat");
                //file is not fresh, download file, update file_meta, do local stat
                download_file(userdata,path);
                local_utimens(path);
                vec[check_client_file_exist].second.Tc = time(NULL);
                ret = client_local_stat(userdata,path,statbuf);
                return ret;
            }

        }
        else{
            DLOG("IN watdfs_cli_getattr: client file is open in read-write or write only mode, only do local stat");
            //file is open in read-write or write only mode, only writer, do local stat
            ret = client_local_stat(userdata,path,statbuf);
            return ret;
        }

    }
    else{
        DLOG("IN watdfs_cli_getattr: client file exist but is not open, open and copy file, do local stat, close file");
        //file exist but is not open, open and copy file, do local stat, close file
        struct fuse_file_info fi;
        fi.flags = O_RDONLY;
        watdfs_cli_open(userdata,path,&fi);
        ret = client_local_stat(userdata,path,statbuf);
        watdfs_cli_release(userdata,path,&fi);
        return ret;
    }

}


//------------------------------------------------------------------
//------------------------------------------------------------------
//------------------------------------------------------------------
//------------------------------------------------------------------
//------------------------------------------------------------------

int watdfs_cli_mknod_aux(void *userdata, const char *path, mode_t mode, dev_t dev){
    // Called to create a file.

    //------------------------------------
	//------------------------------------
	// getattr has 3 arguments.
    int ARG_COUNT = 4;

    // Allocate space for the output arguments.
    void **args = new void*[ARG_COUNT];

    // Allocate the space for arg types, and one extra space for the null
    // array element.
    int arg_types[ARG_COUNT + 1];


    // The path has string length (strlen) + 1 (for the null character).
    int pathlen = strlen(path) + 1;

    // Fill in the arguments
    // The first argument is the path, it is an input only argument, and a char
    // array. The length of the array is the length of the path.
    arg_types[0] =
        (1u << ARG_INPUT) | (1u << ARG_ARRAY) | (ARG_CHAR << 16u) | (uint) pathlen;
    // For arrays the argument is the array pointer, not a pointer to a pointer.
    args[0] = (void *)path;



    // The second argument is the mode datatype. This argument is an input
    // only argument, it is an integer.
    arg_types[1] = (1u << ARG_INPUT) | (ARG_INT << 16u);
    args[1] = (void *)&mode;



    // The third argument is the dev datatype. This argument is an input
    // only argument, it is LONG.
    arg_types[2] = (1u << ARG_INPUT) | (ARG_LONG << 16u);
    args[2] = (void *)&dev;



    // The fourth argument is the returncode. This argument is an output
    // only argument, it is an integer.
    int retcode;
    arg_types[3] = (1u << ARG_OUTPUT) | (ARG_INT << 16u);
    args[3] = (void *)&retcode;


    // Finally, the last position of the arg types is 0. There is no
    // corresponding arg.
    arg_types[4] = 0;



    // MAKE THE RPC CALL
    int rpc_ret = rpcCall((char *)"mknod", arg_types, args);

    // HANDLE THE RETURN
    // The integer value watdfs_cli_mknod will return.
    int fxn_ret = 0;
    if (rpc_ret < 0) {
        DLOG("mknod rpc failed with error '%d'", rpc_ret);
        // Something went wrong with the rpcCall, return a sensible return
        // value. In this case lets return, -EINVAL
        fxn_ret = -EINVAL;
    } else {
        // Our RPC call succeeded. However, it's possible that the return code
        // from the server is not 0, that is it may be -errno. Therefore, we
        // should set our function return value to the retcode from the server.

        fxn_ret = retcode;	
        //fxn_ret = *((int *)args[3]);


        // TODO: set the function return value to the return code from the server.
    }

    // Clean up the memory we have allocated.
    delete []args;

    // Finally return the value we got from the server.


    return fxn_ret;

    //------------------------------------
    //------------------------------------
}



// CREATE, OPEN AND CLOSE
int watdfs_cli_mknod(void *userdata, const char *path, mode_t mode, dev_t dev) {
    
    DLOG("IN watdfs_cli_mknod: starting ----------");

    struct stat statbuf;
    int check_server_file = watdfs_cli_getattr_aux(userdata,path, &statbuf);
    int check_client_file_exist = check_file_exist(path);


    //int check_client_file_exist = check_file_exist(path);

    if(check_server_file < 0){

        DLOG("IN watdfs_cli_mknod: server file does not exist, create node client and server");
        //memset(statbuf, 0, sizeof(struct stat));
        int ret = watdfs_cli_mknod_aux(userdata,path,mode,dev);
        DLOG("IN watdfs_cli_mknod: server mknod return value '%d'", ret);
        if(ret < 0){
            return ret;
        }
        ret = mknod(get_full_path(path),mode,dev);
        DLOG("IN watdfs_cli_mknod: client mknod return value '%d'", ret);
        if(ret < 0){
            return -errno;
        }
        ret = create_client_file_metadata(userdata, path, -1,-1, -1,-1);
        log_file_meta(path);

        return 0;


    }
    else if(check_client_file_exist == -1){

        DLOG("IN watdfs_cli_mknod: server file exist but no client file");


        int mknod_ret = mknod(get_full_path(path),mode,dev);
        DLOG("IN watdfs_cli_mknod: client mknod return value '%d'", mknod_ret);
        if(mknod_ret < 0){
            return -errno;
        }
        create_client_file_metadata(userdata, path, -1,-1, -1,-1);
        log_file_meta(path);

        return mknod_ret;

    }
    else{
        DLOG("IN watdfs_cli_mknod: both client and server exist, return -EEXIST");

        return -EEXIST;
    }

}



int watdfs_cli_open(void *userdata, const char *path,struct fuse_file_info *fi){

    /*
    
    Possible scenarios

    1. returns error if server file does not exist and O_CREAT is not passed in flags
    2. creates and opens file if server file does not exist and O_CREAT is passed
    3. File exists and can be opened
    4. File exists but cannot be opened due to mutual exclusion condition 
    
    */
    
    DLOG("watdfs_cli_open STARTING ---------------------------------------------------------");

    int server_fh = -1;

    struct stat test_buf;
    DLOG("IN watdfs_cli_open: CALLING GET ATTR TO CHECK IF SERVER FILE EXIST");
    int ret = watdfs_cli_getattr_aux(userdata,path,&test_buf);
    DLOG("IN watdfs_cli_open: RETURN VALUE GET ATTR '%d'", ret);

    int open_mode = (fi->flags) & (O_ACCMODE);
    DLOG("IN watdfs_cli_open: Open mode '%d'", open_mode);

    //assumption is that client file cannot exist without server file
    //hence if server file doesnt exist, client also doesnt exist and create both files
    //if client exist and server doesnt, then that break the logic of try to open client and copy from server.

    if(ret < 0){
        DLOG("IN watdfs_cli_open: server file does not exist, check O_CREAT flag");

        int check_o_create = (fi->flags) & (O_CREAT);
        if(check_o_create == O_CREAT){
            DLOG("IN watdfs_cli_open: O_CREAT flag given, create client and server");

            ret = watdfs_cli_mknod_aux(userdata,path,0100640,0);
            ret = mknod(get_full_path(path),0100640,0);

            if(open_mode == O_RDWR || open_mode == O_WRONLY){
                DLOG("IN watdfs_cli_open: CHECK IF WRITER EXIST ON SERVER, calling watdfs_cli_open_aux");
                ret = watdfs_cli_open_aux(userdata,path,fi);
                DLOG("IN watdfs_cli_open: watdfs_cli_open_aux return value '%d'",ret);
                if(ret == -EACCES){
                    DLOG("IN watdfs_cli_open: WRITER EXIST ON SERVER, return -EACCES");
                    return ret;
                }
                if(ret == 0){
                    server_fh = fi->fh;
                }
            }

            DLOG("IN watdfs_cli_open: Open downloaded client file with original user flags");
            int client_fh = open(get_full_path(path), open_mode);
            DLOG("IN watdfs_cli_open: RETURN VALUE local open '%d'", client_fh);

            ret = create_client_file_metadata(userdata, path, 1,client_fh, server_fh,open_mode);

            fi->fh = client_fh;

            log_file_meta(path);
            
            DLOG("watdfs_cli_open ENDING ---------------------------------------------------------");

            return 0;

        }
        else{
            DLOG("IN watdfs_cli_open: server file does not exist, and no O_CREAT flag, return");
            return ret;
        }
    }

    
    DLOG("IN watdfs_cli_open: CHECK IF CLIENT FILE EXIST");
    int client_file_exists = check_file_exist(path);
    DLOG("IN watdfs_cli_open: RETURN VALUE check_if_client_file_exists '%d'", client_file_exists);

    int check_o_excl = (fi->flags) & (O_EXCL);

    /*

    At this point, server file must exist, as enforced by previous checks

    1. Client file does not exist
    2. Client exist and opened, so error
    3. Client file exist but is not opened

    1 and 3 follows the same steps of download file, open and create metadata 

    */

    DLOG("IN watdfs_cli_open: CHECK IF CLIENT FILE OPEN");
    int client_file_open = check_if_file_open(path);
    DLOG("IN watdfs_cli_open: RETURN VALUE check_if_client_file_is_open '%d'", client_file_open);

    if(client_file_open == 0){
        DLOG("IN watdfs_cli_open: CLIENT EXISTS AND IS OPEN, RETURNING -EMFILE");
        return -EMFILE;
    }

    if(check_o_excl == O_EXCL){
        DLOG("IN watdfs_cli_open: EXCL FLAG PASSED, REMOVE IT,");
        //fi->flags = open_mode;
    }

    if(open_mode == O_RDWR || open_mode == O_WRONLY){

        DLOG("IN watdfs_cli_open: CHECK IF WRITER EXIST ON SERVER, calling watdfs_cli_open_aux");
        ret = watdfs_cli_open_aux(userdata,path,fi);
        DLOG("IN watdfs_cli_open: watdfs_cli_open_aux return value '%d'",ret);
        if(ret == -EACCES){
            DLOG("IN watdfs_cli_open: WRITER EXIST ON SERVER, return -EACCES");
            return ret;
        }
        if(ret == 0){
            server_fh = fi->fh;
        }

    }

    DLOG("IN watdfs_cli_open: DOWNLOADING FILE");
    ret = download_file(userdata,path);
    DLOG("IN watdfs_cli_open: RETURN VALUE download_file '%d'", ret);

    DLOG("IN watdfs_cli_open: Open downloaded client file with original user flags");
    int client_fh = open(get_full_path(path), fi->flags);
    DLOG("IN watdfs_cli_open: RETURN VALUE local open '%d'", client_fh);

    ret = create_client_file_metadata(userdata, path, 1,client_fh, server_fh,open_mode);
    
    fi->fh = client_fh;

    log_file_meta(path);
    
    DLOG("watdfs_cli_open ENDING ---------------------------------------------------------");

    return 0;


}


int watdfs_cli_release(void *userdata, const char *path,
                       struct fuse_file_info *fi) {
    
    DLOG("Closing FILE STARTING ---------------------------------------------------------");

    int i = check_file_exist(path);
    const char *cache_full_path = get_full_path(path);


    if(vec[i].second.open_mode != O_RDONLY){
        DLOG("IN watdfs_cli_release: file was opened in write mode, flush to server");
        upload_file(userdata,path);

        struct stat statbuf;
        stat(cache_full_path,&statbuf);

        struct timespec ts[2];

        struct timespec ts1;
        ts1.tv_nsec = UTIME_OMIT;

        struct timespec ts2;
        ts2 = statbuf.st_mtim;

        ts[0] = ts1;
        ts[1] = ts2;

        watdfs_cli_utimens_aux(userdata,path,ts);
    }

    DLOG("IN watdfs_cli_release: Fuse_flag '%ld', table_flag '%d'", fi->fh, vec[i].second.fh_client);

    DLOG("IN watdfs_cli_release: calling local close");
    //int ret = close(file_meta[path].fh_client);
    int ret = close(vec[i].second.fh_client);

    DLOG("IN watdfs_cli_release: local close return value '%d'", ret);


    vec[i].second.is_open = -1;
    vec[i].second.open_mode = -1;
    vec[i].second.fh_client = -1;

    if(vec[i].second.fh_server != -1){
        struct fuse_file_info f_s;
        DLOG("IN watdfs_cli_release: close server writer");
        f_s.fh = vec[i].second.fh_server;
        ret = watdfs_cli_release_aux(userdata,path, &f_s);
        DLOG("IN watdfs_cli_release: server close return value '%d'", ret);
    }
    vec[i].second.fh_server = -1;

    return ret;


    DLOG("Closing FILE ENDING ---------------------------------------------------------");

    
}




int watdfs_cli_read(void *userdata, const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {

    DLOG("IN watdfs_cli_read: starting -------------------------------------------");

    int check_client_file_exist = check_file_exist(path);
    int check_client_file_open = check_if_file_open(path);
    const char *cache_full_path = get_full_path(path);
    int ret = 0;

    if(check_client_file_open == 0){

        DLOG("IN watdfs_cli_read: file is open");

        int open_mode = vec[check_client_file_exist].second.open_mode;
        int client_fh = vec[check_client_file_exist].second.fh_client;

        if(open_mode == O_RDONLY){

            DLOG("IN watdfs_cli_read: file open in O_RDONLY");

            
            int fresh_check = file_freshness_check(userdata,path);
            DLOG("IN watdfs_cli_read: fresh check '%d'",fresh_check);

            vec[check_client_file_exist].second.Tc = time(NULL);


            if(fresh_check == 0){
                
                DLOG("IN watdfs_cli_read: file is still fresh, do local read, update tc");

                ret = pread(client_fh,buf,size,offset);
                DLOG("IN watdfs_cli_read: local read return '%d'",ret);

                return ret;  
                //read from local
                //update tc
                //return bytes read
            }
            else{
                DLOG("IN watdfs_cli_read: file is not fresh, download, update meta, local read");
                download_file(userdata,path);
                local_utimens(path);
                ret = pread(client_fh,buf,size,offset);
                DLOG("IN watdfs_cli_read: local read return '%d'",ret);

                return ret;  

                //download from server
                //read local
            }

        }
        else if(open_mode == O_RDWR){
            DLOG("IN watdfs_cli_read: file opened in O_RDWR, only writer, do just local read");

            ret = pread(client_fh,buf,size,offset);
            DLOG("IN watdfs_cli_read: local read return '%d'",ret);

            return ret;
        }
        else{
            DLOG("IN watdfs_cli_read: file opened in O_WRONLY, cannot read, return -EPERM");
            return -EPERM;
        }
        
    }
    else{
        DLOG("IN watdfs_cli_read: FILE not open, open file, download and local read, close");
        struct fuse_file_info fi;
        fi.flags = O_RDONLY;
        ret = watdfs_cli_open(userdata,path,&fi);
        DLOG("IN watdfs_cli_read: open return '%d'",ret);

        ret = pread(fi.fh,buf,size,offset);
        DLOG("IN watdfs_cli_read: local read return '%d'",ret);

        watdfs_cli_release(userdata,path,&fi);
        return ret;
    }

}



int watdfs_cli_write(void *userdata, const char *path, const char *buf,
                     size_t size, off_t offset, struct fuse_file_info *fi) {
    
    DLOG("IN watdfs_cli_write: starting -------------------------------------------");

    int check_client_file_exist = check_file_exist(path);
    int check_client_file_open = check_if_file_open(path);
    const char *cache_full_path = get_full_path(path);
    int ret = 0;

    if(check_client_file_open == 0){
        DLOG("IN watdfs_cli_write: client file is open");

        int open_mode = vec[check_client_file_exist].second.open_mode;
        int client_fh = vec[check_client_file_exist].second.fh_client;


        if(open_mode == O_RDONLY){
            DLOG("IN watdfs_cli_write: client file open in O_RDONLY, hence cannot write, return -EMFILE");

            return -EMFILE;
        }
        else{
            
            DLOG("IN watdfs_cli_write: client file open in write, hence can write");

            int write_bytes = pwrite(client_fh,buf,size,offset);
            DLOG("IN watdfs_cli_write: local write return value '%d'",write_bytes);

            DLOG("IN watdfs_cli_write: find client new modification time");
            struct stat statbuf;
            ret = stat(cache_full_path,&statbuf);
            DLOG("IN watdfs_cli_write: local stat return value '%d'",ret);

            int check_fresh = file_freshness_check(userdata,path);
            DLOG("IN watdfs_cli_write: fresh check after write '%d'",check_fresh);

            vec[check_client_file_exist].second.Tc = time(NULL);


            if(check_fresh == 0){
                DLOG("IN watdfs_cli_write: client file still fresh, no need to flush back");
                return write_bytes;
            }
            else{
                DLOG("IN watdfs_cli_write: file not fresh, upload to server");
                ret = upload_file(userdata,path);
                DLOG("IN watdfs_cli_write: upload return value '%d'",ret);

                struct timespec ts[2];

                struct timespec ts1;
                ts1.tv_nsec = UTIME_OMIT;

                struct timespec ts2;
                ts2 = statbuf.st_mtim;

                ts[0] = ts1;
                ts[1] = ts2;

                ret = watdfs_cli_utimens_aux(userdata,path,ts);
                DLOG("IN watdfs_cli_write: rpc utimens return value '%d'",ret);

                return write_bytes;

            }

        }

    }
    else{
        DLOG("IN watdfs_cli_write: client file is not open");

        struct fuse_file_info fi;
        fi.flags = O_WRONLY;

        DLOG("IN watdfs_cli_write: try open file in write mode");
        int ret = watdfs_cli_open(userdata,path,&fi);
        DLOG("IN watdfs_cli_write: open return value '%d'",ret);
       
        if(ret == -EACCES || ret < 0){
            DLOG("IN watdfs_cli_write: cannot open file in writemode");
            return ret;
        }

        int bytes_written = pwrite(fi.fh,buf,size,offset);
        DLOG("IN watdfs_cli_write: local write return value '%d'",ret);
        ret = watdfs_cli_release(userdata,path,&fi);

        return bytes_written;
   
    }
    
}



int watdfs_cli_truncate(void *userdata, const char *path, off_t newsize) {
    
    DLOG("IN watdfs_cli_truncate: starting -------------------------------------------");

    int check_client_file_exist = check_file_exist(path);
    int check_client_file_open = check_if_file_open(path);
    const char *cache_full_path = get_full_path(path);
    int ret = 0;

    if(check_client_file_open == 0){
        DLOG("IN watdfs_cli_truncate: client file is open");

        int open_mode = vec[check_client_file_exist].second.open_mode;
        int client_fh = vec[check_client_file_exist].second.fh_client;


        if(open_mode == O_RDONLY){
            DLOG("IN watdfs_cli_truncate: client file open in O_RDONLY, hence cannot truncate, return -EMFILE");

            return -EMFILE;
        }
        else{
            
            DLOG("IN watdfs_cli_truncate: client file open in write, hence can truncate");
            int trunc_ret = truncate(cache_full_path,newsize);
            //int write_bytes = pwrite(client_fh,buf,size,offset);
            DLOG("IN watdfs_cli_truncate: local truncate return value '%d'",trunc_ret);

            DLOG("IN watdfs_cli_truncate: find client new modification time");
            struct stat statbuf;
            ret = stat(cache_full_path,&statbuf);

            DLOG("IN watdfs_cli_truncate: local stat return value '%d'",ret);

            int check_fresh = file_freshness_check(userdata,path);
            DLOG("IN watdfs_cli_truncate: fresh check after write '%d'",check_fresh);

            vec[check_client_file_exist].second.Tc = time(NULL);


            if(check_fresh == 0){
                DLOG("IN watdfs_cli_truncate: client file still fresh, no need to flush back");
                return trunc_ret;
            }
            else{
                DLOG("IN watdfs_cli_truncate: file not fresh, upload to server");
                ret = upload_file(userdata,path);
                DLOG("IN watdfs_cli_truncate: upload return value '%d'",ret);

                struct timespec ts[2];

                struct timespec ts1;
                ts1.tv_nsec = UTIME_OMIT;

                struct timespec ts2;
                //ts2 = statbuf.st_mtim;
                ts2 = statbuf.st_mtim;

                ts[0] = ts1;
                ts[1] = ts2;

                ret = watdfs_cli_utimens_aux(userdata,path,ts);
                DLOG("IN watdfs_cli_truncate: rpc utimens return value '%d'",ret);

                return trunc_ret;

            }

        }

    }
    else{
        DLOG("IN watdfs_cli_truncate: client file is not open");

        struct fuse_file_info fi;
        fi.flags = O_WRONLY;

        DLOG("IN watdfs_cli_truncate: try open file in write mode");
        int ret = watdfs_cli_open(userdata,path,&fi);
        DLOG("IN watdfs_cli_truncate: open return value '%d'",ret);
       
        if(ret == -EACCES || ret < 0){
            DLOG("IN watdfs_cli_truncate: cannot open file in writemode");
            return ret;
        }

        int trunc_ret = truncate(cache_full_path,newsize);
        DLOG("IN watdfs_cli_truncate: local truncate return value '%d'",ret);
        
        ret = watdfs_cli_release(userdata,path,&fi);

        return trunc_ret;

    }


}


int watdfs_cli_fsync(void *userdata, const char *path, struct fuse_file_info *fi) {
    
    DLOG("IN watdfs_cli_fsync: starting -------------------------------------------");

    int check_client_file_exist = check_file_exist(path);
    int check_client_file_open = check_if_file_open(path);
    const char *cache_full_path = get_full_path(path);
    int ret = 0;

    if(check_client_file_open == 0){

        DLOG("IN watdfs_cli_fsync: client file is open");
        int open_mode = vec[check_client_file_exist].second.open_mode;

        if(open_mode == O_RDONLY){

            DLOG("IN watdfs_cli_fsync: client file open in O_RDONLY, hence cannot fsync, return -EMFILE");
            return -EMFILE;

        }
        else{

            DLOG("IN watdfs_cli_fsync: file open in write, can do fsync");

            int fsync_ret = fsync(vec[check_client_file_exist].second.fh_client);
            DLOG("IN watdfs_cli_fsync: local fsync return value '%d'",fsync_ret);


            ret = upload_file(userdata,path);
            DLOG("IN watdfs_cli_fsync: upload_file return value '%d'",ret);


            //vec[check_client_file_exist].second.Tc = time(NULL);

            struct stat statbuf;
            stat(cache_full_path,&statbuf);

            struct timespec ts[2];

            struct timespec ts1;
            ts1.tv_nsec = UTIME_OMIT;

            struct timespec ts2;
            ts2 = statbuf.st_mtim;

            ts[0] = ts1;
            ts[1] = ts2;

            ret = watdfs_cli_utimens_aux(userdata,path,ts);
            DLOG("IN watdfs_cli_fsync: rpc utimens return value '%d'",ret);

            return fsync_ret;
        }

    }
}



// CHANGE METADATA
int watdfs_cli_utimens(void *userdata, const char *path,
                       const struct timespec ts[2]) {
    

    DLOG("IN watdfs_cli_utimens: starting -------------------------------------------");

    int check_client_file_exist = check_file_exist(path);
    int check_client_file_open = check_if_file_open(path);
    const char *cache_full_path = get_full_path(path);
    int ret = 0;

    if(check_client_file_open == 0){
        DLOG("IN watdfs_cli_utimens: client file is open");

        int open_mode = vec[check_client_file_exist].second.open_mode;
        int client_fh = vec[check_client_file_exist].second.fh_client;


        if(open_mode == O_RDONLY){
            DLOG("IN watdfs_cli_utimens: client file open in O_RDONLY, hence cannot utimens, return -EMFILE");

            return -EMFILE;
        }
        else{
            
            DLOG("IN watdfs_cli_utimens: client file open in write, hence can utimens");
            int uti_ret = utimensat(client_fh, cache_full_path, ts, 0);
            DLOG("IN watdfs_cli_utimens: local utimensat return value '%d'",uti_ret);

            int check_fresh = file_freshness_check(userdata,path);
            DLOG("IN watdfs_cli_utimens: fresh check after write '%d'",check_fresh);

            vec[check_client_file_exist].second.Tc = time(NULL);


            if(check_fresh == 0){
                DLOG("IN watdfs_cli_utimens: client file still fresh, no need to flush back");
                return uti_ret;
            }
            else{
                DLOG("IN watdfs_cli_utimens: file not fresh, update modification time on server");

                ret = watdfs_cli_utimens_aux(userdata,path,ts);
                DLOG("IN watdfs_cli_utimens: rpc utimens return value '%d'",ret);

                return uti_ret;

            }

        }

    }
    else{
        DLOG("IN watdfs_cli_utimens: client file is not open");

        struct fuse_file_info fi;
        fi.flags = O_WRONLY;

        DLOG("IN watdfs_cli_utimens: try open file in write mode");
        int ret = watdfs_cli_open(userdata,path,&fi);
        DLOG("IN watdfs_cli_utimens: open return value '%d'",ret);
       
        if(ret == -EACCES || ret < 0){
            DLOG("IN watdfs_cli_utimens: cannot open file in writemode");
            return ret;
        }

        int uti_ret = utimensat(fi.fh, cache_full_path, ts, 0);
        DLOG("IN watdfs_cli_utimens: local utimensat return value '%d'",uti_ret);
        ret = watdfs_cli_release(userdata,path,&fi);

        return uti_ret;

    }

}
