#ifndef FREE_HPP
#define FREE_HPP

#include <vector>
#include <cstring>
#include <ctime>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cstdint>
#include "HashMap.hpp"

using namespace std;

class Bitmap {
private:
    uint8_t* data;
    uint32_t total_bits;
    uint32_t total_bytes;
    uint32_t free_bits;

public:
    Bitmap() : data(nullptr), total_bits(0), total_bytes(0), free_bits(0) {}
    ~Bitmap() { delete[] data; }

    void initialize(uint32_t num_blocks) {
        total_bits = num_blocks;
        total_bytes = (num_blocks + 7) / 8;
        free_bits = num_blocks;
        data = new uint8_t[total_bytes];
        memset(data, 0xFF, total_bytes); // 1 = free, 0 = allocated
    }

    bool set_bit(uint32_t bit) {
        if (bit >= total_bits) return false;
        uint32_t byte_idx = bit / 8;
        uint32_t bit_idx = bit % 8;
        bool was_free = (data[byte_idx] & (1 << bit_idx)) != 0;
        data[byte_idx] &= ~(1 << bit_idx); // allocate
        if (was_free) free_bits--;
        return was_free;
    }

    bool clear_bit(uint32_t bit) {
        if (bit >= total_bits) return false;
        uint32_t byte_idx = bit / 8;
        uint32_t bit_idx = bit % 8;
        bool was_allocated = (data[byte_idx] & (1 << bit_idx)) == 0;
        data[byte_idx] |= (1 << bit_idx); // free
        if (was_allocated) free_bits++;
        return was_allocated;
    }

    bool is_free(uint32_t bit) const {
        if (bit >= total_bits) return false;
        uint32_t byte_idx = bit / 8;
        uint32_t bit_idx = bit % 8;
        return (data[byte_idx] & (1 << bit_idx)) != 0;
    }

    uint32_t get_free_count() const { return free_bits; }
    uint32_t get_total_count() const { return total_bits; }
    uint32_t get_bitmap_size() const { return total_bytes; }

    bool allocate_range(uint32_t start, uint32_t count) {
        if (start + count > total_bits) return false;
        for (uint32_t i = start; i < start + count; i++)
            if (!is_free(i)) return false;
        for (uint32_t i = start; i < start + count; i++)
            set_bit(i);
        return true;
    }

    bool free_range(uint32_t start, uint32_t count) {
        if (start + count > total_bits) return false;
        for (uint32_t i = start; i < start + count; i++)
            clear_bit(i);
        return true;
    }
};

#define BLOCK_METADATA_SIZE 64

struct BlockMetadata {
    uint32_t file_id;
    uint32_t sequence_number;
    uint32_t data_size;
    uint32_t next_block;
    uint32_t timestamp;
    uint8_t reserved[32];
};

class FreeSpaceManager {
private:
    Bitmap bitmap;
    uint32_t total_blocks;
    HashMap<uint32_t, vector<uint32_t>> file_block_map;
    HashMap<uint32_t, BlockMetadata> block_metadata_map;
    uint32_t next_file_id;

public:
    FreeSpaceManager() : total_blocks(0), next_file_id(1) {}

    void initialize(uint32_t num_blocks) {
        total_blocks = num_blocks;
        bitmap.initialize(num_blocks);
        file_block_map.clear();
        block_metadata_map.clear();
        next_file_id = 1;
    }

    uint32_t allocate_blocks(uint32_t count) {
        if (count == 0 || count > bitmap.get_free_count())
            return 0;

        vector<uint32_t> allocated_blocks;
        for (uint32_t i = 0; i < total_blocks && allocated_blocks.size() < count; i++) {
            if (bitmap.is_free(i)) {
                bitmap.set_bit(i);
                allocated_blocks.push_back(i);

                BlockMetadata meta{};
                meta.file_id = next_file_id;
                meta.sequence_number = allocated_blocks.size() - 1;
                meta.data_size = 0;
                meta.next_block = 0;
                meta.timestamp = static_cast<uint32_t>(time(nullptr));
                block_metadata_map.insert(i, meta);
            }
        }

        if (allocated_blocks.size() != count) {
            for (uint32_t block : allocated_blocks)
                bitmap.clear_bit(block);
            return 0;
        }

        for (size_t i = 0; i < allocated_blocks.size(); i++) {
            if (i + 1 < allocated_blocks.size()) {
                BlockMetadata* m = block_metadata_map.get(allocated_blocks[i]);
                if (m) m->next_block = allocated_blocks[i + 1];
            }
        }

        file_block_map.insert(next_file_id, allocated_blocks);
        uint32_t file_id = next_file_id++;
        cout << "✓ Allocated " << count << " blocks for file_id: " << file_id << "\n";
        return file_id;
    }

    int allocate_single_block() {
        uint32_t file_id = allocate_blocks(1);
        if (file_id == 0) return -1;
        vector<uint32_t>* blocks = file_block_map.get(file_id);
        return blocks ? (*blocks)[0] : -1;
    }

    bool free_blocks(uint32_t start, uint32_t count) {
        vector<uint32_t>* blocks = file_block_map.get(start);
        if (blocks) {
            for (uint32_t block : *blocks) {
                bitmap.clear_bit(block);
                block_metadata_map.erase(block);
            }
            file_block_map.erase(start);
            cout << "✓ Freed all blocks for file_id: " << start << "\n";
            return true;
        }

        if (start + count > total_blocks) return false;
        for (uint32_t i = start; i < start + count; i++) {
            bitmap.clear_bit(i);
            block_metadata_map.erase(i);
        }
        return true;
    }

    bool free_single_block(uint32_t block) {
        block_metadata_map.erase(block);
        return bitmap.clear_bit(block);
    }

    bool is_block_free(uint32_t block) const { return bitmap.is_free(block); }
    uint32_t get_free_blocks() const { return bitmap.get_free_count(); }
    uint32_t get_total_blocks() const { return total_blocks; }
    uint32_t get_bitmap_memory_size() const { return bitmap.get_bitmap_size(); }

    double get_fragmentation_percentage() const {
        if (total_blocks == 0) return 0.0;
        uint32_t used_blocks = total_blocks - get_free_blocks();
        return (static_cast<double>(used_blocks) / total_blocks) * 100.0;
    }

    bool read_block_metadata(uint32_t block, BlockMetadata& meta) const {
        const BlockMetadata* ptr = block_metadata_map.get(block);
        if (!ptr) return false;
        meta = *ptr;
        return true;
    }

    bool write_block_metadata(uint32_t block, const BlockMetadata& meta) {
        if (block >= total_blocks) return false;
        block_metadata_map.insert(block, meta);
        return true;
    }

    bool update_block_metadata(uint32_t block, uint32_t data_size, uint32_t next_block) {
        BlockMetadata* m = block_metadata_map.get(block);
        if (!m) return false;
        m->data_size = data_size;
        m->next_block = next_block;
        return true;
    }

    vector<uint32_t> get_file_blocks(uint32_t file_id) const {
        const vector<uint32_t>* blocks = file_block_map.get(file_id);
        if (blocks) return *blocks;
        return {};
    }

    uint64_t get_file_total_size(uint32_t file_id) const {
        const vector<uint32_t>* blocks = file_block_map.get(file_id);
        if (!blocks) return 0;
        uint64_t total_size = 0;
        for (uint32_t block : *blocks) {
            const BlockMetadata* m = block_metadata_map.get(block);
            if (m) total_size += m->data_size;
        }
        return total_size;
    }

    uint32_t get_file_count() const { return file_block_map.size(); }

    void print_allocation_map() const {
        cout << "\n=== Block Allocation Map ===\n";
        vector<uint32_t> keys = file_block_map.keys();
        for (uint32_t fid : keys) {
            const vector<uint32_t>* blocks = file_block_map.get(fid);
            if (!blocks) continue;
            cout << "File ID " << fid << ": ";
            for (uint32_t b : *blocks) cout << b << " ";
            cout << "\n";
        }
        cout << "Free Blocks: " << get_free_blocks() << " / " << total_blocks << "\n";
        cout << "Usage: " << fixed << setprecision(2)
             << get_fragmentation_percentage() << "%\n";
    }
};

#endif