#pragma once

#include <QWidget>

class QPushButton;
class QTableWidget;

class DocumentsPage : public QWidget
{
    Q_OBJECT

public:
    explicit DocumentsPage(QWidget *parent = nullptr);

    void clearTable();
    QTableWidget *table() const;

signals:
    void uploadRequested();
    void refreshRequested();

private:
    QPushButton *m_uploadButton = nullptr;
    QPushButton *m_refreshButton = nullptr;
    QTableWidget *m_table = nullptr;
};
