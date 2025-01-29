#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <qframe.h>
#include <QMouseEvent>
#include <QFrame>
#include <QTimer>
#include <QColor>
#include <mutex>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

// Classe Main Window
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_pushButton_clicked();

    void on_horizontalSlider_valueChanged(int value);

private:
    Ui::MainWindow *ui;
    QTimer *timer;
    int interval = 1000;
};

// Classe GameArea
class GameArea : public QFrame
{
    Q_OBJECT
public:
    GameArea(QWidget *parent = nullptr);
    void nextGeneration();
    void processLines(int startRow, int endRow, std::vector<std::vector<int>> &nextGrid); // Processa linhas específicas

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

private:
    static const int rows; //100 Limite
    static const int cols; //125 Limite
    static const int cellSize;
    std::vector<std::vector<int>> grid;
    std::mutex gridMutex;
    static const int threadCount;  // Número de threads
    std::vector<QColor> threadColors;  // Cores associadas às threads
    std::vector<int> threadRowRanges;  // Faixas de linhas para cada thread
};

#endif // MAINWINDOW_H
