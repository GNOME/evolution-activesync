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

    void validateAutoConfigInputs();
    void validateManualConfigInputs();

    void onConfigSuccess();
    void onConfigFailure();

private:
    enum State
    {
        AutoConfigDetails,
        TryingAutoConfig,
        ManualConfigDetails,
        TryingManualConfig,
        ConfirmRequirements,
        Finish,
        Error
    };

    void changeState(State currentState);
    void setTitle(const QString title, const QString& subTitle);
    void setButtonCaptions(const QString& nextButtonCaption = "", const QString& backButtonCaption = "");

private:
    Ui::ConfigWizard* ui;
    QString password;
    State currentState;
    bool manualConfigRequested;
};


#endif // CONFIGWIZARD_H
