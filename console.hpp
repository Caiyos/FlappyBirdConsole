#ifndef CONSOLE_HPP_INCLUDED
#define CONSOLE_HPP_INCLUDED

namespace console {
    // Função para definir o tamanho da janela do console
    void setConsoleWindowSize(int width, int height);

    // Função para esconder o cursor do console
    void hideConsoleCursor();

    // Função para copiar o buffer para o console
    void writeBufferToConsole();

    // Função para limpar o buffer de tela
    void clearBuffer();

    // Função para liberar memoria do buffer
    void freeBufferMemory();

    // Desenha um caracter na tela
    void drawChar(int x, int y, wchar_t c, unsigned short color = 0x000F);

    // Desenha um texto na tela
    void drawString(int x, int y, std::wstring c, unsigned short color = 0x000F);
}

#endif // CONSOLE_HPP_INCLUDED
