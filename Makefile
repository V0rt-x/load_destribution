all: gen.o alg.o

gen.o:
	g++ gen.cpp -o gen

alg.o:
	g++ alg.cpp -o alg

clean:
	rm -rf *.o *.exe *.out

	
