#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif

// Copyright 2015 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#pragma once

#include <array>
#include <map>
#include <string>
#include <utility>

#include <QCoreApplication>
#include <QFileInfo>
#include <QImage>
#include <QObject>
#include <QStandardItem>
#include <QString>
#include <QWidget>

#include "common/common_types.h"
#include "common/logging/log.h"
#include "common/string_util.h"
#include "yuzu/uisettings.h"
#include "yuzu/util/util.h"

enum class GameListItemType {
    Game = QStandardItem::UserType + 1,
    CustomDir = QStandardItem::UserType + 2,
    SdmcDir = QStandardItem::UserType + 3,
    UserNandDir = QStandardItem::UserType + 4,
    SysNandDir = QStandardItem::UserType + 5,
    AddDir = QStandardItem::UserType + 6
};

Q_DECLARE_METATYPE(GameListItemType);

/**
 * Gets the default icon (for games without valid title metadata)
 * @param size The desired width and height of the default icon.
 * @return QPixmap default icon
 */
static QPixmap GetDefaultIcon(u32 size) {
    QPixmap icon(size, size);
    icon.fill(Qt::transparent);
    return icon;
}

class GameListItem : public QStandardItem {

public:
    // used to access type from item index
    static const int TypeRole = Qt::UserRole + 1;
    static const int SortRole = Qt::UserRole + 2;
    GameListItem() = default;
    GameListItem(const QString& string) : QStandardItem(string) {
        setData(string, SortRole);
    }
};

/**
 * A specialization of GameListItem for path values.
 * This class ensures that for every full path value it holds, a correct string representation
 * of just the filename (with no extension) will be displayed to the user.
 * If this class receives valid title metadata, it will also display game icons and titles.
 */
class GameListItemPath : public GameListItem {
public:
    static const int TitleRole = SortRole;
    static const int FullPathRole = SortRole + 1;
    static const int ProgramIdRole = SortRole + 2;
    static const int FileTypeRole = SortRole + 3;

    GameListItemPath() = default;
    GameListItemPath(const QString& game_path, const std::vector<u8>& picture_data,
                     const QString& game_name, const QString& game_type, u64 program_id) {
        setData(type(), TypeRole);
        setData(game_path, FullPathRole);
        setData(game_name, TitleRole);
        setData(qulonglong(program_id), ProgramIdRole);
        setData(game_type, FileTypeRole);

        const u32 size = UISettings::values.icon_size;

        QPixmap picture;
        if (!picture.loadFromData(picture_data.data(), static_cast<u32>(picture_data.size()))) {
            picture = GetDefaultIcon(size);
        }
        picture = picture.scaled(size, size, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

        setData(picture, Qt::DecorationRole);
    }

    int type() const override {
        return static_cast<int>(GameListItemType::Game);
    }

    QVariant data(int role) const override {
        if (role == Qt::DisplayRole) {
            std::string filename;
            Common::SplitPath(data(FullPathRole).toString().toStdString(), nullptr, &filename,
                              nullptr);

            const std::array<QString, 4> row_data{{
                QString::fromStdString(filename),
                data(FileTypeRole).toString(),
                QString::fromStdString(fmt::format("0x{:016X}", data(ProgramIdRole).toULongLong())),
                data(TitleRole).toString(),
            }};

            const auto& row1 = row_data.at(UISettings::values.row_1_text_id);
            const auto& row2 = row_data.at(UISettings::values.row_2_text_id);

            if (row1.isEmpty() || row1 == row2)
                return row2;
            if (row2.isEmpty())
                return row1;

            return QString(row1 + QStringLiteral("\n    ") + row2);
        }

        return GameListItem::data(role);
    }
};

class GameListItemCompat : public GameListItem {
    Q_DECLARE_TR_FUNCTIONS(GameListItemCompat)
public:
    static const int CompatNumberRole = SortRole;
    GameListItemCompat() = default;
    explicit GameListItemCompat(const QString& compatibility) {
        setData(type(), TypeRole);

        struct CompatStatus {
            QString color;
            const char* text;
            const char* tooltip;
        };
        // clang-format off
        static const std::map<QString, CompatStatus> status_data = {
            {QStringLiteral("0"),  {QStringLiteral("#5c93ed"), QT_TR_NOOP("完美"),    QT_TR_NOOP("游戏功能无措，没有音频或故障的图形，所有测试功能按预期正常运行，而不\n任何变通办法需要.")}},
            {QStringLiteral("1"),  {QStringLiteral("#47d35c"), QT_TR_NOOP("很好"),      QT_TR_NOOP("轻微的图形或音频故障的游戏功能，可播放从开始到结束，可能需要一些\n解决方法.")}},
            {QStringLiteral("2"),  {QStringLiteral("#94b242"), QT_TR_NOOP("好的"),       QT_TR_NOOP("与主要的图形或音频故障，但游戏游戏功能，可玩从开始到完成用\n解决方法.")}},
            {QStringLiteral("3"),  {QStringLiteral("#f2d624"), QT_TR_NOOP("坏"),        QT_TR_NOOP("游戏功能，但主要的图形或音频故障，无法在因故障的具体领域取得的进展\n即使有解决办法.")}},
            {QStringLiteral("4"),  {QStringLiteral("#FF0000"), QT_TR_NOOP("简介/菜单"), QT_TR_NOOP("游戏是完全无法播放，由于主要的图形或音频故障，过去无法在开始进展\n屏幕.")}},
            {QStringLiteral("5"),  {QStringLiteral("#828282"), QT_TR_NOOP("无法启动"), QT_TR_NOOP("尝试启动游戏时崩溃.")}},
            {QStringLiteral("99"), {QStringLiteral("#000000"), QT_TR_NOOP("未测试"), QT_TR_NOOP("游戏尚未测试.")}},
        };
        // clang-format on

        auto iterator = status_data.find(compatibility);
        if (iterator == status_data.end()) {
            LOG_WARNING(Frontend, "Invalid compatibility number {}", compatibility.toStdString());
            return;
        }
        const CompatStatus& status = iterator->second;
        setData(compatibility, CompatNumberRole);
        setText(QObject::tr(status.text));
        setToolTip(QObject::tr(status.tooltip));
        setData(CreateCirclePixmapFromColor(status.color), Qt::DecorationRole);
    }

    int type() const override {
        return static_cast<int>(GameListItemType::Game);
    }

    bool operator<(const QStandardItem& other) const override {
        return data(CompatNumberRole) < other.data(CompatNumberRole);
    }
};

/**
 * A specialization of GameListItem for size values.
 * This class ensures that for every numerical size value it holds (in bytes), a correct
 * human-readable string representation will be displayed to the user.
 */
class GameListItemSize : public GameListItem {
public:
    static const int SizeRole = SortRole;

    GameListItemSize() = default;
    explicit GameListItemSize(const qulonglong size_bytes) {
        setData(type(), TypeRole);
        setData(size_bytes, SizeRole);
    }

    void setData(const QVariant& value, int role) override {
        // By specializing setData for SizeRole, we can ensure that the numerical and string
        // representations of the data are always accurate and in the correct format.
        if (role == SizeRole) {
            qulonglong size_bytes = value.toULongLong();
            GameListItem::setData(ReadableByteSize(size_bytes), Qt::DisplayRole);
            GameListItem::setData(value, SizeRole);
        } else {
            GameListItem::setData(value, role);
        }
    }

    int type() const override {
        return static_cast<int>(GameListItemType::Game);
    }

    /**
     * This operator is, in practice, only used by the TreeView sorting systems.
     * Override it so that it will correctly sort by numerical value instead of by string
     * representation.
     */
    bool operator<(const QStandardItem& other) const override {
        return data(SizeRole).toULongLong() < other.data(SizeRole).toULongLong();
    }
};

class GameListDir : public GameListItem {
public:
    static const int GameDirRole = Qt::UserRole + 2;

    explicit GameListDir(UISettings::GameDir& directory,
                         GameListItemType dir_type = GameListItemType::CustomDir)
        : dir_type{dir_type} {
        setData(type(), TypeRole);

        UISettings::GameDir* game_dir = &directory;
        setData(QVariant::fromValue(game_dir), GameDirRole);

        const int icon_size = std::min(static_cast<int>(UISettings::values.icon_size), 64);
        switch (dir_type) {
        case GameListItemType::SdmcDir:
            setData(
                QIcon::fromTheme(QStringLiteral("sd_card"))
                    .pixmap(icon_size)
                    .scaled(icon_size, icon_size, Qt::IgnoreAspectRatio, Qt::SmoothTransformation),
                Qt::DecorationRole);
            setData(QObject::tr("安装 SD 标题"), Qt::DisplayRole);
            break;
        case GameListItemType::UserNandDir:
            setData(
                QIcon::fromTheme(QStringLiteral("chip"))
                    .pixmap(icon_size)
                    .scaled(icon_size, icon_size, Qt::IgnoreAspectRatio, Qt::SmoothTransformation),
                Qt::DecorationRole);
            setData(QObject::tr("安装 NAND 标题"), Qt::DisplayRole);
            break;
        case GameListItemType::SysNandDir:
            setData(
                QIcon::fromTheme(QStringLiteral("chip"))
                    .pixmap(icon_size)
                    .scaled(icon_size, icon_size, Qt::IgnoreAspectRatio, Qt::SmoothTransformation),
                Qt::DecorationRole);
            setData(QObject::tr("系统标题"), Qt::DisplayRole);
            break;
        case GameListItemType::CustomDir: {
            const QString icon_name = QFileInfo::exists(game_dir->path)
                                          ? QStringLiteral("folder")
                                          : QStringLiteral("bad_folder");
            setData(QIcon::fromTheme(icon_name).pixmap(icon_size).scaled(
                        icon_size, icon_size, Qt::IgnoreAspectRatio, Qt::SmoothTransformation),
                    Qt::DecorationRole);
            setData(game_dir->path, Qt::DisplayRole);
            break;
        }
        default:
            break;
        }
    }

    int type() const override {
        return static_cast<int>(dir_type);
    }

private:
    GameListItemType dir_type;
};

class GameListAddDir : public GameListItem {
public:
    explicit GameListAddDir() {
        setData(type(), TypeRole);

        const int icon_size = std::min(static_cast<int>(UISettings::values.icon_size), 64);
        setData(QIcon::fromTheme(QStringLiteral("plus"))
                    .pixmap(icon_size)
                    .scaled(icon_size, icon_size, Qt::IgnoreAspectRatio, Qt::SmoothTransformation),
                Qt::DecorationRole);
        setData(QObject::tr("添加新的游戏目录"), Qt::DisplayRole);
    }

    int type() const override {
        return static_cast<int>(GameListItemType::AddDir);
    }
};

class GameList;
class QHBoxLayout;
class QTreeView;
class QLabel;
class QLineEdit;
class QToolButton;

class GameListSearchField : public QWidget {
    Q_OBJECT

public:
    explicit GameListSearchField(GameList* parent = nullptr);

    void setFilterResult(int visible, int total);

    void clear();
    void setFocus();

private:
    class KeyReleaseEater : public QObject {
    public:
        explicit KeyReleaseEater(GameList* gamelist);

    private:
        GameList* gamelist = nullptr;
        QString edit_filter_text_old;

    protected:
        // EventFilter in order to process systemkeys while editing the searchfield
        bool eventFilter(QObject* obj, QEvent* event) override;
    };
    int visible;
    int total;

    QHBoxLayout* layout_filter = nullptr;
    QTreeView* tree_view = nullptr;
    QLabel* label_filter = nullptr;
    QLineEdit* edit_filter = nullptr;
    QLabel* label_filter_result = nullptr;
    QToolButton* button_filter_close = nullptr;
};
