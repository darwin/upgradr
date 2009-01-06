all:
	echo "make (unix|win)"

clean:
	rm -f a.out*

../src/libshttpd.a:
	cd ../src && $(MAKE) unix

unix: ../src/libshttpd.a
	$(CC) example.c -I ../src  ../src/libshttpd.a

win:
	cl example.c /I..\src ..\src\shttpd.lib
