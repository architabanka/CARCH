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
        if(!root) {
            return;
        }
        else if((!root->left) && (!root->right)) {
            codeTable[root->symbol] = currentCode + "1";
        }
        else{
            if(root->left) {
                buildCodeTable(root->left, currentCode + "0", codeTable);
                buildCodeTable(root->right, currentCode + "0", codeTable);
            }
        }
    }

    //ADD : leaves of huffman tree nodes should be 1 rest nodes should correspond to 0
    //generate encoding in buffer, storing preorder bit for compressed huffman tree
    void serializeTree(Node* root, BitWriter& bw) {
        if(root->left || root->right) {
            bw.writeBit(0);
            serializeTree(root->left, bw);
            serializeTree(root->right, bw);
        }
        if(root->right){
            bw.writeBit(1);
        }
    }

    //ADD : reconstructing the Huffman tree exactly as it was using the preorder bit sequence stored in the archive
    Node* deserializeTree(BitReader& br) {
        // now only we dont know the freq table
        // we update the frequencies and symbols of the node in archive.cpp
        Node root{-1, nullptr, nullptr};
        int num_zero = 0;
        int temp_num = 0;
        while(true) {
            int temp = br.readBit();
            if(temp == -1) {
                break;
            }
            else if(temp == 0) {
                temp_num++;
            }
            else{
                // bit read is 1
                num_zero = std::max(num_zero, temp_num);
                temp_num = 0;
            }
        }
        Node temp = root;
        for(int i = 0; i<num_zero; i++){
            Node r{-1, nullptr, nullptr};
            Node l{-1, nullptr, nullptr};
            temp.right = &r;
            temp.left = &l;
            temp = r;
        }
        Node l{-1, nullptr, nullptr};
        temp.left = &l;
        return &root;
    }
}
