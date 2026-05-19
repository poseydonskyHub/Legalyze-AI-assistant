#pragma once

#include <QWidget>

class QLineEdit;
class QPushButton;
class QSpinBox;
class QTableWidget;

class KnowledgeBasePage : public QWidget
{
    Q_OBJECT

public:
    explicit KnowledgeBasePage(QWidget *parent = nullptr);

    QString url() const;
    int maxDocuments() const;
    void clearTable();
    QTableWidget *table() const;

signals:
    void ingestRequested();
    void refreshRequested();

private:
    QLineEdit *m_urlEdit = nullptr;
    QSpinBox *m_maxDocuments = nullptr;
    QPushButton *m_ingestButton = nullptr;
    QPushButton *m_refreshButton = nullptr;
    QTableWidget *m_table = nullptr;
};
