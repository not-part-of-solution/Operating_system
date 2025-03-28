#include <windows.h>
#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <mutex>

const std::wstring pipe_name = L"\\\\.\\pipe\\MyNamedPipe";
std::mutex cout_mutex; // Для безопасного вывода в консоль

void HandleClient(HANDLE pipe) {
    wchar_t buffer[1024];
    DWORD bytesRead;

    // Чтение первого сообщения от клиента (идентификатор)
    if (ReadFile(pipe, buffer, sizeof(buffer) - sizeof(wchar_t), &bytesRead, NULL)) {
        buffer[bytesRead / sizeof(wchar_t)] = L'\0';
        {
            std::lock_guard<std::mutex> lock(cout_mutex);
            std::wcout << L"Client connected: " << buffer << std::endl;
        }
    }
    else {
        std::lock_guard<std::mutex> lock(cout_mutex);
        std::wcerr << L"Failed to read from pipe. Error: " << GetLastError() << std::endl;
        CloseHandle(pipe);
        return;
    }

    // Основной цикл обработки сообщений
    while (true) {
        if (ReadFile(pipe, buffer, sizeof(buffer) - sizeof(wchar_t), &bytesRead, NULL)) {
            buffer[bytesRead / sizeof(wchar_t)] = L'\0';
            
            // Проверка на команду отключения
            if (wcscmp(buffer, L"DISCONNECT") == 0) {
                {
                    std::lock_guard<std::mutex> lock(cout_mutex);
                    std::wcout << L"Client disconnected: " << buffer << std::endl;
                }
                break;
            }

            // Вывод полученного сообщения
            {
                std::lock_guard<std::mutex> lock(cout_mutex);
                std::wcout << L"Received message: " << buffer << std::endl;
            }

            // Отправка ответа клиенту
            std::wstring response = L"Server received: ";
            response += buffer;
            DWORD bytesWritten;
            if (!WriteFile(pipe, response.c_str(), static_cast<DWORD>((response.size() + 1) * sizeof(wchar_t)), &bytesWritten, NULL)) {
                std::lock_guard<std::mutex> lock(cout_mutex);
                std::wcerr << L"Failed to send response. Error: " << GetLastError() << std::endl;
                break;
            }
        }
        else {
            DWORD err = GetLastError();
            std::lock_guard<std::mutex> lock(cout_mutex);
            if (err == ERROR_BROKEN_PIPE) {
                std::wcout << L"Client disconnected unexpectedly." << std::endl;
            } else {
                std::wcerr << L"Failed to read from pipe. Error: " << err << std::endl;
            }
            break;
        }
    }

    // Завершение работы с каналом
    if (!DisconnectNamedPipe(pipe)) {
        std::lock_guard<std::mutex> lock(cout_mutex);
        std::wcerr << L"Failed to disconnect pipe. Error: " << GetLastError() << std::endl;
    }
    CloseHandle(pipe);
}

void MonitorClients() {
    std::vector<std::thread> client_threads;

    while (true) {
        HANDLE pipe = CreateNamedPipeW(
            pipe_name.c_str(),  //имя каналов
            PIPE_ACCESS_DUPLEX,  // атрибут доступа
            PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,  // режим доступа
            PIPE_UNLIMITED_INSTANCES, //кол-во экземпляров
            1024,    //вход файл
            1024,     // выход файл
            0,        //тайм
            
            NULL);

        if (pipe == INVALID_HANDLE_VALUE) {
            std::lock_guard<std::mutex> lock(cout_mutex);
            std::wcerr << L"CreateNamedPipe failed. Error: " << GetLastError() << std::endl;
            Sleep(1000);
            continue;
        }

        // Ожидание подключения клиента
        BOOL connected = ConnectNamedPipe(pipe, NULL) ? TRUE : (GetLastError() == ERROR_PIPE_CONNECTED);
        if (!connected) {
            std::lock_guard<std::mutex> lock(cout_mutex);
            std::wcerr << L"Failed to connect to client. Error: " << GetLastError() << std::endl;
            CloseHandle(pipe);
            continue;
        }

        // Создание потока для обработки клиента
        client_threads.emplace_back(HandleClient, pipe);

        // Очистка завершенных потоков
        for (auto it = client_threads.begin(); it != client_threads.end(); ) {
            if (it->joinable()) {
                it->join();
                it = client_threads.erase(it);
            }
            else {
                ++it;
            }
        }
    }
}

int main() {
    std::wcout << L"Starting named pipe server..." << std::endl;
    MonitorClients();
    return 0;
}