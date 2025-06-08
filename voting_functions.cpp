#include "voting_functions.h"

// Implementasi fungsi-fungsi BST
BSTNode* insertBST(BSTNode* root, const std::string& id, const std::string& nama) {
    if (!root) {
        return new BSTNode(id, nama);
    }
    
    if (id < root->idPemilih) {
        root->left = insertBST(root->left, id, nama);
    } else if (id > root->idPemilih) {
        root->right = insertBST(root->right, id, nama);
    }
    
    return root;
}

bool findBST(BSTNode* root, const std::string& id) {
    if (!root) return false;
    if (root->idPemilih == id) return true;
    if (id < root->idPemilih) return findBST(root->left, id);
    return findBST(root->right, id);
}

void visualizeBST(BSTNode* root, int level) {
    if (!root) return;
    
    visualizeBST(root->right, level + 1);
    
    std::cout << std::string(level * 4, ' ') << "└── " 
              << root->idPemilih << " (" << root->nama << ")" << std::endl;
    
    visualizeBST(root->left, level + 1);
}

void freeBST(BSTNode* root) {
    if (!root) return;
    freeBST(root->left);
    freeBST(root->right);
    delete root;
}

// Implementasi fungsi-fungsi AVL
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
    
    y->height = std::max(getHeight(y->left), getHeight(y->right)) + 1;
    x->height = std::max(getHeight(x->left), getHeight(x->right)) + 1;
    
    return x;
}

AVLNode* leftRotate(AVLNode* x) {
    AVLNode* y = x->right;
    AVLNode* T2 = y->left;
    
    y->left = x;
    x->right = T2;
    
    x->height = std::max(getHeight(x->left), getHeight(x->right)) + 1;
    y->height = std::max(getHeight(y->left), getHeight(y->right)) + 1;
    
    return y;
}

AVLNode* insertAVL(AVLNode* root, int kandidatId, int voteCount) {
    if (!root) {
        return new AVLNode(kandidatId, voteCount);
    }
    
    if (kandidatId < root->kandidatId) {
        root->left = insertAVL(root->left, kandidatId, voteCount);
    } else if (kandidatId > root->kandidatId) {
        root->right = insertAVL(root->right, kandidatId, voteCount);
    } else {
        root->voteCount += voteCount;
        return root;
    }
    
    root->height = std::max(getHeight(root->left), getHeight(root->right)) + 1;
    
    int balance = getBalance(root);
    
    // Left Left Case
    if (balance > 1 && kandidatId < root->left->kandidatId)
        return rightRotate(root);
    
    // Right Right Case
    if (balance < -1 && kandidatId > root->right->kandidatId)
        return leftRotate(root);
    
    // Left Right Case
    if (balance > 1 && kandidatId > root->left->kandidatId) {
        root->left = leftRotate(root->left);
        return rightRotate(root);
    }
    
    // Right Left Case
    if (balance < -1 && kandidatId < root->right->kandidatId) {
        root->right = rightRotate(root->right);
        return leftRotate(root);
    }
    
    return root;
}

void visualizeAVL(AVLNode* root, int level) {
    if (!root) return;
    
    visualizeAVL(root->right, level + 1);
    
    std::cout << std::string(level * 4, ' ') << "└── " 
              << "Kandidat " << root->kandidatId << " (" 
              << root->voteCount << " suara)" << std::endl;
    
    visualizeAVL(root->left, level + 1);
}

void freeAVL(AVLNode* root) {
    if (!root) return;
    freeAVL(root->left);
    freeAVL(root->right);
    delete root;
}

// Implementasi fungsi-fungsi Graph
void addVoterToGraph(std::map<std::string, std::vector<std::string>>& graph, 
                    const std::string& idPemilih) {
    if (graph.find(idPemilih) == graph.end()) {
        graph[idPemilih] = std::vector<std::string>();
    }
}

void addConnection(std::map<std::string, std::vector<std::string>>& graph,
                  const std::string& from, const std::string& to) {
    addVoterToGraph(graph, from);
    addVoterToGraph(graph, to);
    graph[from].push_back(to);
}

std::vector<std::string> BFS(const std::map<std::string, std::vector<std::string>>& graph,
                            const std::string& start) {
    std::vector<std::string> visited;
    std::queue<std::string> queue;
    std::set<std::string> seen;
    
    queue.push(start);
    seen.insert(start);
    
    while (!queue.empty()) {
        std::string current = queue.front();
        queue.pop();
        visited.push_back(current);
        
        for (const auto& neighbor : graph.at(current)) {
            if (seen.find(neighbor) == seen.end()) {
                seen.insert(neighbor);
                queue.push(neighbor);
            }
        }
    }
    
    return visited;
}

std::vector<std::string> DFS(const std::map<std::string, std::vector<std::string>>& graph,
                            const std::string& start) {
    std::vector<std::string> visited;
    std::stack<std::string> stack;
    std::set<std::string> seen;
    
    stack.push(start);
    seen.insert(start);
    
    while (!stack.empty()) {
        std::string current = stack.top();
        stack.pop();
        visited.push_back(current);
        
        for (const auto& neighbor : graph.at(current)) {
            if (seen.find(neighbor) == seen.end()) {
                seen.insert(neighbor);
                stack.push(neighbor);
            }
        }
    }
    
    return visited;
}

void visualizeGraph(const std::map<std::string, std::vector<std::string>>& graph) {
    std::map<std::string, int> nodeIndices;
    int index = 0;
    
    for (const auto& pair : graph) {
        nodeIndices[pair.first] = index++;
    }
    
    std::vector<std::vector<int>> matrix(index, std::vector<int>(index, 0));
    
    for (const auto& pair : graph) {
        int i = nodeIndices[pair.first];
        for (const auto& neighbor : pair.second) {
            int j = nodeIndices[neighbor];
            matrix[i][j] = 1;
        }
    }
    
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

std::vector<std::string> findShortestPath(
    const std::map<std::string, std::vector<std::string>>& graph,
    const std::string& start, const std::string& end) {
    
    std::map<std::string, int> distance;
    std::map<std::string, std::string> previous;
    std::set<std::string> unvisited;
    
    for (const auto& pair : graph) {
        distance[pair.first] = INT_MAX;
        unvisited.insert(pair.first);
    }
    distance[start] = 0;
    
    while (!unvisited.empty()) {
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
        
        for (const auto& neighbor : graph.at(current)) {
            if (unvisited.find(neighbor) != unvisited.end()) {
                int newDist = distance[current] + 1;
                if (newDist < distance[neighbor]) {
                    distance[neighbor] = newDist;
                    previous[neighbor] = current;
                }
            }
        }
    }
    
    std::vector<std::string> path;
    for (std::string current = end; current != start; current = previous[current]) {
        path.push_back(current);
    }
    path.push_back(start);
    std::reverse(path.begin(), path.end());
    
    return path;
}

std::vector<std::vector<std::string>> findStronglyConnectedComponents(
    const std::map<std::string, std::vector<std::string>>& graph) {
    
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
            
            for (const auto& w : graph.at(v)) {
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
    
    for (const auto& pair : graph) {
        if (index.find(pair.first) == index.end()) {
            strongConnect(pair.first);
        }
    }
    
    return components;
}

// Implementasi fungsi-fungsi Hash Table
void initHashTable(std::vector<HashEntry>& table, size_t size) {
    table.resize(size);
}

size_t hashFunction(const std::string& id, size_t tableSize) {
    size_t hash = 0;
    for (char c : id) {
        hash = (hash * 31 + c) % tableSize;
    }
    return hash;
}

void insertHash(std::vector<HashEntry>& table, const std::string& id, const std::string& nama) {
    size_t index = hashFunction(id, table.size());
    size_t originalIndex = index;
    
    do {
        if (!table[index].isOccupied) {
            table[index].idPemilih = id;
            table[index].nama = nama;
            table[index].isOccupied = true;
            return;
        }
        index = (index + 1) % table.size();
    } while (index != originalIndex);
    
    throw std::runtime_error("Hash table penuh!");
}

bool findHash(const std::vector<HashEntry>& table, const std::string& id) {
    size_t index = hashFunction(id, table.size());
    size_t originalIndex = index;
    
    do {
        if (!table[index].isOccupied) return false;
        if (table[index].idPemilih == id) return true;
        index = (index + 1) % table.size();
    } while (index != originalIndex);
    
    return false;
}

// Implementasi fungsi-fungsi Sorting
void quickSort(std::vector<std::pair<int, int>>& arr, int low, int high) {
    if (low < high) {
        int pivot = arr[high].second;
        int i = low - 1;
        
        for (int j = low; j < high; j++) {
            if (arr[j].second >= pivot) {
                i++;
                std::swap(arr[i], arr[j]);
            }
        }
        std::swap(arr[i + 1], arr[high]);
        
        int pi = i + 1;
        quickSort(arr, low, pi - 1);
        quickSort(arr, pi + 1, high);
    }
}

void merge(std::vector<std::pair<int, int>>& arr, int left, int mid, int right) {
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

void mergeSort(std::vector<std::pair<int, int>>& arr, int left, int right) {
    if (left < right) {
        int mid = left + (right - left) / 2;
        mergeSort(arr, left, mid);
        mergeSort(arr, mid + 1, right);
        merge(arr, left, mid, right);
    }
}

void heapify(std::vector<std::pair<int, int>>& arr, int n, int i) {
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

void heapSort(std::vector<std::pair<int, int>>& arr) {
    int n = arr.size();
    
    for (int i = n / 2 - 1; i >= 0; i--)
        heapify(arr, n, i);
    
    for (int i = n - 1; i > 0; i--) {
        std::swap(arr[0], arr[i]);
        heapify(arr, i, 0);
    }
}

// Implementasi fungsi-fungsi File dan Logging
void writeLog(const std::string& level, const std::string& message) {
    std::ofstream logFile("voting_log.txt", std::ios::app);
    if (logFile.is_open()) {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
        logFile << "[" << ss.str() << "] [" << level << "] " << message << std::endl;
    }
}

void createBackup(const std::string& sourceFile, const std::string& backupDir) {
    std::string timestamp = std::to_string(std::time(nullptr));
    std::string backupFile = backupDir + "/backup_" + timestamp + ".dat";
    
    std::ifstream source(sourceFile, std::ios::binary);
    std::ofstream backup(backupFile, std::ios::binary);
    
    if (!source || !backup) {
        throw std::runtime_error("Gagal membuat backup: File tidak dapat dibuka");
    }
    
    backup << source.rdbuf();
    
    std::ifstream inFile(backupFile);
    std::string content((std::istreambuf_iterator<char>(inFile)),
                       std::istreambuf_iterator<char>());
    std::string compressed = compressData(content);
    
    std::ofstream outFile(backupFile + ".compressed");
    outFile << compressed;
    
    writeLog("INFO", "Backup berhasil dibuat: " + backupFile + ".compressed");
}

void restoreBackup(const std::string& backupFile, const std::string& targetFile) {
    std::ifstream inFile(backupFile);
    std::string compressed((std::istreambuf_iterator<char>(inFile)),
                          std::istreambuf_iterator<char>());
    std::string decompressed = decompressData(compressed);
    
    std::ofstream outFile(targetFile);
    outFile << decompressed;
    
    writeLog("INFO", "Backup berhasil dipulihkan ke: " + targetFile);
}

std::string compressData(const std::string& data) {
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

std::string decompressData(const std::string& compressed) {
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

// Implementasi fungsi-fungsi Utility
std::string getCurrentTime() {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::string timeStr = std::ctime(&time);
    timeStr.pop_back(); // Hapus newline
    return timeStr;
}

std::string encryptData(const std::string& data, int key) {
    std::string encrypted;
    for (char c : data) {
        if (isalpha(c)) {
            char base = islower(c) ? 'a' : 'A';
            encrypted += char((c - base + key) % 26 + base);
        } else {
            encrypted += char(c + key % 10);
        }
    }
    return encrypted;
}

std::string decryptData(const std::string& data, int key) {
    std::string decrypted;
    for (char c : data) {
        if (isalpha(c)) {
            char base = islower(c) ? 'a' : 'A';
            decrypted += char((c - base - key + 26) % 26 + base);
        } else {
            decrypted += char(c - key % 10);
        }
    }
    return decrypted;
} 