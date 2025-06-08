#include <iostream>
#include <fstream>
#include <iomanip>
#include <vector>
#include <map>
#include <string>
#include <ctime>
#include <sstream>
#include <algorithm>
#include "voting_functions.h"

// Struktur data global
BSTNode* pohonPemilih = nullptr;
AVLNode* statistikSuara = nullptr;
std::map<std::string, std::vector<std::string>> jaringanPemilih;
std::vector<HashEntry> tabelHash;
std::vector<std::pair<std::string, std::string>> daftarKandidat;

// Konstanta
const std::string FILE_PEMILIHAN = "pemilihan.dat";
const std::string FILE_KANDIDAT = "kandidat.txt";
const std::string DIR_BACKUP = "backup";
const int KEY_ENKRIPSI = 7;

// Struktur untuk data pemilihan
struct DataPemilihan {
    std::string idPemilih;
    int idKandidat;
    std::string waktu;
    
    DataPemilihan(const std::string& id, int kandidat) 
        : idPemilih(id), idKandidat(kandidat) {
        waktu = getCurrentTime();
    }
    
    std::string toString() const {
        std::stringstream ss;
        ss << idPemilih << "|" << idKandidat << "|" << waktu;
        return encryptData(ss.str(), KEY_ENKRIPSI);
    }
    
    static DataPemilihan fromString(const std::string& str) {
        std::string decrypted = decryptData(str, KEY_ENKRIPSI);
        std::stringstream ss(decrypted);
        std::string id, waktu;
        int kandidat;
        
        std::getline(ss, id, '|');
        ss >> kandidat;
        ss.ignore();
        std::getline(ss, waktu);
        
        DataPemilihan data(id, kandidat);
        data.waktu = waktu;
        return data;
    }
};

// Fungsi-fungsi utilitas
void tampilkanMenu() {
    std::cout << "\n=== MENU PEMILIHAN ===\n";
    std::cout << "1. Daftar Pemilih\n";
    std::cout << "2. Masuk ke Bilik Suara\n";
    std::cout << "3. Lihat Hasil Pemilihan\n";
    std::cout << "4. Menu Admin\n";
    std::cout << "0. Keluar\n";
    std::cout << "Pilihan: ";
}

void tampilkanMenuAdmin() {
    std::cout << "\n=== MENU ADMIN ===\n";
    std::cout << "1. Visualisasi Data\n";
    std::cout << "2. Analisis Jaringan\n";
    std::cout << "3. Backup Data\n";
    std::cout << "4. Pulihkan Backup\n";
    std::cout << "0. Kembali\n";
    std::cout << "Pilihan: ";
}

void bacaKandidat() {
    std::ifstream file(FILE_KANDIDAT);
    if (!file) {
        writeLog("ERROR", "File kandidat tidak dapat dibuka");
        return;
    }
    
    std::string line;
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string id, nama;
        std::getline(ss, id, '|');
        std::getline(ss, nama);
        daftarKandidat.push_back({id, nama});
    }
}

void daftarPemilih() {
    std::string nama;
    std::cout << "\nMasukkan nama lengkap: ";
    std::getline(std::cin, nama);
    
    // Generate ID unik
    std::stringstream ss;
    ss << "V" << std::time(nullptr) << rand() % 1000;
    std::string id = ss.str();
    
    // Simpan ke BST dan Hash Table
    pohonPemilih = insertBST(pohonPemilih, id, nama);
    insertHash(tabelHash, id, nama);
    
    // Tambahkan ke jaringan sosial
    addVoterToGraph(jaringanPemilih, id);
    
    std::cout << "\nPendaftaran berhasil!\n";
    std::cout << "ID Pemilih Anda: " << id << "\n";
    writeLog("INFO", "Pemilih baru terdaftar: " + id);
}

void simpanPemilihan(const DataPemilihan& data) {
    try {
        // Validasi pemilih
        if (!findBST(pohonPemilih, data.idPemilih) || !findHash(tabelHash, data.idPemilih)) {
            throw std::runtime_error("ID Pemilih tidak valid");
        }
        
        // Simpan ke file
        std::ofstream file(FILE_PEMILIHAN, std::ios::app);
        if (!file) {
            throw std::runtime_error("File pemilihan tidak dapat dibuka");
        }
        
        file << data.toString() << std::endl;
        
        // Update statistik
        statistikSuara = insertAVL(statistikSuara, data.idKandidat, 1);
        
        // Update jaringan sosial
        for (const auto& kandidat : daftarKandidat) {
            if (std::to_string(data.idKandidat) == kandidat.first) {
                addConnection(jaringanPemilih, data.idPemilih, kandidat.first);
                break;
            }
        }
        
        writeLog("INFO", "Suara berhasil disimpan untuk pemilih: " + data.idPemilih);
        
    } catch (const std::exception& e) {
        writeLog("ERROR", "Gagal menyimpan suara: " + std::string(e.what()));
        throw;
    }
}

void bacaSemuaPemilihan() {
    std::ifstream file(FILE_PEMILIHAN);
    if (!file) {
        writeLog("ERROR", "File pemilihan tidak dapat dibuka");
        return;
    }
    
    std::string line;
    while (std::getline(file, line)) {
        try {
            DataPemilihan data = DataPemilihan::fromString(line);
            statistikSuara = insertAVL(statistikSuara, data.idKandidat, 1);
        } catch (const std::exception& e) {
            writeLog("ERROR", "Gagal membaca data: " + std::string(e.what()));
        }
    }
}

void tampilkanHasil() {
    std::cout << "\n=== HASIL PEMILIHAN ===\n";
    
    // Konversi data AVL ke vector untuk sorting
    std::vector<std::pair<int, int>> hasil;
    std::function<void(AVLNode*)> collectVotes = [&](AVLNode* node) {
        if (!node) return;
        collectVotes(node->left);
        hasil.push_back({node->kandidatId, node->voteCount});
        collectVotes(node->right);
    };
    collectVotes(statistikSuara);
    
    // Sorting dengan berbagai algoritma
    std::vector<std::pair<int, int>> hasilQuick = hasil;
    std::vector<std::pair<int, int>> hasilMerge = hasil;
    std::vector<std::pair<int, int>> hasilHeap = hasil;
    
    quickSort(hasilQuick, 0, hasilQuick.size() - 1);
    mergeSort(hasilMerge, 0, hasilMerge.size() - 1);
    heapSort(hasilHeap);
    
    // Tampilkan hasil
    std::cout << "\nHasil (Quick Sort):\n";
    for (const auto& pair : hasilQuick) {
        auto it = std::find_if(daftarKandidat.begin(), daftarKandidat.end(),
            [&](const auto& k) { return std::to_string(pair.first) == k.first; });
        if (it != daftarKandidat.end()) {
            std::cout << it->second << ": " << pair.second << " suara\n";
        }
    }
    
    // Analisis jaringan
    std::cout << "\nAnalisis Jaringan Pemilih:\n";
    auto components = findStronglyConnectedComponents(jaringanPemilih);
    std::cout << "Jumlah cluster pemilih: " << components.size() << "\n";
    
    for (size_t i = 0; i < components.size(); i++) {
        std::cout << "Cluster " << (i + 1) << ":\n";
        for (const auto& voter : components[i]) {
            std::cout << "  " << voter << "\n";
        }
    }
}

void tampilkanVisualisasi() {
    std::cout << "\n=== VISUALISASI DATA ===\n";
    
    std::cout << "\nPohon Pemilih (BST):\n";
    visualizeBST(pohonPemilih, 0);
    
    std::cout << "\nStatistik Suara (AVL):\n";
    visualizeAVL(statistikSuara, 0);
    
    std::cout << "\nJaringan Pemilih:\n";
    visualizeGraph(jaringanPemilih);
}

void tampilkanAnalisisJaringan() {
    std::cout << "\n=== ANALISIS JARINGAN ===\n";
    
    std::string start, end;
    std::cout << "Masukkan ID Pemilih awal: ";
    std::getline(std::cin, start);
    std::cout << "Masukkan ID Pemilih tujuan: ";
    std::getline(std::cin, end);
    
    auto path = findShortestPath(jaringanPemilih, start, end);
    if (path.empty()) {
        std::cout << "Tidak ada jalur yang ditemukan\n";
    } else {
        std::cout << "Jalur terpendek:\n";
        for (size_t i = 0; i < path.size(); i++) {
            std::cout << path[i];
            if (i < path.size() - 1) std::cout << " -> ";
        }
        std::cout << "\n";
    }
    
    auto components = findStronglyConnectedComponents(jaringanPemilih);
    std::cout << "\nKomponen Terhubung Kuat:\n";
    for (size_t i = 0; i < components.size(); i++) {
        std::cout << "Komponen " << (i + 1) << ":\n";
        for (const auto& voter : components[i]) {
            std::cout << "  " << voter << "\n";
        }
    }
}

int main() {
    // Inisialisasi
    srand(time(nullptr));
    initHashTable(tabelHash, 1000);
    bacaKandidat();
    bacaSemuaPemilihan();
    
    std::cout << "SELAMAT DATANG DI SISTEM PEMILIHAN PRESIDEN 2024\n";
    
    int pilihan;
    do {
        tampilkanMenu();
        std::cin >> pilihan;
        std::cin.ignore();
        
        try {
            switch (pilihan) {
                case 1:
                    daftarPemilih();
                    break;
                    
                case 2: {
                    std::string id;
                    std::cout << "\nMasukkan ID Pemilih: ";
                    std::getline(std::cin, id);
                    
                    if (!findBST(pohonPemilih, id) || !findHash(tabelHash, id)) {
                        std::cout << "ID Pemilih tidak valid!\n";
                        break;
                    }
                    
                    std::cout << "\nPilih Kandidat:\n";
                    for (const auto& kandidat : daftarKandidat) {
                        std::cout << kandidat.first << ". " << kandidat.second << "\n";
                    }
                    
                    int pilihanKandidat;
                    std::cout << "Pilihan: ";
                    std::cin >> pilihanKandidat;
                    std::cin.ignore();
                    
                    DataPemilihan data(id, pilihanKandidat);
                    simpanPemilihan(data);
                    std::cout << "Terima kasih telah menggunakan hak pilih Anda!\n";
                    break;
                }
                
                case 3:
                    tampilkanHasil();
                    break;
                    
                case 4: {
                    int pilihanAdmin;
                    do {
                        tampilkanMenuAdmin();
                        std::cin >> pilihanAdmin;
                        std::cin.ignore();
                        
                        switch (pilihanAdmin) {
                            case 1:
                                tampilkanVisualisasi();
                                break;
                                
                            case 2:
                                tampilkanAnalisisJaringan();
                                break;
                                
                            case 3:
                                createBackup(FILE_PEMILIHAN, DIR_BACKUP);
                                std::cout << "Backup berhasil dibuat\n";
                                break;
                                
                            case 4: {
                                std::string backupFile;
                                std::cout << "Masukkan nama file backup: ";
                                std::getline(std::cin, backupFile);
                                restoreBackup(backupFile, FILE_PEMILIHAN);
                                std::cout << "Backup berhasil dipulihkan\n";
                                break;
                            }
                        }
                    } while (pilihanAdmin != 0);
                    break;
                }
            }
        } catch (const std::exception& e) {
            std::cout << "Error: " << e.what() << "\n";
            writeLog("ERROR", e.what());
        }
        
    } while (pilihan != 0);
    
    // Cleanup
    freeBST(pohonPemilih);
    freeAVL(statistikSuara);
    
    return 0;
}