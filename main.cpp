#include <fstream>
#include <iostream>
#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>
#include <map>
#include <random>
#include <cstring>
#include <algorithm>

namespace fs = std::filesystem;
using namespace std;

#define MAX(a, b) ((a) > (b) ? (a) : (b))

class FileScoreTable
{
private:
    int static const capacity = 5;
    multimap<float, pair<string, string>> scores;

public:
    FileScoreTable()
    {
        scores = {};
    }
    ~FileScoreTable()
    {
    }
    void push(string sentence, string orig_sentence, float score)
    {
        // We only need the top 5, so if we already have 5 items, delete
        // the smallest score and its sentence
        if (score < (*scores.begin()).first)
        {
            return;
        }
        if (scores.size() == capacity)
        {
            scores.erase(scores.begin());
        }
        scores.insert(pair(score, pair(sentence, orig_sentence)));
    }
    void print()
    {
        auto n = 0;
        for (auto &item : scores)
        {
            n++;
            cout << "TARGET SENTENCE: " << item.second.first << "\n"
                 << "ORIGINAL SENTENCE: " << item.second.second << "\n"
                 << "SCORE: %" << item.first * 100 << '\n'
                 << "\n";
            if (n == 5)
                break;
        }
    }
};

string strip_newline(string s)
{
    for (unsigned int i = 0; i < s.length(); i++)
    {
        if (s[i] == '\n')
        {
            s[i] = ' ';
        }
    }
    return s;
}

vector<string> split_words(const string s)
{
    vector<string> result = {};
    size_t space_pos = 0;
    for (size_t i = 0; i < s.size(); i++)
    {
        if (s[i] == ' ' || i == s.size() - 1)
        {
            result.push_back(s.substr(space_pos, i - space_pos + 1));
            space_pos = i;
        }
    }
    return result;
}

int maximal_suffix_reverse(string str, size_t known_period, bool order_greater)
{
    int left = 0;
    int right = 1;
    int offset = 0;
    int period = 1;
    int size = str.size();
    while (right + offset < size)
    {
        char a = str[size - (1 + right + offset)];
        char b = str[size - (1 + left + offset)];
        if ((a < b && !order_greater) || (a > b && order_greater))
        {
            right += offset + 1;
            offset = 0;
            period = right - left;
        }
        else if (a == b)
        {
            if (offset + 1 == period)
            {
                right += offset + 1;
                offset = 0;
            }
            else
            {
                offset += 1;
            }
        }
        else
        {
            left = right;
            right += 1;
            offset = 0;
            period = 1;
        }
        if (period == known_period)
        {
            break;
        }
    }
    return left;
}
pair<int, int> maximal_suffix(string str, bool order_greater)
{
    int left = 0;   // i
    int right = 1;  // j
    int offset = 0; // k
    int period = 1; // l
    size_t size = str.size();
    while (left + right < size)
    {
        char a = str[right + offset];
        char b = str[left + offset];
        if ((a < b && !order_greater) || (a > b && order_greater))
        {
            right += offset + 1;
            offset = 0;
            period = right - left;
        }
        else if (a == b)
        {
            if (offset + 1 == period)
            {
                right += offset + 1;
                offset = 0;
            }
            else
            {
                offset += 1;
            }
        }
        else
        {
            left = right;
            right++;
            offset = 0;
            period = 1;
        }
    }
    return pair(left, period);
}

pair<int, int> crit_params(string needle)
{
    auto with_false = maximal_suffix(needle, false);
    auto with_true = maximal_suffix(needle, true);
    if (with_false.first > with_true.first)
    {
        return with_false;
    }
    else
    {
        return with_true;
    }
}
struct TwoWaySearch
{
    string needle;
    int crit_pos;
    int crit_pos_back;
    int period;
    int memory;
    int memory_back;
    TwoWaySearch(string needle)
    {
        auto temp = crit_params(needle);
        this->crit_pos = temp.first;
        this->period = temp.second;
        if (needle.substr(0, crit_pos) == needle.substr(period, period + crit_pos))
        {
            this->crit_pos_back = needle.size() - MAX(
                                                      maximal_suffix_reverse(needle, period, false),
                                                      maximal_suffix_reverse(needle, period, true));
        }
    }
    size_t find(string haystack)
    {
        int pos = 0;
        while (true)
        {
            bool continue_while = false;
            if (haystack.size() < needle.size())
            {
                return -1;
            }
            int start = MAX(crit_pos, memory);
            for (int i = start; i < needle.size(); i++)
            {
                if (needle[i] != haystack[pos + i])
                {
                    pos += i - crit_pos + 1;
                    memory = 0;
                    continue_while = true;
                    break;
                }
            }
            if (continue_while)
            {
                continue;
            }
            start = memory;
            for (int i = crit_pos; i >= start; i--)
            {
                if (needle[i] != haystack[pos + i])
                {
                    pos += period;
                    memory = needle.size() - period;
                    continue_while = true;
                    break;
                }
            }
            if (continue_while)
            {
                continue;
            }
            return pos;
        }
    }
};
int main(int argc, char const *argv[])
{
    // first argument is the folder's name, second is the target file
    char const *folder = "texts";
    char const *target_file_name = "anthem.txt";

    for (auto file_entry : fs::directory_iterator(folder))
    {
        FileScoreTable table;
        vector<float> score_list;

        string target_sentence;
        ifstream target_file(target_file_name);
        while (getline(target_file, target_sentence, '.'))
        {
            ifstream file(file_entry.path());
            string sentence;
            auto words = split_words(target_sentence);
            if (words.size() < 9)
            {
                continue;
            }
            while (getline(file, sentence, '.'))
            {
                sentence = strip_newline(sentence);
                // float sentence_score = get_similarity_score(target_sentence, sentence);
                int similar_counts = 0;
                int consecutive_similars = 0;
                size_t last_pos = 0;
                for (size_t i = 0; i < words.size(); i++)
                {
                    auto w = words[i];
                    TwoWaySearch tw(w);
                    size_t pos;
                    if (consecutive_similars > 0)
                    {
                        // If we found a similar word, search in the sentence after the word
                        // lorem ipsum similar_word rest of the sentence blah blah
                        //                          ^-----------------------------^
                        //                          Only look here, consecutive similarity is more important.
                        // pos = sentence.find(w, last_pos);
                        tw.find(sentence.substr(last_pos));
                    }
                    else
                    {
                        pos = tw.find(sentence);
                    }
                    if (pos != string::npos)
                    {
                        // We found a match of the word in the sentence2, increment the count
                        consecutive_similars++;
                        last_pos = pos;
                    }
                    else
                    {
                        similar_counts += consecutive_similars;
                        consecutive_similars = 0;
                    }
                }
                float sentence_score;
                if (similar_counts < 9)
                {
                    sentence_score = 0.f;
                }
                else
                {
                    sentence_score = (float)similar_counts / ((float)words.size());
                }
                score_list.push_back(sentence_score);
                table.push(target_sentence, sentence, sentence_score);
            }
        }
        float file_score = accumulate(score_list.begin(), score_list.end(), 0.f) / score_list.size();
        cout << "Scores for the file " << file_entry.path() << "\n";
        cout << "Overall score: %" << file_score * 100 << "\n";
        table.print();
    }
    return 0;
}
