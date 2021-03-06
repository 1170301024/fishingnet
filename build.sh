#! /bin/sh

# Joy supports a libtool library libjoy.la in joy/bin/.libs with joy API for users,
# additionally, joy program use some safe C memory and string library which in
# libjoy_la-safe_mem_stud.o and libjoy_la-safe_str_stud.o to replace standord C
# library functions 
# 
# We will use the libtool to manage and build our program
# Following are the usage of the libjoy.la, if you change your code, you should 
# do the following in order
#	1. use libtool to compile your source files
#		eg. libtool -mode=compile gcc -c main
#
#	2. use the command to link
#		eg. libtool --mode=link gcc -o main main.lo libjoy.la
#	   if you use the libpcap or libpthread or libm
#          you should add the link option like -lpcap, -lpthread, -lm 
#          
#          if you use the safe C library from joy, you should add the link
#         object libjoy_la-safe_mem_stud.o and	libjoy_la-safe_str_stud.o
#
# The following command shows a complete link command
link_command="(libtool --mode=link gcc -o joy_api_test -lpcap -lpthread joy_api_test.lo ../lib/joy/.libs/libjoy.la -lm ../lib/safe_c_stub/libjoy_la-safe_mem_stub.o ../lib/safe_c_stub/libjoy_la-safe_str_stub.o)"

# the script is not complete, following commands are used to test
eval $link_command
