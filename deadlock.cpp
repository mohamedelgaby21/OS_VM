
#include <iostream>
#include <fstream>
#include <vector>

using namespace std;

int main() {
    ifstream file("input2.txt");
    if (!file.is_open()) {
        cout << "Error: Cannot open input2.txt" << endl;
        return 0;
    }

    int n, m;
    file >> n >> m;

    vector<int> E(m);
    for (int i = 0; i < m; i++) {
        file >> E[i];
    }

    vector<vector<int>> C(n, vector<int>(m));
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            file >> C[i][j];
        }
    }

    vector<vector<int>> R(n, vector<int>(m));
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            file >> R[i][j];
        }
    }

    vector<int> work(m);
    for (int j = 0; j < m; j++) {
        int sum_allocated = 0;
        for (int i = 0; i < n; i++) {
            sum_allocated += C[i][j];
        }
        work[j] = E[j] - sum_allocated;
    }

    vector<bool> finish(n, false);
    
    for (int i = 0; i < n; i++) {
        bool has_resources = false;
        for (int j = 0; j < m; j++) {
            if (C[i][j] > 0) {
                has_resources = true;
            }
        }
        if (!has_resources) {
            finish[i] = true;
        }
    }

    while (true) {
        bool found_process = false;

        for (int i = 0; i < n; i++) {
            if (!finish[i]) {
                bool can_finish = true;
                for (int j = 0; j < m; j++) {
                    if (R[i][j] > work[j]) {
                        can_finish = false;
                        break;
                    }
                }

                if (can_finish) {
                    for (int j = 0; j < m; j++) {
                        work[j] += C[i][j];
                    }
                    finish[i] = true;
                    found_process = true;
                    break; 
                }
            }
        }

        if (!found_process) {
            break;
        }
    }

    bool has_deadlock = false;
    
    cout << " System Status" << endl;
    for (int i = 0; i < n; i++) {
        if (!finish[i]) {
            has_deadlock = true;
            break; 
        }
    }

    if (has_deadlock) {
        cout << "Result: DEADLOCK DETECTED!" << endl;
        cout << "Deadlocked Processes: ";
        for (int i = 0; i < n; i++) {
            if (!finish[i]) {
                cout << "P" << i << " ";
            }
        }
        cout << endl;
    } else {
        cout << "Result: No Deadlock. System is Safe." << endl;
    }

    return 0;
}