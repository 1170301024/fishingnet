
INCLDDIR = ../include
CFLAGS = -I $(INCLDDIR)/*.h

OBJS = fishing_net.o user.o connect_manage.o \
		distribute.o error.o wrappthread.o flow.o

fishing_net: $(OBJS) -lpthread
	cc $(OBJS) -lpthread -o fishing_net
fishing_net.o: $(INCLDDIR)/*.h
	cc -c $(CFLAGS) fishing_net.c

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

.PHONY: clean
clean: 
	rm *.o ../include/*.gch