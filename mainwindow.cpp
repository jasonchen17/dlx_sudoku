#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <vector>
#include <cstdlib>
#include <ctime>
#include <QMessageBox>
#include <random>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle("Sudoku");
    style_cells();
    std::srand(std::time(0));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::style_cells()
{
    for (int index = 0; index < 81; index++)
    {
        QLineEdit* lineEdit = findChild<QLineEdit*>("lineEdit_" + QString::number(index));
        lineEdit->setAlignment(Qt::AlignCenter);
        lineEdit->setStyleSheet
        (
            "QLineEdit "
            "{"
                "border: 1px solid grey;"
                "border-radius: 0;"
                "font-weight: bold;"
                "font-size: 24px;"
                "color: grey;"
            "}"
        );
        lineEdit->setReadOnly(false);
    }
}

void MainWindow::on_clearButton_clicked()
{
    clear_board();
    style_cells();
    print_board();
}

void MainWindow::on_newButton_clicked()
{
    clear_board();
    style_cells();
    generate_board();
    save_initial_board();
    print_board();
}

void MainWindow::clear_board()
{
    memset(board, 0, sizeof(board));
    memset(initial_board, 0, sizeof(initial_board));
    memset(matrix, 0, sizeof(matrix));
    root = new Node();
    solution.clear();
}

void MainWindow::on_solveButton_clicked()
{
    read_board();
    solve_board();
    print_board();
}

void MainWindow::on_checkButton_clicked()
{
    check_board();
}

void MainWindow::generate_board()
{
    std::vector<std::pair<int, int>> board_positions;

    for (int row = 0; row < 9; ++row)
    {
        for (int col = 0; col < 9; ++col)
        {
            board_positions.emplace_back(row, col);
        }
    }

    std::shuffle(board_positions.begin(), board_positions.end(), std::default_random_engine(std::rand()));
    int value = 9;

    for (int i = 0; i < 9; i++)
    {
        int row = board_positions[i].first;
        int col = board_positions[i].second;
        board[row][col] = value;
        value -= 1;
    }

    solve_board();
    int remove_count;
    QString difficulty = ui->difficultyBox->currentText();

    if (difficulty == "Easy")
    {
        remove_count = 40;
    }
    else if (difficulty == "Medium")
    {
        remove_count = 50;
    }
    else if (difficulty == "Hard")
    {
        remove_count = 60;
    }
    else if (difficulty == "Very Hard")
    {
        remove_count = 70;
    }

    std::shuffle(board_positions.begin(), board_positions.end(), std::default_random_engine(std::rand()));

    for (int i = 0; i < remove_count; i++)
    {
        int row = board_positions[i].first;
        int col = board_positions[i].second;
        board[row][col] = 0;
    }
}

void MainWindow::print_board()
{
    for (int row = 0; row < 9; row++)
    {
        for (int col = 0; col < 9; col++)
        {
            QString value;

            if (board[row][col] == 0)
            {
                value = "";
            }
            else
            {
                value = QString::number(board[row][col]);
            }

            int index = row * 9 + col;
            QLineEdit* lineEdit = findChild<QLineEdit*>("lineEdit_" + QString::number(index));
            lineEdit->setText(value);

            if (initial_board[row][col])
            {
                lineEdit->setStyleSheet(lineEdit->styleSheet() + "QLineEdit { color: deepskyblue; }");
                lineEdit->setReadOnly(true);
            }
        }
    }
}

void MainWindow::check_board()
{
    int rows[9][9] = {0};
    int cols[9][9] = {0};
    int subgrids[9][9] = {0};

    for (int row = 0; row < 9; row++)
    {
        for (int col = 0; col < 9; col++)
        {
            int value = board[row][col] - 1;
            int subgrid = row / 3 * 3 + col / 3;

            if (!board[row][col] || rows[row][value] || cols[col][value] || subgrids[subgrid][value])
            {
                QMessageBox::information(this, "Sudoku Solver", "The board is incorrect");
                return;
            }

            rows[row][value] = 1;
            cols[col][value] = 1;
            subgrids[subgrid][value] = 1;
        }
    }

    QMessageBox::information(this, "Sudoku Solver", "The board is correct");
}

void MainWindow::read_board()
{
    for (int row = 0; row < 9; row++)
    {
        for (int col = 0; col < 9; col++)
        {
            int index = row * 9 + col;
            QLineEdit* lineEdit = findChild<QLineEdit*>("lineEdit_" + QString::number(index));
            QString value = lineEdit->text();

            if (value.isEmpty())
            {
                board[row][col] = 0;
            }
            else
            {
                board[row][col] = value.toInt();
            }
        }
    }
}

void MainWindow::save_initial_board()
{
    for (int row = 0; row < 9; row++)
    {
        for (int col = 0; col < 9; col++)
        {
            initial_board[row][col] = board[row][col];
        }
    }
}

void MainWindow::solve_board()
{
    create_matrix_array();
    create_matrix_linkedlist();
    cover_unsolved_board();

    if (find_solution())
    {
        format_solution();
    }
    else
    {
        QMessageBox::information(this, "Sudoku Solver", "No solution found");
    }
}

void MainWindow::create_matrix_array()
{
    // Matrix for exact cover problem
    // 729 rows for (9 rows * 9 cols * 9 values at each cell)
    // 324 columns for (4 constraints * 9 rows * 9 cols)

    // Cell constraint: no more than one value in each cell
    // Columns 0-80
    // Example:
    // Rows 0-9, "100000000", contain value 1-9 in cell 1
    // Rows 10-18,"010000000", contain value 1-9 in cell 2

    // Row constraint: no more than one unique value in each row
    // Columns 81-161
    // Example:
    // Row 0-9 means values 1-9 in row 1
    // Row 10-18 means values 1-9 in row 1

    // Column constraint: no more than one unique value in each column
    // Columns 162-242
    // Example:
    // Row 0-9 means values 1-9 in column 1
    // Row 10-18 means values 1-9 in column 2

    // Subgrid constraint: no more than one unique value in each 3x3 subgrid
    // Columns 243-324
    // Example:
    // Row 0-9 means values 1-9 in subgrid 1
    // Row 10-18 means values 1-9 in subgrid 1

    int value_counter = 0;
    int row_constraint_offset = 81;
    int col_counter = 0;
    int subgrid_constraint_offset = 243;
    int col_constraint_offset = 162;

    for (int i = 0; i < 729; i++)
    {
        if (i != 0)
        {
            if (i % 243 == 0)
            {
                // New subgrid row
                subgrid_constraint_offset += 9;
            }
            else if (i % 81 == 0)
            {
                // New subgrid subrow
                subgrid_constraint_offset -= 18;
            }
            else if (i % 27 == 0)
            {
                // New subgrid column
                subgrid_constraint_offset += 9;
            }

            if (i % 81 == 0)
            {
                // New row
                row_constraint_offset += 9;

                // Reset columns
                col_counter = 0;
            }
        }

        // Cell constraint
        matrix[i][i/9] = 1;

        // Row constraint
        matrix[i][value_counter + row_constraint_offset] = 1;

        // Column constraint
        matrix[i][col_counter + col_constraint_offset] = 1;

        // Subgrid constraint
        matrix[i][value_counter + subgrid_constraint_offset] = 1;

        // New value
        value_counter += 1;

        // New column
        col_counter += 1;

        // New cell
        if ((i+1) % 9 == 0)
        {
            value_counter = 0;
        }
    }
}

void MainWindow::create_matrix_linkedlist()
{
    // Only overlapping rows and columns are directly linked
    // Non-overlapping rows and columns are loosely linked through the column header
    // So when we cover, we only remove overlapping rows and columns

    Node* curr_ptr = root;

    // Create and link column headers
    for (int i = 0; i < 324; i++)
    {
        Node* new_node = new Node(curr_ptr, root, nullptr, nullptr, nullptr, 0, 0, -1, -1);
        curr_ptr->right = new_node;
        curr_ptr = new_node;
    }

    int current_value = 0;
    int current_row = 1;
    int current_col = 1;

    for (int i = 0; i < 729; i++)
    {
        current_value += 1;

        if (i != 0)
        {
            if (i % 81 == 0)
            {
                // New row
                current_value = 1;
                current_row += 1;
                current_col = 1;
            }
            else if (i % 9 == 0)
            {
                // New col
                current_value = 1;
                current_col += 1;
            }
        }

        Node* col_header = root->right;
        Node* prev = nullptr;

        for (int j = 0; j < 324; j++)
        {
            if (matrix[i][j])
            {
                Node* new_node = new Node(nullptr, nullptr, col_header->up, col_header, col_header, 0, current_value, current_row, current_col);

                // Link row
                if (prev)
                {
                    new_node->left = prev;
                    new_node->right = prev->right;
                    new_node->right->left = new_node;
                    prev->right = new_node;
                }


                // Link column
                col_header->up->down = new_node;
                col_header->up = new_node;
                col_header->size += 1;

                if (col_header->down == col_header)
                {
                    col_header->down = new_node;
                }

                prev = new_node;
            }

            col_header = col_header->right;
        }
    }
}

void MainWindow::cover(Node* node)
{
    node = node->col_header;
    node->unlink_left_right();
    Node* down_ptr = node->down;

    while (down_ptr != node)
    {
        Node* right_ptr = down_ptr->right;

        while (right_ptr != down_ptr)
        {
            right_ptr->unlink_up_down();
            right_ptr = right_ptr->right;
        }

        down_ptr = down_ptr->down;
    }
}

void MainWindow::uncover(Node* node)
{
    node = node->col_header;
    node->link_left_right();
    Node* up_ptr = node->up;

    while (up_ptr != node)
    {
        Node* left_ptr = up_ptr->left;

        while (left_ptr != up_ptr)
        {
            left_ptr->link_up_down();
            left_ptr = left_ptr->left;
        }

        up_ptr = up_ptr->up;
    }
}

Node* MainWindow::find_matching_node(int value, int row, int col)
{
    Node* col_header = root->right;

    while (col_header != root)
    {
        Node* node = col_header->down;

        while (node != col_header)
        {
            if (node->value == value && (node->row_index - 1) == row && (node->col_index - 1) == col)
            {
                return node;
            }

            node = node->down;
        }

        col_header = col_header->right;
    }

    return nullptr;
}

void MainWindow::cover_unsolved_board()
{
    for (int row = 0; row < 9; row++)
    {
        for (int col = 0; col < 9; col++)
        {
            if (!board[row][col])
            {
                continue;
            }

            Node* matching_node = find_matching_node(board[row][col], row, col);

            if (matching_node)
            {
                solution.push_back(matching_node);

                // Cover columns of whole row
                Node* curr_ptr = matching_node;

                do
                {
                    cover(curr_ptr);
                    curr_ptr = curr_ptr->right;
                }
                while (curr_ptr != matching_node);
            }
        }
    }
}

Node* MainWindow::find_smallest_col_header() {
    Node* col_header = root->right;
    Node* smallest = root->right;

    while (col_header != root)
    {
        if (col_header->size < smallest->size)
        {
            smallest = col_header;
        }

        col_header = col_header->right;
    }

    return smallest;
}

bool MainWindow::find_solution()
{
    if (root->right == root)
    {
        return true;
    }

    Node* col_header = find_smallest_col_header();
    cover(col_header);
    Node* down_ptr = col_header->down;

    while (down_ptr != col_header)
    {
        solution.push_back(down_ptr);
        Node* right_ptr = down_ptr->right;

        while (right_ptr != down_ptr)
        {
            cover(right_ptr);
            right_ptr = right_ptr->right;
        }

        if (find_solution())
        {
            return true;
        }

        Node* left_ptr = down_ptr->left;

        while (left_ptr != down_ptr)
        {
            uncover(left_ptr);
            left_ptr = left_ptr->left;
        }

        solution.pop_back();
        down_ptr = down_ptr->down;
    }

    uncover(col_header);
    return false;
}

void MainWindow::format_solution()
{
    for (Node* node : solution)
    {
        board[node->row_index - 1][node->col_index - 1] = node->value;
    }
}
