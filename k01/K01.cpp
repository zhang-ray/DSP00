#include <iostream>
#include <memory>

#include "FrequencyFeature3.hpp"



int main(void){
    auto floatList = FrequencyFeature3<400, true>::readWavFileAndDoMain("/data/a.wav");

    printf("floatList.size()=%d\n", floatList.size());
    printf("floatList.size()/200=%d\n", floatList.size()/200);
    

    return 0;
}



