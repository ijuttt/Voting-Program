#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <map>
#include <iterator>
#include <random>
#include <ctime>
#include <stdexcept>
#include <iomanip>
#include <queue>
#include <stack>
#include <unordered_map>
#include <unordered_set>
#include <functional>
#include <list>
#include <chrono>
#include <set> 
#include <thread>
#include <windows.h>
#include <cmath>

using namespace std;

// Forward declarations
class AdminHashTable;
class AVLTree;
class VoterHashTable;

// Forward declarations for utility functions
string decryptData(const string& data, int key);
void logError(const string& msg);

// Global variables
extern AdminHashTable adminTable;
extern AVLTree kandidatTree;
extern VoterHashTable voterTable;

// Konstanta untuk karakter Unicode
const char BLOCK_CHAR[] = "\u2588"; // Karakter █ dalam UTF-8

// Fungsi agar program bisa menampilkan karakter unicode (simbol)
void setupConsole() {
    #ifdef _WIN32
    // Set console ke mode UTF-8
    SetConsoleOutputCP(CP_UTF8);
    // Enable virtual terminal processing untuk ANSI escape sequences
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;
    GetConsoleMode(hOut, &dwMode);
    SetConsoleMode(hOut, dwMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    #endif
}

// Struktur untuk data kandidat
struct Kandidat {
    int nomor;
    string nama;
    string partai;
    int suara;
    
    Kandidat() : nomor(0), suara(0) {}
    Kandidat(int n, const string& nm, const string& p) : nomor(n), nama(nm), partai(p), suara(0) {}
    
    void tampilkan() const {
        cout << "Nomor Kandidat : " << nomor << "\n";
        cout << "Nama Kandidat  : " << nama << "\n";
        cout << "Partai         : " << partai << "\n";
        cout << string(40, '-') << "\n";
    }
};

// Struktur untuk data vote
struct VoteData {
    chrono::system_clock::time_point waktu;
    string nik;
    int pilihan;
    bool isValid;
    string namaVoter;

    // Constructor
    VoteData() : pilihan(0), isValid(false) {}
    
    VoteData(const string& nama, const string& n, int p, 
             const chrono::system_clock::time_point& w, bool valid) 
        : namaVoter(nama), nik(n), pilihan(p), waktu(w), isValid(valid) {}

    // Method untuk konversi ke string
    string toString() const {
        time_t time_now = chrono::system_clock::to_time_t(waktu);
        tm* timeinfo = localtime(&time_now);
        char buffer[20];
        strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeinfo);
        return namaVoter + "|" + nik + "|" + to_string(pilihan) + "|" + buffer + "|" + (isValid ? "1" : "0");
    }

    // Method untuk parse dari string
    static VoteData fromString(const string& data) {
        VoteData vote;
        size_t pos1 = data.find('|');
        size_t pos2 = data.find('|', pos1 + 1);
        size_t pos3 = data.find('|', pos2 + 1);
        size_t pos4 = data.find('|', pos3 + 1);
        if (pos1 != string::npos && pos2 != string::npos && pos3 != string::npos && pos4 != string::npos) {
            vote.namaVoter = data.substr(0, pos1);
            vote.nik = data.substr(pos1 + 1, pos2 - pos1 - 1);
            vote.pilihan = stoi(data.substr(pos2 + 1, pos3 - pos2 - 1));
            
            // Parse waktu string ke time_point
            string waktuStr = data.substr(pos3 + 1, pos4 - pos3 - 1);
            struct tm tm = {};
            stringstream ss(waktuStr);
            ss >> get_time(&tm, "%Y-%m-%d %H:%M:%S");
            time_t time_now = mktime(&tm);
            vote.waktu = chrono::system_clock::from_time_t(time_now);
            
            vote.isValid = (data.substr(pos4 + 1) == "1");
        }
        return vote;
    }

    string getWaktuStr() const {
        auto time = chrono::system_clock::to_time_t(waktu);
        stringstream ss;
        ss << put_time(localtime(&time), "%Y-%m-%d %H:%M:%S");
        return ss.str();
    }
};

// Struktur untuk node rantai voting
struct VoteNode {
    chrono::system_clock::time_point waktu;
    string nik;
    int pilihan;
    vector<VoteNode*> nextVotes;
    bool visited;

    VoteNode(const chrono::system_clock::time_point& w, 
            const string& n, int p) 
        : waktu(w), nik(n), pilihan(p), visited(false) {}

    // Konversi ke VoteData
    VoteData toVoteData() const {
        return VoteData("", nik, pilihan, waktu, true);
    }

    // Method untuk mendapatkan string waktu
    string getWaktuStr() const {
        auto time = chrono::system_clock::to_time_t(waktu);
        stringstream ss;
        ss << put_time(localtime(&time), "%Y-%m-%d %H:%M:%S");
        return ss.str();
    }
};

// Graph untuk analisis rantai voting
class VoteChainGraph {
private:
    // Hash function untuk time_point
    struct TimePointHash {
        size_t operator()(const chrono::system_clock::time_point& tp) const {
            return chrono::system_clock::to_time_t(tp);
        }
    };

    unordered_map<chrono::system_clock::time_point, VoteNode*, TimePointHash> voteMap;
    vector<VoteNode*> voteNodes;
    const chrono::minutes MAX_CHAIN_TIME = chrono::minutes(5);

    // Unified DFS utility function
    void DFSUtil(VoteNode* node, unordered_set<VoteNode*>& visited, vector<VoteNode*>& result) {
        visited.insert(node);
        result.push_back(node);
        
        for (VoteNode* next : node->nextVotes) {
            if (visited.find(next) == visited.end()) {
                DFSUtil(next, visited, result);
            }
        }
    }

    // Helper for VoteData pattern collection
    void DFSUtilForPattern(VoteNode* node, vector<VoteNode*>& visited, vector<VoteData>& pattern) {
        node->visited = true;
        visited.push_back(node);
        pattern.push_back(node->toVoteData());
        
        for (VoteNode* neighbor : node->nextVotes) {
            if (!neighbor->visited) {
                DFSUtilForPattern(neighbor, visited, pattern);
            }
        }
    }

public:
    VoteChainGraph() = default;

    ~VoteChainGraph() {
        for (auto node : voteNodes) {
            delete node;
        }
    }

    void addVote(const chrono::system_clock::time_point& waktu, 
                 const string& namaVoter, int pilihan) {
        VoteNode* newNode = new VoteNode(waktu, namaVoter, pilihan);
        voteMap[waktu] = newNode;
        voteNodes.push_back(newNode);

        // Hubungkan dengan vote sebelumnya (dalam rentang 5 menit)
        for (auto& pair : voteMap) {
            if (pair.first != waktu) {
                auto diff = chrono::duration_cast<chrono::seconds>(waktu - pair.first);
                if (abs(diff.count()) <= 300) { // 300 detik = 5 menit
                    newNode->nextVotes.push_back(pair.second);
                    pair.second->nextVotes.push_back(newNode);
                }
            }
        }
    }

    vector<VoteNode*> BFS(const chrono::system_clock::time_point& startTime) {
        vector<VoteNode*> result;
        if (voteMap.find(startTime) == voteMap.end()) return result;

        unordered_set<VoteNode*> visited;
        queue<VoteNode*> q;
        VoteNode* start = voteMap[startTime];

        visited.insert(start);
        q.push(start);

        while (!q.empty()) {
            VoteNode* current = q.front();
            q.pop();
            result.push_back(current);

            for (VoteNode* next : current->nextVotes) {
                if (visited.find(next) == visited.end()) {
                    visited.insert(next);
                    q.push(next);
                }
            }
        }
        return result;
    }

    // Unified DFS method
    vector<VoteNode*> DFS(const chrono::system_clock::time_point& startTime) {
        vector<VoteNode*> result;
        if (voteMap.find(startTime) == voteMap.end()) return result;

        unordered_set<VoteNode*> visited;
        VoteNode* start = voteMap[startTime];
        DFSUtil(start, visited, result);
        return result;
    }

    // Method untuk mendapatkan statistik rantai voting
    struct VotingStats {
        int totalVotes;
        int maxChainLength;
        int minChainLength;
        double avgChainLength;
        map<int, int> votesPerCandidate;
    };

    VotingStats getVotingStats() {
        VotingStats stats = {0, 0, INT_MAX, 0.0, {}};
        unordered_set<VoteNode*> visited;
        int chainCount = 0;  // Menghitung jumlah rantai
        
        for (const auto& pair : voteMap) {
            if (visited.find(pair.second) == visited.end()) {
                vector<VoteNode*> chain = DFS(pair.first);
                chainCount++;  // Increment jumlah rantai
                stats.maxChainLength = max(stats.maxChainLength, (int)chain.size());
                stats.minChainLength = min(stats.minChainLength, (int)chain.size());
                
                // Hitung vote per kandidat
                for (const auto& node : chain) {
                    stats.votesPerCandidate[node->pilihan]++;
                }
            }
        }
        
        // Total votes adalah jumlah node unik dalam graph
        stats.totalVotes = voteMap.size();
        
        if (stats.totalVotes > 0) {
            stats.avgChainLength = (double)stats.totalVotes / chainCount;
        }
        if (stats.minChainLength == INT_MAX) stats.minChainLength = 0;
        
        return stats;
    }

    void addVote(const VoteData& vote) {
        addVote(vote.waktu, vote.nik, vote.pilihan);
    }

    vector<vector<VoteData>> analyzeVotingChain() {
        vector<vector<VoteData>> chains;
        vector<VoteNode*> visited;
        
        // Reset visited status
        for (auto& [_, node] : voteMap) {
            node->visited = false;
        }
        
        // Mulai dari setiap node yang belum dikunjungi
        for (auto& [time, node] : voteMap) {
            if (!node->visited) {
                vector<VoteData> currentChain;
                queue<VoteNode*> q;
                q.push(node);
                node->visited = true;
                
                while (!q.empty()) {
                    VoteNode* current = q.front();
                    q.pop();
                    
                    // Tambahkan data voting ke chain menggunakan toVoteData
                    currentChain.push_back(current->toVoteData());
                    
                    // Tambahkan tetangga ke queue
                    for (VoteNode* neighbor : current->nextVotes) {
                        if (!neighbor->visited) {
                            q.push(neighbor);
                            neighbor->visited = true;
                        }
                    }
                }
                
                if (!currentChain.empty()) {
                    chains.push_back(currentChain);
                }
            }
        }
        
        // Urutkan chains berdasarkan panjang
        sort(chains.begin(), chains.end(),
             [](const vector<VoteData>& a, const vector<VoteData>& b) {
                 return a.size() > b.size();
             });
        
        return chains;
    }

    vector<vector<VoteData>> getVotingPatterns() {
        vector<vector<VoteData>> patterns;
        vector<VoteNode*> visited;
        
        // Reset visited status
        for (auto& [_, node] : voteMap) {
            node->visited = false;
        }
        
        // Mulai dari setiap node yang belum dikunjungi
        for (auto& [time, node] : voteMap) {
            if (!node->visited) {
                vector<VoteData> currentPattern;
                DFSUtilForPattern(node, visited, currentPattern);
                if (!currentPattern.empty()) {
                    patterns.push_back(currentPattern);
                }
            }
        }
        
        return patterns;
    }
};

// BST Node untuk menyimpan data kandidat
struct BSTNode {
    Kandidat data;
    BSTNode* left;
    BSTNode* right;
    int height; // Untuk AVL Tree

    BSTNode(const Kandidat& k) : data(k), left(nullptr), right(nullptr), height(1) {}
};

// AVL Tree untuk mengelola kandidat
class AVLTree {
private:
    BSTNode* root;

    int getHeight(BSTNode* node) {
        return node ? node->height : 0;
    }

    int getBalance(BSTNode* node) {
        return node ? getHeight(node->left) - getHeight(node->right) : 0;
    }

    BSTNode* rightRotate(BSTNode* y) {
        BSTNode* x = y->left;
        BSTNode* T2 = x->right;

        x->right = y;
        y->left = T2;

        y->height = max(getHeight(y->left), getHeight(y->right)) + 1;
        x->height = max(getHeight(x->left), getHeight(x->right)) + 1;

        return x;
    }

    BSTNode* leftRotate(BSTNode* x) {
        BSTNode* y = x->right;
        BSTNode* T2 = y->left;

        y->left = x;
        x->right = T2;

        x->height = max(getHeight(x->left), getHeight(x->right)) + 1;
        y->height = max(getHeight(y->left), getHeight(y->right)) + 1;

        return y;
    }

    BSTNode* insert(BSTNode* node, const Kandidat& k) {
        if (!node) return new BSTNode(k);

        if (k.nomor < node->data.nomor)
            node->left = insert(node->left, k);
        else if (k.nomor > node->data.nomor)
            node->right = insert(node->right, k);
        else
            return node;

        node->height = 1 + max(getHeight(node->left), getHeight(node->right));

        int balance = getBalance(node);

        // Left Left Case
        if (balance > 1 && k.nomor < node->left->data.nomor)
            return rightRotate(node);

        // Right Right Case
        if (balance < -1 && k.nomor > node->right->data.nomor)
            return leftRotate(node);

        // Left Right Case
        if (balance > 1 && k.nomor > node->left->data.nomor) {
            node->left = leftRotate(node->left);
            return rightRotate(node);
        }

        // Right Left Case
        if (balance < -1 && k.nomor < node->right->data.nomor) {
            node->right = rightRotate(node->right);
            return leftRotate(node);
        }

        return node;
    }

    void inorderTraversal(BSTNode* node, vector<Kandidat>& result) {
        if (node) {
            inorderTraversal(node->left, result);
            result.push_back(node->data);
            inorderTraversal(node->right, result);
        }
    }

    // Search for candidate by nomor
    BSTNode* searchByNomor(BSTNode* node, int nomor) {
        if (!node || node->data.nomor == nomor) {
            return node;
        }
        
        if (nomor < node->data.nomor) {
            return searchByNomor(node->left, nomor);
        }
        
        return searchByNomor(node->right, nomor);
    }

public:
    AVLTree() : root(nullptr) {}

    void insert(const Kandidat& k) {
        root = insert(root, k);
    }

    vector<Kandidat> getSortedKandidat() {
        vector<Kandidat> result;
        inorderTraversal(root, result);
        return result;
    }

    // Public search method
    Kandidat* searchByNomor(int nomor) {
        BSTNode* node = searchByNomor(root, nomor);
        return node ? &(node->data) : nullptr;
    }
};

// Enum for vote status
enum class VoteStatus {
    NotFound,
    NotVoted,
    AlreadyVoted
};

// Modifikasi Hash Table untuk validasi NIK
class VoterHashTable {
private:
    unordered_map<string, bool> voters;  // NIK -> hasVoted

public:
    VoterHashTable() {
        // Load voting data from votes.dat and keys.dat
        ifstream voteFile("votes.dat");
        ifstream keyFile("keys.dat");
        if (voteFile.is_open() && keyFile.is_open()) {
            string encryptedData;
            int key;
            while (getline(voteFile, encryptedData) && keyFile >> key) {
                try {
                    string decryptedData = decryptData(encryptedData, key);
                    VoteData vote = VoteData::fromString(decryptedData);
                    if (vote.isValid) {
                        voters[vote.nik] = true;  // Mark as voted
                    }
                } catch (const exception& e) {
                    logError("Error loading vote data: " + string(e.what()));
                }
            }
            voteFile.close();
            keyFile.close();
        }
    }

    ~VoterHashTable() {
        // No need to save to separate files anymore
    }

    VoteStatus hasVoted(const string& nik) {
        auto it = voters.find(nik);
        if (it == voters.end()) {
            // NIK not found, auto-register
            voters[nik] = false;
        cout << "\n" << string(50, '-') << "\n";
            cout << "NIK " << nik << " berhasil didaftarkan secara otomatis.\n";
        cout << string(50, '-') << "\n";
            return VoteStatus::NotVoted;
    }

        if (it->second) {
                cout << "\n" << string(50,'*') << "\n";
            cout << "ERROR: NIK " << nik << " sudah melakukan voting!\n";
                cout << string(50,'*') << "\n";
            return VoteStatus::AlreadyVoted;
        }
        
        return VoteStatus::NotVoted;
    }

    void markAsVoted(const string& nik) {
        auto it = voters.find(nik);
        if (it == voters.end()) {
            cout << "\n" << string(50,'*') << "\n";
            cout << "NIK " << nik << " tidak ditemukan untuk ditandai sebagai sudah voting!\n";
            cout << string(50,'*') << "\n";
            return;
        }
        
        if (it->second) {
                    cout << "\n" << string(50,'*') << "\n";
                    cout << "NIK " << nik << " sudah ditandai sebagai sudah voting!\n";
                    cout << string(50,'*') << "\n";
                } else {
            it->second = true;
            system("cls");
                    cout << "\n" << string(50, '-') << "\n";
                    cout << "NIK " << nik << " berhasil ditandai sebagai sudah voting.\n";
                    cout << string(50, '-') << "\n";
                    this_thread::sleep_for(chrono::seconds(1));
                }
    }
};

// Struktur untuk data admin
struct AdminData {
    string username;
    string password;
    bool isSuperAdmin;
    
    AdminData(string u = "", string p = "", bool super = false) 
        : username(u), password(p), isSuperAdmin(super) {}
};

// Hash table untuk menyimpan data admin
class AdminHashTable {
private:
    static const int TABLE_SIZE = 10007;
    vector<list<AdminData>> table;

    size_t hashFunction(const string& username) {
        size_t hash = 0;
        for (char c : username) {
            hash = (hash * 31 + c) % TABLE_SIZE;
        }
        return hash;
    }

public:
    AdminHashTable() : table(TABLE_SIZE) {
        // Tambah admin default
        insert("admin", "admin123", true);
    }

    void insert(const string& username, const string& password, bool isSuperAdmin = false) {
        size_t index = hashFunction(username);
        table[index].push_back({username, password, isSuperAdmin});
    }

    bool verifyAdmin(const string& username, const string& password) {
        size_t index = hashFunction(username);
        for (const auto& admin : table[index]) {
            if (admin.username == username && admin.password == password) {
                return true;
            }
        }
        return false;
    }

    bool isSuperAdmin(const string& username) {
        size_t index = hashFunction(username);
        for (const auto& admin : table[index]) {
            if (admin.username == username) {
                return admin.isSuperAdmin;
            }
        }
        return false;
    }
};

// Deklarasi variabel global
extern AVLTree kandidatTree;
extern VoteChainGraph voteChain;
extern VoterHashTable voterTable;

// Definisi variabel global
AVLTree kandidatTree;
VoteChainGraph voteChain;
VoterHashTable voterTable;

// Global instance untuk admin table
AdminHashTable adminTable;

// Tambahkan deklarasi fungsi-fungsi ini setelah deklarasi struct/class dan sebelum implementasi fungsi
void tungguInput();
bool verifikasiAdmin(const string& username, const string& password);
void menuAdmin(vector<Kandidat>& kandidat);
void verifikasiDataVoting();
void auditTrailVoting();
void tampilkanRantaiVoting();  // Tambahkan deklarasi ini
void tambahKandidat(vector<Kandidat>& kandidat);
void hapusKandidat(vector<Kandidat>& kandidat);
void tampilkanDashboardAdmin(const vector<Kandidat>& kandidat);
void tampilkanDetailVoting();
void setColor(int color);
void tampilkanMenu();
bool isValidNIK(const string& nik);
bool safeWriteToFile(const string& filename, const string& content);
void logError(const string& msg);

// Implementasi fungsi-fungsi
void tungguInput() {
    cout << "\nTekan Enter untuk kembali...";
    cin.clear();  // Clear any error flags
    cin.sync();   // Clear input buffer
    cin.ignore(numeric_limits<streamsize>::max(), '\n');  // Clear any remaining input
    cin.get();    // Wait for single Enter press
}

bool verifikasiAdmin(const string& username, const string& password) {
    return adminTable.verifyAdmin(username, password);
}

void menuAdmin(vector<Kandidat>& kandidat) {
    // Verifikasi login admin terlebih dahulu
    system("cls");
    string username, password;
    cout << "\n" << string(60, '=') << "\n";
    cout << "                      LOGIN ADMIN                    \n";
    cout << string(60, '=') << "\n\n";
    
    cout << "Username: ";
    cin >> username;
    cout << "Password: ";
    cin >> password;
    
    if (!verifikasiAdmin(username, password)) {
        cout << "\n" << string(50, '*') << "\n";
        cout << "Akses ditolak! Username atau password salah.\n";
        cout << string(50, '*') << "\n";
        tungguInput();
        return;
    }

    cout << "\n" << string(50, '-') << "\n";
    cout << "Login berhasil! Selamat datang, " << username << ".\n";
    cout << string(50, '-') << "\n";
    this_thread::sleep_for(chrono::seconds(1));

    while (true) {
        system("cls");
        cout << "\n" << string(80, '=') << "\n";
        cout << "                      MENU ADMIN PEMILU                    \n";
        cout << string(80, '=') << "\n\n";
        
        cout << "1. Dashboard\n";
        cout << "   - Ringkasan Statistik\n";
        cout << "   - Distribusi Suara Per Kandidat\n";
        cout << "   - Verifikasi Data Voting\n";
        cout << "2. Verifikasi & Audit\n";
        cout << "   - Verifikasi Data Voting\n";
        cout << "   - Audit Trail Voting\n";
        cout << "   - Analisis Rantai Voting\n";
        cout << "   - Detail Record Voting\n";
        cout << "3. Kembali ke Menu Utama\n\n";
        
        cout << "Pilihan: ";
        string choice;
        cin >> choice;
        
        if (choice == "1") {
            tampilkanDashboardAdmin(kandidat);
        }
        else if (choice == "2") {
            while (true) {
                system("cls");
                cout << "\n" << string(80, '=') << "\n";
                cout << "                      VERIFIKASI & AUDIT                    \n";
                cout << string(80, '=') << "\n\n";
                
                cout << "1. Verifikasi Data Voting\n";
                cout << "2. Audit Trail Voting\n";
                cout << "3. Analisis Rantai Voting\n";
                cout << "4. Detail Record Voting\n";
                cout << "5. Kembali ke Menu Admin\n\n";
                cout << "Pilihan: ";
                
                string subChoice;
                cin >> subChoice;
                
                if (subChoice == "1") {
                    verifikasiDataVoting();
                }
                else if (subChoice == "2") {
                    auditTrailVoting();
                }
                else if (subChoice == "3") {
                    tampilkanRantaiVoting();
                }
                else if (subChoice == "4") {
                    tampilkanDetailVoting();
                }
                else if (subChoice == "5") {
                    break;
                }
                else {
                    cout << "\nPilihan tidak valid!\n";
                    this_thread::sleep_for(chrono::seconds(1));
                }
            }
        }
        else if (choice == "3") {
            return;
        }
        else {
            cout << "\nPilihan tidak valid!\n";
            this_thread::sleep_for(chrono::seconds(1));
        }
    }
}

// Fungsi enkripsi sederhana (Caesar Cipher dengan kunci acak)
string encryptData(const string& data, int key) {
    string encrypted = "";
    for (char c : data) {
        if (isalpha(c)) {
            char base = islower(c) ? 'a' : 'A';
            encrypted += char((c - base + key) % 26 + base);
        } else {
            encrypted += char(c + key % 10); // Untuk angka dan karakter lain
        }
    }
    return encrypted;
}
// Fungsi dekripsi sederhana
string decryptData(const string& data, int key) {
    string decrypted = "";
    for (char c : data) {
        if (isalpha(c)) {
            char base = islower(c) ? 'a' : 'A';
            decrypted += char((c - base - key + 26) % 26 + base);
        } else {
            decrypted += char(c - key % 10); // Untuk angka dan karakter lain
        }
    }
    return decrypted;
}
// Fungsi untuk mendapatkan waktu saat ini
chrono::system_clock::time_point getCurrentTime() {
    return chrono::system_clock::now();
}
// Fungsi untuk membaca kandidat dari file
vector<Kandidat> bacaKandidat() {
    vector<Kandidat> kandidat;
    ifstream file("kandidat.txt");
    try {
        if (!file.is_open()) {
            throw runtime_error("File kandidat.txt tidak dapat dibuka!");
        }
        string line;
        while (getline(file, line)) {
            if (!line.empty()) {
                // Format: nomor|nama|partai
                    size_t pos1 = line.find('|');
                    size_t pos2 = line.find('|', pos1 + 1);
                    if (pos1 != string::npos && pos2 != string::npos) {
                        Kandidat k;
                        k.nomor = stoi(line.substr(0, pos1));  //stoi untuk konversi string ke int
                        k.nama = line.substr(pos1 + 1, pos2 - pos1 - 1);
                        k.partai = line.substr(pos2 + 1);
                        kandidat.push_back(k);
                        // Tambahkan ke AVL Tree
                        kandidatTree.insert(k);
                }
            }
        }
        file.close();
    }
    catch (const exception& e) {
        string errorMsg = "Error membaca kandidat: " + string(e.what());
        logError(errorMsg);
        cout << "Error: " << e.what() << endl;
        // Jika file tidak ada, buat kandidat default
        kandidat = {
            Kandidat(1, "Joko Widodo", "PDI-P"),
            Kandidat(2, "Prabowo Subianto", "Gerindra"), 
            Kandidat(3, "Anies Baswedan", "NasDem")
        };
        // Simpan ke file dan tambahkan ke struktur data
        ofstream outFile("kandidat.txt");
        for (const auto& k : kandidat) {
            outFile << k.nomor << "|" << k.nama << "|" << k.partai << "\n";
            kandidatTree.insert(k);
        }
        outFile.close();
    }
    return kandidat;
}
// Modifikasi fungsi simpanVote
void simpanVote(const string& namaVoter, const string& nik, int pilihan, 
                const chrono::system_clock::time_point& waktu) {
    try {
        // Generate kunci enkripsi acak
        random_device rd;
        mt19937 gen(rd());
        uniform_int_distribution<> dis(5, 20);
        int key = dis(gen);
        
        // Buat object VoteData dengan isValid = true karena sudah dikonfirmasi
        VoteData vote(namaVoter, nik, pilihan, waktu, true);
        string data = vote.toString();
        string encryptedData = encryptData(data, key);
        
        // Simpan ke file dengan safe writing
        if (!safeWriteToFile("votes.dat", encryptedData + "\n")) {
            throw runtime_error("Tidak dapat menyimpan data vote!");
        }
        if (!safeWriteToFile("keys.dat", to_string(key) + "\n")) {
            throw runtime_error("Tidak dapat menyimpan kunci enkripsi!");
        }
        
        cout << "\n" << string(50, '=') << "\n";
        cout << "               VOTE BERHASIL DISIMPAN" << "\n";
        cout << string(50, '=') << "\n";
        cout << "Terima kasih " << namaVoter << "!\n";
        cout << "Vote Anda telah tersimpan dengan enkripsi.\n";
        cout << "Waktu voting: " << vote.getWaktuStr() << "\n";
        cout << "Data Anda aman dan terlindungi.\n";
        cout << string(50, '=') << "\n";
    }
    catch (const exception& e) {
        string errorMsg = "Error saat menyimpan vote: " + string(e.what());
        logError(errorMsg);
        cout << errorMsg << endl;
    }
}
// Modifikasi fungsi bacaSemuaVotes untuk memfilter voting yang sah
pair<map<int, int>, vector<VoteData>> bacaSemuaVotes() {
    map<int, int> hasilVote;
    vector<VoteData> semuaVote;
    ifstream voteFile("votes.dat");
    ifstream keyFile("keys.dat");
    if (!voteFile.is_open() || !keyFile.is_open()) {
        cout << "File vote tidak ditemukan atau kosong.\n";
        return {hasilVote, semuaVote};
    }
    string encryptedData;
    int key;
    while (getline(voteFile, encryptedData) && keyFile >> key) {
        string decryptedData = decryptData(encryptedData, key);
        VoteData vote = VoteData::fromString(decryptedData);
        // Hanya tambahkan vote yang sah
        if (vote.isValid) {
            semuaVote.push_back(vote);
            hasilVote[vote.pilihan]++;
        }
    }
    voteFile.close();
    keyFile.close();
    return {hasilVote, semuaVote};
}


// Removed quickSort and mergeSort functions - using std::sort with lambda instead
void merge(vector<Kandidat>& arr, int left, int mid, int right) {
    int n1 = mid - left + 1;
    int n2 = right - mid;

    vector<Kandidat> L(n1), R(n2);
    for (int i = 0; i < n1; ++i) L[i] = arr[left + i];
    for (int j = 0; j < n2; ++j) R[j] = arr[mid + 1 + j];

    int i = 0, j = 0, k = left;
    while (i < n1 && j < n2) {
        if (L[i].nomor <= R[j].nomor) arr[k++] = L[i++];
        else arr[k++] = R[j++];
    }

    while (i < n1) arr[k++] = L[i++];
    while (j < n2) arr[k++] = R[j++];
}

void mergeSort(vector<Kandidat>& arr, int left, int right) {
    if (left < right) {
        int mid = left + (right - left) / 2;
        mergeSort(arr, left, mid);
        mergeSort(arr, mid + 1, right);
        merge(arr, left, mid, right);
    }
}

void tampilkanKandidat(vector<Kandidat>& kandidat, int sortMethod = 0) {
    system("cls");
    cout << "\n" << string(60, '=') << "\n";
    cout << "         DAFTAR KANDIDAT PRESIDEN 2029         \n";
    cout << string(60, '=') << "\n\n";

    // Ganti STL sort dengan Merge Sort
    mergeSort(kandidat, 0, kandidat.size() - 1);
    cout << "Menggunakan Merge Sort\n";

    for_each(kandidat.begin(), kandidat.end(),
             [](const Kandidat& k) {
                 k.tampilkan();
             });
    cout << string(60, '=') << "\n";
}
// Modifikasi fungsi lakukanVoting
void lakukanVoting(const vector<Kandidat>& kandidat) {
    system("cls");
    cout << "\n" << string(60, '=') << "\n";
    cout << "                        VOTING SECTION                  \n";
    cout << string(60, '=') << "\n";
    
    // Input nama voter
    string namaVoter;
    cout << "Masukkan nama lengkap Anda: ";
    // Clear any remaining newline from previous input
    if (cin.peek() == '\n') {
        cin.ignore();
    }
    getline(cin, namaVoter);
    if (namaVoter.empty()) {
        cout << "\n" << string(50, '*') << "\n";
        cout << "ERROR: Nama tidak boleh kosong!\n";
        cout << string(50, '*') << "\n";
        return;
    }

    // Input NIK
    string nik;
    cout << "Masukkan NIK Anda (16 digit): ";
    cin >> nik;
    
    // Validasi panjang NIK terlebih dahulu
    if (nik.length() != 16) {
        cout << "\n" << string(50, '*') << "\n";
        cout << "ERROR: NIK harus terdiri dari 16 digit angka!\n";
        cout << "NIK yang dimasukkan: " << nik << " (" << nik.length() << " digit)\n";
        cout << string(50, '*') << "\n";
        return;
    }

    // Validasi NIK harus angka
    if (!all_of(nik.begin(), nik.end(), ::isdigit)) {
        cout << "\n" << string(50, '*') << "\n";
        cout << "ERROR: NIK harus terdiri dari angka saja!\n";
        cout << "NIK yang dimasukkan: " << nik << "\n";
        
        // Tampilkan karakter non-angka yang terdeteksi
        cout << "Karakter non-angka terdeteksi pada posisi:\n";
        for (size_t i = 0; i < nik.length(); i++) {
            if (!isdigit(nik[i])) {
                cout << "  - Posisi " << (i + 1) << ": '" << nik[i] << "'\n";
            }
        }
        
        cout << string(50, '*') << "\n";
        return;
    }

    // Validasi NIK menggunakan hash table
    VoteStatus voteStatus = voterTable.hasVoted(nik);
    if (voteStatus == VoteStatus::AlreadyVoted) {
        cout << "\n" << string(50, '*') << "\n";
        cout << "ERROR: NIK ini tidak dapat melakukan voting!\n";
        cout << "NIK: " << nik << "\n";
        cout << "Satu NIK hanya dapat melakukan voting satu kali.\n";
        cout << "Jika ini adalah kesalahan, silakan hubungi admin.\n";
        cout << string(50, '*') << "\n";
        return;
    }
    // Note: VoteStatus::NotFound is no longer possible due to auto-registration

    system("cls");
    cout << "\nHalo " << namaVoter << "! Selamat datang di sistem voting.\n";
    cout << string(60, '-') << "\n";
    // Tampilkan kandidat dalam format ringkas
    cout << "PILIHAN KANDIDAT:\n";
    for (const auto& k : kandidat) {
        cout << k.nomor << ". " << k.nama << " | Partai: " << k.partai << "\n";
    }
    cout << string(60, '-') << "\n";
    cout << "Masukkan nomor pilihan Anda: ";
    int pilihan;
    cin >> pilihan;
    // Find kandidat yang dipilih menggunakan STL algorithm
    auto it = find_if(kandidat.begin(), kandidat.end(),
                      [pilihan](const Kandidat& k) {
                          return k.nomor == pilihan;
                      });
    if (it != kandidat.end()) {
        system("cls");
        cout << "\n" << string(50, '=') << "\n";
        cout << "KONFIRMASI PILIHAN ANDA:\n";
        cout << string(50, '-') << "\n";
        cout << "Nama Voter     : " << namaVoter << "\n";
        cout << "NIK            : " << nik << "\n";
        cout << "Pilihan Anda   : " << it->nama << "\n";
        cout << "Partai         : " << it->partai << "\n";
        cout << string(50, '-') << "\n";
        cout << "Apakah Anda yakin dengan pilihan ini? (y/n): ";
        char konfirmasi;
        cin >> konfirmasi;
        if (konfirmasi == 'y' || konfirmasi == 'Y') {
            auto waktu = getCurrentTime();
            simpanVote(namaVoter, nik, pilihan, waktu);
            voterTable.markAsVoted(nik);
        } else {
            cout << "\n" << string(50, '-') << "\n";
            cout << "Vote dibatalkan. Silakan coba lagi jika diperlukan.\n";
            cout << string(50, '-') << "\n";
        }
    } else {
        cout << "\n" << string(50,'*') << "\n";
        cout << "ERROR: Nomor kandidat tidak valid!\n";
        cout << "Nomor yang dimasukkan: " << pilihan << "\n";
        cout << "Silakan pilih nomor yang tersedia (1-" << kandidat.size() << ").\n";
        cout << string(50,'*') << "\n";
    }
}
// Modifikasi tampilan hasil untuk hanya menampilkan voting yang sah
void tampilkanHasil(const vector<Kandidat>& kandidat) {
    system("cls");
    cout << "\n" << string(60, '=') << "\n";
    cout << "                      HASIL VOTING                    \n";
    cout << string(60, '=') << "\n\n";

    // Baca semua data voting untuk statistik
    auto [votesPerCandidate, allVotes] = bacaSemuaVotes();
    int totalAll = allVotes.size();
    int totalValid = count_if(allVotes.begin(), allVotes.end(),
                            [](const auto& v){ return v.isValid; });
    int totalInvalid = totalAll - totalValid;

    // Tampilkan hasil per kandidat
    cout << "HASIL VOTING PER KANDIDAT:\n";
    cout << string(60, '-') << "\n";
    
    if (totalAll == 0) {
        cout << "Belum ada data voting untuk ditampilkan.\n";
        return;
    }
    
    // Urutkan kandidat berdasarkan jumlah suara
    vector<Kandidat> sortedKandidat = kandidat;
    sort(sortedKandidat.begin(), sortedKandidat.end(),
         [&votesPerCandidate](const Kandidat& a, const Kandidat& b) {
             return votesPerCandidate[a.nomor] > votesPerCandidate[b.nomor];
         });

    for (const auto& k : sortedKandidat) {
        int votes = votesPerCandidate[k.nomor];
        double percentage = totalAll > 0 ? (votes * 100.0 / totalAll) : 0;
        int barLength = static_cast<int>(percentage * 0.5); // 50 karakter untuk 100%
        
        cout << left << setw(3) << k.nomor << ". "
             << setw(25) << k.nama
             << setw(15) << k.partai
             << setw(5) << votes << " suara ("
             << fixed << setprecision(2) << percentage << "%)\n";
        
        cout << "    ";
        for (int i = 0; i < barLength; i++) {
            cout << BLOCK_CHAR;
        }
        cout << "\n";
    }
    cout << "\n";

    if (allVotes.empty()) {
        cout << "Belum ada data voting.\n";
        tungguInput();
        return;
    }

    // Urutkan berdasarkan waktu terbaru
    sort(allVotes.begin(), allVotes.end(), 
         [](const VoteData& a, const VoteData& b) {
             return a.waktu > b.waktu;
         });

    const int ITEMS_PER_PAGE = 20;
    int totalPages = (allVotes.size() + ITEMS_PER_PAGE - 1) / ITEMS_PER_PAGE;
    int currentPage = 1;

    while (true) {
        cout << "\n" << string(60, '=') << "\n";
        cout << "                DETAIL RECORD VOTING                \n";
        cout << string(60, '=') << "\n\n";

        // Tampilkan header tabel
        cout << setw(5) << "No" << setw(20) << "NIK (Enkripsi)" << setw(25) << "Nama (Enkripsi)" 
             << setw(10) << "Pilihan" << setw(20) << "Waktu" << "\n";
        cout << string(80, '-') << "\n";

        // Hitung indeks awal dan akhir untuk halaman saat ini
        int startIdx = (currentPage - 1) * ITEMS_PER_PAGE;
        int endIdx = min(startIdx + ITEMS_PER_PAGE, static_cast<int>(allVotes.size()));

        // Tampilkan data untuk halaman saat ini
        for (int i = startIdx; i < endIdx; i++) {
            const auto& vote = allVotes[i];
            // Enkripsi NIK dan nama untuk keamanan
            string encryptedNIK = encryptData(vote.nik, 3);
            string encryptedName = encryptData(vote.namaVoter, 3);
            
            cout << setw(5) << (i + 1) << setw(20) << encryptedNIK << setw(25) << encryptedName 
                 << setw(10) << vote.pilihan << setw(20) << vote.getWaktuStr() << "\n";
        }

        cout << string(80, '-') << "\n";
        cout << "Halaman " << currentPage << " dari " << totalPages << "\n";
        cout << "Total Record: " << allVotes.size() << "\n\n";
        
        cout << "Navigasi:\n";
        cout << "1. Halaman Sebelumnya\n";
        cout << "2. Halaman Berikutnya\n";
        cout << "3. Pilih Halaman Spesifik\n";
        cout << "4. Kembali ke Menu Utama\n\n";
        cout << "Pilihan: ";

        string choice;
        cin >> choice;

        if (choice == "1" && currentPage > 1) {
            currentPage--;
        }
        else if (choice == "2" && currentPage < totalPages) {
            currentPage++;
        }
        else if (choice == "3") {
            cout << "Masukkan nomor halaman (1-" << totalPages << "): ";
            int page;
            if (cin >> page && page >= 1 && page <= totalPages) {
                currentPage = page;
            }
            else {
                cout << "Nomor halaman tidak valid!\n";
                this_thread::sleep_for(chrono::seconds(1));
            }
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
        }
        else if (choice == "4") {
            break;
        }
        else {
            cout << "Pilihan tidak valid!\n";
            this_thread::sleep_for(chrono::seconds(1));
        }
    }
}
// Fungsi untuk verifikasi data voting
void verifikasiDataVoting() {
    system("cls");
    cout << "\n" << string(80, '=') << "\n";
    cout << "                      VERIFIKASI DATA VOTING                    \n";
    cout << string(80, '=') << "\n\n";

    // Baca semua data voting
    auto [votesPerCandidate, allVotes] = bacaSemuaVotes();
    
    if (allVotes.empty()) {
        cout << "Belum ada data voting untuk diverifikasi.\n";
        tungguInput();
        return;
    }

    // Urutkan berdasarkan waktu (dari yang terbaru)
    sort(allVotes.begin(), allVotes.end(), 
         [](const VoteData& a, const VoteData& b) {
             return a.waktu > b.waktu;
         });

    const int ITEMS_PER_PAGE = 20;
    int totalPages = (allVotes.size() + ITEMS_PER_PAGE - 1) / ITEMS_PER_PAGE;
    int currentPage = 1;

    while (true) {
        system("cls");
        cout << "\n" << string(80, '=') << "\n";
        cout << "                      VERIFIKASI DATA VOTING                    \n";
        cout << string(80, '=') << "\n\n";

        // Tampilkan header tabel
        setColor(11); // Cyan
        cout << left << setw(20) << "NIK" 
             << setw(30) << "Nama Pemilih" 
             << setw(15) << "Pilihan" 
             << setw(25) << "Waktu Voting" 
             << "Status" << "\n";
        cout << string(80, '-') << "\n";

        // Hitung range data yang akan ditampilkan
        int startIdx = (currentPage - 1) * ITEMS_PER_PAGE;
        int endIdx = min(startIdx + ITEMS_PER_PAGE, (int)allVotes.size());
        
        // Tampilkan data voting untuk halaman saat ini
        for (int i = startIdx; i < endIdx; i++) {
            const auto& vote = allVotes[i];
            auto time = chrono::system_clock::to_time_t(vote.waktu);
            auto ms = chrono::duration_cast<chrono::milliseconds>(
                vote.waktu.time_since_epoch()) % 1000;
            
            stringstream ss;
            ss << put_time(localtime(&time), "%Y-%m-%d %H:%M:%S");
            ss << '.' << setfill('0') << setw(3) << ms.count();
            
            if (vote.isValid) {
                setColor(10); // Hijau untuk vote valid
            } else {
                setColor(12); // Merah untuk vote invalid
            }
            
            cout << left << setw(20) << vote.nik
                 << setw(30) << vote.namaVoter
                 << setw(15) << vote.pilihan
                 << setw(25) << ss.str()
                 << (vote.isValid ? "Valid" : "Invalid") << "\n";
        }

        setColor(7); // Reset warna ke default
        cout << "\n" << string(80, '=') << "\n";
        cout << "                       Total Record: " << allVotes.size() << " voting\n";
        cout << "                       Halaman " << currentPage << " dari " << totalPages << "\n";
        cout << string(80, '=') << "\n\n";
        
        cout << "Navigasi:\n";
        cout << "1. Halaman Sebelumnya\n";
        cout << "2. Halaman Berikutnya\n";
        cout << "3. Pilih Halaman Spesifik\n";
        cout << "4. Detail Data Invalid\n";
        cout << "5. Hapus Data Invalid\n";
        cout << "6. Kembali ke Menu\n";
        cout << "Pilihan: ";
        
        int choice;
        cin >> choice;
        
        switch (choice) {
            case 1:
                if (currentPage > 1) currentPage--;
                break;
            case 2:
                if (currentPage < totalPages) currentPage++;
                break;
            case 3: {
                cout << "Masukkan nomor halaman (1-" << totalPages << "): ";
                int targetPage;
                cin >> targetPage;
                if (targetPage >= 1 && targetPage <= totalPages) {
                    currentPage = targetPage;
    } else {
                    cout << "\nNomor halaman tidak valid!\n";
                    this_thread::sleep_for(chrono::seconds(1));
                }
                break;
            }
            case 4: {
                system("cls");
                cout << "\n" << string(80, '=') << "\n";
                cout << "                      DETAIL DATA INVALID                    \n";
                cout << string(80, '=') << "\n\n";
                
                bool foundInvalid = false;
                for (const auto& vote : allVotes) {
                    if (!vote.isValid) {
                        foundInvalid = true;
                        cout << "NIK: " << vote.nik << "\n";
                        cout << "Nama: " << vote.namaVoter << "\n";
                        cout << "Pilihan: " << vote.pilihan << "\n";
                        cout << "Waktu: " << vote.getWaktuStr() << "\n";
                        
                        // Analisis alasan ketidakvalidan
                        cout << "Alasan Invalid:\n";
                        if (vote.nik.length() != 16) {
                            cout << "- NIK tidak 16 digit\n";
                        }
                        if (!all_of(vote.nik.begin(), vote.nik.end(), ::isdigit)) {
                            cout << "- NIK mengandung karakter non-digit\n";
                        }
                        if (vote.namaVoter.empty()) {
                            cout << "- Nama pemilih kosong\n";
                        }
                        if (vote.pilihan <= 0) {
                            cout << "- Pilihan tidak valid\n";
                        }
                        cout << string(80, '-') << "\n";
                    }
                }
                
                if (!foundInvalid) {
                    cout << "Tidak ada data invalid yang ditemukan.\n";
                }
                
        tungguInput();
                break;
            }
            case 5: {
                cout << "\nApakah Anda yakin ingin menghapus semua data invalid? (y/n): ";
                char confirm;
                cin >> confirm;
                
                if (confirm == 'y' || confirm == 'Y') {
                    // Buat file temporary untuk menyimpan data valid
                    ofstream tempFile("temp_votes.dat");
                    ofstream tempKeys("temp_keys.dat");
    ifstream voteFile("votes.dat");
    ifstream keyFile("keys.dat");
                    
                    if (tempFile.is_open() && tempKeys.is_open() && 
                        voteFile.is_open() && keyFile.is_open()) {

    string encryptedData;
    int key;
                        int validCount = 0;

    while (getline(voteFile, encryptedData) && keyFile >> key) {
        try {
            string decryptedData = decryptData(encryptedData, key);
            VoteData vote = VoteData::fromString(decryptedData);
            
                                if (vote.isValid) {
                                    // Simpan data valid ke file temporary
                                    tempFile << encryptedData << "\n";
                                    tempKeys << key << "\n";
                                    validCount++;
                                }
                            } catch (const exception& e) {
                                logError("Error processing vote data: " + string(e.what()));
                            }
                        }
                        
                        // Tutup semua file
                        tempFile.close();
                        tempKeys.close();
                        voteFile.close();
                        keyFile.close();
                        
                        // Ganti file asli dengan file temporary
                        remove("votes.dat");
                        remove("keys.dat");
                        rename("temp_votes.dat", "votes.dat");
                        rename("temp_keys.dat", "keys.dat");
                        
                        cout << "\nBerhasil menghapus " << (allVotes.size() - validCount) 
                             << " data invalid.\n";
                        cout << "Sisa " << validCount << " data valid.\n";
                        
                        // Refresh data
                        allVotes.clear();
                        tie(votesPerCandidate, allVotes) = bacaSemuaVotes();
                        totalPages = (allVotes.size() + ITEMS_PER_PAGE - 1) / ITEMS_PER_PAGE;
                        if (currentPage > totalPages) currentPage = totalPages;
                    }
                }
                tungguInput();
                break;
            }
            case 6:
                return;
            default:
                cout << "\nPilihan tidak valid!\n";
                this_thread::sleep_for(chrono::seconds(1));
        }
    }
}

// Fungsi untuk audit trail
void auditTrailVoting() {
    string username, password;
    cout << "\nMasukkan username admin: ";
    cin >> username;
    cout << "Masukkan password admin: ";
    cin >> password;
    
    if (!verifikasiAdmin(username, password)) {
        cout << "\nAkses ditolak! Username atau password salah.\n";
        tungguInput();
        return;
    }

    cout << "\n" << string(70, '=') << "\n";
    cout << "                  AUDIT TRAIL VOTING                  \n";
    cout << string(70, '=') << "\n";

    ifstream voteFile("votes.dat");
    ifstream keyFile("keys.dat");
    if (!voteFile.is_open() || !keyFile.is_open()) {
        cout << "File data voting tidak ditemukan!\n";
        return;
    }

    string encryptedData;
    int key;
    int counter = 1;
    map<string, int> votePerWaktu;  // Menghitung vote per waktu
    map<string, set<string>> voterPerWaktu;  // Menghitung voter unik per waktu

    cout << "Menganalisis audit trail...\n\n";
    while (getline(voteFile, encryptedData) && keyFile >> key) {
        try {
            string decryptedData = decryptData(encryptedData, key);
            VoteData vote = VoteData::fromString(decryptedData);
            
            if (vote.isValid) {
                // Ekstrak jam dari waktu (format: YYYY-MM-DD HH:MM:SS)
                string jam = vote.getWaktuStr().substr(11, 2);
                votePerWaktu[jam]++;
                voterPerWaktu[jam].insert(vote.nik);
            }
        }
        catch (const exception& e) {
            string errorMsg = "Error pada vote #" + to_string(counter) + " dalam audit trail: " + string(e.what());
            logError(errorMsg);
            cout << "⚠️  Error pada vote #" << counter << "!\n";
        }
        counter++;
    }

    cout << "ANALISIS VOTING PER JAM:\n";
    cout << string(70, '-') << "\n";
    for (const auto& pair : votePerWaktu) {
        cout << "Jam " << pair.first << ":00\n";
        cout << "   Total Vote: " << pair.second << "\n";
        cout << "   Voter Unik: " << voterPerWaktu[pair.first].size() << "\n";
        cout << string(70, '-') << "\n";
    }

    voteFile.close();
    keyFile.close();
    cout << string(70, '=') << "\n";
    tungguInput();  // Add this line to wait for user input
}

// Tambahkan fungsi-fungsi analisis baru setelah fungsi auditTrailVoting
void tampilkanStatistikVoting() {
    auto [hasilVote, semuaVote] = bacaSemuaVotes();
    if (semuaVote.empty()) {
        cout << "\n" << string(50, '*') << "\n";
        cout << "Belum ada data voting yang tersimpan!\n";
        cout << string(50, '*') << "\n";
        return;
    }

    cout << "\n" << string(70, '=') << "\n";
    cout << "                  STATISTIK VOTING DETAIL                  \n";
    cout << string(70, '=') << "\n\n";

    // Hitung total vote dan vote sah
    int totalVote = semuaVote.size();
    int totalVoteSah = count_if(semuaVote.begin(), semuaVote.end(),
                               [](const VoteData& vote) { return vote.isValid; });
    int totalVoteTidakSah = totalVote - totalVoteSah;

    cout << "STATISTIK UMUM:\n";
    cout << string(70, '-') << "\n";
    cout << "Total Vote: " << totalVote << "\n";
    cout << "Vote Sah: " << totalVoteSah << " (" 
         << fixed << setprecision(2) << (totalVoteSah * 100.0 / totalVote) << "%)\n";
    cout << "Vote Tidak Sah: " << totalVoteTidakSah << " ("
         << fixed << setprecision(2) << (totalVoteTidakSah * 100.0 / totalVote) << "%)\n\n";

    // Analisis berdasarkan waktu
    map<string, int> votePerHari;
    map<string, int> votePerJam;
    for (const auto& vote : semuaVote) {
        if (vote.isValid) {
            string waktu = vote.getWaktuStr();
            string hari = waktu.substr(0, 10);  // YYYY-MM-DD
            string jam = waktu.substr(11, 2);   // HH
            
            votePerHari[hari]++;
            votePerJam[jam]++;
        }
    }

    cout << "ANALISIS BERDASARKAN WAKTU:\n";
    cout << string(70, '-') << "\n";
    cout << "Vote per Hari:\n";
    for (const auto& pair : votePerHari) {
        cout << "  " << pair.first << ": " << pair.second << " vote\n";
    }
    cout << "\nVote per Jam:\n";
    for (const auto& pair : votePerJam) {
        cout << "  Jam " << pair.first << ":00 - " << pair.first << ":59: " 
             << pair.second << " vote\n";
    }
    cout << string(70, '-') << "\n";
}

void tampilkanRantaiVoting() {
    system("cls");
    cout << "\n" << string(80, '=') << "\n";
    cout << "                      ANALISIS RANTAI VOTING                    \n";
    cout << string(80, '=') << "\n\n";

    // Baca semua data voting
    auto [votesPerCandidate, allVotes] = bacaSemuaVotes();
    
    if (allVotes.empty()) {
        cout << "Belum ada data voting untuk dianalisis.\n";
        tungguInput();
        return;
    }

    // Buat graph rantai voting
    VoteChainGraph graph;
    for (const auto& vote : allVotes) {
        graph.addVote(vote);
    }

    // Dapatkan statistik rantai voting
    auto stats = graph.getVotingStats();
    
    // Tampilkan statistik rantai
    setColor(11); // Cyan
    cout << " STATISTIK RANTAI VOTING\n";
    cout << string(80, '-') << "\n";
    cout << "Total Rantai    : " << stats.totalVotes << " rantai\n";
    cout << "Rantai Terpanjang: " << stats.maxChainLength << " voting\n";
    cout << "Rantai Terpendek : " << stats.minChainLength << " voting\n";
    cout << "Rata-rata Rantai : " << fixed << setprecision(2) 
         << stats.avgChainLength << " voting\n\n";

    // Tampilkan distribusi rantai
    setColor(10); // Hijau
    cout << " DISTRIBUSI RANTAI VOTING\n";
    cout << string(80, '-') << "\n";
    
    // Dapatkan semua rantai voting
    auto chains = graph.analyzeVotingChain();
    
    // Urutkan rantai berdasarkan panjang (dari terpanjang)
    sort(chains.begin(), chains.end(),
         [](const vector<VoteData>& a, const vector<VoteData>& b) {
             return a.size() > b.size();
         });

    // Tampilkan detail rantai
    for (size_t i = 0; i < chains.size(); i++) {
        const auto& chain = chains[i];
        cout << "\nRantai #" << (i + 1) << " (" << chain.size() << " voting):\n";
        cout << string(80, '-') << "\n";
        
        for (const auto& vote : chain) {
            cout << "NIK: " << vote.nik << "\n";
            cout << "Nama: " << vote.namaVoter << "\n";
            cout << "Pilihan: " << vote.pilihan << "\n";
            cout << "Waktu: " << vote.getWaktuStr() << "\n";
            cout << string(40, '-') << "\n";
        }
    }

    cout << "\n" << string(80, '=') << "\n";
    cout << "Total Rantai: " << chains.size() << " rantai voting\n";
    cout << string(80, '=') << "\n";

    setColor(7); // Reset warna ke default
    tungguInput();
}

void tampilkanDistribusiVote(const vector<Kandidat>& kandidat) {
    auto [hasilVote, semuaVote] = bacaSemuaVotes();
    if (semuaVote.empty()) {
        cout << "\n" << string(50, '*') << "\n";
        cout << "Belum ada data voting yang tersimpan!\n";
        cout << string(50, '*') << "\n";
        return;
    }

    cout << "\n" << string(70, '=') << "\n";
    cout << "                  DISTRIBUSI VOTE DETAIL                  \n";
    cout << string(70, '=') << "\n\n";

    // Hitung total vote sah
    int totalVoteSah = count_if(semuaVote.begin(), semuaVote.end(),
                               [](const VoteData& vote) { return vote.isValid; });

    // Analisis per kandidat
    cout << "DISTRIBUSI VOTE PER KANDIDAT:\n";
    cout << string(70, '-') << "\n";
    
    if (totalVoteSah==0) {
        cout<<"Belum ada data voting untuk ditampilkan.\n";
        return;
    }
    
    for (const auto& k : kandidat) {
        int jumlahVote = count_if(semuaVote.begin(), semuaVote.end(),
                                [&k](const VoteData& vote) {
                                    return vote.isValid && vote.pilihan == k.nomor;
                                });
        double persentase = totalVoteSah > 0 ? (jumlahVote * 100.0 / totalVoteSah) : 0;
        int barLength = (int)(persentase / 2);
        
        cout << "Kandidat " << k.nomor << " (" << k.nama << "):\n";
        cout << "Jumlah Vote: " << jumlahVote << "\n";
        cout << "Persentase : ";
        for (int i = 0; i < barLength; i++) cout << "█";
        for (int i = barLength; i < 50; i++) cout << "░";
        cout << " " << fixed << setprecision(2) << persentase << "%\n\n";
    }

    // Analisis berdasarkan waktu voting
    cout << "DISTRIBUSI VOTE BERDASARKAN WAKTU:\n";
    cout << string(70, '-') << "\n";
    map<string, map<int, int>> votePerHariPerKandidat;
    for (const auto& vote : semuaVote) {
        if (vote.isValid) {
            string hari = vote.getWaktuStr().substr(0, 10);
            votePerHariPerKandidat[hari][vote.pilihan]++;
        }
    }

    for (const auto& pair : votePerHariPerKandidat) {
        cout << "Tanggal " << pair.first << ":\n";
        for (const auto& k : kandidat) {
            // Gunakan find untuk mengakses const map
            auto it = pair.second.find(k.nomor);
            int jumlahVote = (it != pair.second.end()) ? it->second : 0;
            cout << "  Kandidat " << k.nomor << ": " << jumlahVote << " vote\n";
        }
        cout << "\n";
    }
    cout << string(70, '=') << "\n";
}

void tampilkanDashboardAdmin(const vector<Kandidat>& kandidat) {
    system("cls");
    #ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    #endif
    
    cout << "\n" << string(80, '=') << "\n";
    cout << "                      DASHBOARD ADMIN PEMILU                    \n";
    cout << string(80, '=') << "\n\n";

    // Tampilkan timestamp
    auto now = chrono::system_clock::now();
    auto time = chrono::system_clock::to_time_t(now);
    auto ms = chrono::duration_cast<chrono::milliseconds>(
        now.time_since_epoch()) % 1000;
    
    stringstream ss;
    ss << put_time(localtime(&time), "%Y-%m-%d %H:%M:%S");
    ss << '.' << setfill('0') << setw(3) << ms.count();
    
    setColor(11); // Cyan
    cout << "Timestamp: " << ss.str() << "\n\n";

    // Baca data voting
    auto [votesPerCandidate, allVotes] = bacaSemuaVotes();
    int totalAll     = allVotes.size();
    int totalValid   = count_if(allVotes.begin(), allVotes.end(),
                                [](auto& v){ return v.isValid; });
    int totalInvalid = totalAll - totalValid;
    
    // Tampilkan ringkasan statistik
    setColor(14); // Kuning
    cout << " RINGKASAN STATISTIK\n";
    cout << string(80, '-') << "\n";
    cout << "Total Vote       : " << totalAll << " suara\n";
    cout << "  - Vote Sah     : " << totalValid << " suara\n";
    cout << "  - Vote Tidak Sah: " << totalInvalid << " suara\n";
    cout << "Total Kandidat  : " << kandidat.size() << " kandidat\n";
    cout << "Total Transaksi : " << allVotes.size() << " transaksi\n\n";

    // Tampilkan distribusi suara
    setColor(10); // Hijau
    cout << " DISTRIBUSI SUARA PER KANDIDAT\n";
    cout << string(80, '-') << "\n";
    
    if (totalAll==0) {
        cout<<"Belum ada data voting untuk ditampilkan.\n";
        return;
    }
    
    // Urutkan kandidat berdasarkan jumlah suara
    vector<Kandidat> sortedKandidat = kandidat;
    sort(sortedKandidat.begin(), sortedKandidat.end(),
         [&votesPerCandidate](const Kandidat& a, const Kandidat& b) {
             return votesPerCandidate[a.nomor] > votesPerCandidate[b.nomor];
         });

    for (const auto& k : sortedKandidat) {
        int votes = votesPerCandidate[k.nomor];
        double percentage = totalAll > 0 ? (votes * 100.0 / totalAll) : 0;
        int barLength = static_cast<int>(percentage * 0.5); // 50 karakter untuk 100%
        
        cout << left << setw(3) << k.nomor << ". "
             << setw(25) << k.nama
             << setw(15) << k.partai
             << setw(5) << votes << " suara ("
             << fixed << setprecision(2) << percentage << "%)\n";
        
        cout << "    ";
        for (int i = 0; i < barLength; i++) {
            cout << BLOCK_CHAR;
        }
        cout << "\n";
    }
    cout << "\n";

    // Tampilkan analisis waktu voting
    setColor(13); // Magenta
    cout << " ANALISIS WAKTU VOTING\n";
    cout << string(80, '-') << "\n";
    
    map<int, int> votesPerHour;
    for (const auto& vote : allVotes) {
        auto time = chrono::system_clock::to_time_t(vote.waktu);
        tm* timeinfo = localtime(&time);
        votesPerHour[timeinfo->tm_hour]++;
    }
    
    int maxVotes = 0;
    for (const auto& [hour, votes] : votesPerHour) {
        maxVotes = max(maxVotes, votes);
    }
    
    for (int hour = 0; hour < 24; hour++) {
        int votes = votesPerHour[hour];
        int barLength = maxVotes > 0 ? (votes * 30 / maxVotes) : 0;
        
        cout << setw(2) << hour << ":00 - " << setw(2) << hour << ":59 | ";
        for (int i = 0; i < barLength; i++) {
            cout << BLOCK_CHAR;
        }
        cout << " " << votes << " suara\n";
    }
    cout << "\n";
    setColor(7);
    tungguInput();
}

void tampilkanDetailVoting() {
    system("cls");
    setupConsole();
    
    cout << "\n" << string(80, '=') << "\n";
    cout << "                               DETAIL RECORD VOTING                    \n";
    cout << string(80, '=') << "\n\n";

    // Baca semua data voting
    auto [votesPerCandidate, allVotes] = bacaSemuaVotes();

    // Urutkan berdasarkan waktu (dari yang terbaru)
    sort(allVotes.begin(), allVotes.end(), 
         [](const VoteData& a, const VoteData& b) {
             return a.waktu > b.waktu;
         });

    const int ITEMS_PER_PAGE = 20;  // Jumlah item per halaman
    int totalPages = (allVotes.size() + ITEMS_PER_PAGE - 1) / ITEMS_PER_PAGE;
    int currentPage = 1;

    while (true) {
        system("cls");
        cout << "\n" << string(80, '=') << "\n";
        cout << "                               DETAIL RECORD VOTING                    \n";
        cout << string(80, '=') << "\n\n";

    // Tampilkan header tabel
    setColor(11); // Cyan
    cout << left << setw(20) << "NIK" 
         << setw(30) << "Nama Pemilih" 
         << setw(15) << "Pilihan" 
         << setw(25) << "Waktu Voting" 
         << "Status" << "\n";
    cout << string(80, '-') << "\n";
    
        // Hitung range data yang akan ditampilkan
        int startIdx = (currentPage - 1) * ITEMS_PER_PAGE;
        int endIdx = min(startIdx + ITEMS_PER_PAGE, (int)allVotes.size());
        
        // Tampilkan data voting untuk halaman saat ini
        for (int i = startIdx; i < endIdx; i++) {
            const auto& vote = allVotes[i];
        auto time = chrono::system_clock::to_time_t(vote.waktu);
        auto ms = chrono::duration_cast<chrono::milliseconds>(
            vote.waktu.time_since_epoch()) % 1000;
        
        stringstream ss;
        ss << put_time(localtime(&time), "%Y-%m-%d %H:%M:%S");
        ss << '.' << setfill('0') << setw(3) << ms.count();
        
        if (vote.isValid) {
            setColor(10); // Hijau untuk vote valid
        } else {
            setColor(12); // Merah untuk vote invalid
        }
        
        cout << left << setw(20) << vote.nik
             << setw(30) << vote.namaVoter
             << setw(15) << vote.pilihan
             << setw(25) << ss.str()
             << (vote.isValid ? "Valid" : "Invalid") << "\n";
    }
    
    setColor(7); // Reset warna ke default
    cout << "\n" << string(80, '=') << "\n";
    cout << "                       Total Record: " << allVotes.size() << " voting\n";
        cout << "                       Halaman " << currentPage << " dari " << totalPages << "\n";
    cout << string(80, '=') << "\n\n";

        cout << "Navigasi:\n";
        cout << "1. Halaman Sebelumnya\n";
        cout << "2. Halaman Berikutnya\n";
        cout << "3. Pilih Halaman Spesifik\n";
        cout << "4. Kembali ke Menu\n";
        cout << "Pilihan: ";
        
        int choice;
        cin >> choice;
        
        switch (choice) {
            case 1:
                if (currentPage > 1) currentPage--;
                break;
            case 2:
                if (currentPage < totalPages) currentPage++;
                break;
            case 3: {
                cout << "Masukkan nomor halaman (1-" << totalPages << "): ";
                int targetPage;
                cin >> targetPage;
                if (targetPage >= 1 && targetPage <= totalPages) {
                    currentPage = targetPage;
        } else {
                    cout << "\nNomor halaman tidak valid!\n";
                    this_thread::sleep_for(chrono::seconds(1));
                }
                break;
            }
            case 4:
                return;
            default:
                cout << "\nPilihan tidak valid!\n";
                this_thread::sleep_for(chrono::seconds(1));
        }
    }
}

void setColor(int color) {
#ifdef _WIN32
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hConsole != INVALID_HANDLE_VALUE) {
        SetConsoleTextAttribute(hConsole, color);
    }
#else
    // ANSI escape codes untuk Unix-like systems
    const char* colors[] = {
        "\033[0m",  // Reset
        "\033[30m", // Black
        "\033[31m", // Red
        "\033[32m", // Green
        "\033[33m", // Yellow
        "\033[34m", // Blue
        "\033[35m", // Magenta
        "\033[36m", // Cyan
        "\033[37m", // White
        "\033[1;30m", // Bright Black
        "\033[1;31m", // Bright Red
        "\033[1;32m", // Bright Green
        "\033[1;33m", // Bright Yellow
        "\033[1;34m", // Bright Blue
        "\033[1;35m", // Bright Magenta
        "\033[1;36m", // Bright Cyan
        "\033[1;37m"  // Bright White
    };
    if (color >= 0 && color < 16) {
        cout << colors[color];
    }
#endif
}

int main();

#ifdef _WIN32
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // Redirect ke main biasa
    return main();
}
#endif

// Implementasi main
int main() {
    setupConsole();  // Set console encoding at program start
    
    // Baca data kandidat
    vector<Kandidat> kandidat = bacaKandidat();
    if (kandidat.empty()) {
        cout << "Error: Tidak dapat membaca data kandidat!\n";
        return 1;
    }

    int pilihan;
    do {
        system("cls");
        tampilkanMenu();
        cin >> pilihan;

        switch (pilihan) {
            case 1:
                tampilkanKandidat(kandidat);
                tungguInput();
                break;
            case 2:
                lakukanVoting(kandidat);
                tungguInput();
                break;
            case 3:
                tampilkanHasil(kandidat);
                tungguInput();
                break;
            case 4:
                menuAdmin(kandidat);
                if (pilihan != 4) {
                    tungguInput();
                }
                break;
            case 5:
                cout << "\nTerima kasih telah menggunakan sistem voting!\n";
                break;
            default:
                cout << "\nPilihan tidak valid!\n";
                tungguInput();
        }
    } while (pilihan != 5);

    return 0;
}

// Implementasi tampilkanMenu
void tampilkanMenu() {
    system("cls");
    cout << "\n" << string(60, '=') << "\n";
    cout << "   ⠀⠀⠀⠀⣴⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢶⡄⠀⠀⠀" << endl;
    cout << "   ⠀⠀⠀⣼⢸⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢸⠹⠀⠀⠀" << endl;
    cout << "   ⠀⠀⡰⡇⠘⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⡀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⡞⠀⠷⡀⠀" << endl;
    cout << "   ⠀⠀⡇⢸⢄⣇⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢀⣠⣴⣦⠤⠤⠤⠤⠴⢚⣷⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢀⢇⢰⠀⡇⠀" << endl;
    cout << "   ⠀⣰⢇⢬⡜⣜⣄⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣸⣯⡠⡴⣾⡷⠃⢠⡄⠨⡟⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢀⠞⡞⢎⢀⢷⠀" << endl;
    cout << "   ⠀⣿⠘⡜⡜⣌⢌⠢⡀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢠⢎⣬⠤⢤⣬⡍⣑⣬⠀⡇⣆⢹⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣠⢋⢎⠞⠞⡞⢨⡄" << endl;
    cout << "   ⢸⠙⡄⡘⣜⣌⢮⠣⡙⢦⣀⠀⠀⠀⠀⠀⠀⠀⠀⠘⠃⠸⣟⣋⠥⢯⠁⢀⡈⣀⣠⠸⡀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢀⣠⠞⡡⣫⢊⢞⠞⡄⡎⡇" << endl;
    cout << "   ⢸⢧⢙⡝⣌⢮⠢⣑⢬⣺⣌⡑⠲⣤⣀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣶⡾⣻⣟⣿⢯⣷⣵⣾⡋⠀⠀⠀⠀⠀⠀⠀⣀⣠⡴⢚⣩⣾⣪⢔⡥⢫⢎⣜⣜⣠⢻" << endl;
    cout << "   ⢸⡈⢺⢮⠪⡢⣝⠪⣑⡯⣕⡂⢽⣄⡀⢭⣓⢆⠀⠀⠀⠀⠀⢬⣿⢳⡽⣏⡶⣻⣏⡞⣽⡅⠀⠀⠀⠀⢠⢖⡫⠅⣀⣼⢅⣒⡭⣗⡥⢚⠵⡫⣪⣺⠃⣸" << endl;
    cout << "   ⢨⠱⣴⣕⢵⣬⡲⢽⣲⡭⠵⣶⡿⠤⢤⣙⠈⡏⢣⡀⠀⠠⣼⣿⢯⣻⣝⣯⡟⣼⣷⣟⣿⣿⡀⠀⠀⡰⠃⡇⢈⡡⠤⢼⣵⡶⠭⣟⡺⠕⣫⣾⢕⢕⡜⢹" << endl;
    cout << "   ⢸⢦⢨⡪⣓⠍⣛⠵⢺⣿⣭⣭⣟⣒⣒⡢⠀⢰⡀⠙⠒⢲⣿⣿⣾⣹⣯⣷⣭⣾⣹⣎⣿⣿⡗⠒⠚⠁⡸⠀⢰⣒⣒⣻⣯⣭⣭⣿⠲⢝⡋⢕⡭⣪⣠⢺" << endl;
    cout << "   ⢸⣆⠙⡯⣲⠭⣚⣻⢯⣧⣤⣴⣿⡖⠒⣚⡃⠀⢻⢉⡉⠉⠉⠉⠉⢉⠉⠉⡏⠉⠉⠉⢉⡉⠉⠉⠉⢹⠃⠀⣛⡒⠒⣾⣷⣤⣤⣯⢟⣛⡪⣕⡪⡛⢁⡞" << endl;
    cout << "   ⠈⣧⡙⡻⣖⡭⣗⣚⣉⣗⣒⣲⢽⣯⠭⢑⣺⡄⢸⢸⡇⠀⠀⠀⠀⢸⡄⠀⡇⢀⡴⠿⠩⣍⣛⢢⡀⢸⢀⣼⣒⠩⢭⣿⢷⣒⣚⣏⣙⣒⡯⣕⡾⡛⣡⠇" << endl;
    cout << "   ⠀⠳⡉⠺⢷⣊⠭⠭⣉⠭⣯⣉⡷⣿⣯⣕⣿⣿⣿⢪⣧⣴⣶⣶⣤⣾⠇⠀⡇⣼⣭⣯⣾⣽⣿⣿⡇⢸⡿⡯⣕⣽⣿⡷⣏⣹⠯⢭⡭⠭⢅⣲⠍⠋⡱⠁" << endl;
    cout << "   ⠀⢯⡓⢭⣛⣒⣭⣭⣥⡼⢯⣽⢯⣿⣿⣿⣼⣸⠀⠛⢷⢿⣿⣯⣿⣤⣤⣧⣼⣿⣿⣿⣿⠛⠛⠃⢸⣸⣼⣿⣿⣯⢿⣽⠿⣤⣭⣭⣕⣒⣭⠗⣩⠃⠀" << endl;
    cout << "   ⠀⠘⢏⡙⠛⣧⡴⠶⣤⣿⠻⣽⣿⣟⣿⡿⣿⣿⠀⠀⠸⣿⡿⢸⣿⣿⣿⢻⣿⣿⣿⠛⠛⠓⠂⠀⢸⣯⣻⣿⣿⣿⣿⡽⢻⣦⡴⠦⣤⡟⢓⣉⠟⠀⠀" << endl;
    cout << "   ⠀⠀⠈⢮⣅⣒⣺⣿⡶⣶⣟⣣⣝⣾⣻⡿⣿⣺⢻⡟⡛⠛⠛⢻⣿⣉⠁⠈⢉⣿⣿⠛⠛⠛⠛⠛⢻⣻⣿⣿⣿⣞⣿⣛⣳⡶⣾⣿⣒⣈⣭⠋⠀⠀⠀" << endl;
    cout << "   ⠀⠀⠀⠀⠓⣄⠤⠽⠶⢿⣋⣳⢿⣻⣕⡮⣺⣽⢼⣿⣷⢿⣷⢸⣿⣿⣤⣤⣸⣿⣿⣀⣴⣶⣦⡀⢸⣽⡺⣞⣽⣻⢿⣏⣽⠷⠾⠤⢄⡖⠁⠀⠀⠀⠀" << endl;
    cout << "   ⠀⠀⠀⠀⠀⠈⠙⢭⣍⣉⣉⠷⢾⣽⢾⡿⢿⣾⣿⣿⣿⣿⣟⣦⢻⣿⣿⣿⣿⣿⣷⡿⠃⢀⣿⡇⡾⣯⢿⣿⢮⣽⠾⢏⣉⣉⣭⠟⠉⠀⠀⠀⠀⠀⠀" << endl;
    cout << "   ⠀⠀⠀⠀⠀⠀⠀⠀⠈⠑⠲⠶⠍⠉⠭⠽⢛⣏⣿⣽⣿⣿⣯⣯⣿⢍⡻⡿⢻⣿⠋⠀⢀⣾⡟⣼⣩⠙⠿⠼⠏⠉⠵⠶⠒⠉⠀⠀⠀⠀⠀⠀⠀⠀⠀" << endl;
    cout << "   ⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢀⣼⣿⢽⡽⣾⣿⣿⣽⣿⠿⢧⡇⣿⡇⠀⣠⣾⣏⣾⣿⣿⣧⡀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀" << endl;
    cout << "     ⠀⠀⠀⣄⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢼⡭⣾⢳⡧⡽⣿⣿⡿⠿⢤⡀⡇⠙⠿⠿⣻⣷⣿⢼⡶⣷⢽⡇⠀⠀⠀⠀⠀⠀⠀⠀⠀⢀⣠⠀⠀⠀⠀⠀" << endl;
    cout << "   ⠀⠀⠀⠀⠀⠹⠙⢦⡀⠀⠀⠀⠀⠀⠀⠀⢸⢶⣿⢹⣯⠻⢿⡹⣿⣷⢢⢤⣧⡤⣶⣿⢽⠹⠾⣽⡟⣿⡶⡇⠀⠀⠀⠀⠀⠀⠀⢀⠔⢩⠃⠀⠀⠀⠀⠀" << endl;
    cout << "   ⠀⠀⠀⠘⠯⣁⠀⠀⠑⣄⠀⣀⠀⠀⠀⣠⠃⢀⠟⠁⠀⠀⡼⣿⣿⣩⡿⡉⣩⡿⣡⢿⡾⡆⠀⠀⠈⠣⡀⠸⡄⠀⠀⠀⡀⠀⡴⠁⠀⠈⣨⠝⠃⠀⠀⠀" << endl;
    cout << "   ⠀⠀⠀⠀⠀⠈⢦⡀⢀⡼⠋⠉⠑⣄⠴⠣⡴⠁⠀⠀⠀⢰⢣⢻⡎⢿⣥⠋⢙⡥⠻⡳⠻⠸⡄⠀⠀⠀⠘⣦⠞⠦⡴⠋⠉⠙⣄⠀⢀⠞⠁⠀⠀⠀⠀⠀" << endl;
    cout << "   ⠀⠀⠀⠀⠀⠀⠀⠳⣘⡿⢦⡴⡺⣻⠁⡤⣮⠇⠀⠀⢀⡎⡌⡎⣟⢻⢃⠗⢺⣃⡷⣧⢇⢇⢣⠀⠀⠀⡼⣵⢤⠨⣾⣗⢤⠾⣿⣠⠋⠀⠀⠀⠀⠀⠀⠀" << endl;
    cout << "   ⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢾⣟⣡⢪⣾⣿⣽⣒⠤⣼⢸⢹⢰⡸⡎⡏⢦⢼⢿⡇⡞⡘⡜⣜⣦⠤⣚⣽⡻⣦⢕⣌⣯⠿⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀" << endl;
    cout << "   ⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠈⠛⠙⠫⢿⡿⣿⣷⣦⣯⣽⣻⣷⠧⢷⣤⡼⣼⣼⣿⣓⣏⣥⣲⣀⠘⣿⡯⠛⠉⠛⠁⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀" << endl;
    cout << "⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠈⢑⡿⡯⣞⣿⣿⣾⡆⠈⣏⢯⣿⣿⣷⣟⣿⡿⣿⢿⠉⠁⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀" << endl;
    cout << "   ⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢀⡼⣜⣱⣣⢣⢫⢋⡽⡹⢹⠉⡏⣏⠹⣱⡱⣼⣌⡪⢇⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀" << endl;
    cout << "   ⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠈⠉⠑⠛⠛⠛⠓⣿⣳⠵⠋⠓⢼⣟⡟⠓⠛⠓⠓⠉⠉⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀" << endl;
    cout << "        SISTEM VOTING PEMILU PRESIDEN 2029        \n";
    cout << string(60, '=') << "\n";
    cout << "│                                                          │\n";
    cout << "│                  1. Lihat Daftar Kandidat                │\n";
    cout << "│                  2. Lakukan Voting                       │\n";
    cout << "│                  3. Analisis Hasil Voting                │\n";
    cout << "│                  4. Menu Admin                           │\n";
    cout << "│                  5. Keluar                               │\n";
    cout << "│                                                          │\n";
    cout << string(60, '=') << "\n";
    cout << "Pilihan: ";
}

// Centralized NIK validation function
bool isValidNIK(const string& nik) {
    return nik.length() == 16 && all_of(nik.begin(), nik.end(), ::isdigit);
}

// Safe file writing function
bool safeWriteToFile(const string& filename, const string& content) {
    // Open file in append mode
    ofstream file(filename, ios::app);
    if (!file.is_open()) {
        return false;
    }
    
    // Append new content
    file << content;
    file.close();
    
    return true;
}

// Logging utility function
void logError(const string& msg) {
    ofstream logFile("error.log", ios::app);
    if (logFile.is_open()) {
        auto now = chrono::system_clock::now();
        auto time = chrono::system_clock::to_time_t(now);
        stringstream ss;
        ss << put_time(localtime(&time), "%Y-%m-%d %H:%M:%S");
        logFile << "[" << ss.str() << "] ERROR: " << msg << "\n";
        logFile.close();
    }
}