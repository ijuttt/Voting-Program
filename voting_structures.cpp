#include <iostream>
#include <string>
#include <vector>
#include <queue>
#include <stack>
#include <unordered_map>
#include <algorithm>
using namespace std;

// BST Node for Voter ID System
struct VoterNode {
    string voterId;
    string name;
    bool hasVoted;
    VoterNode* left;
    VoterNode* right;
    
    VoterNode(string id, string n) : voterId(id), name(n), hasVoted(false), left(nullptr), right(nullptr) {}
};

// AVL Tree Node for Vote Statistics
struct AVLNode {
    int candidateId;
    int voteCount;
    int height;
    AVLNode* left;
    AVLNode* right;
    
    AVLNode(int id, int votes) : candidateId(id), voteCount(votes), height(1), left(nullptr), right(nullptr) {}
};

// Graph Node for Social Network
struct GraphNode {
    string voterId;
    vector<string> connections; // Connected voter IDs
    int candidateChoice;
    
    GraphNode(string id) : voterId(id), candidateChoice(0) {}
};

// Hash Table for Quick Voter Lookup
class VoterHashTable {
private:
    static const int TABLE_SIZE = 1000;
    struct HashNode {
        string voterId;
        string name;
        bool hasVoted;
        HashNode* next;
        
        HashNode(string id, string n) : voterId(id), name(n), hasVoted(false), next(nullptr) {}
    };
    HashNode* table[TABLE_SIZE];
    
    int hashFunction(const string& key) {
        int hash = 0;
        for (char c : key) {
            hash = (hash * 31 + c) % TABLE_SIZE;
        }
        return hash;
    }
    
public:
    VoterHashTable() {
        for (int i = 0; i < TABLE_SIZE; i++) {
            table[i] = nullptr;
        }
    }
    
    void insert(const string& voterId, const string& name) {
        int index = hashFunction(voterId);
        HashNode* newNode = new HashNode(voterId, name);
        newNode->next = table[index];
        table[index] = newNode;
    }
    
    bool find(const string& voterId) {
        int index = hashFunction(voterId);
        HashNode* current = table[index];
        while (current != nullptr) {
            if (current->voterId == voterId) {
                return true;
            }
            current = current->next;
        }
        return false;
    }
};

// BST Implementation for Voter Management
class VoterBST {
private:
    VoterNode* root;
    
    VoterNode* insertRecursive(VoterNode* node, const string& voterId, const string& name) {
        if (node == nullptr) {
            return new VoterNode(voterId, name);
        }
        
        if (voterId < node->voterId) {
            node->left = insertRecursive(node->left, voterId, name);
        } else if (voterId > node->voterId) {
            node->right = insertRecursive(node->right, voterId, name);
        }
        
        return node;
    }
    
public:
    VoterBST() : root(nullptr) {}
    
    void insert(const string& voterId, const string& name) {
        root = insertRecursive(root, voterId, name);
    }
    
    bool findVoter(const string& voterId) {
        VoterNode* current = root;
        while (current != nullptr) {
            if (voterId == current->voterId) {
                return true;
            }
            if (voterId < current->voterId) {
                current = current->left;
            } else {
                current = current->right;
            }
        }
        return false;
    }
};

// AVL Tree Implementation for Vote Statistics
class VoteAVL {
private:
    AVLNode* root;
    
    int getHeight(AVLNode* node) {
        return node ? node->height : 0;
    }
    
    int getBalance(AVLNode* node) {
        return node ? getHeight(node->left) - getHeight(node->right) : 0;
    }
    
    AVLNode* rightRotate(AVLNode* y) {
        AVLNode* x = y->left;
        AVLNode* T2 = x->right;
        
        x->right = y;
        y->left = T2;
        
        y->height = max(getHeight(y->left), getHeight(y->right)) + 1;
        x->height = max(getHeight(x->left), getHeight(x->right)) + 1;
        
        return x;
    }
    
    AVLNode* leftRotate(AVLNode* x) {
        AVLNode* y = x->right;
        AVLNode* T2 = y->left;
        
        y->left = x;
        x->right = T2;
        
        x->height = max(getHeight(x->left), getHeight(x->right)) + 1;
        y->height = max(getHeight(y->left), getHeight(y->right)) + 1;
        
        return y;
    }
    
    AVLNode* insertRecursive(AVLNode* node, int candidateId, int votes) {
        if (node == nullptr) {
            return new AVLNode(candidateId, votes);
        }
        
        if (candidateId < node->candidateId) {
            node->left = insertRecursive(node->left, candidateId, votes);
        } else if (candidateId > node->candidateId) {
            node->right = insertRecursive(node->right, candidateId, votes);
        } else {
            node->voteCount += votes;
            return node;
        }
        
        node->height = max(getHeight(node->left), getHeight(node->right)) + 1;
        
        int balance = getBalance(node);
        
        // Left Left Case
        if (balance > 1 && candidateId < node->left->candidateId) {
            return rightRotate(node);
        }
        
        // Right Right Case
        if (balance < -1 && candidateId > node->right->candidateId) {
            return leftRotate(node);
        }
        
        // Left Right Case
        if (balance > 1 && candidateId > node->left->candidateId) {
            node->left = leftRotate(node->left);
            return rightRotate(node);
        }
        
        // Right Left Case
        if (balance < -1 && candidateId < node->right->candidateId) {
            node->right = rightRotate(node->right);
            return leftRotate(node);
        }
        
        return node;
    }
    
public:
    VoteAVL() : root(nullptr) {}
    
    void insert(int candidateId, int votes) {
        root = insertRecursive(root, candidateId, votes);
    }
    
    int getVotes(int candidateId) {
        AVLNode* current = root;
        while (current != nullptr) {
            if (candidateId == current->candidateId) {
                return current->voteCount;
            }
            if (candidateId < current->candidateId) {
                current = current->left;
            } else {
                current = current->right;
            }
        }
        return 0;
    }
};

// Graph Implementation for Social Network
class VotingGraph {
private:
    unordered_map<string, GraphNode*> nodes;
    
public:
    void addVoter(const string& voterId) {
        if (nodes.find(voterId) == nodes.end()) {
            nodes[voterId] = new GraphNode(voterId);
        }
    }
    
    void addConnection(const string& voter1, const string& voter2) {
        if (nodes.find(voter1) != nodes.end() && nodes.find(voter2) != nodes.end()) {
            nodes[voter1]->connections.push_back(voter2);
            nodes[voter2]->connections.push_back(voter1);
        }
    }
    
    vector<string> BFS(const string& startVoter) {
        vector<string> visited;
        if (nodes.find(startVoter) == nodes.end()) return visited;
        
        queue<string> q;
        unordered_map<string, bool> visitedMap;
        
        q.push(startVoter);
        visitedMap[startVoter] = true;
        
        while (!q.empty()) {
            string current = q.front();
            q.pop();
            visited.push_back(current);
            
            for (const string& neighbor : nodes[current]->connections) {
                if (!visitedMap[neighbor]) {
                    visitedMap[neighbor] = true;
                    q.push(neighbor);
                }
            }
        }
        
        return visited;
    }
    
    vector<string> DFS(const string& startVoter) {
        vector<string> visited;
        if (nodes.find(startVoter) == nodes.end()) return visited;
        
        stack<string> s;
        unordered_map<string, bool> visitedMap;
        
        s.push(startVoter);
        
        while (!s.empty()) {
            string current = s.top();
            s.pop();
            
            if (!visitedMap[current]) {
                visited.push_back(current);
                visitedMap[current] = true;
                
                for (const string& neighbor : nodes[current]->connections) {
                    if (!visitedMap[neighbor]) {
                        s.push(neighbor);
                    }
                }
            }
        }
        
        return visited;
    }
};

// Sorting Implementation
class VoteSorter {
public:
    // Quick Sort implementation for sorting candidates by vote count
    static void quickSort(vector<pair<int, int>>& candidates, int low, int high) {
        if (low < high) {
            int pi = partition(candidates, low, high);
            quickSort(candidates, low, pi - 1);
            quickSort(candidates, pi + 1, high);
        }
    }
    
private:
    static int partition(vector<pair<int, int>>& candidates, int low, int high) {
        int pivot = candidates[high].second;
        int i = low - 1;
        
        for (int j = low; j < high; j++) {
            if (candidates[j].second > pivot) {
                i++;
                swap(candidates[i], candidates[j]);
            }
        }
        swap(candidates[i + 1], candidates[high]);
        return i + 1;
    }
}; 