@echo off
echo "Make Simpe Dalvik VM"
echo "Make simple_dvm"
mingw32-gcc -DSIMPLE_DVM_DEBUG -g simple_dvm\simple_dvm*.c -o simple_dvm_debug 
mingw32-gcc -g simple_dvm\simple_dvm*.c -o simple_dvm 
echo "Make simple_dvm successful"
