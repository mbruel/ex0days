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

#include "About.h"
#include "ui_About.h"
#include "Ex0days.h"

About::About(Ex0days *app, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::About)
{
    ui->setupUi(this);
#if QT_VERSION >= QT_VERSION_CHECK(5, 9, 0)
    setWindowFlag(Qt::FramelessWindowHint);
#endif
    setStyleSheet("QDialog {border:2px solid black}");

    ui->titleLbl->setText(QString("<pre>%1</pre>").arg(app->escapeXML(app->asciiArtWithVersion())));

    ui->copyrightLbl->setText("Copyright © 2020 - Matthieu Bruel");
    ui->copyrightLbl->setStyleSheet("QLabel { color : darkgray; }");
    ui->copyrightLbl->setFont(QFont( "Arial", 12, QFont::Bold));

    ui->descLbl->setTextFormat(Qt::RichText);
    ui->descLbl->setText(app->desc(true));
    ui->descLbl->setOpenExternalLinks(true);
    ui->descLbl->setStyleSheet(QString("QLabel { color : %1; }").arg(sTextColor));
    ui->descLbl->setFont(QFont( "Caladea", 14, QFont::Medium));

    connect(ui->donateButton, &QAbstractButton::clicked, app, &Ex0days::onDonate);
    connect(ui->closeButton, &QAbstractButton::clicked, this, &QWidget::close);
}

About::~About()
{
    delete ui;
}

void About::keyPressEvent(QKeyEvent *e)
{
    Q_UNUSED(e)
    close();
}

#include <QMouseEvent>
void About::mousePressEvent(QMouseEvent *e)
{
    Q_UNUSED(e)
    if (e->button() == Qt::RightButton)
        close();
}
