// Class include
#include "ConfigWizard.h"
#ifdef HAVE_CONFIG_H /* In an autotools build */
#include "ConfigWizard.ui.h"
#else
#include "ui_ConfigWizard.h"
#endif
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
#include "../libeasmail/src/eas-provision-list.h"


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
            theWizard->storeServerDetails(qServerUri);
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
  serverDetailsEnteredManually(false),
  serverHasProvisioningReqts(false),
  mailHandler(0),
  tid(0),
  tidStatus(0)
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


    // TEMPORARY TEST DATA!!
    ui->editEmailAddress->setText("andy@cstylianou.com");
    ui->editUsername1->setText("andy"); // Will also set editUsername2 thanks to the signal/slot above
    ui->editServerUri->setText("https://cstylianou.com/Microsoft-Server-ActiveSync");


    changeState(AutoDiscoverDetails);
}


/**
 * Destructor
 */
ConfigWizard::~ConfigWizard()
{
    g_free(tid);
    g_free(tidStatus);
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
        storeServerDetails(ui->editServerUri->text().trimmed());
        break;

    case ConfirmProvisionReqts:
        acceptProvisionReqts();
        break;

    case Finish:
    case Error:
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
        changeState(serverDetailsEnteredManually ? ManualServerDetails : AutoDiscoverDetails);
        break;

    case Finish:
        if (serverHasProvisioningReqts)
        {
            getProvisionReqts();
        }
        else
        {
            changeState(serverDetailsEnteredManually ? ManualServerDetails : AutoDiscoverDetails);
        }
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
 * Slot: Auto-discovery or manual server entry completed successfully
 */
void ConfigWizard::storeServerDetails(const QString& uri)
{
    qDebug() << "Entering storeServerDetails() with uri=" << uri;
    serverUri = uri;

    // TODO: ADD BETTER ERROR HANDLING?

    // Store the account details in GConf
    GConfClient* gconfClient = gconf_client_get_default();
    if (gconfClient)
    {
        EasAccountList* easAccountList = eas_account_list_new(gconfClient);
        if (easAccountList)
        {
            EasAccount* easAccount = eas_account_new();
            if (easAccount)
            {
                eas_account_set_uid(easAccount, (const gchar*)emailAddress.toUtf8().constData());
                eas_account_set_uri(easAccount, (const gchar*)serverUri.toUtf8().constData());
                eas_account_set_username(easAccount, (const gchar*)username.toUtf8().constData());
                eas_account_list_save_account(easAccountList, easAccount);
                g_object_unref(easAccount);
            }
            else // easAccount is null
            {
                qWarning() << "Failed to create new EasAccountList in ConfigWizard::storeServerDetails()";
            }

            g_object_unref(easAccountList);
        }
        else // easAccountList is null
        {
            qWarning() << "Failed to create new EasAccount in ConfigWizard::storeServerDetails()";
        }

        g_object_unref(gconfClient);
    }
    else // gconfClienf is null
    {
        qWarning() << "Failed to create new GConfClient in ConfigWizard::storeServerDetails()";
    }

    getProvisionReqts();
}


/**
 * Slot: auto-discovery failed
 */
void ConfigWizard::onAutoDiscoverFailure()
{
    changeState(ManualServerDetails);
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
        ui->wizard->setCurrentWidget(ui->pageBusy);
        ui->btnNext->setEnabled(false);
        setTitle(tr("Attempting automatic configuration"), tr("Please wait..."));

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
        serverDetailsEnteredManually = true;
        ui->wizard->setCurrentWidget(ui->pageManualConfig);
        setTitle(tr("Manual configuration"), tr("Sorry, your Exchange server details could not be guessed. Please enter them below."));
        validateManualServerInputs();
        break;

    case GettingProvisionReqts:
        ui->wizard->setCurrentWidget(ui->pageBusy);
        ui->btnNext->setEnabled(false);
        setTitle(tr("Getting server provisioning requirements"), tr("Please wait..."));
        break;

    case ConfirmProvisionReqts:
        {
        ui->wizard->setCurrentWidget(ui->pageConfirmRequirements);
        setTitle(tr("Confirm ActiveSync requirements"), tr("The server at <b>%1</b> requires you to accept the following features before continuing.").arg(ui->editServerUri->text().trimmed()));
        setButtonCaptions(tr("Accept"));
        }
        break;

    case Finish:
        ui->wizard->setCurrentWidget(ui->pageFinish);
        setTitle(tr("ActiveSync is now configured"), tr("Please confirm which other services you would like to synchronise with this device."));
        setButtonCaptions(tr("Finish"));
        ui->btnCancel->setEnabled(false);
        break;

    case Error:
        ui->wizard->setCurrentWidget(ui->pageError);
        ui->btnBack->setEnabled(false);
        setTitle(tr("Error"), tr("Sorry, something has gone wrong. See below for details."));
        setButtonCaptions(tr("Quit"));
        break;

    default:
        break;
    }
}


/**
 * Contact the server and get the provisioning requirements (if any)
 */
void ConfigWizard::getProvisionReqts()
{
    changeState(GettingProvisionReqts);

    GError* error = 0;

    // Prepare member variable
    if (mailHandler)
    {
        g_object_unref(mailHandler);
        mailHandler = 0;
    }
    if (tid)
    {
        g_free(tid);
        tid = 0;
    }
    if (tidStatus)
    {
        g_free(tidStatus);
        tidStatus = 0;
    }


    mailHandler = eas_mail_handler_new(emailAddress.toUtf8().constData(), &error);
    if (error)
    {
        showError(QString(error->message));
        g_error_free(error);
        error = 0;
        return;
    }

    if (mailHandler)
    {
        GSList* folderList = 0;
        if (eas_mail_handler_get_folder_list(mailHandler, true, &folderList, 0, &error))
        {
            // TODO: decide how to deal with this. Do we even need to call ...get_folder_list()?
            // Will ...get_provision_list() on its own do?
        }
        if (error)
        {
            g_error_free(error);
            error = 0;
        }

        EasProvisionList* provisionList = 0;
        if (eas_mail_handler_get_provision_list(mailHandler, &tid, &tidStatus, &provisionList, 0, &error))
        {
            if (provisionList)
            {
                serverHasProvisioningReqts = true;

                // Ask the user to accept the provisioning requirements
                ui->listRequirements->clear();
                ui->listRequirements->addItem(tr("DevicePasswordEnabled: ") + provisionList->DevicePasswordEnabled);
                ui->listRequirements->addItem(tr("AlphaNumericDevicePasswordRequired: ") + provisionList->AlphaNumericDevicePasswordRequired);
                ui->listRequirements->addItem(tr("PasswordRecoveryEnabled: ") + provisionList->PasswordRecoveryEnabled);
                ui->listRequirements->addItem(tr("RequireStorageCardEncryption: ") + provisionList->RequireStorageCardEncryption);
                ui->listRequirements->addItem(tr("AttachmentsEnabled: ") + provisionList->AttachmentsEnabled);
                ui->listRequirements->addItem(tr("MinDevicePasswordLength: ") + provisionList->MinDevicePasswordLength);
                ui->listRequirements->addItem(tr("MaxInactivityTimeDeviceLock: ") + provisionList->MaxInactivityTimeDeviceLock);
                ui->listRequirements->addItem(tr("MaxDevicePasswordFailedAttempts: ") + provisionList->MaxDevicePasswordFailedAttempts);
                ui->listRequirements->addItem(tr("MaxAttachmentSize: ") + provisionList->MaxAttachmentSize);
                ui->listRequirements->addItem(tr("AllowSimpleDevicePassword: ") + provisionList->AllowSimpleDevicePassword);
                ui->listRequirements->addItem(tr("DevicePasswordExpiration: ") + provisionList->DevicePasswordExpiration);
                ui->listRequirements->addItem(tr("DevicePasswordHistory: ") + provisionList->DevicePasswordHistory);
                ui->listRequirements->addItem(tr("AllowStorageCard: ") + provisionList->AllowStorageCard);
                ui->listRequirements->addItem(tr("AllowCamera: ") + provisionList->AllowCamera);
                ui->listRequirements->addItem(tr("RequireDeviceEncryption: ") + provisionList->RequireDeviceEncryption);
                ui->listRequirements->addItem(tr("AllowUnsignedApplications: ") + provisionList->AllowUnsignedApplications);
                ui->listRequirements->addItem(tr("AllowUnsignedInstallationPackages: ") + provisionList->AllowUnsignedInstallationPackages);
                ui->listRequirements->addItem(tr("MinDevicePasswordComplexCharacters: ") + provisionList->MinDevicePasswordComplexCharacters);
                ui->listRequirements->addItem(tr("AllowWifi: ") + provisionList->AllowWifi);
                ui->listRequirements->addItem(tr("AllowTextMessaging: ") + provisionList->AllowTextMessaging);
                ui->listRequirements->addItem(tr("AllowPOPIMAPEmail: ") + provisionList->AllowPOPIMAPEmail);
                ui->listRequirements->addItem(tr("AllowBluetooth: ") + provisionList->AllowBluetooth);
                ui->listRequirements->addItem(tr("AllowIrDA: ") + provisionList->AllowIrDA);
                ui->listRequirements->addItem(tr("RequireManualSyncWhenRoaming: ") + provisionList->RequireManualSyncWhenRoaming);
                ui->listRequirements->addItem(tr("AllowDesktopSync: ") + provisionList->AllowDesktopSync);
                ui->listRequirements->addItem(tr("MaxCalendarAgeFilter: ") + provisionList->MaxCalendarAgeFilter);
                ui->listRequirements->addItem(tr("AllowHTMLEmail: ") + provisionList->AllowHTMLEmail);
                ui->listRequirements->addItem(tr("MaxEmailAgeFilter: ") + provisionList->MaxEmailAgeFilter);
                ui->listRequirements->addItem(tr("MaxEmailBodyTruncationSize: ") + provisionList->MaxEmailBodyTruncationSize);
                ui->listRequirements->addItem(tr("MaxEmailHTMLBodyTruncationSize: ") + provisionList->MaxEmailHTMLBodyTruncationSize);
                ui->listRequirements->addItem(tr("RequireSignedSMIMEMessages: ") + provisionList->RequireSignedSMIMEMessages);
                ui->listRequirements->addItem(tr("RequireEncryptedSMIMEMessages: ") + provisionList->RequireEncryptedSMIMEMessages);
                ui->listRequirements->addItem(tr("RequireSignedSMIMEAlgorithm: ") + provisionList->RequireSignedSMIMEAlgorithm);
                ui->listRequirements->addItem(tr("RequireEncryptionSMIMEAlgorithm: ") + provisionList->RequireEncryptionSMIMEAlgorithm);
                ui->listRequirements->addItem(tr("AllowSMIMEEncryptionAlgorithmNegotiation: ") + provisionList->AllowSMIMEEncryptionAlgorithmNegotiation);
                ui->listRequirements->addItem(tr("AllowSMIMESoftCerts: ") + provisionList->AllowSMIMESoftCerts);
                ui->listRequirements->addItem(tr("AllowBrowser: ") + provisionList->AllowBrowser);
                ui->listRequirements->addItem(tr("AllowConsumerEmail: ") + provisionList->AllowConsumerEmail);
                ui->listRequirements->addItem(tr("AllowRemoteDesktop: ") + provisionList->AllowRemoteDesktop);
                ui->listRequirements->addItem(tr("AllowInternetSharing: ") + provisionList->AllowInternetSharing);
                // TODO: add the two list members too

                changeState(ConfirmProvisionReqts);
            }
            else
            {
                serverHasProvisioningReqts = false;
                changeState(Finish);
            }
        }
        else if (error != 0)
        {
            showError(QString(error->message));
            g_error_free(error);
        }
        else
        {
            showError(tr("Something went wrong and no error message was returned. :("));
        }
    }
    else // mailHandler is null
    {
        showError("Failed to construct new EasEmailHandler in ConfigWizard::changeState()");
        return;
    }
}


/**
 * Inform the server the user has accepted the provisioning requirements
 */
void ConfigWizard::acceptProvisionReqts()
{
    if (mailHandler && tid && tidStatus)
    {
        GError* error = 0;
        if (eas_mail_handler_accept_provision_list(mailHandler, tid, tidStatus, 0, &error))
        {
            changeState(Finish);
        }
        else
        {
            showError(tr("Failed to accept the server requirements."));
        }
    }

    // Tidy up any instantiated g-objects
    g_free(tid);
    tid = 0;
    g_free(tidStatus);
    tidStatus = 0;
    if (mailHandler)
    {
        g_object_unref(mailHandler);
        mailHandler = 0;
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


void ConfigWizard::showError(const QString& msg)
{
    ui->lblErrorDetails->setText(msg);
    changeState(Error);
}

