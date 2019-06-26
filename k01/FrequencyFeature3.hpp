#pragma once



#include <kfr/base.hpp>
#include <kfr/dft.hpp>
#include <kfr/dsp.hpp>
#include <kfr/io.hpp>
#include <kfr/math.hpp>
#include <string>
#include <vector>
#include <fstream>

template<size_t SIZE=400, bool PRINT_DEBUG=false>
class FrequencyFeature3{
    class FftContext{
    private:
        const kfr::dft_plan<kfr::fbase> dft_;
        const kfr::internal::expression_hamming<kfr::fbase> hamming_;

    public:
        // perform forward fft
        FftContext(): dft_(SIZE), hamming_(SIZE)
        { }

        auto doWindowAndFftAndGetAbs(const kfr::univector<kfr::fbase, SIZE> &in, const size_t insaneWholeLength){

            kfr::univector<kfr::fbase, SIZE/2> out;

            kfr::univector<kfr::complex<kfr::fbase>, SIZE> inComplex = in * hamming_;
            kfr::univector<kfr::complex<kfr::fbase>, SIZE> outComplex;

            kfr::univector<kfr::u8> tempVector(dft_.temp_size);
            // perform forward fft
            dft_.execute(outComplex, inComplex, tempVector); // try realdft function?
            // scale output

            kfr::univector<kfr::fbase, SIZE> hehe = cabs(outComplex);
            std::copy(hehe.begin(), hehe.begin() + SIZE/2, out.begin());
            out = log(hehe / insaneWholeLength/*SIZE*/ + 1) ;

            return std::move(out);
        }
    };

private:
    FftContext fftContext_;
public:
    FrequencyFeature3()
        :fftContext_()
    {

    }

    auto doMain(const std::vector<int16_t> &mono_16k_s16_pcm){
        std::vector<float> result;
        if(PRINT_DEBUG){
            printf("kfr::library_version(): %s\n",kfr::library_version());
        }

        kfr::univector<kfr::fbase, SIZE> in;

        int insaneNbEpoch = (mono_16k_s16_pcm.size()/16000.0*1000 - 25 )/ 10 ;//


        std::shared_ptr<std::ofstream> dbg_ofs = nullptr;
        if(PRINT_DEBUG){            
            dbg_ofs=std::make_shared<std::ofstream>("/data/out.csv");
        }

        for (int epoch = 0; epoch < insaneNbEpoch; epoch++){
            for (int index = 0; index < SIZE; index++){
                in[index]=((kfr::fbase)mono_16k_s16_pcm[epoch*160 + index]);
            }
            auto out = fftContext_.doWindowAndFftAndGetAbs(in, mono_16k_s16_pcm.size());
            if(PRINT_DEBUG){
                (*dbg_ofs) << out[0];
                for (int i = 1; i < out.size(); i++){
                    (*dbg_ofs) << "," << out[i];
                }
                (*dbg_ofs)<< "\n";
            }

            for (int i = 0; i < out.size(); i++){
                result.push_back(out[i]);
            }
        }
        if (PRINT_DEBUG){
            dbg_ofs->close();
        }

        return result;
    }

};



