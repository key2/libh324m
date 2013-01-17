


cfiles: *.c
#	cd asn1; make
	gcc -g -ggdb -static -c *.c
	g++ -g -ggdb -c -static -I ./asn1/ *.cpp
	cp *.o ./output/
	cd output; make
clean:
	make -C asn1 clean
#	make -C output clean
	rm -rf *.o *.a

