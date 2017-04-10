generate:
	gcc -Wall -Wextra generate.c -o generate
	./generate test.txt 4 2000

sort:
	gcc -Wall -Wextra sort.c -o sort

clean: 
	rm generate sort test.txt

copy:
	cp tmp.txt test.txt

run:
	cp test.txt tmp.txt
	./sort test.txt 4 lib
