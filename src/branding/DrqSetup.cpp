#include "branding/DrqSetup.h"

#include <cctype>
#include <cstdio>
#include <cstring>
#include <string>

#ifdef XMRIG_OS_WIN
#   include <io.h>
#   include <windows.h>
#else
#   include <unistd.h>
#   include <sys/stat.h>
#endif

#include "3rdparty/rapidjson/document.h"
#include "base/io/json/Json.h"
#include "base/io/json/JsonChain.h"
#include "base/kernel/Process.h"
#include "branding/DrqBanner.h"
#include "core/config/Config.h"
#include "version.h"


namespace xmrig {


namespace {


struct Preset
{
    const char *label;
    const char *algo;
    const char *pool;
};


static const Preset kPresets[] = {
    { "VerusHash (CPU)",           "verushash",       "usw.vipor.net:5040" },
    { "DERO AstroBWT v3",          "astrobwtv3/dero", "dero.rabidmining.com:10300" },
    { "Neuromorph (Cereblix)",     "nm/1",            "stratum.cereblix.com:3333" },
    { "RandomX (Monero)",          "rx/0",            "pool.supportxmr.com:3333" },
    { "Ghostrider (Raptoreum)",    "ghostrider",      "stratum+tcp://rtm.suprnova.cc:6277" },
};


static void trim(std::string &s)
{
    while (!s.empty() && std::isspace(static_cast<unsigned char>(s.back()))) {
        s.pop_back();
    }

    size_t start = 0;
    while (start < s.size() && std::isspace(static_cast<unsigned char>(s[start]))) {
        ++start;
    }

    if (start > 0) {
        s.erase(0, start);
    }
}


static bool isInteractive()
{
#ifdef XMRIG_OS_WIN
    return _isatty(_fileno(stdin)) != 0;
#else
    return isatty(fileno(stdin)) != 0;
#endif
}


static bool hasNetworkCli(const Process &process)
{
    const Arguments &args = process.arguments();

    for (int i = 1; i < args.argc(); ++i) {
        const char *arg = args.argv()[i];

        if (strcmp(arg, "-o") == 0 || strcmp(arg, "--url") == 0) {
            return true;
        }

        if (strncmp(arg, "-o", 2) == 0 && arg[2] != '\0') {
            return true;
        }

        if (strncmp(arg, "--url=", 6) == 0 && arg[6] != '\0') {
            return true;
        }
    }

    return false;
}


static bool localConfigValid(const Process &process)
{
    JsonChain chain;
    chain.addFile(Process::location(Process::DataLocation, "config.json"));

    Config config;
    return config.read(chain, chain.fileName());
}


static bool readLine(const char *prompt, std::string &out, const char *defaultValue = nullptr)
{
    printf("%s", prompt);
    fflush(stdout);

    char buf[512] = {};
    if (!fgets(buf, sizeof(buf), stdin)) {
        return false;
    }

    out = buf;
    trim(out);

    if (out.empty() && defaultValue) {
        out = defaultValue;
    }

    return true;
}


static bool confirmOverwrite(const char *path)
{
    printf("\n\"%s\" already exists.\n", path);
    printf("Overwrite? [y/N]: ");
    fflush(stdout);

    char buf[16] = {};
    if (!fgets(buf, sizeof(buf), stdin)) {
        return false;
    }

    return buf[0] == 'y' || buf[0] == 'Y';
}


static bool pickPreset(std::string &algo, std::string &pool)
{
    DrqBanner::print();
    printf("\n" APP_NAME " setup — choose what to mine\n");
    printf("====================================\n\n");

    const size_t count = sizeof(kPresets) / sizeof(kPresets[0]);
    for (size_t i = 0; i < count; ++i) {
        printf("  %zu) %s\n", i + 1, kPresets[i].label);
        printf("     algo: %-18s pool: %s\n", kPresets[i].algo, kPresets[i].pool);
    }
    printf("  %zu) Custom (enter algo and pool yourself)\n\n", count + 1);

    std::string choice;
    if (!readLine("Choice [1]: ", choice, "1")) {
        return false;
    }

    size_t index = 0;
    if (!choice.empty()) {
        char *end = nullptr;
        const unsigned long n = strtoul(choice.c_str(), &end, 10);
        if (end == choice.c_str() || n < 1 || n > count + 1) {
            printf("Invalid choice.\n");
            return false;
        }
        index = static_cast<size_t>(n - 1);
    }

    if (index < count) {
        algo = kPresets[index].algo;
        pool = kPresets[index].pool;

        std::string customPool;
        char prompt[256];
        snprintf(prompt, sizeof(prompt), "Pool [%s]: ", pool.c_str());
        if (!readLine(prompt, customPool)) {
            return false;
        }
        if (!customPool.empty()) {
            pool = customPool;
        }

        return true;
    }

    if (!readLine("Algorithm (e.g. verushash): ", algo) || algo.empty()) {
        printf("Algorithm is required.\n");
        return false;
    }

    if (!readLine("Pool host:port: ", pool) || pool.empty()) {
        printf("Pool is required.\n");
        return false;
    }

    return true;
}


static bool buildUserString(std::string &user)
{
    std::string wallet;
    if (!readLine("Wallet / pool username: ", wallet) || wallet.empty()) {
        printf("Wallet is required.\n");
        return false;
    }

    std::string worker;
    if (!readLine("Worker name (optional, appended as wallet.worker): ", worker)) {
        return false;
    }

    trim(worker);
    user = wallet;
    if (!worker.empty() && user.find('.') == std::string::npos) {
        user += '.';
        user += worker;
    }

    return true;
}


static bool writeConfigFile(const char *path, const std::string &algo, const std::string &pool, const std::string &user)
{
    using namespace rapidjson;

    Document doc(kObjectType);
    auto &alloc = doc.GetAllocator();

    Value cpu(kObjectType);
    cpu.AddMember("enabled", true, alloc);
    cpu.AddMember("huge-pages", true, alloc);
    cpu.AddMember("yield", true, alloc);
    cpu.AddMember("max-threads-hint", 100, alloc);
    cpu.AddMember("asm", true, alloc);
    doc.AddMember("autosave", true, alloc);
    doc.AddMember("background", false, alloc);
    doc.AddMember("colors", true, alloc);
    doc.AddMember("title", true, alloc);
    doc.AddMember("cpu", cpu, alloc);
    doc.AddMember("donate-level", 0.5, alloc);
    doc.AddMember("donate-over-proxy", 1, alloc);
    doc.AddMember("print-time", 60, alloc);
    doc.AddMember("retries", 5, alloc);
    doc.AddMember("retry-pause", 5, alloc);

    Value poolObj(kObjectType);
    poolObj.AddMember("url", Value(pool.c_str(), static_cast<SizeType>(pool.size()), alloc), alloc);
    poolObj.AddMember("user", Value(user.c_str(), static_cast<SizeType>(user.size()), alloc), alloc);
    poolObj.AddMember("pass", "x", alloc);
    poolObj.AddMember("enabled", true, alloc);
    poolObj.AddMember("tls", false, alloc);
    if (!algo.empty()) {
        poolObj.AddMember("algo", Value(algo.c_str(), static_cast<SizeType>(algo.size()), alloc), alloc);
    }

    Value pools(kArrayType);
    pools.PushBack(poolObj, alloc);
    doc.AddMember("pools", pools, alloc);

    if (!Json::save(path, doc)) {
        printf("Failed to write \"%s\".\n", path);
        return false;
    }

    JsonChain chain;
    chain.addFile(path);
    Config config;
    if (!config.read(chain, chain.fileName())) {
        printf("Generated config failed validation. Check pool URL and wallet.\n");
        return false;
    }

    return true;
}


static std::string exeBaseName(const Process &process)
{
    const String path = Process::exepath();
    if (path.isNull()) {
        return APP_ID;
    }

    const char *base = strrchr(path, XMRIG_DIR_SEPARATOR[0]);
    return base ? std::string(base + 1) : path.data();
}


static bool writeLaunchScript(const Process &process, const char *configPath)
{
    const String scriptPath = Process::location(Process::DataLocation,
#ifdef XMRIG_OS_WIN
        "start-miner.bat"
#else
        "start-miner.sh"
#endif
    );

    const std::string miner = exeBaseName(process);

    FILE *fp = fopen(scriptPath.data(), "wb");
    if (!fp) {
        printf("Failed to write \"%s\".\n", scriptPath.data());
        return false;
    }

#ifdef XMRIG_OS_WIN
    fprintf(fp, "@echo off\r\n");
    fprintf(fp, "REM DRQ Miner launch script — generated by %s --setup\r\n", miner.c_str());
    fprintf(fp, "REM Re-run \"%s --setup\" to change pool or wallet.\r\n", miner.c_str());
    fprintf(fp, "setlocal\r\n");
    fprintf(fp, "cd /d \"%%~dp0\"\r\n\r\n");
    fprintf(fp, "set \"MINER=%%~dp0%s\"\r\n", miner.c_str());
    fprintf(fp, "if not exist \"%%MINER%%\" set \"MINER=%s\"\r\n\r\n", miner.c_str());
    fprintf(fp, "\"%%MINER%%\" --config=\"%s\"\r\n", configPath);
    fprintf(fp, "if errorlevel 1 pause\r\n");
#else
    fprintf(fp, "#!/bin/sh\n");
    fprintf(fp, "# DRQ Miner launch script — generated by %s --setup\n", miner.c_str());
    fprintf(fp, "cd \"$(dirname \"$0\")\" || exit 1\n");
    fprintf(fp, "exec \"./%s\" --config=\"%s\"\n", miner.c_str(), configPath);
#endif

    fclose(fp);

#ifndef XMRIG_OS_WIN
    chmod(scriptPath.data(), 0755);
#endif

    return true;
}


static void printNonInteractiveHelp(const char *argv0)
{
    fprintf(stderr, "\nNo valid configuration found.\n\n");
    fprintf(stderr, "Quick start (no config file):\n");
    fprintf(stderr, "  %s -a verushash -o POOL:PORT -u WALLET.worker\n\n", argv0);
    fprintf(stderr, "Interactive setup (writes config.json + start-miner script):\n");
    fprintf(stderr, "  %s --setup\n\n", argv0);
#ifdef XMRIG_OS_WIN
    if (isInteractive()) {
        fprintf(stderr, "Press Enter to close...");
        fflush(stderr);
        fgetc(stdin);
    }
#endif
}


static bool stdinAvailable()
{
#ifdef XMRIG_OS_WIN
    if (_isatty(_fileno(stdin)) != 0) {
        return true;
    }

    const HANDLE h = GetStdHandle(STD_INPUT_HANDLE);
    if (h == INVALID_HANDLE_VALUE) {
        return false;
    }

    const DWORD type = GetFileType(h);
    return type == FILE_TYPE_PIPE || type == FILE_TYPE_DISK;
#else
    return isatty(fileno(stdin)) || !feof(stdin);
#endif
}


static int runWizard(const Process &process, bool force)
{
    if (!stdinAvailable()) {
        printNonInteractiveHelp(process.arguments().argv()[0]);
        return 2;
    }

    if (!force && !isInteractive()) {
        printNonInteractiveHelp(process.arguments().argv()[0]);
        return 2;
    }

    const String configPath = Process::location(Process::DataLocation, "config.json");

    if (!force && localConfigValid(process)) {
        return 0;
    }

    if (force && localConfigValid(process) && !confirmOverwrite(configPath.data())) {
        return 0;
    }

    std::string algo;
    std::string pool;
    if (!pickPreset(algo, pool)) {
        return 1;
    }

    std::string user;
    if (!buildUserString(user)) {
        return 1;
    }

    printf("\nWriting configuration...\n");

    if (!writeConfigFile(configPath.data(), algo, pool, user)) {
        return 2;
    }

    if (!writeLaunchScript(process, configPath.data())) {
        return 2;
    }

    printf("\nCreated:\n");
    printf("  %s\n", configPath.data());
#ifdef XMRIG_OS_WIN
    printf("  %s\n", Process::location(Process::DataLocation, "start-miner.bat").data());
#else
    printf("  %s\n", Process::location(Process::DataLocation, "start-miner.sh").data());
#endif
    printf("\nNext time you can double-click the start-miner script, or run this executable again.\n\n");

    return 0;
}


} // namespace


int DrqSetup::run(const Process &process, Mode mode)
{
    if (mode == Auto) {
        if (hasNetworkCli(process) || localConfigValid(process)) {
            return 0;
        }

        const int rc = runWizard(process, false);
        return rc;
    }

    return runWizard(process, true);
}


} // namespace xmrig
