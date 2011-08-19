#ifndef CONFIGWIZARD_H
#define CONFIGWIZARD_H

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

public slots:
    void onNext();
    void onBack();
    void onCancel();

    void validateAutoDiscoverInputs();
    void validateManualServerInputs();

    void getProvisionReqts(const QString& serverUri);
    void onAutoDiscoverFailure();
    void onProvisionSuccess();
    void onProvisionFailure();
//    void onConfigSuccess();
//    void onConfigFailure();

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

    void changeState(State currentState);
    void setTitle(const QString title, const QString& subTitle);
    void setButtonCaptions(const QString& nextButtonCaption = "", const QString& backButtonCaption = "");

private:
    Ui::ConfigWizard* ui;
    State currentState;
    bool manualConfigRequested;
    QString serverUri;
    QString emailAddress;
    QString username;
};


#endif // CONFIGWIZARD_H
