#include "stdafx.h"

#include "MuLogger.h"

#include <array>
#include <mutex>
#include <unordered_map>
#include <vector>

#include <spdlog/sinks/dist_sink.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

namespace mu::log
{
namespace
{
constexpr std::array<const char*, 11> kLoggerNames = {"core",  "network",  "render", "data",     "gameplay", "ui",
                                                      "audio", "platform", "dotnet", "gameshop", "scenes"};

struct LoggerState
{
    std::mutex mutex;
    std::unordered_map<std::string, std::shared_ptr<spdlog::logger>> loggers;
    std::shared_ptr<spdlog::sinks::dist_sink_mt> sink = std::make_shared<spdlog::sinks::dist_sink_mt>();
    bool sinksInitialized = false;
    bool explicitlyInitialized = false;
    std::string sinkFailureReason;
};

LoggerState& State()
{
    static LoggerState state;
    return state;
}

std::shared_ptr<spdlog::logger> CreateLogger(LoggerState& state, const std::string& name)
{
    auto logger = std::make_shared<spdlog::logger>(name, state.sink);
    logger->set_level(spdlog::level::info);
    logger->flush_on(spdlog::level::err);
    spdlog::register_logger(logger);
    state.loggers.emplace(name, logger);
    return logger;
}

// Returns false when the log file could not be opened and the console sink was
// used instead. Not being able to write a log must never take the client down:
// macOS App Translocation mounts a quarantined unsigned .app read-only, and
// installed locations (e.g. /Applications, Program Files) can be read-only too.
bool ConfigureSinks(LoggerState& state, const std::filesystem::path& directory)
{
    std::vector<spdlog::sink_ptr> sinks;
    bool fileLogging = true;
    try
    {
        sinks.push_back(std::make_shared<spdlog::sinks::rotating_file_sink_mt>((directory / "MuError.log").string(),
                                                                               5 * 1024 * 1024, 3));
    }
    catch (const spdlog::spdlog_ex& error)
    {
        fileLogging = false;
        state.sinkFailureReason = error.what();
        sinks.push_back(std::make_shared<spdlog::sinks::stderr_color_sink_mt>());
    }

    state.sink->flush();
    state.sink->set_sinks(std::move(sinks));
    state.sinksInitialized = true;
    return fileLogging;
}
} // namespace

void Init(const std::filesystem::path& logDirectory)
{
    LoggerState& state = State();
    std::lock_guard lock(state.mutex);
    if (state.explicitlyInitialized)
        return;

    const std::filesystem::path directory = logDirectory.empty() ? std::filesystem::current_path() : logDirectory;
    std::error_code error;
    std::filesystem::create_directories(directory, error);

    const bool fileLogging = ConfigureSinks(state, directory);

    for (const char* name : kLoggerNames)
    {
        if (!state.loggers.contains(name))
            CreateLogger(state, name);
    }

    if (!fileLogging)
    {
        if (const auto core = state.loggers.find("core"); core != state.loggers.end())
            core->second->warn("File logging disabled ({}); logs go to the console and '{}' stays untouched",
                               state.sinkFailureReason, (directory / "MuError.log").string());
    }

    state.explicitlyInitialized = true;
}

void Shutdown()
{
    LoggerState& state = State();
    std::lock_guard lock(state.mutex);
    for (auto& entry : state.loggers)
        entry.second->flush();
    state.loggers.clear();
    state.sink->set_sinks({});
    spdlog::shutdown();
    state.sinksInitialized = false;
    state.explicitlyInitialized = false;
}

std::shared_ptr<spdlog::logger> Get(const std::string& name)
{
    LoggerState& state = State();
    std::lock_guard lock(state.mutex);
    if (!state.sinksInitialized)
        ConfigureSinks(state, std::filesystem::current_path());

    const auto found = state.loggers.find(name);
    return found != state.loggers.end() ? found->second : CreateLogger(state, name);
}
} // namespace mu::log
