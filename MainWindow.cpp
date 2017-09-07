#include "MainWindow.h"
#include <QDesktopWidget>

#include "ui_mainwindow.h"

#include "RandomVariableInputWidget.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QGroupBox>
#include <QScrollArea>
#include <QPushButton>
#include "RandomVariableInputWidget.h"
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QDir>
#include <QTextStream>
#include <QAction>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>
#include <HeaderWidget.h>
#include <FooterWidget.h>


#include "InputWidgetEDP.h"
#include "InputWidgetFEM.h"
#include "SamplingMethodInputWidget.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent)
{
  layout = new QVBoxLayout;
  
  // create a widget and make the central widget
  // set the new widgets layout to be one created
  QWidget *window = new QWidget();
  window->setLayout(layout);
  setCentralWidget(window);
  
  
  this->createActions();

  this->makeHeader();
  this->makeRV();
  this->makeFEM();
  this->makeEDP();
  this->makeUQMethod();
  this->makeFooter(); // this should be in some parent NHERI widget class
  
  QRect rec = QApplication::desktop()->screenGeometry();

  int height = 0.7*rec.height();
  int width = 0.7*rec.width();

  this->resize(width, height);
  //resize(QDesktopWidget().availableGeometry(this).size() * 0.9);

}

MainWindow::~MainWindow()
{
//    delete ui;
}

void MainWindow::makeHeader(void) {
    HeaderWidget *header = new HeaderWidget();
    header->setHeadingText(tr("DAKOTA-FEM Uncertainty Quantification Application"));

//  QLabel *header = new QLabel();
//  header->setText(tr("<html><head/><body><p><span style=\" font-size:18pt;\">DAKOTA-FEM Uncertainty Quantification Application </span></p></body></html>"));
  layout->addWidget(header);
}

void MainWindow::makeFooter(void) {
      FooterWidget *footer = new FooterWidget();
  // create label for now
 // QLabel *footer = new QLabel();
 // footer->setText(tr("This work is based on material supported by the National Science Foundation under grant 1612843-2"));
  layout->addWidget(footer);
}  

void MainWindow::makeRV(void) {
   rvWidget = new RandomVariableInputWidget();
   layout->addWidget(rvWidget);
}

void MainWindow::makeEDP(void) {
    edpWidget = new InputWidgetEDP();
    layout->addWidget(edpWidget);
}

void MainWindow::makeUQMethod(void) {
    uqWidget = new SamplingMethodInputWidget();
    layout->addWidget(uqWidget);
}

void MainWindow::makeFEM(void) {
    femWidget = new InputWidgetFEM();
    layout->addWidget(femWidget);
}

bool MainWindow::save()
{
  if (currentFile.isEmpty()) {
    return saveAs();
  } else {
    return saveFile(currentFile);
  }
}

bool MainWindow::saveAs()
{
  //
  // get filename
  //

  QFileDialog dialog(this);
  dialog.setWindowModality(Qt::WindowModal);
  dialog.setAcceptMode(QFileDialog::AcceptSave);
  if (dialog.exec() != QDialog::Accepted)
    return false;

  // and save the file
  return saveFile(dialog.selectedFiles().first());
}

void MainWindow::open()
{
  QString fileName = QFileDialog::getOpenFileName(this);
  if (!fileName.isEmpty())
    loadFile(fileName);
}



void MainWindow::newFile()
{
  // clear old
  rvWidget->clear();
  edpWidget->clear();
  femWidget->clear();
  uqWidget->clear();

  // set currentFile blank
  setCurrentFile(QString());
}


void MainWindow::setCurrentFile(const QString &fileName)
{
  currentFile = fileName;
  //  setWindowModified(false);

  QString shownName = currentFile;
  if (currentFile.isEmpty())
    shownName = "untitled.json";

  setWindowFilePath(shownName);
}

bool MainWindow::saveFile(const QString &fileName)
{
  //
  // open file
  //

  QFile file(fileName);
  if (!file.open(QFile::WriteOnly | QFile::Text)) {
    QMessageBox::warning(this, tr("Application"),
             tr("Cannot write file %1:\n%2.")
             .arg(QDir::toNativeSeparators(fileName),
                  file.errorString()));
    return false;
  }


  //
  // create a json object, fill it in & then use a QJsonDocument
  // to write the contents of the object to the file in JSON format
  //

  QJsonObject json;
  rvWidget->outputToJSON(json);

  edpWidget->outputToJSON(json);

  femWidget->outputToJSON(json);

  uqWidget->outputToJSON(json);

  QJsonDocument doc(json);
  file.write(doc.toJson());

  // close file
  file.close();

  // set current file
  setCurrentFile(fileName);

  return true;
}

void MainWindow::loadFile(const QString &fileName)
{
  //
  // open file
  //

  QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
      QMessageBox::warning(this, tr("Application"),
                             tr("Cannot read file %1:\n%2.")
                             .arg(QDir::toNativeSeparators(fileName), file.errorString()));
        return;
    }

    // place contents of file into json object
    QString val;
    val=file.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(val.toUtf8());
    QJsonObject jsonObj = doc.object();

    // close file
    file.close();

    // given the json object, create the C++ objects
    rvWidget->inputFromJSON(jsonObj);
    edpWidget->inputFromJSON(jsonObj);
   // femWidget->inputFromJSON(jsonObj);
   // uqWidget->inputFromJSON(jsonObj);

    setCurrentFile(fileName);
}


void MainWindow::createActions() {
 QMenu *fileMenu = menuBar()->addMenu(tr("&File"));


 //const QIcon openIcon = QIcon::fromTheme("document-open", QIcon(":/images/open.png"));
 //const QIcon saveIcon = QIcon::fromTheme("document-save", QIcon(":/images/save.png"));

 //QToolBar *fileToolBar = addToolBar(tr("File"));

 QAction *newAction = new QAction(tr("&New"), this);
 newAction->setShortcuts(QKeySequence::New);
 newAction->setStatusTip(tr("Create a new file"));
 connect(newAction, &QAction::triggered, this, &MainWindow::newFile);
 fileMenu->addAction(newAction);
 //fileToolBar->addAction(newAction);

 QAction *openAction = new QAction(tr("&Open"), this);
 openAction->setShortcuts(QKeySequence::Open);
 openAction->setStatusTip(tr("Open an existing file"));
 connect(openAction, &QAction::triggered, this, &MainWindow::open);
 fileMenu->addAction(openAction);
 //fileToolBar->addAction(openAction);


 QAction *saveAction = new QAction(tr("&Save"), this);
 saveAction->setShortcuts(QKeySequence::Save);
 saveAction->setStatusTip(tr("Save the document to disk"));
 connect(saveAction, &QAction::triggered, this, &MainWindow::save);
 fileMenu->addAction(saveAction);


 QAction *saveAsAction = new QAction(tr("&Save As"), this);
 saveAction->setStatusTip(tr("Save the document with new filename to disk"));
 connect(saveAsAction, &QAction::triggered, this, &MainWindow::saveAs);
 fileMenu->addAction(saveAsAction);

 // strangely, this does not appear in menu (at least on a mac)!! ..
 // does Qt not allow as in tool menu by default?
 // check for yourself by changing Quit to drivel and it works
 QAction *exitAction = new QAction(tr("&Quit"), this);
 connect(exitAction, SIGNAL(triggered()), qApp, SLOT(quit()));
 // exitAction->setShortcuts(QKeySequence::Quit);
 exitAction->setStatusTip(tr("Exit the application"));
 fileMenu->addAction(exitAction);
}
