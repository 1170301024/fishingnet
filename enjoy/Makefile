
INCLDDIR = ../include
CFLAGS = -I $(INCLDDIR)/*.h

LADIR = ../lib/joy
OBJS =  user.o connect_manage.o \
		distribute.o error.o wrappthread.o flow.o feature_extract.o nflog.o 

fishing_net: fishing_net.lo $(OBJS) -lpthread -lpcap
	libtool --mode=link gcc -o fishing_net $(OBJS) -lpthread -lpcap \
			fishing_net.lo -lm $(LADIR)/libjoy.la  -lm \
			../lib/safe_c_stub/libjoy_la-safe_mem_stub.o \
			../lib/safe_c_stub/libjoy_la-safe_str_stub.o

fishing_net.lo: $(INCLDDIR)/*.h
	libtool --mode=compile gcc -c fishing_net.c

user.o: $(INCLDDIR)/*.h
	cc -c $(CFLAGS) user.c

connect_manage.o: $(INCLDDIR)/*.h
	cc -c $(CFLAGS) connect_manage.c

distribute.o: $(INCLDDIR)/*.h
	cc -c $(CFLAGS) distribute.c

error.o: $(INCLDDIR)/*.h
	cc -c $(CFLAGS) error.c

wrappthread.o: $(INCLDDIR)/*.h
	cc -c $(CFLAGS) wrappthread.c

flow.o: $(INCLDDIR)/*.h
	cc -c $(CFLAGS) flow.c

feature_extract.o:
	cc -c $(CFLAGS) feature_extract.c

nflog.o:
	cc -c $(CFLAGS) nflog.c
.PHONY: clean
clean: 
	rm *.o ../include/*.gch joy-output*
	rm *.lo
	rm -r .libs/