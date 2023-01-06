#include "spellicons.hpp"

#include <algorithm>
#include <iomanip>
#include <list>
#include <sstream>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

#include <MyGUI_Align.h>
#include <MyGUI_ImageBox.h>
#include <MyGUI_StringUtility.h>
#include <MyGUI_Types.h>
#include <MyGUI_Widget.h>

#include <apps/openmw/mwmechanics/activespells.hpp>
#include <apps/openmw/mwmechanics/magiceffects.hpp>
#include <apps/openmw/mwworld/ptr.hpp>
#include <apps/openmw/mwworld/store.hpp>

#include <components/esm/attr.hpp>
#include <components/esm3/activespells.hpp>
#include <components/esm3/loadgmst.hpp>
#include <components/esm3/loadmgef.hpp>
#include <components/esm3/loadskil.hpp>
#include <components/esm3/variant.hpp>
#include <components/misc/notnullptr.hpp>
#include <components/misc/resourcehelpers.hpp>
#include <components/resource/resourcesystem.hpp>
#include <components/settings/settings.hpp>

#include "../mwbase/environment.hpp"
#include "../mwbase/windowmanager.hpp"
#include "../mwbase/world.hpp"

#include "../mwworld/class.hpp"
#include "../mwworld/esmstore.hpp"

#include "../mwmechanics/actorutil.hpp"
#include "../mwmechanics/creaturestats.hpp"

#include "tooltips.hpp"

namespace MWGui
{
    void SpellIcons::updateWidgets(MyGUI::Widget* parent, bool adjustSize)
    {
        MWWorld::Ptr player = MWMechanics::getPlayer();
        const MWMechanics::CreatureStats& stats = player.getClass().getCreatureStats(player);

        std::map<int, std::vector<MagicEffectInfo>> effects;
        for (const auto& params : stats.getActiveSpells())
        {
            for (const auto& effect : params.getEffects())
            {
                if (!(effect.mFlags & ESM::ActiveEffect::Flag_Applied))
                    continue;
                MagicEffectInfo newEffectSource;
                newEffectSource.mKey = MWMechanics::EffectKey(effect.mEffectId, effect.mArg);
                newEffectSource.mMagnitude = static_cast<int>(effect.mMagnitude);
                newEffectSource.mPermanent = effect.mDuration == -1.f;
                newEffectSource.mRemainingTime = effect.mTimeLeft;
                newEffectSource.mSource = params.getDisplayName();
                newEffectSource.mTotalTime = effect.mDuration;
                effects[effect.mEffectId].push_back(newEffectSource);
            }
        }

        int w = 2;

        for (const auto& [effectId, effectInfos] : effects)
        {
            const ESM::MagicEffect* effect
                = MWBase::Environment::get().getWorld()->getStore().get<ESM::MagicEffect>().find(effectId);

            float remainingDuration = 0;
            float totalDuration = 0;

            std::string sourcesDescription;

            static const float fadeTime = MWBase::Environment::get()
                                              .getWorld()
                                              ->getStore()
                                              .get<ESM::GameSetting>()
                                              .find("fMagicStartIconBlink")
                                              ->mValue.getFloat();

            bool addNewLine = false;
            for (const MagicEffectInfo& effectInfo : effectInfos)
            {
                if (addNewLine)
                    sourcesDescription += '\n';

                // if at least one of the effect sources is permanent, the effect will never wear off
                if (effectInfo.mPermanent)
                {
                    remainingDuration = fadeTime;
                    totalDuration = fadeTime;
                }
                else
                {
                    remainingDuration = std::max(remainingDuration, effectInfo.mRemainingTime);
                    totalDuration = std::max(totalDuration, effectInfo.mTotalTime);
                }

                sourcesDescription += effectInfo.mSource;

                if (effect->mData.mFlags & ESM::MagicEffect::TargetSkill)
                {
                    sourcesDescription += " (";
                    sourcesDescription += MWBase::Environment::get().getWindowManager()->getGameSettingString(
                        ESM::Skill::sSkillNameIds[effectInfo.mKey.mArg], {});
                    sourcesDescription += ')';
                }
                if (effect->mData.mFlags & ESM::MagicEffect::TargetAttribute)
                {
                    sourcesDescription += " (";
                    sourcesDescription += MWBase::Environment::get().getWindowManager()->getGameSettingString(
                        ESM::Attribute::sGmstAttributeIds[effectInfo.mKey.mArg], {});
                    sourcesDescription += ')';
                }
                ESM::MagicEffect::MagnitudeDisplayType displayType = effect->getMagnitudeDisplayType();
                if (displayType == ESM::MagicEffect::MDT_TimesInt)
                {
                    std::string_view timesInt
                        = MWBase::Environment::get().getWindowManager()->getGameSettingString("sXTimesINT", {});
                    std::stringstream formatter;
                    formatter << std::fixed << std::setprecision(1) << " " << (effectInfo.mMagnitude / 10.0f)
                              << timesInt;
                    sourcesDescription += formatter.str();
                }
                else if (displayType != ESM::MagicEffect::MDT_None)
                {
                    sourcesDescription += ": " + MyGUI::utility::toString(effectInfo.mMagnitude);

                    if (displayType == ESM::MagicEffect::MDT_Percentage)
                        sourcesDescription
                            += MWBase::Environment::get().getWindowManager()->getGameSettingString("spercent", {});
                    else if (displayType == ESM::MagicEffect::MDT_Feet)
                    {
                        sourcesDescription += ' ';
                        sourcesDescription
                            += MWBase::Environment::get().getWindowManager()->getGameSettingString("sfeet", {});
                    }
                    else if (displayType == ESM::MagicEffect::MDT_Level)
                    {
                        sourcesDescription += ' ';
                        if (effectInfo.mMagnitude > 1)
                            sourcesDescription
                                += MWBase::Environment::get().getWindowManager()->getGameSettingString("sLevels", {});
                        else
                            sourcesDescription
                                += MWBase::Environment::get().getWindowManager()->getGameSettingString("sLevel", {});
                    }
                    else // ESM::MagicEffect::MDT_Points
                    {
                        sourcesDescription += ' ';
                        if (effectInfo.mMagnitude > 1)
                            sourcesDescription
                                += MWBase::Environment::get().getWindowManager()->getGameSettingString("spoints", {});
                        else
                            sourcesDescription
                                += MWBase::Environment::get().getWindowManager()->getGameSettingString("spoint", {});
                    }
                }
                if (effectInfo.mRemainingTime > -1 && Settings::Manager::getBool("show effect duration", "Game"))
                    sourcesDescription
                        += MWGui::ToolTips::getDurationString(effectInfo.mRemainingTime, " #{sDuration}");

                addNewLine = true;
            }

            if (remainingDuration > 0.f)
            {
                MyGUI::ImageBox* image;
                if (mWidgetMap.find(effectId) == mWidgetMap.end())
                {
                    image = parent->createWidget<MyGUI::ImageBox>(
                        "ImageBox", MyGUI::IntCoord(w, 2, 16, 16), MyGUI::Align::Default);
                    mWidgetMap[effectId] = image;

                    image->setImageTexture(Misc::ResourceHelpers::correctIconPath(
                        effect->mIcon, MWBase::Environment::get().getResourceSystem()->getVFS()));

                    const std::string& name = ESM::MagicEffect::effectIdToString(effectId);

                    ToolTipInfo tooltipInfo;
                    tooltipInfo.caption = "#{" + name + "}";
                    tooltipInfo.icon = effect->mIcon;
                    tooltipInfo.imageSize = 16;
                    tooltipInfo.wordWrap = false;

                    image->setUserData(tooltipInfo);
                    image->setUserString("ToolTipType", "ToolTipInfo");
                }
                else
                    image = mWidgetMap[effectId];

                image->setPosition(w, 2);
                image->setVisible(true);
                w += 16;

                ToolTipInfo* tooltipInfo = image->getUserData<ToolTipInfo>();
                tooltipInfo->text = sourcesDescription;

                // Fade out
                if (totalDuration >= fadeTime && fadeTime > 0.f)
                    image->setAlpha(std::min(remainingDuration / fadeTime, 1.f));
            }
            else if (mWidgetMap.find(effectId) != mWidgetMap.end())
            {
                MyGUI::ImageBox* image = mWidgetMap[effectId];
                image->setVisible(false);
                image->setAlpha(1.f);
            }
        }

        if (adjustSize)
        {
            int s = w + 2;
            if (effects.empty())
                s = 0;
            int diff = parent->getWidth() - s;
            parent->setSize(s, parent->getHeight());
            parent->setPosition(parent->getLeft() + diff, parent->getTop());
        }

        // hide inactive effects
        for (auto& widgetPair : mWidgetMap)
        {
            if (effects.find(widgetPair.first) == effects.end())
                widgetPair.second->setVisible(false);
        }
    }

}
