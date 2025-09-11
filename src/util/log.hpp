#pragma once

#include <cassert>
#include <cctype>

#include "flags.hpp"
#include "nowide/iostream.hpp"

// borrowed from chromium
class VoidifyStream {
public:
    VoidifyStream() = default;

    explicit VoidifyStream(bool) { }

    // Binary & has lower precedence than << but higher than ?:
    void operator& (std::ostream &) { }
};

// borrowed from chromium
extern std::ostream *g_swallow_stream;
#define EAT_CHECK_STREAM_PARAMS(expr) true ? (void)0 : ::VoidifyStream(expr) & (*g_swallow_stream)

namespace Silver {
    struct Logger;
    struct LogMessageStream;

    namespace {
        extern Logger *global_logger;

        // https://stackoverflow.com/a/46455079
        class NullStream: public std::ostream {
            class NullBuffer: public std::streambuf {
            public:
                int overflow(int c) { return c; }
            } m_nb;

        public:
            NullStream() :
                std::ostream(&m_nb) { }
        };
    } // namespace

    namespace Colors {
#define AnsiColor(str) \
    "\x1B" \
    "[" str "m"
        constexpr const char *fgBlack   = AnsiColor("30");
        constexpr const char *bgBlack   = AnsiColor("40");
        constexpr const char *fgRed     = AnsiColor("31");
        constexpr const char *bgRed     = AnsiColor("41");
        constexpr const char *fgGreen   = AnsiColor("32");
        constexpr const char *bgGreen   = AnsiColor("42");
        constexpr const char *fgYellow  = AnsiColor("33");
        constexpr const char *bgYellow  = AnsiColor("43");
        constexpr const char *fgBlue    = AnsiColor("34");
        constexpr const char *bgBlue    = AnsiColor("44");
        constexpr const char *fgMagenta = AnsiColor("35");
        constexpr const char *bgMagenta = AnsiColor("45");
        constexpr const char *fgCyan    = AnsiColor("36");
        constexpr const char *bgCyan    = AnsiColor("46");
        constexpr const char *fgWhite   = AnsiColor("37");
        constexpr const char *bgWhite   = AnsiColor("47");
        constexpr const char *fgDefault = AnsiColor("39");
        constexpr const char *bgDefault = AnsiColor("49");
        constexpr const char *fgReset   = AnsiColor("0");
        constexpr const char *bgReset   = AnsiColor("0");
#undef AnsiColor
    } // namespace Colors

    struct Logger {
        enum LogLevel { Debug, Info, Warn, Error, Fatal };

        static constexpr const char *logLevelNames[] = {"Debug", "Info", "Warn", "Error", "Fatal"};

        static const char           *getLogLevelName(LogLevel level) {
            assert(level >= LogLevel::Debug && level < LogLevel::Fatal);
            return logLevelNames[static_cast<int>(level)];
        }

        Logger(LogLevel minLevel = LogLevel::Warn) :
            minLevel(minLevel) { }

        void setLogLevel(LogLevel minLevel) { this->minLevel = minLevel; }

        void setLogLevel(const std::string &levelStr) {
            auto caseInsEquals = [](const std::string &a, const std::string &b) {
                auto caseInsentiveCompare = [](char a, char b) {
                    return std::tolower(static_cast<unsigned char>(a)) == std::tolower(static_cast<unsigned char>(b));
                };
                return std::equal(a.begin(), a.end(), b.begin(), b.end(), caseInsentiveCompare);
            };

            if(caseInsEquals(levelStr, "Debug")) {
                setLogLevel(LogLevel::Debug);
            } else if(caseInsEquals(levelStr, "Info")) {
                setLogLevel(LogLevel::Info);
            } else if(caseInsEquals(levelStr, "Warn")) {
                setLogLevel(LogLevel::Warn);
            } else if(caseInsEquals(levelStr, "Error")) {
                setLogLevel(LogLevel::Error);
            } else if(caseInsEquals(levelStr, "Fatal")) {
                setLogLevel(LogLevel::Fatal);
            } else {
                this->log(LogLevel::Error, "Logger") << "Invalid log level: " << levelStr;
            }

            this->log(LogLevel::Info, "Logger") << "Set log level to " << getLogLevelName(this->minLevel) << std::endl;
        }

        std::ostream &log(LogLevel level, const char *id) {
            if(level < this->minLevel) {
                return nullStream;
            }

            switch(level) {
            case LogLevel::Debug: nowide::cout << Colors::fgGreen << "[" << "Debug"; break;
            case LogLevel::Info:  nowide::cout << Colors::fgBlue << "[" << "Info"; break;
            case LogLevel::Warn:  nowide::cout << Colors::fgYellow << "[" << "Warn"; break;
            case LogLevel::Error: nowide::cout << Colors::fgRed << "[" << "Error"; break;
            case LogLevel::Fatal: nowide::cout << Colors::fgMagenta << "[" << "Fatal"; break;
            default:              unreachable();
            }

            return nowide::cout << "] " << id << Colors::fgDefault << ": ";
        }

    private:
        LogLevel   minLevel;
        NullStream nullStream;
    };

    Logger &getLogger();

    struct LogMessageStream {
        const char   *id;
        std::ostream &_stream;

        LogMessageStream(Logger::LogLevel level, const char *id) :
            id(id), _stream(getLogger().log(level, id)) { };

        ~LogMessageStream() { stream() << std::endl; }

        std::ostream &stream() const { return _stream; }

        template<typename T>
        std::ostream &operator<< (T &&streamed_type) {
            return stream() << streamed_type;
        }
    };

    // stripped-down version of chromium's CheckError
    struct DebugCheckError: public LogMessageStream {
        DebugCheckError(const char *expr) :
            LogMessageStream(Silver::Logger::LogLevel::Fatal, "DebugCheck") {
            stream() << "Check failed: " << expr;
        };

        ~DebugCheckError() {
            // crash
            assert(false);
        }
    };
} // namespace Silver

#define LogDebug(id) Silver::LogMessageStream(Silver::Logger::LogLevel::Debug, id)
#define LogInfo(id)  Silver::LogMessageStream(Silver::Logger::LogLevel::Info, id)
#define LogWarn(id)  Silver::LogMessageStream(Silver::Logger::LogLevel::Warn, id)
#define LogError(id) Silver::LogMessageStream(Silver::Logger::LogLevel::Error, id)
#define LogFatal(id) Silver::LogMessageStream(Silver::Logger::LogLevel::Fatal, id)

#if defined(_DEBUG)
  // borrowed from chromium
#define DebugCheck(expr) \
    switch(0) \
    case 0: \
    default: \
        if((expr)) { \
        } else
Silver::DebugCheckError(#expr)
#else
#define DebugCheck(expr) EAT_CHECK_STREAM_PARAMS(!(expr))
#endif
