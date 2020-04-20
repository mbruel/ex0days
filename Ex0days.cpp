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

#include "Ex0days.h"
#include "MainWindow.h"
#include "About.h"
#include <QApplication>
#include <QCommandLineParser>
#include <QRegularExpression>
#include <QDir>
#include <QTime>
#include <cmath>
#include <QSettings>
#include <QDebug>
#include <QDesktopServices>
#include <QUrl>

const QString Ex0days::sDonationURL = "https://www.paypal.com/cgi-bin/webscr?cmd=_donations&business=W2C236U6JNTUA&item_name=ex0days&currency_code=EUR";

const QMap<Ex0days::Opt, QString> Ex0days::sOptionNames =
{
    {Opt::HELP,    "help"},
    {Opt::VERSION, "version"},
    {Opt::DEBUG,   "debug"},
    {Opt::INPUT,   "input"},
    {Opt::OUTPUT,  "output"},
    {Opt::TEST,    "test"},
    {Opt::DEL,     "del"},
    {Opt::Z7,      "7z"},
    {Opt::UNRAR,   "unrar"},
    {Opt::UNACE,   "unace"}
};

const QList<QCommandLineOption> Ex0days::sCmdOptions = {
    {{"h", sOptionNames[Opt::HELP]},      tr("Help: display syntax")},
    {{"v", sOptionNames[Opt::VERSION]},   tr("app version")},
    {sOptionNames[Opt::DEBUG],            tr("display debug informations")},
    {{"i", sOptionNames[Opt::INPUT]},     tr("input parent folder (containing 0days)"), sOptionNames[Opt::INPUT]},
    {{"o", sOptionNames[Opt::OUTPUT]},    tr("output folder (or temporary)"), sOptionNames[Opt::OUTPUT]},
    {{"t", sOptionNames[Opt::TEST]},      tr("test only")},
    {{"d", sOptionNames[Opt::DEL]},       tr("delete sources once extracted")},
    {sOptionNames[Opt::Z7],               tr("7z full path"), sOptionNames[Opt::Z7]},
    {sOptionNames[Opt::UNRAR],            tr("unrar full path"), sOptionNames[Opt::UNRAR]},
    {sOptionNames[Opt::UNACE],            tr("unace full path"), sOptionNames[Opt::UNACE]}
};

const QMap<Ex0days::Param, QString> Ex0days::sParamValues = {
    {Param::cmd7z,    "cmd7z"},
    {Param::cmdRar,   "cmdRar"},
    {Param::cmdAce,   "cmdAce"},
    {Param::dstDir,   "dstDir"},
    {Param::testOnly, "testOnly"},
    {Param::delSrc,   "delSrc"},
    {Param::debug,    "debug"}
};

const QStringList Ex0days::s7zArgs = {"x", "-y"};

const QRegularExpression Ex0days::sRegExpArchiveExtensions = QRegularExpression("^.*\\.(rar|ace|arj)$");
const QRegularExpression Ex0days::sRegExpArchiveFiles      = QRegularExpression("^.*\\.(part)?(([rac])?\\d+)(\\.rar)?$");
const QRegularExpression Ex0days::sRegExpNumberOne         = QRegularExpression("^0*1$");


Ex0days::Ex0days(int &argc, char *argv[]):
    QObject(), CmdOrGuiApp (argc, argv),
    _state(STATE::IDLE),
#if defined(WIN32) || defined(__MINGW64__)
    _7zCmd("./7z.exe"), _unrarCmd("./unrar.exe"), _unaceCmd("./unace.exe"),
#else
    _7zCmd("/usr/bin/7z"), _unrarCmd("/usr/bin/unrar"), _unaceCmd("/usr/bin/unace"),
#endif
    _dstDir(nullptr),
    _cout(stdout), _cerr(stderr),
    _extProc(),
    _foldersToExtract(),
    _srcDir(nullptr),
    _zipFiles(), _currentZip(),
    _fistArchive(),
    _archiveType(ARCHIVE_TYPE::UNKNOWN),
    _timeStart(),
    _settings(nullptr),
    _stopProcess(false),
    _testOnly(false), _delSrc(false),
    _debug(false),
    _logFile(nullptr), _logStream()
{
#if defined(WIN32) || defined(__MINGW64__)
    _settings = new QSettings(QString("%1.ini").arg(appName()), QSettings::Format::IniFormat);
#else
    _settings = new QSettings(QSettings::NativeFormat, QSettings::UserScope, sAppName, sVersion);
#endif

    if (_hmi)
    {
        _hmi->setWindowTitle(QString("%1 v%2 - %3").arg(sAppName).arg(sVersion).arg(sDesc));
        _hmi->setWindowIcon(QIcon(":/icons/ex0days.png"));
        _hmi->setAsciiSignature(sASCII);
    }

    // queued to let hand to the HMI and avoid stack overflow ;)
    connect(this, &Ex0days::processNextFolder, this, &Ex0days::onProcessNextFolder, Qt::QueuedConnection);
    connect(this, &Ex0days::unzipNext,         this, &Ex0days::onUnzipNextFile,     Qt::QueuedConnection);


    connect(&_extProc, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
            this, &Ex0days::onProcFinished);

    _loadSettings();
}

Ex0days::~Ex0days()
{
    if (_extProc.state()!= QProcess::NotRunning)
    {
        _extProc.terminate();
        _extProc.waitForFinished();
    }

    _clearDir();
    _clearLogFile();

    if (_hmi)
        _hmi->saveParams();
    _settings->sync();
    delete _settings;

    if (_dstDir)
        delete _dstDir;
}


bool Ex0days::parseCommandLine(int argc, char *argv[])
{
    QString appVersion = QString("%1_v%2").arg(sAppName).arg(sVersion);
    QCommandLineParser parser;
    parser.setApplicationDescription(appVersion);
    parser.addOptions(sCmdOptions);


    // Process the actual command line arguments given by the user
    QStringList args;
    for (int i = 0; i < argc; ++i)
        args << argv[i];

    bool res = parser.parse(args);
#ifdef __DEBUG__
    qDebug() << "args: " << args
             << "=> parsing: " << res << " (error: " << parser.errorText() << ")";
#endif


    if (!parser.parse(args))
    {
        _error(tr("Error syntax: %1\nTo list the available options use: %2 --help\n").arg(parser.errorText()).arg(argv[0]));
        return false;
    }

    if (parser.isSet(sOptionNames[Opt::HELP]))
    {
        _showVersionASCII();
        _syntax(argv[0]);
        return false;
    }

    if (parser.isSet(sOptionNames[Opt::VERSION]))
    {
        _showVersionASCII();
        return false;
    }

    _loadSettings();

    if (parser.isSet(sOptionNames[Opt::DEBUG]))
    {
        _log(tr("Debug mode"));
        _debug = true;
    }

    if (parser.isSet(sOptionNames[Opt::TEST]))
    {
        _log(tr("Testing mode!"));
        _testOnly = true;
    }

    if (parser.isSet(sOptionNames[Opt::DEL]))
    {
        if (_testOnly)
        {
            _error(tr("You shouldn't delete source folder in Test mode..."));
            return false;
        }
        else
        {
            _log(tr("Deleting source once extracted"));
            _delSrc = true;
        }
    }

    if (!parser.isSet(sOptionNames[Opt::INPUT]) && !parser.isSet(sOptionNames[Opt::OUTPUT]) )
    {
        _error(tr("Error syntax: you should provide at least one input folder and the output directory"));
        return false;
    }

    if (parser.isSet(sOptionNames[Opt::Z7]) && !set7zCmd(parser.value(sOptionNames[Opt::Z7])))
    {
        _error(tr("Please provide a valid path for 7z..."));
        return false;
    }
    if (parser.isSet(sOptionNames[Opt::UNRAR]) && !setUnrarCmd(parser.value(sOptionNames[Opt::UNRAR])))
    {
        _error(tr("Please provide a valid path for unrar..."));
        return false;
    }
    if (parser.isSet(sOptionNames[Opt::UNACE]) && !setUnaceCmd(parser.value(sOptionNames[Opt::UNACE])))
    {
        _error(tr("Please provide a valid path for unace..."));
        return false;
    }

    if (!setDstFolder(parser.value(sOptionNames[Opt::OUTPUT])))
    {
        _error(tr("Please provide a writable directory for the output"));
        return false;
    }

    QStringList srcFolders;
    for (const QString &path : parser.values(sOptionNames[Opt::INPUT]))
    {
        QFileInfo fi(path);
        if (fi.exists() && fi.isDir() && fi.isReadable())
            srcFolders << path;
        else
        {
            _error(tr(""));
            return false;
        }
    }

    processFolders(srcFolders);

    return true;
}

int Ex0days::startHMI()
{
    _hmi->init(this);
    _hmi->show();
    return _app->exec();
}

void Ex0days::processFolders(const QStringList &srcFolders)
{
    _stopProcess = false;
    _logFile = new QFile(QString("./%1/%2_%3.csv").arg(
                             sLogFolder).arg(
                             appName()).arg(
                             QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss")));
    if (_logFile->open(QIODevice::WriteOnly|QIODevice::Text))
        _logStream.setDevice(_logFile);
    else
    {
        _error("Issue creating log file...");
        return;
    }

    _timeStart.start();

    for (const QString &srcFolder : srcFolders)
    {
        QFileInfo fi(srcFolder);
        _browseDir(srcFolder, {fi.path(), fi.fileName()});
    }

#ifdef __DEBUG__
    for (const auto & folder : _foldersToExtract)
        qDebug() << "0day folder: " << folder.join("/");
#endif

    _log(tr("<b>There are %1 0days folders to process</b>").arg(_foldersToExtract.size()));
    if (_hmi)
    {
        _folderIdx = 0;
        _hmi->setProgressMax(_foldersToExtract.size());
    }

    emit processNextFolder();
}

void Ex0days::stopProcessing()
{
    _stopProcess = true;
    if (_extProc.state()!= QProcess::NotRunning)
        _extProc.terminate();
    if (_hmi)
    {
        _error(tr("Job stopped with %1 0days extracted").arg(_folderIdx-1));
        _hmi->setIDLE();
    }
}

void Ex0days::_browseDir(const QString &folderPath, const QStringList &parents)
{
    QDir dir(folderPath);
    QFileInfoList subFolders = dir.entryInfoList(QDir::AllDirs|QDir::Hidden|QDir::NoDotAndDotDot|QDir::NoSymLinks,  QDir::Name);
    if (subFolders.isEmpty())
    {
//        qDebug() << "0day folder: " << dir.absolutePath();
        _foldersToExtract << parents;
    }
    else
    {
        for (const QFileInfo &subFolder : subFolders)
        {
            QStringList newParents(parents);
            newParents << subFolder.fileName();
            _browseDir(subFolder.absoluteFilePath(), newParents);
        }
    }
}

void Ex0days::_log(const QString &msg, bool success)
{
    _cout << msg << endl << flush;
    if (_hmi)
    {
        if (success)
            _hmi->success(msg);
        else
            _hmi->log(msg);
    }
}


void Ex0days::_error(const QString &msg)
{
    _cerr << msg << endl << flush;
    if (_hmi)
        _hmi->error(msg);
}

void Ex0days::_failExtract(const QString &reason)
{
    _logStream << _srcDir->absolutePath() << ", " << reason << "\n" << flush;
    _error(tr("%1 KO (%2)").arg(_srcDir->absolutePath()).arg(reason));
}

void Ex0days::_clearDir()
{
    _state = STATE::IDLE;
    if (_srcDir)
    {
        delete _srcDir;
        _srcDir = nullptr;
    }
    _zipFiles.clear();
    _currentPath.clear();
    _currentZip = QFileInfo();
}

void Ex0days::_clearLogFile()
{
    if (_logFile)
    {
        _logStream.setDevice(nullptr);
        _logFile->close();
        delete _logFile;
        _logFile = nullptr;
    }
}

void Ex0days::onProcessNextFolder()
{
    if (_hmi)
        _hmi->setProgress(_folderIdx++);

    _clearDir();
    if (_stopProcess || _foldersToExtract.isEmpty())
    {
        _clearLogFile();
        _logTimeElapsed();
        if (_hmi)
        {
            _hmi->setProgress(_folderIdx);
            _hmi->setIDLE();
        }
        else
            qApp->quit();
    }
    else
    {
        _currentPath = _foldersToExtract.dequeue();
        _srcDir  = new QDir(_currentPath.join("/"));
        QFileInfoList files = _srcDir->entryInfoList(QDir::Files|QDir::Hidden|QDir::Readable|QDir::NoSymLinks, QDir::Name);
        if (files.isEmpty())
        {
            _failExtract(tr("empty folder"));
        }
        else
        {
            qDebug() << tr("%1 ===>").arg(_srcDir->absolutePath());
            QFileInfoList zips;
            for (const QFileInfo &fi : files)
            {
                if  (fi.suffix().toLower() == "zip")
                    zips << fi;
            }

            if (zips.isEmpty())
            {
                _failExtract(tr("no zip files"));
                _goToNextFolder();
            }
            else
            {
                // Copy all zip in dest folder
                if (_debug)
                    _log(tr("Processing %1 (%2 zips)").arg(_srcDir->absolutePath()).arg(zips.size()));
                QString subPath = _subPath();
                if (!_dstDir->mkpath(subPath))
                    qCritical() << "Error creating folder: " << _dstDir->absolutePath() << "/" << subPath;
                for (const QFileInfo &fi : zips)
                {
                    QFileInfo copy(QString("%1/%2/%3").arg(
                                       _dstDir->absolutePath()).arg(
                                       subPath).arg(
                                       fi.fileName()));
                    if (QFile::copy(fi.absoluteFilePath(), copy.absoluteFilePath()))
                        _zipFiles << copy;
                    else
                        qCritical() << "Error copying file: " << fi.absoluteFilePath()
                                    << " to " << copy.absoluteFilePath();
                }

                _state = STATE::UNZIP;
                _extProc.setWorkingDirectory(QString("%1/%2").arg(_dstDir->absolutePath()).arg(subPath));
                onUnzipNextFile();
            }
        }
    }
}

void Ex0days::onUnzipNextFile()
{
    if (_zipFiles.isEmpty())
        _doSecondExtract();
    else
    {
        _currentZip = _zipFiles.dequeue();

        QStringList args = s7zArgs;
        args << _currentZip.fileName(); // the process is in the good directory!

        qDebug() << _7zCmd << " "  << args.join(" ");
        _extProc.start(_7zCmd, args);
    }
}

void Ex0days::onAbout()
{
    About about(this);
    about.exec();
}


void Ex0days::onDonate()
{
    QDesktopServices::openUrl(sDonationURL);
}

void Ex0days::onProcFinished(int exitCode)
{
    if (_stopProcess)
    {
        --_folderIdx;
        onProcessNextFolder();
    }
    else if (_state == STATE::UNZIP)
    {
        qDebug() << "Zip extracted: "<< _currentZip.absoluteFilePath() << " : " << exitCode;
        if (exitCode != 0)
        {
            _failExtract(tr("error #%1 on zip file: %2").arg(exitCode).arg(_currentZip.fileName()));
            _goToNextFolder();
        }
        else
        {
            QFile file(_currentZip.absoluteFilePath());
            if (!file.remove())
            {
                qCritical() << "Error deleting " << _currentZip.absoluteFilePath() << ": " << file.errorString();
            }
            emit unzipNext();
        }
    }
    else if (_state == STATE::FINAL)
    {
        if (exitCode != 0)
        {
            _failExtract(tr("error #%1 extracting archive: %2").arg(exitCode).arg(_fistArchive.fileName()));
        }
        else
            _log(tr("%1 OK").arg(_srcDir->absolutePath()), true);

        _goToNextFolder();
    }
}

void Ex0days::_goToNextFolder()
{
    if (_testOnly)
    {
        QDir copyDir(QString("%1/%2").arg(_dstDir->absolutePath()).arg(_subPath()));
        if (!copyDir.removeRecursively())
            _error(tr("Error deleting copy directory: %1").arg(copyDir.absolutePath()));
        else if (_debug)
            _log(tr("Copy directory deleted: %1").arg(copyDir.absolutePath()));
    }
    else if (_delSrc)
    {
        if (!_srcDir->removeRecursively())
            _error(tr("Error deleting source directory: %1").arg(_srcDir->absolutePath()));
        else if (_debug)
            _log(tr("Source directory deleted: %1").arg(_srcDir->absolutePath()));
    }

    emit processNextFolder();
}

void Ex0days::_syntax(char *appName)
{
    QString app = QFileInfo(appName).fileName();
    _cout << desc() << "\n"
          << tr("Syntax: ") << app << " (options)* (-i <src_folder>)+ -o <output_folder>\n";
    for (const QCommandLineOption & opt : sCmdOptions)
    {
        if (opt.names().size() == 1)
            _cout << QString("\t--%1: %2\n").arg(opt.names().first(), -17).arg(tr(opt.description().toLocal8Bit().constData()));
        else
            _cout << QString("\t-%1: %2\n").arg(opt.names().join(" or --"), -18).arg(tr(opt.description().toLocal8Bit().constData()));
    }
    _cout << "\n" << flush;
}

void Ex0days::_doSecondExtract()
{
    _state = STATE::FINAL;
    qDebug() << tr("Ready for Second Extract!");

    QDir copyDir(QString("%1/%2").arg(_dstDir->absolutePath()).arg(_subPath()));
    QFileInfoList unzippedFiles = copyDir.entryInfoList(QDir::Files|QDir::Hidden|QDir::Readable|QDir::NoSymLinks, QDir::Name);
    bool isFirstArchive = false;
    for (const QFileInfo &file : unzippedFiles)
    {
        _findArchiveType(file, isFirstArchive);
        if (isFirstArchive)
        {
            _fistArchive = file;
            break;
        }
    }

    if (isFirstArchive)
    {
        if (_debug)
            _log(tr("  - first archive found: %1").arg(_fistArchive.fileName()));
        QStringList args = s7zArgs;
        args << _fistArchive.fileName(); // the process is in the good directory!

        const QString &cmd = _extractCMD();
        qDebug() << cmd << " "  << args.join(" ");
        _extProc.start(cmd, args);
    }
    else
    {
        _failExtract(tr("first archive is missing"));
        _goToNextFolder();
    }
}

void Ex0days::_logTimeElapsed()
{
    int duration = static_cast<int>(_timeStart.elapsed());
    double sec = (double)duration/1000;

    _log(tr("<br/><b> => %1 folders extracted in %2 sec (%3)</b>").arg(
             _folderIdx-1).arg(std::round(sec)).arg(QTime::fromMSecsSinceStartOfDay(duration).toString("hh:mm:ss.zzz")));
}



bool Ex0days::set7zCmd(const QString &path)
{
    QFileInfo fi(path);
    if (fi.exists() && fi.isFile() && fi.isExecutable())
    {
        _7zCmd = path;
        _settings->setValue(sParamValues[Param::cmd7z], _7zCmd);
        return true;
    }
    else
        return false;
}

bool Ex0days::setUnrarCmd(const QString &path)
{
    QFileInfo fi(path);
    if (fi.exists() && fi.isFile() && fi.isExecutable())
    {
        _unrarCmd = path;
        _settings->setValue(sParamValues[Param::cmdRar], _unrarCmd);
        return true;
    }
    else
        return false;
}

bool Ex0days::setUnaceCmd(const QString &path)
{
    QFileInfo fi(path);
    if (fi.exists() && fi.isFile() && fi.isExecutable())
    {
        _unaceCmd = path;
        _settings->setValue(sParamValues[Param::cmdAce], _unaceCmd);
        return true;
    }
    else
        return false;
}

bool Ex0days::setDstFolder(const QString &path)
{
    QFileInfo fi(path);
    if (fi.exists() && fi.isDir() && fi.isWritable())
    {
        if (_dstDir)
            delete _dstDir;
        _dstDir = new QDir(path);
        _settings->setValue(sParamValues[Param::dstDir], path);
        return true;
    }
    else
        return false;
}

void Ex0days::setOptions(bool testOnly, bool delSrc, bool debug)
{
    _testOnly  = testOnly;
    _delSrc    = delSrc;
    _debug     = debug;

    _settings->setValue(sParamValues[Param::testOnly], testOnly);
    _settings->setValue(sParamValues[Param::delSrc],   delSrc);
    _settings->setValue(sParamValues[Param::debug],    debug);
}

QString Ex0days::setting(Ex0days::Param param) const
{
    return _settings->value(sParamValues[param]).toString();
}


void Ex0days::_findArchiveType(const QFileInfo &file, bool &firstArchive)
{
    _archiveType = ARCHIVE_TYPE::UNKNOWN;
    QString fileNameLowerCase = file.fileName().toLower();

    QRegularExpressionMatch match = sRegExpArchiveExtensions.match(fileNameLowerCase);
    if (match.hasMatch())
    {
        firstArchive = true;
        QString ext = match.captured(1);
        if (ext == "rar")
            _archiveType = ARCHIVE_TYPE::RAR;
        else if (ext == "ace")
            _archiveType = ARCHIVE_TYPE::ACE;
        else
            _archiveType = ARCHIVE_TYPE::ARJ;
    }

    if (_archiveType == ARCHIVE_TYPE::UNKNOWN || _archiveType == ARCHIVE_TYPE::RAR)
    {
        match = sRegExpArchiveFiles.match(fileNameLowerCase);
        if (match.hasMatch())
        {
            QString newRar         = match.captured(1);
            QString number         = match.captured(2);
            QString oldStyleLetter = match.captured(3);
            QString lastExtenstion = match.captured(5);

            if (!newRar.isEmpty())
                _archiveType = ARCHIVE_TYPE::RAR;
            else if (!oldStyleLetter.isEmpty())
            {
                const QChar &letter = oldStyleLetter.at(0);
                if (letter == 'r')
                    _archiveType = ARCHIVE_TYPE::RAR;
                else if (letter == 'a')
                    _archiveType = ARCHIVE_TYPE::ARJ;
                else
                    _archiveType = ARCHIVE_TYPE::ACE;
            }
            else if (!number.isEmpty())
                _archiveType = ARCHIVE_TYPE::RAR;

            if (_archiveType == ARCHIVE_TYPE::RAR)
            {
                if (sRegExpNumberOne.match(number).hasMatch())
                    firstArchive = true;
                else
                    firstArchive = false;
            }

//            qDebug() << file << " matches with newRar: " << newRar
//                     << ", number: " << number << ", oldStyleLetter: " << oldStyleLetter
//                     << ", lastExt: " << lastExtenstion;
        }
    }

#ifdef __DEBUG__
    QString archiveType;
    switch (_archiveType) {
    case ARCHIVE_TYPE::RAR:
        archiveType = "RAR";
        break;
    case ARCHIVE_TYPE::ACE:
        archiveType = "ACE";
        break;
    case ARCHIVE_TYPE::ARJ:
        archiveType = "ARJ";
        break;
    default:
        archiveType = "UNKNOWN";
        break;
    }
    qDebug() << file << " is " << archiveType << " (first: " << firstArchive << ")";
#endif
}

void Ex0days::_loadSettings()
{
    if (!QFileInfo(sLogFolder).exists())
        QDir(".").mkdir(sLogFolder);

    if (_settings->value(sParamValues[Param::cmd7z]).isValid())
    {
        _7zCmd    = setting(Param::cmd7z);
        _unrarCmd = setting(Param::cmdRar);
        _unaceCmd = setting(Param::cmdAce);
    }
    else
    {
        set7zCmd(_7zCmd);
        setUnrarCmd(_unrarCmd);
        setUnaceCmd(_unaceCmd);
    }

    if (_hmi)
    {
        _testOnly = _settings->value(sParamValues[Param::testOnly]).toBool();
        _delSrc   = _settings->value(sParamValues[Param::delSrc]).toBool();
        _debug    = _settings->value(sParamValues[Param::debug]).toBool();
    }
}



const QString Ex0days::sASCII = "\
              _______       .___\n\
  ____ ___  __\\   _  \\    __| _/____  ___.__. ______\n\
_/ __ \\\\  \\/  /  /_\\  \\  / __ |\\__  \\<   |  |/  ___/\n\
\\  ___/ >    <\\  \\_/   \\/ /_/ | / __ \\\\___  |\\___ \\\n\
 \\___  >__/\\_ \\\\_____  /\\____ |(____  / ____/____  >\n\
     \\/      \\/      \\/      \\/     \\/\\/         \\/\
";




