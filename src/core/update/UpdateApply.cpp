/* DRQ Miner — download, verify SHA256, replace binary. */

#include "core/update/UpdateApply.h"

#include "base/io/log/Log.h"
#include "base/io/log/Tags.h"
#include "base/kernel/Platform.h"
#include "base/kernel/Process.h"
#include "version.h"


#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>


#ifdef XMRIG_OS_WIN
#   include <windows.h>
#   include <winhttp.h>
#   include <wincrypt.h>
#   pragma comment(lib, "winhttp.lib")
#   pragma comment(lib, "advapi32.lib")
#else
#   include <unistd.h>
#endif


namespace xmrig {


static std::string toLowerHex(const std::string &in)
{
    std::string out = in;
    for (char &c : out) {
        if (c >= 'A' && c <= 'F') {
            c = static_cast<char>(c - 'A' + 'a');
        }
    }
    return out;
}


#ifdef XMRIG_OS_WIN
static bool sha256FileWin(const std::string &path, std::string &outHex)
{
    std::ifstream in(path, std::ios::binary);
    if (!in) {
        return false;
    }

    std::vector<char> data((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    if (data.empty()) {
        return false;
    }

    HCRYPTPROV prov = 0;
    HCRYPTHASH hash = 0;
    if (!CryptAcquireContext(&prov, nullptr, nullptr, PROV_RSA_AES, CRYPT_VERIFYCONTEXT)) {
        return false;
    }
    if (!CryptCreateHash(prov, CALG_SHA_256, 0, 0, &hash)) {
        CryptReleaseContext(prov, 0);
        return false;
    }
    if (!CryptHashData(hash, reinterpret_cast<const BYTE *>(data.data()), static_cast<DWORD>(data.size()), 0)) {
        CryptDestroyHash(hash);
        CryptReleaseContext(prov, 0);
        return false;
    }

    DWORD len = 32;
    BYTE digest[32];
    if (!CryptGetHashParam(hash, HP_HASHVAL, digest, &len, 0)) {
        CryptDestroyHash(hash);
        CryptReleaseContext(prov, 0);
        return false;
    }

    CryptDestroyHash(hash);
    CryptReleaseContext(prov, 0);

    char buf[65] = {};
    for (DWORD i = 0; i < len; ++i) {
        snprintf(buf + i * 2, 3, "%02x", digest[i]);
    }
    outHex = buf;
    return true;
}


static bool downloadUrlWin(const char *url, const std::string &dest)
{
    std::wstring wurl;
    {
        const int n = MultiByteToWideChar(CP_UTF8, 0, url, -1, nullptr, 0);
        if (n <= 0) {
            return false;
        }
        std::vector<wchar_t> buf(static_cast<size_t>(n));
        MultiByteToWideChar(CP_UTF8, 0, url, -1, buf.data(), n);
        wurl.assign(buf.data());
    }

    URL_COMPONENTS parts{};
    parts.dwStructSize     = sizeof(parts);
    wchar_t host[256]      = {};
    wchar_t path[2048]     = {};
    parts.lpszHostName     = host;
    parts.dwHostNameLength = static_cast<DWORD>(std::size(host));
    parts.lpszUrlPath      = path;
    parts.dwUrlPathLength  = static_cast<DWORD>(std::size(path));

    if (!WinHttpCrackUrl(wurl.c_str(), 0, 0, &parts)) {
        return false;
    }

    const bool tls = parts.nScheme == INTERNET_SCHEME_HTTPS;
    HINTERNET session = WinHttpOpen(L"DRQMiner-Update/1.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, nullptr, nullptr, 0);
    if (!session) {
        return false;
    }

    HINTERNET connect = WinHttpConnect(session, host, parts.nPort ? parts.nPort : (tls ? 443 : 80), 0);
    if (!connect) {
        WinHttpCloseHandle(session);
        return false;
    }

    DWORD flags = tls ? WINHTTP_FLAG_SECURE : 0;
    HINTERNET request = WinHttpOpenRequest(connect, L"GET", path, nullptr, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, flags);
    if (!request) {
        WinHttpCloseHandle(connect);
        WinHttpCloseHandle(session);
        return false;
    }

    if (!WinHttpSendRequest(request, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0) ||
        !WinHttpReceiveResponse(request, nullptr)) {
        WinHttpCloseHandle(request);
        WinHttpCloseHandle(connect);
        WinHttpCloseHandle(session);
        return false;
    }

    std::ofstream out(dest, std::ios::binary);
    if (!out) {
        WinHttpCloseHandle(request);
        WinHttpCloseHandle(connect);
        WinHttpCloseHandle(session);
        return false;
    }

    DWORD available = 0;
    do {
        if (!WinHttpQueryDataAvailable(request, &available)) {
            break;
        }
        if (available == 0) {
            break;
        }
        std::vector<char> chunk(available);
        DWORD read = 0;
        if (!WinHttpReadData(request, chunk.data(), available, &read)) {
            break;
        }
        out.write(chunk.data(), read);
    } while (available > 0);

    WinHttpCloseHandle(request);
    WinHttpCloseHandle(connect);
    WinHttpCloseHandle(session);

    return out.good();
}


static bool spawnReplaceHelper(const std::string &archivePath, const std::string &exeDir)
{
    const String helper = Process::location(Process::ExeLocation, "apply-update.bat");
    std::ofstream bat(helper.data());
    if (!bat) {
        return false;
    }

    bat << "@echo off\n";
    bat << "timeout /t 2 /nobreak >nul\n";
    bat << "cd /d \"" << exeDir << "\"\n";
    bat << "powershell -NoProfile -Command \"Expand-Archive -Force -Path '" << archivePath << "' -DestinationPath '" << exeDir << "\\.update_staging'\"\n";
    bat << "if exist \".update_staging\\DRQMiner-Release.exe\" copy /Y \".update_staging\\DRQMiner-Release.exe\" \"DRQMiner-Release.exe\"\n";
    bat << "if exist \".update_staging\\drqminer.exe\" copy /Y \".update_staging\\drqminer.exe\" \"DRQMiner-Release.exe\"\n";
    bat << "rd /s /q \".update_staging\" 2>nul\n";
    bat << "del /f \"" << archivePath << "\" 2>nul\n";
    bat << "del /f \"%~f0\" 2>nul\n";
    bat.close();

    STARTUPINFOA si{};
    PROCESS_INFORMATION pi{};
    si.cb = sizeof(si);
    const std::string cmdStr = std::string("cmd /c start \"\" \"") + helper.data() + "\"";
    std::vector<char> cmd(cmdStr.begin(), cmdStr.end());
    cmd.push_back('\0');
    if (!CreateProcessA(nullptr, cmd.data(), nullptr, nullptr, FALSE, DETACHED_PROCESS | CREATE_NO_WINDOW, nullptr, exeDir.c_str(), &si, &pi)) {
        return false;
    }
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    return true;
}
#endif


#ifndef XMRIG_OS_WIN
static bool sha256FilePosix(const std::string &path, std::string &outHex)
{
    std::string cmd = "sha256sum \"" + path + "\" 2>/dev/null";
    FILE *fp = popen(cmd.c_str(), "r");
    if (!fp) {
        return false;
    }
    char buf[128] = {};
    if (!fgets(buf, sizeof(buf), fp)) {
        pclose(fp);
        return false;
    }
    pclose(fp);
    outHex.assign(buf, strcspn(buf, " \t\n"));
    return outHex.size() == 64;
}


static bool downloadUrlPosix(const char *url, const std::string &dest)
{
    std::string cmd = std::string("curl -fsSL -o \"") + dest + "\" \"" + url + "\"";
    return system(cmd.c_str()) == 0;
}


static bool linuxReplace(const std::string &archivePath, const std::string &exePath)
{
    const auto dir = exePath.substr(0, exePath.find_last_of("/\\"));
    std::string staging = dir + "/.update_staging";
    std::string cmd = "mkdir -p \"" + staging + "\" && tar xzf \"" + archivePath + "\" -C \"" + staging + "\"";
    if (system(cmd.c_str()) != 0) {
        return false;
    }

    std::string newBin = staging + "/drqminer";
    if (access(newBin.c_str(), X_OK) != 0) {
        newBin = staging + "/DRQMiner-Release";
    }
    if (access(newBin.c_str(), X_OK) != 0) {
        return false;
    }

    std::string backup = exePath + ".bak";
    rename(exePath.c_str(), backup.c_str());
    if (rename(newBin.c_str(), exePath.c_str()) != 0) {
        rename(backup.c_str(), exePath.c_str());
        return false;
    }

    unlink(archivePath.c_str());
    return true;
}
#endif


void UpdateApply::run(const char *url, const char *expectedSha256, const char *version)
{
    if (!url || !expectedSha256) {
        return;
    }

    LOG_NOTICE("%s " CYAN_BOLD("applying update") " to %s ...", Tags::network(), version ? version : "newer");

    const String exePath = Process::exepath();
    const std::string exeDir = exePath.data();
    const size_t slash = exeDir.find_last_of("/\\");
    const std::string workDir = slash == std::string::npos ? "." : exeDir.substr(0, slash);

#ifdef XMRIG_OS_WIN
    const std::string dest = workDir + "\\drq-update.zip";
    if (!downloadUrlWin(url, dest)) {
        LOG_ERR("%s " RED("update download failed"), Tags::network());
        return;
    }

    std::string hash;
    if (!sha256FileWin(dest, hash) || toLowerHex(hash) != toLowerHex(expectedSha256)) {
        LOG_ERR("%s " RED("update SHA256 mismatch"), Tags::network());
        DeleteFileA(dest.c_str());
        return;
    }

    LOG_NOTICE("%s " GREEN_BOLD("update verified") " — scheduling replace after exit", Tags::network());
    if (!spawnReplaceHelper(dest, workDir)) {
        LOG_ERR("%s " RED("failed to spawn update helper"), Tags::network());
        return;
    }

    std::exit(0);
#else
    const std::string dest = workDir + "/drq-update.tgz";
    if (!downloadUrlPosix(url, dest)) {
        LOG_ERR("%s " RED("update download failed"), Tags::network());
        return;
    }

    std::string hash;
    if (!sha256FilePosix(dest, hash) || toLowerHex(hash) != toLowerHex(expectedSha256)) {
        LOG_ERR("%s " RED("update SHA256 mismatch"), Tags::network());
        unlink(dest.c_str());
        return;
    }

    if (!linuxReplace(dest, exePath.data())) {
        LOG_ERR("%s " RED("update replace failed"), Tags::network());
        return;
    }

    LOG_NOTICE("%s " GREEN_BOLD("update installed") " — restart miner", Tags::network());
    std::exit(0);
#endif
}


} // namespace xmrig
