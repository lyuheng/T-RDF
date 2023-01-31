mkdir -p lib objs
rm -rf tools/libantlr3c-3.4/
cd tools; tar -zxvf libantlr3c-3.4.tar.gz
cd libantlr3c-3.4/;  ./configure --enable-64bit; make; cd ../../
rm -rf lib/libantlr.a
ar -crv lib/libantlr.a tools/libantlr3c-3.4/*.o 
rm -rf Parser/Sparql*
cd tools; tar -xzvf sparql.tar.gz; mv Sparql* ../Parser/
