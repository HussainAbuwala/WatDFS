import os,sys
import time
import stat


     
# Get the current working
# directory (CWD)
     
# Print the current working
# directory (CWD)



'''print(os.O_RDWR)
print(os.O_CREAT)
mode = os.O_RDWR | os.O_CREAT
print(mode)
print(mode & os.O_CREAT)'''

cache_path_1 = "/tmp/hsabuwal/cache"
mount_path_1 = "/tmp/hsabuwal/mount"

cache_path_2 = "/tmp/hsabuwal/cache2"
mount_path_2 = "/tmp/hsabuwal/mount2"

server_path = "/tmp/hsabuwal/server"
file_name = "/tr.txt"

#x = os.O_RDWR | os.O_CREAT |os.O_EXCL
#print(x & os.O_EXCL)
#print(os.O_EXCL)

fd = os.open(mount_path_1 + file_name, os.O_RDWR | os.O_CREAT)

'''s = "Hussain great and the best"
line = str.encode(s)

time.sleep(10)

numBytes = os.write(fd, line)
print(numBytes)

readBytes = os.read(fd, 10)
  
# Print the bytes read
print(readBytes)'''

os.close(fd)

isFile = os.path.isfile(server_path + file_name)
print(isFile)

isFile = os.path.isfile(cache_path_1 + file_name)
print(isFile)

#print(os.path.getsize(cache_path_1 + file_name))
#print(os.path.getsize(server_path + file_name))


'''  
# Path
#path = "filex.txt"
 
# Permission to use
per = 0o600
 
# type of node to be created
node_type = stat.S_IRUSR
mode = per | node_type
 
# Create a file system node
# with specified permission
# and type using
# os.mknod() method
os.mknod(server_path + file_name, mode)'''

'''
time.sleep(10)
file1 = os.open(mount_path_1 + file_name, os.O_RDWR | os.O_CREAT)

s = "Hussain great and the best"
  
# Convert the string to bytes
line = str.encode(s)
  
# Write the bytestring to the file
# associated with the file
# descriptor fd and get the number of
# Bytes actually written
numBytes = os.write(file1, line)
print(numBytes)
#file2 = os.open(mount_path_2 + file_name, os.O_RDONLY)

os.close(file1)

isFile = os.path.isfile(server_path + file_name)
print(isFile)

isFile = os.path.isfile(cache_path_1 + file_name)
print(isFile)


print(os.path.getsize(cache_path_1 + file_name))'''
#print(os.path.getsize(server_path + file_name))
#os.close(file2)

#file1 = os.open(mount_path_1 + file_name, os.O_RDONLY)
#time.sleep(10)

#status = os.stat(mount_path_1 + file_name)
  
  
# Print the status
# of the specified path
#print(status)




  
# Check whether the 
# specified path is 
# an existing file


#isFile = os.path.isfile(cache_path_2 + file_name)
#print(isFile)

#status = os.stat(mount_path + file_name)
#print(status)


