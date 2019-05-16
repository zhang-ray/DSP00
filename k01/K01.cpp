#include <iostream>


#include "Feature3.hpp"




int main(void){

    Feature3<> feature3;

    std::vector<int16_t> myPCM(16000*2.28, 1<<14);

    auto t0 = std::chrono::system_clock::now();
    auto stringResult = feature3.doMain(myPCM);
    auto t1 = std::chrono::system_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0);

    stringResult = stringResult+ "\n"+"use: " + std::to_string(
            double(duration.count()) *  std::chrono::microseconds::period::num /  std::chrono::microseconds::period::den
    ) + "s";



    printf("%s\n",stringResult.c_str());
}



