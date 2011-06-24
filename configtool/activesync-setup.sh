#!/bin/sh

# Read email address
read -r -p "Enter email address: " EMAILADDR

read -r -p "Enter username: " ASUSERNAME

# Shouldn't ask for this in advance; SSO might work, and the dæmon will
# if it needs it.
#read -r -p "Enter password: " -s ASPASSWD
#echo

# We should do autodiscover here to find the URL
read -r -p "Enter server URL: " SERVERURL

# Should test the connectivity here.

# Would be nice if it were like this...
gconftool-2 --set --type=string /apps/activesyncd/accounts/$EMAILADDR/username "$ASUSERNAME"
gconftool-2 --set --type=string /apps/activesyncd/accounts/$EMAILADDR/serverUri "$SERVERURL"
#gconftool-2 --set --type=string /apps/activesyncd/accounts/$EMAILADDR/password "$ASPASSWD"

# Add Evolution account in /apps/evolution/mail/accounts with URL
# eas:///account_uid=$EMAILADDR;check_all
# Again, we *really* do need to append, not overwrite.

# Add SyncEvolution account
