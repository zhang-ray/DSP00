
rm -rf ../k03-build
mkdir ../k03-build && cd ../k03-build

cmake -DCMAKE_CXX_COMPILER=/home/administrator/Programs/clang+llvm-8.0.0-x86_64-linux-gnu-ubuntu-16.04/bin/clang++ ../k03

make -j8