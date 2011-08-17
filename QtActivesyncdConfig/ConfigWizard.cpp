// Class include
#include "ConfigWizard.h"
#include "ui_ConfigWizard.h"
// System includes
#include <QAbstractButton>
#include <QApplication>
#include <QInputDialog>
#include <QLineEdit>
#include <QRegExp>
#include <QRegExpValidator>
#include <QStackedWidget>
#include <QStringList>
#include <QTimer>
// User includes


/**
 * Constructor
 */
ConfigWizard::ConfigWizard(QWidget *parent)
: QDialog(parent),
  ui(new Ui::ConfigWizard),
  manualConfigRequested(false)
{
    ui->setupUi(this);

    ui->editEmailAddress->setValidator(new QRegExpValidator(QRegExp("^\\S{2,}@\\S{2,}\\.\\S{2,}"), ui->editEmailAddress));
    ui->editServerAddress->setValidator(new QRegExpValidator(QRegExp("^\\S{2,}\\.\\S{2,}"), ui->editServerAddress));
    ui->editUsername2->setValidator(new QRegExpValidator(QRegExp("\\S+"), ui->editUsername2));

    // Connect signals/slots
    connect(ui->btnNext, SIGNAL(clicked()), SLOT(onNext()));
    connect(ui->btnBack, SIGNAL(clicked()), SLOT(onBack()));
    connect(ui->btnCancel, SIGNAL(clicked()), SLOT(onCancel()));
    connect(ui->editEmailAddress, SIGNAL(textEdited(QString)), SLOT(validateAutoConfigInputs()));
    connect(ui->editServerAddress, SIGNAL(textEdited(QString)), SLOT(validateManualConfigInputs()));
    connect(ui->editUsername2, SIGNAL(textEdited(QString)), SLOT(validateManualConfigInputs()));
    // Connect the two username fields
    connect(ui->editUsername1, SIGNAL(textChanged(QString)), ui->editUsername2, SLOT(setText(QString)));
    connect(ui->editUsername2, SIGNAL(textChanged(QString)), ui->editUsername1, SLOT(setText(QString)));

    changeState(AutoConfigDetails);
}


/**
 * Destructor
 */
ConfigWizard::~ConfigWizard()
{
    delete ui;
}


/**
 * Slot: next button clicked
 */
void ConfigWizard::onNext()
{
    switch (currentState)
    {
    case AutoConfigDetails:
        changeState(TryingAutoConfig);
        break;
    case ManualConfigDetails:
        changeState(TryingManualConfig);
        break;
    case ConfirmRequirements:
        changeState(Finish);
        break;
    case Finish:
        // TODO: apply settings
        QApplication::quit();
        break;
    default:
        break;
    }
}


/**
 * Slot: back button clicked
 */
void ConfigWizard::onBack()
{
    switch (currentState)
    {
    case TryingAutoConfig:
    case ManualConfigDetails:
        changeState(AutoConfigDetails);
        break;
    case TryingManualConfig:
        changeState(ManualConfigDetails);
        break;
    case ConfirmRequirements:
        changeState(manualConfigRequested ? ManualConfigDetails : AutoConfigDetails);
        break;
    case Finish:
        changeState(ConfirmRequirements);
        break;
    default:
        break;
    }
}


/**
 * Slot: cancel button clicked
 */
void ConfigWizard::onCancel()
{
    QApplication::quit();
}


/**
 * Slot: Auto-configuration completed successfully
 */
void ConfigWizard::onConfigSuccess()
{
    changeState(ConfirmRequirements);
}


/**
 * Slot: auto-configuration failed
 */
void ConfigWizard::onConfigFailure()
{
    changeState(ManualConfigDetails);
}


/**
 * Slot: text in one of the auto-config inputs has changed
 */
void ConfigWizard::validateAutoConfigInputs()
{
    ui->btnNext->setEnabled(ui->editEmailAddress->hasAcceptableInput());
}


/**
 * Slot: text in one of the manual config inputs has changed
 */
void ConfigWizard::validateManualConfigInputs()
{
    ui->btnNext->setEnabled(ui->editServerAddress->hasAcceptableInput() && ui->editUsername2->hasAcceptableInput());
}


/**
 * Switch to a specific page in the wizard
 */
void ConfigWizard::changeState(ConfigWizard::State state)
{
    currentState = state;

    // Reset the button captions to defaults
    setButtonCaptions();

    // Set button defaults
    ui->btnBack->setEnabled(state != AutoConfigDetails); // Enabled on all but the first page
    ui->btnNext->setEnabled(true);
    ui->btnCancel->setEnabled(true);

    switch (currentState)
    {
    case AutoConfigDetails:
        ui->wizard->setCurrentWidget(ui->pageAutoConfig);
        setTitle(tr("Automatic configuration"), tr("Enter your e-mail adress and we'll try to guess your Exchange server details."));
        validateAutoConfigInputs();
        break;

    case TryingAutoConfig:
        ui->wizard->setCurrentWidget(ui->pageBusy);
        ui->btnNext->setEnabled(false);
        setTitle(tr("Attempting automatic configuration"), tr("Please wait..."));

        // TEMP
        QTimer::singleShot(2000, this, SLOT(onConfigFailure()));

        break;

    case ManualConfigDetails:
        manualConfigRequested = true;
        ui->wizard->setCurrentWidget(ui->pageManualConfig);
        setTitle(tr("Manual configuration"), tr("Sorry, your Exchange server details could not be guessed. Please enter them below."));
        validateManualConfigInputs();
        break;

    case TryingManualConfig:
        ui->wizard->setCurrentWidget(ui->pageBusy);
        ui->btnNext->setEnabled(false);
        setTitle(tr("Attempting configuration"), tr("Please wait..."));

        // TEMP
        QTimer::singleShot(2000, this, SLOT(onConfigSuccess()));

        break;

    case ConfirmRequirements:
        {
        ui->wizard->setCurrentWidget(ui->pageConfirmRequirements);
        setTitle(tr("Confirm ActiveSync requirements"), tr("The server at <b>%1</b> requires you to accept the following features before continuing.").arg(ui->editServerAddress->text().trimmed()));
        setButtonCaptions(tr("Accept"));

        // TEMP
        QStringList items;
        items << tr("Remote wipe")
              << tr("Disable all other POP and IMAP e-mail accounts on this device (and by the way, isn't this a long item?)")
              << tr("Offer up first-born")
              << tr("And another") << tr("And another") << tr("And another") << tr("And another")
              << tr("And another") << tr("And another") << tr("And another") << tr("And another")
              << tr("And another") << tr("And another") << tr("And another") << tr("And another")
              << tr("And another") << tr("And another") << tr("And another") << tr("And another")
              << tr("And another") << tr("And another");
        ui->listRequirements->addItems(items);
        }
        break;

    case Finish:
        ui->wizard->setCurrentWidget(ui->pageFinish);
        setTitle(tr("ActveSync is now configured"), tr("Please confirm which other services you would like to synchronise with this device."));
        setButtonCaptions(tr("Finish"));
        ui->btnCancel->setEnabled(false);
        break;

    case Error:
        ui->wizard->setCurrentWidget(ui->pageError);
        setTitle(tr("An error has occurred"), tr("Sorry, it was not possible to configure this device for Exchange access."));
        setButtonCaptions(tr("Finish"));
        break;

    default:
        break;
    }
}


/**
 * Set the wizard title & subtitle fields
 */
void ConfigWizard::setTitle(const QString title, const QString& subTitle)
{
    ui->lblTitle->setText(title);
    ui->lblSubTitle->setText(subTitle);
}


/**
 * Set the wizard button captions
 */
void ConfigWizard::setButtonCaptions(const QString& nextButtonCaption, const QString& backButtonCaption)
{
    ui->btnNext->setText(nextButtonCaption.isEmpty() ? tr("Next") : nextButtonCaption);
    ui->btnBack->setText(backButtonCaption.isEmpty() ? tr("Back") : backButtonCaption);
}

