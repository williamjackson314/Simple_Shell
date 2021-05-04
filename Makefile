make:
	gcc -Wall -g -o simpsh dieWithError.c ScannerAndParser.c main.c

clean:
	rm simpsh