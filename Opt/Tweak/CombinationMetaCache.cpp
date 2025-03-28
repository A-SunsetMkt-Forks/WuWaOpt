//
// Created by EMCJava on 7/26/2024.
//

#include "CombinationMetaCache.hpp"
#include "CombinationTweaker.hpp"

#include <Opt/OptimizerParmSwitcher.hpp>

CombinationMetaCache::CombinationMetaCache( const std::vector<EffectiveStats>& EffectiveEchoList )
    : EffectiveEchoList( EffectiveEchoList )
{
}

void
CombinationMetaCache::Deactivate( )
{
    m_Valid            = false;
    m_CombinationStats = m_DisplayStats = m_IncreasePayOff = { };
}

void
CombinationMetaCache::SetAsCombination( const Backpack& BackPack, const PlotCombinationMeta& CombinationDetails, const CharacterConfig& Config )
{
    SlotCount = std::distance( CombinationDetails.Indices.begin( ), std::ranges::find( CombinationDetails.Indices, -1 ) );

    std::array<int, 5> NewEchoIndices { };
    for ( int i = 0; i < SlotCount; ++i )
        NewEchoIndices[ i ] = CombinationDetails.Indices[ i ];

    // Check if the combination has changed
    if ( m_Valid && m_ElementOffset == (int) Config.CharacterElement && m_FoundationOffset == (int) Config.CharacterStatsFoundation && m_CachedStateID == Config.InternalStageID + BackPack.GetHash( ) && std::ranges::equal( NewEchoIndices, m_CombinationEchoIndices ) ) return;
    m_CombinationEchoIndices = NewEchoIndices;
    m_CommonStats            = Config.GetCombinedStatsWithoutFoundation( );
    m_CachedStateID          = Config.InternalStageID + BackPack.GetHash( );
    m_CharacterCfg           = Config;

    m_ElementOffset    = (int) Config.CharacterElement;
    m_FoundationOffset = (int) Config.CharacterStatsFoundation;
    m_Valid            = true;

    const auto IndexToEchoTransform =
        std::views::transform( [ & ]( int EchoIndex ) -> EffectiveStats {
            return EffectiveEchoList[ m_CombinationEchoIndices[ EchoIndex ] ];
        } );

    m_Echoes =
        std::views::iota( 0, SlotCount )
        | IndexToEchoTransform
        | std::ranges::to<std::vector>( );

    m_FullEchoes =
        std::views::iota( 0, SlotCount )
        | std::views::transform( [ & ]( int EchoIndex ) -> FullStats {
              return BackPack.GetSelectedContent( )[ m_CombinationEchoIndices[ EchoIndex ] ];
          } )
        | std::ranges::to<std::vector>( );

    m_EchoNames =
        std::views::iota( 0, SlotCount )
        | std::views::transform( [ & ]( int EchoIndex ) {
              return BackPack.GetSelectedContent( )[ m_CombinationEchoIndices[ EchoIndex ] ].EchoName;
          } )
        | std::ranges::to<std::vector>( );

    m_CombinationStats =
        OptimizerParmSwitcher::SwitchCalculateCombinationalStat(
            m_ElementOffset,
            m_Echoes,
            m_CommonStats );

    for ( int i = 0; i < SlotCount; ++i )
    {
        m_EchoesWithoutAt[ i ] =
            std::views::iota( 0, SlotCount )
            | std::views::filter( [ i ]( int EchoIndex ) { return EchoIndex != i; } )
            | IndexToEchoTransform
            | std::ranges::to<std::vector>( );
    }

    m_DisplayStats.flat_attack  = m_CombinationStats.flat_attack;
    m_DisplayStats.flat_health  = m_CombinationStats.flat_health;
    m_DisplayStats.flat_defence = m_CombinationStats.flat_defence;

    m_DisplayStats.percentage_attack  = m_CombinationStats.percentage_attack * 100;
    m_DisplayStats.percentage_health  = m_CombinationStats.percentage_health * 100;
    m_DisplayStats.percentage_defence = m_CombinationStats.percentage_defence * 100;

    m_DisplayStats.regen             = m_CombinationStats.RegenStat( ) * 100;
    m_DisplayStats.buff_multiplier   = m_CombinationStats.buff_multiplier * 100;
    m_DisplayStats.auto_attack_buff  = m_CombinationStats.auto_attack_buff * 100;
    m_DisplayStats.heavy_attack_buff = m_CombinationStats.heavy_attack_buff * 100;
    m_DisplayStats.skill_buff        = m_CombinationStats.skill_buff * 100;
    m_DisplayStats.ult_buff          = m_CombinationStats.ult_buff * 100;
    m_DisplayStats.heal_buff         = m_CombinationStats.heal_buff * 100;
    m_DisplayStats.crit_rate         = m_CombinationStats.CritRateStat( ) * 100;
    m_DisplayStats.crit_damage       = m_CombinationStats.CritDamageStat( ) * 100;

    m_FinalAttack  = m_CombinationStats.FoundationStat( StatsFoundation::eFoundationAttack, m_CharacterCfg.GetBaseFoundation( StatsFoundation::eFoundationAttack ) );
    m_FinalHealth  = m_CombinationStats.FoundationStat( StatsFoundation::eFoundationHealth, m_CharacterCfg.GetBaseFoundation( StatsFoundation::eFoundationHealth ) );
    m_FinalDefence = m_CombinationStats.FoundationStat( StatsFoundation::eFoundationDefence, m_CharacterCfg.GetBaseFoundation( StatsFoundation::eFoundationDefence ) );

    CalculateDamages( );
}

void
CombinationMetaCache::CalculateDamages( )
{
    const FloatTy Resistances    = m_CharacterCfg.GetResistances( );
    const FloatTy BaseFoundation = m_CharacterCfg.GetBaseFoundation( );

    m_CombinationStats.ExtractOptimizingStats( m_CharacterCfg.CharacterStatsFoundation,
                                               BaseFoundation,
                                               Resistances,
                                               &m_CharacterCfg.SkillConfig,
                                               &m_CharacterCfg.CharacterOverallDeepenStats,
                                               m_HealingAmount,
                                               m_NormalDamage,
                                               m_CritDamage,
                                               m_ExpectedDamage,
                                               m_OptimizingValue );

    static constexpr std::array<FloatTy EffectiveStats::*, 12> PercentageStats {
        &EffectiveStats::regen,
        &EffectiveStats::percentage_attack,
        &EffectiveStats::percentage_health,
        &EffectiveStats::percentage_defence,
        &EffectiveStats::buff_multiplier,
        &EffectiveStats::crit_rate,
        &EffectiveStats::crit_damage,
        &EffectiveStats::auto_attack_buff,
        &EffectiveStats::heavy_attack_buff,
        &EffectiveStats::skill_buff,
        &EffectiveStats::ult_buff,
        &EffectiveStats::heal_buff,
    };

    FloatTy MaxDamageBuff = 0;

    {
        auto NewStat = m_CombinationStats;
        NewStat.flat_attack += 1;

        const auto NewExpValue = NewStat.OptimizingValue( m_CharacterCfg.CharacterStatsFoundation, BaseFoundation, Resistances, &m_CharacterCfg.SkillConfig, &m_CharacterCfg.CharacterOverallDeepenStats );
        MaxDamageBuff          = std::max( m_IncreasePayOff.flat_attack = NewExpValue - m_OptimizingValue, MaxDamageBuff );
    }

    {
        auto NewStat = m_CombinationStats;
        NewStat.flat_health += 1;

        const auto NewExpValue = NewStat.OptimizingValue( m_CharacterCfg.CharacterStatsFoundation, BaseFoundation, Resistances, &m_CharacterCfg.SkillConfig, &m_CharacterCfg.CharacterOverallDeepenStats );
        MaxDamageBuff          = std::max( m_IncreasePayOff.flat_health = NewExpValue - m_OptimizingValue, MaxDamageBuff );
    }

    {
        auto NewStat = m_CombinationStats;
        NewStat.flat_defence += 1;

        const auto NewExpValue = NewStat.OptimizingValue( m_CharacterCfg.CharacterStatsFoundation, BaseFoundation, Resistances, &m_CharacterCfg.SkillConfig, &m_CharacterCfg.CharacterOverallDeepenStats );
        MaxDamageBuff          = std::max( m_IncreasePayOff.flat_defence = NewExpValue - m_OptimizingValue, MaxDamageBuff );
    }

    for ( auto StatSlot : PercentageStats )
    {
        auto NewStat = m_CombinationStats;
        NewStat.*StatSlot += 0.01f;

        const auto NewExpValue = NewStat.OptimizingValue( m_CharacterCfg.CharacterStatsFoundation, BaseFoundation, Resistances, &m_CharacterCfg.SkillConfig, &m_CharacterCfg.CharacterOverallDeepenStats );
        MaxDamageBuff          = std::max( m_IncreasePayOff.*StatSlot = NewExpValue - m_OptimizingValue, MaxDamageBuff );
    }

    m_IncreasePayOffWeight.flat_attack = m_IncreasePayOff.flat_attack / MaxDamageBuff;
    for ( auto StatSlot : PercentageStats )
        m_IncreasePayOffWeight.*StatSlot = m_IncreasePayOff.*StatSlot / MaxDamageBuff;

    for ( int i = 0; i < SlotCount; ++i )
    {
        const auto MinMax = CalculateMinMaxPotentialAtSlot( i );
        if ( MinMax.second - MinMax.first > 0.0001f )
        {
            m_EchoScoreAt[ i ] = std::clamp( ( m_OptimizingValue - MinMax.first ) / ( MinMax.second - MinMax.first ), 0.f, 1.f );
        } else
        {
            m_EchoScoreAt[ i ] = 1.f;
        }

        static constexpr auto V   = 2.5;
        m_EchoSigmoidScoreAt[ i ] = 1. / ( 1 + pow( 0.5 * m_EchoScoreAt[ i ] / ( 1 - m_EchoScoreAt[ i ] ), -V ) );
    }
}

FloatTy
CombinationMetaCache::GetOVReplaceEchoAt( int EchoIndex, EffectiveStats Echo ) const
{
    const FloatTy Resistances    = m_CharacterCfg.GetResistances( );
    const FloatTy BaseFoundation = m_CharacterCfg.GetBaseFoundation( );

    // I don't like this
    auto& EchoesReplaced = const_cast<std::vector<EffectiveStats>&>( m_EchoesWithoutAt[ EchoIndex ] );
    EchoesReplaced.push_back( Echo );
    const auto NewOV =
        OptimizerParmSwitcher::SwitchCalculateCombinationalStat(
            m_ElementOffset, EchoesReplaced, m_CommonStats )
            .OptimizingValue( m_CharacterCfg.CharacterStatsFoundation, BaseFoundation, Resistances, &m_CharacterCfg.SkillConfig, &m_CharacterCfg.CharacterOverallDeepenStats );
    EchoesReplaced.pop_back( );

    EchoesReplaced.push_back( m_Echoes[ EchoIndex ] );
    const auto ttt =
        OptimizerParmSwitcher::SwitchCalculateCombinationalStat(
            m_ElementOffset, EchoesReplaced, m_CommonStats )
            .OptimizingValue( m_CharacterCfg.CharacterStatsFoundation, BaseFoundation, Resistances, &m_CharacterCfg.SkillConfig, &m_CharacterCfg.CharacterOverallDeepenStats );
    EchoesReplaced.pop_back( );


    if ( NewOV >= 144.34419 )
    {
        auto a = 0;
    }

    return NewOV;
}

std::vector<int>
CombinationMetaCache::GetCombinationIndices( ) const noexcept
{
    return m_CombinationEchoIndices | std::views::take( SlotCount ) | std::ranges::to<std::vector>( );
}

std::pair<FloatTy, FloatTy>
CombinationMetaCache::CalculateMinMaxPotentialAtSlot( int EchoSlot ) const
{
    return CombinationTweaker { *this }.CalculateSubStatMinMaxExpectedDamage( EchoSlot );
}
