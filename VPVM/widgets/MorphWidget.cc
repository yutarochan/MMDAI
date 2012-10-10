/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2010-2012  hkrn                                    */
/*                                                                   */
/* All rights reserved.                                              */
/*                                                                   */
/* Redistribution and use in source and binary forms, with or        */
/* without modification, are permitted provided that the following   */
/* conditions are met:                                               */
/*                                                                   */
/* - Redistributions of source code must retain the above copyright  */
/*   notice, this list of conditions and the following disclaimer.   */
/* - Redistributions in binary form must reproduce the above         */
/*   copyright notice, this list of conditions and the following     */
/*   disclaimer in the documentation and/or other materials provided */
/*   with the distribution.                                          */
/* - Neither the name of the MMDAI project team nor the names of     */
/*   its contributors may be used to endorse or promote products     */
/*   derived from this software without specific prior written       */
/*   permission.                                                     */
/*                                                                   */
/* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND            */
/* CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,       */
/* INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF          */
/* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE          */
/* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS */
/* BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,          */
/* EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED   */
/* TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,     */
/* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON */
/* ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,   */
/* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY    */
/* OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE           */
/* POSSIBILITY OF SUCH DAMAGE.                                       */
/* ----------------------------------------------------------------- */

#include "common/util.h"
#include "models/MorphMotionModel.h"
#include "widgets/MorphWidget.h"

#include <QtGui/QtGui>
#include <vpvl2/vpvl2.h>
#include <vpvl2/qt/CString.h>

/* lupdate cannot parse tr() syntax correctly */

namespace vpvm
{

using namespace vpvl2;
using namespace vpvl2::qt;

MorphWidget::MorphWidget(MorphMotionModel *mmm, QWidget *parent) :
    QWidget(parent),
    m_morphMotionModel(mmm),
    m_seek(true)
{
    /* 目(左上) */
    QVBoxLayout *eyeVBoxLayout = new QVBoxLayout();
    m_eyeRegisterButton = new QPushButton();
    m_eyes = new QComboBox();
    m_eyesEdit = new QLineEdit();
    m_eyesCompleter = new QCompleter();
    m_eyesCompleterModel = new QStringListModel();
    m_eyeSlider = createSlider();
    m_eyesCompleter->setModel(m_eyesCompleterModel);
    m_eyesEdit->setCompleter(m_eyesCompleter);
    m_eyes->setLineEdit(m_eyesEdit);
    connect(m_eyes, SIGNAL(currentIndexChanged(int)), SLOT(updateMorphWeightValues()));
    connect(m_eyeRegisterButton, SIGNAL(clicked()), SLOT(registerEye()));
    connect(m_eyeSlider, SIGNAL(valueChanged(int)), SLOT(setEyeWeight(int)));
    eyeVBoxLayout->addWidget(m_eyes);
    eyeVBoxLayout->addWidget(m_eyeSlider);
    eyeVBoxLayout->addWidget(m_eyeRegisterButton);
    m_eyeGroup = new QGroupBox();
    m_eyeGroup->setLayout(eyeVBoxLayout);
    /* 口唇(右上) */
    QVBoxLayout *lipVBoxLayout = new QVBoxLayout();
    m_lipRegisterButton = new QPushButton();
    m_lips = new QComboBox();
    m_lipsEdit = new QLineEdit();
    m_lipsCompleter = new QCompleter();
    m_lipsCompleterModel = new QStringListModel();
    m_lipSlider = createSlider();
    m_lipsCompleter->setModel(m_lipsCompleterModel);
    m_lipsEdit->setCompleter(m_lipsCompleter);
    m_lips->setLineEdit(m_lipsEdit);
    connect(m_lips, SIGNAL(currentIndexChanged(int)), SLOT(updateMorphWeightValues()));
    connect(m_lipRegisterButton, SIGNAL(clicked()), SLOT(registerLip()));
    connect(m_lipSlider, SIGNAL(valueChanged(int)), SLOT(setLipWeight(int)));
    lipVBoxLayout->addWidget(m_lips);
    lipVBoxLayout->addWidget(m_lipSlider);
    lipVBoxLayout->addWidget(m_lipRegisterButton);
    m_lipGroup = new QGroupBox();
    m_lipGroup->setLayout(lipVBoxLayout);
    /* まゆ(左下) */
    QVBoxLayout *eyeblowVBoxLayout = new QVBoxLayout();
    m_eyeblowRegisterButton = new QPushButton();
    m_eyeblows = new QComboBox();
    m_eyeblowsEdit = new QLineEdit();
    m_eyeblowsCompleter = new QCompleter();
    m_eyeblowsCompleterModel = new QStringListModel();
    m_eyeblowSlider = createSlider();
    m_eyeblowsCompleter->setModel(m_eyeblowsCompleterModel);
    m_eyeblowsEdit->setCompleter(m_eyeblowsCompleter);
    m_eyeblows->setLineEdit(m_eyeblowsEdit);
    connect(m_eyeblows, SIGNAL(currentIndexChanged(int)), SLOT(updateMorphWeightValues()));
    connect(m_eyeblowRegisterButton, SIGNAL(clicked()), SLOT(registerEyeblow()));
    connect(m_eyeblowSlider, SIGNAL(valueChanged(int)), SLOT(setEyeblowWeight(int)));
    eyeblowVBoxLayout->addWidget(m_eyeblows);
    eyeblowVBoxLayout->addWidget(m_eyeblowSlider);
    eyeblowVBoxLayout->addWidget(m_eyeblowRegisterButton);
    m_eyeblowGroup = new QGroupBox();
    m_eyeblowGroup->setLayout(eyeblowVBoxLayout);
    /* その他(右下) */
    QVBoxLayout *otherVBoxLayout = new QVBoxLayout();
    m_otherRegisterButton = new QPushButton();
    m_others = new QComboBox();
    m_othersEdit = new QLineEdit();
    m_othersCompleter = new QCompleter();
    m_othersCompleterModel = new QStringListModel();
    m_otherSlider = createSlider();
    m_othersCompleter->setModel(m_othersCompleterModel);
    m_othersEdit->setCompleter(m_othersCompleter);
    m_others->setLineEdit(m_othersEdit);
    connect(m_others, SIGNAL(currentIndexChanged(int)), SLOT(updateMorphWeightValues()));
    connect(m_otherRegisterButton, SIGNAL(clicked()), SLOT(registerOther()));
    connect(m_otherSlider, SIGNAL(valueChanged(int)), SLOT(setOtherWeight(int)));
    otherVBoxLayout->addWidget(m_others);
    otherVBoxLayout->addWidget(m_otherSlider);
    otherVBoxLayout->addWidget(m_otherRegisterButton);
    m_otherGroup = new QGroupBox();
    m_otherGroup->setLayout(otherVBoxLayout);
    /* 「全てのモーフをリセット」ボタン */
    m_resetAllButton = new QPushButton();
    connect(m_resetAllButton, SIGNAL(clicked()), m_morphMotionModel, SLOT(resetAllMorphs()));
    /* レイアウト結合 */
    QVBoxLayout *mainLayout = new QVBoxLayout();
    QLayout *subLayout = new QHBoxLayout();
    subLayout->addWidget(m_eyeGroup);
    subLayout->addWidget(m_lipGroup);
    mainLayout->addLayout(subLayout);
    subLayout = new QHBoxLayout();
    subLayout->addWidget(m_eyeblowGroup);
    subLayout->addWidget(m_otherGroup);
    mainLayout->addLayout(subLayout);
    mainLayout->addWidget(m_resetAllButton);
    mainLayout->addStretch();
    retranslate();
    setLayout(mainLayout);
    setEnabled(false);
    connect(m_morphMotionModel, SIGNAL(modelDidChange(IModel*)), SLOT(setPMDModel(IModel*)));
}

void MorphWidget::retranslate()
{
    const QString &buttonText = vpvm::MorphWidget::tr("Register");
    m_eyeRegisterButton->setText(buttonText);
    m_lipRegisterButton->setText(buttonText);
    m_eyeblowRegisterButton->setText(buttonText);
    m_otherRegisterButton->setText(buttonText);
    m_eyeGroup->setTitle(vpvm::MorphWidget::tr("Eye"));
    m_lipGroup->setTitle(vpvm::MorphWidget::tr("Lip"));
    m_eyeblowGroup->setTitle(vpvm::MorphWidget::tr("Eyeblow"));
    m_otherGroup->setTitle(vpvm::MorphWidget::tr("Other"));
    m_resetAllButton->setText(vpvm::MorphWidget::tr("Reset all morphs"));
}

void MorphWidget::setPMDModel(IModel *model)
{
    QStringList eyes, lips, eyeblows, others;
    m_eyes->clear();
    m_lips->clear();
    m_eyeblows->clear();
    m_others->clear();
    if (model) {
        Array<IMorph *> morphs;
        m_morphMotionModel->selectedModel()->getMorphRefs(morphs);
        const int nmorphs = morphs.count();
        for (int i = 0; i < nmorphs; i++) {
            IMorph *morph = morphs[i];
            const QString &name = toQStringFromMorph(morph);
            switch (morph->category()) {
            case IMorph::kEye:
                m_eyes->addItem(name, name);
                eyes.append(name);
                break;
            case IMorph::kLip:
                m_lips->addItem(name, name);
                lips.append(name);
                break;
            case IMorph::kEyeblow:
                m_eyeblows->addItem(name, name);
                eyeblows.append(name);
                break;
            case IMorph::kOther:
                m_others->addItem(name, name);
                others.append(name);
                break;
            default:
                break;
            }
        }
        setEnabled(true);
    }
    else {
        setEnabled(false);
    }
    m_eyesCompleterModel->setStringList(eyes);
    m_lipsCompleterModel->setStringList(lips);
    m_eyeblowsCompleterModel->setStringList(eyeblows);
    m_othersCompleterModel->setStringList(others);
}

void MorphWidget::setEyeWeight(int value)
{
    setMorphWeight(m_eyes, value);
}

void MorphWidget::setLipWeight(int value)
{
    setMorphWeight(m_lips, value);
}

void MorphWidget::setEyeblowWeight(int value)
{
    setMorphWeight(m_eyeblows, value);
}

void MorphWidget::setOtherWeight(int value)
{
    setMorphWeight(m_others, value);
}

void MorphWidget::registerEye()
{
    registerBase(m_eyes);
}

void MorphWidget::registerLip()
{
    registerBase(m_lips);
}

void MorphWidget::registerEyeblow()
{
    registerBase(m_eyeblows);
}

void MorphWidget::registerOther()
{
    registerBase(m_others);
}

void MorphWidget::registerBase(const QComboBox *comboBox)
{
    IModel *model = m_morphMotionModel->selectedModel();
    int index = comboBox->currentIndex();
    if (model && index >= 0) {
        const CString s(comboBox->itemText(index));
        IMorph *morph = model->findMorph(&s);
        if (morph)
            emit morphDidRegister(morph);
    }
}

void MorphWidget::updateMorphWeightValues()
{
    /*
     * SceneWidget#seekMotion でモデルのモーフ値が変更済みなので、その値を取り出してスライダーに反映させる
     * また、二重にシークして意図しないモーションの動きが発生することを防ぐために一時的にシークを無効にする
     * (ボーンタイムラインでシークする時の MorphMotionModel::m_frameIndex が 0 であるため)
     */
    m_seek = false;
    updateMorphWeight(m_eyes, m_eyeSlider);
    updateMorphWeight(m_lips, m_lipSlider);
    updateMorphWeight(m_eyeblows, m_eyeblowSlider);
    updateMorphWeight(m_others, m_otherSlider);
    m_seek = true;
}

void MorphWidget::updateMorphWeight(const QComboBox *comboBox, QSlider *slider)
{
    IModel *model = m_morphMotionModel->selectedModel();
    int index = comboBox->currentIndex();
    if (model && index >= 0) {
        const CString s(comboBox->itemText(index));
        IMorph *morph = model->findMorph(&s);
        if (morph)
            slider->setValue(morph->weight() * kSliderMaximumValue);
    }
}

void MorphWidget::setMorphWeight(const QComboBox *comboBox, int value)
{
    IModel *model = m_morphMotionModel->selectedModel();
    int index = comboBox->currentIndex();
    if (model && index >= 0) {
        const CString s(comboBox->itemText(index));
        IMorph *morph = model->findMorph(&s);
        if (morph) {
            /* モデルのモーフの変更だけ行う。キーフレームの登録は行わない */
            const Scalar &newWeight = value / static_cast<Scalar>(kSliderMaximumValue);
            m_morphMotionModel->setWeight(newWeight, morph);
            m_morphMotionModel->updateModel(m_morphMotionModel->selectedModel(), m_seek);
        }
    }
}

QSlider *MorphWidget::createSlider() const
{
    QSlider *slider = new QSlider(Qt::Horizontal);
    connect(slider, SIGNAL(sliderPressed()), SIGNAL(morphWillChange()));
    connect(slider, SIGNAL(sliderReleased()), SIGNAL(morphDidChange()));
    slider->setTickInterval(20);
    slider->setMinimum(0);
    slider->setMaximum(kSliderMaximumValue);
    return slider;
}

} /* namespace vpvm */
