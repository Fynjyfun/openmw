#ifndef MWGUI_STATSWATCHER_H
#define MWGUI_STATSWATCHER_H

#include <set>
#include <string>
#include <string_view>
#include <vector>

#include <components/esm/attr.hpp>
#include <components/esm/refid.hpp>
#include <components/esm3/loadskil.hpp>

#include "../mwmechanics/stat.hpp"

#include "../mwworld/ptr.hpp"

namespace MWGui
{
    class StatsListener
    {
    public:
        /// Set value for the given ID.
        virtual void setValue(std::string_view id, const MWMechanics::AttributeValue& value) {}
        virtual void setValue(std::string_view id, const MWMechanics::DynamicStat<float>& value) {}
        virtual void setValue(std::string_view, const std::string& value) {}
        virtual void setValue(std::string_view, int value) {}
        virtual void setValue(const ESM::Skill::SkillEnum parSkill, const MWMechanics::SkillValue& value) {}
        virtual void configureSkills(const std::vector<int>& major, const std::vector<int>& minor) {}
    };

    class StatsWatcher
    {
        MWWorld::Ptr mWatched;

        MWMechanics::AttributeValue mWatchedAttributes[ESM::Attribute::Length];
        MWMechanics::SkillValue mWatchedSkills[ESM::Skill::Length];

        MWMechanics::DynamicStat<float> mWatchedHealth;
        MWMechanics::DynamicStat<float> mWatchedMagicka;
        MWMechanics::DynamicStat<float> mWatchedFatigue;

        std::string mWatchedName;
        ESM::RefId mWatchedRace;
        ESM::RefId mWatchedClass;

        int mWatchedLevel;

        float mWatchedTimeToStartDrowning;

        bool mWatchedStatsEmpty;

        std::set<StatsListener*> mListeners;

        void setValue(std::string_view id, const MWMechanics::AttributeValue& value);
        void setValue(std::string_view id, const MWMechanics::DynamicStat<float>& value);
        void setValue(std::string_view id, const std::string& value);
        void setValue(std::string_view id, int value);
        void setValue(const ESM::Skill::SkillEnum parSkill, const MWMechanics::SkillValue& value);
        void configureSkills(const std::vector<int>& major, const std::vector<int>& minor);

    public:
        StatsWatcher();

        void update();
        void addListener(StatsListener* listener);
        void removeListener(StatsListener* listener);

        void watchActor(const MWWorld::Ptr& ptr);
        MWWorld::Ptr getWatchedActor() const { return mWatched; }

        void forceUpdate() { mWatchedStatsEmpty = true; }
    };
}

#endif
