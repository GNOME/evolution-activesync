/* We don't use CONFIGWIZARD_H for the include guard, because
   in the autotools build, the ui.h file uses that. */
#ifndef __ACTIVESYNC_CONFIGWIZARD_H__
#define __ACTIVESYNC_CONFIGWIZARD_H__


// System includes
#include <QApplication>
#include <QDialog>
#include <QString>
// User includes
#include "../libeasaccount/src/eas-account.h"
#include "../libeasaccount/src/eas-account-list.h"
#include <libeasmail.h>


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

    bool attemptAutoDiscovery();
    void getProvisionReqts();
    void acceptProvisionReqts();
    void changeState(State currentState);
    void setTitle(const QString title, const QString& subTitle);
    void setButtonCaptions(const QString& nextButtonCaption = "", const QString& backButtonCaption = "");
    void createEvolutionMailAccount();
    void createSyncEvolutionAccounts();

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

private:
    /**
     * Simple class for showing a wait cursor that automatically dismisses when the function returns.
     * Can also be started/stopped manually and ensures the cursor stack is always emptied when stopped.
     * (See http://doc.qt.nokia.com/4.7/qapplication.html#setOverrideCursor)
     */
    class AutoWaitCursor
    {
    public:
        AutoWaitCursor() : count(0) { start(); }
        ~AutoWaitCursor() { stop(); }
        void start() { QApplication::setOverrideCursor(QCursor(Qt::WaitCursor)); count++; }
        void stop() { while (count--) QApplication::restoreOverrideCursor(); }
    private:
        int count;
    };
};


#endif // __ACTIVESYNC_CONFIGWIZARD_H__
