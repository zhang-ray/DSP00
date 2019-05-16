#pragma once



#include <kfr/base.hpp>
#include <kfr/dft.hpp>
#include <kfr/dsp.hpp>
#include <kfr/io.hpp>
#include <kfr/math.hpp>
#include <string>
#include <vector>


template<size_t SIZE=400>
class Feature3{
    class FftContext{
    private:
        const kfr::dft_plan<kfr::fbase> dft_;
        kfr::univector<kfr::u8> temp_;
        const kfr::internal::expression_hamming<kfr::fbase> hamming_;

    public:
        // perform forward fft
        FftContext(): dft_(SIZE), temp_(SIZE), hamming_(SIZE)
        { }

        void doWindowAndFftAndGetAbs(const kfr::univector<kfr::fbase, SIZE> &in, kfr::univector<kfr::fbase, SIZE/2> &out){
            kfr::univector<kfr::complex<kfr::fbase>, SIZE> inComplex = in * hamming_;
            kfr::univector<kfr::complex<kfr::fbase>, SIZE> outComplex;
            // perform forward fft
            dft_.execute(outComplex, inComplex, temp_); // try realdft function?
            // scale output

            kfr::univector<kfr::fbase, SIZE> hehe = cabs(outComplex);
            std::copy(hehe.begin(), hehe.begin() + SIZE/2, out.begin());
            out = log(hehe / SIZE + 1) ;
        }
    };

private:
    FftContext fftContext_;
public:
    Feature3()
        :fftContext_()
    { }

    std::string doMain(const std::vector<int16_t> &mono_16k_s16_pcm){
        std::string result = kfr::library_version() ;
        result = result + "\n";


        kfr::univector<kfr::fbase, SIZE> in;
        kfr::univector<kfr::fbase, SIZE/2> out;

        for (int epoch = 0; epoch*160 + 400 < mono_16k_s16_pcm.size() ; epoch++){
            for (int index = 0; index < SIZE; index++){
                in[index]=(kfr::fbase)mono_16k_s16_pcm[epoch*160]/(1<<15);
            }
            fftContext_.doWindowAndFftAndGetAbs(in, out);
        }

        return result;
    }
};



