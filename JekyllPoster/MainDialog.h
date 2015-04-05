#ifndef MAINDIALOG_H
#define MAINDIALOG_H

#include <QDialog>

namespace Ui {
class Dialog;
}

class Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog(QWidget *parent = 0);
    ~Dialog();

private slots:
    void on_btnCancel_clicked();

    void on_btnSave_clicked();

    void on_btnSiteFolder_clicked();

    void on_btnOpenSite_clicked();

private:
    void init();
    void init_ui();
    void init_config();
    bool gen_poster();

    bool verify_path(const QString& path);
    bool load_config(const QString& path=QString());
    bool save_config(const QString& path=QString());

private:
    Ui::Dialog *ui;

    bool mDirtySitePath;
    QString mSitePath;
};

#endif // MAINDIALOG_H
