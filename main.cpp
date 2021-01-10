#include <fstream>
#include <iostream>
#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>
#include <map>
#include <random>

namespace fs = std::filesystem;
using namespace std;

class FileScoreTable
{
private:
    int static const capacity = 5;
    multimap<float, string> scores;

public:
    FileScoreTable()
    {
        scores = {};
    }
    ~FileScoreTable()
    {
    }
    void push(string sentence, float score)
    {
        // We only need the top 5, so if we already have 5 items, delete
        // the smallest score and its sentence
        if (scores.size() == capacity){
            scores.erase(scores.begin());
        }
        scores.insert(pair(score, sentence));
    }
    void print()
    {
        auto n = 0;
        for (auto &item: scores)
        {
            n++;
            cout << item.second << "\t" << item.first << endl;
            if (n == 5) break;
        }
    }
};

string strip_newline(string s)
{
    for (int i = 0; i < s.length(); i++)
    {
        char c = s[i];
        if (s[i] == '\n')
        {
            s[i] = ' ';
        }
    }
    return s;
}

float get_similarity_score(string sentence1, string sentence2)
{
    /* TODO: IMPLEMENT THIS, give the similarity score instead of random value
    *  Returns a value between 0 and 1 */
    std::random_device rd;
    std::mt19937 generator(rd());
    std::uniform_int_distribution<std::mt19937::result_type> distribution(0, 100);

    return (float)distribution(generator) / 100.0f;
}

int main(int argc, char const *argv[])
{
    // first argument is the folder's name, second is the target file
    char const *folder = argv[1];
    char const *target_file_name = argv[2];

    for (auto file_entry : fs::directory_iterator(folder))
    {
        FileScoreTable table;
        float score = 0.0;
        vector<float> score_list;
        ifstream file(file_entry.path());
        string target_sentence;
        ifstream target_file(target_file_name);

        // TODO: If possible, change this to read actual sentences, in an efficient way
        while (getline(target_file, target_sentence, '.'))
        {
            string sentence;
            while (getline(file, sentence, '.'))
            {
                sentence = strip_newline(sentence);
                float sentence_score = get_similarity_score(target_sentence, sentence);
                score_list.push_back(sentence_score);
                table.push(sentence, sentence_score);
            }
        }
        for (float f : score_list)
        {
            score += f / score_list.size();
        }
        cout << "Scores for the file " << file_entry.path() << "\n";
        cout << "Overall score: " << score << "\n";
        table.print();
    }
    return 0;
}
