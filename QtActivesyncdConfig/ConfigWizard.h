/* We don't use CONFIGWIZARD_H for the include guard, because
   in the autotools build, the ui.h file uses that. */
#ifndef __ACTIVESYNC_CONFIGWIZARD_H__
#define __ACTIVESYNC_CONFIGWIZARD_H__


// System includes
#include <QDialog>
#include <QString>
// User includes
#include "../libeasaccount/src/eas-account.h"
#include "../libeasaccount/src/eas-account-list.h"
#include "../libeasmail/src/libeasmail.h"


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

    void showError(const QString& msg);

    void useDebugDetails(bool useDebug);

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
    void acceptProvisionReqts();
    void changeState(State currentState);
    void setTitle(const QString title, const QString& subTitle);
    void setButtonCaptions(const QString& nextButtonCaption = "", const QString& backButtonCaption = "");
    void createEvolutionMailAccount();

private:
    Ui::ConfigWizard* ui;
    State currentState;
    bool serverDetailsEnteredManually;
    bool serverHasProvisioningReqts;
    QString fullName;
    QString emailAddress;
    QString serverUri;
    QString username;

    EasEmailHandler* mailHandler;

    // Tokens used when negotiating server provisioning requirements
    gchar* tid;
    gchar* tidStatus;
};


#endif // __ACTIVESYNC_CONFIGWIZARD_H__
