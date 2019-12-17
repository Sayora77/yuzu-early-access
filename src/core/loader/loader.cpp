#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif

// Copyright 2018 yuzu emulator team
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include <memory>
#include <ostream>
#include <string>
#include "common/file_util.h"
#include "common/logging/log.h"
#include "common/string_util.h"
#include "core/hle/kernel/process.h"
#include "core/loader/deconstructed_rom_directory.h"
#include "core/loader/elf.h"
#include "core/loader/kip.h"
#include "core/loader/nax.h"
#include "core/loader/nca.h"
#include "core/loader/nro.h"
#include "core/loader/nso.h"
#include "core/loader/nsp.h"
#include "core/loader/xci.h"

namespace Loader {

FileType IdentifyFile(FileSys::VirtualFile file) {
    FileType type;

#define CHECK_TYPE(loader)                                                                         \
    type = AppLoader_##loader::IdentifyType(file);                                                 \
    if (FileType::Error != type)                                                                   \
        return type;

    CHECK_TYPE(DeconstructedRomDirectory)
    CHECK_TYPE(ELF)
    CHECK_TYPE(NSO)
    CHECK_TYPE(NRO)
    CHECK_TYPE(NCA)
    CHECK_TYPE(XCI)
    CHECK_TYPE(NAX)
    CHECK_TYPE(NSP)
    CHECK_TYPE(KIP)

#undef CHECK_TYPE

    return FileType::Unknown;
}

FileType GuessFromFilename(const std::string& name) {
    if (name == "main")
        return FileType::DeconstructedRomDirectory;
    if (name == "00")
        return FileType::NCA;

    const std::string extension =
        Common::ToLower(std::string(FileUtil::GetExtensionFromFilename(name)));

    if (extension == "elf")
        return FileType::ELF;
    if (extension == "nro")
        return FileType::NRO;
    if (extension == "nso")
        return FileType::NSO;
    if (extension == "nca")
        return FileType::NCA;
    if (extension == "xci")
        return FileType::XCI;
    if (extension == "nsp")
        return FileType::NSP;
    if (extension == "kip")
        return FileType::KIP;

    return FileType::Unknown;
}

std::string GetFileTypeString(FileType type) {
    switch (type) {
    case FileType::ELF:
        return "ELF";
    case FileType::NRO:
        return "NRO";
    case FileType::NSO:
        return "NSO";
    case FileType::NCA:
        return "NCA";
    case FileType::XCI:
        return "XCI";
    case FileType::NAX:
        return "NAX";
    case FileType::NSP:
        return "NSP";
    case FileType::KIP:
        return "KIP";
    case FileType::DeconstructedRomDirectory:
        return "Directory";
    case FileType::Error:
    case FileType::Unknown:
        break;
    }

    return "unknown";
}

constexpr std::array<const char*, 66> RESULT_MESSAGES{
    "操作已成功完成.",
    "要求负载的装载机已加载.",
    "该操作未实现.",
    "装载机是不正确初始化.",
    "该NPDM文件有一个坏头.",
    "该NPDM有坏ACID头.",
    "该NPDM有坏ACI头,",
    "该NPDM文件有一个错误的文件访问控制.",
    "该NPDM有一个错误的文件访问头.",
    "该NPDM有错误的内核能力描述符.",
    "该PFS / HFS分区有一个坏头.",
    "该PFS / HFS分区具有由报头确定尺寸不正确.",
    "该NCA文件有一个坏头.",
    "一般的密钥文件找不到.",
    "该NCA标题key密钥无法找到.",
    "该NCA标题密钥不正确或标头无效.",
    "对于NCA2型种NCA支持未实现.",
    "对于NCA0型种NCA支持未实现.",
    "此权利ID的标题键找不到.",
    "这个加密修订标题KEK找不到.",
    "标头中的版权ID无效.",
    "此应用程序类型和加密版本的关键领域重点找不到.",
    "关键区域的关键是不正确或部分标题是无效的.",
    "该标题键和/或标题KEK不正确或节头是无效.",
    "该XCI文件缺少计划型NCA.",
    "该NCA文件不是应用程序.",
    "该ExeFS分区无法找到.",
    "该XCI文件有一个坏头.",
    "该XCI文件丢失分区.",
    "文件无法找到或不存在.",
    "本场比赛缺少程序元数据文件（main.npdm）.",
    "游戏采用当前未实现32位架构.",
    "无法加载模拟过程时完全解析内核元",
    "该ROMFS找不到.",
    "ELF文件具有通过标题来确定尺寸不正确.",
    "有一个普遍的错误装载NRO到模拟存储器.",
    "有一个普遍的错误装载NSO到模拟存储器.",
    "没有可用图标.",
    "没有控制数据可用.",
    "该NAX文件有一个坏头.",
    "该NAX文件具有由报头确定尺寸不正确.",
    "该HMAC所产生的NAX解密密钥失败.",
    "该HMAC验证失败NAX解密密钥.",
    "该NAX密钥推导失败.",
    "该NAX文件不能被解释为NCA文件.",
    "该NAX文件有一个不正确的路径.",
    "在SD种子无法找到或派生.",
    "该SD KEK来源找不到.",
    "该AES KEK生成源找不到.",
    "AES密钥生成源找不到.",
    "该SD保存项来源找不到.",
    "该SD NCA重点税源找不到.",
    "该NSP文件缺少计划型NCA.",
    "将在BKTR型NCA有一个不好的头在BKTR.",
    "将在BKTR分段项不搬迁入境后立即设.",
    "将在BKTR分段项不是在媒体块的结束.",
    "将在BKTR型NCA有坏块重定位.",
    "将在BKTR型NCA有坏块分段.",
    "将在BKTR型NCA有一个不好的搬迁桶.",
    "将在BKTR型NCA有一个不好的分段桶.",
    "该bktr型NCA缺少碱ROMFS.",
    "该NSP或XCI不包含的更新除了基本游戏.",
    "该KIP文件中有一个坏头.",
    "该部分的KIP BLZ解压意外失败.",
    "该 INI 文件中有一个坏头.",
    "该 INI 文件包含比KIP文件的最大允许数量更多.",
};

std::ostream& operator<<(std::ostream& os, ResultStatus status) {
    os << RESULT_MESSAGES.at(static_cast<std::size_t>(status));
    return os;
}

AppLoader::AppLoader(FileSys::VirtualFile file) : file(std::move(file)) {}
AppLoader::~AppLoader() = default;

/**
 * Get a loader for a file with a specific type
 * @param file The file to load
 * @param type The type of the file
 * @param file the file to retrieve the loader for
 * @param type the file type
 * @return std::unique_ptr<AppLoader> a pointer to a loader object;  nullptr for unsupported type
 */
static std::unique_ptr<AppLoader> GetFileLoader(FileSys::VirtualFile file, FileType type) {
    switch (type) {

    // Standard ELF file format.
    case FileType::ELF:
        return std::make_unique<AppLoader_ELF>(std::move(file));

    // NX NSO file format.
    case FileType::NSO:
        return std::make_unique<AppLoader_NSO>(std::move(file));

    // NX NRO file format.
    case FileType::NRO:
        return std::make_unique<AppLoader_NRO>(std::move(file));

    // NX NCA (Nintendo Content Archive) file format.
    case FileType::NCA:
        return std::make_unique<AppLoader_NCA>(std::move(file));

    // NX XCI (nX Card Image) file format.
    case FileType::XCI:
        return std::make_unique<AppLoader_XCI>(std::move(file));

    // NX NAX (NintendoAesXts) file format.
    case FileType::NAX:
        return std::make_unique<AppLoader_NAX>(std::move(file));

    // NX NSP (Nintendo Submission Package) file format
    case FileType::NSP:
        return std::make_unique<AppLoader_NSP>(std::move(file));

    // NX KIP (Kernel Internal Process) file format
    case FileType::KIP:
        return std::make_unique<AppLoader_KIP>(std::move(file));

    // NX deconstructed ROM directory.
    case FileType::DeconstructedRomDirectory:
        return std::make_unique<AppLoader_DeconstructedRomDirectory>(std::move(file));

    default:
        return nullptr;
    }
}

std::unique_ptr<AppLoader> GetLoader(FileSys::VirtualFile file) {
    FileType type = IdentifyFile(file);
    FileType filename_type = GuessFromFilename(file->GetName());

    // Special case: 00 is either a NCA or NAX.
    if (type != filename_type && !(file->GetName() == "00" && type == FileType::NAX)) {
        LOG_WARNING(Loader, "File {} has a different type than its extension.", file->GetName());
        if (FileType::Unknown == type)
            type = filename_type;
    }

    LOG_DEBUG(Loader, "Loading file {} as {}...", file->GetName(), GetFileTypeString(type));

    return GetFileLoader(std::move(file), type);
}

} // namespace Loader
