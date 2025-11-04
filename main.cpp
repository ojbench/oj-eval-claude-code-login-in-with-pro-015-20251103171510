#include <cstdio>
#include <cstring>
#include <vector>
#include <algorithm>
#include <set>

using namespace std;

const char* DATA_FILE = "database.dat";
const int BUFFER_SIZE = 65536;
const int CACHE_SIZE = 10000;

struct Entry {
    char index[65];
    int value;
    bool deleted;
};

struct EntryKey {
    char index[65];
    int value;

    bool operator<(const EntryKey& other) const {
        int cmp = strcmp(index, other.index);
        if (cmp != 0) return cmp < 0;
        return value < other.value;
    }
};

set<EntryKey> recentInserts;

int main() {
    int n;
    scanf("%d", &n);

    Entry entry;

    for (int i = 0; i < n; i++) {
        char cmd[10];
        scanf("%s", cmd);

        if (cmd[0] == 'i') { // insert
            char idx[65];
            int val;
            scanf("%s%d", idx, &val);

            EntryKey key;
            strncpy(key.index, idx, 64);
            key.index[64] = 0;
            key.value = val;

            // Check recent inserts cache first
            if (recentInserts.count(key)) {
                continue; // Already exists
            }

            // Check file for duplicates only if not in cache
            bool exists = false;
            FILE* f = fopen(DATA_FILE, "rb");
            if (f) {
                setvbuf(f, nullptr, _IOFBF, BUFFER_SIZE);
                while (fread(&entry, sizeof(Entry), 1, f) == 1) {
                    if (!entry.deleted && strcmp(entry.index, idx) == 0 && entry.value == val) {
                        exists = true;
                        break;
                    }
                }
                fclose(f);
            }

            if (!exists) {
                f = fopen(DATA_FILE, "ab");
                if (f) {
                    setvbuf(f, nullptr, _IOFBF, BUFFER_SIZE);
                    Entry newEntry;
                    memset(&newEntry, 0, sizeof(Entry));
                    strncpy(newEntry.index, idx, 64);
                    newEntry.value = val;
                    newEntry.deleted = false;
                    fwrite(&newEntry, sizeof(Entry), 1, f);
                    fclose(f);
                }

                // Add to cache
                recentInserts.insert(key);
                if (recentInserts.size() > CACHE_SIZE) {
                    recentInserts.erase(recentInserts.begin());
                }
            }
        } else if (cmd[0] == 'd') { // delete
            char idx[65];
            int val;
            scanf("%s%d", idx, &val);

            FILE* f = fopen(DATA_FILE, "rb+");
            if (f) {
                setvbuf(f, nullptr, _IOFBF, BUFFER_SIZE);
                long pos = 0;
                while (fread(&entry, sizeof(Entry), 1, f) == 1) {
                    if (!entry.deleted && strcmp(entry.index, idx) == 0 && entry.value == val) {
                        entry.deleted = true;
                        fseek(f, pos, SEEK_SET);
                        fwrite(&entry, sizeof(Entry), 1, f);
                        break;
                    }
                    pos += sizeof(Entry);
                }
                fclose(f);
            }

            // Remove from cache
            EntryKey key;
            strncpy(key.index, idx, 64);
            key.index[64] = 0;
            key.value = val;
            recentInserts.erase(key);
        } else { // find
            char idx[65];
            scanf("%s", idx);

            vector<int> values;
            FILE* f = fopen(DATA_FILE, "rb");
            if (f) {
                setvbuf(f, nullptr, _IOFBF, BUFFER_SIZE);
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
    }

    return 0;
}
