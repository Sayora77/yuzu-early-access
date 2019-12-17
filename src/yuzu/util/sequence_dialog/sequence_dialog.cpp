﻿#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif

// Copyright 2018 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include <QDialogButtonBox>
#include <QKeySequenceEdit>
#include <QVBoxLayout>
#include "yuzu/util/sequence_dialog/sequence_dialog.h"

SequenceDialog::SequenceDialog(QWidget* parent) : QDialog(parent) {
    setWindowTitle(tr("输入热键"));
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    key_sequence = new QKeySequenceEdit;

    auto* const buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    buttons->setCenterButtons(true);

    auto* const layout = new QVBoxLayout(this);
    layout->addWidget(key_sequence);
    layout->addWidget(buttons);

    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

SequenceDialog::~SequenceDialog() = default;

QKeySequence SequenceDialog::GetSequence() const {
    // Only the first key is returned. The other 3, if present, are ignored.
    return QKeySequence(key_sequence->keySequence()[0]);
}

bool SequenceDialog::focusNextPrevChild(bool next) {
    return false;
}

void SequenceDialog::closeEvent(QCloseEvent*) {
    reject();
}
