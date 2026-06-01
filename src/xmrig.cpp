/* XMRig
 * Copyright (c) 2018-2021 SChernykh   <https://github.com/SChernykh>
 * Copyright (c) 2016-2021 XMRig       <https://github.com/xmrig>, <support@xmrig.com>
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "App.h"
#include "base/kernel/Entry.h"
#include "base/kernel/Process.h"

#ifdef XMRIG_ALGO_VERUSHASH
#   include "crypto/verushash/verus_hash.h"
#endif


int main(int argc, char **argv)
{
    using namespace xmrig;

#ifdef XMRIG_ALGO_VERUSHASH
    // Optional CLI self-test: computes a VerusHash v2 of a fixed buffer and exits.
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--verushash-self-test") == 0) {
            static const char kTestInput[] =
                "Test1234Test1234Test1234Test1234"
                "Test1234Test1234Test1234Test1234"
                "Test1234Test1234Test1234Test1234";

            CVerusHashV2::init();
            unsigned char hash[32] = { 0 };
            verus_hash_v2(hash, kTestInput, sizeof(kTestInput) - 1);

            printf("VerusHash v2 self-test input: \"%s\"\n", kTestInput);
            printf("VerusHash v2 self-test hash : ");
            for (size_t j = 0; j < sizeof(hash); ++j) {
                printf("%02x", static_cast<unsigned int>(hash[j]));
            }
            printf("\n");

            return 0;
        }
    }
#endif

    Process process(argc, argv);
    const Entry::Id entry = Entry::get(process);
    if (entry) {
        return Entry::exec(process, entry);
    }

    App app(&process);

    return app.exec();
}
