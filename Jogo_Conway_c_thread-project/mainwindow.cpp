#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QPainter>
#include <thread>
#include <mutex>

const int GameArea::rows = 100; //100 Limite
const int GameArea::cols = 125; //125 Limite
const int GameArea::cellSize = 8;

// Construtor da classe MainWIndow
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , timer(new QTimer(this))
{
    ui->setupUi(this);
    GameArea *mesh = new GameArea(ui->Area_Jogo);
    mesh->setObjectName("GameArea");
    mesh->setGeometry(ui->Area_Jogo->rect());
    connect(timer, &QTimer::timeout, mesh, &GameArea::nextGeneration);
}

MainWindow::~MainWindow()
{
    delete ui;
}

// Construtor da classe GameArea
GameArea::GameArea(QWidget *parent)
    : QFrame(parent), grid(rows, std::vector<int>(cols, 0))
{
    setFixedSize(cols * cellSize, rows * cellSize);

    // Definir o número de threads
    threadColors = {Qt::red, Qt::green, Qt::blue, Qt::yellow};  // Cores por thread
    threadRowRanges.resize(threadColors.size() + 1);

    int rowsPerThread = rows / threadColors.size();

    for (int i = 0; i < threadColors.size(); ++i) {
        threadRowRanges[i] = i * rowsPerThread;
    }
    threadRowRanges[threadColors.size()] = rows;  // Último índice cobre todas as linhas
}

void GameArea::paintEvent(QPaintEvent *event)
{
    QFrame::paintEvent(event);

    QPainter painter(this);
    painter.setPen(Qt::black);

    // Desenhar linhas do grid
    for (int i = 0; i <= width(); ++i) {
        if (i * cellSize > width()) break;
        painter.drawLine(i * cellSize, 0, i * cellSize, height());  // Linhas verticais
    }
    for (int i = 0; i <= height(); ++i) {
        if (i * cellSize > height()) break;
        painter.drawLine(0, i * cellSize, width(), i * cellSize);  // Linhas horizontais
    }

    // Desenhar células
    for (int row = 0; row < rows; ++row) {
        // Determinar a cor da thread que processa esta linha
        int threadIndex = 0;
        for (int i = 0; i < threadColors.size(); ++i) {
            if (row < threadRowRanges[i + 1]) {
                threadIndex = i;
                break;
            }
        }

        painter.setBrush(threadColors[threadIndex]);  // Cor da thread

        for (int col = 0; col < cols; ++col) {
            if (grid[row][col]) {
                painter.drawRect(col * cellSize, row * cellSize, cellSize, cellSize);
            }
        }
    }
}


void GameArea::mousePressEvent(QMouseEvent *event)
{
    int x = event->pos().x();
    int y = event->pos().y();

    int col = x / cellSize;
    int row = y / cellSize;

    if (row >= 0 && row < rows && col >= 0 && col < cols) {
        std::lock_guard<std::mutex> lock(gridMutex);
        grid[row][col] = !grid[row][col];
        update();
    }
}

// Atualiza a geração
void GameArea::nextGeneration()
{
    std::vector<std::vector<int>> nextGrid = grid;

    std::vector<std::thread> threads;
    int linesPerThread = 3;

    for (int startRow = 0; startRow < rows; startRow += linesPerThread) {
        int endRow = std::min(startRow + linesPerThread, rows);
        threads.emplace_back(&GameArea::processLines, this, startRow, endRow, std::ref(nextGrid));
    }

    for (auto &thread : threads) {
        thread.join();
    }

    {
        std::lock_guard<std::mutex> lock(gridMutex);
        grid = nextGrid;
    }

    update();
}

// Processa as linhas específicas
void GameArea::processLines(int startRow, int endRow, std::vector<std::vector<int>> &nextGrid)
{
    for (int i = startRow; i < endRow; ++i) {
        for (int j = 0; j < cols; ++j) {
            int liveNeighbors = 0;

            for (int di = -1; di <= 1; ++di) {
                for (int dj = -1; dj <= 1; ++dj) {
                    if (di == 0 && dj == 0) continue;
                    int ni = i + di;
                    int nj = j + dj;

                    if (ni >= 0 && ni < rows && nj >= 0 && nj < cols) {
                        liveNeighbors += grid[ni][nj];
                    }
                }
            }

            if (grid[i][j] == 1) {
                nextGrid[i][j] = (liveNeighbors == 2 || liveNeighbors == 3) ? 1 : 0;
            } else {
                nextGrid[i][j] = (liveNeighbors == 3) ? 1 : 0;
            }
        }
    }
}

// SLOTS
void MainWindow::on_pushButton_clicked()
{
    if (timer->isActive()) {
        timer->stop(); // Para o timer
        ui->pushButton->setText("Iniciar"); // Altera o texto do botão
    } else {
        timer->start(interval);
        ui->pushButton->setText("Parar"); // Altera o texto do botão
    }
}

void MainWindow::on_horizontalSlider_valueChanged(int value)
{
    // Ajusta o intervalo do timer com base no valor do slider
    interval = 1000 - (value * 10);
    if (interval < 10) interval = 10;
    if (interval > 900) interval = 900;
    timer->setInterval(interval);
}

