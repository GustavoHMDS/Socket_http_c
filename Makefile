all:
	make -C Servidor
	make -C Cliente

clean:
	make -C Servidor clean
	make -C Cliente clean
