CFLAGS = -g -Wall
objects = ipmsg.o userlist.o utils.o udp_comm.o msglist.o encode.o staff.o

ipmsg: $(objects)
	gcc $(CFLAGS) -o ipmsg $(objects) -lpthread

ipmsg.o: ipmsg.c msglist.h userlist.h staff.h ipmsg.h  udp_comm.h
	gcc $(CFLAGS) -c $^ 

userlist.o: userlist.c userlist.h  utils.h
	gcc $(CFLAGS) -c $^

utils.o: utils.c utils.h
	gcc $(CFLAGS) -c $^

udp_comm.o: udp_comm.c udp_comm.h
	gcc $(CFLAGS) -c $^

msglist.o: msglist.c ipmsg.h msglist.h encode.h
	gcc $(CFLAGS) -c $^

encode.o: encode.c encode.h
	gcc $(CFLAGS) -c $^

staff.o:staff.c staff.h msglist.h utils.h

.PHONY: clean
clean:
	rm -f ipmsg $(objects) *.gch
cgch:
	rm -f *.gch
