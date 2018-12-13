all: rtk_hciattach

rtk_hciattach: hciattach.c hciattach_rtk.o
	$(CC) -o rtk_hciattach hciattach.c hciattach_rtk.o

hciconfig: hciconfig.o
	$(CC) -o hciconfig hciconfig.o

hciattach_rtk.o:hciattach_rtk.c
	$(CC) -c hciattach_rtk.c

hciconfig.o: hciconfig.c
	$(CC) -c hciconfig.c

clean:
	rm -f *.o  rtk_hciattach tags cscope.*

tags: FORCE
	ctags -R
	find ./ -name "*.h" -o -name "*.c" -o -name "*.cc" -o -name "*.cpp" > cscope.files
	cscope -bkq -i cscope.files
PHONY += FORCE
FORCE:
