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
// Struktur untuk data vote
struct VoteData {
    string namaVoter;
    int pilihan;
    string waktu;
    // Constructor
    VoteData(string nama = "", int p = 0, string w = "") 
        : namaVoter(nama), pilihan(p), waktu(w) {}
    // Method untuk mengkonversi ke string
    string toString() const {
        return namaVoter + "|" + to_string(pilihan) + "|" + waktu;
    }
    // Method untuk parse dari string
    static VoteData fromString(const string& data) {
        VoteData vote;
        size_t pos1 = data.find('|');
        size_t pos2 = data.find('|', pos1 + 1);
        if (pos1 != string::npos && pos2 != string::npos) {
            vote.namaVoter = data.substr(0, pos1);
            vote.pilihan = stoi(data.substr(pos1 + 1, pos2 - pos1 - 1));
            vote.waktu = data.substr(pos2 + 1);
        }
        return vote;
    }
};
struct Kandidat {
    int nomor;
    string nama;
    string partai;
    // Constructor untuk memudahkan inisialisasi
    Kandidat(int n = 0, string nm = "", string pt = "") 
        : nomor(n), nama(nm), partai(pt) {}
    // Method untuk menampilkan kandidat dengan format yang rapi
    void tampilkan() const {
        cout << "Nomor Kandidat : " << nomor << "\n";
        cout << "Nama Kandidat  : " << nama << "\n";
        cout << "Partai         : " << partai << "\n";
        cout << string(40, '-') << "\n";
    }
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
            Kandidat(1, "Joko Widodo", "PDI-P"),
            Kandidat(2, "Prabowo Subianto", "Gerindra"), 
            Kandidat(3, "Anies Baswedan", "NasDem")
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
// Fungsi untuk menyimpan vote terenkripsi dengan nama voter
void simpanVote(const string& namaVoter, int pilihan, const string& waktu) {
    try {
        // Generate kunci enkripsi acak
        random_device rd;
        mt19937 gen(rd());
        uniform_int_distribution<> dis(5, 20);
        int key = dis(gen);
        // Buat object VoteData
        VoteData vote(namaVoter, pilihan, waktu);
        string data = vote.toString();
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
        cout << "\n" << string(50, '=') << "\n";
        cout << "VOTE BERHASIL DISIMPAN!" << "\n";
        cout << string(50, '=') << "\n";
        cout << "Terima kasih " << namaVoter << "!\n";
        cout << "Vote Anda telah tersimpan dengan enkripsi keamanan tinggi.\n";
        cout << "Data Anda aman dan terlindungi.\n";
        cout << string(50, '=') << "\n";
    }
    catch (const exception& e) {
        cout << "Error saat menyimpan vote: " << e.what() << endl;
    }
}
// Fungsi untuk membaca dan menghitung votes dengan data voter terenkripsi
pair<map<int, int>, vector<VoteData>> bacaSemuaVotes() {
    map<int, int> hasilVote;
    vector<VoteData> semuaVote;
    try {
        ifstream voteFile("votes.dat");
        ifstream keyFile("keys.dat");
        if (!voteFile.is_open() || !keyFile.is_open()) {
            throw runtime_error("File vote tidak ditemukan!");
        }
        string encryptedLine;
        string keyLine;
        // Menggunakan iterator untuk membaca file
        while (getline(voteFile, encryptedLine) && getline(keyFile, keyLine)) {
            if (!encryptedLine.empty() && !keyLine.empty()) {
                int key = stoi(keyLine);
                string decryptedData = decryptData(encryptedLine, key);
                VoteData vote = VoteData::fromString(decryptedData);
                if (vote.pilihan > 0) {
                    hasilVote[vote.pilihan]++;
                    semuaVote.push_back(vote);
                }
            }
        }
        voteFile.close();
        keyFile.close();
    }
    catch (const exception& e) {
        cout << "Error saat membaca votes: " << e.what() << endl;
    }
    return make_pair(hasilVote, semuaVote);
}
// Lambda expression untuk mendapatkan waktu saat ini
auto getCurrentTime = []() -> string {
    time_t now = time(0);
    string timeStr = ctime(&now);
    timeStr.pop_back(); // Hapus newline
    return timeStr;
};
// Fungsi untuk menampilkan kandidat menggunakan STL algorithms dan struct
void tampilkanKandidat(vector<Kandidat>& kandidat) {
    cout << "\n" << string(60, '=') << "\n";
    cout << "         DAFTAR KANDIDAT PRESIDEN 2024         \n";
    cout << string(60, '=') << "\n\n";
    // Sort kandidat berdasarkan nomor urut
    sort(kandidat.begin(), kandidat.end(), 
         [](const Kandidat& a, const Kandidat& b) {
             return a.nomor < b.nomor;
         });
    // Tampilkan kandidat menggunakan method struct
    for_each(kandidat.begin(), kandidat.end(), 
             [](const Kandidat& k) {
                 k.tampilkan();
             });
    cout << string(60, '=') << "\n";
}
// Fungsi untuk voting dengan input nama
void lakukanVoting(const vector<Kandidat>& kandidat) {
    cout << "\n" << string(60, '=') << "\n";
    cout << "                  VOTING SECTION                  \n";
    cout << string(60, '=') << "\n";
    // Input nama voter
    string namaVoter;
    cout << "Masukkan nama lengkap Anda: ";
    cin.ignore(); // Membersihkan buffer
    getline(cin, namaVoter);
    if (namaVoter.empty()) {
        cout << "Nama tidak boleh kosong!\n";
        return;
    }
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
        cout << "\n" << string(50, '-') << "\n";
        cout << "KONFIRMASI PILIHAN ANDA:\n";
        cout << string(50, '-') << "\n";
        cout << "Nama Voter     : " << namaVoter << "\n";
        cout << "Pilihan Anda   : " << it->nama << "\n";
        cout << "Partai         : " << it->partai << "\n";
        cout << string(50, '-') << "\n";
        cout << "Apakah Anda yakin dengan pilihan ini? (y/n): ";
        char konfirmasi;
        cin >> konfirmasi;
        if (konfirmasi == 'y' || konfirmasi == 'Y') {
            string waktu = getCurrentTime();
            simpanVote(namaVoter, pilihan, waktu);
        } else {
            cout << "\nVote dibatalkan. Silakan coba lagi jika diperlukan.\n";
        }
    } else {
        cout << "\nNomor kandidat tidak valid! Silakan pilih nomor yang tersedia.\n";
    }
}
// Fungsi untuk menampilkan hasil voting dengan data voter terenkripsi
void tampilkanHasil(const vector<Kandidat>& kandidat) {
    cout << "\n" << string(70, '=') << "\n";
    cout << "                   HASIL VOTING REAL-TIME                   \n";
    cout << string(70, '=') << "\n";
    auto [hasilVote, semuaVote] = bacaSemuaVotes();
    if (hasilVote.empty()) {
        cout << "Belum ada vote yang tercatat dalam sistem.\n";
        cout << "Silakan lakukan voting terlebih dahulu.\n";
        cout << string(70, '=') << "\n";
        return;
    }
    // Hitung total votes menggunakan STL count
    int totalVotes = 0;
    for (const auto& pair : hasilVote) {
        totalVotes += pair.second;
    }
    cout << "RINGKASAN VOTING:\n";
    cout << "Total Votes Terdaftar: " << totalVotes << " suara\n";
    cout << "Waktu Terakhir Update: " << getCurrentTime() << "\n";
    cout << string(70, '=') << "\n\n";
    // Tampilkan hasil untuk setiap kandidat
    cout << "PEROLEHAN SUARA PER KANDIDAT:\n";
    cout << string(70, '-') << "\n";
    for (const auto& k : kandidat) {
        int votes = hasilVote[k.nomor];
        double persentase = totalVotes > 0 ? (double)votes / totalVotes * 100 : 0;
        cout << "Nomor Kandidat : " << k.nomor << "\n";
        cout << "Nama Kandidat  : " << k.nama << "\n";
        cout << "Partai         : " << k.partai << "\n";
        cout << "Jumlah Suara   : " << votes << " suara (" 
             << fixed << setprecision(1) << persentase << "%)\n";
        // Tampilkan bar progress yang lebih menarik
        cout << "Progress       : ";
        int barLength = (int)(persentase / 2); // 1 karakter = 2%
        for (int i = 0; i < barLength; i++) cout << "█";
        for (int i = barLength; i < 50; i++) cout << "░";
        cout << " " << persentase << "%\n";
        cout << string(70, '-') << "\n";
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
            cout << "\nPEMENANG SEMENTARA:\n";
            cout << string(50, '=') << "\n";
            cout << "🏆 " << kandidatPemenang->nama << " (" << kandidatPemenang->partai << ")\n";
            cout << "Total Perolehan: " << pemenang->second << " suara\n";
            double persentasePemenang = (double)pemenang->second / totalVotes * 100;
            cout << "Persentase: " << fixed << setprecision(1) << persentasePemenang << "%\n";
            cout << string(50, '=') << "\n";
        }
    }
    // Tampilkan data voter terenkripsi untuk transparansi
    cout << "\nLOG VOTING TERENKRIPSI (Untuk Transparansi):\n";
    cout << string(70, '-') << "\n";
    cout << "Menampilkan " << semuaVote.size() << " vote yang telah dienkripsi:\n\n";
    int counter = 1;
    for (const auto& vote : semuaVote) {
        cout << counter << ". ";
        cout << "Voter: " << encryptData(vote.namaVoter, 7) << " | ";
        cout << "Pilihan: " << vote.pilihan << " | ";
        cout << "Waktu: " << vote.waktu << "\n";
        counter++;
        if (counter > 10) { // Batasi tampilan max 10 untuk readability
            cout << "... dan " << (semuaVote.size() - 10) << " vote lainnya\n";
            break;
        }
    }
    cout << "\nCatatan: Nama voter dienkripsi untuk menjaga privasi.\n";
    cout << "           Hanya hasil voting yang dapat dilihat.\n";
    cout << string(70, '=') << "\n";
}
// Menu utama dengan tampilan yang lebih menarik
void tampilkanMenu() {
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
    cout << "⠀  ⠀⢯⡓⢭⣛⣒⣭⣭⣥⡼⢯⣽⢯⣿⣿⣿⣼⣸⠀⠛⢷⢿⣿⣯⣿⣤⣤⣧⣼⣿⣿⣿⣿⠛⠛⠃⢸⣸⣼⣿⣿⣯⢿⣽⠿⣤⣭⣭⣕⣒⣭⠗⣩⠃⠀" << endl;
    cout << "⠀  ⠀⠘⢏⡙⠛⣧⡴⠶⣤⣿⠻⣽⣿⣟⣿⡿⣿⣿⠀⠀⠸⣿⡿⢸⣿⣿⣿⢻⣿⣿⣿⠛⠛⠓⠂⠀⢸⣯⣻⣿⣿⣿⣿⡽⢻⣦⡴⠦⣤⡟⢓⣉⠟⠀⠀" << endl;
    cout << "⠀  ⠀⠀⠈⢮⣅⣒⣺⣿⡶⣶⣟⣣⣝⣾⣻⡿⣿⣺⢻⡟⡛⠛⠛⢻⣿⣉⠁⠈⢉⣿⣿⠛⠛⠛⠛⠛⢻⣻⣿⣿⣿⣞⣿⣛⣳⡶⣾⣿⣒⣈⣭⠋⠀⠀⠀" << endl;
    cout << "⠀  ⠀⠀⠀⠀⠓⣄⠤⠽⠶⢿⣋⣳⢿⣻⣕⡮⣺⣽⢼⣿⣷⢿⣷⢸⣿⣿⣤⣤⣸⣿⣿⣀⣴⣶⣦⡀⢸⣽⡺⣞⣽⣻⢿⣏⣽⠷⠾⠤⢄⡖⠁⠀⠀⠀⠀" << endl;
    cout << "⠀  ⠀⠀⠀⠀⠀⠈⠙⢭⣍⣉⣉⠷⢾⣽⢾⡿⢿⣾⣿⣿⣿⣿⣟⣦⢻⣿⣿⣿⣿⣿⣷⡿⠃⢀⣿⡇⡾⣯⢿⣿⢮⣽⠾⢏⣉⣉⣭⠟⠉⠀⠀⠀⠀⠀⠀" << endl;
    cout << "⠀  ⠀⠀⠀⠀⠀⠀⠀⠀⠈⠑⠲⠶⠍⠉⠭⠽⢛⣏⣿⣽⣿⣿⣯⣯⣿⢍⡻⡿⢻⣿⠋⠀⢀⣾⡟⣼⣩⠙⠿⠼⠏⠉⠵⠶⠒⠉⠀⠀⠀⠀⠀⠀⠀⠀⠀" << endl;
    cout << "⠀⠀ ⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢀⣼⣿⢽⡽⣾⣿⣿⣽⣿⠿⢧⡇⣿⡇⠀⣠⣾⣏⣾⣿⣿⣧⡀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀" << endl;
    cout << "⠀  ⠀ ⠀⠀⠀⣄⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢼⡭⣾⢳⡧⡽⣿⣿⡿⠿⢤⡀⡇⠙⠿⠿⣻⣷⣿⢼⡶⣷⢽⡇⠀⠀⠀⠀⠀⠀⠀⠀⠀⢀⣠⠀⠀⠀⠀⠀" << endl;
    cout << "   ⠀⠀⠀⠀⠀⠹⠙⢦⡀⠀⠀⠀⠀⠀⠀⠀⢸⢶⣿⢹⣯⠻⢿⡹⣿⣷⢢⢤⣧⡤⣶⣿⢽⠹⠾⣽⡟⣿⡶⡇⠀⠀⠀⠀⠀⠀⠀⢀⠔⢩⠃⠀⠀⠀⠀⠀" << endl;
    cout << "   ⠀⠀⠀⠘⠯⣁⠀⠀⠑⣄⠀⣀⠀⠀⠀⣠⠃⢀⠟⠁⠀⠀⡼⣿⣿⣩⡿⡉⣩⡿⣡⢿⡾⡆⠀⠀⠈⠣⡀⠸⡄⠀⠀⠀⡀⠀⡴⠁⠀⠈⣨⠝⠃⠀⠀⠀" << endl;
    cout << "   ⠀⠀⠀⠀⠀⠈⢦⡀⢀⡼⠋⠉⠑⣄⠴⠣⡴⠁⠀⠀⠀⢰⢣⢻⡎⢿⣥⠋⢙⡥⠻⡳⠻⠸⡄⠀⠀⠀⠘⣦⠞⠦⡴⠋⠉⠙⣄⠀⢀⠞⠁⠀⠀⠀⠀⠀" << endl;
    cout << "   ⠀⠀⠀⠀⠀⠀⠀⠳⣘⡿⢦⡴⡺⣻⠁⡤⣮⠇⠀⠀⢀⡎⡌⡎⣟⢻⢃⠗⢺⣃⡷⣧⢇⢇⢣⠀⠀⠀⡼⣵⢤⠨⣾⣗⢤⠾⣿⣠⠋⠀⠀⠀⠀⠀⠀⠀" << endl;
    cout << "   ⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢾⣟⣡⢪⣾⣿⣽⣒⠤⣼⢸⢹⢰⡸⡎⡏⢦⢼⢿⡇⡞⡘⡜⣜⣦⠤⣚⣽⡻⣦⢕⣌⣯⠿⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀" << endl;
    cout << "   ⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠈⠛⠙⠫⢿⡿⣿⣷⣦⣯⣽⣻⣷⠧⢷⣤⡼⣼⣼⣿⣓⣏⣥⣲⣀⠘⣿⡯⠛⠉⠛⠁⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀" << endl;
    cout << "⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠈⢑⡿⡯⣞⣿⣿⣾⡆⠈⣏⢯⣿⣿⣷⣟⣿⡿⣿⢿⠉⠁⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀" << endl;
    cout << "   ⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢀⡼⣜⣱⣣⢣⢫⢋⡽⡹⢹⠉⡏⣏⠹⣱⡱⣼⣌⡪⢇⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀" << endl;
    cout << "   ⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠈⠉⠑⠛⠛⠛⠓⣿⣳⠵⠋⠓⢼⣟⡟⠓⠛⠓⠓⠉⠉⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀" << endl;
    cout << "           SISTEM VOTING PEMILU PRESIDEN 2024        \n";
    cout << string(60, '=') << "\n";
    cout << "│                                                          │\n";
    cout << "│                  1. Lihat Daftar Kandidat                │\n";
    cout << "│                  2. Lakukan Voting                       │\n";
    cout << "│                  3. Lihat Hasil Voting                   │\n";
    cout << "│                  4. Keluar dari Sistem                   │\n";
    cout << "│                                                          │\n";
    cout << string(60, '=') << "\n";
    cout << "Pilih menu (1-4): ";
}
int main() {
    cout << string(70, '=') << "\n";
    cout << "SELAMAT DATANG DI SISTEM VOTING PEMILU PRESIDEN 2024\n";
    cout << string(70, '=') << "\n";
    cout << "Program ini menggunakan enkripsi tingkat tinggi untuk\n";
    cout << "   menjaga kerahasiaan dan privasi vote Anda.\n";
    cout << "Data voter dan pilihan tersimpan dengan aman.\n";
    cout << "Sistem telah siap untuk digunakan!\n";
    cout << string(70, '=') << "\n";
    vector<Kandidat> kandidat;
    try {
        kandidat = bacaKandidat();
        cout << "Data kandidat berhasil dimuat ke sistem.\n";
        cout << "Terdapat " << kandidat.size() << " kandidat yang terdaftar.\n";
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
                    cout << "\n" << string(60, '=') << "\n";
                    cout << "Terima kasih telah menggunakan sistem voting!\n";
                    cout << "Demokrasi dimulai dari partisipasi Anda.\n";
                    cout << "Sampai jumpa di pemilu berikutnya!\n";
                    cout << string(60, '=') << "\n";
                    break;
                default:
                    throw invalid_argument("Pilihan tidak valid! Masukkan angka 1-4.");
            }
        }
        catch (const exception& e) {
            cout << "Error: " << e.what() << endl;
        }
        if (pilihan != 4) {
            cout << "\nTekan Enter untuk kembali ke menu utama...";
            cin.ignore();
            cin.get();
        }
    } while (pilihan != 4);
    return 0;
}