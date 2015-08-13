#!/bin/sh

# Read email address
read -r -p "Enter email address: " EMAILADDR

read -r -p "Enter username: " ASUSERNAME


# We should do autodiscover here to find the URL
read -r -p "Enter server URL: " SERVERURL

# Should test the connectivity here.

# Would be nice if it were like this...
# Not usable untill we can finish the account listener bug
:||{
match=0
for account in `gsettings get org.meego.activesyncd accounts`
do
	account=${account%[\']*}
	account=${account#*[\']}
	if [ "$account" = "$EMAILADDR" ] 
	then
		match=1
	fi	
done

if [ "$match" -eq "0" ]
then
	gsettings set org.meego.activesyncd accounts "`gsettings get org.meego.activesyncd accounts | sed s/]//`, '$EMAILADDR']"
fi

gsettings set org.meego.activesyncd.account:/org/meego/activesyncd/account/$EMAILADDR/ username "$ASUSERNAME"
gsettings set org.meego.activesyncd.account:/org/meego/activesyncd/account/$EMAILADDR/ serveruri "$SERVERURL"
}

# Add Evolution account in /apps/evolution/mail/accounts with URL
# eas:///account_uid=$EMAILADDR;check_all
# Again, we *really* do need to append, not overwrite.
#
# gconftool-2 --set /apps/evolution/mail/accounts --type=list --list-type=string '[<?xml version="1.0"?>  
# <account name="David Woodhouse" uid="1309534230.10835.2@localhost.localdomain" enabled="true"><identity><name>David Woodhouse</name><addr-spec>david.woodhouse@intel.com</addr-spec><signature uid=""/></identity><source save-passwd="false" keep-on-server="false" auto-check="false" auto-check-timeout="0"><url>eas://david.woodhouse@intel.com/;sync_offline=1;account_uid=david.woodhouse@intel.com</url></source><transport save-passwd="false"><url>eas://david.woodhouse@intel.com/;account_uid=david.woodhouse@intel.com</url></transport><drafts-folder>mbox:/home/meego/.local/share/evolution/mail/local#Drafts</drafts-folder><sent-folder>mbox:/home/meego/.local/share/evolution/mail/local#Sent</sent-folder><auto-cc always="false"><recipients></recipients></auto-cc><auto-bcc always="false"><recipients></recipients></auto-bcc><receipt-policy policy="never"/><pgp encrypt-to-self="false" always-trust="false" always-sign="false" no-imip-sign="false"/><smime sign-default="false" encrypt-default="false" encrypt-to-self="false"/></account>]'

# Add SyncEvolution account
