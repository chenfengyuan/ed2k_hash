#include <string>
#include <iostream>
#include <openssl/md4.h>
#include <stdio.h>
#include <vector>
#include <sstream>
#include <fstream>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/case_conv.hpp>

std::ifstream::pos_type get_file_size(const char* filename)
{
    std::ifstream in(filename, std::ifstream::ate | std::ifstream::binary);
    return in.tellg(); 
}
std::string process_file(std::string fn){
    static const auto blocksize = 9500 * 1024;
    std::vector<char> buf;
    buf.resize(blocksize);
    auto file_size = get_file_size(fn.c_str());
    if(file_size <= blocksize){
        MD4_CTX context;
        MD4_Init(&context);
        std::ifstream f(fn, std::ifstream::binary);
        if(!f.is_open())
            return "";
        f.read(buf.data(), buf.size());
        auto length = f.gcount();
        MD4_Update(&context, buf.data(), length);
        unsigned char md4[MD4_DIGEST_LENGTH];
        MD4_Final(md4, &context);
        std::ostringstream out;
        out << std::hex;
        for(auto i=0;i<MD4_DIGEST_LENGTH;++i){
            out.width(2);
            out.fill('0');
            out << static_cast<int>(md4[i]);
        }
        return out.str();
    }else{
        std::ostringstream md4_hashes;
        std::ifstream f(fn, std::ifstream::binary);
        if(!f.is_open())
            return "";
        while(true){
            MD4_CTX context;
            MD4_Init(&context);
            f.read(buf.data(), buf.size());
            auto length = f.gcount();
            if(length <= 0)
                break;
            MD4_Update(&context, buf.data(), length);
            unsigned char md4[MD4_DIGEST_LENGTH];
            MD4_Final(md4, &context);
            for(auto i=0;i<MD4_DIGEST_LENGTH;++i){
                md4_hashes << (md4[i]);
            }
            if(length < blocksize)
                break;
        }
        MD4_CTX context;
        MD4_Init(&context);
        auto content = md4_hashes.str();
        MD4_Update(&context, content.c_str(), content.length());
        unsigned char md4[MD4_DIGEST_LENGTH];
        MD4_Final(md4, &context);
        std::ostringstream out;
        out << std::hex;
        for(auto i=0;i<MD4_DIGEST_LENGTH;++i){
            out.width(2);
            out.fill('0');
            out << static_cast<int>(md4[i]);
        }
        return out.str();
    }
    return "";
}

std::vector<std::string> split(std::string str, std::string sep){
    std::vector<std::string> split_vector;
    boost::split(split_vector, str, boost::algorithm::is_any_of(sep));
    return split_vector;
}

int main(int argc, char* argv[]){
    if(argc > 1){
        if (std::string(argv[1]) == "-c"){
            std::ifstream fin(argv[2]);
            while(true){
                std::string fn, ed2k_hash;
                fin >> ed2k_hash >> fn;
                if(fn.length() == 0)
                    break;
                std::string file_hash = process_file(fn);
                if(file_hash == ed2k_hash)
                    std::cout << fn << ": OK\n";
                else
                    std::cout << fn << ": FAILED\n";
            }
            return 0;
        }
        if (std::string(argv[1]) == "-e"){
            std::ifstream fin(argv[2]);
            while(true){
                std::string ed2k_link;
                fin >> ed2k_link;
                if(ed2k_link.length() == 0)
                    break;
                auto tmp = split(ed2k_link, "|");
                auto file_length = tmp[3];
                auto file_name = tmp[2];
                auto md4_hash = tmp[4];
                auto real_file_length = get_file_size(file_name.c_str());
                auto correctp = false;
                if(std::to_string(real_file_length) == file_length){
                    auto file_hash = process_file(file_name);
                    if(file_hash == md4_hash){
                        correctp = true;
                    }
                }
                if(correctp)
                    std::cout << file_name << ": OK\n";
                else
                    std::cout << file_name << ": FAILED\n";
            }
            return 0;
        }
        for(int i=1;i<argc;++i){
            std::cout << process_file(argv[i]) << " " << argv[i] << "\n";
        }
        return 0;
    }
    return 0;
}
