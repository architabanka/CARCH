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
        Node * curr = root;
        if(curr==NULL) return;
        if(curr->left==NULL && curr->right==NULL){
            bw.writeBit(1);
            char c = curr->symbol;
            for(int i=7;i>=0;i--){
                int bit = (c>>i)&1;
                bw.writeBit(bit);
            }
            return;
        }
        bw.writeBit(0);
        serializeTree(curr->left,bw);
        serializeTree(curr->right,bw);
        return;
    }

    //ADD : reconstructing the Huffman tree exactly as it was using the preorder bit sequence stored in the archive
    Node* deserializeTree(BitReader& br) {
    }
}
