#include <sstream>
#include <iostream>
#include <map>
#include <vector>
#include <string>

using namespace std;

vector<string> counter(string stroka) {
    vector<string> words;
    char delitel = ' ';
    stringstream ss(stroka);
    string word;

    while (ss >> word) {  // Обработка пробелов
        words.push_back(word);
    }

    return words;
}

map<string, int> counter_word(vector<string> vector) {
    map<string, int> mappy;

    for (const auto& word : vector) {
        mappy[word]++;
    }

    return mappy;
}

string max_Count_word(map<string, int> map) {
    int maximum = 0;
    string most_double_word;

    for (const auto& pair : map) {
        if (pair.second > maximum) {
            maximum = pair.second;
            most_double_word = pair.first;
        }
    }

    return most_double_word;
}

char want_to_contin() {
    cout << "Do you want to continue? (y/n)\n";
    char answer;

    while (true) {
        cin >> answer;
        if (tolower(answer) == 'y' || tolower(answer) == 'n') {
            break;  // Валидный ввод, выход из цикла
        } else {
            cout << "Invalid input. Please enter 'y' or 'n'.\n";
        }
    }

    return tolower(answer);
}

int main() {
    char answer;

    do {
        string stroka;

        cout << "Please enter a string>> ";
        cin.ignore();
        getline(cin, stroka);

        vector<string> slovo = counter(stroka);
        map<string, int> count_word = counter_word(slovo);
        string my_word_is = max_Count_word(count_word);

        cout << "The most frequent word is: " << my_word_is << endl;

        answer = want_to_contin();
    } while (answer == 'y');

    return 0;
}
