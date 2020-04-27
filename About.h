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

#ifndef ABOUT_H
#define ABOUT_H

#include <QDialog>
class Ex0days;

namespace Ui {
class About;
}

class About : public QDialog
{
    Q_OBJECT
private:
    static constexpr const char *sTextColor = "#25aae1"; //!< same blue that the logo

public:
    explicit About(Ex0days *app, QWidget *parent = nullptr);
    ~About();

protected:
    void keyPressEvent(QKeyEvent *e) override;
    void mousePressEvent(QMouseEvent *e) override;

private:
    Ui::About *ui;
};

#endif // ABOUT_H
