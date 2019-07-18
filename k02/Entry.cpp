#include <iostream>
#include <cinttypes>
#include <vector>
#include <string.h>
#include <cmath>
#include <numeric>
#include <cassert>
#include <chrono>
#include <random>

namespace UniformLinearMicArray{


constexpr double c(){return 340;} //340m/s


/***
 * for estimating tDOA:
 *     assume  d/c >> fs
 *     assume  oneSegmentSize >> maxOffset
 */

class BeamFormerDelayAndSum {
private:
    const int nbMic_;
    const double d_;
    const int sampleRate_;
    const bool autoEstimateDoa_;

    int maxDoaOffset_;

    std::vector<int16_t> largestRawBuffer_;


private:
    void appendData(const std::vector<int16_t> &incomeData){
        auto oldSize = largestRawBuffer_.size();
        auto appendSize = incomeData.size();
        largestRawBuffer_.resize(oldSize+appendSize);
        memcpy(&(largestRawBuffer_[oldSize]), incomeData.data(), appendSize * sizeof(int16_t));
    }


    void estimateDoa(const std::vector<int16_t> &incomeData, int &estimatedOffset){
        auto segmentSize = incomeData.size()/nbMic_;

        auto finalOffset = -2*maxDoaOffset_;
        auto finalSum = 0;


        for (int currentOffset = -maxDoaOffset_; currentOffset <=maxDoaOffset_; currentOffset++ ){
            int  localIndex = -1;
            int  localSum = 0;

            int indexStart = currentOffset < 0 ? (nbMic_-1) * (-currentOffset) : 0;
            int indexEnd = currentOffset < 0 ? segmentSize: segmentSize - (nbMic_-1) * currentOffset ;
            for (int currentIndexOfCh0 = indexStart;
                 currentIndexOfCh0 < indexEnd;
                 currentIndexOfCh0++){
                int currentSum = 0;
                for (int n = 0; n < nbMic_; n++){
                    auto theIndex = (currentIndexOfCh0 + currentOffset*n)*nbMic_ + n;
                    assert(theIndex >= 0);
                    assert(theIndex < incomeData.size());
                    currentSum += std::abs(incomeData[theIndex]);
                }
                if (currentSum > localSum){
                    localIndex = currentIndexOfCh0;
                    localSum = currentSum;
                }

                assert(localSum <= (nbMic_ << 15));
            }

            if (localSum > finalSum){
                finalOffset = currentOffset;
                finalSum = localSum;
            }
        }

        estimatedOffset = finalOffset;
    }

public:
    BeamFormerDelayAndSum(const int16_t nbMic, const double d, const int sampleRate, const bool autoEstimateDoa)
        : nbMic_(nbMic)
        , d_(d)
        , sampleRate_(sampleRate)
        , autoEstimateDoa_(autoEstimateDoa)
        , maxDoaOffset_(std::floor(sampleRate_ * d_ / c()))
    {    }

    void beamForm(const std::vector<int16_t> &incomeData, double &estimateDOA, std::vector<int16_t> &outputMonoData){
        appendData(incomeData);
        if (autoEstimateDoa_){
            int result;
            estimateDoa(incomeData, result);

            {
                auto tDOA = 1.0/sampleRate_*result;
                auto length = tDOA * c();
                //// cos(theta) = length / d
                auto radius = acos(length/d_);
                estimateDOA = radius * 180/M_PI;
            }
        }
    }
};
}



void tester(){
    UniformLinearMicArray::BeamFormerDelayAndSum handler(4, 0.043, 48000, true);

    std::vector<int16_t> incomeData(1<<12, 0);
    std::vector<int16_t> outputMonoData(1<<10, 0);



    std::random_device rd;  //Will be used to obtain a seed for the random number engine
    std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
    std::uniform_int_distribution<int16_t> dis(-1<<10, 1<<10);


    for (auto &v : incomeData){
        v=dis(gen);
    }

    for (int i = 0; i< 1; i++){
        double doa=0.0;
        handler.beamForm(incomeData, doa, outputMonoData);
        printf("DOA = %f°\n", doa);
    }
}



int main(void){
    tester();

    return 0;
}
