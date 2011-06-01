#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "eas-logger.h"


void eas_logger(const gchar *log_domain, 
                       GLogLevelFlags log_level,
                       const gchar *message,
                       gpointer user_data)
{
    FILE *logfile = NULL;
    pid_t pid = getpid();
    int envLevel = 4; 
    
    if (getenv("EAS_DEBUG_FILE"))
    {
        logfile = fopen (g_getenv("EAS_DEBUG_FILE"), "a");
    }

    if (getenv("EAS_DEBUG"))
    {
        envLevel = atoi (g_getenv ("EAS_DEBUG"));
    }

    if (log_level == G_LOG_LEVEL_ERROR)
    {
        g_log_default_handler(log_domain, log_level, message, user_data);
        return;
    }

    if (logfile)
    {
        if (envLevel > 0 && log_level == G_LOG_LEVEL_CRITICAL)
            fprintf(logfile, "(process:%d): %s%s*** CRITICAL ***:%s \n", pid, (log_domain?log_domain:""), (log_domain?"-":""), message);

        if (envLevel > 1 && log_level == G_LOG_LEVEL_WARNING)
            fprintf(logfile, "(process:%d): %s%sWARNING **:%s\n", pid, (log_domain?log_domain:""), (log_domain?"-":""), message);

        if (envLevel > 2 && log_level == G_LOG_LEVEL_MESSAGE)
            fprintf(logfile, "(process:%d): %s%sMESSAGE:%s\n", pid, (log_domain?log_domain:""), (log_domain?"-":""), message);

        if (envLevel > 3 && log_level == G_LOG_LEVEL_DEBUG)
            fprintf(logfile,"(process:%d): %s%sDEBUG:%s\n", pid, (log_domain?log_domain:""), (log_domain?"-":""), message);
                
        fclose(logfile);
    }
    else
    {
        if (envLevel > 0 && log_level == G_LOG_LEVEL_CRITICAL)
            fprintf(stderr, "(process:%d): %s%s*** CRITICAL ***:%s \n", pid, (log_domain?log_domain:""), (log_domain?"-":""), message);

        if (envLevel > 1 && log_level == G_LOG_LEVEL_WARNING)
            fprintf(stderr, "(process:%d): %s%sWARNING **:%s\n", pid, (log_domain?log_domain:""), (log_domain?"-":""), message);

        if (envLevel > 2 && log_level == G_LOG_LEVEL_MESSAGE)
            fprintf(stderr, "(process:%d): %s%sMessage:%s\n", pid, (log_domain?log_domain:""), (log_domain?"-":""), message);

        if (envLevel > 3 && log_level == G_LOG_LEVEL_DEBUG)
            fprintf(stdout,"(process:%d): %s%sDEBUG:%s\n", pid, (log_domain?log_domain:""), (log_domain?"-":""), message);
    }
}
