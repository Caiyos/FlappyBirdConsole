#include <iostream>
#include <windows.h> // wincon.h, cores

#include "console.hpp"

namespace console
{

CHAR_INFO *bufferScreen;

int SCREEN_WIDTH;
int SCREEN_HEIGHT;

// Função para definir o tamanho da janela do console
void setConsoleWindowSize(int width, int height)
{
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hConsole == INVALID_HANDLE_VALUE)
    {
        std::cerr << "Erro ao obter o handle do console!" << std::endl;
        return;
    }

    SCREEN_WIDTH = width;
    SCREEN_HEIGHT = height;

    // Alocar memoria
    bufferScreen = new CHAR_INFO[SCREEN_WIDTH * SCREEN_HEIGHT];

    // Define as dimensões da janela do console
    SMALL_RECT rect;
    rect.Left = 0;
    rect.Top = 0;
    rect.Right = width - 1;
    rect.Bottom = height - 1;
    SetConsoleWindowInfo(hConsole, TRUE, &rect);

    // Define o buffer de tela para o tamanho desejado
    COORD coord;
    coord.X = width;
    coord.Y = height;
    SetConsoleScreenBufferSize(hConsole, coord);
}

// Função para esconder o cursor do console
void hideConsoleCursor()
{
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(hConsole, &cursorInfo);
    cursorInfo.bVisible = FALSE;
    SetConsoleCursorInfo(hConsole, &cursorInfo);
}

// Função para copiar o buffer para o console
void writeBufferToConsole()
{
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

    // Define o Console para exibir simbolos Unicode
    SetConsoleOutputCP(CP_UTF8);

    // Define a área a ser escrita
    COORD bufferSize = {SCREEN_WIDTH, SCREEN_HEIGHT};
    COORD bufferCoord = {0, 0};
    SMALL_RECT writeRegion = {0, 0, SCREEN_WIDTH - 1, SCREEN_HEIGHT - 1};

    // Escreve o buffer no console
    WriteConsoleOutputW(hConsole, bufferScreen, bufferSize, bufferCoord, &writeRegion);
}

// Função para limpar o buffer de tela
void clearBuffer()
{
    for (int y = 0; y < SCREEN_HEIGHT; y++)
    {
        for (int x = 0; x < SCREEN_WIDTH; x++)
        {
            bufferScreen[y * SCREEN_WIDTH + x].Char.UnicodeChar = L' ';
            bufferScreen[y * SCREEN_WIDTH + x].Attributes = 0xF0FF;
        }
    }
}

// Função para liberar memoria do buffer
void freeBufferMemory()
{
    if (bufferScreen)
        delete[] bufferScreen;
}

//
void drawChar(int x, int y, wchar_t c, unsigned short color)
{
    if (x < 0 || x >= SCREEN_WIDTH || y < 0 || y >= SCREEN_HEIGHT)
        return;

    bufferScreen[y * SCREEN_WIDTH + x].Char.UnicodeChar = c;
    bufferScreen[y * SCREEN_WIDTH + x].Attributes = color;
}

//
void drawString(int x, int y, std::wstring c, unsigned short color)
{
    if (x < 0 || x >= SCREEN_WIDTH || y < 0 || y >= SCREEN_HEIGHT) // A FAZER: TESTAR SE A STRING VAI PASSAR O LIMITE DA TELA
        return;

    for (size_t i = 0; i < c.size(); i++)
    {
        bufferScreen[y * SCREEN_WIDTH + x + i].Char.UnicodeChar = c[i];
        bufferScreen[y * SCREEN_WIDTH + x + i].Attributes = color;
    }
}

}
