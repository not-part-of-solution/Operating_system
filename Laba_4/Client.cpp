#include <windows.h>
#include <iostream>
#include <thread>
#include <string>
#include <vector>

const std::wstring pipe_name = L"\\\\.\\pipe\\MyNamedPipe";

void SendMessageToServer(int idClient) {
    HANDLE pipe = INVALID_HANDLE_VALUE;
    const DWORD bufferSize = 1024;
    wchar_t buffer[bufferSize]; //одинаково должно быть
    DWORD bytesWritten;

    // Пытаемся подключиться к серверу
    while (true) {
        if (WaitNamedPipeW(pipe_name.c_str(), NMPWAIT_USE_DEFAULT_WAIT)) {
            pipe = CreateFileW(
                pipe_name.c_str(),
                GENERIC_READ | GENERIC_WRITE,
                0,
                NULL,
                OPEN_EXISTING,
                0,
                NULL);

            if (pipe != INVALID_HANDLE_VALUE) {
                break;
            }
        }

        DWORD lastError = GetLastError();
        if (lastError == ERROR_FILE_NOT_FOUND) {
            std::wcerr << L"Waiting for server to start..." << std::endl;
            Sleep(1000);
            continue;
        }

        std::wcerr << L"Could not open pipe. Error: " << lastError << std::endl;
        return;
    }

    // Устанавливаем режим работы с каналом
    DWORD mode = PIPE_READMODE_MESSAGE;
    if (!SetNamedPipeHandleState(pipe, &mode, NULL, NULL)) {
        std::wcerr << L"SetNamedPipeHandleState failed. Error: " << GetLastError() << std::endl;
        CloseHandle(pipe);
        return;
    }

    // Отправляем приветственное сообщение
    std::wstring connectMessage = L"Client" + std::to_wstring(idClient) + L" connect";
    if (!WriteFile(pipe, connectMessage.c_str(), (connectMessage.length() + 1) * sizeof(wchar_t), &bytesWritten, NULL)) {
        std::wcerr << L"Failed to write to pipe. Error: " << GetLastError() << std::endl;
        CloseHandle(pipe);
        return;
    }

    // Основной цикл обмена сообщениями
    while (true) {
        std::wstring message;
        std::wcout << L"Enter message for server (or 'exit' to disconnect): ";
        std::getline(std::wcin, message);

        if (message == L"exit") {
            const wchar_t* disconnectMsg = L"DISCONNECT";
            if (!WriteFile(pipe, disconnectMsg, (wcslen(disconnectMsg) + 1) * sizeof(wchar_t), &bytesWritten, NULL)) {
                std::wcerr << L"Failed to send disconnect message. Error: " << GetLastError() << std::endl;
            }
            break;
        }

        if (!WriteFile(pipe, message.c_str(), (message.length() + 1) * sizeof(wchar_t), &bytesWritten, NULL)) {
            std::wcerr << L"Failed to write to pipe. Error: " << GetLastError() << std::endl;
            break;
        }

        // Чтение ответа от сервера
        DWORD bytesRead;
        if (ReadFile(pipe, buffer, bufferSize * sizeof(wchar_t), &bytesRead, NULL)) {
            buffer[bytesRead / sizeof(wchar_t)] = L'\0';
            std::wcout << L"Server response: " << buffer << std::endl;
        }
        else {
            std::wcerr << L"Failed to read response. Error: " << GetLastError() << std::endl;
            break;
        }
    }

    // Корректное закрытие соединения
    if (!FlushFileBuffers(pipe)) {
        std::wcerr << L"FlushFileBuffers failed. Error: " << GetLastError() << std::endl;
    }
    CloseHandle(pipe);
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::wcerr << L"Usage: " << argv[0] << " <client_id>" << std::endl;
        return 1;
    }

    int clientId = std::stoi(argv[1]);
    SendMessageToServer(clientId);

    return 0;
}