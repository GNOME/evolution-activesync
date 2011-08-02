#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "eas-logger.h"

static GStaticMutex g_mutex = G_STATIC_MUTEX_INIT;

void eas_logger (const gchar *log_domain,
                 GLogLevelFlags log_level,
                 const gchar *message,
                 gpointer user_data)
{
    FILE *logfile = NULL;
    GThread *tid = g_thread_self();
    pid_t pid = getpid();
    int envLevel = 4;

    g_static_mutex_lock (&g_mutex);

    if (getenv ("EAS_DEBUG_FILE"))
    {
        logfile = fopen (g_getenv ("EAS_DEBUG_FILE"), "a");
    }

    if (getenv ("EAS_DEBUG"))
    {
        envLevel = atoi (g_getenv ("EAS_DEBUG"));
    }

    if (log_level == G_LOG_LEVEL_ERROR)
    {
        g_log_default_handler (log_domain, log_level, message, user_data);
        g_static_mutex_unlock (&g_mutex);
        return;
    }

    if (logfile)
    {
        if (envLevel > 0 && log_level == G_LOG_LEVEL_CRITICAL)
            fprintf (logfile, "(process:%d:%x): %s%s*** CRITICAL ***:%s \n", pid, (guint)tid, (log_domain ?: ""), (log_domain ? "-" : ""), message);

        if (envLevel > 1 && log_level == G_LOG_LEVEL_WARNING)
            fprintf (logfile, "(process:%d:%x): %s%sWARNING **:%s\n", pid, (guint)tid, (log_domain ?: ""), (log_domain ? "-" : ""), message);

        if (envLevel > 2 && log_level == G_LOG_LEVEL_MESSAGE)
            fprintf (logfile, "(process:%d:%x): %s%sMESSAGE:%s\n", pid, (guint)tid, (log_domain ?: ""), (log_domain ? "-" : ""), message);

        if (envLevel > 3 && log_level == G_LOG_LEVEL_DEBUG)
            fprintf (logfile, "(process:%d:%x): %s%sDEBUG:%s\n", pid, (guint)tid, (log_domain ?: ""), (log_domain ? "-" : ""), message);

        fclose (logfile);
    }
    else
    {
        if (envLevel > 0 && log_level == G_LOG_LEVEL_CRITICAL)
            fprintf (stderr, "(process:%d:%x): %s%s*** CRITICAL ***:%s \n", pid, (guint)tid, (log_domain ?: ""), (log_domain ? "-" : ""), message);

        if (envLevel > 1 && log_level == G_LOG_LEVEL_WARNING)
            fprintf (stderr, "(process:%d:%x): %s%sWARNING **:%s\n", pid, (guint)tid, (log_domain ?: ""), (log_domain ? "-" : ""), message);

        if (envLevel > 2 && log_level == G_LOG_LEVEL_MESSAGE)
            fprintf (stderr, "(process:%d:%x): %s%sMessage:%s\n", pid, (guint)tid, (log_domain ?: ""), (log_domain ? "-" : ""), message);

        if (envLevel > 3 && log_level == G_LOG_LEVEL_DEBUG)
            fprintf (stdout, "(process:%d:%x): %s%sDEBUG:%s\n", pid, (guint)tid, (log_domain ?: ""), (log_domain ? "-" : ""), message);
            
        fflush(stderr);
        fflush (stdout);
    }

    g_static_mutex_unlock (&g_mutex);
}
