#include <iostream>
#include <string>
#include <ctime>
#include "des.h" 

using namespace std;

string getSalt() {
    string hexChars = "0123456789ABCDEF";
    string salt = "";
    for(int i = 0; i < 4; i++) {
        salt += hexChars[rand() % 16];
    }
    return salt;
}

int main() {
    srand(time(0));
    string rawPasswords[10] = {"ahmed24", "ahmed24", "admin", "secure", "root", "user1", "test", "last", "hello", "demo"};
    
    cout << "User\tSalt\tEncrypted (25x DES)\n";
    for(int i = 0; i < 10; i++) {
        string salt = getSalt();
        string currentPass = rawPasswords[i];
        
        for(int j = 0; j < 25; j++) {
            currentPass = encryptDES(currentPass, salt); 
        }
        cout << i+1 << "\t" << salt << "\t" << currentPass << endl;
    }
    return 0;
}