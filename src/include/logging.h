/*
 * Copyright (C) 2009 - Luca Bigliardi
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

/*
 * These new classes enhance logging in FreeJ while keeping 99% compatibility
 * with the old system. The basic idea is to allow, for each object, separate
 * loglevels and separate destinations for logging.
 *
 * Each object which needs these functionality should inherit from Loggable and
 * use its log() and vlog() methods to print messages. For instances of
 * Loggable subclasses it's possible to specify a LogLevel and register an
 * Logger object to process logs. If no Logger is registered then a system wide
 * logger, GlobalLogger, is used.
 *
 * Each class which wants to provide a logging service should inherit from
 * Logger, implementing printlog() and vprintlog() methods.
 *
 * GlobalLogger is a static, catch-all logger. By default its output goes on a
 * filedescriptor, but it's possible to register a Logger on it.
 *
 * For backward compatibility old logging functions from jutils have been
 * reimplemented and a ConsoleController is allowed inside GlobalLogger.
 *
 */

#ifndef __LOGGING_H__
#define __LOGGING_H__

#include <exceptions.h>
#include <console_ctrl.h>

#define MAX_LOG_MSG 1024

enum LogLevel { // ordered by increasing verbosity
  ERROR,
  WARNING,
  NOTICE,
  INFO,
  DEBUG
};

// Interface to implement for providing a logging service
class Logger {
  public:
    virtual int printlog(LogLevel level, const char *format, ...) = 0;
    virtual int vprintlog(LogLevel level, const char *format, va_list arg) = 0;
};

// This class has to be inherited by all the classes which expect to print on a
// Logger
class Loggable {
  public:
    class Error : public FreejError {
      public:
        Error(const std::string& msg, int rv)
          : FreejError(msg, rv) { }
    };

    Loggable();
    virtual ~Loggable();
    bool register_logger(Logger *l);
    bool unregister_logger(Logger *l);
    LogLevel get_loglevel() { return loglevel_; }
    void set_loglevel(LogLevel level) { loglevel_ = level; }

  protected:
    int log(LogLevel level, const char *format, ...);
    int vlog(LogLevel level, const char *format, va_list arg);

  private:
    Logger *logger_;
    LogLevel loglevel_;
    pthread_mutex_t logger_mutex_;
};

// Static log system. Allowing ConsoleController for backward compatibility.
class GlobalLogger {
  public:
    static int printlog(LogLevel level, const char *format, ...);
    static int vprintlog(LogLevel level, const char *format, va_list arg);
    static bool register_logger(Logger *l);
    static bool unregister_logger(Logger *l);
    static LogLevel get_loglevel();
    static void set_loglevel(LogLevel level);
    static void set_console(ConsoleController *c);
  private:
    static LogLevel loglevel_;
    static Logger *logger_;
    static pthread_mutex_t logger_mutex_;
    static ConsoleController *console_; // accessed under logger_mutex_
    static char logbuf_[];
};

// These are for backward compatibility:
#define MAX_ERR_MSG MAX_LOG_MSG
void set_debug(int lev);
int get_debug();
void set_console(ConsoleController *c);
void error(const char *format, ...);
void warning(const char *format, ...);
void notice(const char *format, ...);
void act(const char *format, ...);
void func(const char *format, ...);

#endif
