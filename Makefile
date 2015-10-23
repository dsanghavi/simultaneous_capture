# simultaneous_capture makefile

CC = g++
OUTPUTNAME = simultaneous_capture${D}
INCLUDE = -I./include/h
LIBS = -L/usr/src/flycapture/lib -lflycapture${D} -lboost_thread -lboost_system

OUTDIR = .

OBJS = simultaneous_capture.o

${OUTPUTNAME}: ${OBJS}
	${CC} -o ${OUTPUTNAME} ${OBJS} ${LIBS} ${COMMON_LIBS} 

%.o: %.cpp
	${CC} ${CFLAGS} ${INCLUDE} -Wall -c $*.cpp
	
clean_obj:
	rm -f ${OBJS}	@echo "all cleaned up!"

clean:
	rm -f ${OUTDIR}/${OUTPUTNAME} ${OBJS}	@echo "all cleaned up!"
