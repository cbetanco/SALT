CC=g++
CFLAGS= -Wall  -std=c++11 -Dsoc_cv_av -g -static-libstdc++ -Wunused-variable
INCLUDES= -I/usr/lib/hwlib/hwlib/include -I/usr/lib/hwlib/hwlib/include/soc_cv_av -I/usr/lib/hwlib/hwlib/include/soc_cv_av/socal/
SRCS= main.cpp I2C.cpp Fpga.cpp CurrentMonitor.cpp ExternalADC.cpp Salt.cpp fastComm.C Dig_Clk_test.C Ana_tests.C DacCalib.cpp
OBJS=$(SRCS:.cpp=.o)
MAIN=main

.PHONY= depend clean
all: $(MAIN)
	@echo  Congratulation! Compilation succeeded! 
$(MAIN): $(OBJS)
	$(CC) $(CFLAGS) $(INCLUDES) -o $(MAIN) $(OBJS)

.cpp.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c  $< -o $@
clean:
	rm *.o

