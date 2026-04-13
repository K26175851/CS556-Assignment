#include <iostream>
#include <vector>
#include <queue>
#include <algorithm>
#include <iomanip>
#include <cstring>
#include <map>
#include <set>
#include <fstream>

using namespace std;

// Packet structure
struct Packet {
    int id;
    int arrival_time;
    int source_port;
    int dest_port;
};

// Global input trace
vector<Packet> input_trace = {
    {1, 0, 0, 0}, {2, 0, 0, 1}, {3, 0, 1, 0}, {4, 0, 1, 2}, {5, 0, 2, 0},
    {6, 1, 0, 2}, {7, 1, 2, 1}, {8, 2, 1, 1}, {9, 2, 2, 2}, {10, 3, 0, 1},
    {11, 3, 1, 0}, {12, 3, 2, 1}, {13, 4, 0, 0}, {14, 4, 1, 2}, {15, 4, 2, 2},
    {16, 5, 0, 2}, {17, 5, 1, 1}, {18, 5, 2, 0}
};

// ============================================================================
// PART 1: Standard FIFO Queue with HoL Blocking
// ============================================================================

class FIFOSimulator {
public:
    int total_service_time;
    vector<string> log;

    FIFOSimulator() : total_service_time(0) {}

    void simulate() {
        vector<queue<Packet>> input_queues(3);  // 3 input ports
        int current_time = 0;
        int next_trace_idx = 0;
        int max_iterations = 1000;
        int iterations = 0;

        log.push_back("\n=== PART 1: FIFO QUEUE SIMULATION ===\n");
        log.push_back("Time Slot | Event\n");
        log.push_back("----------+----------------------------------------------\n");

        while (iterations < max_iterations) {
            iterations++;

            // Add new arrivals for this time slot
            while (next_trace_idx < (int)input_trace.size() &&
                   input_trace[next_trace_idx].arrival_time == current_time) {
                int src = input_trace[next_trace_idx].source_port;
                log.push_back("Time " + to_string(current_time) + ": p" +
                              to_string(input_trace[next_trace_idx].id) +
                              " arrives at I" + to_string(src) + "\n");
                input_queues[src].push(input_trace[next_trace_idx]);
                next_trace_idx++;
            }

            // Check if there's anything to process
            bool has_packets = false;
            for (int i = 0; i < 3; i++) {
                if (!input_queues[i].empty()) { has_packets = true; break; }
            }
            if (!has_packets && next_trace_idx >= (int)input_trace.size()) break;

            // Try to schedule packets - only front packet of each queue can be scheduled
            bool output_used[3] = {false, false, false};
            bool any_action = false;

            // Greedy scheduling: lowest input port number gets priority
            for (int input_port = 0; input_port < 3; input_port++) {
                if (!input_queues[input_port].empty()) {
                    Packet pkt = input_queues[input_port].front();
                    int output_port = pkt.dest_port;

                    if (!output_used[output_port]) {
                        // Packet can be transmitted
                        output_used[output_port] = true;
                        log.push_back("Time " + to_string(current_time) + ": p" +
                                    to_string(pkt.id) + " sent (I" + to_string(input_port) +
                                    "->O" + to_string(output_port) + ")\n");
                        input_queues[input_port].pop();
                        any_action = true;
                    } else {
                        // HoL blocking: output port is busy
                        log.push_back("Time " + to_string(current_time) +
                                    ": HoL BLOCKING - p" + to_string(pkt.id) +
                                    " blocked (O" + to_string(output_port) + " busy)\n");
                    }
                }
            }

            current_time++;
        }

        total_service_time = current_time;
        log.push_back("\nTotal Service Time (FIFO): " + to_string(total_service_time) +
                     " time slots\n");
    }

    void print_log() {
        for (const auto& line : log) cout << line;
    }

    void save_results(const string& filename) {
        ofstream file(filename);
        for (const auto& line : log) file << line;
        file << "\nTotal Service Time: " << total_service_time << "\n";
        file.close();
    }
};

// ============================================================================
// PART 2: Virtual Output Queuing (VOQ) with Exhaustive Search
// ============================================================================

class VOQOptimalSimulator {
public:
    int total_service_time;
    vector<string> log;

    VOQOptimalSimulator() : total_service_time(0) {}

    // Generate all possible valid matchings recursively
    void generate_matchings(int input_idx, vector<pair<int,int>>& current,
                            set<int>& used_outputs,
                            vector<vector<pair<int,int>>>& all_matchings,
                            vector<vector<queue<Packet>>>& voq) {
        if (input_idx == 3) {
            all_matchings.push_back(current);
            return;
        }
        // Try matching input_idx to each available output that has packets
        for (int out = 0; out < 3; out++) {
            if (!voq[input_idx][out].empty() && !used_outputs.count(out)) {
                used_outputs.insert(out);
                current.push_back({input_idx, out});
                generate_matchings(input_idx + 1, current, used_outputs, all_matchings, voq);
                current.pop_back();
                used_outputs.erase(out);
            }
        }
        // Also try not matching this input
        generate_matchings(input_idx + 1, current, used_outputs, all_matchings, voq);
    }

    void simulate() {
        vector<vector<queue<Packet>>> voq(3, vector<queue<Packet>>(3));
        int current_time = 0;
        int next_trace_idx = 0;
        int max_iterations = 100;
        int iterations = 0;

        log.push_back("\n=== PART 2: VOQ WITH EXHAUSTIVE OPTIMAL SEARCH ===\n");
        log.push_back("Time Slot | Best Matching Configuration\n");
        log.push_back("----------+----------------------------------------------\n");

        while (iterations < max_iterations) {
            iterations++;

            // Add new arrivals for this time slot
            while (next_trace_idx < (int)input_trace.size() &&
                   input_trace[next_trace_idx].arrival_time == current_time) {
                const Packet& pkt = input_trace[next_trace_idx];
                voq[pkt.source_port][pkt.dest_port].push(pkt);
                log.push_back("Time " + to_string(current_time) + ": p" +
                              to_string(pkt.id) + " arrives at VOQ[I" +
                              to_string(pkt.source_port) + "][O" +
                              to_string(pkt.dest_port) + "]\n");
                next_trace_idx++;
            }

            // Check if any packets remain
            bool has_packets = false;
            for (int i = 0; i < 3 && !has_packets; i++)
                for (int j = 0; j < 3 && !has_packets; j++)
                    if (!voq[i][j].empty()) has_packets = true;

            if (!has_packets && next_trace_idx >= (int)input_trace.size()) break;
            if (!has_packets) { current_time++; continue; }

            // Generate all possible valid matchings
            vector<vector<pair<int,int>>> all_matchings;
            vector<pair<int,int>> current_matching;
            set<int> used_outputs;
            generate_matchings(0, current_matching, used_outputs, all_matchings, voq);

            // Find matching that transfers the most packets
            int best_packets = 0;
            vector<pair<int,int>> best_match;
            for (const auto& matching : all_matchings) {
                if ((int)matching.size() > best_packets) {
                    best_packets = (int)matching.size();
                    best_match = matching;
                }
            }

            // Apply the best matching
            log.push_back("Time " + to_string(current_time) + ": Scheduled: ");
            if (best_match.empty()) {
                log.push_back("(none)\n");
            } else {
                for (const auto& m : best_match) {
                    int input = m.first, output = m.second;
                    if (!voq[input][output].empty()) {
                        Packet pkt = voq[input][output].front();
                        voq[input][output].pop();
                        log.push_back("p" + to_string(pkt.id) + "(I" +
                                    to_string(input) + "->O" + to_string(output) + ") ");
                    }
                }
                log.push_back("\n");
            }

            current_time++;
        }

        total_service_time = current_time;
        log.push_back("\nTotal Service Time (Optimal VOQ): " + to_string(total_service_time) +
                     " time slots\n");
    }

    void print_log() {
        for (const auto& line : log) cout << line;
    }

    void save_results(const string& filename) {
        ofstream file(filename);
        for (const auto& line : log) file << line;
        file << "\nTotal Service Time: " << total_service_time << "\n";
        file.close();
    }
};

// ============================================================================
// PART 3: VOQ with iSLIP Scheduling Algorithm
// ============================================================================
class iSLIPSimulator {
public:
    int total_service_time;
    vector<string> log;
    vector<int> grant_pointers;   // For each output
    vector<int> accept_pointers;  // For each input

    iSLIPSimulator() : total_service_time(0), grant_pointers(3, 0), accept_pointers(3, 0) {}

    void simulate() {
        vector<vector<queue<Packet>>> voq(3, vector<queue<Packet>>(3));
        int current_time = 0;

        // Load initial packets into VOQ
        for (const auto& pkt : input_trace) {
            voq[pkt.source_port][pkt.dest_port].push(pkt);
        }

        log.push_back("\n=== PART 3: iSLIP SCHEDULING ALGORITHM ===\n");
        log.push_back("Algorithm Phases: Request -> Grant -> Accept\n");
        log.push_back("----------+----------------------------------------------\n");

        int max_iterations = 100;
        int iterations = 0;

        while (iterations < max_iterations) {
            iterations++;

            // Check if any packets remain
            bool has_packets = false;
            for (int i = 0; i < 3; i++) {
                for (int j = 0; j < 3; j++) {
                    if (!voq[i][j].empty()) {
                        has_packets = true;
                        break;
                    }
                }
                if (has_packets) break;
            }

            if (!has_packets) break;

            log.push_back("\nTime " + to_string(current_time) + ":\n");

            // ========== PHASE 1: REQUEST ==========
            // Each input requests all outputs for which it has packets
            vector<set<int>> requests(3);
            log.push_back("  REQUEST PHASE:\n");
            for (int input = 0; input < 3; input++) {
                for (int output = 0; output < 3; output++) {
                    if (!voq[input][output].empty()) {
                        requests[input].insert(output);
                    }
                }
                if (!requests[input].empty()) {
                    log.push_back("    I" + to_string(input) + " requests O{");
                    bool first = true;
                    for (int out : requests[input]) {
                        if (!first) log.push_back(",");
                        log.push_back(to_string(out));
                        first = false;
                    }
                    log.push_back("}\n");
                }
            }

            // ========== PHASE 2: GRANT ==========
            // Each output selects an input using round-robin from grant pointer
            vector<int> grants(3, -1);
            log.push_back("  GRANT PHASE:\n");
            for (int output = 0; output < 3; output++) {
                int start = grant_pointers[output];
                bool found = false;

                // Search in round-robin order starting from grant pointer
                for (int offset = 0; offset < 3; offset++) {
                    int input = (start + offset) % 3;
                    if (requests[input].count(output)) {
                        grants[output] = input;
                        log.push_back("    O" + to_string(output) + " grants to I" +
                                    to_string(input) + "\n");
                        found = true;
                        break;
                    }
                }

                if (!found) {
                    log.push_back("    O" + to_string(output) + " grants to none\n");
                }
            }

            // ========== PHASE 3: ACCEPT ==========
            // Each input selects a grant using round-robin from accept pointer
            vector<pair<int, int>> matchings;
            vector<bool> output_used(3, false);
            log.push_back("  ACCEPT PHASE:\n");

            for (int input = 0; input < 3; input++) {
                int start = accept_pointers[input];
                bool found = false;

                // Search in round-robin order starting from accept pointer
                for (int offset = 0; offset < 3; offset++) {
                    int output = (start + offset) % 3;
                    if (grants[output] == input && !output_used[output]) {
                        matchings.push_back({input, output});
                        output_used[output] = true;
                        log.push_back("    I" + to_string(input) + " accepts from O" +
                                    to_string(output) + "\n");
                        found = true;
                        break;
                    }
                }

                if (!found) {
                    log.push_back("    I" + to_string(input) + " accepts from none\n");
                }
            }

            // ========== EXECUTION ==========
            // Execute the accepted matchings
            log.push_back("  EXECUTION:\n");
            for (const auto& m : matchings) {
                int input = m.first;
                int output = m.second;
                if (!voq[input][output].empty()) {
                    Packet pkt = voq[input][output].front();
                    voq[input][output].pop();
                    log.push_back("    p" + to_string(pkt.id) + " sent from I" +
                                to_string(input) + " to O" + to_string(output) + "\n");

                    // Update accept pointer: move to next position after accepted output
                    accept_pointers[input] = (output + 1) % 3;
                }
            }

            // Update grant pointers for outputs that granted
            for (const auto& m : matchings) {
                int output = m.second;
                grant_pointers[output] = (grant_pointers[output] + 1) % 3;
            }

            // Print current pointer states
            log.push_back("  POINTER UPDATE:\n");
            log.push_back("    Grant pointers: ");
            for (int i = 0; i < 3; i++) {
                log.push_back(to_string(grant_pointers[i]) + " ");
            }
            log.push_back("\n    Accept pointers: ");
            for (int i = 0; i < 3; i++) {
                log.push_back(to_string(accept_pointers[i]) + " ");
            }
            log.push_back("\n");

            current_time++;
        }

        total_service_time = current_time;
        log.push_back("\nTotal Service Time (iSLIP): " + to_string(total_service_time) + 
                     " time slots\n");
    }

    void print_log() {
        for (const auto& line : log) {
            cout << line;
        }
    }

    void save_results(const string& filename) {
        ofstream file(filename);
        for (const auto& line : log) {
            file << line;
        }
        file << "\nTotal Service Time: " << total_service_time << "\n";
        file.close();
    }
};


// ============================================================================
// Main Function
// ============================================================================

int main() {
    cout << "=============================================================================\n";
    cout << "        Network Switch Scheduling Algorithms - Comprehensive Simulation\n";
    cout << "=============================================================================\n";

    FIFOSimulator fifo;
    cout << "\n[Running Part 1: FIFO Queue with HoL Blocking...]\n";
    fifo.simulate();
    fifo.print_log();
    fifo.save_results("part1_fifo_results.txt");

    VOQOptimalSimulator voq_optimal;
    cout << "\n[Running Part 2: VOQ with Optimal Exhaustive Search...]\n";
    voq_optimal.simulate();
    voq_optimal.print_log();
    voq_optimal.save_results("part2_voq_optimal_results.txt");

    iSLIPSimulator islip;
    cout << "\n[Running Part 3: VOQ with iSLIP Scheduling...]\n";
    islip.simulate();
    islip.print_log();
    islip.save_results("part3_islip_results.txt");

    cout << "\n=============================================================================\n";
    cout << "                              SUMMARY\n";
    cout << "=============================================================================\n";
    cout << "\nTotal Service Times:\n";
    cout << "  FIFO (with HoL blocking):    " << fifo.total_service_time << " time slots\n";
    cout << "  VOQ (Optimal):               " << voq_optimal.total_service_time << " time slots\n";
    cout << "  iSLIP (Practical Algorithm): " << islip.total_service_time << " time slots\n";

    cout << "\nResults saved to:\n";
    cout << "  - part1_fifo_results.txt\n";
    cout << "  - part2_voq_optimal_results.txt\n";
    cout << "  - part3_islip_results.txt\n";

    return 0;
}