
#include <Windows.h>
#include <cstdio>
#include "Enum.h"
#include <fstream>
#include <map>
#include <mutex>

std::once_flag failures, inventory, battlenet;

typedef bool(__cdecl *JamCheckFn)(WORD);

struct JamGroup
{
    JamCheckFn BelongsToGroup;
    JamCheckFn RequiresInstanceConnection;
};

void WINAPI DumpEnum(Enum const& enumData, std::string const& fileNameBase)
{
    std::ofstream dump(fileNameBase + ".h");
    dump << SourceOutput<Enum>(std::make_unique<CppEnum>(), enumData, 0);
    dump.close();

    dump.open(fileNameBase + ".idc");
    dump << SourceOutput<Enum>(std::make_unique<IdcEnum>(), enumData, 0);
    dump.close();
}

std::string to_hex(unsigned i)
{
#define TO_HEX(n) (n <= 9 ? '0' + n : 'A' - 10 + n)

    char res[11];

    res[0] = '0';
    res[1] = 'x';
    res[2] = TO_HEX(((i & 0xF0000000) >> 28));
    res[3] = TO_HEX(((i & 0x0F000000) >> 24));
    res[4] = TO_HEX(((i & 0x00F00000) >> 20));
    res[5] = TO_HEX(((i & 0x000F0000) >> 16));
    res[6] = TO_HEX(((i & 0x0000F000) >> 12));
    res[7] = TO_HEX(((i & 0x00000F00) >> 8));
    res[8] = TO_HEX(((i & 0x000000F0) >> 4));
    res[9] = TO_HEX(((i & 0x0000000F) >> 0));
    res[10] = '\0';

#undef TO_HEX

    return res;
}

extern "C"
{
__declspec(dllexport)
BOOL WINAPI DumpSpellFailures()
{
    std::call_once(failures, []()
    {
        typedef char*(__cdecl* pGetErrorString)(int);
        pGetErrorString GetStringReason = (pGetErrorString)((DWORD_PTR)GetModuleHandle(NULL) + 0x268B4A);

        Enum spellFailures;
        spellFailures.SetName("SPELL_FAILED_REASON");
        int err = 0;
        std::string error = GetStringReason(err);
        while (true)
        {
            if (!error.empty())
                spellFailures.AddMember(Enum::Member(std::size_t(err), error, ""));

            if (error == "SPELL_FAILED_UNKNOWN")
                break;

            error = GetStringReason(++err);
        }

        DumpEnum(spellFailures, "SpellCastResult");
    });
    return TRUE;
}

__declspec(dllexport)
BOOL WINAPI DumpInventoryErrors()
{
    struct UIErrorInfo
    {
        char const* ErrorName;
        int z;
        void* a;
        int b[2];
    };

    std::call_once(inventory, []()
    {
        UIErrorInfo* uis = (UIErrorInfo*)((DWORD_PTR)GetModuleHandle(NULL) + 0xBB7BE8);

        typedef int(__cdecl* GetGameErrorFn)(int);
        GetGameErrorFn CGBag_C_GetGameError = (GetGameErrorFn)((DWORD_PTR)GetModuleHandle(NULL) + 0x3737FC);

        Enum spellFailures;
        spellFailures.SetName("InventoryResult");
        spellFailures.SetPaddingAfterValueName(55);
        int err = 0;
        int error = CGBag_C_GetGameError(err);
        std::multimap<std::string, int> duplicates;
        while (err <= 97)
        {
            std::string err_name = std::string("EQUIP_");
            if (error < 946)
                err_name += uis[error].ErrorName;
            else
                err_name += "NONE";

            duplicates.emplace(err_name, err);
            if (duplicates.count(err_name) > 1)
                err_name += "_" + std::to_string(duplicates.count(err_name));

            spellFailures.AddMember(Enum::Member(std::size_t(err), err_name, ""));
            error = CGBag_C_GetGameError(++err);
        }

        DumpEnum(spellFailures, "InventoryResult");
    });
    return TRUE;
}

__declspec(dllexport)
BOOL WINAPI DumpBattlenetErrors()
{
    std::call_once(battlenet, []()
    {
        typedef char*(__cdecl* pGetErrorString)(unsigned);
        pGetErrorString GetStringReason = (pGetErrorString)((DWORD_PTR)GetModuleHandle(NULL) + 0x6C7B1C);

        Enum auroraStatus;
        auroraStatus.SetName("BattlenetRpcErrorCode");
        auroraStatus.SetPaddingAfterValueName(72);
        for (unsigned err = 0; err < 100010; ++err)
        {
            std::string error = GetStringReason(err);
            if (!error.empty() && error != "Status has no name.")
                auroraStatus.AddMember(Enum::Member(std::size_t(err), to_hex(err), error, ""));
        }

        for (unsigned err = 0x80000000; err < 0x80000200; ++err)
        {
            std::string error = GetStringReason(err);
            if (!error.empty() && error != "Status has no name.")
                auroraStatus.AddMember(Enum::Member(std::size_t(err), to_hex(err), error, ""));
        }

        DumpEnum(auroraStatus, "BattlenetRpcErrorCodes");
    });
    return TRUE;
}

__declspec(dllexport)
BOOL WINAPI DumpSwitchedEnums()
{
    DumpSpellFailures();
    DumpInventoryErrors();
    DumpBattlenetErrors();
    return TRUE;
}

BOOL WINAPI DllMain(HANDLE hDllHandle, DWORD dwReason, LPVOID lpreserved)
{
    DumpSwitchedEnums();

    // don't stay loaded, work was already done.
    // yes, this is horrible
    return FALSE;
}
}
