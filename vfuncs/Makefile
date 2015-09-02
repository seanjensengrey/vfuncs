
scd: main.c sc_vfunc.h sc_vfunc.c sc_rng.c sc_rng.h
	gcc -Wall -g sc_vfunc.c sc_rng.c main.c -lm -o scd	

sc: main.c sc_vfunc.h sc_vfunc.c sc_rng.c sc_rng.h
	gcc -Wall -DNDEBUG -O5 sc_vfunc.c sc_rng.c main.c -lm -o sc	
	
clean: 
	rm -f sc scd *.o *.so
	


