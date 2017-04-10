
static:
	cd ../zad1 && $(MAKE) static
	gcc -Wall -Wextra test.c -o test -L../zad1 ../zad1/libList.a
shared:
	cd ../zad1 && $(MAKE) shared
	gcc -c -Wall -Wextra -I ../zad1 test.c
	gcc test.o -Wall -Wextra -L../zad1 -lList -o test

dynamic:
	cd ../zad1 && $(MAKE) shared
	gcc -Wall -Wextra test_dynamic.c -I ../zad1 -o test_dynamic -ldl

run_static:
	./test

run_shared:
	export LD_LIBRARY_PATH=$(CURRENT_PATH):$(LIBRARY_PATH):../zad1; ./test

run_dynamic:
	export LD_LIBRARY_PATH=$(CURRENT_PATH):$(LIBRARY_PATH):../zad1; ./test_dynamic

clean:
	rm *.o test test_dynamic
