#include <cstdio>
#include <cstring>
#include <vector>
#include <algorithm>
#include <unordered_map>
#include <string>

using namespace std;

const char* DATA_FILE = "database.dat";
const int MAX_INDEX_SIZE = 5000; // Limit index size to save memory

struct Entry {
    char index[65];
    int value;
    bool deleted;
};

// Lightweight index - only store unique values for each key, not positions
unordered_map<string, vector<int>> keyIndex;
int entryCount = 0;

void insert(const char* idx, int val) {
    string key(idx);

    // Check if exists in index first (fast path)
    if (keyIndex.count(key)) {
        for (int v : keyIndex[key]) {
            if (v == val) return; // Already exists
        }
    }

    // Check if exists in file
    FILE* f = fopen(DATA_FILE, "rb");
    if (f) {
        Entry entry;
        while (fread(&entry, sizeof(Entry), 1, f) == 1) {
            if (!entry.deleted && strcmp(entry.index, idx) == 0 && entry.value == val) {
                fclose(f);
                return; // Already exists
            }
        }
        fclose(f);
    }

    // Append to file
    f = fopen(DATA_FILE, "ab");
    if (f) {
        Entry entry;
        memset(&entry, 0, sizeof(Entry));
        strncpy(entry.index, idx, 64);
        entry.value = val;
        entry.deleted = false;
        fwrite(&entry, sizeof(Entry), 1, f);
        fclose(f);
    }

    // Update index if not too large
    if (entryCount < MAX_INDEX_SIZE) {
        keyIndex[key].push_back(val);
    }
    entryCount++;
}

void remove(const char* idx, int val) {
    FILE* f = fopen(DATA_FILE, "rb+");
    if (!f) return;

    Entry entry;
    int pos = 0;

    while (fread(&entry, sizeof(Entry), 1, f) == 1) {
        if (!entry.deleted && strcmp(entry.index, idx) == 0 && entry.value == val) {
            entry.deleted = true;
            fseek(f, pos, SEEK_SET);
            fwrite(&entry, sizeof(Entry), 1, f);
            fclose(f);

            // Remove from index
            string key(idx);
            if (keyIndex.count(key)) {
                auto& vec = keyIndex[key];
                for (size_t i = 0; i < vec.size(); i++) {
                    if (vec[i] == val) {
                        vec.erase(vec.begin() + i);
                        if (vec.empty()) {
                            keyIndex.erase(key);
                        }
                        break;
                    }
                }
            }
            return;
        }
        pos += sizeof(Entry);
    }
    fclose(f);
}

void find(const char* idx) {
    vector<int> values;

    FILE* f = fopen(DATA_FILE, "rb");
    if (f) {
        Entry entry;
        while (fread(&entry, sizeof(Entry), 1, f) == 1) {
            if (!entry.deleted && strcmp(entry.index, idx) == 0) {
                values.push_back(entry.value);
            }
        }
        fclose(f);
    }

    if (values.empty()) {
        printf("null\n");
    } else {
        sort(values.begin(), values.end());
        for (size_t i = 0; i < values.size(); i++) {
            if (i > 0) printf(" ");
            printf("%d", values[i]);
        }
        printf("\n");
    }
}

int main() {
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

    return 0;
}
