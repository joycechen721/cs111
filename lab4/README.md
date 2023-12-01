# Hey! I'm Filing Here

In this lab, I successfully implemented a 1 MiB ext2 file system with 2 directories, 1 regular file, and 1 symbolic link. 

I first created a superblock that specified general information about the file system, such as the number of free blocks, the block size, # of blocks per group, etc. The superblock was written to the filesystem through ```write_superblock(fd)```.

I then built the block group descriptor table by specifying the locations of the block/inode bitmaps. To account for the 2 directories and 2 other inodes, I filled the block/inode bitmaps correspondingly with 0's and 1's to indicate which data blocks were being used and which were free.

To write the actual files and directories, I first created inodes structs for them and wrote these structs to their corresponding blocks in the inode table. These structs specified which blocks (the blockno macros) the actual data was located in.

## Building

```make```

## Running

Run the executable that will create cs111-base.img.
``````shell
./ext2-create 
``````
Dump the filesystem information to debug.
``````shell
dumpe2fs cs111 -base.img 
``````
Check that the filesystem is correct.
``````shell
fsck.ext2 cs111 -base.img
``````
Create a directory "mnt" to mount the filesystem to.
``````shell
mkdir mntx
``````
Mount the filesystem.
``````shell
sudo mount -o loop cs111 -base.img mnt
``````

## Cleaning up

```make clean```

Unmount the filesystem.
``````shell
sudo umount mnt
``````
Delete the directory used for mounting.
``````shell
rmdir mnt
``````
