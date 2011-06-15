/**
 *  
 *  Filename:activesyncd-common-defs.h
 *  Project: activesyncd 
 *  Description: Define the activesyncd dbus service.
 *
 */
#ifndef ACTIVESYNCD_COMMON_DEFS_H
#define ACTIVESYNCD_COMMON_DEFS_H

/*
#define EAS_SERVICE_NAME        "org.meego.eas.daemon"
#define EAS_SERVICE_OBJECT_PATH "/EAS"
#define EAS_SERVICE_INTERFACE   "org.meego.Eas"
*/

#define EAS_SERVICE_NAME        "org.meego.activesyncd"

#define EAS_SERVICE_SYNC_OBJECT_PATH "/EasSync"
#define EAS_SERVICE_SYNC_INTERFACE   "org.meego.activesyncd.EasSync"

#define EAS_SERVICE_COMMON_OBJECT_PATH 	 "/EasCommon"
#define EAS_SERVICE_COMMON_INTERFACE   "org.meego.activesyncd.EasCommon"


#define EAS_SERVICE_MAIL_INTERFACE   "org.meego.activesyncd.EasMail"
#define EAS_SERVICE_MAIL_OBJECT_PATH	 "/EasMail"



#endif /* ACTIVESYNCD_COMMON_DEFS_H */

