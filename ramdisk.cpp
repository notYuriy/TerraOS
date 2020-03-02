#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <cstring>
#include <experimental/filesystem>

namespace fs = std::experimental::filesystem;

void visitDirectory(std::string path, std::ofstream& ofstream, uint64_t root_off){
    for(auto& p : fs::directory_iterator(path)){
        std::string filename = fs::path(p).filename();
        if(fs::is_directory(p)){
            std::cout << "Directory " << p << "\n";
            size_t offset = ofstream.tellp();
            ofstream.write("        ", 8);
            char name[100]; std::memset(name, '\0', 100);
            std::memcpy(name, filename.c_str(), (filename.size() > 100)?100:filename.size());
            ofstream.write(name, 100);
            uint64_t type = 1;
            ofstream.write((char*)&type, sizeof(type));
            ofstream.write((char*)&root_off, sizeof(root_off));
            visitDirectory(p.path(), ofstream, offset);
            size_t offset2 = ofstream.tellp();
            ofstream.seekp(offset);
            uint64_t size = offset2 - offset;
            ofstream.write((char*)&size, 8);
            ofstream.seekp(offset2);
        }
        if(fs::is_regular_file(p)){
            std::cout << "Regular file " << p << "\n";
            size_t offset = ofstream.tellp();
            ofstream.write("        ", 8);
            char name[100]; std::memset(name, '\0', 100);
            std::memcpy(name, filename.c_str(), (filename.size() > 100)?100:filename.size());
            ofstream.write(name, 100);
            uint64_t type = 0;
            ofstream.write((char*)&type, sizeof(type));
            ofstream.write((char*)&root_off, sizeof(root_off));
            std::ifstream file(p.path());
            char buf[1024]; size_t read = 0;
            while(read = file.readsome(buf, 1024)){
                ofstream.write(buf, read);
            }
            size_t offset2 = ofstream.tellp();
            ofstream.seekp(offset);
            uint64_t size = offset2 - offset;
            ofstream.write((char*)&size, 8);
            ofstream.seekp(offset2);
        }
    }
}

int main(int argc, char** argv){
    if(argc != 3){
        return EXIT_FAILURE;
    }
    std::string folder = argv[1];
    std::string disk = argv[2];
    std::ofstream output(disk, std::ios_base::binary);
    visitDirectory(folder, output, 0);
    return 0;
}