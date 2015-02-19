#define _FILE_OFFSET_BITS 64
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
#include <boost/assert.hpp>
#include <fcntl.h>
#include <unistd.h>

std::ifstream::pos_type get_file_size(const char* filename)
{
    std::ifstream in(filename, std::ifstream::ate | std::ifstream::binary);
    return in.tellg(); 
}
class File{
    int fd;
    long long xread(int fd, void *buf, size_t len)
    {
        const static long long MAX_IO_SIZE = 8 * 1024 * 1024;
        ssize_t nr;
        if (len > MAX_IO_SIZE)
            len = MAX_IO_SIZE;
        while (true) {
            nr = ::read(fd, buf, len);
            if ((nr < 0) && (errno == EAGAIN || errno == EINTR))
                continue;
            return nr;
        }
    }
public:
    File(std::string const & filename):fd(open(filename.c_str(), O_RDONLY)){
#if _XOPEN_SOURCE >= 600 || _POSIX_C_SOURCE >= 200112L
        posix_fadvise(fd, 0, 0, POSIX_FADV_SEQUENTIAL);
#endif
    }
    operator bool() const{
        return fd != -1;
    }
    long long read(std::vector<char> & buf, long long size){
        if(fd == -1)
            return 0;
        long long blocksize = 128 * 1024;
        if(blocksize > size)
            blocksize = size;
        if(size > static_cast<long long>(buf.size()))
            buf.resize(size);
        long long base = 0;
        while(size){
            auto need_read = std::min(blocksize, size);
            BOOST_ASSERT_MSG(need_read + base <= static_cast<long long>(buf.size()), "buf overflow!");
            long long read_size = xread(fd, buf.data() + base, need_read);
            if(read_size == 0)
                break;
            if(read_size == -1)
                return 0;
            base += read_size;
            size -= read_size;
        }
        return base;
    }
    ~File(){
        if(fd != -1)
            close(fd);
    }
};

std::string process_file(std::string fn){
    static const auto blocksize = 9500 * 1024;
    std::vector<char> buf;
    auto file_size = get_file_size(fn.c_str());
    if(file_size <= blocksize){
        MD4_CTX context;
        MD4_Init(&context);
        File f(fn);
        if(!f)
            return "";
        auto length = f.read(buf, blocksize);
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
        File f(fn);
        if(!f)
            return "";
        while(true){
            MD4_CTX context;
            MD4_Init(&context);
            auto length = f.read(buf, blocksize);
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
                boost::to_lower(md4_hash);
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
