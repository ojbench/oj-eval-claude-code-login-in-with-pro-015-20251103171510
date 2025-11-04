#include <iostream>
#include <fstream>
#include <cstring>
#include <vector>
#include <algorithm>
#include <map>
#include <set>

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
    // In-memory index: key -> set of (value, file_position)
    map<string, set<pair<int, long>>> index;
    long nextPos;

    void loadIndex() {
        ifstream indexFile(INDEX_FILE, ios::binary);
        if (!indexFile.is_open()) {
            // No index file yet
            nextPos = 0;
            return;
        }

        // Read index from file
        size_t mapSize;
        indexFile.read((char*)&mapSize, sizeof(mapSize));

        for (size_t i = 0; i < mapSize; i++) {
            size_t keyLen;
            indexFile.read((char*)&keyLen, sizeof(keyLen));

            string key;
            key.resize(keyLen);
            indexFile.read(&key[0], keyLen);

            size_t setSize;
            indexFile.read((char*)&setSize, sizeof(setSize));

            set<pair<int, long>> values;
            for (size_t j = 0; j < setSize; j++) {
                int value;
                long pos;
                indexFile.read((char*)&value, sizeof(value));
                indexFile.read((char*)&pos, sizeof(pos));
                values.insert({value, pos});
            }

            index[key] = values;
        }

        indexFile.read((char*)&nextPos, sizeof(nextPos));
        indexFile.close();
    }

    void saveIndex() {
        ofstream indexFile(INDEX_FILE, ios::binary);

        size_t mapSize = index.size();
        indexFile.write((char*)&mapSize, sizeof(mapSize));

        for (const auto& kv : index) {
            size_t keyLen = kv.first.size();
            indexFile.write((char*)&keyLen, sizeof(keyLen));
            indexFile.write(kv.first.c_str(), keyLen);

            size_t setSize = kv.second.size();
            indexFile.write((char*)&setSize, sizeof(setSize));

            for (const auto& vp : kv.second) {
                indexFile.write((char*)&vp.first, sizeof(vp.first));
                indexFile.write((char*)&vp.second, sizeof(vp.second));
            }
        }

        indexFile.write((char*)&nextPos, sizeof(nextPos));
        indexFile.close();
    }

public:
    FileDB() {
        loadIndex();
    }

    ~FileDB() {
        saveIndex();
    }

    void insert(const char* idx, int value) {
        string key(idx);

        // Check if entry already exists
        if (index.count(key) && index[key].count({value, -1})) {
            // Find actual position
            for (const auto& vp : index[key]) {
                if (vp.first == value) {
                    return; // Already exists
                }
            }
        }

        // Append to data file
        fstream dataFile(DATA_FILE, ios::in | ios::out | ios::binary | ios::app);
        if (!dataFile.is_open()) {
            dataFile.open(DATA_FILE, ios::out | ios::binary);
            dataFile.close();
            dataFile.open(DATA_FILE, ios::in | ios::out | ios::binary | ios::app);
        }

        long pos = nextPos;
        Entry entry(idx, value);
        dataFile.write((char*)&entry, sizeof(Entry));
        dataFile.close();

        // Update index
        index[key].insert({value, pos});
        nextPos += sizeof(Entry);
    }

    void remove(const char* idx, int value) {
        string key(idx);

        if (!index.count(key)) {
            return; // Key doesn't exist
        }

        // Find the entry in index
        auto it = index[key].begin();
        long posToDelete = -1;

        for (auto& vp : index[key]) {
            if (vp.first == value) {
                posToDelete = vp.second;
                break;
            }
        }

        if (posToDelete == -1) {
            return; // Entry doesn't exist
        }

        // Mark as deleted in file
        fstream dataFile(DATA_FILE, ios::in | ios::out | ios::binary);
        if (!dataFile.is_open()) {
            return;
        }

        Entry entry;
        dataFile.seekg(posToDelete, ios::beg);
        dataFile.read((char*)&entry, sizeof(Entry));
        entry.deleted = true;
        dataFile.seekp(posToDelete, ios::beg);
        dataFile.write((char*)&entry, sizeof(Entry));
        dataFile.close();

        // Remove from index
        index[key].erase({value, posToDelete});
        if (index[key].empty()) {
            index.erase(key);
        }
    }

    void find(const char* idx) {
        string key(idx);

        if (!index.count(key) || index[key].empty()) {
            cout << "null" << endl;
            return;
        }

        vector<int> values;
        for (const auto& vp : index[key]) {
            values.push_back(vp.first);
        }

        sort(values.begin(), values.end());

        for (size_t i = 0; i < values.size(); i++) {
            if (i > 0) cout << " ";
            cout << values[i];
        }
        cout << endl;
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
