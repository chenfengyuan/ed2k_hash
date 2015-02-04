#include <string>
#include <iostream>
#include <openssl/md4.h>
#include <stdio.h>
#include <vector>
#include <sstream>
#include <fstream>

long long get_file_size (const char *fn)
{
        FILE * file;
        long long fsize;

        file = fopen(fn, "rb");
        if (!file)
            return 0;

        if (fseek(file, 0, SEEK_END)<0)         /* go to very last byte of file */
                return 0;

        fsize = ftell(file);
        if (fsize == -1)
                return 0;

        fclose(file);

        return fsize;
}
std::string process_file(std::string fn){
    static const auto blocksize = 9500 * 1024;
    std::FILE * f;
    std::vector<char> buf;
    buf.resize(blocksize);
    auto file_size = get_file_size(fn.c_str());
    if(file_size <= blocksize){
        MD4_CTX context;
        MD4_Init(&context);
        f = std::fopen(fn.c_str(), "rb");
        if(!f)
            return "";
        auto length = std::fread(buf.data(), 1, buf.size(), f);
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
        fclose(f);
        return out.str();
    }else{
        std::ostringstream md4_hashes;
        f = std::fopen(fn.c_str(), "rb");
        if(!f)
            return "";
        while(true){
            MD4_CTX context;
            MD4_Init(&context);
            auto length = std::fread(buf.data(), 1, buf.size(), f);
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
        fclose(f);
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

int main(int argc, char* argv[]){
    if(argc > 1){
        if (std::string(argv[1]) != std::string("-c")){
            for(int i=1;i<argc;++i){
                std::cout << process_file(argv[i]) << " " << argv[i] << "\n";
            }
        }else{
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
        }
    }
    return 0;
}
