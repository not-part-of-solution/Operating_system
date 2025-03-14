#include <iostream>
#include <fstream>
#include <string>
#include <sys/types.h>
#include <unistd.h>
#include <cmath>
#include <cstring>

using namespace std;

const string ERROR_MESSAGE = "> Enter fail. Try again, please\n (；⌣̀_⌣́)\n";
const string ERROR_MESSAGE_FILE = "> Fail doesn't open\n (；⌣̀_⌣́)\n";

struct Integral {
    double leftBorder;
    double rightBorder;
    int count;
};

int pipe_in[2];
int pipe_out[2];
pid_t pid;

void help() {
    cout << "Help\n"
         << "To calculate the integral using the rectangle method, run the program without keys\n"
         << "To change the calculation function, change the function() in the program source code \n>_<\n";
}

double read_double(const string& msg) {
    double result;
    while (true) {
        cout << msg;
        cin >> result;
        if (cin.fail() || (cin.peek() != '\n')) {
            cin.clear();
            cin.ignore(1000, '\n');
            cout << ERROR_MESSAGE;
        } else {
            cin.ignore(1000, '\n'); 
            return result;
        }
    }
}

int read_int(const string& msg) {
    int result;
    while (true) {
        cout << msg;
        cin >> result;
        if (result <= 0 || cin.fail() || (cin.peek() != '\n')) {
            cin.clear();
            cin.ignore(1000, '\n');
            cout << ERROR_MESSAGE;
        } else {
            cin.ignore(1000, '\n'); 
            return result;
        }
    }
}

void check_Border(double &leftB, double &rightB) {
    if (leftB > rightB) {
        cout << "┐(￣ヘ￣)┌ \n"
             << "Mistake. the boundaries of integration have been replaced with the correct ones\n";
        swap(leftB, rightB);
    }
}

double function(double x) {
    return -x*x + 6*x + 7; // Пример функции f(x) = -x^2 + 6x + 7
}

double calculate(double leftBorder, double rightBorder, int count) {
    double h = (rightBorder - leftBorder) / count;
    double result = 0;

    for (int i = 0; i < count; i++) {
        result += function(leftBorder + h * i);
    }

    result *= h;
    return result;
}

void frontend(int ch_p) {
    Integral data;
    string file_in_name, file_out_name;

    if (ch_p == 1) {
        data.leftBorder = read_double("Enter the beginning of the integration segment: ");
        data.rightBorder = read_double("Enter the ending of the integration segment: ");
        check_Border(data.leftBorder, data.rightBorder);
        data.count = read_int("Enter the count of the rectangles: ");
    } else {
        cout << "Please, enter name file to read >> "
             << "and the name file to write:\n";
        cin >> file_in_name >> file_out_name;

        ifstream inputFile(file_in_name);
        if (!inputFile.is_open()) {
            cout << ERROR_MESSAGE_FILE;
            exit(1);
        }

        inputFile >> data.leftBorder >> data.rightBorder >> data.count;
        check_Border(data.leftBorder, data.rightBorder);
    }

    write(pipe_in[1], &data, sizeof(Integral));
    double result;
    read(pipe_out[0], &result, sizeof(double));

    cout << "Result: " << result << endl << ">_<\n";
    
    ofstream outputFile(file_out_name);
    if (outputFile.is_open()) {
        outputFile << "Result: " << result << endl << ">_<\n";
        outputFile.close(); 
    } else {
        cout << ERROR_MESSAGE_FILE;
    }

    exit(0);
}

void backend() {
    Integral data;
    read(pipe_in[0], &data, sizeof(Integral));
    double result = calculate(data.leftBorder, data.rightBorder, data.count);
    write(pipe_out[1], &result, sizeof(double));
}

int choose_path() {
    int number_path;
    cout << "Choose the path for entering information >>\n"
         << "1 - from the console\n"
         << "2 - from the file" << endl;
    cin >> number_path;

    while (number_path != 1 && number_path != 2) {
        cout << ERROR_MESSAGE;
        cin >> number_path;
    }
    return number_path;
}

int main() {
    if (pipe(pipe_in) == -1 || pipe(pipe_out) == -1) {
        cerr << "Error creating pipes" << endl;
        return 1;
    }

    int ch_p = choose_path();
    pid = fork();

    if (pid < 0) {
        cerr << "Error: Fork error" << endl;
        return 1;
    } else if (pid > 0) {
        frontend(ch_p);
    } else {
        backend();
    }
    for (int i = 0; i < 2; i++) {
        close(pipe_in[i]);
        close(pipe_out[i]);
    }

    return 0;
}
