#include <iostream>
#include <vector>
#include <iomanip>

using namespace std;

// ==========================================
// 1. Data Setup: Structure for our processes
// ==========================================
struct Process {
    int id;
    int arrivalTime;
    int burstTime;
};

// ==========================================
// 2. FCFS (First-Come, First-Served)
// ==========================================
float calculateFCFS(vector<Process> procs) {
    int n = procs.size();
    vector<int> waitingTime(n, 0);
    int currentTime = 0;
    float totalWait = 0;

    for (int i = 0; i < n; i++) {
        // If the CPU is idle, fast-forward to the next arrival
        if (currentTime < procs[i].arrivalTime) {
            currentTime = procs[i].arrivalTime;
        }
        waitingTime[i] = currentTime - procs[i].arrivalTime;
        currentTime += procs[i].burstTime;
        totalWait += waitingTime[i];
    }
    return totalWait / n;
}

// ==========================================
// 3. SJF (Shortest Job First - Non-Preemptive)
// ==========================================
float calculateSJF(vector<Process> procs) {
    int n = procs.size();
    vector<int> waitingTime(n, 0);
    vector<bool> completed(n, false);
    int currentTime = 0;
    int completedCount = 0;
    float totalWait = 0;

    while (completedCount < n) {
        int minBurst = 99999; // Start with a huge number for comparison
        int shortestIndex = -1;

        // Find the shortest process that has arrived but not finished
        for (int i = 0; i < n; i++) {
            if (procs[i].arrivalTime <= currentTime && !completed[i]) {
                if (procs[i].burstTime < minBurst) {
                    minBurst = procs[i].burstTime;
                    shortestIndex = i;
                }
            }
        }

        // If no process is ready, tick the clock forward
        if (shortestIndex == -1) {
            currentTime++;
        } else {
            waitingTime[shortestIndex] = currentTime - procs[shortestIndex].arrivalTime;
            currentTime += procs[shortestIndex].burstTime;
            completed[shortestIndex] = true;
            completedCount++;
            totalWait += waitingTime[shortestIndex];
        }
    }
    return totalWait / n;
}

// ==========================================
// 4. Round Robin
// ==========================================
float calculateRR(vector<Process> procs, int quantum) {
    int n = procs.size();
    vector<int> remainingBurst(n);
    
    // Copy burst times because we will subtract from them
    for (int i = 0; i < n; i++) {
        remainingBurst[i] = procs[i].burstTime;
    }

    int currentTime = procs[0].arrivalTime;
    vector<int> queue;
    vector<bool> inQueue(n, false);

    queue.push_back(0);
    inQueue[0] = true;
    int completedCount = 0;
    float totalWait = 0;

    while (completedCount < n) {
        // If the queue is empty but processes are still coming
        if (queue.empty()) {
            currentTime++;
            for (int i = 0; i < n; i++) {
                if (procs[i].arrivalTime <= currentTime && !inQueue[i] && remainingBurst[i] > 0) {
                    queue.push_back(i);
                    inQueue[i] = true;
                }
            }
            continue;
        }

        int i = queue.front();
        queue.erase(queue.begin()); // Pop from the front of the line

        // Execute for the quantum or whatever burst time is left
        if (remainingBurst[i] > quantum) {
            currentTime += quantum;
            remainingBurst[i] -= quantum;
        } else {
            currentTime += remainingBurst[i];
            int waitTime = currentTime - procs[i].arrivalTime - procs[i].burstTime;
            totalWait += waitTime;
            remainingBurst[i] = 0;
            completedCount++;
        }

        // Check if any new processes arrived while this one was executing
        for (int j = 0; j < n; j++) {
            if (procs[j].arrivalTime <= currentTime && !inQueue[j] && remainingBurst[j] > 0) {
                queue.push_back(j);
                inQueue[j] = true;
            }
        }

        // If the current process isn't done, send it to the back of the line
        if (remainingBurst[i] > 0) {
            queue.push_back(i);
        }
    }
    return totalWait / n;
}

// ==========================================
// 5. Main Execution and Chart Output
// ==========================================
int main() {
    // Processes: {ID, Arrival Time, Burst Time}
    vector<Process> processes = {
        {1, 0, 8},
        {2, 1, 4},
        {3, 2, 9},
        {4, 3, 5}
    };

    int timeQuantum = 3;

    float avgFCFS = calculateFCFS(processes);
    float avgSJF = calculateSJF(processes);
    float avgRR = calculateRR(processes, timeQuantum);

    // Print raw numbers
    cout << fixed << setprecision(2);
    cout << "Average Waiting Time - FCFS: " << avgFCFS << endl;
    cout << "Average Waiting Time - SJF:  " << avgSJF << endl;
    cout << "Average Waiting Time - RR:   " << avgRR << endl;

    // Plotting the ASCII Bar Chart in the Console
    cout << "\n============================================\n";
    cout << "     CPU SCHEDULING ALGORITHM COMPARISON      \n";
    cout << "============================================\n";
    
    // Draw FCFS Bar
    cout << "FCFS : "; 
    for(int i = 0; i < (int)avgFCFS; i++) cout << "■ "; 
    cout << "(" << avgFCFS << ")\n";

    // Draw SJF Bar
    cout << "SJF  : "; 
    for(int i = 0; i < (int)avgSJF; i++) cout << "■ "; 
    cout << "(" << avgSJF << ")\n";

    // Draw RR Bar
    cout << "RR   : "; 
    for(int i = 0; i < (int)avgRR; i++) cout << "■ "; 
    cout << "(" << avgRR << ")\n\n";

    return 0;
}