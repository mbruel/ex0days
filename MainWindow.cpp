//========================================================================
//
// Copyright (C) 2020 Matthieu Bruel <Matthieu.Bruel@gmail.com>
//
// This file is a part of ex0days : https://github.com/mbruel/ex0days
//
// ex0days is free software; you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as
// published by the Free Software Foundation; version 3.0 of the License.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// You should have received a copy of the GNU Lesser General Public
// License along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301,
// USA.
//
//========================================================================

#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "Ex0days.h"

#include <QDragEnterEvent>
#include <QMimeData>
#include <QFileInfo>
#include <QMessageBox>
#include <QProgressBar>
#include <QFileDialog>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , _ui(new Ui::MainWindow),
      _progressBar(new QProgressBar(this)),
      _app(nullptr),
      _state(STATE::IDLE)
{
    _ui->setupUi(this);
    setAcceptDrops(true);

    _ui->foldersBox->setTitle(tr("Folders"));
    _ui->foldersBox->setStyleSheet(sGroupBoxStyle);

    _ui->compressionBox->setTitle(tr("Compression"));
    _ui->compressionBox->setStyleSheet(sGroupBoxStyle);

    _ui->logBox->setTitle(tr("Logs"));
    _ui->logBox->setStyleSheet(sGroupBoxStyle);

    _ui->splitter->setStyleSheet(sSplitterStyle);
    _ui->splitter->setSizes({500, 100});

    connect(_ui->zipButton,  &QAbstractButton::clicked, this, &MainWindow::on7zipPath);
    connect(_ui->rarButton,  &QAbstractButton::clicked, this, &MainWindow::onUnrarPath);
    connect(_ui->aceButton,  &QAbstractButton::clicked, this, &MainWindow::onUnacePath);
    connect(_ui->arjButton,  &QAbstractButton::clicked, this, &MainWindow::onArjPath);
    connect(_ui->dstButton,  &QAbstractButton::clicked, this, &MainWindow::onDstPath);

    connect(_ui->testOnlyCB, &QAbstractButton::toggled, this, &MainWindow::onTestOnly);
    connect(_ui->delSrcCB,   &QAbstractButton::toggled, this, &MainWindow::onDelSrc);
    connect(_ui->debugCB,    &QAbstractButton::toggled, this, &MainWindow::onDebugToggled);
    connect(_ui->dispCompressionCB, &QAbstractButton::toggled, this, &MainWindow::onDispCompressionPaths);

    connect(_ui->clearSrcButton, &QAbstractButton::clicked, _ui->srcList,    &SignedListWidget::onDeleteSelectedItems);
    connect(_ui->addSrcButton,   &QAbstractButton::clicked, this,            &MainWindow::onAddSrc);
    connect(_ui->clearLogButton, &QAbstractButton::clicked, _ui->logBrowser, &QTextBrowser::clear);

    connect(_ui->launchButton,   &QAbstractButton::clicked, this,            &MainWindow::onLaunch);

    _ui->srcList->setSelectionMode(QAbstractItemView::ExtendedSelection);
    connect(_ui->srcList, &SignedListWidget::rightClick, this, &MainWindow::onAddSrc);

    statusBar()->addPermanentWidget(_progressBar, 2);
}

MainWindow::~MainWindow()
{
    delete _ui;
}

void MainWindow::setAsciiSignature(const QString &ascii)
{
    _ui->srcList->setAsciiSignature(ascii);
}

void MainWindow::init(Ex0days *app)
{
    _app = app;

    setIDLE();
    setProgressMax(100);

    _ui->zipLE->setText(_app->setting(Ex0days::Param::cmd7z));
    _ui->rarLE->setText(_app->setting(Ex0days::Param::cmdRar));
    _ui->aceLE->setText(_app->setting(Ex0days::Param::cmdAce));
    _ui->arjLE->setText(_app->setting(Ex0days::Param::cmdArj));
    _ui->dstLE->setText(_app->setting(Ex0days::Param::dstDir));

    _ui->testOnlyCB->setChecked(_app->testOnly());
    _ui->delSrcCB->setChecked(_app->delSrc());
    _ui->debugCB->setChecked(_app->debug());
    _ui->dispCompressionCB->setChecked(_app->dispPaths());
    onDispCompressionPaths(_app->dispPaths());


#ifdef __DEBUG__
//    _ui->srcList->addPath("/tmp/ahbaz/1999.01.01", true);
    _ui->srcList->addPath("/tmp/ahbaz/test1", true);
#endif

    connect(_ui->aboutButton,  &QAbstractButton::clicked, app, &Ex0days::onAbout);
    connect(_ui->donateButton, &QAbstractButton::clicked, app, &Ex0days::onDonate);
}

void MainWindow::setIDLE()
{
    _state = STATE::IDLE;
    _ui->launchButton->setText(tr("Launch"));
}

void MainWindow::setProgressMax(int max)
{
    _progressBar->setRange(0, max);
    _progressBar->setValue(0);
}

void MainWindow::setProgress(int value)
{
    _progressBar->setValue(value);
}

void MainWindow::log(const QString &msg)
{
    _ui->logBrowser->append(msg);
}
void MainWindow::success(const QString &msg)
{
    _ui->logBrowser->append(QString("<font color='darkgreen'>%1</font>").arg(msg));
}

void MainWindow::error(const QString &msg)
{
    _ui->logBrowser->append(QString("<font color='darkred'>%1</font>").arg(msg));
}

void MainWindow::saveParams()
{
    _app->set7zCmd(_ui->zipLE->text());
    _app->setUnaceCmd(_ui->aceLE->text());
    _app->setArjCmd(_ui->arjLE->text());
    if (!_ui->rarLE->text().isEmpty())
        _app->setUnrarCmd(_ui->rarLE->text());

    _app->setDstFolder(_ui->dstLE->text());
    _app->setOptions(_ui->testOnlyCB->isChecked(), _ui->delSrcCB->isChecked(),
                     _ui->debugCB->isChecked(), _ui->dispCompressionCB->isChecked());
}

void MainWindow::onLaunch()
{
    if (_state == STATE::IDLE)
    {
        if (!_updateParams())
            return;

        QStringList folders;
        for (int i = 0 ; i < _ui->srcList->count() ; ++i)
            folders << _ui->srcList->item(i)->text();

        if (folders.isEmpty())
            QMessageBox::warning(nullptr,
                                 tr("No source folder..."),
                                 tr("Please select some source folder(s)!"));
        else
        {
            _state = STATE::RUNNING;
            _ui->launchButton->setText(tr("Stop"));
            _app->processFolders(folders);
        }
    }
    else
        _app->stopProcessing();
}

void MainWindow::onTestOnly(bool checked)
{
    if (checked)
        _ui->delSrcCB->setChecked(false);
}

void MainWindow::onDelSrc(bool checked)
{
    if (checked)
        _ui->testOnlyCB->setChecked(false);
}

void MainWindow::onDebugToggled(bool checked)
{
    _app->setDebug(checked);
}

void MainWindow::on7zipPath()
{
    QString file = QFileDialog::getOpenFileName(
                this,
                tr("Select 7z"),
                _ui->zipLE->text()
            #if defined(WIN32) || defined(__MINGW64__)
                , "*.exe"
            #endif
                );

    if (!file.isEmpty())
        _ui->zipLE->setText(file);
}

void MainWindow::onUnrarPath()
{
    QString file = QFileDialog::getOpenFileName(
                this,
                tr("Select Urar"),
                _ui->rarLE->text()
            #if defined(WIN32) || defined(__MINGW64__)
                , "*.exe"
            #endif
                );

    if (!file.isEmpty())
        _ui->rarLE->setText(file);
}

void MainWindow::onUnacePath()
{
    QString file = QFileDialog::getOpenFileName(
                this,
                tr("Select Unace"),
                _ui->aceLE->text()
            #if defined(WIN32) || defined(__MINGW64__)
                , "*.exe"
            #endif
                );

    if (!file.isEmpty())
        _ui->aceLE->setText(file);
}

void MainWindow::onArjPath()
{
    QString file = QFileDialog::getOpenFileName(
                this,
                tr("Select arj"),
                _ui->arjLE->text()
            #if defined(WIN32) || defined(__MINGW64__)
                , "*.exe"
            #endif
                );

    if (!file.isEmpty())
        _ui->arjLE->setText(file);
}

void MainWindow::onDstPath()
{
    QString folder = QFileDialog::getExistingDirectory(
                this,
                tr("Select a Folder"),
                _ui->dstLE->text(),
                QFileDialog::ShowDirsOnly);

    if (!folder.isEmpty())
        _ui->dstLE->setText(folder);
}

void MainWindow::onDispCompressionPaths(bool display)
{
    _ui->compressionBox->setVisible(display);
}

void MainWindow::onAddSrc()
{
    QString folder = QFileDialog::getExistingDirectory(
                this,
                tr("Select a Folder"),
                "",
                QFileDialog::ShowDirsOnly);

    if (!folder.isEmpty())
        _ui->srcList->addPathIfNotInList(folder, _ui->srcList->count(), true);
}


void MainWindow::dragEnterEvent(QDragEnterEvent *e)
{
    if (e->mimeData()->hasUrls())
        e->acceptProposedAction();
}

void MainWindow::dropEvent(QDropEvent *e)
{
    int currentNbFiles = _ui->srcList->count();
    for (const QUrl &url : e->mimeData()->urls())
    {
        QString fileName = url.toLocalFile();
        if (QFileInfo(fileName).isDir()) // we only add folders
            _ui->srcList->addPathIfNotInList(fileName, currentNbFiles, true);
    }
}


void MainWindow::closeEvent(QCloseEvent *event)
{
    if (_state == STATE::RUNNING)
    {
        int res = QMessageBox::question(this,
                                        tr("Close while still unpacking?"),
                                        tr("%1 is currently unpacking.\nAre you sure you want to quit?").arg(_app->appName()),
                                        QMessageBox::Yes,
                                        QMessageBox::No);
        if (res == QMessageBox::Yes)
        {
            _app->stopProcessing();
            qApp->processEvents();
            event->accept();
        }
        else
            event->ignore();
    }
    else
        event->accept();
}


bool MainWindow::_updateParams()
{
    if (!_app->set7zCmd(_ui->zipLE->text()))
    {
        QMessageBox::warning(nullptr,
                             tr("7z path not valid..."),
                             tr("Please set 7z path to the right executable"));
        return false;
    }
    if (!_app->setDstFolder(_ui->dstLE->text()))
    {
        QMessageBox::warning(nullptr,
                             tr("dest folder not valid..."),
                             tr("Please set the destination folder to a writable folder"));
        return false;
    }

    if (!_app->setUnaceCmd(_ui->aceLE->text()))
    {
        QMessageBox::warning(nullptr,
                             tr("unace path not valid..."),
                             tr("Please set unace path to the right executable"));
        return false;
    }
    if (!_app->setArjCmd(_ui->arjLE->text()))
    {
        QMessageBox::warning(nullptr,
                             tr("arj path not valid..."),
                             tr("Please set arj path to the right executable"));
        return false;
    }

    if (!_ui->rarLE->text().isEmpty() && !_app->setUnrarCmd(_ui->rarLE->text()))
    {
        QMessageBox::warning(nullptr,
                             tr("unrar path not valid..."),
                             tr("Please set unrar path to the right executable"));
        return false;
    }

    _app->setOptions(_ui->testOnlyCB->isChecked(), _ui->delSrcCB->isChecked(),
                     _ui->debugCB->isChecked(), _ui->dispCompressionCB->isChecked());

    return true;
}


const QString MainWindow::sGroupBoxStyle =  "\
QGroupBox {\
        font: bold; \
        border: 1px solid silver;\
        border-radius: 6px;\
        margin-top: 6px;\
}\
QGroupBox::title {\
        subcontrol-origin:  margin;\
        left: 7px;\
        padding: 0 5px 0 5px;\
}";


const QString MainWindow::sSplitterStyle =  "\
QSplitter::handle:horizontal,\
QSplitter::handle:vertical{\
  background-color: rgb(85, 170, 255);\
    border: 1px solid #101010;\
    border-radius: 1px;\
    border-style: dotted;\
}";
