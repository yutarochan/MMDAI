/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2010-2013  hkrn                                    */
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

#include "vpvl2/vpvl2.h"
#include "vpvl2/internal/util.h"
#include "vpvl2/pmd2/Bone.h"
#include "vpvl2/pmd2/Label.h"
#include "vpvl2/pmd2/Morph.h"

namespace
{

#pragma pack(push, 1)

struct BoneLabel
{
    vpvl2::uint16_t boneIndex;
    vpvl2::uint8_t categoryIndex;
};

#pragma pack(pop)
}

namespace vpvl2
{
namespace pmd2
{

Label::Label(IModel *modelRef, IEncoding *encodingRef, const uint8_t *name, Type type)
    : m_modelRef(modelRef),
      m_encodingRef(encodingRef),
      m_namePtr(0),
      m_englishNamePtr(0),
      m_type(type),
      m_index(-1)
{
    m_namePtr = m_encodingRef->toString(name, IString::kShiftJIS, Bone::kCategoryNameSize);
}

Label::~Label()
{
    delete m_namePtr;
    m_namePtr = 0;
    delete m_englishNamePtr;
    m_englishNamePtr = 0;
    m_encodingRef = 0;
    m_index = -1;
}

bool Label::preparse(uint8_t *&ptr, size_t &rest, Model::DataInfo &info)
{
    uint8_t size;
    if (!internal::getTyped<uint8_t>(ptr, rest, size) || size * sizeof(uint16_t) > rest) {
        return false;
    }
    info.morphLabelsCount = size;
    info.morphLabelsPtr = ptr;
    internal::drainBytes(size * sizeof(uint16_t), ptr, rest);
    if (!internal::getTyped<uint8_t>(ptr, rest, size) || size * Bone::kCategoryNameSize > rest) {
        return false;
    }
    info.boneCategoryNamesCount = size;
    info.boneCategoryNamesPtr = ptr;
    internal::drainBytes(size * Bone::kCategoryNameSize, ptr, rest);
    int size32;
    if (!internal::getTyped<int>(ptr, rest, size32) || size32 * sizeof(BoneLabel) > rest) {
        return false;
    }
    info.boneLabelsCount = size32;
    info.boneLabelsPtr = ptr;
    internal::drainBytes(size32 * sizeof(BoneLabel), ptr, rest);
    return true;
}

bool Label::loadLabels(const Array<Label *> &labels, const Array<Bone *> &bones, const Array<Morph *> &morphs)
{
    const int nlabels = labels.count();
    const int nbones = bones.count();
    const int nmorphs = morphs.count();
    for (int i = 0; i < nlabels; i++) {
        Label *label = labels[i];
        switch (label->type()) {
        case kSpecialBoneCategoryLabel:
        case kBoneCategoryLabel: {
            const Array<int> &indices = label->m_boneIndices;
            const int nindices = indices.count();
            for (int j = 0; j < nindices; j++) {
                int index = indices[j];
                if (internal::checkBound(index, 0, nbones)) {
                    label->m_boneRefs.append(bones[index]);
                }
            }
            break;
        }
        case kMorphCategoryLabel: {
            const Array<int> &indices = label->m_morphIndices;
            const int nindices = indices.count();
            for (int j = 0; j < nindices; j++) {
                int index = indices[j];
                if (internal::checkBound(index, 0, nmorphs)) {
                    label->m_morphRefs.append(morphs[index]);
                }
            }
            break;
        }
        default:
            break;
        }
        label->setIndex(i);
    }
    return true;
}

void Label::writeLabels(const Array<Label *> &labels, const Model::DataInfo &info, uint8_t *&data)
{
    const int nlabels = labels.count();
    int nbones = 0, nmorphs = 0, ncategories = 0;
    for (int i = 0; i < nlabels; i++) {
        Label *label = labels[i];
        switch (label->type()) {
        case kSpecialBoneCategoryLabel:
        case kBoneCategoryLabel: {
            nbones += label->m_boneRefs.count();
            ncategories++;
            break;
        }
        case kMorphCategoryLabel: {
            nmorphs += label->m_morphRefs.count();
            break;
        }
        default:
            break;
        }
    }
    internal::writeUnsignedIndex(nmorphs, sizeof(uint8_t), data);
    for (int i = 0; i < nlabels; i++) {
        Label *label = labels[i];
        Label::Type type = label->type();
        if (type == kMorphCategoryLabel) {
            label->write(data, info);
        }
    }
    const IEncoding *encodingRef = info.encoding;
    internal::writeUnsignedIndex(ncategories, sizeof(uint8_t), data);
    uint8_t categoryName[Bone::kCategoryNameSize], *categoryNamePtr = categoryName;
    for (int i = 0; i < nlabels; i++) {
        Label *label = labels[i];
        Label::Type type = label->type();
        if (type == kSpecialBoneCategoryLabel || type == kBoneCategoryLabel) {
            internal::writeStringAsByteArray(label->name(), IString::kShiftJIS, encodingRef, sizeof(categoryName), categoryNamePtr);
            internal::writeBytes(categoryName, sizeof(categoryName), data);
            categoryNamePtr = categoryName;
        }
    }
    internal::writeBytes(&nbones, sizeof(nbones), data);
    for (int i = 0; i < nlabels; i++) {
        Label *label = labels[i];
        Label::Type type = label->type();
        if (type == kSpecialBoneCategoryLabel || type == kBoneCategoryLabel) {
            label->write(data, info);
        }
    }
}

void Label::writeEnglishNames(const Array<Label *> &labels, const Model::DataInfo &info, uint8_t *&data)
{
    const IEncoding *encodingRef = info.encoding;
    const int nlabels = labels.count();
    for (int i = 0; i < nlabels; i++) {
        Label *label = labels[i];
        Label::Type type = label->type();
        if (type == kSpecialBoneCategoryLabel || type == kBoneCategoryLabel) {
            internal::writeStringAsByteArray(label->englishName(), IString::kShiftJIS, encodingRef, Bone::kCategoryNameSize, data);
        }
    }
}

size_t Label::estimateTotalSize(const Array<Label *> &labels, const Model::DataInfo &info)
{
    const int nlabels = labels.count();
    size_t size = sizeof(int32_t) + sizeof(uint8_t) + sizeof(uint8_t), ncategories = 0;
    for (int i = 0; i < nlabels; i++) {
        Label *label = labels[i];
        size += label->estimateSize(info);
        Type type = label->type();
        if (type == kSpecialBoneCategoryLabel || type == kBoneCategoryLabel) {
            ncategories++;
        }
    }
    size += ncategories * Bone::kCategoryNameSize;
    return size;
}

Label *Label::selectCategory(const Array<Label *> &labels, const uint8_t *data)
{
    BoneLabel label;
    internal::getData(data, label);
    int index = label.categoryIndex - 1;
    if (internal::checkBound(index, 0, labels.count())) {
        Label *label = labels[index];
        return label;
    }
    return 0;
}

void Label::read(const uint8_t *data, const Model::DataInfo & /* info */, size_t &size)
{
    switch (m_type) {
    case kSpecialBoneCategoryLabel:
    case kBoneCategoryLabel: {
        BoneLabel label;
        internal::getData(data, label);
        m_boneIndices.append(label.boneIndex);
        size = sizeof(label);
        break;
    }
    case kMorphCategoryLabel: {
        uint16_t morphIndex;
        internal::getData(data, morphIndex);
        m_morphIndices.append(morphIndex);
        size = sizeof(morphIndex);
        break;
    }
    default:
        size = 0;
        break;
    }
}

void Label::readEnglishName(const uint8_t *data, int index)
{
    if (data && index >= 0) {
        internal::setStringDirect(m_encodingRef->toString(data + kBoneCategoryLabel * index, IString::kShiftJIS, kBoneCategoryLabel), m_englishNamePtr);
    }
}

size_t Label::estimateSize(const Model::DataInfo & /* info */) const
{
    size_t size = 0;
    switch (m_type) {
    case kSpecialBoneCategoryLabel:
    case kBoneCategoryLabel: {
        size += sizeof(BoneLabel) * m_boneRefs.count();
        break;
    }
    case kMorphCategoryLabel: {
        size += sizeof(uint16_t) * m_morphRefs.count();
        break;
    }
    default:
        break;
    }
    return size;
}

void Label::write(uint8_t *&data, const Model::DataInfo & /* info */) const
{
    switch (m_type) {
    case kSpecialBoneCategoryLabel:
    case kBoneCategoryLabel: {
        const int nindices = m_boneRefs.count();
        BoneLabel label;
        for (int i = 0; i < nindices; i++) {
            IBone *bone = m_boneRefs[i];
            label.boneIndex = bone->index();
            label.categoryIndex = index() + 1;
            internal::writeBytes(&label, sizeof(label), data);
        }
        break;
    }
    case kMorphCategoryLabel: {
        const int nindices = m_morphRefs.count();
        uint16_t value;
        for (int i = 0; i < nindices; i++) {
            IMorph *morph = m_morphRefs[i];
            value = morph->index();
            internal::writeBytes(&value, sizeof(value), data);
        }
        break;
    }
    default:
        break;
    }
}

const IString *Label::name() const
{
    return m_namePtr;
}

const IString *Label::englishName() const
{
    return m_namePtr;
}

bool Label::isSpecial() const
{
    return m_type == kSpecialBoneCategoryLabel;
}

int Label::count() const
{
    switch (m_type) {
    case kSpecialBoneCategoryLabel:
    case kBoneCategoryLabel:
        return m_boneRefs.count();
    case kMorphCategoryLabel:
        return m_morphRefs.count();
    default:
        return 0;
    }
}

IBone *Label::bone(int index) const
{
    if ((m_type == kSpecialBoneCategoryLabel || m_type == kBoneCategoryLabel) &&
            internal::checkBound(index, 0, m_boneRefs.count())) {
        Bone *bone = m_boneRefs[index];
        return bone;
    }
    return 0;
}

IMorph *Label::morph(int index) const
{
    if (m_type == kMorphCategoryLabel && internal::checkBound(index, 0, m_morphRefs.count())) {
        Morph *morph = m_morphRefs[index];
        return morph;
    }
    return 0;
}

IModel *Label::parentModelRef() const
{
    return m_modelRef;
}

int Label::index() const
{
    return m_index;
}

Label::Type Label::type() const
{
    return m_type;
}

void Label::setIndex(int value)
{
    m_index = value;
}

}
}
