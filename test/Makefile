# use normal make for this Makefile
#
# Makefile for building user programs to run on top of Nachos
#
# Several things to be aware of:
#
#    Nachos assumes that the location of the program startup routine (the
#    location the kernel jumps to when the program initially starts up)
#    is at location 0.  This means: start.o must be the first .o passed
#    to ld, in order for the routine "Start" to be loaded at location 0
#

# If you are running on SunOS 5.x machines, uncomment the next line
GCCDIR = ../gnu/
LDFLAGS = -T newscript -N
ASFLAGS = -mips1
CPPFLAGS = $(INCDIR)

# if you aren't cross-compiling:
# GCCDIR =
# LDFLAGS = -N -T 0
# ASFLAGS =
# CPPFLAGS = -P $(INCDIR)

CC = $(GCCDIR)gcc
AS = $(GCCDIR)as
LD = $(GCCDIR)ld
CPP = $(GCCDIR)cpp

INCDIR =-I../userprog -I../threads
CFLAGS = -G 0 -c -w $(INCDIR)

all: halt matmult sort networkTests newpassportoffice customer applicationclerk pictureclerk passportclerk cashier manager clerkstarter customerstarter

start.o: start.s ../userprog/syscall.h
	$(CPP) $(CPPFLAGS) start.s > strt.s
	$(AS) $(ASFLAGS) -o start.o strt.s
	rm strt.s

halt.o: halt.c
	$(CC) $(CFLAGS) -c halt.c
halt: halt.o start.o
	$(LD) $(LDFLAGS) start.o halt.o -o halt.coff
	../bin/coff2noff halt.coff halt

sort.o: sort.c
	$(CC) $(CFLAGS) -c sort.c
sort: sort.o start.o
	$(LD) $(LDFLAGS) start.o sort.o -o sort.coff
	../bin/coff2noff sort.coff sort

matmult.o: matmult.c
	$(CC) $(CFLAGS) -c matmult.c
matmult: matmult.o start.o
	$(LD) $(LDFLAGS) start.o matmult.o -o matmult.coff
	../bin/coff2noff matmult.coff matmult

networkTests.o: networkTests.c
	$(CC) $(CFLAGS) -c networkTests.c
networkTests: networkTests.o start.o
	$(LD) $(LDFLAGS) start.o networkTests.o -o networkTests.coff
	../bin/coff2noff networkTests.coff networkTests

newpassportoffice.o: newpassportoffice.c
	$(CC) $(CFLAGS) -c newpassportoffice.c
newpassportoffice: newpassportoffice.o start.o
	$(LD) $(LDFLAGS) start.o newpassportoffice.o -o newpassportoffice.coff
	../bin/coff2noff newpassportoffice.coff newpassportoffice

customer.o: customer.c
	$(CC) $(CFLAGS) -c customer.c
customer: customer.o start.o
	$(LD) $(LDFLAGS) start.o customer.o -o customer.coff
	../bin/coff2noff customer.coff customer

applicationclerk.o: applicationclerk.c
	$(CC) $(CFLAGS) -c applicationclerk.c
applicationclerk: applicationclerk.o start.o
	$(LD) $(LDFLAGS) start.o applicationclerk.o -o applicationclerk.coff
	../bin/coff2noff applicationclerk.coff applicationclerk

pictureclerk.o: pictureclerk.c
	$(CC) $(CFLAGS) -c pictureclerk.c
pictureclerk: pictureclerk.o start.o
	$(LD) $(LDFLAGS) start.o pictureclerk.o -o pictureclerk.coff
	../bin/coff2noff pictureclerk.coff pictureclerk

passportclerk.o: passportclerk.c
	$(CC) $(CFLAGS) -c passportclerk.c
passportclerk: passportclerk.o start.o
	$(LD) $(LDFLAGS) start.o passportclerk.o -o passportclerk.coff
	../bin/coff2noff passportclerk.coff passportclerk

cashier.o: cashier.c
	$(CC) $(CFLAGS) -c cashier.c
cashier: cashier.o start.o
	$(LD) $(LDFLAGS) start.o cashier.o -o cashier.coff
	../bin/coff2noff cashier.coff cashier

manager.o: manager.c
	$(CC) $(CFLAGS) -c manager.c
manager: manager.o start.o
	$(LD) $(LDFLAGS) start.o manager.o -o manager.coff
	../bin/coff2noff manager.coff manager

clerkstarter.o: clerkstarter.c
	$(CC) $(CFLAGS) -c clerkstarter.c
clerkstarter: clerkstarter.o start.o
	$(LD) $(LDFLAGS) start.o clerkstarter.o -o clerkstarter.coff
	../bin/coff2noff clerkstarter.coff clerkstarter

customerstarter.o: customerstarter.c
	$(CC) $(CFLAGS) -c customerstarter.c
customerstarter: customerstarter.o start.o
	$(LD) $(LDFLAGS) start.o customerstarter.o -o customerstarter.coff
	../bin/coff2noff customerstarter.coff customerstarter

clean:
	rm -f *.o *.coff
	rm -f halt matmult sort passportOffice networkTests customer applicationclerk newpassportoffice pictureclerk passportclerk cashier manager clerkstarter customerstarter
