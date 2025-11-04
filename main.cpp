#include <iostream>
#include <fstream>
#include <cstring>
#include <vector>
#include <algorithm>

using namespace std;

const char* DATA_FILE = "database.dat";
const char* INDEX_FILE = "index.dat";

struct Entry {
    char index[65];
    int value;
    bool deleted;

    Entry() : value(0), deleted(false) {
        memset(index, 0, sizeof(index));
    }

    Entry(const char* idx, int val) : value(val), deleted(false) {
        memset(index, 0, sizeof(index));
        strncpy(index, idx, 64);
    }
};

class FileDB {
private:
    fstream dataFile;

    void openDataFile() {
        dataFile.open(DATA_FILE, ios::in | ios::out | ios::binary);
        if (!dataFile.is_open()) {
            // File doesn't exist, create it
            dataFile.open(DATA_FILE, ios::out | ios::binary);
            dataFile.close();
            dataFile.open(DATA_FILE, ios::in | ios::out | ios::binary);
        }
    }

    void closeDataFile() {
        if (dataFile.is_open()) {
            dataFile.close();
        }
    }

public:
    FileDB() {}

    ~FileDB() {
        closeDataFile();
    }

    void insert(const char* index, int value) {
        openDataFile();

        // Check if entry already exists
        dataFile.seekg(0, ios::beg);
        Entry entry;
        while (dataFile.read((char*)&entry, sizeof(Entry))) {
            if (!entry.deleted && strcmp(entry.index, index) == 0 && entry.value == value) {
                // Entry already exists, don't insert duplicate
                closeDataFile();
                return;
            }
        }

        // Clear any error flags
        dataFile.clear();

        // Append new entry
        dataFile.seekp(0, ios::end);
        Entry newEntry(index, value);
        dataFile.write((char*)&newEntry, sizeof(Entry));
        dataFile.flush();

        closeDataFile();
    }

    void remove(const char* index, int value) {
        openDataFile();

        dataFile.seekg(0, ios::beg);
        Entry entry;
        long pos = 0;

        while (dataFile.read((char*)&entry, sizeof(Entry))) {
            if (!entry.deleted && strcmp(entry.index, index) == 0 && entry.value == value) {
                // Mark as deleted
                entry.deleted = true;
                dataFile.seekp(pos, ios::beg);
                dataFile.write((char*)&entry, sizeof(Entry));
                dataFile.flush();
                closeDataFile();
                return;
            }
            pos = dataFile.tellg();
        }

        closeDataFile();
    }

    void find(const char* index) {
        openDataFile();

        vector<int> values;
        dataFile.seekg(0, ios::beg);
        Entry entry;

        while (dataFile.read((char*)&entry, sizeof(Entry))) {
            if (!entry.deleted && strcmp(entry.index, index) == 0) {
                values.push_back(entry.value);
            }
        }

        closeDataFile();

        if (values.empty()) {
            cout << "null" << endl;
        } else {
            sort(values.begin(), values.end());
            for (size_t i = 0; i < values.size(); i++) {
                if (i > 0) cout << " ";
                cout << values[i];
            }
            cout << endl;
        }
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(0);

    int n;
    cin >> n;

    FileDB db;

    for (int i = 0; i < n; i++) {
        string cmd;
        cin >> cmd;

        if (cmd == "insert") {
            char index[65];
            int value;
            cin >> index >> value;
            db.insert(index, value);
        } else if (cmd == "delete") {
            char index[65];
            int value;
            cin >> index >> value;
            db.remove(index, value);
        } else if (cmd == "find") {
            char index[65];
            cin >> index;
            db.find(index);
        }
    }

    return 0;
}
