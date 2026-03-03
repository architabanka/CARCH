#include "huffman.hpp"
#include <vector>
#include <algorithm>

namespace Huffman {

    Node::Node(uint8_t symbol, uint64_t freq) : symbol(symbol), freq(freq), left(nullptr), right(nullptr) {}

    Node::Node(uint64_t freq, Node* left, Node* right) : symbol(0), freq(freq), left(left), right(right) {}

    Node::~Node() {
        delete left;
        delete right;
    }

    bool CompareNode::operator()(Node* a, Node* b) {
        return a->freq > b->freq;
    }
    //ADD : code for encoding from huffman tree
    void buildCodeTable(Node* root, const std::string& currentCode, std::unordered_map<uint8_t, std::string>& codeTable) {
        
    }

    //ADD : leaves of huffman tree nodes should be 1 rest nodes should correspond to 0
    //generate encoding in buffer, storing preorder bit for compressed huffman tree
    void serializeTree(Node* root, BitWriter& bw) {
        
    }

    //ADD : reconstructing the Huffman tree exactly as it was using the preorder bit sequence stored in the archive
    Node* deserializeTree(BitReader& br) {
        
    }
}
