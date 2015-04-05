#include "QApplication"
#include "QString"
#include "QStringList"
#include "QFile"
#include "QDir"
#include "QFileInfo"
#include "QTextStream"
#include "QMessageBox"
#include "QProcess"
#include "QDate"
#include "QDateTime"
#include "QFileDialog"
#include "QProcessEnvironment"

#include "MainDialog.h"
#include "ui_MainDialog.h"

static QString kConfigName = "config";
static QString kSitePathEntry = "SitePath";

/*
 * the following function is originally from qt creator source
 */
void OpenInExplorer(const QString& pathIn)
{
    if(pathIn == "") return;

#if defined(Q_OS_WIN)
    const QString explorer = Environment::systemEnvironment().searchInPath(QLatin1String("explorer.exe"));
    if (explorer.isEmpty()) {
        QMessageBox::warning(parent,
                             tr("Launching Windows Explorer failed"),
                             tr("Could not find explorer.exe in path to launch Windows Explorer."));
        return;
    }
    QString param;
    if (!QFileInfo(pathIn).isDir())
        param = QLatin1String("/select,");
    param += QDir::toNativeSeparators(pathIn);
    QString command = explorer + " " + param;
    QProcess::startDetached(command);
#elif defined(Q_OS_MAC)
    QStringList scriptArgs;
    scriptArgs << QLatin1String("-e")
               << QString::fromLatin1("tell application \"Finder\" to reveal POSIX file \"%1\"")
                                     .arg(pathIn);
    QProcess::execute(QLatin1String("/usr/bin/osascript"), scriptArgs);
    scriptArgs.clear();
    scriptArgs << QLatin1String("-e")
               << QLatin1String("tell application \"Finder\" to activate");
    QProcess::execute("/usr/bin/osascript", scriptArgs);
#else
    // we cannot select a file here, because no file browser really supports it...
    const QFileInfo fileInfo(pathIn);
    const QString folder = fileInfo.absoluteFilePath();
    const QString app = Utils::UnixUtils::fileBrowser(Core::ICore::instance()->settings());
    QProcess browserProc;
    const QString browserArgs = Utils::UnixUtils::substituteFileBrowserParameters(app, folder);
    if (debug)
        qDebug() <<  browserArgs;
    bool success = browserProc.startDetached(browserArgs);
    const QString error = QString::fromLocal8Bit(browserProc.readAllStandardError());
    success = success && error.isEmpty();
    if (!success)
        showGraphicalShellError(parent, app, error);
#endif
}

Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog),
    mDirtySitePath(false)
{
    ui->setupUi(this);
    init();
}

Dialog::~Dialog()
{
    delete ui;
}

void Dialog::on_btnCancel_clicked()
{
    this->close();
}

void Dialog::on_btnSave_clicked()
{
    if(gen_poster())
        this->close();
}

void Dialog::on_btnSiteFolder_clicked()
{
    QString siteFolder = QFileDialog::getExistingDirectory(0, "Choose the site folder");
    if(siteFolder.isEmpty()) return;

/*
 * let the python script to create the _post dir
 *
    QDir dir(siteFolder);
    QString postDir = "_post";
    if(!dir.cd(postDir) and !dir.mkdir(postDir))
    {
        QMessageBox::warning(0, "Error", QString("Invalid site folder: failed to create subfolder %1").arg(postDir));
        return;
    }
*/

    if( mSitePath != siteFolder )
    {
        mSitePath = siteFolder;
        ui->edtSite->setText(siteFolder);
        mDirtySitePath = true;
    }
}

void Dialog::init()
{
    init_config();
    init_ui();
}

void Dialog::init_ui()
{
    //QDate today = QDate::currentDate();
    //ui->edtDate->setText(today.toString("yyyy-MM-dd"));

    //setWindowIcon(QIcon(":/AppIcon.icns"));

    QDateTime rightNow = QDateTime::currentDateTime();
    ui->edtDate->setText(rightNow.toString("yyyy-MM-dd HH:mm:ss"));

    ui->edtSite->setText(mSitePath);
}


void Dialog::init_config()
{
    load_config();

    if(mSitePath.isEmpty())
    {
        on_btnSiteFolder_clicked();
        if(!mSitePath.isEmpty())
            save_config();
    }
}

bool Dialog::gen_poster()
{
    // verify post params
    QString title = ui->edtTitle->text().trimmed();
    QString date = ui->edtDate->text().trimmed();
    QString site = ui->edtSite->text().trimmed();

    if(title.isEmpty())
    {
        QMessageBox::warning(NULL, "Error", "title for the post is required!");
        return false;
    }

    if(site.isEmpty())
    {
        QMessageBox::warning(NULL, "Error", "the local site path must be assigned!");
        return false;
    }

    // prepare command
    QDir dir(qApp->applicationDirPath());
    QString cmdFile = "gen_poster.py";
    QString cmdFullPath = dir.absoluteFilePath(cmdFile);
    if(!QFileInfo::exists(cmdFullPath))
    {
        QMessageBox::critical(NULL, "Error!", QString("Cannot find the command script: ") + cmdFullPath );
        return false;
    }

    // execute
    QProcess proc;
    QStringList args(cmdFullPath);
    args.append("-t");
    args.append(title);
    args.append("-d");
    args.append(date);
    args.append("-s");
    args.append(site);

    int code = proc.execute("python", args);

    if(code != 0) return false;

    // finalize: save configs
    if(mDirtySitePath || mSitePath != site)
    {
        mSitePath = site;
        save_config();
    }

    return true;
}

bool Dialog::verify_path(const QString &path)
{
    if(path.isEmpty()) return false;

    return QDir(path).exists();
}

bool Dialog::load_config(const QString& path)
{
    QFile configFile;
    if(path.isEmpty())
    {
        QDir dir(qApp->applicationDirPath());
        QString configPath = dir.absoluteFilePath(kConfigName);
        configFile.setFileName(configPath);
    }
    else
    {
        configFile.setFileName(path);
    }

    if(!configFile.exists())
    {
        if(!configFile.open(QFile::WriteOnly)) throw;

        configFile.close(); // simply create an empty config file
        return true;
    }

    if(!configFile.open(QFile::ReadOnly)) throw;

    QTextStream stream(&configFile);
    QString line;
    do
    {
        line = stream.readLine().trimmed();
        QStringList parts = line.split(':');
        if(parts.length() == 2 && parts.at(0) == kSitePathEntry)
        {
            const QString& entry = parts.at(1).trimmed();
            if(entry.isNull()) break;

            if(!QDir(entry).exists())
            {
                QMessageBox::warning(0, "Warning", QString("Invalid site path in the config file: %1").arg(entry) );
            }
            else
            {
                mSitePath = entry;
                break;
            }
        }
    }while(!line.isNull());
    configFile.close();

    return true;
}

bool Dialog::save_config(const QString& path)
{
    // !! make sure the sitepath is valid before saving the config

    QFile configFile;
    if(path.isEmpty())
    {
        QDir dir(qApp->applicationDirPath());
        QString configPath = dir.absoluteFilePath(kConfigName);
        configFile.setFileName(configPath);
    }
    else
    {
        configFile.setFileName(path);
    }

    if(!configFile.open(QFile::WriteOnly)) throw;

    QTextStream stream(&configFile);

    stream << kSitePathEntry << ":" << mSitePath;
    configFile.close();

    mDirtySitePath = false;
    return true;
}


void Dialog::on_btnOpenSite_clicked()
{
    QString site = ui->edtSite->text();
    if(site == "") return;

    QDir dir(site);
    if(dir.exists()) OpenInExplorer(site);
}
