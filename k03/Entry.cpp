
#include <iostream>
#include <functional>
#include <thread>
#include <vector>
#include <fstream>
#include <mutex>
#include <condition_variable>

#include <kfr/base.hpp>
#include <kfr/dft.hpp>
#include <kfr/dsp.hpp>


#include <Eigen/Dense>

#include <portaudio.h>



namespace {
std::mutex m;
std::condition_variable cv;
bool shouldStop_ = false;

std::array<int, 4> channelMap_ordinary = {0,2,1,3};
std::array<int, 4> channelMap_PS_eye={0,2,1,3};

}



template<size_t SAMPLE_RATE, size_t NB_SAMPLES_PER_CHANNEL>
class MicArrayRunner {
private:
    PaStream* paStream_ = nullptr;
    PaStreamParameters  paInputParameters_;
    std::function<void(const int16_t * inputBuffer, const size_t framesPerBuffer, const size_t nbChannel)> callback_ = nullptr;
    int finalNbChannel_ = -1;

public:
    MicArrayRunner(decltype(callback_) callback)
        :callback_(callback)
    { }

    bool micArrayFilter(const PaDeviceInfo* deviceInfo) {
        return (deviceInfo->maxInputChannels < 6 && deviceInfo->maxInputChannels > 2);
    }

    void init() {
        auto err = Pa_Initialize();
        if( err != paNoError ) {
            throw;
        }

        auto numDevices = Pa_GetDeviceCount();
        auto finalIndex = -1;
        for(auto i=0; i<numDevices; i++ ){
            auto deviceInfo = Pa_GetDeviceInfo( i );

            if(micArrayFilter(deviceInfo)){
                printf("deviceID=%02d: %s\n \t\t nbChannel=%02d\n", i, deviceInfo->name, deviceInfo->maxInputChannels);
                finalIndex = i;
                finalNbChannel_ = deviceInfo->maxInputChannels;
            }
        }

        paInputParameters_.device = finalIndex;
        paInputParameters_.channelCount = finalNbChannel_;
        paInputParameters_.sampleFormat = paInt16;
        paInputParameters_.suggestedLatency = Pa_GetDeviceInfo( paInputParameters_.device )->defaultLowInputLatency;
        paInputParameters_.hostApiSpecificStreamInfo = NULL;
        err = Pa_OpenStream(&paStream_, &paInputParameters_, nullptr, SAMPLE_RATE, NB_SAMPLES_PER_CHANNEL, paClipOff, MicArrayRunner::theCallback, this);
    }

    void start() {
        Pa_StartStream(paStream_);
    }

    void stop() {
        Pa_StopStream(paStream_);
    }

    void deInit() {
        stop();
        Pa_Terminate();
    }

    ~MicArrayRunner() {
        deInit();
    }


private:
    static int theCallback(const void *inputBuffer, void *outputBuffer,
                           unsigned long framesPerBuffer,
                           const PaStreamCallbackTimeInfo* timeInfo,
                           PaStreamCallbackFlags statusFlags,
                           void *userData ) {
        auto _runner = (MicArrayRunner*)userData;
        _runner->callback_((const int16_t*)inputBuffer, framesPerBuffer, _runner->finalNbChannel_);
        return 0;
    }
};




template<size_t SAMPLE_RATE, size_t NB_CHANNEL, size_t NB_SAMPLES_PER_CHANNEL, const size_t CORR_RESULT_SIZE = 2 * NB_SAMPLES_PER_CHANNEL - 1>
class MultichannelCrossCorrelationCoefficientAlgorithm {
private:
    kfr::univector<kfr::fbase, NB_CHANNEL*NB_SAMPLES_PER_CHANNEL> kfrFbase_;
    kfr::univector<kfr::univector<kfr::fbase, NB_SAMPLES_PER_CHANNEL>, NB_CHANNEL> splitedAudio_;
    kfr::univector<kfr::univector<kfr::univector<kfr::fbase, CORR_RESULT_SIZE>,NB_CHANNEL>, NB_CHANNEL> raP_;
    kfr::univector<Eigen::Matrix<double, NB_CHANNEL, NB_CHANNEL>, CORR_RESULT_SIZE> eigenMatrix_;
    kfr::univector<kfr::fbase, CORR_RESULT_SIZE> detRaP_;

    std::array<int, NB_CHANNEL> channelMap_;
private:
    void calcCorr(const kfr::univector<kfr::fbase, NB_SAMPLES_PER_CHANNEL> &a,
                  const kfr::univector<kfr::fbase, NB_SAMPLES_PER_CHANNEL> &b,
                  kfr::univector<kfr::fbase, CORR_RESULT_SIZE> &result
                  ){
        result = correlate(a,b);
    }


    void constructMatrix(const size_t p,
                         const kfr::univector<kfr::univector<kfr::univector<kfr::fbase, CORR_RESULT_SIZE>, NB_CHANNEL> ,NB_CHANNEL> &RaP,
                         Eigen::Matrix<double, NB_CHANNEL, NB_CHANNEL> &outputMatrix
                         ){
        for (int row = 0; row < NB_CHANNEL; row++){
            for (int col = 0; col < NB_CHANNEL; col++){
                outputMatrix(row,col) = RaP[row][col][p];
            }
        }
    }

    void paraCalcCorr(){
        
    }

    void captureAudio(kfr::univector<kfr::fbase, NB_CHANNEL*NB_SAMPLES_PER_CHANNEL> &samples){
        // fill samples
    }

    void int16ToKfrFbase(const int16_t * inputBuffer,
                         kfr::univector<kfr::fbase, NB_CHANNEL*NB_SAMPLES_PER_CHANNEL> &outputKfrFbase ){
        for (int i = 0; i < NB_CHANNEL*NB_SAMPLES_PER_CHANNEL; i++){
            outputKfrFbase[i]=(kfr::fbase)inputBuffer[i]/(1<<15);
        }

    }

    void splitChannels(
            const kfr::univector<kfr::fbase, NB_CHANNEL*NB_SAMPLES_PER_CHANNEL> &samples,
            kfr::univector<kfr::univector<kfr::fbase, NB_SAMPLES_PER_CHANNEL>, NB_CHANNEL> &splitedAudio
            ) {
        for(int channel = 0; channel < NB_CHANNEL; channel++){
            for (int sample = 0; sample < NB_SAMPLES_PER_CHANNEL; sample++){
                splitedAudio[channel][sample] = samples[NB_CHANNEL*sample + channelMap_[channel]];
            }
        }
    }

public:
    MultichannelCrossCorrelationCoefficientAlgorithm(const decltype(channelMap_) channelMap)
        :channelMap_(channelMap)
    { }

    void entry(){
        MicArrayRunner<SAMPLE_RATE, NB_SAMPLES_PER_CHANNEL> runner(
                    [this](const int16_t * inputBuffer, const size_t framesPerBuffer, const size_t nbChannel)
        {
            int16ToKfrFbase(inputBuffer, kfrFbase_);
            splitChannels(kfrFbase_, splitedAudio_);
            for (auto row = 0; row < NB_CHANNEL; row++){
                for (auto col = 0; col < NB_CHANNEL; col++){
                    calcCorr(splitedAudio_[row], splitedAudio_[col], raP_[row][col]);
                }
            }

            for(int p = 0; p<CORR_RESULT_SIZE; p++){
                constructMatrix(p, raP_, eigenMatrix_[p]);
                detRaP_[p]= eigenMatrix_[p].determinant();
            }

            {
                int argMinP = 0;
                double minDetRaP = detRaP_[argMinP];
                for(int p = 1; p<CORR_RESULT_SIZE; p++){
                    if (detRaP_[p] < minDetRaP){
                        argMinP=p;
                        minDetRaP=detRaP_[p];
                    }
                }
                auto tDOA = (double)((int)argMinP - (int)NB_SAMPLES_PER_CHANNEL)/(int)SAMPLE_RATE;

                {
                    printf("tDOA=%f \t  argMinP=%d \t minDetRaP=%f\n", tDOA, argMinP, minDetRaP);
                }
            }
        });

        runner.init();
        runner.start();

        cv.notify_one();

        // wait for the worker
        {
            std::unique_lock<std::mutex> lk(m);
            cv.wait(lk, []{return shouldStop_;});
        }

        runner.stop();
    }
};



auto test00(){
    kfr::univector<kfr::fbase, 4> a({ 1, 2, 3, 4 });
    kfr::univector<kfr::fbase, 4> b({ 1, 2, 3, 4 });
    auto c = correlate(a, b);
    kfr::println(c);
    //CHECK(c.size() == 9u);
    //CHECK(rms(c - kfr::univector<kfr::fbase>({ 1.5, 1., 1.5, 2.5, 3.75, -4., 7.75, 3.5, 1.25 })) < 0.0001);
}




int main(void){
    MultichannelCrossCorrelationCoefficientAlgorithm<16000, 4, 1<<8> handler(channelMap_PS_eye);
    handler.entry();

    return 0;
}
