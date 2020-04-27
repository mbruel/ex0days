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

#ifndef EX0DAYS_H
#define EX0DAYS_H
#include "CmdOrGuiApp.h"
#include <QCommandLineOption>
#include <QTextStream>
#include <QProcess>
#include <QQueue>
#include <QFileInfo>
#include <QElapsedTimer>
class QSettings;
class MainWindow;

class Ex0days : public QObject, public CmdOrGuiApp
{
    Q_OBJECT
public:
    enum class Param {cmd7z, cmdRar, cmdAce, cmdArj, dstDir,
                      testOnly, delSrc, debug, dispPaths};

private:
    enum class Opt {HELP = 0, VERSION, DEBUG,
                    INPUT, OUTPUT, TEST, DEL,
                    Z7, UNRAR, UNACE
                   };
    enum class STATE : char {IDLE = 0, UNZIP, FINAL};

    enum class ARCHIVE_TYPE {UNKNOWN = 0, RAR, ACE, ARJ, Z7};


    STATE               _state;

    QString             _7zCmd;
    QString             _unrarCmd;
    QString             _unaceCmd;
    QString             _arjCmd;

    QDir               *_dstDir;

    QTextStream         _cout; //!< stream for stdout
    QTextStream         _cerr; //!< stream for stderr
    QProcess            _extProc;
    QQueue<QStringList> _foldersToExtract;
    int                 _folderIdx;
    QDir               *_srcDir;
    QStringList         _currentPath;
    QQueue<QFileInfo>   _zipFiles;
    QFileInfo           _currentZip;
    QFileInfo           _fistArchive;
    ARCHIVE_TYPE        _archiveType;
    QFileInfoList       _unzippedFiles;

    QElapsedTimer       _timeStart;

    QSettings          *_settings;
    bool                _stopProcess;

    bool                _testOnly;
    bool                _delSrc;

    bool                _debug;
    QFile              *_logFile;
    QTextStream         _logStream;

    bool                _useWinrar;



public:
    explicit Ex0days(int &argc, char *argv[]);
    ~Ex0days() override;

    // pure virtual methods
    inline const char * appName() override;
    bool parseCommandLine(int argc, char *argv[]) override;

    int startHMI() override;

    void processFolders(const QStringList &srcFolders);
    void stopProcessing();

    bool set7zCmd(const QString &path);
    bool setUnrarCmd(const QString &path);
    bool setUnaceCmd(const QString &path);
    bool setArjCmd(const QString &path);
    bool setDstFolder(const QString &path);

    void setOptions(bool testOnly, bool delSrc, bool debug, bool dispPaths);

    QString setting(Param param) const;

    inline bool testOnly() const;
    inline bool delSrc() const;
    inline bool debug() const;
    bool dispPaths() const;

    inline void setDebug(bool enable);



signals:
    void processNextFolder();
    void unzipNext();

public slots:
    void onProcFinished(int exitCode);
    void onProcessNextFolder();
    void onUnzipNextFile();

    void onAbout();
    void onDonate();



private:
    void _browseDir(const QString &folderPath, const QStringList &parents);
    void _log(const QString &msg, bool success = false);
    void _error(const QString &msg);
    void _failExtract(const QString &reason);
    void _clearDir();
    void _clearLogFile();
    void _doSecondExtract();

    inline QString _subPath() const;
    void _logTimeElapsed();

    void _findArchiveType(const QFileInfo &file, bool &firstArchive);
    inline const QString &_extractCMD() const;

    void _loadSettings();

    void _goToNextFolder(bool success, bool delUnzippedFiles = true);

    inline void _showVersionASCII();
    void _syntax(char *appName);


// statics
private:
    // statics

    static constexpr const char *sAppName   = "ex0days";
    static constexpr const char *sVersion   = "1.3";
    static constexpr const char *sDesc      = "extract double compressed archives";

    static constexpr const char *sLogFolder = "./logs";

    static const QMap<Opt, QString> sOptionNames;
    static const QList<QCommandLineOption> sCmdOptions;
    static const QStringList s7zArgs;

    static const QMap<Param, QString> sParamValues;

    static const QString sASCII;

    static const QRegularExpression sRegExpArchiveExtensions;
    static const QRegularExpression sRegExpArchiveFiles;
    static const QRegularExpression sRegExpNumberOne;

    static const QString sDonationURL;


public:
    inline static QString desc(bool useHTML = false);
    inline static QString asciiArtWithVersion();
    inline static const QString &donationURL();

};

const char *Ex0days::appName() { return sAppName; }

bool Ex0days::testOnly() const { return _testOnly; }
bool Ex0days::delSrc()   const { return _delSrc; }
bool Ex0days::debug()    const { return _debug; }

void Ex0days::setDebug(bool enable) { _debug = enable; }

const QString &Ex0days::donationURL() { return sDonationURL; }

QString Ex0days::_subPath() const
{
    QStringList subPath(_currentPath);
    subPath.removeFirst();
    return subPath.join("/");
}

const QString &Ex0days::_extractCMD() const
{    
    switch (_archiveType) {
    case ARCHIVE_TYPE::ACE:
        return _unaceCmd;
    case ARCHIVE_TYPE::RAR:
        return _unrarCmd.isEmpty() ? _7zCmd : _unrarCmd;
    case ARCHIVE_TYPE::ARJ:
        return _arjCmd;
    default:
        return _7zCmd;
    }
}

void Ex0days::_showVersionASCII()
{
    _cout << asciiArtWithVersion() << "\n\n" << flush;
}

QString Ex0days::desc(bool useHTML)
{
    QString desc;
    if (useHTML)
        desc = QString("%1<br/>%2<ul><li>%3</li><li>%4</li></ul>%5<br/><br/>");
    else
        desc = QString("%1\n%2\n  - %3\n  - %4\n%5\n");
    return desc.arg(
            tr("Command line / GUI tool to automate extraction (or test) of double compressed 0days")).arg(
            tr("It will browse recursively the folders you provide.")).arg(
            tr("a 0day folder must not have subfolders.")).arg(
            tr("it should contain zip files as first compression method")).arg(
            tr("for more details, cf %1").arg(
                    useHTML ? "<a href=\"https://github.com/mbruel/ex0days/\">https://github.com/mbruel/ex0days</a>"
                            : "https://github.com/mbruel/ex0days"));}

QString Ex0days::asciiArtWithVersion()
{
    return QString("%1\n                                         v%2").arg(sASCII).arg(sVersion);
}

#endif // EX0DAYS_H
