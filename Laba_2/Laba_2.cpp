#include<iostream> 
#include <string> 
#include <fstream> 
#include <fcntl.h>
#include <sys/stat.h> 
#include <unistd.h>
#include <map>
#include <cstdio>
#include <filesystem>
#include <cstring> 
#include <functional>

using namespace std;
namespace fs = filesystem;

const string ERROR_MESSAGE = "Incorrect value, please try again.\n";
//Программа должна иметь возможность осуществлять:  
//- копирование файлов,
// - перемещение файлов,  
//- получение информации о файле (права, размер, время изменения),  
//- изменение прав на выбранный файл.
void defects(){
    cin.clear();
    cout << ERROR_MESSAGE;
}
/* 
Функция копирования файлов - название копируемого файла - название копии файла - копирует содержимое 
*/ 

void show_help(){
    cout << " Options:\n"
            "--help         Show this message\n"
            "--copy         Copy file_1 to file_2\n"
            "--move         Move file_1 to file_2\n"
            "--info         Show information about file\n"
            "--chmod        Change permissions of file1\n"
            "--exit         Exit from program\n";
}

void info(){
    //dev — номер устройства; 1
    //ino — номер inode; 1
    //mode — режим защиты inode; 1
    //nlink — количество ссылок; 1
    //uid — идентификатор пользователя владельца; 1
    //gid — идентификатор группы владельца; 1
    //rdev — тип устройства, если устройство inode; 1
    //size — размер в байтах; 1
    //atime — время последнего доступа (временная метка Unix); 1
    //mtime — время последней модификации (временная метка Unix); 1
    //ctime — время последнего изменения inode (временная метка Unix); 1
    cout << "Please, enter the name of the file>> " << endl;
    string file_first_name;
    cin >> file_first_name;
    struct stat file_info;
    stat(file_first_name.c_str(), &file_info);
    cout << "Device number" << file_info.st_dev << '\n'
        << "Inode number" << file_info.st_ino << '\n'
        << "inode protection mode" << file_info.st_mode << '\n'
        << "Count of links" << file_info.st_nlink << '\n'
        << "the owner's user ID" << file_info.st_uid << '\n'
        << "ID of the owner's group" << file_info.st_gid << '\n'
        << "device type, if the device is inode" << file_info.st_rdev << '\n'
        << "size in bytes" << file_info.st_size << '\n'
        << "last access time (Unix timestamp)" << file_info.st_atime << '\n'
        << "last modification time (Unix timestamp)" << file_info.st_mtime << '\n'
        << "the time of the last inode change (Unix timestamp)" << file_info.st_ctime << endl;
}

void chmod_file(){
    cout << "Please, enter the name of the file, and command for chmod >> " << endl;
    string file_first_name, mode;
    cin >> file_first_name >> mode;
    mode_t flags = 0;
    flags = stoul(mode, nullptr, 8);
    if(chmod(file_first_name.c_str(), flags) != 0){
        defects();
    }
    else{
        cout << "File access rights " << file_first_name << " have been successfully changed" << endl;
    }
    return;
}

void cp(){

    cout << "Please, enter the name of the file, which you want to copy>> " << endl;
    string file_first_name;
    cin >> file_first_name;
    cout << "Now, enter the name of the file copy>> "<< endl;
    string file_second_name; 
    cin >> file_second_name;
    if(file_first_name == file_second_name){
        defects();
        return;
    }

    ifstream fin;
    size_t bufsize = 4;
    char* buf = new char[bufsize];
    fin.open(file_first_name, ios::binary);
    if(fin.is_open()){
        ofstream fout;
        fout.open(file_second_name, ios::binary);
        while(!fin.eof()){
            fin.read(buf, bufsize);
            if(fin.gcount()) fout.write(buf, fin.gcount());
        }
        cout << ">>The copying was successful" << endl;
        delete[] buf;
        fin.close();
        fout.close();
    }
    else{
        defects();
        return;
    }
}

void mv(){
    cout << "Please, enter the name of the file, which you want to move>> " << endl;
    string file_first_name;
    cin >> file_first_name;
    cout << "Now, enter the name of the file move>> "<< endl;
    string file_second_name; 
    cin >> file_second_name;
    std::filesystem::path src(file_first_name);
    
    if (!std::filesystem::exists(src)) {
        std::cerr << "The file \"" << file_first_name << "\" does not exist." << std::endl;
        return;
    }
    if(rename(file_first_name.c_str(), file_second_name.c_str()) != 0 ){
        defects();
        return;
    }
    else{
        cout << ">>The moving was successful" << endl;
    }
}


int main() {
    map< string, std::function<void()>> command;
    command["--help"] = show_help;
    command["--copy"] = cp;
    command["--move"] = mv;
    command["--info"] = info;
    command["--chmod"] = chmod_file;
    while (true) {
        cout << "What do you want to do?(You can enter --help)" << endl;
        string commanda;
        cin >> commanda;

        if(commanda == "--exit"){
            break;
        }
        else{
            auto it = command.find(commanda);
            if (it != command.end()) {
                it->second(); // Вызов соответствующей функции
            } else {
                defects(); // Подсказка об ошибке
            }
        }
    }
    return 0;
}