all: gen.o alg.o

gen.o:
	g++ src/gen.cpp -o src/gen_MihailovDM_213

alg.o:
	g++ src/alg.cpp -o src/alg_MihailovDM_213

clean:
	rm -rf src/*.o src/*.exe src/*.out

	