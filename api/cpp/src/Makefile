lib_dir=../lib/
socket_obj_dir=../../../objs/

libgstoreconnector.a: GstoreConnector.o $(socket_obj_dir)Socket.o
	ar -crv $(lib_dir)libgstoreconnector.a GstoreConnector.o $(socket_obj_dir)Socket.o

GstoreConnector.o: GstoreConnector.cpp GstoreConnector.h $(socket_obj_dir)Socket.o
	g++ -c GstoreConnector.cpp -o GstoreConnector.o

clean:
	rm -rf GstoreConnector.o $(lib_dir)libgstoreconnector.a
