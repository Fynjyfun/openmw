/*
  OpenMW - The completely unofficial reimplementation of Morrowind
  Copyright (C) 2008-2010  Nicolay Korslund
  Email: < korslund@gmail.com >
  WWW: https://openmw.org/

  This file (data.h) is part of the OpenMW package.

  OpenMW is distributed as free software: you can redistribute it
  and/or modify it under the terms of the GNU General Public License
  version 3, as published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License
  version 3 along with this program. If not, see
  https://www.gnu.org/licenses/ .

 */

#ifndef OPENMW_COMPONENTS_NIF_DATA_HPP
#define OPENMW_COMPONENTS_NIF_DATA_HPP

#include "nifkey.hpp"
#include "niftypes.hpp" // Transformation
#include "recordptr.hpp"
#include <components/nif/node.hpp>

namespace Nif
{

    // Common ancestor for several data classes
    struct NiGeometryData : public Record
    {
        std::vector<osg::Vec3f> vertices, normals, tangents, bitangents;
        std::vector<osg::Vec4f> colors;
        std::vector<std::vector<osg::Vec2f>> uvlist;
        osg::Vec3f center;
        float radius;

        void read(NIFStream* nif) override;
    };

    // Abstract
    struct NiTriBasedGeomData : public NiGeometryData
    {
        uint16_t mNumTriangles;

        void read(NIFStream* nif) override;
    };

    struct NiTriShapeData : public NiTriBasedGeomData
    {
        // Triangles, three vertex indices per triangle
        std::vector<unsigned short> triangles;

        void read(NIFStream* nif) override;
    };

    struct NiTriStripsData : public NiTriBasedGeomData
    {
        // Triangle strips, series of vertex indices.
        std::vector<std::vector<unsigned short>> strips;

        void read(NIFStream* nif) override;
    };

    struct NiLinesData : public NiGeometryData
    {
        // Lines, series of indices that correspond to connected vertices.
        // NB: assumes <=65536 number of vertices
        std::vector<uint16_t> mLines;

        void read(NIFStream* nif) override;
    };

    struct NiParticlesData : public NiGeometryData
    {
        int numParticles{ 0 };

        int activeCount{ 0 };

        std::vector<float> particleRadii, sizes, rotationAngles;
        std::vector<osg::Quat> rotations;
        std::vector<osg::Vec3f> rotationAxes;

        void read(NIFStream* nif) override;
    };

    struct NiRotatingParticlesData : public NiParticlesData
    {
        void read(NIFStream* nif) override;
    };

    struct NiPosData : public Record
    {
        Vector3KeyMapPtr mKeyList;

        void read(NIFStream* nif) override;
    };

    struct NiUVData : public Record
    {
        std::array<FloatKeyMapPtr, 4> mKeyList;

        void read(NIFStream* nif) override;
    };

    struct NiFloatData : public Record
    {
        FloatKeyMapPtr mKeyList;

        void read(NIFStream* nif) override;
    };

    struct NiPixelData : public Record
    {
        enum Format
        {
            NIPXFMT_RGB8,
            NIPXFMT_RGBA8,
            NIPXFMT_PAL8,
            NIPXFMT_PALA8,
            NIPXFMT_BGR8,
            NIPXFMT_BGRA8,
            NIPXFMT_DXT1,
            NIPXFMT_DXT3,
            NIPXFMT_DXT5
        };
        Format fmt{ NIPXFMT_RGB8 };

        unsigned int colorMask[4]{ 0 };
        unsigned int bpp{ 0 }, pixelTiling{ 0 };
        bool sRGB{ false };

        NiPalettePtr palette;
        unsigned int numberOfMipmaps{ 0 };

        struct Mipmap
        {
            int width, height;
            int dataOffset;
        };
        std::vector<Mipmap> mipmaps;

        std::vector<unsigned char> data;

        void read(NIFStream* nif) override;
        void post(Reader& nif) override;
    };

    struct NiColorData : public Record
    {
        Vector4KeyMapPtr mKeyMap;

        void read(NIFStream* nif) override;
    };

    struct NiVisData : public Record
    {
        struct VisData
        {
            float time;
            bool isSet;
        };
        std::vector<VisData> mVis;

        void read(NIFStream* nif) override;
    };

    struct NiSkinInstance : public Record
    {
        NiSkinDataPtr mData;
        NiSkinPartitionPtr mPartitions;
        NodePtr mRoot;
        NodeList mBones;

        void read(NIFStream* nif) override;
        void post(Reader& nif) override;
    };

    struct BSDismemberSkinInstance : public NiSkinInstance
    {
        struct BodyPart
        {
            uint16_t mFlags;
            uint16_t mType;
        };

        std::vector<BodyPart> mParts;

        void read(NIFStream* nif) override;
    };

    struct NiSkinData : public Record
    {
        struct VertWeight
        {
            unsigned short vertex;
            float weight;
        };

        struct BoneInfo
        {
            Transformation trafo;
            osg::Vec3f boundSphereCenter;
            float boundSphereRadius;
            std::vector<VertWeight> weights;
        };

        Transformation trafo;
        std::vector<BoneInfo> bones;
        NiSkinPartitionPtr partitions;

        void read(NIFStream* nif) override;
        void post(Reader& nif) override;
    };

    struct NiSkinPartition : public Record
    {
        struct Partition
        {
            std::vector<unsigned short> bones;
            std::vector<unsigned short> vertexMap;
            std::vector<float> weights;
            std::vector<std::vector<unsigned short>> strips;
            std::vector<unsigned short> triangles;
            std::vector<unsigned short> trueTriangles;
            std::vector<char> boneIndices;
            BSVertexDesc mVertexDesc;

            void read(NIFStream* nif);
            std::vector<unsigned short> getTrueTriangles() const;
            std::vector<std::vector<unsigned short>> getTrueStrips() const;
        };
        unsigned int mPartitionNum;
        std::vector<Partition> mPartitions;

        unsigned int mDataSize;
        unsigned int mVertexSize;
        BSVertexDesc mVertexDesc;
        std::vector<BSVertexData> mVertexData;

        void read(NIFStream* nif) override;
    };

    struct NiMorphData : public Record
    {
        struct MorphData
        {
            FloatKeyMapPtr mKeyFrames;
            std::vector<osg::Vec3f> mVertices;
        };

        uint8_t mRelativeTargets;
        std::vector<MorphData> mMorphs;

        void read(NIFStream* nif) override;
    };

    struct NiKeyframeData : public Record
    {
        QuaternionKeyMapPtr mRotations;

        // may be NULL
        FloatKeyMapPtr mXRotations;
        FloatKeyMapPtr mYRotations;
        FloatKeyMapPtr mZRotations;

        Vector3KeyMapPtr mTranslations;
        FloatKeyMapPtr mScales;

        enum class AxisOrder
        {
            Order_XYZ = 0,
            Order_XZY = 1,
            Order_YZX = 2,
            Order_YXZ = 3,
            Order_ZXY = 4,
            Order_ZYX = 5,
            Order_XYX = 6,
            Order_YZY = 7,
            Order_ZXZ = 8
        };

        AxisOrder mAxisOrder{ AxisOrder::Order_XYZ };

        void read(NIFStream* nif) override;
    };

    struct NiPalette : public Record
    {
        // 32-bit RGBA colors that correspond to 8-bit indices
        std::vector<uint32_t> mColors;

        void read(NIFStream* nif) override;
    };

    struct NiStringPalette : public Record
    {
        std::string mPalette;

        void read(NIFStream* nif) override;
    };

    struct NiBoolData : public Record
    {
        ByteKeyMapPtr mKeyList;
        void read(NIFStream* nif) override;
    };

    struct BSMultiBound : public Record
    {
        BSMultiBoundDataPtr mData;

        void read(NIFStream* nif) override;
        void post(Reader& nif) override;
    };

    // Abstract
    struct BSMultiBoundData : public Record
    {
    };

    struct BSMultiBoundOBB : public BSMultiBoundData
    {
        osg::Vec3f mCenter;
        osg::Vec3f mSize;
        Nif::Matrix3 mRotation;

        void read(NIFStream* nif) override;
    };

    struct BSMultiBoundSphere : public BSMultiBoundData
    {
        osg::Vec3f mCenter;
        float mRadius;

        void read(NIFStream* nif) override;
    };

} // Namespace
#endif
