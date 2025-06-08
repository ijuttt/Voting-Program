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

using namespace std;

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

// Struktur untuk kandidat
struct Kandidat {
    int nomor;
    string nama;
    string partai;
};

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
                    k.nomor = stoi(line.substr(0, pos1));
                    k.nama = line.substr(pos1 + 1, pos2 - pos1 - 1);
                    k.partai = line.substr(pos2 + 1);
                    kandidat.push_back(k);
                }
            }
        }
        file.close();
    }
    catch (const exception& e) {
        cout << "Error: " << e.what() << endl;
        // Jika file tidak ada, buat kandidat default
        kandidat = {
            {1, "Joko Widodo", "PDI-P"},
            {2, "Prabowo Subianto", "Gerindra"},
            {3, "Anies Baswedan", "NasDem"}
        };
        
        // Simpan ke file
        ofstream outFile("kandidat.txt");
        for (const auto& k : kandidat) {
            outFile << k.nomor << "|" << k.nama << "|" << k.partai << "\n";
        }
        outFile.close();
    }
    
    return kandidat;
}

// Fungsi untuk menyimpan vote terenkripsi
void simpanVote(int pilihan, const string& waktu) {
    try {
        // Generate kunci enkripsi acak
        random_device rd;
        mt19937 gen(rd());
        uniform_int_distribution<> dis(1, 25);
        int key = dis(gen);
        
        // Data yang akan dienkripsi: pilihan|waktu
        string data = to_string(pilihan) + "|" + waktu;
        string encryptedData = encryptData(data, key);
        
        // Simpan ke file dengan kunci terpisah
        ofstream voteFile("votes.dat", ios::app);
        ofstream keyFile("keys.dat", ios::app);
        
        if (!voteFile.is_open() || !keyFile.is_open()) {
            throw runtime_error("Tidak dapat membuka file untuk menyimpan vote!");
        }
        
        voteFile << encryptedData << "\n";
        keyFile << key << "\n";
        
        voteFile.close();
        keyFile.close();
        
        cout << "Vote berhasil disimpan dengan enkripsi!\n";
    }
    catch (const exception& e) {
        cout << "Error saat menyimpan vote: " << e.what() << endl;
    }
}

// Fungsi untuk membaca dan menghitung votes
map<int, int> hitungVotes() {
    map<int, int> hasilVote;
    
    try {
        ifstream voteFile("votes.dat");
        ifstream keyFile("keys.dat");
        
        if (!voteFile.is_open() || !keyFile.is_open()) {
            throw runtime_error("File vote tidak ditemukan atau tidak dapat dibuka!");
        }
        
        string encryptedLine;
        string keyLine;
        
        // Menggunakan iterator untuk membaca file
        while (getline(voteFile, encryptedLine) && getline(keyFile, keyLine)) {
            if (!encryptedLine.empty() && !keyLine.empty()) {
                int key = stoi(keyLine);
                string decryptedData = decryptData(encryptedLine, key);
                
                // Parse data: pilihan|waktu
                size_t pos = decryptedData.find('|');
                if (pos != string::npos) {
                    int pilihan = stoi(decryptedData.substr(0, pos));
                    hasilVote[pilihan]++;
                }
            }
        }
        
        voteFile.close();
        keyFile.close();
    }
    catch (const exception& e) {
        cout << "Error saat membaca votes: " << e.what() << endl;
    }
    
    return hasilVote;
}

// Lambda expression untuk mendapatkan waktu saat ini
auto getCurrentTime = []() -> string {
    time_t now = time(0);
    string timeStr = ctime(&now);
    timeStr.pop_back(); // Hapus newline
    return timeStr;
};

// Fungsi untuk menampilkan kandidat menggunakan STL algorithms
void tampilkanKandidat(vector<Kandidat>& kandidat) {
    cout << "\n=== DAFTAR KANDIDAT PRESIDEN ===\n";
    
    // Sort kandidat berdasarkan nomor urut
    sort(kandidat.begin(), kandidat.end(), 
         [](const Kandidat& a, const Kandidat& b) {
             return a.nomor < b.nomor;
         });
    
    // Tampilkan kandidat
    for_each(kandidat.begin(), kandidat.end(), 
             [](const Kandidat& k) {
                 cout << k.nomor << ". " << k.nama << " (" << k.partai << ")\n";
             });
}

// Fungsi untuk voting
void lakukanVoting(const vector<Kandidat>& kandidat) {
    cout << "\n=== VOTING ===\n";
    cout << "Masukkan nomor pilihan Anda: ";
    
    int pilihan;
    cin >> pilihan;
    
    // Find kandidat yang dipilih menggunakan STL algorithm
    auto it = find_if(kandidat.begin(), kandidat.end(),
                      [pilihan](const Kandidat& k) {
                          return k.nomor == pilihan;
                      });
    
    if (it != kandidat.end()) {
        cout << "Anda memilih: " << it->nama << " (" << it->partai << ")\n";
        cout << "Konfirmasi pilihan? (y/n): ";
        
        char konfirmasi;
        cin >> konfirmasi;
        
        if (konfirmasi == 'y' || konfirmasi == 'Y') {
            string waktu = getCurrentTime();
            simpanVote(pilihan, waktu);
            cout << "Terima kasih! Vote Anda telah tercatat.\n";
        } else {
            cout << "Vote dibatalkan.\n";
        }
    } else {
        cout << "Nomor kandidat tidak valid!\n";
    }
}

// Fungsi untuk menampilkan hasil voting
void tampilkanHasil(const vector<Kandidat>& kandidat) {
    cout << "\n=== HASIL VOTING ===\n";
    
    map<int, int> hasilVote = hitungVotes();
    
    if (hasilVote.empty()) {
        cout << "Belum ada vote yang tercatat.\n";
        return;
    }
    
    // Hitung total votes menggunakan STL count
    int totalVotes = 0;
    for (const auto& pair : hasilVote) {
        totalVotes += pair.second;
    }
    
    cout << "Total Votes: " << totalVotes << "\n\n";
    
    // Tampilkan hasil untuk setiap kandidat
    for (const auto& k : kandidat) {
        int votes = hasilVote[k.nomor];
        double persentase = totalVotes > 0 ? (double)votes / totalVotes * 100 : 0;
        
        cout << k.nomor << ". " << k.nama << " (" << k.partai << ")\n";
        cout << "   Votes: " << votes << " (" << persentase << "%)\n";
        cout << "   Progress: ";
        
        // Tampilkan bar progress sederhana
        int barLength = (int)(persentase / 5); // 1 karakter = 5%
        for (int i = 0; i < barLength; i++) cout << "█";
        cout << "\n\n";
    }
    
    // Cari pemenang menggunakan STL max_element
    if (!hasilVote.empty()) {
        auto pemenang = max_element(hasilVote.begin(), hasilVote.end(),
                                   [](const pair<int, int>& a, const pair<int, int>& b) {
                                       return a.second < b.second;
                                   });
        
        auto kandidatPemenang = find_if(kandidat.begin(), kandidat.end(),
                                       [pemenang](const Kandidat& k) {
                                           return k.nomor == pemenang->first;
                                       });
        
        if (kandidatPemenang != kandidat.end()) {
            cout << "🏆 PEMENANG SEMENTARA: " << kandidatPemenang->nama 
                 << " dengan " << pemenang->second << " votes!\n";
        }
    }
}

// Menu utama
void tampilkanMenu() {
    cout << "\n=== SISTEM VOTING PEMILU PRESIDEN ===\n";
    cout << "1. Lihat Daftar Kandidat\n";
    cout << "2. Lakukan Voting\n";
    cout << "3. Lihat Hasil Voting\n";
    cout << "4. Keluar\n";
    cout << "Pilih menu (1-4): ";
}

int main() {
    cout << "Selamat datang di Sistem Voting Pemilu Presiden!\n";
    cout << "Program ini menggunakan enkripsi sederhana untuk menjaga kerahasiaan vote.\n";
    
    vector<Kandidat> kandidat;
    
    try {
        kandidat = bacaKandidat();
        cout << "Data kandidat berhasil dimuat.\n";
    }
    catch (const exception& e) {
        cout << "Error: " << e.what() << endl;
        return 1;
    }
    
    int pilihan;
    
    do {
        tampilkanMenu();
        cin >> pilihan;
        
        try {
            switch (pilihan) {
                case 1:
                    tampilkanKandidat(kandidat);
                    break;
                case 2:
                    lakukanVoting(kandidat);
                    break;
                case 3:
                    tampilkanHasil(kandidat);
                    break;
                case 4:
                    cout << "Terima kasih telah menggunakan sistem voting!\n";
                    break;
                default:
                    throw invalid_argument("Pilihan tidak valid! Masukkan angka 1-4.");
            }
        }
        catch (const exception& e) {
            cout << "Error: " << e.what() << endl;
        }
        
        if (pilihan != 4) {
            cout << "\nTekan Enter untuk melanjutkan...";
            cin.ignore();
            cin.get();
        }
        
    } while (pilihan != 4);
    
    return 0;
}

/* 
PENJELASAN FITUR YANG DIGUNAKAN:

1. EXCEPTION HANDLING:
   - try-catch blocks untuk menangani error file dan input
   - throw runtime_error dan invalid_argument

2. STL CONTAINERS:
   - vector<Kandidat> untuk menyimpan data kandidat
   - map<int, int> untuk menghitung hasil voting

3. ITERATOR:
   - kandidat.begin(), kandidat.end()
   - Digunakan dalam find_if, sort, max_element

4. SORT:
   - sort() dengan lambda untuk mengurutkan kandidat

5. FIND:
   - find_if() untuk mencari kandidat berdasarkan nomor

6. COUNT:
   - Manual counting dalam loop untuk total votes

7. FILE HANDLING:
   - ifstream dan ofstream untuk baca/tulis file
   - File kandidat.txt, votes.dat, keys.dat

8. LAMBDA EXPRESSION:
   - getCurrentTime lambda untuk mendapatkan waktu
   - Lambda dalam sort, find_if, max_element, for_each

ENKRIPSI:
- Menggunakan Caesar Cipher sederhana dengan kunci acak
- Setiap vote dienkripsi dengan kunci berbeda
- Kunci disimpan terpisah dari data vote
- Admin tidak bisa melihat siapa voting apa karena data terenkripsi

CARA PENGGUNAAN:
1. Compile: g++ -o voting voting.cpp
2. Run: ./voting
3. File akan otomatis dibuat saat pertama kali dijalankan
4. Vote tersimpan terenkripsi di votes.dat
5. Kunci enkripsi tersimpan di keys.dat

UNTUK NETWORKING (KOMPUTER LAIN):
- Program ini bisa dikembangkan dengan socket programming
- Atau bisa menggunakan shared folder di network
- File votes.dat bisa diakses dari komputer lain via network drive
*/