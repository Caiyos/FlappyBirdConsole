#include <iostream>
#include <conio.h> // teclas...
#include <chrono>
#include <thread> // sleep...
#include <string>
#include <vector>
#include <algorithm>

#include "game.hpp"
#include "console.hpp"

using namespace std;
using namespace console;

namespace game
{

enum COLOR
{
    // wincon.h, biblioteca c específica para trabalhar com terminal do windows, cores prontas
    FOREGROUND_BLUE   = 0x1,
    FOREGROUND_GREEN  = 0x2,
    FOREGROUND_RED    = 0x4,
    FOREGROUND_WHITE  = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE,
    FOREGROUND_YELLOW = FOREGROUND_RED | FOREGROUND_GREEN,

    BACKGROUND_BLUE   = 0x10,
    BACKGROUND_GREEN  = 0x20,
    BACKGROUND_RED    = 0x40,
    BACKGROUND_WHITE  = BACKGROUND_BLUE | BACKGROUND_GREEN | BACKGROUND_RED | 0x80
};

typedef struct _PIXEL
{
    // tipo de char maior que suporta o unicode que consta no wincon.h
    wchar_t unicode; // usa elementos de blocos do unicode, maior variedade de caracteres
    COLOR color;
} PIXEL;

typedef struct _BIRD
{
    float x;
    float y;
    float velocityY;
    int wing;
} BIRD;
BIRD bird;

typedef struct _GROUND
{
    float x;
    float y;
    short height;
    float velocityX;
    PIXEL texture[6]; // Aqui, não é necessário ultilizar ponteiro como é usado abaixo pois
                      // posteriormente não são usadas várias variáveis de ground
    int width;
} GROUND;
GROUND ground;

typedef struct _PIPE
{
    float x;
    float y;
    short height;
    float velocityX;
    PIXEL *texture;
    int width;
    bool scored;
} PIPE;
vector<PIPE> pipes;
// dinamico, tipo de dados PIPE

const short PIPE_WIDTH = 4;
const short GAP_SIZE = 6;
const float BIRD_X = 10;
int frame = 0;
// add fps


PIXEL *pipeTexture;

// Controlar os estados do jogo
bool isRunning = true;
bool isWaiting = true;
bool isGameOver = false;

int FPS = 0;
float SCORE = 0.0;
int BEST = 0;

// Controlar a força gravitacional do jogo
const float gravity = 1.0;
const float velocity = -6.0;

void highestScoreUpdate()
{
    if(SCORE>BEST)
    {
        BEST = SCORE;

        // w irá sobrescrever o melhor score antigo
        FILE *file = fopen("highestScore.txt", "w");
        if (file == NULL)
        {
            exit(1);
        }

        fprintf(file, "Melhor score historico foi %.1f", SCORE);

        fclose(file);
    }

}

void highestScoreRead()
{
    FILE *file = fopen("highestScore.txt", "r");
    if (file == NULL) {
        return;
    }

    char linha[32];  // Buffer que armazena cada linha
    float bestHistorico = 0.0;

    if (fgets(linha, sizeof(linha), file) != NULL)
    {
        sscanf(linha, "%*s %*s %*s %*s %f", &bestHistorico);
    }

    fclose(file);
    BEST = bestHistorico;
}

// Configuração inicial do jogo
void setup()
{
    console::setConsoleWindowSize(SCREEN_WIDTH, SCREEN_HEIGHT);
    console::hideConsoleCursor();

    srand((unsigned)time(NULL)); // usada para definir a semente (seed) do gerador
                                 // de números pseudo-aleatórios usado pela função rand

    /* Textura do chão
    *
    *  ▓▒▒
    *  ▒▒▒
    */
    ground.texture[0].unicode = L'▓';
    ground.texture[0].color = FOREGROUND_WHITE;
    ground.texture[1].unicode = L'▒';
    ground.texture[1].color = FOREGROUND_WHITE;
    ground.texture[2].unicode = L'▒';
    ground.texture[2].color = FOREGROUND_WHITE;
    ground.texture[3].unicode = L'▒';
    ground.texture[3].color = FOREGROUND_WHITE;
    ground.texture[4].unicode = L'▒';
    ground.texture[4].color = FOREGROUND_WHITE;
    ground.texture[5].unicode = L'▒';
    ground.texture[5].color = FOREGROUND_WHITE;
    ground.height = 3;
    ground.x = 0.f;
    ground.y = SCREEN_HEIGHT - ground.height;
    ground.velocityX = 6.f;
    ground.width = 3;

    // Alocar memória para textura
    pipeTexture = new PIXEL[PIPE_WIDTH];
    // Ponteiro agora aponta para o local
    // alocado dinamicamente com o new, com o tamanho dos
    // 4 pixeis necessários

    /* Textura do tubo
    *
    *  ▓▒▒░
    */
    pipeTexture[0].unicode = L'▓';
    pipeTexture[0].color = FOREGROUND_GREEN;
    pipeTexture[1].unicode = L'▒';
    pipeTexture[1].color = FOREGROUND_GREEN;
    pipeTexture[2].unicode = L'▒';
    pipeTexture[2].color = FOREGROUND_GREEN;
    pipeTexture[3].unicode = L'░';
    pipeTexture[3].color = FOREGROUND_GREEN;

    bird.x = BIRD_X;
    bird.y = 10.0;
    bird.velocityY = 0.0;
    bird.wing = 0;

    highestScoreRead();
}

void drawGround()
{
    int dx0 = (int)ground.x  % ground.width; // x dividido por 3 sempre será ou 0, ou 1 ou 2
    int dx1 = (int)(dx0 + 1) % ground.width; // se x0 = 0 x1 = 1
    int dx2 = (int)(dx0 + 2) % ground.width; // se x0 = 0 x2 = 2

    for (int i = 0; i < SCREEN_WIDTH - 2;)
    {
        // Linha 1 -> gera animação por intercalação
        drawChar(i    , ground.y, ground.texture[dx0].unicode, ground.texture[dx0].color);
        drawChar(i + 1, ground.y, ground.texture[dx1].unicode, ground.texture[dx1].color);
        drawChar(i + 2, ground.y, ground.texture[dx2].unicode, ground.texture[dx2].color);
        // Linha 2 -> fixa
        drawChar(i    , ground.y + 1, ground.texture[3].unicode, ground.texture[3].color);
        drawChar(i + 1, ground.y + 1, ground.texture[4].unicode, ground.texture[4].color);
        drawChar(i + 2, ground.y + 1, ground.texture[5].unicode, ground.texture[5].color);
        // Duplicar a linha 2
        drawChar(i    , ground.y + 2, ground.texture[3].unicode, ground.texture[3].color);
        drawChar(i + 1, ground.y + 2, ground.texture[4].unicode, ground.texture[4].color);
        drawChar(i + 2, ground.y + 2, ground.texture[5].unicode, ground.texture[5].color);
        i += 3;
    }
}

void drawPipes()
{
    // se addPipes ainda não foi chamado, não entra no loop pois o vector está vazio

    for (auto& pipe : pipes) // O loop for percorre cada elemento da coleção pipes.
                             // Para cada iteração, a variável pipe aponta para o elemento atual
                             // da coleção que está sendo referenciado.
    {
        for (int y = pipe.y; y < pipe.y + pipe.height; y++) // pinta da coordenada inicial até altura
        {
            drawChar(pipe.x    , y, pipe.texture[0].unicode, pipe.texture[0].color);
            drawChar(pipe.x + 1, y, pipe.texture[1].unicode, pipe.texture[1].color);
            drawChar(pipe.x + 2, y, pipe.texture[2].unicode, pipe.texture[2].color);
            drawChar(pipe.x + 3, y, pipe.texture[3].unicode, pipe.texture[3].color);
        }
    }
}

void drawBird()
{
    if(bird.wing == 0)
    {
        drawChar(bird.x - 1, bird.y, L'^', FOREGROUND_YELLOW | BACKGROUND_WHITE);
    }
    else
    {
        drawChar(bird.x - 1, bird.y, L'v', FOREGROUND_YELLOW | BACKGROUND_WHITE);
    }

    drawChar(bird.x, bird.y, L'►', FOREGROUND_YELLOW | BACKGROUND_WHITE); // o char precisa do L (setta como long)
                                                                          // porque o tipo unicode utiliza 8+ bits
    // 'v►' '^►' 'v►' animação
}

// Gerar novos pares de tubos
void addPipes(double deltaTime)
{
    static double dt;

    if (isGameOver) dt = 0.0;

    dt += deltaTime;

    if (dt > 3) // Adiciona novos tubos a cada 3 segundos
    {
        short gapPosition = 3 + rand() % 14; // (32767 % 14) = número inteiro no intervalo de 0 a 13
        // intervalo final: 3 a 16
        PIPE pipeUp;
        pipeUp.texture = pipeTexture;
        pipeUp.height = gapPosition;
        pipeUp.x = SCREEN_WIDTH;
        pipeUp.y = 0;
        pipeUp.velocityX = velocity;
        pipeUp.width = PIPE_WIDTH;
        pipeUp.scored = false;

        pipes.push_back(pipeUp);

        PIPE pipeDown;
        pipeDown.texture = pipeTexture;
        pipeDown.height = SCREEN_HEIGHT - ground.height - gapPosition - GAP_SIZE;
        pipeDown.x = SCREEN_WIDTH;
        pipeDown.y = ground.y - pipeDown.height;
        pipeDown.velocityX = velocity;
        pipeDown.width = PIPE_WIDTH;
        pipeDown.scored = false;

        pipes.push_back(pipeDown);

        dt = 0.0;
    }
}

// Remover pares de tubos fora da tela
void removePipes()
{
    pipes.erase(std::remove_if(pipes.begin(), pipes.end(), [](PIPE pipe)
    {
        return pipe.x < -PIPE_WIDTH;
    }), pipes.end());
}

// Verificar colisão com o chão e tubos
bool collision()
{
    if (bird.y >= ground.y)
    {
        bird.y = ground.y - 1;
        bird.velocityY = 0.0;

        return true;
    }

    for (auto& pipe : pipes)
    {
        if (!pipe.scored && pipe.x <= bird.x) // Passando pelo pipe
        {
            if (bird.y >= pipe.y && bird.y <= (pipe.y + pipe.height)) // Checa se colidindo
            {
                return true;
            }
        }
    }

    return false;
}

// Limpa e reinicia o jogo
void resetGame()
{
    isRunning = true;
    isWaiting = true;
    isGameOver = false;

    FPS = 0;
    SCORE = 0.0;

    bird.x = BIRD_X;
    bird.y = 10.0;
    bird.velocityY = 0.0;

    ground.height = 3;
    ground.x = 0.0;
    ground.y = SCREEN_HEIGHT - ground.height;
    ground.velocityX = 6.0;
    ground.width = 3;

    pipes.clear();
}

// Função para processar interação com o jogador
void processInput()
{
    if (kbhit()) // checa se tecla pressionada
    {
        char ch = getch();
        if (ch == 13 && isGameOver) // ENTER
        {
            resetGame();
        }
        if (ch == 32) // BARRA DE ESPAÇO
        {
            if (isWaiting) isWaiting = false;

            if (bird.y > 2)
            {
                bird.velocityY = -0.35;
            }
        }
        if (ch == 27) // ESC
        {
            isRunning = false;
        }
    }
}

// Função para atualizar a lógica do jogo
void update(double deltaTime)
{
    frame++;

    if(isWaiting){
        // GROUND
        ground.x += ground.velocityX * deltaTime;

        return;
    }

    // BIRD
    bird.velocityY += gravity * deltaTime;
    bird.y += bird.velocityY;

    // WINGS
    if(frame%20 == 0){
        bird.wing = 1-bird.wing;
    }

    // GROUND
    ground.x += ground.velocityX * deltaTime;

    // PIPES
    removePipes();

    for (auto& pipe : pipes)
    {
        pipe.x += pipe.velocityX * deltaTime;

        // SCORE
        if (!pipe.scored && pipe.x < BIRD_X - PIPE_WIDTH)
        {
            SCORE += .5;
            pipe.scored = true;
        }
    }

    // GAMEOVER?
    if (collision())
    {
        isGameOver = true;
        highestScoreUpdate();
    }


    // A partir daqui lógicas e funções que dependem do status GAMEOVER

    // PIPES --
    addPipes(deltaTime);
}

// Função para desenhar a tela do jogo
void render()
{
    // Limpa o buffer de tela
    clearBuffer();

    drawGround();
    drawPipes();
    drawBird();

    drawString(86, 1, L"FPS: " + to_wstring(FPS));
    drawString( 1, 1, L"SCORE: " + to_wstring((int)SCORE));
    drawString( 1, 3, L"BEST: " + to_wstring((int)BEST));

    if (isGameOver)
    {
        drawString(41, 11, L"-- GAMEOVER --");
        drawString(31, 13, L"PRESSIONE  -- ESC --  PARA SAIR");
        drawString(29, 14, L"PRESSIONE -- ENTER -- PARA RECOMEÇAR");
    }

    if (isWaiting)
        drawString(25, 12, L"-- PRESSIONE A BARRA DE SPAÇO PARA COMEÇAR --");

    writeBufferToConsole();
}

void calculateFramesPerSecond(double deltaTime)
{
    static int frames;
    static double dt;

    frames++;
    dt += deltaTime;

    if (dt >= 1.0)
    {
        FPS = frames;

        frames = 0;
        dt = 0;
    }
}

// Função que trata o loop principal do jogo
int run()
{
    // Setup inicial do jogo
    setup();

    using clock = chrono::high_resolution_clock;

    auto lastTime = clock::now();
    double frameTime = 1.0 / 60.0; // Aproximadamente 60 FPS

    // Loop do jogo
    while (isRunning)
    {
        auto current = clock::now();
        chrono::duration<double> elapsed = current - lastTime;
        lastTime = current;
        double deltaTime = elapsed.count();

        // Processa interação com o jogador
        processInput();

        // Atualiza a lógica do jogo
        if (!isGameOver)
            update(frameTime); // Como o FPS é constante, deve passar o frameTime fixo

        // Renderiza a tela do jogo
        render();

        // Calcula o tempo necessário para manter uma taxa de quadros constante
        if (deltaTime < frameTime)
            this_thread::sleep_for(chrono::duration<double>(frameTime - deltaTime));

        // Calcula o FPS do jogo
        current = clock::now();
        elapsed = current - lastTime;
        deltaTime = elapsed.count();

        calculateFramesPerSecond(deltaTime);
    }

    // Libera memoria alocadas
    delete[] pipeTexture;
    freeBufferMemory();

    return 0;
}

}
