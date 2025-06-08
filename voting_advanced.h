#ifndef VOTING_ADVANCED_H
#define VOTING_ADVANCED_H

#include <iostream>
#include <vector>
#include <queue>
#include <stack>
#include <string>
#include <fstream>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <map>
#include <set>
#include <algorithm>
#include <memory>
#include <stdexcept>
#include <functional>

// Custom Exception Classes
class VotingException : public std::runtime_error {
public:
    explicit VotingException(const std::string& msg) : std::runtime_error(msg) {}
};

class DataValidationException : public VotingException {
public:
    explicit DataValidationException(const std::string& msg) : VotingException("Validasi Data: " + msg) {}
};

class SecurityException : public VotingException {
public:
    explicit SecurityException(const std::string& msg) : VotingException("Keamanan: " + msg) {}
};

// Logging System
class VotingLogger {
private:
    std::ofstream logFile;
    std::string getTimestamp() {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
        return ss.str();
    }

public:
    VotingLogger(const std::string& filename = "voting_log.txt") {
        logFile.open(filename, std::ios::app);
    }

    ~VotingLogger() {
        if (logFile.is_open()) {
            logFile.close();
        }
    }

    void log(const std::string& level, const std::string& message) {
        if (logFile.is_open()) {
            logFile << "[" << getTimestamp() << "] [" << level << "] " << message << std::endl;
        }
    }

    void info(const std::string& message) { log("INFO", message); }
    void error(const std::string& message) { log("ERROR", message); }
    void warning(const std::string& message) { log("WARNING", message); }
    void debug(const std::string& message) { log("DEBUG", message); }
};

// Tree Visualization
class TreeVisualizer {
public:
    static void visualizeBST(const std::shared_ptr<VoterNode>& root, int level = 0) {
        if (!root) return;
        
        visualizeBST(root->right, level + 1);
        
        std::cout << std::string(level * 4, ' ') << "└── " 
                  << root->voterId << " (" << root->nama << ")" << std::endl;
        
        visualizeBST(root->left, level + 1);
    }

    static void visualizeAVL(const std::shared_ptr<AVLNode>& root, int level = 0) {
        if (!root) return;
        
        visualizeAVL(root->right, level + 1);
        
        std::cout << std::string(level * 4, ' ') << "└── " 
                  << "Kandidat " << root->kandidatId << " (" 
                  << root->voteCount << " suara)" << std::endl;
        
        visualizeAVL(root->left, level + 1);
    }
};

// Graph Visualization dan Algoritma Tambahan
class AdvancedGraph : public VotingGraph {
public:
    // Visualisasi graf menggunakan adjacency matrix
    void visualizeGraph() const {
        std::map<std::string, int> nodeIndices;
        int index = 0;
        
        // Assign indices to nodes
        for (const auto& pair : adjacencyList) {
            nodeIndices[pair.first] = index++;
        }
        
        // Create adjacency matrix
        std::vector<std::vector<int>> matrix(index, std::vector<int>(index, 0));
        
        // Fill matrix
        for (const auto& pair : adjacencyList) {
            int i = nodeIndices[pair.first];
            for (const auto& neighbor : pair.second) {
                int j = nodeIndices[neighbor];
                matrix[i][j] = 1;
            }
        }
        
        // Print matrix
        std::cout << "\nVisualisasi Jaringan Pemilih:\n";
        std::cout << "   ";
        for (const auto& pair : nodeIndices) {
            std::cout << pair.first.substr(0, 3) << " ";
        }
        std::cout << "\n";
        
        for (int i = 0; i < index; i++) {
            std::cout << nodeIndices.begin()->first.substr(0, 3) << " ";
            for (int j = 0; j < index; j++) {
                std::cout << (matrix[i][j] ? "1  " : "0  ");
            }
            std::cout << "\n";
        }
    }

    // Dijkstra's Algorithm untuk mencari jalur terpendek
    std::vector<std::string> findShortestPath(const std::string& start, const std::string& end) {
        std::map<std::string, int> distance;
        std::map<std::string, std::string> previous;
        std::set<std::string> unvisited;
        
        // Initialize
        for (const auto& pair : adjacencyList) {
            distance[pair.first] = INT_MAX;
            unvisited.insert(pair.first);
        }
        distance[start] = 0;
        
        while (!unvisited.empty()) {
            // Find unvisited node with minimum distance
            std::string current = *unvisited.begin();
            int minDist = INT_MAX;
            for (const auto& node : unvisited) {
                if (distance[node] < minDist) {
                    minDist = distance[node];
                    current = node;
                }
            }
            
            if (current == end) break;
            unvisited.erase(current);
            
            // Update distances
            for (const auto& neighbor : adjacencyList[current]) {
                if (unvisited.find(neighbor) != unvisited.end()) {
                    int newDist = distance[current] + 1;
                    if (newDist < distance[neighbor]) {
                        distance[neighbor] = newDist;
                        previous[neighbor] = current;
                    }
                }
            }
        }
        
        // Reconstruct path
        std::vector<std::string> path;
        for (std::string current = end; current != start; current = previous[current]) {
            path.push_back(current);
        }
        path.push_back(start);
        std::reverse(path.begin(), path.end());
        
        return path;
    }

    // Tarjan's Algorithm untuk mencari Strongly Connected Components
    std::vector<std::vector<std::string>> findStronglyConnectedComponents() {
        std::map<std::string, int> index;
        std::map<std::string, int> lowlink;
        std::stack<std::string> stack;
        std::set<std::string> onStack;
        std::vector<std::vector<std::string>> components;
        int currentIndex = 0;
        
        std::function<void(const std::string&)> strongConnect = 
            [&](const std::string& v) {
                index[v] = currentIndex;
                lowlink[v] = currentIndex;
                currentIndex++;
                stack.push(v);
                onStack.insert(v);
                
                for (const auto& w : adjacencyList[v]) {
                    if (index.find(w) == index.end()) {
                        strongConnect(w);
                        lowlink[v] = std::min(lowlink[v], lowlink[w]);
                    } else if (onStack.find(w) != onStack.end()) {
                        lowlink[v] = std::min(lowlink[v], index[w]);
                    }
                }
                
                if (lowlink[v] == index[v]) {
                    std::vector<std::string> component;
                    std::string w;
                    do {
                        w = stack.top();
                        stack.pop();
                        onStack.erase(w);
                        component.push_back(w);
                    } while (w != v);
                    components.push_back(component);
                }
            };
        
        for (const auto& pair : adjacencyList) {
            if (index.find(pair.first) == index.end()) {
                strongConnect(pair.first);
            }
        }
        
        return components;
    }
};

// Advanced Sorting dengan Multiple Algorithms
class AdvancedSorter {
public:
    // Merge Sort
    static void mergeSort(std::vector<std::pair<int, int>>& arr, int left, int right) {
        if (left < right) {
            int mid = left + (right - left) / 2;
            mergeSort(arr, left, mid);
            mergeSort(arr, mid + 1, right);
            merge(arr, left, mid, right);
        }
    }

    // Heap Sort
    static void heapSort(std::vector<std::pair<int, int>>& arr) {
        int n = arr.size();
        
        // Build heap
        for (int i = n / 2 - 1; i >= 0; i--)
            heapify(arr, n, i);
        
        // Extract elements from heap
        for (int i = n - 1; i > 0; i--) {
            std::swap(arr[0], arr[i]);
            heapify(arr, i, 0);
        }
    }

private:
    static void merge(std::vector<std::pair<int, int>>& arr, int left, int mid, int right) {
        int n1 = mid - left + 1;
        int n2 = right - mid;
        
        std::vector<std::pair<int, int>> L(n1), R(n2);
        
        for (int i = 0; i < n1; i++)
            L[i] = arr[left + i];
        for (int j = 0; j < n2; j++)
            R[j] = arr[mid + 1 + j];
        
        int i = 0, j = 0, k = left;
        while (i < n1 && j < n2) {
            if (L[i].second >= R[j].second) {
                arr[k] = L[i];
                i++;
            } else {
                arr[k] = R[j];
                j++;
            }
            k++;
        }
        
        while (i < n1) {
            arr[k] = L[i];
            i++;
            k++;
        }
        
        while (j < n2) {
            arr[k] = R[j];
            j++;
            k++;
        }
    }

    static void heapify(std::vector<std::pair<int, int>>& arr, int n, int i) {
        int largest = i;
        int left = 2 * i + 1;
        int right = 2 * i + 2;
        
        if (left < n && arr[left].second > arr[largest].second)
            largest = left;
        
        if (right < n && arr[right].second > arr[largest].second)
            largest = right;
        
        if (largest != i) {
            std::swap(arr[i], arr[largest]);
            heapify(arr, n, largest);
        }
    }
};

// Data Compression
class DataCompressor {
public:
    static std::string compress(const std::string& data) {
        std::string compressed;
        int count = 1;
        
        for (size_t i = 1; i <= data.length(); i++) {
            if (i == data.length() || data[i] != data[i-1]) {
                if (count > 1) {
                    compressed += std::to_string(count);
                }
                compressed += data[i-1];
                count = 1;
            } else {
                count++;
            }
        }
        
        return compressed;
    }

    static std::string decompress(const std::string& compressed) {
        std::string decompressed;
        int count = 0;
        
        for (char c : compressed) {
            if (isdigit(c)) {
                count = count * 10 + (c - '0');
            } else {
                if (count == 0) count = 1;
                decompressed.append(count, c);
                count = 0;
            }
        }
        
        return decompressed;
    }
};

// Backup System
class BackupSystem {
private:
    VotingLogger& logger;
    
public:
    BackupSystem(VotingLogger& log) : logger(log) {}
    
    void createBackup(const std::string& sourceFile, const std::string& backupDir) {
        try {
            std::string timestamp = std::to_string(std::time(nullptr));
            std::string backupFile = backupDir + "/backup_" + timestamp + ".dat";
            
            std::ifstream source(sourceFile, std::ios::binary);
            std::ofstream backup(backupFile, std::ios::binary);
            
            if (!source || !backup) {
                throw VotingException("Gagal membuat backup: File tidak dapat dibuka");
            }
            
            backup << source.rdbuf();
            
            // Compress backup file
            std::ifstream inFile(backupFile);
            std::string content((std::istreambuf_iterator<char>(inFile)),
                              std::istreambuf_iterator<char>());
            std::string compressed = DataCompressor::compress(content);
            
            std::ofstream outFile(backupFile + ".compressed");
            outFile << compressed;
            
            logger.info("Backup berhasil dibuat: " + backupFile + ".compressed");
        }
        catch (const std::exception& e) {
            logger.error("Error saat membuat backup: " + std::string(e.what()));
            throw;
        }
    }
    
    void restoreBackup(const std::string& backupFile, const std::string& targetFile) {
        try {
            std::ifstream inFile(backupFile);
            std::string compressed((std::istreambuf_iterator<char>(inFile)),
                                 std::istreambuf_iterator<char>());
            std::string decompressed = DataCompressor::decompress(compressed);
            
            std::ofstream outFile(targetFile);
            outFile << decompressed;
            
            logger.info("Backup berhasil dipulihkan ke: " + targetFile);
        }
        catch (const std::exception& e) {
            logger.error("Error saat memulihkan backup: " + std::string(e.what()));
            throw;
        }
    }
};

#endif // VOTING_ADVANCED_H 