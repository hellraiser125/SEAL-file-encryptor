#include <iostream>
#include <fstream>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <string>
#include "SEAL.h"

using namespace std;

void generate_key(int* key, const int KEY_SIZE) {
    srand(time(nullptr));
    for (int i = 0; i < KEY_SIZE / sizeof(int); i++) {
        key[i] = rand();
    }
    // Get file path from user
    string file_path;
    cout << "Enter the file path to save the generated key: ";
    cin >> file_path;
    // Open file for writing
    ofstream outfile(file_path, ios::binary);
    if (!outfile) {
        cerr << "Failed to open file for writing." << endl;
        return;
    }
    // Write key to file
    outfile.write(reinterpret_cast<const char*>(key), KEY_SIZE);
    outfile.close();
    // Print key to console
    cout << "Generated key: ";
    for (int i = 0; i < KEY_SIZE / sizeof(int); i++) {
        cout << hex << key[i];
    }
    cout << "\nKey saved to file: " << file_path << "\n" << endl;
}

int encryptFile(string &input_file_path, string &key_file_path,string &output_file_path, int* key, int KEY_SIZE, int* &enc, int &L, int &n, char* buffer, int* text) {

    SEAL seal;

    cout << "Enter the path to the file you want to encrypt:" << endl;
    cin.ignore();
    getline(cin, input_file_path);
    //cout << input_file_path;
    ifstream input_file;
    input_file.open(input_file_path, ios::binary | ios::ate);
    if (!input_file.is_open()) {
        cout << "Error opening input file" << endl;
        return 1;
    }
    L = input_file.tellg(); // get file size in bytes
    input_file.seekg(0, ios::beg);
    buffer = new char[L];
    input_file.read(buffer, L);
    input_file.close();
    text = new int[ceil((float)L / 4)];
    // pack chars into ints without leading zeros
    for (int j = 0; j < ceil((float)L / 4); j++) {
        int tmp = 0;
        for (int k = 0; k < 4; k++) {
            int num = 4 * j + k;
            tmp = tmp << 8;
            if (num < L) // if there are still characters left in the buffer
                tmp += buffer[num];
        }
        text[j] = tmp;
    }

    cout << "Enter the file path to the key: ";
    cin >> key_file_path;
    // Open key file for reading
    ifstream key_file(key_file_path, ios::binary | ios::ate);
    if (!key_file.is_open()) {
        cerr << "Failed to open key file." << endl;
        cout << "Want to create new key?\n1.Yes\n2.No\n>>>";
        int newKeyGen = 0;
        cin >> newKeyGen;
        if (newKeyGen == 1) {
            generate_key(key, KEY_SIZE);
            cout << "Enter the file path to the key: ";
            cin.ignore();
            cin >> key_file_path;
        }
        else
            return 1;
    }
    
    // Open key file for reading
    ifstream new_key_file(key_file_path, ios::binary | ios::ate);
    if (!new_key_file.is_open()) {
        cerr << "Failed to open key file." << endl;
        return 1;
    }
    int key_file_size = new_key_file.tellg(); // get file size in bytes
    if (key_file_size != KEY_SIZE) {
        cerr << "Invalid key size." << endl;
        return 1;
    }
    new_key_file.seekg(0, ios::beg);
    new_key_file.read(reinterpret_cast<char*>(key), KEY_SIZE);
    new_key_file.close();
    cout << "Enter n:" << endl;
    cin >> n;
    enc = seal.coding(text, L * 8, key, n);
    delete[] buffer;
    delete[] text;

    // Write encrypted data to file
    cout << "Enter the path to the output file for encrypted data:" << endl;
    cin.ignore(); // Ignore the newline character left in the buffer after cin
    getline(cin, output_file_path);
    ofstream output_file(output_file_path, ios::binary | ios::trunc);
    if (!output_file.is_open()) {
        cout << "Error opening output file" << endl;
        return 1;
    }


    output_file.write((char*)&L, sizeof(int)); // Write file size to output file
    output_file.write((char*)&n, sizeof(int)); // Write n to output file


    output_file.write((char*)enc, L * sizeof(int));
    output_file.close();
    
}


int main(int argc, char* argv[]) {

    SEAL seal;
    int v = 0, L = 0, n = 0;
    string input_file_path, output_file_path,
    output_file_path_decrypted,key_file_path;
    int* text = 0;
    int* enc = 0;
    int* dec = 0;
    char* buffer = 0;
    const int KEY_SIZE = 20;
    int key[KEY_SIZE / sizeof(int)];
    cout << "Menu: \n"
        << "1.Generate key\n"
        << "2.Encrypt info\n"
        << "3.Decrypt info\n"
        << "4.Exit\n";
    do
    {
        cout << "\nChoose variant >>> ";
        cin >> v;
        switch (v)
        {

        case 1:
        {
            generate_key(key, KEY_SIZE);
            break;
        }
        case 2:
        {
            encryptFile(input_file_path, key_file_path, output_file_path, key, KEY_SIZE, enc, L, n, buffer, text);
            break;
        }
        case 3:
        {

            string input_file_path_decrypted;
            cout << "Enter the path to the file you want to decrypt:" << endl;
            cin.ignore();
            getline(cin, input_file_path_decrypted);
            ifstream input_file_decrypted(input_file_path_decrypted, ios::binary | ios::ate);
            if (!input_file_decrypted.is_open()) {
                cout << "Error opening input file" << endl;
                return 1;
            }
            int L_decrypted = input_file_decrypted.tellg(); // get file size in bytes
            input_file_decrypted.seekg(0, ios::beg);
            int* enc_decrypted = new int[L_decrypted / sizeof(int)];
            input_file_decrypted.read((char*)enc_decrypted, L_decrypted);
            input_file_decrypted.close();


            dec = seal.coding(enc, L * 8, key, n);
            char* decoded = new char[L];
            int d = 0;
            for (int i = 0; i < ceil((float)L / 4); i++) {
                for (int s = 3; s >= 0; s--) {
                    decoded[d++] = dec[i] >> (8 * s);
                }
            }
            delete[] enc;
            cout << "Enter the path to the output file for decrypted data:" << endl;
            //cin.ignore();
            getline(cin, output_file_path_decrypted);
            ofstream output_file_decrypted(output_file_path_decrypted, ios::binary | ios::trunc);
            if (!output_file_decrypted.is_open()) {
                cout << "Error opening output file" << endl;
                return 1;
            }
            output_file_decrypted.write(decoded, L);
            output_file_decrypted.close();

            cout << "\nDecoded data has been written to file." << endl;
            break;
        }
        default:
            break;
        }

    } while (v != 4);

    system("PAUSE");
    return 0;
}