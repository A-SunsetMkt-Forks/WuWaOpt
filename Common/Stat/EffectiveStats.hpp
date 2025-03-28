//
// Created by EMCJava on 8/5/2024.
//

#pragma once

#include <Opt/Config/SkillMultiplierConfig.hpp>

#include <Common/Types.hpp>
#include <Common/ElementType.hpp>

#include "EchoSet.hpp"

#include <yaml-cpp/yaml.h>

// Some character might use defence/health as the base for calculation
enum class StatsFoundation {
    eFoundationAttack,
    eFoundationHealth,
    eFoundationDefence,
};

struct StatValueConfig;
struct EffectiveStats {

    EchoSet Set    = EchoSet::eEchoSetNone;
    int     NameID = 0;
    int     Cost   = 0;

    // attack/health/defence can be "union" like buff_multiplier, but now we have caching, this should not be that big of a deal
    FloatTy flat_attack       = 0;
    FloatTy percentage_attack = 0;

    FloatTy flat_health       = 0;
    FloatTy percentage_health = 0;

    FloatTy flat_defence       = 0;
    FloatTy percentage_defence = 0;

    FloatTy regen = 0;

    // Only one elemental buffer used
    FloatTy buff_multiplier = 0;

    FloatTy crit_rate   = 0;
    FloatTy crit_damage = 0;

    FloatTy auto_attack_buff  = 0;
    FloatTy heavy_attack_buff = 0;
    FloatTy skill_buff        = 0;
    FloatTy ult_buff          = 0;
    FloatTy heal_buff         = 0;

    EffectiveStats&              operator+=( const EffectiveStats& Other ) noexcept;
    [[nodiscard]] EffectiveStats operator+( const EffectiveStats& Other ) const noexcept;
    [[nodiscard]] EffectiveStats operator+( const StatValueConfig& StatValue ) const noexcept;
    [[nodiscard]] EffectiveStats operator-( const EffectiveStats& Other ) const noexcept;
    [[nodiscard]] bool           operator==( const EffectiveStats& Other ) const noexcept;

    static constexpr FloatTy CharacterDefaultRegen      = 1;
    static constexpr FloatTy CharacterDefaultCritRate   = 0.05;
    static constexpr FloatTy CharacterDefaultCritDamage = 1.5;

    [[nodiscard]] FloatTy RegenStat( ) const noexcept;
    [[nodiscard]] FloatTy CritRateStat( ) const noexcept;
    [[nodiscard]] FloatTy CritDamageStat( ) const noexcept;

    /*
     * StatsFoundation: Attack / Defence / Health stats from character + weapon base stat
     * OptimizingDamagePercentage: Defence / Resistance from enemy
     */
    [[nodiscard]] FloatTy FoundationStat( StatsFoundation character_foundation, FloatTy /* From character / weapon? */ foundation_base ) const noexcept;

    [[nodiscard]] FloatTy HealingAmount( StatsFoundation character_foundation, FloatTy /* From character / weapon? */ foundation_base, const SkillMultiplierConfig* multiplier_config, const SkillMultiplierConfig* deepen_config ) const noexcept;
    [[nodiscard]] FloatTy NormalDamage( StatsFoundation character_foundation, FloatTy /* From character / weapon? */ foundation_base, const SkillMultiplierConfig* multiplier_config, const SkillMultiplierConfig* deepen_config ) const noexcept;
    [[nodiscard]] FloatTy CritDamage( StatsFoundation character_foundation, FloatTy /* From character / weapon? */ foundation_base, const SkillMultiplierConfig* multiplier_config, const SkillMultiplierConfig* deepen_config ) const noexcept;
    [[nodiscard]] FloatTy ExpectedDamage( StatsFoundation character_foundation, FloatTy /* From character / weapon? */ foundation_base, const SkillMultiplierConfig* multiplier_config, const SkillMultiplierConfig* deepen_config ) const noexcept;
    [[nodiscard]] FloatTy OptimizingValue( StatsFoundation character_foundation, FloatTy /* From character / weapon? */ foundation_base, FloatTy OptimizingDamagePercentage, const SkillMultiplierConfig* multiplier_config, const SkillMultiplierConfig* deepen_config ) const noexcept;
    void                  ExtractOptimizingStats( StatsFoundation character_foundation, FloatTy /* From character / weapon? */ foundation_base, FloatTy OptimizingDamagePercentage, const SkillMultiplierConfig* multiplier_config, const SkillMultiplierConfig* deepen_config,
                                                  FloatTy& HA, FloatTy& ND, FloatTy& CD, FloatTy& ED, FloatTy& OV ) const noexcept;

    std::string_view   GetSetName( ) const noexcept;
    static const char* GetStatName( const FloatTy EffectiveStats::* stat_type );
};

struct StatValueConfig {
    FloatTy EffectiveStats::* ValuePtr = { };
    FloatTy                   Value    = { };
};

YAML::Node ToNode( const EffectiveStats& rhs ) noexcept;
bool       FromNode( const YAML::Node& Node, EffectiveStats& rhs ) noexcept;

namespace YAML
{
template <>
struct convert<EffectiveStats> {
    static Node encode( const EffectiveStats& rhs )
    {
        return ToNode( rhs );
    }
    static bool decode( const Node& node, EffectiveStats& rhs )
    {
        return FromNode( node, rhs );
    }
};
}   // namespace YAML