#include "actor.hpp"

#include <memory>
#include <unordered_map>
#include <utility>

#include <osg/Group>
#include <osg/MatrixTransform>
#include <osg/Node>

#include <apps/opencs/model/world/actoradapter.hpp>
#include <apps/opencs/model/world/idcollection.hpp>
#include <apps/opencs/model/world/record.hpp>

#include <components/esm3/loadbody.hpp>
#include <components/esm3/mappings.hpp>
#include <components/misc/resourcehelpers.hpp>
#include <components/resource/resourcesystem.hpp>
#include <components/resource/scenemanager.hpp>
#include <components/sceneutil/attach.hpp>
#include <components/sceneutil/skeleton.hpp>

#include "../../model/world/data.hpp"

namespace CSVRender
{
    const std::string Actor::MeshPrefix = "meshes\\";

    Actor::Actor(const ESM::RefId& id, CSMWorld::Data& data)
        : mId(id)
        , mData(data)
        , mBaseNode(new osg::Group())
        , mSkeleton(nullptr)
    {
        mActorData = mData.getActorAdapter()->getActorData(mId);
        connect(mData.getActorAdapter(), &CSMWorld::ActorAdapter::actorChanged, this, &Actor::handleActorChanged);
    }

    osg::Group* Actor::getBaseNode()
    {
        return mBaseNode;
    }

    void Actor::update()
    {
        mBaseNode->removeChildren(0, mBaseNode->getNumChildren());

        // Load skeleton
        std::string skeletonModel = mActorData->getSkeleton();
        skeletonModel
            = Misc::ResourceHelpers::correctActorModelPath(skeletonModel, mData.getResourceSystem()->getVFS());
        loadSkeleton(skeletonModel);

        if (!mActorData->isCreature())
        {
            // Get rid of the extra attachments
            SceneUtil::CleanObjectRootVisitor cleanVisitor;
            mSkeleton->accept(cleanVisitor);
            cleanVisitor.remove();

            // Attach parts to skeleton
            loadBodyParts();
        }
        else
        {
            SceneUtil::RemoveTriBipVisitor removeTriBipVisitor;
            mSkeleton->accept(removeTriBipVisitor);
            removeTriBipVisitor.remove();
        }

        // Post setup
        mSkeleton->markDirty();
        mSkeleton->setActive(SceneUtil::Skeleton::Active);
    }

    void Actor::handleActorChanged(const ESM::RefId& refId)
    {
        if (mId == refId)
        {
            update();
        }
    }

    void Actor::loadSkeleton(const std::string& model)
    {
        auto sceneMgr = mData.getResourceSystem()->getSceneManager();

        osg::ref_ptr<osg::Node> temp = sceneMgr->getInstance(model);
        mSkeleton = dynamic_cast<SceneUtil::Skeleton*>(temp.get());
        if (!mSkeleton)
        {
            mSkeleton = new SceneUtil::Skeleton();
            mSkeleton->addChild(temp);
        }
        mBaseNode->addChild(mSkeleton);

        // Map bone names to bones
        mNodeMap.clear();
        SceneUtil::NodeMapVisitor nmVisitor(mNodeMap);
        mSkeleton->accept(nmVisitor);
    }

    void Actor::loadBodyParts()
    {
        for (int i = 0; i < ESM::PRT_Count; ++i)
        {
            auto type = (ESM::PartReferenceType)i;
            const std::string_view partId = mActorData->getPart(type);
            attachBodyPart(type, getBodyPartMesh(partId));
        }
    }

    void Actor::attachBodyPart(ESM::PartReferenceType type, const std::string& mesh)
    {
        auto sceneMgr = mData.getResourceSystem()->getSceneManager();

        // Attach to skeleton
        std::string boneName = ESM::getBoneName(type);
        auto node = mNodeMap.find(boneName);
        if (!mesh.empty() && node != mNodeMap.end())
        {
            auto instance = sceneMgr->getInstance(mesh);
            SceneUtil::attach(instance, mSkeleton, boneName, node->second, sceneMgr);
        }
    }

    std::string Actor::getBodyPartMesh(std::string_view bodyPartId)
    {
        const auto& bodyParts = mData.getBodyParts();

        const int index = bodyParts.searchId(ESM::RefId::stringRefId(bodyPartId));
        if (index != -1 && !bodyParts.getRecord(index).isDeleted())
            return MeshPrefix + bodyParts.getRecord(index).get().mModel;
        else
            return "";
    }
}
