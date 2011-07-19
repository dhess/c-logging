/*
 * logtest.c
 * Demonstrate the logging interface.
 *
 * Written in 2011 by Drew Hess <dhess-src@bothan.net>.
 *
 * To the extent possible under law, the author(s) have dedicated all
 * copyright and related and neighboring rights to this software to
 * the public domain worldwide. This software is distributed without
 * any warranty.
 *
 * You should have received a copy of the CC0 Public Domain Dedication
 * along with this software. If not, see
 * <http://creativecommons.org/publicdomain/zero/1.0/>.
 */

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "logging.h"

/*
 * A custom logging level prefix function for use with the stderr
 * logger. See main() for details.
 */
const char *
custom_level_prefix(int priority)
{
    /*
     * priority is a logical OR of the log level with the log
     * facility. We want just the level. The non-standard (yet
     * widely-supported) LOG_PRI macro can do that for us.
     */
    switch (LOG_PRI(priority)) {
    case LOG_DEBUG:
        return "debug msg: ";
    case LOG_INFO:
        return "informational msg: ";
    case LOG_NOTICE:
        return "notice: ";
    case LOG_WARNING:
        return "waring message: ";
    case LOG_ERR:
        return "error message: ";
    case LOG_CRIT:
        return "critical error message: ";
    case LOG_ALERT:
        return "alert: ";
    case LOG_EMERG:
        return "emergency message: ";
    default:

        /* Just for good measure. */
        return "unknown msg: ";
    }
}

int
main(int argc, char *argv[])
{
   /*
    * Function pointers to store the current setlogmask function and
    * syslog function.
    */
    setlogmask_fun logmask;
    syslog_fun log;

    /*
     * Get function handles for the stderr logging functions. We don't
     * use the vsyslog(3)-equivalent function in this example program,
     * so its pointer argument is NULL.
     */
    get_stderr_logger(&log, 0, &logmask);

    /*
     * We set the logging mask just like we do with syslog. Let's log
     * everything up to and including LOG_NOTICE.
     */
    logmask(LOG_UPTO(LOG_NOTICE));

    /*
     * Instead of calling syslog(3), call the stderr logger via the
     * log function pointer we set up earlier. The stderr logger has
     * the same interface (parameters, semantics, etc.) as syslog(3).
     *
     * Note that some syslog(3) concepts, such as the logging
     * facility, have no effect on the stderr logger. You can specify
     * them if you like, for compatibility with syslog(3), but they're
     * ignored.
     */
    log(LOG_NOTICE, "This message will go to stderr.");
    log(LOG_DEBUG, "This message will not, as LOG_DEBUG is masked.");

    /*
     * You can change the logging mask at any time.
     */
    logmask(LOG_UPTO(LOG_DEBUG));
    log(LOG_DEBUG, "This message will also go to stderr.");

    /*
     * The stderr logger can use format strings, just like syslog(3)
     * does.
     */
    log(LOG_WARNING, "This message prints a %d and a %s.", 7, "string");

    /*
     * It also supports the special "%m" format, which prints an error
     * message corresponding to contents of errno.
     */

    /* Double-close a file on purpose. */
    char tmpfile_template[] = "/tmp/tmpXXXXXXlogtest";
    int fd = mkstemps(tmpfile_template, strlen("logtest"));
    close(fd);
    close(fd); /* error! */
    log(LOG_ERR, "Just tried to close fd %d twice: %m", fd);
    unlink(tmpfile_template);

    /*
     * The stderr logger prints a logging prefix that corresponds to
     * the message's log level, by default. It uses prefixes of the
     * form "LOG_DEBUG: ", "LOG_NOTICE: ", etc.
     *
     * You can override this function by providing your own logging
     * prefix function. The function should take one argument, the
     * message priority, extract the log level from that priority
     * (using, e.g., the LOG_PRI macro), and return a const,
     * NULL-terminated string for use as a prefix.
     *
     * Here we install a custom logging prefix function, defined
     * above. Save the original function so we can restore it later.
     */
    level_prefix_fun builtin =
        set_stderr_level_prefix_fun(custom_level_prefix);
    log(LOG_ERR,
        "This error message will have a different prefix than the others.");
    log(LOG_NOTICE, "So will this notice.");
    
    /* Restore the original. */
    set_stderr_level_prefix_fun(builtin);
    log(LOG_NOTICE, "Using the built-in level prefix function again.");

    /*
     * We can also use the standard syslog(3) logger via the same
     * interface, and we can switch back and forth at any time.
     *
     * N.B.: be sure to see logging.h for details on precautions when
     * using the logging interface with threads.
     */

    /* Get the syslog logger. */
    get_syslog_logger(&log, 0, &logmask);

    /*
     * N.B.: the log mask we set up for the stderr logger does *not*
     * apply to the syslog logger. They are separate.
     */
    logmask(LOG_UPTO(LOG_NOTICE));
    log(LOG_NOTICE, "This message will show up in the syslog (syslog config depending), but not on stderr.");

    /* Back to the stderr logger. */
    get_stderr_logger(&log, 0, &logmask);
    log(LOG_DEBUG, "Back to stderr logging.");

    log(LOG_NOTICE, "All done!");
    
    return 0;
}
