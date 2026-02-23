#include <iostream>
#include <pthread.h>
#include <string>
#include <map>
#include <sstream>
#include <fstream>
#include <vector>

using namespace std;


struct ThreadData {
    int id;
    vector<string> textLines; 
    map<string, int> localWordCount;
};

void* countWords(void* arg) {
    ThreadData* data = (ThreadData*) arg;
    
   
    for (int i = 0; i < data->textLines.size(); i++) {
        stringstream ss(data->textLines[i]);
        string word;
        while (ss >> word) {
            data->localWordCount[word]++;
        }
    }
    
    cout << "Thread " << data->id << " completed its segment." << endl;
    pthread_exit(NULL);
}

int main() {
    const int N = 3; 
    pthread_t threads[N];
    ThreadData threadData[N];
    
    
    ifstream file("input.txt");
    if (!file.is_open()) {
        cout << "Error: Could not open the file named 'input.txt'." << endl;
        return 1;
    }

    
    string line;
    int currentThread = 0;
    while (getline(file, line)) {
        
        threadData[currentThread].textLines.push_back(line);
        
        
        currentThread = (currentThread + 1) % N; 
    }
    file.close();

    
    for (int i = 0; i < N; i++) {
        threadData[i].id = i;
        pthread_create(&threads[i], NULL, countWords, &threadData[i]);
    }

    
    for (int i = 0; i < N; i++) {
        pthread_join(threads[i], NULL);
    }

    
    map<string, int> totalWordCount;
    for (int i = 0; i < N; i++) {
        for (auto const& pair : threadData[i].localWordCount) {
            totalWordCount[pair.first] += pair.second;
        }
    }

   
    cout << "\n--- Final Word Count ---" << endl;
    for (auto const& pair : totalWordCount) {
        cout << pair.first << ": " << pair.second << endl;
    }

    return 0;
}