#include <cstdio>
#include <cstring>
#include <vector>
#include <algorithm>
#include <map>
#include <string>

using namespace std;

const char* DATA_FILE = "database.dat";
const char* INDEX_FILE = "index.dat";

struct Entry {
    char index[65];
    int value;
    bool deleted;
};

// Compact index using short strings
map<string, vector<pair<int, int>>> keyIndex; // key -> [(value, position)]
int nextPos = 0;

void loadIndex() {
    FILE* f = fopen(INDEX_FILE, "rb");
    if (!f) {
        f = fopen(DATA_FILE, "rb");
        if (!f) return;

        // Build index from data file
        Entry entry;
        int pos = 0;
        while (fread(&entry, sizeof(Entry), 1, f) == 1) {
            if (!entry.deleted) {
                keyIndex[string(entry.index)].push_back({entry.value, pos});
            }
            pos += sizeof(Entry);
        }
        nextPos = pos;
        fclose(f);
        return;
    }

    // Load from index file
    int mapSize;
    if (fread(&mapSize, sizeof(int), 1, f) != 1) {
        fclose(f);
        return;
    }

    for (int i = 0; i < mapSize; i++) {
        short keyLen;
        fread(&keyLen, sizeof(short), 1, f);

        char key[256];
        fread(key, 1, keyLen, f);
        key[keyLen] = 0;

        short vecSize;
        fread(&vecSize, sizeof(short), 1, f);

        vector<pair<int, int>> values;
        for (int j = 0; j < vecSize; j++) {
            int val, pos;
            fread(&val, sizeof(int), 1, f);
            fread(&pos, sizeof(int), 1, f);
            values.push_back({val, pos});
        }

        keyIndex[string(key)] = values;
    }

    fread(&nextPos, sizeof(int), 1, f);
    fclose(f);
}

void saveIndex() {
    FILE* f = fopen(INDEX_FILE, "wb");
    if (!f) return;

    int mapSize = keyIndex.size();
    fwrite(&mapSize, sizeof(int), 1, f);

    for (auto& kv : keyIndex) {
        short keyLen = kv.first.size();
        fwrite(&keyLen, sizeof(short), 1, f);
        fwrite(kv.first.c_str(), 1, keyLen, f);

        short vecSize = kv.second.size();
        fwrite(&vecSize, sizeof(short), 1, f);

        for (auto& vp : kv.second) {
            fwrite(&vp.first, sizeof(int), 1, f);
            fwrite(&vp.second, sizeof(int), 1, f);
        }
    }

    fwrite(&nextPos, sizeof(int), 1, f);
    fclose(f);
}

void insert(const char* idx, int val) {
    string key(idx);

    // Check if exists in index
    if (keyIndex.count(key)) {
        for (auto& vp : keyIndex[key]) {
            if (vp.first == val) {
                return; // Already exists
            }
        }
    }

    // Append to file
    FILE* f = fopen(DATA_FILE, "ab");
    if (f) {
        Entry entry;
        memset(&entry, 0, sizeof(Entry));
        strncpy(entry.index, idx, 64);
        entry.value = val;
        entry.deleted = false;
        fwrite(&entry, sizeof(Entry), 1, f);
        fclose(f);
    }

    // Update index
    keyIndex[key].push_back({val, nextPos});
    nextPos += sizeof(Entry);
}

void remove(const char* idx, int val) {
    string key(idx);

    if (!keyIndex.count(key)) return;

    int posToDelete = -1;
    auto& vec = keyIndex[key];

    for (size_t i = 0; i < vec.size(); i++) {
        if (vec[i].first == val) {
            posToDelete = vec[i].second;
            vec.erase(vec.begin() + i);
            if (vec.empty()) {
                keyIndex.erase(key);
            }
            break;
        }
    }

    if (posToDelete >= 0) {
        FILE* f = fopen(DATA_FILE, "rb+");
        if (f) {
            Entry entry;
            fseek(f, posToDelete, SEEK_SET);
            fread(&entry, sizeof(Entry), 1, f);
            entry.deleted = true;
            fseek(f, posToDelete, SEEK_SET);
            fwrite(&entry, sizeof(Entry), 1, f);
            fclose(f);
        }
    }
}

void find(const char* idx) {
    string key(idx);

    if (!keyIndex.count(key) || keyIndex[key].empty()) {
        printf("null\n");
        return;
    }

    vector<int> values;
    for (auto& vp : keyIndex[key]) {
        values.push_back(vp.first);
    }

    sort(values.begin(), values.end());

    for (size_t i = 0; i < values.size(); i++) {
        if (i > 0) printf(" ");
        printf("%d", values[i]);
    }
    printf("\n");
}

int main() {
    loadIndex();

    int n;
    scanf("%d", &n);

    for (int i = 0; i < n; i++) {
        char cmd[10];
        scanf("%s", cmd);

        if (cmd[0] == 'i') {
            char idx[65];
            int val;
            scanf("%s%d", idx, &val);
            insert(idx, val);
        } else if (cmd[0] == 'd') {
            char idx[65];
            int val;
            scanf("%s%d", idx, &val);
            remove(idx, val);
        } else {
            char idx[65];
            scanf("%s", idx);
            find(idx);
        }
    }

    saveIndex();
    return 0;
}
