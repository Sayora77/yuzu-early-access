#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif

// Copyright 2016 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include <array>
#include <chrono>
#include <optional>

#include <QFileDialog>
#include <QGraphicsItem>
#include <QMessageBox>
#include "common/assert.h"
#include "common/file_util.h"
#include "core/core.h"
#include "core/settings.h"
#include "ui_configure_system.h"
#include "yuzu/configuration/configure_system.h"

ConfigureSystem::ConfigureSystem(QWidget* parent) : QWidget(parent), ui(new Ui::ConfigureSystem) {
    ui->setupUi(this);
    connect(ui->button_regenerate_console_id, &QPushButton::clicked, this,
            &ConfigureSystem::RefreshConsoleID);

    connect(ui->rng_seed_checkbox, &QCheckBox::stateChanged, this, [this](bool checked) {
        ui->rng_seed_edit->setEnabled(checked);
        if (!checked) {
            ui->rng_seed_edit->setText(QStringLiteral("00000000"));
        }
    });

    connect(ui->custom_rtc_checkbox, &QCheckBox::stateChanged, this, [this](bool checked) {
        ui->custom_rtc_edit->setEnabled(checked);
        if (!checked) {
            ui->custom_rtc_edit->setDateTime(QDateTime::currentDateTime());
        }
    });

    SetConfiguration();
}

ConfigureSystem::~ConfigureSystem() = default;

void ConfigureSystem::changeEvent(QEvent* event) {
    if (event->type() == QEvent::LanguageChange) {
        RetranslateUI();
    }

    QWidget::changeEvent(event);
}

void ConfigureSystem::RetranslateUI() {
    ui->retranslateUi(this);
}

void ConfigureSystem::SetConfiguration() {
    enabled = !Core::System::GetInstance().IsPoweredOn();

    ui->combo_language->setCurrentIndex(Settings::values.language_index);

    ui->rng_seed_checkbox->setChecked(Settings::values.rng_seed.has_value());
    ui->rng_seed_edit->setEnabled(Settings::values.rng_seed.has_value());

    const auto rng_seed = QStringLiteral("%1")
                              .arg(Settings::values.rng_seed.value_or(0), 8, 16, QLatin1Char{'0'})
                              .toUpper();
    ui->rng_seed_edit->setText(rng_seed);

    ui->custom_rtc_checkbox->setChecked(Settings::values.custom_rtc.has_value());
    ui->custom_rtc_edit->setEnabled(Settings::values.custom_rtc.has_value());

    const auto rtc_time = Settings::values.custom_rtc.value_or(
        std::chrono::seconds(QDateTime::currentSecsSinceEpoch()));
    ui->custom_rtc_edit->setDateTime(QDateTime::fromSecsSinceEpoch(rtc_time.count()));
}

void ConfigureSystem::ReadSystemSettings() {}

void ConfigureSystem::ApplyConfiguration() {
    if (!enabled) {
        return;
    }

    Settings::values.language_index = ui->combo_language->currentIndex();

    if (ui->rng_seed_checkbox->isChecked()) {
        Settings::values.rng_seed = ui->rng_seed_edit->text().toULongLong(nullptr, 16);
    } else {
        Settings::values.rng_seed = std::nullopt;
    }

    if (ui->custom_rtc_checkbox->isChecked()) {
        Settings::values.custom_rtc =
            std::chrono::seconds(ui->custom_rtc_edit->dateTime().toSecsSinceEpoch());
    } else {
        Settings::values.custom_rtc = std::nullopt;
    }

    Settings::Apply();
}

void ConfigureSystem::RefreshConsoleID() {
    QMessageBox::StandardButton reply;
    QString warning_text = tr("这将有一个新的您当前的虚拟交换机将"
                              "无法恢复，这可能在游戏意想不到的"
                              "效果替换当前的虚拟交换机。如果您使用"
                              "的是过时的配置秘技这可能会失败，继续?");
    reply = QMessageBox::critical(this, tr("Warning"), warning_text,
                                  QMessageBox::No | QMessageBox::Yes);
    if (reply == QMessageBox::No) {
        return;
    }

    u64 console_id{};
    ui->label_console_id->setText(
        tr("Console ID: 0x%1").arg(QString::number(console_id, 16).toUpper()));
}
