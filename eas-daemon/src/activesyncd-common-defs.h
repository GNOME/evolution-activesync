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

#define EAS_SERVICE_CALENDAR_OBJECT_PATH "/EasCalendar"
#define EAS_SERVICE_CALENDAR_INTERFACE   "org.meego.activesyncd.EasCalendar"

#define EAS_SERVICE_COMMON_OBJECT_PATH 	 "/EasCommon"
#define EAS_SERVICE_COMMON_INTERFACE   "org.meego.activesyncd.EasCommon"

#define EAS_SERVICE_CONTACT_OBJECT_PATH  "/EasContact"
#define EAS_SERVICE_CONTACT_INTERFACE   "org.meego.activesyncd.EasContact"

#define EAS_SERVICE_MAIL_INTERFACE   "org.meego.activesyncd.EasMail"
#define EAS_SERVICE_MAIL_OBJECT_PATH	 "/EasMail"



#endif /* ACTIVESYNCD_COMMON_DEFS_H */

