
all: sc scd attotest

scd: main.c sc_vfunc.h sc_vfunc.c sc_rng.c sc_rng.h
	gcc -Wall -g sc_vfunc.c sc_rng.c main.c -lm -o scd	

sc: main.c sc_vfunc.h sc_vfunc.c sc_rng.c sc_rng.h
	gcc -Wall -DNDEBUG -O5 sc_vfunc.c sc_rng.c main.c -lm -o sc	

attotest: attotest.c atto.c atto.h variant.h
	gcc -Wall -lm attotest.c atto.c -o attotest
	
backup:
	echo BACKUP to - ~/zz/vfuncs-0.7.0.`date +%Y%m%d_%H%M%S`.tgz ; 
	tar -zcvf ~/zz/vfuncs-0.7.0.`date +%Y%m%d_%H%M%S`.tgz ../vfuncs-0.7.0; 

clean: 
	rm -f sc scd sc_var attotest *.o *.so
	


