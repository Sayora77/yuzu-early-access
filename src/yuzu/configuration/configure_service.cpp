#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif

// Copyright 2019 yuzu Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include <QGraphicsItem>
#include <QtConcurrent/QtConcurrent>
#include "core/hle/service/bcat/backend/boxcat.h"
#include "core/settings.h"
#include "ui_configure_service.h"
#include "yuzu/configuration/configure_service.h"

namespace {
QString FormatEventStatusString(const Service::BCAT::EventStatus& status) {
    QString out;

    if (status.header.has_value()) {
        out += QStringLiteral("<i>%1</i><br>").arg(QString::fromStdString(*status.header));
    }

    if (status.events.size() == 1) {
        out += QStringLiteral("%1<br>").arg(QString::fromStdString(status.events.front()));
    } else {
        for (const auto& event : status.events) {
            out += QStringLiteral("- %1<br>").arg(QString::fromStdString(event));
        }
    }

    if (status.footer.has_value()) {
        out += QStringLiteral("<i>%1</i><br>").arg(QString::fromStdString(*status.footer));
    }

    return out;
}
} // Anonymous namespace

ConfigureService::ConfigureService(QWidget* parent)
    : QWidget(parent), ui(std::make_unique<Ui::ConfigureService>()) {
    ui->setupUi(this);

    ui->bcat_source->addItem(QStringLiteral("None"));
    ui->bcat_empty_label->setHidden(true);
    ui->bcat_empty_header->setHidden(true);

#ifdef YUZU_ENABLE_BOXCAT
    ui->bcat_source->addItem(QStringLiteral("Boxcat"), QStringLiteral("boxcat"));
#endif

    connect(ui->bcat_source, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            &ConfigureService::OnBCATImplChanged);

    this->SetConfiguration();
}

ConfigureService::~ConfigureService() = default;

void ConfigureService::ApplyConfiguration() {
    Settings::values.bcat_backend = ui->bcat_source->currentText().toLower().toStdString();
}

void ConfigureService::RetranslateUi() {
    ui->retranslateUi(this);
}

void ConfigureService::SetConfiguration() {
    const int index =
        ui->bcat_source->findData(QString::fromStdString(Settings::values.bcat_backend));
    ui->bcat_source->setCurrentIndex(index == -1 ? 0 : index);
}

std::pair<QString, QString> ConfigureService::BCATDownloadEvents() {
    std::optional<std::string> global;
    std::map<std::string, Service::BCAT::EventStatus> map;
    const auto res = Service::BCAT::Boxcat::GetStatus(global, map);

    switch (res) {
    case Service::BCAT::Boxcat::StatusResult::Success:
        break;
    case Service::BCAT::Boxcat::StatusResult::Offline:
        return {QString{},
                tr("该boxcat服务处于脱机状态，或者你没有连接到互联网.")};
    case Service::BCAT::Boxcat::StatusResult::ParseError:
        return {QString{},
                tr("在处理boxcat事件数据时出错，联系yuzu "
                   "开发商.")};
    case Service::BCAT::Boxcat::StatusResult::BadClientVersion:
        return {QString{},
                tr("您正在使用yuzu的版本是太新或太旧服务器. "
                   "尝试更新到yuzu的最新正式发布.")};
    }

    if (map.empty()) {
        return {QStringLiteral("Current Boxcat Events"),
                tr("上有boxcat目前没有活动.")};
    }

    QString out;

    if (global.has_value()) {
        out += QStringLiteral("%1<br>").arg(QString::fromStdString(*global));
    }

    for (const auto& [key, value] : map) {
        out += QStringLiteral("%1<b>%2</b><br>%3")
                   .arg(out.isEmpty() ? QString{} : QStringLiteral("<br>"))
                   .arg(QString::fromStdString(key))
                   .arg(FormatEventStatusString(value));
    }
    return {QStringLiteral("Current Boxcat Events"), std::move(out)};
}

void ConfigureService::OnBCATImplChanged() {
#ifdef YUZU_ENABLE_BOXCAT
    const auto boxcat = ui->bcat_source->currentText() == QStringLiteral("Boxcat");
    ui->bcat_empty_header->setHidden(!boxcat);
    ui->bcat_empty_label->setHidden(!boxcat);
    ui->bcat_empty_header->setText(QString{});
    ui->bcat_empty_label->setText(tr("yuzu被检索最新boxcat状态..."));

    if (!boxcat)
        return;

    const auto future = QtConcurrent::run([this] { return BCATDownloadEvents(); });

    watcher.setFuture(future);
    connect(&watcher, &QFutureWatcher<std::pair<QString, QString>>::finished, this,
            [this] { OnUpdateBCATEmptyLabel(watcher.result()); });
#endif
}

void ConfigureService::OnUpdateBCATEmptyLabel(std::pair<QString, QString> string) {
#ifdef YUZU_ENABLE_BOXCAT
    const auto boxcat = ui->bcat_source->currentText() == QStringLiteral("Boxcat");
    if (boxcat) {
        ui->bcat_empty_header->setText(string.first);
        ui->bcat_empty_label->setText(string.second);
    }
#endif
}
