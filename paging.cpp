#include <iostream>
#include <fstream>
#include <vector>

using namespace std;


struct Page {
    int id;
    unsigned char counter; 
};


int simulateAging(const vector<int>& references, int numFrames) {
    vector<Page> memory;
    int pageFaults = 0;

    for (int i = 0; i < references.size(); i++) {
        int currentRef = references[i];
        bool pageFound = false;

        
        for (int j = 0; j < memory.size(); j++) {
            memory[j].counter = memory[j].counter >> 1; 
        }

        
        for (int j = 0; j < memory.size(); j++) {
            if (memory[j].id == currentRef) {
                
                memory[j].counter = memory[j].counter | 128;
                pageFound = true;
                break;
            }
        }

        
        if (!pageFound) {
            pageFaults++;

            
            if (memory.size() == numFrames) {
                int evictIndex = 0;
                unsigned char minCounter = 255; 

                
                for (int j = 0; j < memory.size(); j++) {
                    if (memory[j].counter < minCounter) {
                        minCounter = memory[j].counter;
                        evictIndex = j;
                    }
                }
                
                
                memory[evictIndex].id = currentRef;
                memory[evictIndex].counter = 128; 
            } else {
               
                Page newPage;
                newPage.id = currentRef;
                newPage.counter = 128;
                memory.push_back(newPage);
            }
        }
    }
    
    return pageFaults;
}

int main() {
    
    ifstream inputFile("references.txt");
    if (!inputFile.is_open()) {
        cout << "Error: Could not open references.txt" << endl;
        cout << "Please create a file named 'references.txt' with numbers in it." << endl;
        return 1;
    }

    vector<int> references;
    int ref;
    while (inputFile >> ref) {
        references.push_back(ref);
    }
    inputFile.close();

    int totalRefs = references.size();
    if (totalRefs == 0) {
        cout << "Error: The input file is empty." << endl;
        return 1;
    }

    cout << "Loaded " << totalRefs << " memory references." << endl;

    
    ofstream outputFile("results.csv");
    outputFile << "Frames,Faults_Per_1000\n"; 

    cout << "Running simulation..." << endl;

    
    for (int frames = 1; frames <= 50; frames++) {
        int faults = simulateAging(references, frames);
        
        
        double faultsPer1000 = ((double)faults / totalRefs) * 1000.0;
        
        
        outputFile << frames << "," << faultsPer1000 << "\n";
    }

    outputFile.close();
    cout << "Simulation complete!" << endl;
    cout << "Results saved to 'results.csv'. You can open this in Excel to plot the graph." << endl;

    return 0;
}