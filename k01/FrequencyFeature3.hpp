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
        kfr::univector<kfr::u8> temp_;
        const kfr::internal::expression_hamming<kfr::fbase> hamming_;

    public:
        // perform forward fft
        FftContext(): dft_(SIZE), temp_(SIZE), hamming_(SIZE)
        { }

        auto doWindowAndFftAndGetAbs(const kfr::univector<kfr::fbase, SIZE> &in, const size_t insaneWholeLength){

            kfr::univector<kfr::fbase, SIZE/2> out;

            kfr::univector<kfr::complex<kfr::fbase>, SIZE> inComplex = in * hamming_;
            kfr::univector<kfr::complex<kfr::fbase>, SIZE> outComplex;
            // perform forward fft
            dft_.execute(outComplex, inComplex, temp_); // try realdft function?
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

    std::string doMain(const std::vector<int16_t> &mono_16k_s16_pcm){
        std::string result = kfr::library_version() ;
        result = result + "\n";

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
        }
        if (PRINT_DEBUG){
            dbg_ofs->close();
        }

        return result;
    }

    static auto readPCM(const char *filePath){
        std::vector<int16_t> thePCM(16000*100);
        kfr::audio_reader_wav<int16_t> reader(kfr::open_file_for_reading(filePath));
        auto readSize = reader.read(thePCM.data(), thePCM.size());
        thePCM.resize(readSize);

        return std::move(thePCM);
    }

    static void readWavFileAndDoMain(const char *filePath){
        FrequencyFeature3<400,false> feature3;
        auto t0 = std::chrono::system_clock::now();
        auto thePcm = readPCM(filePath);
        auto stringResult = feature3.doMain(thePcm);
        auto t1 = std::chrono::system_clock::now();

        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0);

        stringResult = stringResult+ "\n"+"use: " + std::to_string(
                double(duration.count()) *  std::chrono::microseconds::period::num /  std::chrono::microseconds::period::den
        ) + "s";

        printf("%s\n",stringResult.c_str());

    }
};



