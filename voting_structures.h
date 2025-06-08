#ifndef VOTING_STRUCTURES_H
#define VOTING_STRUCTURES_H

#include <string>
#include <vector>
#include <utility>

// Forward declarations
struct VoterNode;
struct AVLNode;
struct GraphNode;

// Hash Table for Quick Voter Lookup
class VoterHashTable {
public:
    VoterHashTable();
    void insert(const std::string& voterId, const std::string& name);
    bool find(const std::string& voterId);
private:
    static const int TABLE_SIZE = 1000;
    struct HashNode;
    HashNode* table[TABLE_SIZE];
    int hashFunction(const std::string& key);
};

// BST Implementation for Voter Management
class VoterBST {
public:
    VoterBST();
    void insert(const std::string& voterId, const std::string& name);
    bool findVoter(const std::string& voterId);
private:
    VoterNode* root;
    VoterNode* insertRecursive(VoterNode* node, const std::string& voterId, const std::string& name);
};

// AVL Tree Implementation for Vote Statistics
class VoteAVL {
public:
    VoteAVL();
    void insert(int candidateId, int votes);
    int getVotes(int candidateId);
private:
    AVLNode* root;
    int getHeight(AVLNode* node);
    int getBalance(AVLNode* node);
    AVLNode* rightRotate(AVLNode* y);
    AVLNode* leftRotate(AVLNode* x);
    AVLNode* insertRecursive(AVLNode* node, int candidateId, int votes);
};

// Graph Implementation for Social Network
class VotingGraph {
public:
    void addVoter(const std::string& voterId);
    void addConnection(const std::string& voter1, const std::string& voter2);
    std::vector<std::string> BFS(const std::string& startVoter);
    std::vector<std::string> DFS(const std::string& startVoter);
private:
    std::unordered_map<std::string, GraphNode*> nodes;
};

// Sorting Implementation
class VoteSorter {
public:
    static void quickSort(std::vector<std::pair<int, int>>& candidates, int low, int high);
private:
    static int partition(std::vector<std::pair<int, int>>& candidates, int low, int high);
};

#endif // VOTING_STRUCTURES_H 