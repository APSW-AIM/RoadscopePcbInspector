#include "mainwindowcontents.h"

MainWindowContents::MainWindowContents(QWidget* parent)
    : QWidget(parent)
    , m_pUi(new Ui::MainWindowContents)

{
    m_pUi->setupUi(this);
}

MainWindowContents::~MainWindowContents()
{
    delete m_pUi;
}
