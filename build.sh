#! /bin/sh

mkdir -p build
c++ $(cat compile_flags.txt) -c -o build/main.o src/main.cxx &
c++ $(cat compile_flags.txt) -c -o build/tokenizer.o src/tokenizer.cxx &
wait
c++ $(cat compile_flags.txt) -o build/main build/*.o
