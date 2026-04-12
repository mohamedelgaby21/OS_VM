#ifndef DES_H
#define DES_H
#include <string>


std::string encryptDES(std::string data, std::string key) {
    std::string output = data;
    for(int i = 0; i < data.length(); i++) {
        output[i] = data[i] ^ key[i % key.length()];
        output[i] = (output[i] << 1) | (output[i] >> 7);
    }
    return output;
}
#endif