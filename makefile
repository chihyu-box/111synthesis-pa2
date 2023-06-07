TARGET	= main
CC	= g++ -std=c++17
ABC	= /home/yu-vm/Workshop/cpp_workspace/abc_pa2

SRCS	= $(wildcard *.cpp)
OBJS	= ${SRCS:.cpp=.o}
LIB = ${ABC}/libabc.a -lm -ldl -lrt -rdynamic -lreadline -ltermcap -lpthread 
INCLUDE = -I. -I${ABC}/src
CFLAGW  = -Wall -O3
CFLAGS  = -O3
${TARGET}: ${OBJS}
	${CC} ${CFLAGS} ${OBJS} ${LIB} ${LIBS} -o ${TARGET}

.SUFFIXES: .cpp .o

.cpp.o:
	${CC} -DLIN64 ${CFLAGW} ${INCLUDE} -o $@ -c $<

clean:
	rm -f core *~ $(TARGET); \
	rm *.o
