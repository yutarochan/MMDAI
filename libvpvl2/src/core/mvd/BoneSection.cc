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

#include "vpvl2/mvd/BoneKeyframe.h"
#include "vpvl2/mvd/BoneSection.h"
#include "vpvl2/mvd/NameListSection.h"

namespace vpvl2
{
namespace mvd
{

#pragma pack(push, 1)

struct BoneSectionHeader {
    int key;
    int sizeOfKeyframe;
    int countOfKeyframes;
    int countOfLayers;
};

#pragma pack(pop)

class BoneSection::PrivateContext : public BaseSectionContext {
public:
    IBone *boneRef;
    Vector3 position;
    Quaternion rotation;
    IKeyframe::LayerIndex countOfLayers;
    PrivateContext()
        : boneRef(0),
          position(kZeroV3),
          rotation(Quaternion::getIdentity()),
          countOfLayers(1)
    {
    }
    ~PrivateContext() {
        boneRef = 0;
        position.setZero();
        rotation.setValue(0, 0, 0, 1);
        countOfLayers = 0;
    }
    void seek(const IKeyframe::TimeIndex &timeIndex) {
        if (boneRef && keyframes.count() > 0) {
            int fromIndex, toIndex;
            IKeyframe::TimeIndex currentTimeIndex;
            findKeyframeIndices(timeIndex, currentTimeIndex, fromIndex, toIndex);
            const BoneKeyframe *keyframeFrom = reinterpret_cast<const BoneKeyframe *>(keyframes[fromIndex]),
                    *keyframeTo = reinterpret_cast<const BoneKeyframe *>(keyframes[toIndex]);
            const IKeyframe::TimeIndex &timeIndexFrom = keyframeFrom->timeIndex(), &timeIndexTo = keyframeTo->timeIndex();
            const Vector3 &positionFrom = keyframeFrom->localPosition(), &positionTo = keyframeTo->localPosition();
            const Quaternion &rotationFrom = keyframeFrom->localRotation(), &rotationTo = keyframeTo->localRotation();
            if (timeIndexFrom != timeIndexTo && timeIndexFrom < currentTimeIndex) {
                if (timeIndexTo <= currentTimeIndex) {
                    position = positionTo;
                    rotation = rotationTo;
                }
                else {
                    const IKeyframe::SmoothPrecision &weight = calculateWeight(currentTimeIndex, timeIndexFrom, timeIndexTo);
                    IKeyframe::SmoothPrecision x = 0, y = 0, z = 0;
                    interpolate(keyframeTo->tableForX(), positionFrom, positionTo, weight, 0, x);
                    interpolate(keyframeTo->tableForY(), positionFrom, positionTo, weight, 1, y);
                    interpolate(keyframeTo->tableForZ(), positionFrom, positionTo, weight, 2, z);
                    position.setValue(Scalar(x), Scalar(y), Scalar(z));
                    const Motion::InterpolationTable &tableForRotation = keyframeTo->tableForRotation();
                    if (tableForRotation.linear) {
                        rotation = rotationFrom.slerp(rotationTo, Scalar(weight));
                    }
                    else {
                        const IKeyframe::SmoothPrecision &weight2 = calculateInterpolatedWeight(tableForRotation, weight);
                        rotation = rotationFrom.slerp(rotationTo, Scalar(weight2));
                    }
                }
            }
            else {
                position = positionFrom;
                rotation = rotationFrom;
            }
            boneRef->setLocalPosition(position);
            boneRef->setLocalRotation(rotation);
        }
    }
};

BoneSection::BoneSection(const Motion *motionRef, IModel *modelRef)
    : BaseSection(motionRef),
      m_modelRef(modelRef),
      m_keyframePtr(0)
{
}

BoneSection::~BoneSection()
{
    release();
}

bool BoneSection::preparse(uint8_t *&ptr, size_t &rest, Motion::DataInfo &info)
{
    BoneSectionHeader header;
    if (!internal::validateSize(ptr, sizeof(header), rest)) {
        VPVL2_LOG(LOG(WARNING) << "Invalid size of MVDBoneSection header detected: " << rest);
        return false;
    }
    internal::getData(ptr - sizeof(header), header);
    if (!internal::validateSize(ptr, sizeof(uint8_t), header.countOfLayers, rest)) {
        VPVL2_LOG(LOG(WARNING) << "Invalid size of MVDBoneSection layers detected: size=" << header.countOfLayers << " rest=" << rest);
        return false;
    }
    const int nkeyframes = header.countOfKeyframes;
    const size_t reserved = header.sizeOfKeyframe - BoneKeyframe::size();
    VPVL2_LOG(VLOG(2) << "MVDBoneSection(Header): key=" << header.key);
    VPVL2_LOG(VLOG(2) << "MVDBoneSection(Header): nkeyframes=" << nkeyframes);
    VPVL2_LOG(VLOG(2) << "MVDBoneSection(Header): nlayers=" << header.countOfLayers);
    VPVL2_LOG(VLOG(2) << "MVDBoneSection(Header): sizeofKeyframe=" << header.sizeOfKeyframe);
    VPVL2_LOG(VLOG(2) << "MVDBoneSection(Header): reserved=" << reserved);
    for (int i = 0; i < nkeyframes; i++) {
        if (!BoneKeyframe::preparse(ptr, rest, reserved, info)) {
            VPVL2_LOG(LOG(WARNING) << "Invalid size of MVDBoneSection key detected: index=" << i << " rest=" << rest);
            return false;
        }
    }
    return true;
}

void BoneSection::release()
{
    BaseSection::release();
    delete m_keyframePtr;
    m_keyframePtr = 0;
    m_allKeyframeRefs.clear();
    m_context2names.clear();
}

void BoneSection::read(const uint8_t *data)
{
    uint8_t *ptr = const_cast<uint8_t *>(data);
    BoneSectionHeader header;
    internal::getData(ptr, header);
    const size_t sizeOfKeyframe = header.sizeOfKeyframe;
    const int nkeyframes = header.countOfKeyframes;
    ptr += sizeof(header) + sizeof(uint8_t) * header.countOfLayers;
    const int key = header.key;
    const IString *name = m_nameListSectionRef->value(key);
    PrivateContext *contextPtr = m_name2contexts.insert(key, new PrivateContext());
    m_context2names.insert(contextPtr, key);
    contextPtr->keyframes.reserve(nkeyframes);
    for (int i = 0; i < nkeyframes; i++) {
        m_keyframePtr = contextPtr->keyframes.append(new BoneKeyframe(m_motionRef));
        m_keyframePtr->read(ptr);
        m_keyframePtr->setName(name);
        addKeyframe0(m_keyframePtr);
        ptr += sizeOfKeyframe;
    }
    contextPtr->keyframes.sort(KeyframeTimeIndexPredication());
    contextPtr->boneRef = m_modelRef ? m_modelRef->findBone(name) : 0;
    contextPtr->countOfLayers = header.countOfLayers;
    m_keyframePtr = 0;
}

void BoneSection::seek(const IKeyframe::TimeIndex &timeIndex)
{
    if (m_modelRef) {
        const int ncontexts = m_name2contexts.count();
        for (int i = 0; i < ncontexts; i++) {
            if (PrivateContext *const *context = m_name2contexts.value(i)) {
                (*context)->seek(timeIndex);
            }
        }
    }
    saveCurrentTimeIndex(timeIndex);
}

void BoneSection::setParentModel(IModel *modelRef)
{
    m_modelRef = modelRef;
    if (modelRef) {
        const int ncontexts = m_name2contexts.count();
        for (int i = 0; i < ncontexts; i++) {
            if (PrivateContext *const *context = m_name2contexts.value(i)) {
                PrivateContext *contextRef = *context;
                if (const int *keyPtr = m_context2names.find(contextRef)) {
                    int key = *keyPtr;
                    IBone *bone = modelRef->findBone(m_nameListSectionRef->value(key));
                    contextRef->boneRef = bone;
                }
                else {
                    contextRef->boneRef = 0;
                }
            }
        }
    }
}

void BoneSection::write(uint8_t *data) const
{
    const int ncontexts = m_name2contexts.count();
    for (int i = 0; i < ncontexts; i++) {
        const PrivateContext *const *context = m_name2contexts.value(i);
        const PrivateContext *contextRef = *context;
        const IBone *boneRef = contextRef->boneRef;
        if (boneRef) {
            const PrivateContext::KeyframeCollection &keyframes = contextRef->keyframes;
            const int nkeyframes = keyframes.count();
            const int nlayers = contextRef->countOfLayers;
            Motion::SectionTag tag;
            tag.type = Motion::kBoneSection;
            tag.minor = 0;
            internal::writeBytes(reinterpret_cast<const uint8_t *>(&tag), sizeof(tag), data);
            BoneSectionHeader header;
            header.countOfKeyframes = nkeyframes;
            header.countOfLayers = nlayers;
            header.key = m_nameListSectionRef->key(boneRef->name());
            header.sizeOfKeyframe = BoneKeyframe::size();
            internal::writeBytes(reinterpret_cast<const uint8_t *>(&header), sizeof(header), data);
            for (int i = 0; i < nlayers; i++) {
                internal::writeSignedIndex(0, sizeof(uint8_t), data);
            }
            for (int i = 0 ; i < nkeyframes; i++) {
                const IKeyframe *keyframe = keyframes[i];
                keyframe->write(data);
                data += keyframe->estimateSize();
            }
        }
    }
}

size_t BoneSection::estimateSize() const
{
    size_t size = 0;
    const int ncontexts = m_name2contexts.count();
    for (int i = 0; i < ncontexts; i++) {
        const PrivateContext *const *context = m_name2contexts.value(i);
        const PrivateContext *contextRef = *context;
        if (contextRef->boneRef) {
            const PrivateContext::KeyframeCollection &keyframes = contextRef->keyframes;
            const int nkeyframes = keyframes.count();
            size += sizeof(Motion::SectionTag);
            size += sizeof(BoneSectionHeader);
            size += sizeof(uint8_t) * contextRef->countOfLayers;
            for (int i = 0 ; i < nkeyframes; i++) {
                const IKeyframe *keyframe = keyframes[i];
                size += keyframe->estimateSize();
            }
        }
    }
    return size;
}

size_t BoneSection::countKeyframes() const
{
    return m_allKeyframeRefs.count();
}

IKeyframe::LayerIndex BoneSection::countLayers(const IString *name) const
{
    PrivateContext *const *context = m_name2contexts.find(m_nameListSectionRef->key(name));
    return context ? (*context)->countOfLayers : 0;
}

void BoneSection::addKeyframe(IKeyframe *keyframe)
{
    int key = m_nameListSectionRef->key(keyframe->name());
    PrivateContext *const *context = m_name2contexts.find(key), *contextPtr = 0;
    if (context) {
        contextPtr = *context;
        contextPtr->keyframes.append(keyframe);
        addKeyframe0(keyframe);
    }
    else if (m_modelRef) {
        contextPtr = m_name2contexts.insert(key, new PrivateContext());
        contextPtr->boneRef = m_modelRef->findBone(keyframe->name());
        contextPtr->keyframes.append(keyframe);
        addKeyframe0(keyframe);
        m_name2contexts.insert(key, contextPtr);
        m_context2names.insert(contextPtr, key);
    }
}

void BoneSection::deleteKeyframe(IKeyframe *&keyframe)
{
    int key = m_nameListSectionRef->key(keyframe->name());
    PrivateContext *const *context = m_name2contexts.find(key);
    if (context) {
        PrivateContext *contextPtr = *context;
        contextPtr->keyframes.remove(keyframe);
        m_allKeyframeRefs.remove(keyframe);
        if (contextPtr->keyframes.count() == 0) {
            m_name2contexts.remove(key);
            m_context2names.remove(contextPtr);
            delete contextPtr;
        }
        delete keyframe;
        keyframe = 0;
    }
}

void BoneSection::getKeyframes(const IKeyframe::TimeIndex & /* timeIndex */,
                               const IKeyframe::LayerIndex & /* layerIndex */,
                               Array<IKeyframe *> & /* keyframes */)
{
}

IBoneKeyframe *BoneSection::findKeyframe(const IKeyframe::TimeIndex &timeIndex,
                                         const IString *name,
                                         const IKeyframe::LayerIndex &layerIndex) const
{
    PrivateContext *const *context = m_name2contexts.find(m_nameListSectionRef->key(name));
    if (context) {
        const PrivateContext::KeyframeCollection &keyframes = (*context)->keyframes;
        const int nkeyframes = keyframes.count();
        for (int i = 0; i < nkeyframes; i++) {
            BoneKeyframe *keyframe = reinterpret_cast<BoneKeyframe *>(keyframes[i]);
            if (keyframe->timeIndex() == timeIndex && keyframe->layerIndex() == layerIndex) {
                return keyframe;
            }
        }
    }
    return 0;
}

IBoneKeyframe *BoneSection::findKeyframeAt(int index) const
{
    if (internal::checkBound(index, 0, m_allKeyframeRefs.count())) {
        BoneKeyframe *keyframe = reinterpret_cast<BoneKeyframe *>(m_allKeyframeRefs[index]);
        return keyframe;
    }
    return 0;
}

void BoneSection::addKeyframe0(IKeyframe *keyframe)
{
    setMaxTimeIndex(keyframe);
    m_allKeyframeRefs.append(keyframe);
}

} /* namespace mvd */
} /* namespace vpvl2 */
