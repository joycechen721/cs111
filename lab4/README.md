# Hey! I'm Filing Here

In this lab, I successfully implemented the following TODO

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
