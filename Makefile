


update : 
	git pull origin master
	git submodule init
	git submodule update


all : 
	cd lithp && make all
	cd console && make all
	ln -s console/console.py .
	ln -s console/_console.so .
	ln -s console/ext/Monaco_Linux.ttf .


dist :
	./setup.py dist_xo

clean :
	cd lithp && make clean
	cd console && make clean
	rm -rf *~ *.pyc console.py _console.so Monaco_Linux.ttf dist
