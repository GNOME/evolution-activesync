/* We don't use CONFIGWIZARD_H for the include guard, because
   in the autotools build, the ui.h file uses that. */
#ifndef __ACTIVESYNC_CONFIGWIZARD_H__
#define __ACTIVESYNC_CONFIGWIZARD_H__

#include <QDialog>
#include <QString>


namespace Ui {
    class ConfigWizard;
}


// Forward declarations
//class QLineEdit;
//class QAbstractButton;


class ConfigWizard : public QDialog
{
    Q_OBJECT

public:
    explicit ConfigWizard(QWidget* parent = 0);
    ~ConfigWizard();

public Q_SLOTS:
    void onNext();
    void onBack();
    void onCancel();

    void validateAutoDiscoverInputs();
    void validateManualServerInputs();

    void storeServerDetails(const QString& uri);
    void onAutoDiscoverFailure();
    void onProvisionSuccess();
    void onProvisionFailure();

    void error(const QString& msg);

private:
    enum State
    {
        AutoDiscoverDetails,
        TryingAutoDiscover,
        ManualServerDetails,
        GettingProvisionReqts,
        ConfirmProvisionReqts,
        Finish,
        Error
    };

    void getProvisionReqts();
    void changeState(State currentState);
    void setTitle(const QString title, const QString& subTitle);
    void setButtonCaptions(const QString& nextButtonCaption = "", const QString& backButtonCaption = "");

private:
    Ui::ConfigWizard* ui;
    State currentState;
    bool serverDetailsEnteredManually;
    QString serverUri;
    QString emailAddress;
    QString username;
};


#endif // __ACTIVESYNC_CONFIGWIZARD_H__
