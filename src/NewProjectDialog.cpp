#include "NewProjectDialog.h"
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QFileDialog>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QDialogButtonBox>

NewProjectDialog::NewProjectDialog(QWidget* parent) : QDialog(parent) {
    setWindowTitle("New PS2 Project");
    resize(520, 220);
    m_name = new QLineEdit("MyPS2Game", this);
    m_path = new QLineEdit(this);
    m_type = new QComboBox(this);
    m_engine = new QComboBox(this);
    m_engine->addItems({"PS2 Studio Native (PS2SDK + gsKit)", "Tyra Engine"});
    m_type->addItems({"Empty PS2 Application", "2D Game (gsKit)", "2D Platformer", "3D Game (gsKit/VU1)", "3D Third Person", "FPS", "Racing", "UI / Menu", "2D/3D Hybrid", "IOP Module"});
    auto browse = new QPushButton("Browse…", this);
    connect(browse, &QPushButton::clicked, this, [this]{
        const auto p = QFileDialog::getExistingDirectory(this, "Project Location");
        if (!p.isEmpty()) m_path->setText(p);
    });
    auto pathRow = new QHBoxLayout;
    pathRow->addWidget(m_path, 1);
    pathRow->addWidget(browse);
    auto form = new QFormLayout;
    form->addRow("Name", m_name);
    form->addRow("Engine", m_engine);
    form->addRow("Type", m_type);
    form->addRow("Location", pathRow);
    auto buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    auto root = new QVBoxLayout(this);
    root->addLayout(form);
    root->addStretch();
    root->addWidget(buttons);
}
QString NewProjectDialog::projectName() const { return m_name->text().trimmed(); }
QString NewProjectDialog::projectPath() const { return m_path->text().trimmed(); }
QString NewProjectDialog::projectType() const { return m_type->currentText(); }

QString NewProjectDialog::engineBackend() const { return m_engine->currentText().startsWith("Tyra") ? "tyra" : "native"; }
