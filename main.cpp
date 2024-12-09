#include <iostream>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <map>
#include <set>
#include <vector>
#include <list>
#include <cctype>
#include <algorithm>
#include <string>

using namespace std;
namespace fs = std::filesystem;

// Function to get file names from the specified directory
list<string> getFileNames(const string &dir_path) {
    list<string> file_paths;
    for (const auto &dir_entry : fs::directory_iterator(dir_path))
        file_paths.push_back(dir_entry.path().string());
    return file_paths;
}

// Function to load file content
string loadFileContent(const string &file_path) {
    ifstream file(file_path);
    stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

// Enhanced stemming function to handle specific endings
string stem(const string &word) {
    if (word.length() > 2 && word.substr(word.length() - 2) == "er") {
        return word.substr(0, word.length() - 2); // Remove 'er'
    }
    if (word.length() > 2 && word.substr(word.length() - 2) == "or") {
        return word.substr(0, word.length() - 2); // Remove 'or'
    }
    if (word.length() > 1 && word.back() == 'r') {
        return word.substr(0, word.length() - 1); // Remove 'r'
    }
    if (word.length() > 2 && word.substr(word.length() - 2) == "ed") {
        return word.substr(0, word.length() - 2); // Remove 'ed'
    }
    if (word.length() > 3 && word.substr(word.length() - 3) == "ing") {
        return word.substr(0, word.length() - 3); // Remove 'ing'
    }
    if (word.length() > 2 && word.back() == 's') {
        return word.substr(0, word.length() - 1); // Remove plural 's'
    }
    return word; // Return the original word if no changes
}

// Function to clear and normalize tokens
string clearToken(const string &token) {
    string cleared_token;
    for (char ch : token) {
        if (isalpha(ch)) {
            cleared_token += char(tolower(ch));
        }
    }
    return cleared_token.empty() ? "" : stem(cleared_token);
}

// Function to check for common words
bool isCommonWord(const string &word) {
    static const set<string> stopWords = {
        "aa", "aaa", "a", "an", "the", "and", "or", "is", "to", "in", "of", "for", "on", "with", ","
    };
    return stopWords.find(word) != stopWords.end();
}

// Function to check if a term is meaningful
bool isMeaningfulTerm(const string &word) {
    return !word.empty() && word.length() >= 3 && !isCommonWord(word);
}

// Function to extract tokens from document content
map<string, set<int>> getTokens(const string &content, const int &doc_id) {
    map<string, set<int>> tokens;
    string token;

    for (char itr : content) {
        if (itr == ' ' || itr == '\n' || itr == '\t') {
            if (!token.empty()) {
                string cleared = clearToken(token);
                if (!cleared.empty() && isMeaningfulTerm(cleared)) {
                    tokens[cleared].insert(doc_id);
                }
                token.clear();
            }
        } else {
            token += itr;
        }
    }

    if (!token.empty()) {
        string cleared = clearToken(token);
        if (!cleared.empty() && isMeaningfulTerm(cleared)) {
            tokens[cleared].insert(doc_id);
        }
    }

    return tokens;
}
void writeIndexToCSV(const map<string, set<int>>& index, const string& outputFile) {
    ofstream outFile(outputFile);
    
    for (const auto& entry : index) {
        outFile << entry.first; // Write the term at the beginning of the line

        // Write the frequency (number of documents containing the term)
        outFile << ", " << entry.second.size();

        // Write the document IDs associated with the term
        for (auto it = entry.second.begin(); it != entry.second.end(); ++it) {
            outFile << ", " << *it; // Use a single comma before each ID
        }

        outFile << endl; // Move to the next line for the next term
    }

    outFile.close();
}
void mergeFiles(const vector<string> &subIndexFiles, const string &outputFile) {
    map<string, set<int>> finalIndex;

    for (const auto &subIndexFile : subIndexFiles) {
        ifstream file(subIndexFile);
        string term;
        int doc_id;

        while (file >> term) {
            file.ignore(); // Skip the comma
            file >> doc_id; // Read the first document ID

            // Add the document ID to the set
            finalIndex[term].insert(doc_id);

            // While there are more document IDs in the line, read them
            while (file.peek() == ',') {
                file.ignore(); // Skip the comma
                file >> doc_id; // Read the next document ID
                finalIndex[term].insert(doc_id); // Add to set to avoid duplicates
            }
        }
    }

    writeIndexToCSV(finalIndex, outputFile);

    // Clean up: remove sub-index files after merging
    for (const auto &file : subIndexFiles) {
        fs::remove(file);
    }
}
// Function to build the index from documents
void buildIndex(const string &base_path, int mxTermsInSubInd) {
    map<string, set<int>> dictionary;
    list<string> file_names = getFileNames(base_path);
    const int BLOCK_SIZE = 400000;
    int current_size = 0;
    int block_num = 1;
    vector<string> subIndexFiles;

    for (const string &file_name : file_names) {
        string file_path = file_name;
        current_size += fs::file_size(file_path);

        string loaded_content = loadFileContent(file_path);
        map<string, set<int>> tokens = getTokens(loaded_content, stoi(fs::path(file_name).filename().string()));

        for (const auto &term_doc : tokens) {
            dictionary[term_doc.first].insert(term_doc.second.begin(), term_doc.second.end());

            // Check if the number of terms in the dictionary exceeds mxTermsInSubInd
            if (dictionary.size() >= mxTermsInSubInd) {
                stringstream ss;
                ss << "subIndex_" << block_num << ".csv";
                writeIndexToCSV(dictionary, ss.str());
                subIndexFiles.push_back(ss.str());
                block_num++;
                dictionary.clear();  // Clear the dictionary for the next block
            }
        }

        // Check if current size exceeds BLOCK_SIZE and if dictionary has terms to write
        if (current_size >= BLOCK_SIZE && !dictionary.empty()) {
            stringstream ss;
            ss << "subIndex_" << block_num << ".csv";
            writeIndexToCSV(dictionary, ss.str());
            subIndexFiles.push_back(ss.str());
            current_size = 0;
            block_num++;
            dictionary.clear();
        }
    }

    // Write any remaining terms in the dictionary to a sub-index file
    if (!dictionary.empty()) {
        stringstream ss;
        ss << "subIndex_" << block_num << ".csv";
        writeIndexToCSV(dictionary, ss.str());
        subIndexFiles.push_back(ss.str());
    }

    mergeFiles(subIndexFiles, "invIndex.csv");
}

int main() {
    int mxTermsInSubInd = 10000;  
    buildIndex("/Users/mqbwq/Desktop/IRAssi/SPIMI/collection/", mxTermsInSubInd);
    return 0;
}