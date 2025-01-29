#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QPainter>

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
}

// Desenha o grid e as células vivas
void GameArea::paintEvent(QPaintEvent *event)
{
    QFrame::paintEvent(event);

    QPainter painter(this);
    painter.setPen(Qt::black);

    // Desenhar linhas do grid
    for (int i = 0; i <= width(); ++i) {
        if (i * cellSize > width()) break;
        painter.drawLine(i * cellSize, 0, i * cellSize, height());
    }
    for (int i = 0; i <= height(); ++i) {
        if (i * cellSize > height()) break;
        painter.drawLine(0, i * cellSize, width(), i * cellSize);
    }

    // Desenhar células vivas
    painter.setBrush(Qt::black);
    for (int row = 0; row < rows; ++row) {
        for (int col = 0; col < cols; ++col) {
            if (grid[row][col]) {
                painter.drawRect(col * cellSize, row * cellSize, cellSize, cellSize);
            }
        }
    }
}


// Permite alternar o estado das células ao clicar nelas
void GameArea::mousePressEvent(QMouseEvent *event)
{
    // Obter coordenadas do clique
    int x = event->pos().x();
    int y = event->pos().y();

    // Converter para índices da célula
    int col = x / cellSize;
    int row = y / cellSize;

    // Verificar se está dentro dos limites
    if (row >= 0 && row < rows && col >= 0 && col < cols) {
        grid[row][col] = !grid[row][col]; // Alternar o estado da célula
        update(); // Redesenhar a área
    }
}

// Atualiza o estado do grid com base nas regras do Jogo da Vida
void GameArea::nextGeneration()
{
    std::vector<std::vector<int>> nextGrid = grid;

    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            int liveNeighbors = 0;

            // Verifica os vizinhos ao redor
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

            // Aplica as regras do Jogo da Vida
            if (grid[i][j] == 1) {
                nextGrid[i][j] = (liveNeighbors == 2 || liveNeighbors == 3) ? 1 : 0;
            } else {
                nextGrid[i][j] = (liveNeighbors == 3) ? 1 : 0;
            }
        }
    }

    grid = nextGrid;
    update(); // Atualiza a interface
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

