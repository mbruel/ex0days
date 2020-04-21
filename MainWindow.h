//========================================================================
//
// Copyright (C) 2020 Matthieu Bruel <Matthieu.Bruel@gmail.com>
//
// This file is a part of ex0days : https://github.com/mbruel/ex0days
//
// ngPost is free software; you can redistribute it and/or modify
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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
class Ex0days;
class QProgressBar;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT
private:
    enum class STATE :bool {IDLE = 0, RUNNING};

    Ui::MainWindow *_ui;
    QProgressBar   *_progressBar;
    Ex0days        *_app;
    STATE           _state;

    static const QString sGroupBoxStyle;
    static const QString sSplitterStyle;


public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

    void setAsciiSignature(const QString &ascii);

    void init(Ex0days *app);

    void setIDLE();
    void setProgressMax(int max);
    void setProgress(int value);


    void log(const QString &msg);
    void success(const QString &msg);
    void error(const QString &msg);

    void saveParams();

public slots:
    void onLaunch();
    void onTestOnly(bool checked);
    void onDelSrc(bool checked);
    void onAddSrc();
    void onDebugToggled(bool checked);
    void on7zipPath();
    void onUnrarPath();
    void onUnacePath();
    void onArjPath();
    void onDstPath();
    void onDispCompressionPaths(bool display);

protected:
    void dragEnterEvent(QDragEnterEvent *e) override;
    void dropEvent(QDropEvent *e) override;

    void closeEvent(QCloseEvent *event) override;

private:
    bool _updateParams();
};

#endif // MAINWINDOW_H
