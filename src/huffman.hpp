// Tree example for frequencies: a=2, b=3, c=4
//          [-1: freq=9]           <- root (internal)
//         /             \
//      [c: freq=4]    [-1: freq=5]  <- internal
//                     /           \
//                 [a: freq=2]   [b: freq=3]
// ─────────────────────────────────────────────────────────────
#pragma once
#include <iostream>
#include <fstream>
#include <cstdint>
#include <string>
#include <queue>
#include <unordered_map>
#include "bitio.hpp"

namespace Huffman {

    struct Node {
        uint8_t symbol;
        uint64_t freq;
        Node* left;
        Node* right;

        Node(uint8_t symbol, uint64_t freq);
        Node(uint64_t freq, Node* left, Node* right);
        ~Node();
    };

    struct CompareNode {
        bool operator()(Node* a, Node* b);
    };

    void buildCodeTable(Node* root, const std::string& currentCode, std::unordered_map<uint8_t, std::string>& codeTable);
    void serializeTree(Node* root, BitWriter& bw);
    Node* deserializeTree(BitReader& br);
}
