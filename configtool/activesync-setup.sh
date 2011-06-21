#!/bin/sh

# Read email address
read -r -p "Enter email address: " EMAILADDR

read -r -p "Enter username: " ASUSERNAME
set | grep ASUSERNAME
# Shouldn't ask for this in advance; SSO might work, and we 
# should prompt when we need it
read -r -p "Enter password: " -s ASPASSWD
echo

# We should do autodiscover here to find the URL
read -r -p "Enter server URL: " SERVERURL

# Should test the connectivity here.

# Would be nice if it were like this...
#gconftool-2 --set --type=string /apps/activesync/accounts/$EMAILADDR/username "$USERNAME"
#gconftool-2 --set --type=string /apps/activesync/accounts/$EMAILADDR/server_uri "$SERVERURL"
#gconftool-2 --set --type=string /apps/activesync/accounts/$EMAILADDR/password "$ASPASSWD"
ESCAPED_URL=$(echo "$SERVERURL" | sed -e 's/</&lt;/' -e 's/>/&gt;/' -e 's/&/&amp;/')
ESCAPED_USERNAME=$(echo "$ASUSERNAME" | sed -e 's/</&lt;/' -e 's/>/&gt;/' -e 's/&/&amp;/')
ESCAPED_PASSWD=$(echo "$ASPASSWD" | sed -e 's/</&lt;/' -e 's/>/&gt;/' -e 's/&/&amp;/')

# Append, don't overwrite
gconftool-2 --set --type=list --list-type=string /apps/activesyncd/accounts "[<?xml version=\"1.0\"?> <account uid=\"$EMAILADDR\"> <uri>$ESCAPED_URL</uri> <username>$ESCAPED_USERNAME</username> <password>$ESCAPED_PASSWD</password> </account>]"

# Add Evolution account in /apps/evolution/mail/accounts with URL
# eas:///account_uid=$EMAILADDR;check_all
# Again, we *really* do need to append, not overwrite.

# Add SyncEvolution account
