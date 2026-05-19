#include "widgets/CitationWidget.h"

#include <QLabel>
#include <QVBoxLayout>

CitationWidget::CitationWidget(QWidget *parent)
    : QWidget(parent)
{
    auto *layout = new QVBoxLayout(this);
    layout->addWidget(new QLabel("Citation", this));
    layout->addWidget(new QLabel("Article / quote / pravo.gov.ru link", this));
}
