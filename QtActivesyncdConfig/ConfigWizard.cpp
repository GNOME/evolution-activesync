// Class include
#include "ConfigWizard.h"
#include "ui_ConfigWizard.h"
// System includes
#include <QAbstractButton>
#include <QApplication>
#include <QDebug>
#include <QInputDialog>
#include <QLineEdit>
#include <QRegExp>
#include <QRegExpValidator>
#include <QStackedWidget>
#include <QStringList>
#include <QTimer>
// User includes
#include "../eas-daemon/libeas/eas-connection.h"


extern ConfigWizard* theWizard;


/**
 * Global callback, called after attempt at auto-discovery
 */
void autoDiscoverCallback(char* server_uri, void* /*data*/, GError* /*error*/)
{
    qDebug("Entering autoDiscoverCallback() with server_uri=%s", (server_uri ? server_uri : "0"));

    if (server_uri)
    {
        QString qServerUri(server_uri);
        if (!qServerUri.isEmpty())
        {
            theWizard->getProvisionReqts(qServerUri);
            return;
        }
    }

    theWizard->onAutoDiscoverFailure();
}


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
    ui->editServerUri->setValidator(new QRegExpValidator(QRegExp("^\\S{2,}\\.\\S{2,}"), ui->editServerUri));
    ui->editUsername2->setValidator(new QRegExpValidator(QRegExp("\\S+"), ui->editUsername2));

    // Connect signals/slots
    connect(ui->btnNext, SIGNAL(clicked()), SLOT(onNext()));
    connect(ui->btnBack, SIGNAL(clicked()), SLOT(onBack()));
    connect(ui->btnCancel, SIGNAL(clicked()), SLOT(onCancel()));
    connect(ui->editEmailAddress, SIGNAL(textEdited(QString)), SLOT(validateAutoDiscoverInputs()));
    connect(ui->editServerUri, SIGNAL(textEdited(QString)), SLOT(validateManualServerInputs()));
    connect(ui->editUsername2, SIGNAL(textEdited(QString)), SLOT(validateManualServerInputs()));
    // Connect the two username fields
    connect(ui->editUsername1, SIGNAL(textChanged(QString)), ui->editUsername2, SLOT(setText(QString)));
    connect(ui->editUsername2, SIGNAL(textChanged(QString)), ui->editUsername1, SLOT(setText(QString)));

    changeState(AutoDiscoverDetails);
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
    case AutoDiscoverDetails:
        changeState(TryingAutoDiscover);
        break;

    case ManualServerDetails:
        username = ui->editUsername2->text().trimmed();
        getProvisionReqts(ui->editServerUri->text().trimmed());
        break;

    case ConfirmProvisionReqts:
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
    case TryingAutoDiscover:
    case ManualServerDetails:
        changeState(AutoDiscoverDetails);
        break;

    case GettingProvisionReqts:
    case ConfirmProvisionReqts:
        changeState(manualConfigRequested ? ManualServerDetails : AutoDiscoverDetails);
        break;

    case Finish:
        changeState(ConfirmProvisionReqts);
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
 * Slot: Auto-discovery completed successfully
 */
void ConfigWizard::getProvisionReqts(const QString& serverUri)
{
    qDebug() << "Entering getProvisionReqts() with serverUri=" << serverUri;
    this->serverUri = serverUri;
    changeState(GettingProvisionReqts);
}


/**
 * Slot: auto-discovery failed
 */
void ConfigWizard::onAutoDiscoverFailure()
{
    changeState(ManualServerDetails);
}


/**
 * Slot
 */
void ConfigWizard::onProvisionFailure()
{

}


/**
 * Slot
 */
void ConfigWizard::onProvisionSuccess()
{

}


/**
 * Slot: text in one of the auto-config inputs has changed
 */
void ConfigWizard::validateAutoDiscoverInputs()
{
    ui->btnNext->setEnabled(ui->editEmailAddress->hasAcceptableInput());
}


/**
 * Slot: text in one of the manual config inputs has changed
 */
void ConfigWizard::validateManualServerInputs()
{
    ui->btnNext->setEnabled(ui->editServerUri->hasAcceptableInput() && ui->editUsername2->hasAcceptableInput());
}


/**
 * Switch to a specific page in the wizard
 */
void ConfigWizard::changeState(ConfigWizard::State state)
{
    qDebug() << "Entering changeState";

    currentState = state;

    // Reset the button captions to defaults
    setButtonCaptions();

    // Set button defaults
    ui->btnBack->setEnabled(state != AutoDiscoverDetails); // Enabled on all but the first page
    ui->btnNext->setEnabled(true);
    ui->btnCancel->setEnabled(true);

    switch (currentState)
    {
    case AutoDiscoverDetails:
        ui->wizard->setCurrentWidget(ui->pageAutoConfig);
        setTitle(tr("Automatic configuration"), tr("Enter your e-mail adress and we'll try to guess your Exchange server details."));
        validateAutoDiscoverInputs();
        break;

    case TryingAutoDiscover:
        qDebug() << "Entering state: TryingAutoDiscover";
        ui->wizard->setCurrentWidget(ui->pageBusy);
        ui->btnNext->setEnabled(false);
        setTitle(tr("Attempting automatic configuration"), tr("Please wait..."));

        qDebug() << "GOT TO HERE";

        // Use username if entered, otherwise use the e-mail address
        emailAddress = ui->editEmailAddress->text();
        username = ui->editUsername1->text();
        qDebug() << "Calling autodiscover with email address" << emailAddress << "username" << username;

        eas_connection_autodiscover(
            autoDiscoverCallback, 0,
            (const gchar*)emailAddress.constData(),
            (username.isEmpty() ? 0 : (const gchar*)username.constData()));
        break;

    case ManualServerDetails:
        manualConfigRequested = true;
        ui->wizard->setCurrentWidget(ui->pageManualConfig);
        setTitle(tr("Manual configuration"), tr("Sorry, your Exchange server details could not be guessed. Please enter them below."));
        validateManualServerInputs();
        break;

    case GettingProvisionReqts:
        ui->wizard->setCurrentWidget(ui->pageBusy);
        ui->btnNext->setEnabled(false);
        setTitle(tr("Getting server provisioning requirements"), tr("Please wait..."));

        // TODO: call eas_mail_handler_get_folder_list()

        // TEMP
        QTimer::singleShot(2000, this, SLOT(onConfigSuccess()));

        break;

    case ConfirmProvisionReqts:
        {
        ui->wizard->setCurrentWidget(ui->pageConfirmRequirements);
        setTitle(tr("Confirm ActiveSync requirements"), tr("The server at <b>%1</b> requires you to accept the following features before continuing.").arg(ui->editServerUri->text().trimmed()));
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
        setTitle(tr("Error"), tr("Sorry, something has gone wrong."));
        setButtonCaptions(tr("Quit"));
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


void ConfigWizard::error(const QString& msg)
{
    ui->lblErrorDetails->setText(msg);
    changeState(Error);
}

