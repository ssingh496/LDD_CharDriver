# LDD_CharDriver
Linux device character driver and user level code to test


This is a  simple implementation of a character device driver. A character buffer is used to hold data. You can read and write from this buffer. 
A userlevel test program is also written to test the charDriver.


Sample output of the userlevel :

root@sandeep:/home/sandeep/charDriver# sudo ./userLevelPrgm.o
Starting device test code example...
Type in a short string to send to the kernel module:
sandeep sending to drv
Writing message to the device [sandeep sending to drv].
Press ENTER to read back from the device...

Reading from the device...
The received message is: [sandeep sending to drv(22 letters)]
End of the program

Steps to load the driver:

1. sudo insmod <drv>.ko
2. Check if the driver is present at cd /dev
3. Run the user level program 
4. Driver should response to the buffer which you are sending from the user level










