#include <iostream>
#include <string>
#include <thread>
#include <chrono>

void user_menu(){
    using namespace std;
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
    cout << "                     SISTEM VOTING PEMILU PRESIDEN 2029        \n";
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
bool login_admin(){
    using namespace std;
    string password;
    string username;
    system("cls");
    cout << "Masukkan Username: ";
    cin >> username;
    cout << "Masukkan Password: ";
    cin >> password;
    if (username == "admin" && password == "admin123"){
        return 1;
    }
    else{
        cout << "Username atau Password salah" << endl;
        this_thread::sleep_for(chrono::seconds(1));
        return 0;
    }
}

int admin_menu(){
    using namespace std;
    system("cls");
    login_admin();
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

void tungguInput(){
    using namespace std;
    cout << "Tekan Enter untuk kembali...";
    cin.get();
}

int main() {  
    using namespace std;
    user_menu();
    string pilihan;
    cin >> pilihan;
    if (pilihan == "1"){
        cout << "Daftar Kandidat" << endl;
    }
    else if (pilihan == "2"){
        cout << "Lakukan Voting" << endl;
    }
    else if (pilihan == "3"){
        cout << "Analisis Hasil Voting" << endl;
    }
    else if (pilihan == "4"){
        admin_menu();
        tungguInput();
    }
    else if (pilihan == "5"){
    return 1;
    }
}