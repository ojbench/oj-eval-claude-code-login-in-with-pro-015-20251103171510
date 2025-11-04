#include <cstdio>
#include <cstring>
#include <vector>
#include <algorithm>

using namespace std;

const char* DATA_FILE = "database.dat";
const int BUFFER_SIZE = 65536;

struct Entry {
    char index[65];
    int value;
    bool deleted;
};

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

            // Check if exists
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
