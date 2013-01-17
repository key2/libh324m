all: asn1.cxx AVNDecoder.cxx AVNEncoder.cxx BERDecoder.cxx BEREncoder.cxx CoderEnv.cxx InvalidTracer.cxx PERDecoder.cxx PEREncoder.cxx ValidChecker.cxx multimedia_system_control.cxx 
	g++ -g -ggdb -c -static -DHAVE_CONFIG_H  -DNDEBUG -I. *.cxx
	cp *.o ../output/

clean:
	rm -rf *.o *.a *.mach

