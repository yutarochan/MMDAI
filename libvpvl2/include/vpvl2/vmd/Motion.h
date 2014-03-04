/**

 Copyright (c) 2009-2011  Nagoya Institute of Technology
                          Department of Computer Science
               2010-2014  hkrn

 All rights reserved.

 Redistribution and use in source and binary forms, with or
 without modification, are permitted provided that the following
 conditions are met:

 - Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
 - Redistributions in binary form must reproduce the above
   copyright notice, this list of conditions and the following
   disclaimer in the documentation and/or other materials provided
   with the distribution.
 - Neither the name of the MMDAI project team nor the names of
   its contributors may be used to endorse or promote products
   derived from this software without specific prior written
   permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.

*/

#pragma once
#ifndef VPVL2_VMD_VMDMOTION_H_
#define VPVL2_VMD_VMDMOTION_H_

#include "vpvl2/IEncoding.h"
#include "vpvl2/IMotion.h"
#include "vpvl2/vmd/BoneAnimation.h"
#include "vpvl2/vmd/CameraAnimation.h"
#include "vpvl2/vmd/LightAnimation.h"
#include "vpvl2/vmd/MorphAnimation.h"

namespace vpvl2
{
namespace VPVL2_VERSION_NS
{
namespace vmd
{

/**
 * @file
 * @author Nagoya Institute of Technology Department of Computer Science
 * @author hkrn
 *
 * @section DESCRIPTION
 *
 * Bone class represents a Vocaloid Motion Data object, set of bone, face and camera motion.
 */

class VPVL2_API Motion VPVL2_DECL_FINAL : public IMotion
{
public:
    struct DataInfo
    {
        const uint8 *basePtr;
        const uint8 *namePtr;
        const uint8 *boneKeyframePtr;
        int boneKeyframeCount;
        const uint8 *morphKeyframePtr;
        int morphKeyframeCount;
        const uint8 *cameraKeyframePtr;
        int cameraKeyframeCount;
        const uint8 *lightKeyframePtr;
        int lightKeyframeCount;
        const uint8 *selfShadowKeyframePtr;
        int selfShadowKeyframeCount;
        const uint8 *modelKeyframePtr;
        int modelKeyframeCount;
    };

    static const uint8 *kSignature;
    static const int kSignatureSize = 30;
    static const int kNameSize = 20;

    Motion(IModel *modelRef, IEncoding *encodingRef);
    ~Motion();

    bool preparse(const uint8 *data, vsize size, DataInfo &info);
    bool load(const uint8 *data, vsize size);
    void save(uint8 *data) const;
    vsize estimateSize() const;
    void setParentSceneRef(Scene *value);
    void setParentModelRef(IModel *value);
    void seekSeconds(const float64 &seconds);
    void seekSceneSeconds(const float64 &seconds, Scene *scene);
    void seekTimeIndex(const IKeyframe::TimeIndex &timeIndex);
    void seekSceneTimeIndex(const IKeyframe::TimeIndex &timeIndex, Scene *scene);
    void advance(const IKeyframe::TimeIndex &deltaTimeIndex);
    void advanceScene(const IKeyframe::TimeIndex &deltaTimeIndex, Scene *scene);
    void reload();
    void reset();
    float64 durationSeconds() const;
    IKeyframe::TimeIndex durationTimeIndex() const;
    bool isReachedTo(const IKeyframe::TimeIndex &atEnd) const;
    bool isNullFrameEnabled() const;
    void setNullFrameEnable(bool value);

    IBoneKeyframe *createBoneKeyframe();
    ICameraKeyframe *createCameraKeyframe();
    IEffectKeyframe *createEffectKeyframe();
    ILightKeyframe *createLightKeyframe();
    IModelKeyframe *createModelKeyframe();
    IMorphKeyframe *createMorphKeyframe();
    IProjectKeyframe *createProjectKeyframe();
    void addKeyframe(IKeyframe *value);
    int countKeyframes(IKeyframe::Type value) const;
    void getKeyframeRefs(const IKeyframe::TimeIndex &timeIndex,
                      const IKeyframe::LayerIndex &layerIndex,
                      IKeyframe::Type type,
                      Array<IKeyframe *> &keyframes);
    IKeyframe::LayerIndex countLayers(const IString *name,
                                      IKeyframe::Type type) const;
    IBoneKeyframe *findBoneKeyframeRef(const IKeyframe::TimeIndex &timeIndex,
                                    const IString *name,
                                    const IKeyframe::LayerIndex &layerIndex) const;
    IBoneKeyframe *findBoneKeyframeRefAt(int index) const;
    ICameraKeyframe *findCameraKeyframeRef(const IKeyframe::TimeIndex &timeIndex,
                                        const IKeyframe::LayerIndex &layerIndex) const;
    ICameraKeyframe *findCameraKeyframeRefAt(int index) const;
    IEffectKeyframe *findEffectKeyframeRef(const IKeyframe::TimeIndex &timeIndex,
                                        const IString *name,
                                        const IKeyframe::LayerIndex &layerIndex) const;
    IEffectKeyframe *findEffectKeyframeRefAt(int index) const;
    ILightKeyframe *findLightKeyframeRef(const IKeyframe::TimeIndex &timeIndex,
                                      const IKeyframe::LayerIndex &layerIndex) const;
    ILightKeyframe *findLightKeyframeRefAt(int index) const;
    IModelKeyframe *findModelKeyframeRef(const IKeyframe::TimeIndex &timeIndex,
                                      const IKeyframe::LayerIndex &layerIndex) const;
    IModelKeyframe *findModelKeyframeRefAt(int index) const;
    IMorphKeyframe *findMorphKeyframeRef(const IKeyframe::TimeIndex &timeIndex,
                                      const IString *name,
                                      const IKeyframe::LayerIndex &layerIndex) const;
    IMorphKeyframe *findMorphKeyframeRefAt(int index) const;
    IProjectKeyframe *findProjectKeyframeRef(const IKeyframe::TimeIndex &timeIndex,
                                          const IKeyframe::LayerIndex &layerIndex) const;
    IProjectKeyframe *findProjectKeyframeRefAt(int index) const;
    void replaceKeyframe(IKeyframe *value, bool alsoDelete);
    void removeKeyframe(IKeyframe *value);
    void deleteKeyframe(IKeyframe *&value);
    void deleteKeyframes(const IKeyframe::TimeIndex &timeIndex, IKeyframe::Type type);
    void update(IKeyframe::Type type);
    void getAllKeyframeRefs(Array<IKeyframe *> &value, IKeyframe::Type type);
    void setAllKeyframes(const Array<IKeyframe *> &value, IKeyframe::Type type);
    IMotion *clone() const;

    const IString *name() const;
    Scene *parentSceneRef() const;
    IModel *parentModelRef() const;
    Error error() const;
    const BoneAnimation &boneAnimation() const;
    const CameraAnimation &cameraAnimation() const;
    const MorphAnimation &morphAnimation() const;
    const LightAnimation &lightAnimation() const;
    const DataInfo &result() const;
    BoneAnimation *mutableBoneAnimation();
    CameraAnimation *mutableCameraAnimation();
    MorphAnimation *mutableMorphAnimation();
    LightAnimation *mutableLightAnimation();
    bool isActive() const;
    FormatType type() const;

private:
    struct PrivateContext;
    PrivateContext *m_context;

    VPVL2_DISABLE_COPY_AND_ASSIGN(Motion)
};

} /* namespace vmd */
} /* namespace VPVL2_VERSION_NS */
} /* namespace vpvl2 */

#endif
