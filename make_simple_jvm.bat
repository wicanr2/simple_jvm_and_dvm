@echo off
echo "Make Simpe Java VM"
PATH=%PATH%;.\mingw\bin
echo "Make simple_jvm"
mingw32-gcc -DSIMPLE_JVM_DEBUG -g simple_jvm\simple_jvm*.c -o simple_jvm_debug 
mingw32-gcc -g simple_jvm\simple_jvm*.c -o simple_jvm 
echo "Make simple_jvm successful"
