FLAGS=-O3 -fopenmp
EXEC=knn
CC=gcc

ALL: $(EXEC)

$(EXEC):
	$(CC) $(FLAGS) $(EXEC).c -c -o $(EXEC).o
	$(CC) $(FLAGS) $(EXEC).o  -o $(EXEC)

clean:
	rm -rf $(EXEC) *.o
