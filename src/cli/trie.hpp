/**
 * @file trie.hpp
 * @author lukem
 * @date 2025-12-26
 * @brief Prefix tree for preprocessor acceleration
 */

#ifndef CBUILD_TRIE_HPP
#define CBUILD_TRIE_HPP

#include <unordered_map>
#include <cwchar>

/**
 * @brief Prefix tree node
 */
class TrieNode {
public:
    std::unordered_map<wchar_t, TrieNode*> children;
    bool isEndOfWord = false;
};

class Trie {
private:
    TrieNode* root;

public:
    Trie() {
        root = new TrieNode();
    }

    // Insert a word into the trie
    void insert(const string& word) {
        TrieNode* node = root;
        for (char ch : word) {
            if (node->children.find(ch) == node->children.end()) {
                node->children[ch] = new TrieNode();
            }
            node = node->children[ch];
        }
        node->isEndOfWord = true;
    }

    // Search for a word in the trie
    bool search(const string& word) {
        TrieNode* node = root;
        for (char ch : word) {
            if (node->children.find(ch) == node->children.end()) {
                return false;
            }
            node = node->children[ch];
        }
        return node->isEndOfWord;
    }

    // Check if there is any word in the trie that starts with the given prefix
    bool startsWith(const string& prefix) {
        TrieNode* node = root;
        for (char ch : prefix) {
            if (node->children.find(ch) == node->children.end()) {
                return false;
            }
            node = node->children[ch];
        }
        return true;
    }
};

#endif