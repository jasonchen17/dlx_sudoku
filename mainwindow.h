#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

struct Node
{
    Node* left;
    Node* right;
    Node* up;
    Node* down;
    Node* col_header;

    int size;
    int value;
    int row_index;
    int col_index;

    Node(Node* l = nullptr, Node* r = nullptr, Node* u = nullptr, Node* d = nullptr, Node* ch = nullptr, int s = 0, int v = 0, int ri = -1, int ci = -1)
    {
        left = l ? l : this;
        right = r ? r : this;
        up = u ? u : this;
        down = d ? d : this;
        col_header = ch ? ch : this;
        size = s;
        value = v;
        row_index = ri;
        col_index = ci;
    }

    void link_left_right()
    {
        left->right = this;
        right->left = this;
    }

    void unlink_left_right()
    {
        left->right = right;
        right->left = left;
    }

    void link_up_down()
    {
        up->down = this;
        down->up = this;
        col_header->size += 1;
    }

    void unlink_up_down()
    {
        up->down = down;
        down->up = up;
        col_header->size -= 1;
    }
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

private slots:
    void on_newButton_clicked();
    void on_solveButton_clicked();
    void on_checkButton_clicked();
    void on_clearButton_clicked();

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;

    int board[9][9] = {0};
    int initial_board[9][9] = {0};

    void style_cells();
    void clear_board();
    void generate_board();
    void print_board();
    void check_board();
    void read_board();
    void save_initial_board();
    void solve_board();

    bool matrix[729][324] = { { 0 } };
    Node* root = new Node();
    std::vector<Node*> solution;

    void create_matrix_array();
    void create_matrix_linkedlist();
    void cover(Node* node);
    void uncover(Node* node);
    Node* find_matching_node(int value, int row, int col);
    void cover_unsolved_board();
    Node* find_smallest_col_header();
    bool find_solution();
    void format_solution();
};
#endif // MAINWINDOW_H
