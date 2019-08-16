FLAG=-c -std=c++11
CC=mpicxx 

output: vectorc.o Data.o GList.o RList.o MuC.o RTree.o MuC_RTree.o partition.o clustering.o main.o
	$(CC) -o output vectorc.o Data.o GList.o RList.o MuC.o RTree.o MuC_RTree.o partition.o clustering.o main.o -lm

main.o: main.cpp
	$(CC) $(FLAG) main.cpp -lm

vectorc.o: vectorc.cpp
	$(CC) $(FLAG) vectorc.cpp -lm

Data.o: Data.cpp
	$(CC) $(FLAG) Data.cpp -lm

MuC_RTree.o: MuC_RTree.cpp
	$(CC) $(FLAG) MuC_RTree.cpp  -lm

GList.o: GList.cpp
	$(CC) $(FLAG) GList.cpp  -lm 

RList.o: RList.cpp
	$(CC) $(FLAG) RList.cpp  -lm 

MuC.o: MuC.cpp
	$(CC) $(FLAG) MuC.cpp  -lm
	 
RTree.o: RTree.cpp
	$(CC) $(FLAG) RTree.cpp  -lm

partition.o: partition.cpp
	$(CC) $(FLAG) partition.cpp  -lm

clustering.o: clustering.cpp
	$(CC) $(FLAG) clustering.cpp  -lm

clean:
	rm *.o
	rm -f test*