#include <iostream>
#include <filesystem>
#include <map>
#include <string>


using namespace std;


namespace fs = std::filesystem;

int main() {
    string path;
    uintmax_t bin_width;

   
    cout << "Enter the directory path: ";
    getline(cin, path);
    cout << "Enter the bin width: ";
    cin >> bin_width;

    
    map<uintmax_t, int> histogram;

    try {
       
        for (const auto& entry : fs::recursive_directory_iterator(path)) {
            
          
            if (fs::is_regular_file(entry)) {
                uintmax_t size = fs::file_size(entry);
                
                
                uintmax_t bin_index = size / bin_width;
                histogram[bin_index]++;
            }
        }

        
        cout << "\nFile Size Histogram:" << endl;
        
        
        for (auto const& [bin_index, count] : histogram) {
            uintmax_t low = bin_index * bin_width;
            uintmax_t high = low + bin_width - 1;
            
            cout << "[" << low << " - " << high << " bytes]: " << count << " file(s)" << endl;
        }

    } catch (const fs::filesystem_error& e) {
        cerr << "Error: Could not read the directory." << endl;
    }

    return 0;
}