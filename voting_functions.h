#ifndef VOTING_FUNCTIONS_H
#define VOTING_FUNCTIONS_H

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

// Struktur data untuk BST
struct BSTNode {
    std::string idPemilih;
    std::string nama;
    BSTNode* left;
    BSTNode* right;
    BSTNode(const std::string& id, const std::string& nm) 
        : idPemilih(id), nama(nm), left(nullptr), right(nullptr) {}
};

// Struktur data untuk AVL
struct AVLNode {
    int kandidatId;
    int voteCount;
    int height;
    AVLNode* left;
    AVLNode* right;
    AVLNode(int id, int count) 
        : kandidatId(id), voteCount(count), height(1), left(nullptr), right(nullptr) {}
};

// Struktur data untuk Graph
struct GraphNode {
    std::string idPemilih;
    std::vector<std::string> neighbors;
    GraphNode(const std::string& id) : idPemilih(id) {}
};

// Struktur data untuk Hash Table
struct HashEntry {
    std::string idPemilih;
    std::string nama;
    bool isOccupied;
    HashEntry() : isOccupied(false) {}
};

// Fungsi-fungsi BST
BSTNode* insertBST(BSTNode* root, const std::string& id, const std::string& nama);
bool findBST(BSTNode* root, const std::string& id);
void visualizeBST(BSTNode* root, int level = 0);
void freeBST(BSTNode* root);

// Fungsi-fungsi AVL
AVLNode* insertAVL(AVLNode* root, int kandidatId, int voteCount);
int getHeight(AVLNode* node);
int getBalance(AVLNode* node);
AVLNode* rightRotate(AVLNode* y);
AVLNode* leftRotate(AVLNode* x);
void visualizeAVL(AVLNode* root, int level = 0);
void freeAVL(AVLNode* root);

// Fungsi-fungsi Graph
void addVoterToGraph(std::map<std::string, std::vector<std::string>>& graph, 
                    const std::string& idPemilih);
void addConnection(std::map<std::string, std::vector<std::string>>& graph,
                  const std::string& from, const std::string& to);
std::vector<std::string> BFS(const std::map<std::string, std::vector<std::string>>& graph,
                            const std::string& start);
std::vector<std::string> DFS(const std::map<std::string, std::vector<std::string>>& graph,
                            const std::string& start);
void visualizeGraph(const std::map<std::string, std::vector<std::string>>& graph);
std::vector<std::string> findShortestPath(const std::map<std::string, std::vector<std::string>>& graph,
                                        const std::string& start, const std::string& end);
std::vector<std::vector<std::string>> findStronglyConnectedComponents(
    const std::map<std::string, std::vector<std::string>>& graph);

// Fungsi-fungsi Hash Table
void initHashTable(std::vector<HashEntry>& table, size_t size);
size_t hashFunction(const std::string& id, size_t tableSize);
void insertHash(std::vector<HashEntry>& table, const std::string& id, const std::string& nama);
bool findHash(const std::vector<HashEntry>& table, const std::string& id);

// Fungsi-fungsi Sorting
void quickSort(std::vector<std::pair<int, int>>& arr, int low, int high);
void mergeSort(std::vector<std::pair<int, int>>& arr, int left, int right);
void heapSort(std::vector<std::pair<int, int>>& arr);
void merge(std::vector<std::pair<int, int>>& arr, int left, int mid, int right);
void heapify(std::vector<std::pair<int, int>>& arr, int n, int i);

// Fungsi-fungsi File dan Logging
void writeLog(const std::string& level, const std::string& message);
void createBackup(const std::string& sourceFile, const std::string& backupDir);
void restoreBackup(const std::string& backupFile, const std::string& targetFile);
std::string compressData(const std::string& data);
std::string decompressData(const std::string& compressed);

// Fungsi-fungsi Utility
std::string getCurrentTime();
std::string encryptData(const std::string& data, int key);
std::string decryptData(const std::string& data, int key);

#endif // VOTING_FUNCTIONS_H 